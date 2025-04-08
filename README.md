# Deribit API Client

This project is a C++ client for interacting with the Deribit API, supporting both REST and WebSocket communications.

## Prerequisites

- **CMake**: Version 3.15 or higher
- **vcpkg**: For managing dependencies
- **OpenSSL**: For secure communications
- **Boost**: For system, thread, and random components
- **CURL**: For HTTP requests
- **nlohmann_json**: For JSON handling
- **WebSocket++**: For WebSocket communications

## Installing Dependencies

1. **Install vcpkg**: Follow the instructions at [vcpkg GitHub](https://github.com/microsoft/vcpkg) to install vcpkg.

2. **Install required packages** using vcpkg:
   ```bash
   vcpkg install openssl boost-system boost-thread boost-random curl nlohmann-json websocketpp
   ```

3. **Set vcpkg environment variables**:
   Ensure that the vcpkg toolchain file is used by setting the `CMAKE_TOOLCHAIN_FILE` environment variable:
   ```bash
   set CMAKE_TOOLCHAIN_FILE=D:/vcpkg/scripts/buildsystems/vcpkg.cmake
   ```

## Building the Project

1. **Clone the repository**:
   ```bash
   git clone <repository-url>
   cd <repository-directory>
   ```

2. **Build the project**:
   Run the provided batch script to build the project:
   ```bash
   build.bat
   ```

## Running the Project

1. **Copy Tokens**:
   Ensure that your access and refresh tokens are copied to the `build/release` directory. These tokens are necessary for authenticating with the Deribit API.

2. **Run the executable**:
   Navigate to the `build/release` directory and run the executable:
   ```bash
   cd build/release
   ./deribit_api
   ```

## Configuration

- **API Keys**: Set your API key and secret in the `Config` class.
- **Testnet/Mainnet**: Configure the `Config` class to use testnet or mainnet as needed.

## Notes

- Ensure that all dependencies are correctly installed and accessible via vcpkg.
- The project uses C++17, so ensure your compiler supports this standard.
- Adjust the contract size in the `ApiClient` class as needed for your specific trading instrument.

For further assistance, refer to the Deribit API documentation or contact support. 