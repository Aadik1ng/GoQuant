#pragma once

#include <string>
#include <memory>
#include <functional>
#include <unordered_map>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <thread>
#include <atomic>
#include <nlohmann/json.hpp>

#include "deribit/websocket_client.hpp"
#include "deribit/rest_client.hpp"
#include "deribit/orderbook.hpp"
#include "deribit/position.hpp"
#include "deribit/order.hpp"
#include "deribit/config.hpp"

namespace deribit {

/**
 * @brief Main API client for interacting with Deribit
 */
class ApiClient {
public:
    /**
     * @brief Constructor
     * @param config Configuration for the API client
     */
    explicit ApiClient(const Config& config);
    
    /**
     * @brief Destructor
     */
    ~ApiClient();

    /**
     * @brief Initialize the API client
     * @return true if initialization was successful, false otherwise
     */
    bool initialize();

    /**
     * @brief Authenticate with the Deribit API
     * @return true if authentication was successful, false otherwise
     */
    bool authenticate();

    /**
     * @brief Place a buy order
     * @param instrument_name The instrument name (e.g., "BTC-PERPETUAL")
     * @param amount The amount to buy
     * @param type The order type (e.g., "market", "limit")
     * @param price The price for limit orders
     * @param label Optional label for the order
     * @return true if order placement was successful, false otherwise
     */
    bool placeBuyOrder(const std::string& instrument_name, double amount, const std::string& type, double price, const std::string& label = "");

    /**
     * @brief Place a sell order
     * @param instrument_name The instrument name (e.g., "BTC-PERPETUAL")
     * @param amount The amount to sell
     * @param type The order type (e.g., "market", "limit")
     * @param price The price for limit orders
     * @param label Optional label for the order
     * @return true if order placement was successful, false otherwise
     */
    bool placeSellOrder(const std::string& instrument_name, double amount, const std::string& type, double price, const std::string& label = "");

    /**
     * @brief Cancel an order
     * @param order_id The ID of the order to cancel
     * @return true if cancellation was successful, false otherwise
     */
    bool cancelOrder(const std::string& order_id);

    /**
     * @brief Modify an existing order
     * @param order_id The ID of the order to modify
     * @param amount The new amount
     * @param price The new price
     * @return true if modification was successful, false otherwise
     */
    bool modifyOrder(
        const std::string& order_id,
        double amount,
        double price);

    /**
     * @brief Get the orderbook for an instrument
     * @param instrument_name The instrument name (e.g., "BTC-PERPETUAL")
     * @param depth The depth of the orderbook
     * @return The orderbook
     */
    Orderbook getOrderbook(
        const std::string& instrument_name,
        int depth = 10);

    /**
     * @brief Get current positions
     * @param currency The currency (e.g., "BTC")
     * @param kind The kind of instrument (e.g., "future", "option")
     * @return A vector of positions
     */
    std::vector<Position> getPositions(
        const std::string& currency,
        const std::string& kind = "");

    /**
     * @brief Get open orders
     * @param instrument_name Optional instrument name to filter by
     * @return A vector of open orders
     */
    std::vector<Order> getOpenOrders(
        const std::string& instrument_name = "");

    /**
     * @brief Subscribe to orderbook updates for an instrument
     * @param instrument_name The instrument name (e.g., "BTC-PERPETUAL")
     * @param callback The callback function to be called when updates are received
     * @return true if subscription was successful, false otherwise
     */
    bool subscribeOrderbook(
        const std::string& instrument_name,
        std::function<void(const Orderbook&)> callback);

    /**
     * @brief Unsubscribe from orderbook updates for an instrument
     * @param instrument_name The instrument name (e.g., "BTC-PERPETUAL")
     * @return true if unsubscription was successful, false otherwise
     */
    bool unsubscribeOrderbook(const std::string& instrument_name);

    /**
     * @brief Check if the client is connected
     * @return true if connected, false otherwise
     */
    bool isConnected() const;

private:
    Config config_;
    std::unique_ptr<RestClient> rest_client_;
    std::unique_ptr<WebSocketClient> ws_client_;
    
    std::unordered_map<std::string, std::function<void(const Orderbook&)>> orderbook_callbacks_;
    std::mutex callbacks_mutex_;
    
    std::atomic<bool> is_initialized_{false};
    std::atomic<bool> is_authenticated_{false};
    
    // WebSocket message processing thread
    std::thread ws_thread_;
    std::atomic<bool> ws_running_{false};
    
    // Internal methods
    void processWebSocketMessages();
    void handleOrderbookUpdate(const nlohmann::json& data);
};

} // namespace deribit 