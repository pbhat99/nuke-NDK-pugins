// Copyright (c) 2022 The Foundry Visionmongers Ltd. All Rights Reserved.

#ifndef FDK_BASE_TIMETRACER_H
#define FDK_BASE_TIMETRACER_H

#include "fdk/api.h"

#include <chrono>
#include <string>

#if defined(_MSC_VER)
#define FN_FUNC __FUNCTION__
#else
#define FN_FUNC __func__
#endif

// This can be enabled by cmake by using -DFN_ENABLE_TIME_TRACER=ON (default OFF).
#if defined(FN_ENABLE_TIME_TRACER)

#define mFnTimeTracer(msg) fdk::base::TimeTracer timeTracer(msg)
#define mFnTimeTracedFunction() mFnTimeTracer(FN_FUNC)
#define mFnTimeTracerFunctionMessage(msg) \
  std::string message = FN_FUNC; \
  message += " ("; \
  message += msg; \
  message += ")"; \
  mFnTimeTracer(message.c_str());
#define mFnTimeTracerReport(msg) timeTracer.report(msg)

#else
#define mFnTimeTracer(msg) do { } while(0)
#define mFnTimeTracedFunction() do { } while(0)
#define mFnTimeTracerFunctionMessage(msg) do { } while(0)
#define mFnTimeTracerReport(msg) do { } while(0)
#endif

namespace fdk {
  namespace base {
    class FDK_API TimeTracer {
    public:
      TimeTracer(const char* message = "Elapsed time",
                 bool reportOnOutOfScope = true);
      ~TimeTracer();

      void start();
      void stop();

      // Reports the time elapsed since the start.
      // The message argument can be used to add further details when capturing
      // the time at multiple places in a scope to localise the
      // instrumentation.
      // The method currently prints to the standard output.
      void report(const std::string& message = std::string());

      double elapsed() const;

    private:
      const char* _message;
      std::chrono::steady_clock::time_point _start;
      std::chrono::steady_clock::time_point _end;
      bool _reportOnOutOfScope;
    };
  }
}

#endif
