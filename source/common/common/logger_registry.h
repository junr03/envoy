#pragma once

#include <string>
#include <vector>

#include "common/common/base_logger.h"
#include "common/common/essential_logger_delegates.h"
#include "common/common/macros.h"

#include "spdlog/spdlog.h"

namespace Envoy {
namespace Logger {

// TODO: find out a way for extensions to register new logger IDs
#define ALL_LOGGER_IDS(FUNCTION)                                                                   \
  FUNCTION(admin)                                                                                  \
  FUNCTION(aws)                                                                                    \
  FUNCTION(assert)                                                                                 \
  FUNCTION(backtrace)                                                                              \
  FUNCTION(client)                                                                                 \
  FUNCTION(config)                                                                                 \
  FUNCTION(connection)                                                                             \
  FUNCTION(conn_handler)                                                                           \
  FUNCTION(decompression)                                                                          \
  FUNCTION(dubbo)                                                                                  \
  FUNCTION(file)                                                                                   \
  FUNCTION(filter)                                                                                 \
  FUNCTION(forward_proxy)                                                                          \
  FUNCTION(grpc)                                                                                   \
  FUNCTION(hc)                                                                                     \
  FUNCTION(health_checker)                                                                         \
  FUNCTION(http)                                                                                   \
  FUNCTION(http2)                                                                                  \
  FUNCTION(hystrix)                                                                                \
  FUNCTION(init)                                                                                   \
  FUNCTION(io)                                                                                     \
  FUNCTION(jwt)                                                                                    \
  FUNCTION(kafka)                                                                                  \
  FUNCTION(lua)                                                                                    \
  FUNCTION(main)                                                                                   \
  FUNCTION(misc)                                                                                   \
  FUNCTION(mongo)                                                                                  \
  FUNCTION(quic)                                                                                   \
  FUNCTION(quic_stream)                                                                            \
  FUNCTION(pool)                                                                                   \
  FUNCTION(rbac)                                                                                   \
  FUNCTION(redis)                                                                                  \
  FUNCTION(router)                                                                                 \
  FUNCTION(runtime)                                                                                \
  FUNCTION(stats)                                                                                  \
  FUNCTION(secret)                                                                                 \
  FUNCTION(tap)                                                                                    \
  FUNCTION(testing)                                                                                \
  FUNCTION(thrift)                                                                                 \
  FUNCTION(tracing)                                                                                \
  FUNCTION(upstream)                                                                               \
  FUNCTION(udp)                                                                                    \
  FUNCTION(wasm)

// clang-format off
enum class Id {
  ALL_LOGGER_IDS(GENERATE_ENUM)
};
// clang-format on

/**
 * A registry of all named loggers in envoy. Usable for adjusting levels of each logger
 * individually.
 */
class Registry {
public:
  /**
   * @param id supplies the fixed ID of the logger to create.
   * @return spdlog::logger& a logger with system specified sinks for a given ID.
   */
  static spdlog::logger& getLog(Id id);

  /**
   * @return the singleton sink to use for all loggers.
   */
  static DelegatingLogSinkPtr getSink() {
    static DelegatingLogSinkPtr sink = DelegatingLogSink::init();
    return sink;
  }

  /**
   * Sets the minimum log severity required to print messages.
   * Messages below this loglevel will be suppressed.
   */
  static void setLogLevel(spdlog::level::level_enum log_level);

  /**
   * Sets the log format.
   */
  static void setLogFormat(const std::string& log_format);

  /**
   * @return std::vector<Logger>& the installed loggers.
   */
  static std::vector<Logger>& loggers() { return allLoggers(); }

  /**
   * @Return bool whether the registry has been initialized.
   */
  static bool initialized() { return getSink()->hasLock(); }

  static Logger* logger(const std::string& log_name);

private:
  /*
   * @return std::vector<Logger>& return the installed loggers.
   */
  static std::vector<Logger>& allLoggers();
};

} // namespace Logger
} // namespace Envoy