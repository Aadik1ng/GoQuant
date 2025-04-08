#include "deribit/rest_client.hpp"
#include <sstream>
#include <iostream>
#include <curl/curl.h>
#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <chrono>
#include <iomanip>

namespace deribit {

// Callback function for CURL to write response data
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

RestClient::RestClient(const Config& config)
    : config_(config), curl_(nullptr), is_authenticated_(false) {
}

RestClient::~RestClient() {
    if (curl_) {
        curl_easy_cleanup(curl_);
        curl_global_cleanup();
    }
}

bool RestClient::initialize() {
    CURLcode res = curl_global_init(CURL_GLOBAL_ALL);
    if (res != CURLE_OK) {
        return false;
    }

    curl_ = curl_easy_init();
    return (curl_ != nullptr);
}

nlohmann::json RestClient::authenticate() {
    // Create JSON request
    nlohmann::json request = {
        {"jsonrpc", "2.0"},
        {"id", 9929},
        {"method", "public/auth"},
        {"params", {
            {"grant_type", "client_credentials"},
            {"client_id", config_.getApiKey()},
            {"client_secret", config_.getApiSecret()}
        }}
    };
    
    std::string url = buildUrl("");  // Empty endpoint as it's included in the JSON-RPC request
    std::string json_data = request.dump();
    std::string response;
    
    curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, json_data.c_str());
    curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &response);
    
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    
    curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, headers);
    
    CURLcode res = curl_easy_perform(curl_);
    curl_slist_free_all(headers);
    
    if (res != CURLE_OK) {
        std::cerr << "Authentication failed: " << curl_easy_strerror(res) << std::endl;
        is_authenticated_ = false;
        return nlohmann::json();
    }
    
    nlohmann::json json_response = handleResponse(response);
    
    if (json_response.contains("result")) {
        const auto& result = json_response["result"];
        access_token_ = result["access_token"].get<std::string>();
        refresh_token_ = result["refresh_token"].get<std::string>();
        expires_in_ = result["expires_in"].get<int>();
        token_type_ = result["token_type"].get<std::string>();
        is_authenticated_ = true;
        updateTokenExpiry();
        std::cout << "Authentication successful" << std::endl;
    } else {
        std::cerr << "Authentication failed: " << json_response.dump() << std::endl;
        is_authenticated_ = false;
    }
    
    return json_response;
}

nlohmann::json RestClient::refreshToken() {
    if (refresh_token_.empty()) {
        return nlohmann::json();
    }

    // Use form-encoded data instead of JSON
    std::string form_data = "grant_type=refresh_token&refresh_token=" + refresh_token_ + 
                           "&client_id=" + config_.getApiKey() + 
                           "&client_secret=" + config_.getApiSecret();
    
    std::string url = buildUrl("public/auth");
    std::string response;
    
    curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, form_data.c_str());
    curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &response);
    
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
    
    curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, headers);
    
    CURLcode res = curl_easy_perform(curl_);
    curl_slist_free_all(headers);
    
    if (res != CURLE_OK) {
        std::cerr << "Token refresh failed: " << curl_easy_strerror(res) << std::endl;
        is_authenticated_ = false;
        return nlohmann::json();
    }
    
    nlohmann::json json_response = handleResponse(response);
    
    if (json_response.contains("result")) {
        const auto& result = json_response["result"];
        access_token_ = result["access_token"].get<std::string>();
        refresh_token_ = result["refresh_token"].get<std::string>();
        expires_in_ = result["expires_in"].get<int>();
        token_type_ = result["token_type"].get<std::string>();
        updateTokenExpiry();
        std::cout << "Token refresh successful" << std::endl;
        return json_response;
    } else if (json_response.contains("error")) {
        std::cerr << "Token refresh failed: " << json_response.dump() << std::endl;
        is_authenticated_ = false;
        return nlohmann::json();
    }
    
    return json_response;
}

nlohmann::json RestClient::get(const std::string& endpoint, const nlohmann::json& params) {
    if (!curl_) {
        throw std::runtime_error("CURL not initialized");
    }
    
    // Refresh token if needed
    if (is_authenticated_ && needsRefresh()) {
        auto response = refreshToken();
        if (!response.contains("result")) {
            throw std::runtime_error("Failed to refresh authentication token");
        }
    }
    
    std::string url = buildUrl(endpoint);
    std::string response;
    
    curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &response);
    
    struct curl_slist* headers = nullptr;
    
    // Check if this is an order placement request
    bool is_order_request = (endpoint.find("private/buy") != std::string::npos) || 
                           (endpoint.find("private/sell") != std::string::npos);
    
    if (is_order_request) {
        headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
    } else {
        headers = curl_slist_append(headers, "Content-Type: application/json");
    }
    
    if (is_authenticated_) {
        std::string auth_header = "Authorization: Bearer " + access_token_;
        headers = curl_slist_append(headers, auth_header.c_str());
    }
    
    curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, headers);
    
    CURLcode res = curl_easy_perform(curl_);
    curl_slist_free_all(headers);
    
    if (res != CURLE_OK) {
        throw std::runtime_error("Failed to perform GET request: " + std::string(curl_easy_strerror(res)));
    }
    
    long http_code = 0;
    curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &http_code);
    
    if (http_code != 200) {
        std::cerr << "HTTP request failed with code " << http_code << std::endl;
        try {
            auto error_json = nlohmann::json::parse(response);
            if (error_json.contains("error")) {
                std::cerr << "Error message: " << error_json["error"]["message"] << std::endl;
            }
            return error_json;  // Return the error response for better error handling
        } catch (...) {
            // Ignore JSON parse errors for error responses
        }
        throw std::runtime_error("HTTP request failed with code " + std::to_string(http_code));
    }
    
    try {
        return nlohmann::json::parse(response);
    } catch (const nlohmann::json::parse_error& e) {
        throw std::runtime_error("Failed to parse JSON response: " + std::string(e.what()));
    }
}

nlohmann::json RestClient::post(const std::string& endpoint, const nlohmann::json& data) {
    if (!curl_) {
        return nlohmann::json();
    }

    checkAndRefreshToken();

    std::string url = buildUrl(endpoint);
    std::string response;

    // Check if this is an order placement request
    bool is_order_request = (endpoint.find("private/buy") != std::string::npos) || 
                          (endpoint.find("private/sell") != std::string::npos);

    std::string post_data;
    if (is_order_request) {
        // Convert JSON parameters to form data for order requests
        for (auto it = data.begin(); it != data.end(); ++it) {
            if (!post_data.empty()) {
                post_data += "&";
            }
            std::string value;
            if (it.value().is_string()) {
                value = it.value().get<std::string>();
            } else if (it.value().is_number()) {
                std::ostringstream oss;
                oss << std::fixed << std::setprecision(8) << it.value().get<double>();
                value = oss.str();
            } else {
                value = it.value().dump();
            }
            post_data += it.key() + "=" + value;
        }
    } else {
        post_data = data.dump();
    }

    curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl_, CURLOPT_POST, 1L);
    curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, post_data.c_str());
    curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &response);

    struct curl_slist* headers = nullptr;
    
    // Set appropriate content type header
    if (is_order_request) {
        headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
    } else {
        headers = curl_slist_append(headers, "Content-Type: application/json");
    }
    
    if (is_authenticated_) {
        headers = curl_slist_append(headers, buildAuthHeader().c_str());
    }

    curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, headers);
    CURLcode res = curl_easy_perform(curl_);
    curl_slist_free_all(headers);

    if (res != CURLE_OK) {
        std::cerr << "POST request failed: " << curl_easy_strerror(res) << std::endl;
        return nlohmann::json();
    }

    long http_code = 0;
    curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &http_code);
    
    if (http_code != 200) {
        std::cerr << "HTTP request failed with code " << http_code << std::endl;
        try {
            auto error_json = nlohmann::json::parse(response);
            if (error_json.contains("error")) {
                std::cerr << "Error message: " << error_json["error"]["message"] << std::endl;
            }
            return error_json;  // Return the error response for better error handling
        } catch (...) {
            // Ignore JSON parse errors for error responses
        }
        return nlohmann::json();
    }

    return handleResponse(response);
}

std::string RestClient::buildUrl(const std::string& endpoint) const {
    if (endpoint.empty()) {
        return config_.getRestApiUrl();
    }
    return config_.getRestApiUrl() + "/" + endpoint;
}

std::string RestClient::buildAuthHeader() const {
    return "Authorization: Bearer " + access_token_;
}

nlohmann::json RestClient::handleResponse(const std::string& response) const {
    try {
        return nlohmann::json::parse(response);
    } catch (const nlohmann::json::exception& e) {
        std::cerr << "JSON parsing error: " << e.what() << std::endl;
        return nlohmann::json();
    }
}

void RestClient::updateTokenExpiry() {
    token_expiry_ = std::chrono::system_clock::now() + std::chrono::seconds(expires_in_);
}

bool RestClient::needsRefresh() const {
    if (!is_authenticated_ || refresh_token_.empty()) {
        return false;
    }

    auto now = std::chrono::system_clock::now();
    // Refresh if less than 5 minutes until expiry
    return (token_expiry_ - now) < std::chrono::minutes(5);
}

bool RestClient::checkAndRefreshToken() {
    if (needsRefresh()) {
        auto response = refreshToken();
        return response.contains("result");
    }
    return true;
}

} // namespace deribit 