#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace deribit {

/**
 * @brief Represents a price level in the orderbook
 */
struct PriceLevel {
    double price;
    double amount;
    
    /**
     * @brief Constructor
     * @param price The price
     * @param amount The amount
     */
    PriceLevel(double price, double amount)
        : price(price), amount(amount) {}
};

/**
 * @brief Represents an orderbook for an instrument
 */
class Orderbook {
public:
    /**
     * @brief Constructor
     */
    Orderbook() = default;
    
    /**
     * @brief Constructor with parameters
     * @param instrument_name The instrument name
     * @param timestamp The timestamp
     * @param bids The bids
     * @param asks The asks
     */
    Orderbook(
        const std::string& instrument_name,
        int64_t timestamp,
        const std::vector<PriceLevel>& bids,
        const std::vector<PriceLevel>& asks);
    
    /**
     * @brief Constructor from JSON
     * @param json The JSON data
     */
    explicit Orderbook(const nlohmann::json& json);
    
    /**
     * @brief Get the instrument name
     * @return The instrument name
     */
    const std::string& getInstrumentName() const { return instrument_name_; }
    
    /**
     * @brief Get the timestamp
     * @return The timestamp
     */
    int64_t getTimestamp() const { return timestamp_; }
    
    /**
     * @brief Get the bids
     * @return The bids
     */
    const std::vector<PriceLevel>& getBids() const { return bids_; }
    
    /**
     * @brief Get the asks
     * @return The asks
     */
    const std::vector<PriceLevel>& getAsks() const { return asks_; }
    
    /**
     * @brief Get the best bid price
     * @return The best bid price
     */
    double getBestBidPrice() const;
    
    /**
     * @brief Get the best ask price
     * @return The best ask price
     */
    double getBestAskPrice() const;
    
    /**
     * @brief Get the best bid amount
     * @return The best bid amount
     */
    double getBestBidAmount() const;
    
    /**
     * @brief Get the best ask amount
     * @return The best ask amount
     */
    double getBestAskAmount() const;
    
    /**
     * @brief Update the orderbook with new data
     * @param json The JSON data
     */
    void update(const nlohmann::json& json);
    
    /**
     * @brief Convert the orderbook to JSON
     * @return The JSON representation
     */
    nlohmann::json toJson() const;
    
private:
    std::string instrument_name_;
    int64_t timestamp_{0};
    std::vector<PriceLevel> bids_;
    std::vector<PriceLevel> asks_;
};

} // namespace deribit 