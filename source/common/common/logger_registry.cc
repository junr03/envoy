#include "common/common/logger_registry.h"

namespace Envoy {
namespace Logger {

std::vector<Logger>& Registry::allLoggers() {
  static std::vector<Logger>* all_loggers =
      new std::vector<Logger>({ALL_LOGGER_IDS(GENERATE_LOGGER)});
  return *all_loggers;
}

spdlog::logger& Registry::getLog(Id id) { return allLoggers()[static_cast<int>(id)].logger(); }

void Registry::setLogLevel(spdlog::level::level_enum log_level) {
  for (Logger& logger : allLoggers()) {
    logger.setLevel(static_cast<spdlog::level::level_enum>(log_level));
  }
}

void Registry::setLogFormat(const std::string& log_format) {
  for (Logger& logger : allLoggers()) {
    logger.setPattern(log_format);
  }
}

// TODO(junr03): make this an optional reference wrapper.
Logger* Registry::logger(const std::string& log_name) {
  Logger* logger_to_return = nullptr;
  for (Logger& logger : loggers()) {
    if (logger.name() == log_name) {
      logger_to_return = &logger;
      break;
    }
  }
  return logger_to_return;
}

} // namespace Logger
} // namespace Envoy