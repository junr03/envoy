#include "common/common/logger.h"

#include <cassert> // use direct system-assert to avoid cyclic dependency.
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

#include "common/common/lock_guard.h"

#include "absl/strings/ascii.h"
#include "absl/strings/escaping.h"
#include "absl/strings/strip.h"
// #include "spdlog/sinks/android_sink.h"
#include "spdlog/spdlog.h"

namespace Envoy {
namespace Logger {

StandardLogger::StandardLogger(const std::string& name)
    : Logger(std::make_shared<spdlog::logger>(name, Registry::getSink())) {}

// AndroidLogger::AndroidLogger(const std::string& name)
//     : logger_(std::make_shared<spdlog::logger>(
//           name, std::make_shared<spdlog::sinks::android_sink<std::mutex>>())),
//       Logger(name) {}

static Context* current_context = nullptr;

Context::Context(spdlog::level::level_enum log_level, const std::string& log_format,
                 Thread::BasicLockable& lock, bool should_escape)
    : log_level_(log_level), log_format_(log_format), lock_(lock), should_escape_(should_escape),
      save_context_(current_context) {
  current_context = this;
  activate();
}

Context::~Context() {
  current_context = save_context_;
  if (current_context != nullptr) {
    current_context->activate();
  } else {
    Registry::getSink()->clearLock();
  }
}

void Context::activate() {
  Registry::getSink()->setLock(lock_);
  Registry::getSink()->set_should_escape(should_escape_);
  Registry::setLogLevel(log_level_);
  Registry::setLogFormat(log_format_);
}

} // namespace Logger
} // namespace Envoy
