#include "deribit/api_client.hpp"
#include "deribit/config.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <thread>
#include <iomanip>

// Function to read API credentials from files
bool readCredentials(std::string& api_key, std::string& api_secret) {
    std::cout << "Opening credential files..." << std::endl;
    std::ifstream key_file("api_key.txt");
    std::ifstream secret_file("api_secret.txt");
    
    if (!key_file.is_open() || !secret_file.is_open()) {
        std::cerr << "Failed to open credential files. Please ensure api_key.txt and api_secret.txt exist." << std::endl;
        return false;
    }
    
    std::cout << "Reading API key..." << std::endl;
    std::getline(key_file, api_key);
    std::cout << "Reading API secret..." << std::endl;
    std::getline(secret_file, api_secret);
    
    key_file.close();
    secret_file.close();
    
    std::cout << "Credentials read successfully." << std::endl;
    return true;
}

// Callback function for orderbook updates
void onOrderbookUpdate(const deribit::Orderbook& orderbook) {
    std::cout << "Orderbook update for " << orderbook.getInstrumentName() << " at " 
              << orderbook.getTimestamp() << std::endl;
    
    std::cout << "Best bid: " << orderbook.getBestBidPrice() << " (" 
              << orderbook.getBestBidAmount() << ")" << std::endl;
    
    std::cout << "Best ask: " << orderbook.getBestAskPrice() << " (" 
              << orderbook.getBestAskAmount() << ")" << std::endl;
    
    std::cout << "-------------------" << std::endl;
}

// Function to display the menu
void DisplayMenu() {
    std::cout << "\nSelect an action:\n";
    std::cout << "1. Place Order\n";
    std::cout << "2. Modify Order\n";
    std::cout << "3. Cancel Order\n";
    std::cout << "4. Get Order Book\n";
    std::cout << "5. View Current Positions\n";
    std::cout << "6. Get Open Orders\n";
    std::cout << "7. Connect to WebSocket and Subscribe\n";
    std::cout << "8. Exit\n";
    std::cout << "Enter your choice: ";
}

// Function to handle placing an order
void PlaceOrder(deribit::ApiClient& client) {
    std::string instrument_name, type, direction, label;
    double amount, price;
    
    std::cout << "\nEnter instrument name (e.g., BTC-PERPETUAL): ";
    std::cin >> instrument_name;
    
    std::cout << "Enter order type (limit/market): ";
    std::cin >> type;
    
    std::cout << "Enter direction (buy/sell): ";
    std::cin >> direction;
    
    std::cout << "Enter amount: ";
    std::cin >> amount;
    
    if (type == "limit") {
        std::cout << "Enter price: ";
        std::cin >> price;
    } else {
        price = 0;
    }
    
    std::cout << "Enter label (optional, press enter to skip): ";
    std::cin.ignore();
    std::getline(std::cin, label);
    
    std::string order_id;
    if (direction == "buy") {
        order_id = client.placeBuyOrder(instrument_name, amount, type, price, label);
    } else {
        order_id = client.placeSellOrder(instrument_name, amount, type, price, label);
    }
    
    if (!order_id.empty()) {
        std::cout << "Order placed successfully. Order ID: " << order_id << std::endl;
    } else {
        std::cout << "Failed to place order." << std::endl;
    }
}

// Function to handle modifying an order
void ModifyOrder(deribit::ApiClient& client) {
    std::string order_id;
    double amount, price;
    
    std::cout << "\nEnter order ID: ";
    std::cin >> order_id;
    
    std::cout << "Enter new amount: ";
    std::cin >> amount;
    
    std::cout << "Enter new price: ";
    std::cin >> price;
    
    if (client.modifyOrder(order_id, amount, price)) {
        std::cout << "Order modified successfully." << std::endl;
    } else {
        std::cout << "Failed to modify order." << std::endl;
    }
}

// Function to handle canceling an order
void CancelOrder(deribit::ApiClient& client) {
    std::string order_id;
    
    std::cout << "\nEnter order ID: ";
    std::cin >> order_id;
    
    if (client.cancelOrder(order_id)) {
        std::cout << "Order canceled successfully." << std::endl;
    } else {
        std::cout << "Failed to cancel order." << std::endl;
    }
}

// Function to handle getting the order book
void GetOrderBook(deribit::ApiClient& client) {
    std::string instrument_name;
    int depth;
    
    std::cout << "\nEnter instrument name (e.g., BTC-PERPETUAL): ";
    std::cin >> instrument_name;
    
    std::cout << "Enter depth (1-100): ";
    std::cin >> depth;
    
    deribit::Orderbook orderbook = client.getOrderbook(instrument_name, depth);
    
    std::cout << "\nOrder Book for " << instrument_name << ":" << std::endl;
    std::cout << "Best bid: " << orderbook.getBestBidPrice() << " (" 
              << orderbook.getBestBidAmount() << ")" << std::endl;
    std::cout << "Best ask: " << orderbook.getBestAskPrice() << " (" 
              << orderbook.getBestAskAmount() << ")" << std::endl;
}

// Function to handle viewing current positions
void ViewPositions(deribit::ApiClient& client) {
    std::string currency;
    
    std::cout << "\nEnter currency (BTC/ETH): ";
    std::cin >> currency;
    
    std::vector<deribit::Position> positions = client.getPositions(currency);
    
    if (positions.empty()) {
        std::cout << "No positions found." << std::endl;
        return;
    }
    
    std::cout << "\nCurrent positions:" << std::endl;
    for (const auto& position : positions) {
        std::cout << "Instrument: " << position.getInstrumentName() << std::endl;
        std::cout << "Size: " << position.getSize() << std::endl;
        std::cout << "Average Price: " << position.getAveragePrice() << std::endl;
        std::cout << "Liquidation Price: " << position.getLiquidationPrice() << std::endl;
        std::cout << "-------------------" << std::endl;
    }
}

// Function to handle getting open orders
void GetOpenOrders(deribit::ApiClient& client) {
    std::string instrument_name;
    
    std::cout << "\nEnter instrument name (optional, press enter to skip): ";
    std::cin.ignore();
    std::getline(std::cin, instrument_name);
    
    std::vector<deribit::Order> orders = client.getOpenOrders(instrument_name);
    
    if (orders.empty()) {
        std::cout << "No open orders found." << std::endl;
        return;
    }
    
    std::cout << "\nOpen orders:" << std::endl;
    for (const auto& order : orders) {
        std::cout << "Order ID: " << order.getOrderId() << std::endl;
        std::cout << "Instrument: " << order.getInstrumentName() << std::endl;
        std::cout << "Type: " << order.getOrderType() << std::endl;
        std::cout << "Direction: " << (order.isSell() ? "Sell" : "Buy") << std::endl;
        std::cout << "Price: " << order.getPrice() << std::endl;
        std::cout << "Amount: " << order.getAmount() << std::endl;
        std::cout << "Filled: " << order.getFilledAmount() << std::endl;
        std::cout << "-------------------" << std::endl;
    }
}

// Function to handle WebSocket subscription
void SubscribeToWebSocket(deribit::ApiClient& client) {
    std::string instrument_name;
    
    std::cout << "\nEnter instrument name to subscribe (e.g., BTC-PERPETUAL): ";
    std::cin >> instrument_name;
    
    if (client.subscribeOrderbook(instrument_name, onOrderbookUpdate)) {
        std::cout << "Successfully subscribed to " << instrument_name << " orderbook updates." << std::endl;
    } else {
        std::cout << "Failed to subscribe to orderbook updates." << std::endl;
    }
}

int main() {
    std::cout << "Starting program..." << std::endl << std::flush;
    
    // Read API credentials
    std::string api_key, api_secret;
    std::cout << "Reading credentials..." << std::endl << std::flush;
    if (!readCredentials(api_key, api_secret)) {
        std::cerr << "Failed to read credentials" << std::endl << std::flush;
        return 1;
    }
    
    std::cout << "Using testnet credentials:" << std::endl << std::flush;
    std::cout << "API Key: " << api_key << std::endl << std::flush;
    std::cout << "API Secret: " << api_secret.substr(0, 10) << "..." << std::endl << std::flush;
    
    // Create configuration with testnet enabled
    std::cout << "Creating configuration..." << std::endl << std::flush;
    deribit::Config config(api_key, api_secret, true);
    
    // Create API client
    std::cout << "Creating API client..." << std::endl << std::flush;
    deribit::ApiClient client(config);
    
    // Initialize the client
    std::cout << "Initializing client..." << std::endl << std::flush;
    if (!client.initialize()) {
        std::cerr << "Failed to initialize API client" << std::endl << std::flush;
        return 1;
    }
    
    // Authenticate with the API
    std::cout << "Authenticating with API..." << std::endl << std::flush;
    if (!client.authenticate()) {
        std::cerr << "Failed to authenticate with the API" << std::endl << std::flush;
        return 1;
    }
    
    std::cout << "Successfully authenticated with Deribit API (Testnet)" << std::endl << std::flush;
    
    int choice;
    bool running = true;
    
    while (running) {
        DisplayMenu();
        std::cin >> choice;
        
        switch (choice) {
            case 1:
                PlaceOrder(client);
                break;
            case 2:
                ModifyOrder(client);
                break;
            case 3:
                CancelOrder(client);
                break;
            case 4:
                GetOrderBook(client);
                break;
            case 5:
                ViewPositions(client);
                break;
            case 6:
                GetOpenOrders(client);
                break;
            case 7:
                SubscribeToWebSocket(client);
                break;
            case 8:
                running = false;
                break;
            default:
                std::cout << "Invalid choice. Please try again." << std::endl;
        }
    }
    
    return 0;
} 