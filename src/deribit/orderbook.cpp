#include "deribit/orderbook.hpp"
#include <algorithm>

namespace deribit {

Orderbook::Orderbook(
    const std::string& instrument_name,
    int64_t timestamp,
    const std::vector<PriceLevel>& bids,
    const std::vector<PriceLevel>& asks)
    : instrument_name_(instrument_name)
    , timestamp_(timestamp)
    , bids_(bids)
    , asks_(asks) {
}

Orderbook::Orderbook(const nlohmann::json& json) {
    if (json.contains("instrument_name")) {
        instrument_name_ = json["instrument_name"].get<std::string>();
    }
    
    if (json.contains("timestamp")) {
        timestamp_ = json["timestamp"].get<int64_t>();
    }
    
    if (json.contains("bids")) {
        const auto& bids_json = json["bids"];
        for (const auto& bid : bids_json) {
            if (bid.is_array() && bid.size() >= 3) {
                std::string action = bid[0].get<std::string>();
                double price = bid[1].get<double>();
                double amount = bid[2].get<double>();
                
                if (action == "new" || action == "change") {
                    bids_.emplace_back(price, amount);
                }
            }
        }
    }
    
    if (json.contains("asks")) {
        const auto& asks_json = json["asks"];
        for (const auto& ask : asks_json) {
            if (ask.is_array() && ask.size() >= 3) {
                std::string action = ask[0].get<std::string>();
                double price = ask[1].get<double>();
                double amount = ask[2].get<double>();
                
                if (action == "new" || action == "change") {
                    asks_.emplace_back(price, amount);
                }
            }
        }
    }
    
    // Sort bids in descending order (highest first)
    std::sort(bids_.begin(), bids_.end(), 
        [](const PriceLevel& a, const PriceLevel& b) {
            return a.price > b.price;
        });
    
    // Sort asks in ascending order (lowest first)
    std::sort(asks_.begin(), asks_.end(), 
        [](const PriceLevel& a, const PriceLevel& b) {
            return a.price < b.price;
        });
}

double Orderbook::getBestBidPrice() const {
    if (bids_.empty()) {
        return 0.0;
    }
    return bids_.front().price;
}

double Orderbook::getBestAskPrice() const {
    if (asks_.empty()) {
        return 0.0;
    }
    return asks_.front().price;
}

double Orderbook::getBestBidAmount() const {
    if (bids_.empty()) {
        return 0.0;
    }
    return bids_.front().amount;
}

double Orderbook::getBestAskAmount() const {
    if (asks_.empty()) {
        return 0.0;
    }
    return asks_.front().amount;
}

void Orderbook::update(const nlohmann::json& json) {
    if (json.contains("instrument_name")) {
        instrument_name_ = json["instrument_name"].get<std::string>();
    }
    
    if (json.contains("timestamp")) {
        timestamp_ = json["timestamp"].get<int64_t>();
    }
    
    if (json.contains("bids")) {
        const auto& bids_json = json["bids"];
        for (const auto& bid : bids_json) {
            if (bid.is_array() && bid.size() >= 2) {
                double price = bid[0].get<double>();
                double amount = bid[1].get<double>();
                
                // Find if the price level already exists
                auto it = std::find_if(bids_.begin(), bids_.end(),
                    [price](const PriceLevel& level) {
                        return level.price == price;
                    });
                
                if (it != bids_.end()) {
                    // Update existing price level
                    if (amount > 0) {
                        it->amount = amount;
                    } else {
                        // Remove price level if amount is 0
                        bids_.erase(it);
                    }
                } else if (amount > 0) {
                    // Add new price level
                    bids_.emplace_back(price, amount);
                }
            }
        }
        
        // Sort bids in descending order (highest first)
        std::sort(bids_.begin(), bids_.end(), 
            [](const PriceLevel& a, const PriceLevel& b) {
                return a.price > b.price;
            });
    }
    
    if (json.contains("asks")) {
        const auto& asks_json = json["asks"];
        for (const auto& ask : asks_json) {
            if (ask.is_array() && ask.size() >= 2) {
                double price = ask[0].get<double>();
                double amount = ask[1].get<double>();
                
                // Find if the price level already exists
                auto it = std::find_if(asks_.begin(), asks_.end(),
                    [price](const PriceLevel& level) {
                        return level.price == price;
                    });
                
                if (it != asks_.end()) {
                    // Update existing price level
                    if (amount > 0) {
                        it->amount = amount;
                    } else {
                        // Remove price level if amount is 0
                        asks_.erase(it);
                    }
                } else if (amount > 0) {
                    // Add new price level
                    asks_.emplace_back(price, amount);
                }
            }
        }
        
        // Sort asks in ascending order (lowest first)
        std::sort(asks_.begin(), asks_.end(), 
            [](const PriceLevel& a, const PriceLevel& b) {
                return a.price < b.price;
            });
    }
}

nlohmann::json Orderbook::toJson() const {
    nlohmann::json json;
    json["instrument_name"] = instrument_name_;
    json["timestamp"] = timestamp_;
    
    nlohmann::json bids_json = nlohmann::json::array();
    for (const auto& bid : bids_) {
        nlohmann::json bid_json = nlohmann::json::array();
        bid_json.push_back(bid.price);
        bid_json.push_back(bid.amount);
        bids_json.push_back(bid_json);
    }
    json["bids"] = bids_json;
    
    nlohmann::json asks_json = nlohmann::json::array();
    for (const auto& ask : asks_) {
        nlohmann::json ask_json = nlohmann::json::array();
        ask_json.push_back(ask.price);
        ask_json.push_back(ask.amount);
        asks_json.push_back(ask_json);
    }
    json["asks"] = asks_json;
    
    return json;
}

} // namespace deribit 