#pragma once

#include <coroutine>

#include "util/utility.hpp"

namespace scheduler_task {

struct Task {
  struct promise_type {
    std::suspend_always initial_suspend() noexcept;
    std::suspend_always final_suspend() noexcept;
    Task get_return_object();
    void return_void();
    void unhandled_exception();
  };

  Task(std::coroutine_handle<promise_type> handle);
  ~Task();

  std::coroutine_handle<promise_type> get_handle();

  std::coroutine_handle<promise_type> handle;
};

template <typename Scheduler>
Task TaskCoroutine(Scheduler& sch, std::string_view name) {
  // Note:: inital_suspend is injected even before first line
  // Not just before co_await
  util::tid_cout() << "First line of " << name << '\n';
  co_await sch.suspend();
  util::tid_cout() << "Resume from first suspend for " << name << '\n';
  co_await sch.suspend();
  util::tid_cout() << "Resume from second suspend and finish for " << name
                   << '\n';
}

}  // namespace scheduler_task