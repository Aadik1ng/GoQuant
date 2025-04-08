#pragma once

#include <string>
#include <functional>
#include <memory>
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <atomic>
#include <nlohmann/json.hpp>

#define ASIO_STANDALONE
#include <asio/version.hpp>
#include <asio/ssl.hpp>
#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>

#include "deribit/config.hpp"

namespace deribit {

/**
 * @brief WebSocket client for interacting with the Deribit API
 */
class WebSocketClient {
public:
    /**
     * @brief Constructor
     * @param config Configuration for the WebSocket client
     */
    explicit WebSocketClient(const Config& config);
    
    /**
     * @brief Destructor
     */
    ~WebSocketClient();

    /**
     * @brief Initialize the WebSocket client
     * @return true if initialization was successful, false otherwise
     */
    bool initialize();

    /**
     * @brief Connect to the Deribit WebSocket API
     * @return true if connection was successful, false otherwise
     */
    bool connect();

    /**
     * @brief Disconnect from the Deribit WebSocket API
     */
    void disconnect();

    /**
     * @brief Authenticate with the Deribit API
     * @param access_token The access token
     * @return true if authentication was successful, false otherwise
     */
    bool authenticate(const std::string& access_token);

    /**
     * @brief Send a message to the Deribit WebSocket API
     * @param message The message to send
     * @return true if the message was sent successfully, false otherwise
     */
    bool send(const std::string& message);

    /**
     * @brief Subscribe to a channel
     * @param channel The channel to subscribe to
     * @param params The subscription parameters
     * @return true if subscription was successful, false otherwise
     */
    bool subscribe(
        const std::string& channel,
        const nlohmann::json& params = nlohmann::json());

    /**
     * @brief Unsubscribe from a channel
     * @param channel The channel to unsubscribe from
     * @return true if unsubscription was successful, false otherwise
     */
    bool unsubscribe(const std::string& channel);

    /**
     * @brief Set a callback for received messages
     * @param callback The callback function
     */
    void setMessageCallback(std::function<void(const std::string&)> callback);

    /**
     * @brief Check if the client is connected
     * @return true if connected, false otherwise
     */
    bool isConnected() const { return is_connected_; }

    /**
     * @brief Check if the client is authenticated
     * @return true if authenticated, false otherwise
     */
    bool isAuthenticated() const { return is_authenticated_; }

private:
    using ClientConfig = websocketpp::config::asio_tls_client;
    using Client = websocketpp::client<ClientConfig>;
    using ConnectionPtr = Client::connection_ptr;
    using Context = asio::ssl::context;
    using ErrorCode = websocketpp::lib::error_code;
    using MessagePtr = ClientConfig::message_type::ptr;
    using ConnectionHandle = websocketpp::connection_hdl;

    Config config_;
    Client client_;
    ConnectionHandle connection_;
    std::thread ws_thread_;
    std::atomic<bool> is_connected_{false};
    std::atomic<bool> is_authenticated_{false};
    std::atomic<bool> is_running_{false};
    
    std::function<void(const std::string&)> message_callback_;
    
    // Internal methods
    void onOpen(ConnectionHandle hdl);
    void onClose(ConnectionHandle hdl);
    void onMessage(ConnectionHandle hdl, MessagePtr msg);
    void onFail(ConnectionHandle hdl);
    std::shared_ptr<Context> onTlsInit(ConnectionHandle hdl);
    void run();
};

} // namespace deribit 