#pragma once

#include <string>
#include <nlohmann/json.hpp>

namespace deribit {

/**
 * @brief Represents a position in an instrument
 */
class Position {
public:
    /**
     * @brief Constructor
     */
    Position() = default;
    
    /**
     * @brief Constructor from JSON
     * @param json The JSON data
     */
    explicit Position(const nlohmann::json& json);
    
    /**
     * @brief Get the instrument name
     * @return The instrument name
     */
    const std::string& getInstrumentName() const { return instrument_name_; }
    
    /**
     * @brief Get the size
     * @return The size
     */
    double getSize() const { return size_; }
    
    /**
     * @brief Get the average price
     * @return The average price
     */
    double getAveragePrice() const { return average_price_; }
    
    /**
     * @brief Get the liquidation price
     * @return The liquidation price
     */
    double getLiquidationPrice() const { return liquidation_price_; }
    
    /**
     * @brief Get the mark price
     * @return The mark price
     */
    double getMarkPrice() const { return mark_price_; }
    
    /**
     * @brief Get the index price
     * @return The index price
     */
    double getIndexPrice() const { return index_price_; }
    
    /**
     * @brief Get the initial margin
     * @return The initial margin
     */
    double getInitialMargin() const { return initial_margin_; }
    
    /**
     * @brief Get the maintenance margin
     * @return The maintenance margin
     */
    double getMaintenanceMargin() const { return maintenance_margin_; }
    
    /**
     * @brief Get the unrealized profit/loss
     * @return The unrealized profit/loss
     */
    double getUnrealizedPnL() const { return unrealized_pnl_; }
    
    /**
     * @brief Get the realized profit/loss
     * @return The realized profit/loss
     */
    double getRealizedPnL() const { return realized_pnl_; }
    
    /**
     * @brief Get the direction
     * @return The direction ("buy" or "sell")
     */
    const std::string& getDirection() const { return direction_; }
    
    /**
     * @brief Check if the position is long
     * @return true if long, false otherwise
     */
    bool isLong() const { return direction_ == "buy"; }
    
    /**
     * @brief Check if the position is short
     * @return true if short, false otherwise
     */
    bool isShort() const { return direction_ == "sell"; }
    
    /**
     * @brief Convert the position to JSON
     * @return The JSON representation
     */
    nlohmann::json toJson() const;
    
private:
    std::string instrument_name_;
    double size_{0.0};
    double average_price_{0.0};
    double liquidation_price_{0.0};
    double mark_price_{0.0};
    double index_price_{0.0};
    double initial_margin_{0.0};
    double maintenance_margin_{0.0};
    double unrealized_pnl_{0.0};
    double realized_pnl_{0.0};
    std::string direction_;
};

} // namespace deribit 