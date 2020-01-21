#pragma once

#include <iostream>
#include <cstdint>
#include <memory>
#include <string>

#include "common/common/base_logger.h"
#include "common/common/non_copyable.h"
#include "common/common/lock_guard.h"

#include "absl/strings/string_view.h"
#include "absl/strings/ascii.h"
#include "absl/strings/escaping.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "absl/synchronization/mutex.h"

#include "spdlog/spdlog.h"

namespace Envoy {
namespace Logger {

class DelegatingLogSink;
using DelegatingLogSinkPtr = std::shared_ptr<DelegatingLogSink>;

/**
 * Captures a logging sink that can be delegated to for a bounded amount of time.
 * On destruction, logging is reverted to its previous state. SinkDelegates must
 * be allocated/freed as a stack.
 */
class SinkDelegate : NonCopyable {
public:
  explicit SinkDelegate(DelegatingLogSinkPtr log_sink);
  virtual ~SinkDelegate();

  virtual void log(absl::string_view msg) PURE;
  virtual void flush() PURE;

protected:
  SinkDelegate* previous_delegate() { return previous_delegate_; }

private:
  SinkDelegate* previous_delegate_;
  DelegatingLogSinkPtr log_sink_;
};

/**
 * SinkDelegate that writes log messages to stderr.
 */
class StderrSinkDelegate : public SinkDelegate {
public:
  explicit StderrSinkDelegate(DelegatingLogSinkPtr log_sink);

  // SinkDelegate
  void log(absl::string_view msg) override;
  void flush() override;

  bool hasLock() const { return lock_ != nullptr; }
  void setLock(Thread::BasicLockable& lock) { lock_ = &lock; }
  void clearLock() { lock_ = nullptr; }
  Thread::BasicLockable* lock() { return lock_; }

private:
  Thread::BasicLockable* lock_{};
};

/**
 * Stacks logging sinks, so you can temporarily override the logging mechanism, restoring
 * the previous state when the DelegatingSink is destructed.
 */
class DelegatingLogSink : public spdlog::sinks::sink {
public:
  void setLock(Thread::BasicLockable& lock) { stderr_sink_->setLock(lock); }
  void clearLock() { stderr_sink_->clearLock(); }

  // spdlog::sinks::sink
  void log(const spdlog::details::log_msg& msg) override;
  void flush() override { sink_->flush(); }
  void set_pattern(const std::string& pattern) override {
    set_formatter(spdlog::details::make_unique<spdlog::pattern_formatter>(pattern));
  }
  void set_formatter(std::unique_ptr<spdlog::formatter> formatter) override;
  void set_should_escape(bool should_escape) { should_escape_ = should_escape; }

  /**
   * @return bool whether a lock has been established.
   */
  bool hasLock() const { return stderr_sink_->hasLock(); }

  /**
   * Constructs a new DelegatingLogSink, sets up the default sink to stderr,
   * and returns a shared_ptr to it.
   *
   * A shared_ptr is required for sinks used
   * in spdlog::logger; it would not otherwise be required in Envoy. This method
   * must own the construction process because StderrSinkDelegate needs access to
   * the DelegatingLogSinkPtr, not just the DelegatingLogSink*, and that is only
   * available after construction.
   */
  static DelegatingLogSinkPtr init();

  /**
   * Give a log line with trailing whitespace, this will escape all c-style
   * escape sequences except for the trailing whitespace.
   * This allows logging escaped messages, but preserves end-of-line characters.
   *
   * @param source the log line with trailing whitespace
   * @return a string with all c-style escape sequences escaped, except trailing whitespace
   */
  static std::string escapeLogLine(absl::string_view source);

private:
  friend class SinkDelegate;

  DelegatingLogSink() = default;

  void setDelegate(SinkDelegate* sink) { sink_ = sink; }
  SinkDelegate* delegate() { return sink_; }

  SinkDelegate* sink_{nullptr};
  std::unique_ptr<StderrSinkDelegate> stderr_sink_; // Builtin sink to use as a last resort.
  std::unique_ptr<spdlog::formatter> formatter_ ABSL_GUARDED_BY(format_mutex_);
  absl::Mutex format_mutex_; // direct absl reference to break build cycle.
  bool should_escape_{false};
};

} // namespace Logger

} // namespace Envoy
