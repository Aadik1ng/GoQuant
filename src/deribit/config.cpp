#include "deribit/config.hpp"

namespace deribit {

Config::Config(
    const std::string& api_key,
    const std::string& api_secret,
    bool testnet)
    : api_key_(api_key)
    , api_secret_(api_secret)
    , testnet_(testnet) {
}

} // namespace deribit 