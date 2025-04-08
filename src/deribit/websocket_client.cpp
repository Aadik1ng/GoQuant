#include "deribit/websocket_client.hpp"
#include <iostream>
#include <sstream>
#include <chrono>
#include <thread>

namespace deribit {

WebSocketClient::WebSocketClient(const Config& config)
    : config_(config) {
}

WebSocketClient::~WebSocketClient() {
    disconnect();
}

bool WebSocketClient::initialize() {
    try {
        client_.clear_access_channels(websocketpp::log::alevel::all);
        client_.set_access_channels(websocketpp::log::alevel::connect);
        client_.set_access_channels(websocketpp::log::alevel::disconnect);
        client_.set_access_channels(websocketpp::log::alevel::app);

        client_.init_asio();
        client_.set_tls_init_handler(
            std::bind(&WebSocketClient::onTlsInit, this, std::placeholders::_1));

        client_.set_open_handler(
            std::bind(&WebSocketClient::onOpen, this, std::placeholders::_1));
        client_.set_close_handler(
            std::bind(&WebSocketClient::onClose, this, std::placeholders::_1));
        client_.set_message_handler(
            std::bind(&WebSocketClient::onMessage, this,
                std::placeholders::_1,
                std::placeholders::_2));
        client_.set_fail_handler(
            std::bind(&WebSocketClient::onFail, this, std::placeholders::_1));

        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error initializing WebSocket client: " << e.what() << std::endl;
        return false;
    }
}

bool WebSocketClient::connect() {
    if (is_connected_) {
        return true;
    }

    try {
        std::string uri = config_.isTestnet()
            ? "wss://test.deribit.com/ws/api/v2"
            : "wss://www.deribit.com/ws/api/v2";

        websocketpp::lib::error_code ec;
        auto conn = client_.get_connection(uri, ec);
        if (ec) {
            std::cerr << "Could not create connection: " << ec.message() << std::endl;
            return false;
        }

        connection_ = conn->get_handle();
        client_.connect(conn);

        is_running_ = true;
        ws_thread_ = std::thread(&WebSocketClient::run, this);

        // Wait for connection to be established (up to 5 seconds)
        for (int i = 0; i < 10; i++) {
            if (is_connected_) {
                // Add a small delay to ensure connection is fully ready
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                return true;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }

        std::cerr << "Connection timed out" << std::endl;
        return false;
    } catch (const std::exception& e) {
        std::cerr << "Error connecting to WebSocket server: " << e.what() << std::endl;
        return false;
    }
}

void WebSocketClient::disconnect() {
    if (!is_connected_) {
        return;
    }

    try {
        is_running_ = false;
        websocketpp::lib::error_code ec;
        client_.close(connection_, websocketpp::close::status::normal, "", ec);
        if (ec) {
            std::cerr << "Error closing connection: " << ec.message() << std::endl;
        }

        if (ws_thread_.joinable()) {
            ws_thread_.join();
        }
    } catch (const std::exception& e) {
        std::cerr << "Error disconnecting from WebSocket server: " << e.what() << std::endl;
    }
}

bool WebSocketClient::authenticate(const std::string& access_token) {
    if (!is_connected_) {
        return false;
    }

    try {
        nlohmann::json auth_request = {
            {"jsonrpc", "2.0"},
            {"id", 9929},
            {"method", "public/auth"},
            {"params", {
                {"grant_type", "client_credentials"},
                {"client_id", config_.getApiKey()},
                {"client_secret", config_.getApiSecret()}
            }}
        };

        std::cout << "Sending WebSocket authentication request..." << std::endl;
        
        websocketpp::lib::error_code ec;
        client_.send(connection_, auth_request.dump(),
            websocketpp::frame::opcode::text, ec);
        
        if (ec) {
            std::cerr << "Error sending authentication request: " << ec.message() << std::endl;
            return false;
        }

        // Wait for authentication response (up to 5 seconds)
        for (int i = 0; i < 10; i++) {
            if (is_authenticated_) {
                return true;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
        
        std::cerr << "WebSocket authentication timed out" << std::endl;
        return false;
    } catch (const std::exception& e) {
        std::cerr << "Error during authentication: " << e.what() << std::endl;
        return false;
    }
}

bool WebSocketClient::send(const std::string& message) {
    if (!is_connected_) {
        return false;
    }

    try {
        websocketpp::lib::error_code ec;
        client_.send(connection_, message, websocketpp::frame::opcode::text, ec);
        if (ec) {
            std::cerr << "Error sending message: " << ec.message() << std::endl;
            return false;
        }
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error sending message: " << e.what() << std::endl;
        return false;
    }
}

bool WebSocketClient::subscribe(const std::string& channel,
    const nlohmann::json& params) {
    if (!is_connected_ || !is_authenticated_) {
        return false;
    }

    try {
        nlohmann::json sub_request = {
            {"jsonrpc", "2.0"},
            {"id", 9930},
            {"method", "public/subscribe"},
            {"params", {
                {"channels", {channel}}
            }}
        };

        if (!params.is_null()) {
            sub_request["params"].update(params);
        }

        return send(sub_request.dump());
    } catch (const std::exception& e) {
        std::cerr << "Error subscribing to channel: " << e.what() << std::endl;
        return false;
    }
}

bool WebSocketClient::unsubscribe(const std::string& channel) {
    if (!is_connected_ || !is_authenticated_) {
        return false;
    }

    try {
        nlohmann::json unsub_request = {
            {"jsonrpc", "2.0"},
            {"id", 9931},
            {"method", "public/unsubscribe"},
            {"params", {
                {"channels", {channel}}
            }}
        };

        return send(unsub_request.dump());
    } catch (const std::exception& e) {
        std::cerr << "Error unsubscribing from channel: " << e.what() << std::endl;
        return false;
    }
}

void WebSocketClient::setMessageCallback(
    std::function<void(const std::string&)> callback) {
    message_callback_ = std::move(callback);
}

void WebSocketClient::onOpen(ConnectionHandle hdl) {
    is_connected_ = true;
    std::cout << "WebSocket connection established" << std::endl;
}

void WebSocketClient::onClose(ConnectionHandle hdl) {
    is_connected_ = false;
    is_authenticated_ = false;
    std::cout << "WebSocket connection closed" << std::endl;
}

void WebSocketClient::onMessage(ConnectionHandle hdl, MessagePtr msg) {
    try {
        const auto& payload = msg->get_payload();
        
        auto json = nlohmann::json::parse(payload);

        // Check for authentication response
        if (json.contains("id") && json["id"] == 9929) {
            if (json.contains("result") && !json.contains("error")) {
                is_authenticated_ = true;
                std::cout << "WebSocket authentication successful" << std::endl;
            } else if (json.contains("error")) {
                std::cerr << "WebSocket authentication failed: " 
                    << json["error"]["message"].get<std::string>() << std::endl;
                if (json["error"].contains("data")) {
                    std::cerr << "Error data: " << json["error"]["data"].dump() << std::endl;
                }
            }
        }
        // Only print subscription confirmation
        else if (json.contains("id") && json["id"] == 9930) {
            std::cout << "Successfully subscribed to channel" << std::endl;
        }
        // Don't print orderbook updates to avoid flooding the console
        else if (json.contains("method") && json["method"] == "subscription") {
            // Only forward the message to the callback
            if (message_callback_) {
                message_callback_(payload);
            }
        }
        // Print other messages for debugging
        else {
            std::cout << "Received WebSocket message: " << payload << std::endl;
            if (message_callback_) {
                message_callback_(payload);
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error processing message: " << e.what() << std::endl;
    }
}

void WebSocketClient::onFail(ConnectionHandle hdl) {
    is_connected_ = false;
    is_authenticated_ = false;
    std::cerr << "WebSocket connection failed" << std::endl;
}

std::shared_ptr<WebSocketClient::Context> WebSocketClient::onTlsInit(ConnectionHandle hdl) {
    auto ctx = std::make_shared<Context>(Context::tlsv12);
    try {
        ctx->set_options(
            Context::default_workarounds |
            Context::no_sslv2 |
            Context::no_sslv3 |
            Context::single_dh_use);
    } catch (const std::exception& e) {
        std::cerr << "Error in TLS initialization: " << e.what() << std::endl;
    }
    return ctx;
}

void WebSocketClient::run() {
    while (is_running_) {
        try {
            client_.run_one();
        } catch (const std::exception& e) {
            std::cerr << "Error in WebSocket run loop: " << e.what() << std::endl;
            break;
        }
    }
}

} // namespace deribit 