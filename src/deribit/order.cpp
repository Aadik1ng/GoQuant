#include "deribit/order.hpp"

namespace deribit {

Order::Order(const nlohmann::json& json) {
    if (json.contains("order_id")) {
        order_id_ = json["order_id"].get<std::string>();
    }
    
    if (json.contains("instrument_name")) {
        instrument_name_ = json["instrument_name"].get<std::string>();
    }
    
    if (json.contains("amount")) {
        amount_ = json["amount"].get<double>();
    }
    
    if (json.contains("filled_amount")) {
        filled_amount_ = json["filled_amount"].get<double>();
    }
    
    if (json.contains("price")) {
        price_ = json["price"].get<double>();
    }
    
    if (json.contains("average_price")) {
        average_price_ = json["average_price"].get<double>();
    }
    
    if (json.contains("order_type")) {
        order_type_ = json["order_type"].get<std::string>();
    }
    
    if (json.contains("order_state")) {
        order_state_ = json["order_state"].get<std::string>();
    }
    
    if (json.contains("direction")) {
        direction_ = json["direction"].get<std::string>();
    }
    
    if (json.contains("label")) {
        label_ = json["label"].get<std::string>();
    }
    
    if (json.contains("creation_timestamp")) {
        creation_timestamp_ = json["creation_timestamp"].get<int64_t>();
    }
    
    if (json.contains("last_update_timestamp")) {
        last_update_timestamp_ = json["last_update_timestamp"].get<int64_t>();
    }
}

nlohmann::json Order::toJson() const {
    nlohmann::json json;
    json["order_id"] = order_id_;
    json["instrument_name"] = instrument_name_;
    json["amount"] = amount_;
    json["filled_amount"] = filled_amount_;
    json["price"] = price_;
    json["average_price"] = average_price_;
    json["order_type"] = order_type_;
    json["order_state"] = order_state_;
    json["direction"] = direction_;
    json["label"] = label_;
    json["creation_timestamp"] = creation_timestamp_;
    json["last_update_timestamp"] = last_update_timestamp_;
    return json;
}

} // namespace deribit 