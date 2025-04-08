# Set Windows target version for ASIO
if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0601)
endif()

cmake_minimum_required(VERSION 3.15)
project(deribit_api)

# Set CMake policies
cmake_policy(SET CMP0074 NEW)
cmake_policy(SET CMP0079 NEW)
cmake_policy(SET CMP0167 NEW)  # Use BoostConfig.cmake

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set vcpkg paths
set(CMAKE_PREFIX_PATH "D:/vcpkg/installed/x64-windows")
set(OPENSSL_ROOT_DIR "D:/vcpkg/installed/x64-windows")
set(OPENSSL_INCLUDE_DIR "D:/vcpkg/installed/x64-windows/include")
set(OPENSSL_LIBRARIES "D:/vcpkg/installed/x64-windows/lib")
set(OPENSSL_CRYPTO_LIBRARY "D:/vcpkg/installed/x64-windows/lib/libcrypto.lib")
set(OPENSSL_SSL_LIBRARY "D:/vcpkg/installed/x64-windows/lib/libssl.lib")

# Set CURL paths
set(CURL_ROOT "D:/vcpkg/installed/x64-windows")
set(CURL_INCLUDE_DIR "D:/vcpkg/installed/x64-windows/include")
set(CURL_LIBRARY "D:/vcpkg/installed/x64-windows/lib/libcurl.lib")

# Find required packages
find_package(OpenSSL REQUIRED)
find_package(Boost REQUIRED COMPONENTS system thread random)
find_package(Threads REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(CURL CONFIG REQUIRED)

# Set WebSocket++ path
set(WEBSOCKETPP_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/websocketpp")
set(WEBSOCKETPP_INCLUDE_DIR "${WEBSOCKETPP_ROOT}")

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${OPENSSL_INCLUDE_DIR}
    ${Boost_INCLUDE_DIRS}
    ${CURL_INCLUDE_DIRS}
    ${WEBSOCKETPP_INCLUDE_DIR}
)

# Add source directory
add_subdirectory(src) 