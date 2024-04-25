#pragma once
#include <coroutine>
#include <queue>
#include <stack>

#include "scheduler_task.hpp"
// Modified from
// https://github.com/dian-lun-lin/cpp_coroutine_examples/blob/main/single-threaded/scheduler.hpp

namespace singlethread_scheduler {

class Scheduler {
  // try either queue or stack to see what happened
  std::queue<std::coroutine_handle<>> _tasks;
  // std::stack<std::coroutine_handle<>> _tasks;

 public:
  void emplace(std::coroutine_handle<> task) { _tasks.push(task); }

  void schedule() {
    while (!_tasks.empty()) {
      auto task = _tasks.front();
      // auto task = _tasks.top();
      _tasks.pop();
      task.resume();

      if (!task.done()) {
        _tasks.push(task);
      } else {
        task.destroy();
      }
    }
  }

  auto suspend() { return std::suspend_always{}; }
};

void run() {
  using scheduler_task::TaskCoroutine;
  Scheduler sch;

  sch.emplace(TaskCoroutine<Scheduler>(sch, "A").get_handle());
  sch.emplace(TaskCoroutine<Scheduler>(sch, "B").get_handle());

  util::tid_cout() << "Start single thread scheduling...\n";

  sch.schedule();
}

}  // namespace singlethread_scheduler