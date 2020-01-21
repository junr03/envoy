#include "common/common/essential_logger_delegates.h"

namespace Envoy {
namespace Logger {

SinkDelegate::SinkDelegate(DelegatingLogSinkPtr log_sink)
    : previous_delegate_(log_sink->delegate()), log_sink_(log_sink) {
  log_sink->setDelegate(this);
}

SinkDelegate::~SinkDelegate() {
  assert(log_sink_->delegate() == this); // Ensures stacked allocation of delegates.
  log_sink_->setDelegate(previous_delegate_);
}

StderrSinkDelegate::StderrSinkDelegate(DelegatingLogSinkPtr log_sink) : SinkDelegate(log_sink) {}

void StderrSinkDelegate::log(absl::string_view msg) {
  Thread::OptionalLockGuard guard(lock_);
  std::cerr << msg;
}

void StderrSinkDelegate::flush() {
  Thread::OptionalLockGuard guard(lock_);
  std::cerr << std::flush;
}

void DelegatingLogSink::set_formatter(std::unique_ptr<spdlog::formatter> formatter) {
  absl::MutexLock lock(&format_mutex_);
  formatter_ = std::move(formatter);
}

void DelegatingLogSink::log(const spdlog::details::log_msg& msg) {
  absl::ReleasableMutexLock lock(&format_mutex_);
  absl::string_view msg_view = absl::string_view(msg.payload.data(), msg.payload.size());

  // This memory buffer must exist in the scope of the entire function,
  // otherwise the string_view will refer to memory that is already free.
  fmt::memory_buffer formatted;
  if (formatter_) {
    formatter_->format(msg, formatted);
    msg_view = absl::string_view(formatted.data(), formatted.size());
  }
  lock.Release();

  if (should_escape_) {
    sink_->log(escapeLogLine(msg_view));
  } else {
    sink_->log(msg_view);
  }
}

std::string DelegatingLogSink::escapeLogLine(absl::string_view msg_view) {
  // Split the actual message from the trailing whitespace.
  auto eol_it = std::find_if_not(msg_view.rbegin(), msg_view.rend(), absl::ascii_isspace);
  absl::string_view msg_leading = msg_view.substr(0, msg_view.rend() - eol_it);
  absl::string_view msg_trailing_whitespace =
      msg_view.substr(msg_view.rend() - eol_it, eol_it - msg_view.rbegin());

  // Escape the message, but keep the whitespace unescaped.
  return absl::StrCat(absl::CEscape(msg_leading), msg_trailing_whitespace);
}

DelegatingLogSinkPtr DelegatingLogSink::init() {
  DelegatingLogSinkPtr delegating_sink(new DelegatingLogSink);
  delegating_sink->stderr_sink_ = std::make_unique<StderrSinkDelegate>(delegating_sink);
  return delegating_sink;
}

} // namespace Logger
} // namespace Envoy
