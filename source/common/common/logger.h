#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "envoy/thread/thread.h"

#include "common/common/fmt.h"
#include "common/common/macros.h"
#include "common/common/non_copyable.h"
#include "common/common/base_logger.h"

#include "absl/strings/string_view.h"
#include "absl/synchronization/mutex.h"
#include "fmt/ostream.h"
#include "spdlog/spdlog.h"

namespace Envoy {
namespace Logger {

// class AndroidLogger : public Logger {
// private:
//   AndroidLogger(const std::string& name);
// };

/**
 * Defines a scope for the logging system with the specified lock and log level.
 * This is equivalent to setLogLevel, setLogFormat, and setLock, which can be
 * called individually as well, e.g. to set the log level without changing the
 * lock or format.
 *
 * Contexts can be nested. When a nested context is destroyed, the previous
 * context is restored. When all contexts are destroyed, the lock is cleared,
 * and logging will remain unlocked, the same state it is in prior to
 * instantiating a Context.
 */
class Context {
public:
  Context(spdlog::level::level_enum log_level, const std::string& log_format,
          Thread::BasicLockable& lock, bool should_escape);
  ~Context();

private:
  void activate();

  const spdlog::level::level_enum log_level_;
  const std::string log_format_;
  Thread::BasicLockable& lock_;
  bool should_escape_;
  Context* const save_context_;
};

/**
 * Mixin class that allows any class to perform logging with a logger of a particular ID.
 */
template <Id id> class Loggable {
protected:
  /**
   * Do not use this directly, use macros defined below.
   * @return spdlog::logger& the static log instance to use for class local logging.
   */
  static spdlog::logger& __log_do_not_use_read_comment() {
    static spdlog::logger& instance = Registry::getLog(id);
    return instance;
  }
};

} // namespace Logger

// Convert the line macro to a string literal for concatenation in log macros.
#define DO_STRINGIZE(x) STRINGIZE(x)
#define STRINGIZE(x) #x
#define LINE_STRING DO_STRINGIZE(__LINE__)
#define LOG_PREFIX "[" __FILE__ ":" LINE_STRING "] "

/**
 * Base logging macros. It is expected that users will use the convenience macros below rather than
 * invoke these directly.
 */

#define ENVOY_LOG_COMP_LEVEL(LOGGER, LEVEL)                                                        \
  (static_cast<spdlog::level::level_enum>(Envoy::Logger::Logger::LEVEL) >= LOGGER.level())

// Compare levels before invoking logger. This is an optimization to avoid
// executing expressions computing log contents when they would be suppressed.
// The same filtering will also occur in spdlog::logger.
#define ENVOY_LOG_COMP_AND_LOG(LOGGER, LEVEL, ...)                                                 \
  do {                                                                                             \
    if (ENVOY_LOG_COMP_LEVEL(LOGGER, LEVEL)) {                                                     \
      LOGGER.LEVEL(LOG_PREFIX __VA_ARGS__);                                                        \
    }                                                                                              \
  } while (0)

#define ENVOY_LOG_CHECK_LEVEL(LEVEL) ENVOY_LOG_COMP_LEVEL(ENVOY_LOGGER(), LEVEL)

/**
 * Convenience macro to log to a user-specified logger.
 * Maps directly to ENVOY_LOG_COMP_AND_LOG - it could contain macro logic itself, without
 * redirection, but left in case various implementations are required in the future (based on log
 * level for example).
 */
#define ENVOY_LOG_TO_LOGGER(LOGGER, LEVEL, ...) ENVOY_LOG_COMP_AND_LOG(LOGGER, LEVEL, ##__VA_ARGS__)

/**
 * Convenience macro to get logger.
 */
#define ENVOY_LOGGER() __log_do_not_use_read_comment()

/**
 * Convenience macro to flush logger.
 */
#define ENVOY_FLUSH_LOG() ENVOY_LOGGER().flush()

/**
 * Convenience macro to log to the class' logger.
 */
#define ENVOY_LOG(LEVEL, ...) ENVOY_LOG_TO_LOGGER(ENVOY_LOGGER(), LEVEL, ##__VA_ARGS__)

/**
 * Convenience macro to log to the misc logger, which allows for logging without of direct access to
 * a logger.
 */
#define GET_MISC_LOGGER() Logger::Registry::getLog(Logger::Id::misc)
#define ENVOY_LOG_MISC(LEVEL, ...) ENVOY_LOG_TO_LOGGER(GET_MISC_LOGGER(), LEVEL, ##__VA_ARGS__)

/**
 * Convenience macros for logging with connection ID.
 */
#define ENVOY_CONN_LOG_TO_LOGGER(LOGGER, LEVEL, FORMAT, CONNECTION, ...)                           \
  ENVOY_LOG_TO_LOGGER(LOGGER, LEVEL, "[C{}] " FORMAT, (CONNECTION).id(), ##__VA_ARGS__)

#define ENVOY_CONN_LOG(LEVEL, FORMAT, CONNECTION, ...)                                             \
  ENVOY_CONN_LOG_TO_LOGGER(ENVOY_LOGGER(), LEVEL, FORMAT, CONNECTION, ##__VA_ARGS__)

/**
 * Convenience macros for logging with a stream ID and a connection ID.
 */
#define ENVOY_STREAM_LOG_TO_LOGGER(LOGGER, LEVEL, FORMAT, STREAM, ...)                             \
  ENVOY_LOG_TO_LOGGER(LOGGER, LEVEL, "[C{}][S{}] " FORMAT,                                         \
                      (STREAM).connection() ? (STREAM).connection()->id() : 0,                     \
                      (STREAM).streamId(), ##__VA_ARGS__)

#define ENVOY_STREAM_LOG(LEVEL, FORMAT, STREAM, ...)                                               \
  ENVOY_STREAM_LOG_TO_LOGGER(ENVOY_LOGGER(), LEVEL, FORMAT, STREAM, ##__VA_ARGS__)

// TODO(danielhochman): macros(s)/function(s) for logging structures that support iteration.

} // namespace Envoy
