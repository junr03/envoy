#pragma once

#include <string>

#include "common/common/base_logger.h"

namespace Envoy {
namespace Logger {

class StandardLogger : public Logger {
private:
  StandardLogger(const std::string& name);

  friend class Registry;
};

} // namespace Logger
} // namespace Envoy