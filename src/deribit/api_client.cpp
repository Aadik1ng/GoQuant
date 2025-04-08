#include "deribit/api_client.hpp"
#include <iostream>
#include <sstream>
#include <chrono>
#include <thread>

namespace deribit {

ApiClient::ApiClient(const Config& config)
    : config_(config) {
}

ApiClient::~ApiClient() {
    if (ws_running_) {
        ws_running_ = false;
        if (ws_thread_.joinable()) {
            ws_thread_.join();
        }
    }
}

bool ApiClient::initialize() {
    // Initialize REST client
    rest_client_ = std::make_unique<RestClient>(config_);
    if (!rest_client_->initialize()) {
        std::cerr << "Failed to initialize REST client" << std::endl;
        return false;
    }
    
    // Initialize WebSocket client
    ws_client_ = std::make_unique<WebSocketClient>(config_);
    if (!ws_client_->initialize()) {
        std::cerr << "Failed to initialize WebSocket client" << std::endl;
        return false;
    }
    
    is_initialized_ = true;
    return true;
}

bool ApiClient::authenticate() {
    if (!is_initialized_) {
        std::cerr << "API client not initialized" << std::endl;
        return false;
    }
    
    // First authenticate with REST API
    nlohmann::json auth_response = rest_client_->authenticate();
    if (!rest_client_->isAuthenticated()) {
        std::cerr << "REST authentication failed" << std::endl;
        return false;
    }
    
    std::cout << "REST authentication successful" << std::endl;
    
    // Connect to WebSocket API
    if (!ws_client_->connect()) {
        std::cerr << "WebSocket connection failed" << std::endl;
        return false;
    }
    
    // Authenticate with WebSocket API using the access token from REST
    if (!ws_client_->authenticate(rest_client_->getAccessToken())) {
        std::cerr << "WebSocket authentication failed" << std::endl;
        return false;
    }
    
    std::cout << "WebSocket authentication successful" << std::endl;
    is_authenticated_ = true;
    
    // Start WebSocket message processing thread
    ws_running_ = true;
    ws_thread_ = std::thread(&ApiClient::processWebSocketMessages, this);
    
    return true;
}

bool ApiClient::placeBuyOrder(const std::string& instrument_name, double amount, const std::string& type, double price, const std::string& label) {
    try {
        // Ensure the client is authenticated
        if (!is_authenticated_) {
            std::cerr << "API client not authenticated" << std::endl;
            return false;
        }

        // Refresh token if needed
        if (rest_client_->needsRefresh()) {
            if (!rest_client_->refreshToken().contains("result")) {
                std::cerr << "Failed to refresh authentication token" << std::endl;
                return false;
            }
        }

        // Define contract size (example value, adjust as needed)
        const double contract_size = 0.01;  // Example contract size

        // Check if amount is a multiple of contract size
        if (fmod(amount, contract_size) != 0.0) {
            std::cerr << "Amount must be a multiple of contract size: " << contract_size << std::endl;
            return false;
        }

        // Create JSON-RPC request
        nlohmann::json request = {
            {"jsonrpc", "2.0"},
            {"id", 5275},
            {"method", "private/buy"},
            {"params", {
                {"instrument_name", instrument_name},
                {"amount", amount},
                {"type", type},
                {"access_token", rest_client_->getAccessToken()}
            }}
        };

        if (type == "limit") {
            request["params"]["price"] = price;
        }

        if (!label.empty()) {
            request["params"]["label"] = label;
        }

        // Send the request over WebSocket
        if (!ws_client_->send(request.dump())) {
            std::cerr << "Failed to send order request" << std::endl;
            return false;
        }

        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error placing buy order: " << e.what() << std::endl;
        return false;
    }
}

bool ApiClient::placeSellOrder(const std::string& instrument_name, double amount, const std::string& type, double price, const std::string& label) {
    try {
        // Build the endpoint with query parameters
        std::string endpoint = "private/sell";
        std::string query = "amount=" + std::to_string(amount) + 
                           "&instrument_name=" + instrument_name + 
                           "&type=" + type;
        
        if (type == "limit") {
            query += "&price=" + std::to_string(price);
        }
        
        if (!label.empty()) {
            query += "&label=" + label;
        }
        
        // Use GET request with query parameters
        auto response = rest_client_->get(endpoint + "?" + query, nlohmann::json());
        
        if (response.contains("result")) {
            std::cout << "Order placed successfully" << std::endl;
            return true;
        } else if (response.contains("error")) {
            std::cerr << "Order placement failed: " << response["error"]["message"].get<std::string>() << std::endl;
            return false;
        }
        
        return false;
    } catch (const std::exception& e) {
        std::cerr << "Error placing sell order: " << e.what() << std::endl;
        return false;
    }
}

bool ApiClient::cancelOrder(const std::string& order_id) {
    if (!is_authenticated_) {
        std::cerr << "API client not authenticated" << std::endl;
        return false;
    }
    
    nlohmann::json request = {
        {"jsonrpc", "2.0"},
        {"id", 42},
        {"method", "private/cancel"},
        {"params", {
            {"order_id", order_id}
        }}
    };
    
    nlohmann::json response = rest_client_->post("", request);
    
    if (response.contains("error")) {
        std::cerr << "Order cancellation failed: " << response["error"]["message"].get<std::string>() << std::endl;
        return false;
    }
    
    return response.contains("result");
}

bool ApiClient::modifyOrder(
    const std::string& order_id,
    double amount,
    double price) {
    
    if (!is_authenticated_) {
        std::cerr << "API client not authenticated" << std::endl;
        return false;
    }
    
    nlohmann::json request = {
        {"jsonrpc", "2.0"},
        {"id", 42},
        {"method", "private/edit"},
        {"params", {
            {"order_id", order_id},
            {"amount", amount},
            {"price", price}
        }}
    };
    
    nlohmann::json response = rest_client_->post("", request);
    
    if (response.contains("error")) {
        std::cerr << "Order modification failed: " << response["error"]["message"].get<std::string>() << std::endl;
        return false;
    }
    
    return response.contains("result");
}

Orderbook ApiClient::getOrderbook(
    const std::string& instrument_name,
    int depth) {
    
    nlohmann::json request = {
        {"jsonrpc", "2.0"},
        {"id", 42},
        {"method", "public/get_order_book"},
        {"params", {
            {"instrument_name", instrument_name},
            {"depth", depth}
        }}
    };
    
    nlohmann::json response = rest_client_->get("", request);
    
    if (response.contains("result")) {
        return Orderbook(response["result"]);
    }
    
    return Orderbook();
}

std::vector<Position> ApiClient::getPositions(
    const std::string& currency,
    const std::string& kind) {
    
    if (!is_authenticated_) {
        std::cerr << "API client not authenticated" << std::endl;
        return {};
    }
    
    nlohmann::json request = {
        {"jsonrpc", "2.0"},
        {"id", 42},
        {"method", "private/get_positions"},
        {"params", {
            {"currency", currency}
        }}
    };
    
    if (!kind.empty()) {
        request["params"]["kind"] = kind;
    }
    
    nlohmann::json response = rest_client_->get("", request);
    
    std::vector<Position> positions;
    
    if (response.contains("result")) {
        const auto& result = response["result"];
        for (const auto& position_json : result) {
            positions.emplace_back(position_json);
        }
    }
    
    return positions;
}

std::vector<Order> ApiClient::getOpenOrders(
    const std::string& instrument_name) {
    
    if (!is_authenticated_) {
        std::cerr << "API client not authenticated" << std::endl;
        return {};
    }
    
    nlohmann::json request = {
        {"jsonrpc", "2.0"},
        {"id", 42},
        {"method", "private/get_open_orders_by_currency"},
        {"params", {}}
    };
    
    if (!instrument_name.empty()) {
        request["params"]["instrument_name"] = instrument_name;
    }
    
    nlohmann::json response = rest_client_->get("", request);
    
    std::vector<Order> orders;
    
    if (response.contains("result")) {
        const auto& result = response["result"];
        for (const auto& order_json : result) {
            orders.emplace_back(order_json);
        }
    }
    
    return orders;
}

bool ApiClient::subscribeOrderbook(
    const std::string& instrument_name,
    std::function<void(const Orderbook&)> callback) {
    
    if (!is_authenticated_) {
        std::cerr << "API client not authenticated" << std::endl;
        return false;
    }
    
    std::string channel = "book." + instrument_name + ".100ms";
    
    // Store the callback
    {
        std::lock_guard<std::mutex> lock(callbacks_mutex_);
        orderbook_callbacks_[instrument_name] = callback;
    }
    
    // Subscribe to the channel
    nlohmann::json params;
    params["instrument_name"] = instrument_name;
    
    return ws_client_->subscribe(channel, params);
}

bool ApiClient::unsubscribeOrderbook(const std::string& instrument_name) {
    if (!is_authenticated_) {
        std::cerr << "API client not authenticated" << std::endl;
        return false;
    }
    
    std::string channel = "book." + instrument_name + ".100ms";
    
    // Remove the callback
    {
        std::lock_guard<std::mutex> lock(callbacks_mutex_);
        orderbook_callbacks_.erase(instrument_name);
    }
    
    return ws_client_->unsubscribe(channel);
}

bool ApiClient::isConnected() const {
    return is_authenticated_ && ws_client_->isConnected();
}

void ApiClient::processWebSocketMessages() {
    // Set up message callback
    ws_client_->setMessageCallback([this](const std::string& message) {
        try {
            nlohmann::json json = nlohmann::json::parse(message);
            
            // Check if it's a notification
            if (json.contains("method") && json["method"] == "subscription") {
                // Check if it's an orderbook update
                if (json.contains("params") && json["params"].contains("channel") && 
                    json["params"]["channel"].get<std::string>().find("book.") == 0) {
                    
                    handleOrderbookUpdate(json["params"]["data"]);
                }
            }
        } catch (const nlohmann::json::exception& e) {
            std::cerr << "JSON parsing error: " << e.what() << std::endl;
        }
    });
    
    // Keep the thread running
    while (ws_running_) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void ApiClient::handleOrderbookUpdate(const nlohmann::json& data) {
    if (!data.contains("instrument_name")) {
        return;
    }
    
    std::string instrument_name = data["instrument_name"].get<std::string>();
    
    // Find the callback for this instrument
    std::function<void(const Orderbook&)> callback;
    {
        std::lock_guard<std::mutex> lock(callbacks_mutex_);
        auto it = orderbook_callbacks_.find(instrument_name);
        if (it != orderbook_callbacks_.end()) {
            callback = it->second;
        }
    }
    
    // Call the callback if found
    if (callback) {
        try {
            Orderbook orderbook(data);
            callback(orderbook);
        } catch (const std::exception& e) {
            std::cerr << "Error processing orderbook update: " << e.what() << std::endl;
        }
    }
}

} // namespace deribit 