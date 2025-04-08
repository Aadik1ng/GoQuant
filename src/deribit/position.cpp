#include "deribit/position.hpp"

namespace deribit {

Position::Position(const nlohmann::json& json) {
    if (json.contains("instrument_name")) {
        instrument_name_ = json["instrument_name"].get<std::string>();
    }
    
    if (json.contains("size")) {
        size_ = json["size"].get<double>();
    }
    
    if (json.contains("average_price")) {
        average_price_ = json["average_price"].get<double>();
    }
    
    if (json.contains("estimated_liquidation_price")) {
        liquidation_price_ = json["estimated_liquidation_price"].get<double>();
    }
    
    if (json.contains("mark_price")) {
        mark_price_ = json["mark_price"].get<double>();
    }
    
    if (json.contains("index_price")) {
        index_price_ = json["index_price"].get<double>();
    }
    
    if (json.contains("initial_margin")) {
        initial_margin_ = json["initial_margin"].get<double>();
    }
    
    if (json.contains("maintenance_margin")) {
        maintenance_margin_ = json["maintenance_margin"].get<double>();
    }
    
    if (json.contains("floating_profit_loss")) {
        unrealized_pnl_ = json["floating_profit_loss"].get<double>();
    }
    
    if (json.contains("realized_profit_loss")) {
        realized_pnl_ = json["realized_profit_loss"].get<double>();
    }
    
    if (json.contains("direction")) {
        direction_ = json["direction"].get<std::string>();
    }
}

nlohmann::json Position::toJson() const {
    nlohmann::json json;
    json["instrument_name"] = instrument_name_;
    json["size"] = size_;
    json["average_price"] = average_price_;
    json["estimated_liquidation_price"] = liquidation_price_;
    json["mark_price"] = mark_price_;
    json["index_price"] = index_price_;
    json["initial_margin"] = initial_margin_;
    json["maintenance_margin"] = maintenance_margin_;
    json["floating_profit_loss"] = unrealized_pnl_;
    json["realized_profit_loss"] = realized_pnl_;
    json["direction"] = direction_;
    return json;
}

} // namespace deribit 