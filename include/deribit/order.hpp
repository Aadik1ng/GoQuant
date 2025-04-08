#pragma once

#include <string>
#include <nlohmann/json.hpp>

namespace deribit {

/**
 * @brief Represents an order
 */
class Order {
public:
    /**
     * @brief Constructor
     */
    Order() = default;
    
    /**
     * @brief Constructor from JSON
     * @param json The JSON data
     */
    explicit Order(const nlohmann::json& json);
    
    /**
     * @brief Get the order ID
     * @return The order ID
     */
    const std::string& getOrderId() const { return order_id_; }
    
    /**
     * @brief Get the instrument name
     * @return The instrument name
     */
    const std::string& getInstrumentName() const { return instrument_name_; }
    
    /**
     * @brief Get the amount
     * @return The amount
     */
    double getAmount() const { return amount_; }
    
    /**
     * @brief Get the filled amount
     * @return The filled amount
     */
    double getFilledAmount() const { return filled_amount_; }
    
    /**
     * @brief Get the price
     * @return The price
     */
    double getPrice() const { return price_; }
    
    /**
     * @brief Get the average price
     * @return The average price
     */
    double getAveragePrice() const { return average_price_; }
    
    /**
     * @brief Get the order type
     * @return The order type
     */
    const std::string& getOrderType() const { return order_type_; }
    
    /**
     * @brief Get the order state
     * @return The order state
     */
    const std::string& getOrderState() const { return order_state_; }
    
    /**
     * @brief Get the direction
     * @return The direction ("buy" or "sell")
     */
    const std::string& getDirection() const { return direction_; }
    
    /**
     * @brief Get the label
     * @return The label
     */
    const std::string& getLabel() const { return label_; }
    
    /**
     * @brief Get the creation timestamp
     * @return The creation timestamp
     */
    int64_t getCreationTimestamp() const { return creation_timestamp_; }
    
    /**
     * @brief Get the last update timestamp
     * @return The last update timestamp
     */
    int64_t getLastUpdateTimestamp() const { return last_update_timestamp_; }
    
    /**
     * @brief Check if the order is a buy order
     * @return true if buy order, false otherwise
     */
    bool isBuy() const { return direction_ == "buy"; }
    
    /**
     * @brief Check if the order is a sell order
     * @return true if sell order, false otherwise
     */
    bool isSell() const { return direction_ == "sell"; }
    
    /**
     * @brief Check if the order is open
     * @return true if open, false otherwise
     */
    bool isOpen() const { return order_state_ == "open"; }
    
    /**
     * @brief Check if the order is filled
     * @return true if filled, false otherwise
     */
    bool isFilled() const { return order_state_ == "filled"; }
    
    /**
     * @brief Check if the order is cancelled
     * @return true if cancelled, false otherwise
     */
    bool isCancelled() const { return order_state_ == "cancelled"; }
    
    /**
     * @brief Convert the order to JSON
     * @return The JSON representation
     */
    nlohmann::json toJson() const;
    
private:
    std::string order_id_;
    std::string instrument_name_;
    double amount_{0.0};
    double filled_amount_{0.0};
    double price_{0.0};
    double average_price_{0.0};
    std::string order_type_;
    std::string order_state_;
    std::string direction_;
    std::string label_;
    int64_t creation_timestamp_{0};
    int64_t last_update_timestamp_{0};
};

} // namespace deribit 