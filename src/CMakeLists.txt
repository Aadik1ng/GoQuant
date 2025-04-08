# Add source files
set(SOURCES
    main.cpp
    deribit/api_client.cpp
    deribit/config.cpp
    deribit/orderbook.cpp
    deribit/position.cpp
    deribit/order.cpp
    deribit/rest_client.cpp
    deribit/websocket_client.cpp
)

# Create executable
add_executable(deribit_api ${SOURCES})

# Add include directories
target_include_directories(deribit_api
    PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${Boost_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIR}
    ${CURL_INCLUDE_DIRS}
    ${WEBSOCKETPP_INCLUDE_DIR}
)

# Link libraries
target_link_libraries(deribit_api
    PRIVATE
    OpenSSL::SSL
    OpenSSL::Crypto
    Boost::system
    Boost::thread
    Boost::random
    Threads::Threads
    nlohmann_json::nlohmann_json
    CURL::libcurl
) 