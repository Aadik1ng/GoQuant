#pragma once

#include <string>
#include <functional>
#include <memory>
#include <chrono>
#include <nlohmann/json.hpp>
#include <curl/curl.h>
#include "deribit/config.hpp"

namespace deribit {

/**
 * @brief REST client for interacting with the Deribit API
 */
class RestClient {
public:
    /**
     * @brief Constructor
     * @param config Configuration for the REST client
     */
    explicit RestClient(const Config& config);
    
    /**
     * @brief Destructor
     */
    ~RestClient();

    /**
     * @brief Initialize the REST client
     * @return true if initialization was successful, false otherwise
     */
    bool initialize();

    /**
     * @brief Authenticate with the Deribit API
     * @return The authentication response as JSON
     */
    nlohmann::json authenticate();

    /**
     * @brief Refresh the access token
     * @return The refreshed access token as JSON
     */
    nlohmann::json refreshToken();

    /**
     * @brief Send a GET request to the Deribit API
     * @param endpoint The API endpoint
     * @param params The query parameters
     * @return The response as JSON
     */
    nlohmann::json get(
        const std::string& endpoint,
        const nlohmann::json& params = nlohmann::json());

    /**
     * @brief Send a POST request to the Deribit API
     * @param endpoint The API endpoint
     * @param data The request data
     * @return The response as JSON
     */
    nlohmann::json post(
        const std::string& endpoint,
        const nlohmann::json& data);

    /**
     * @brief Check if the client is authenticated
     * @return true if authenticated, false otherwise
     */
    bool isAuthenticated() const { return is_authenticated_; }

    /**
     * @brief Get the access token
     * @return The access token
     */
    const std::string& getAccessToken() const { return access_token_; }

    /**
     * @brief Check if the token needs to be refreshed
     * @return true if the token needs to be refreshed, false otherwise
     */
    bool needsRefresh() const;

private:
    Config config_;
    CURL* curl_;
    std::string access_token_;
    std::string refresh_token_;
    int expires_in_;
    std::string token_type_;
    bool is_authenticated_{false};
    std::chrono::system_clock::time_point token_expiry_;
    
    // Internal methods
    std::string buildUrl(const std::string& endpoint) const;
    std::string buildAuthHeader() const;
    nlohmann::json handleResponse(const std::string& response) const;
    void updateTokenExpiry();
    bool checkAndRefreshToken();
};

} // namespace deribit 