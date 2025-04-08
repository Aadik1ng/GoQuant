#pragma once

#include <string>

namespace deribit {

/**
 * @brief Configuration for the Deribit API client
 */
class Config {
public:
    /**
     * @brief Constructor
     */
    Config() = default;

    /**
     * @brief Constructor with parameters
     * @param api_key API key
     * @param api_secret API secret
     * @param testnet Whether to use testnet (true) or mainnet (false)
     */
    Config(
        const std::string& api_key,
        const std::string& api_secret,
        bool testnet = true);

    /**
     * @brief Get the API key
     * @return The API key
     */
    const std::string& getApiKey() const { return api_key_; }

    /**
     * @brief Get the API secret
     * @return The API secret
     */
    const std::string& getApiSecret() const { return api_secret_; }

    /**
     * @brief Check if testnet is enabled
     * @return true if testnet is enabled, false otherwise
     */
    bool isTestnet() const { return testnet_; }

    /**
     * @brief Get the REST API URL
     * @return The REST API URL
     */
    std::string getRestApiUrl() const {
        return testnet_ ? "https://test.deribit.com/api/v2" : "https://www.deribit.com/api/v2";
    }

    /**
     * @brief Get the WebSocket API URL
     * @return The WebSocket API URL
     */
    std::string getWebSocketApiUrl() const {
        return testnet_ ? "wss://test.deribit.com/ws/api/v2" : "wss://www.deribit.com/ws/api/v2";
    }

private:
    std::string api_key_;
    std::string api_secret_;
    bool testnet_{true};
};

} // namespace deribit 