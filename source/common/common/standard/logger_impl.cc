#include "common/common/logger_impl.h"

#include <memory>

namespace Envoy {
namespace Logger {

#define GENERATE_LOGGER(X) StandardLogger(#X),

StandardLogger::StandardLogger(const std::string& name)
    : Logger(std::make_shared<spdlog::logger>(name, Registry::getSink())) {}

} // namespace Logger
} // namespace Envoy