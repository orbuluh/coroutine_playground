#pragma once
#include <atomic>
#include <condition_variable>
#include <coroutine>
#include <list>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include "scheduler_task.hpp"
// modified from
// https://github.com/dian-lun-lin/cpp_coroutine_examples/blob/main/multi-threaded/scheduler.hpp

namespace multithread_scheduler {

class Scheduler {
 public:
  Scheduler(size_t num_threads = std::thread::hardware_concurrency());

  void emplace(std::coroutine_handle<> task);
  auto suspend();
  void schedule();
  void wait();

 private:
  void _enqueue(std::coroutine_handle<> task);
  void _process(std::coroutine_handle<> task);

  std::vector<std::coroutine_handle<>> _tasks;
  std::queue<std::coroutine_handle<>> _pending_tasks;
  std::vector<std::thread> _workers;

  std::mutex _mtx;
  std::condition_variable _cv;
  bool _stop{false};
  std::atomic<size_t> _finished{0};
};

Scheduler::Scheduler(size_t num_threads) {
  _workers.reserve(num_threads);

  for (size_t t = 0; t < num_threads; ++t) {
    _workers.emplace_back([this]() {
      while (true) {
        std::coroutine_handle<> task;
        {
          std::unique_lock<std::mutex> lock(_mtx);
          _cv.wait(lock, [this] { return _stop || (!_pending_tasks.empty()); });
          if (_stop) {
            return;
          }

          task = _pending_tasks.front();
          _pending_tasks.pop();
        }
        if (task) {
          _process(task);
        }
      }
    });
  }
}

void Scheduler::emplace(std::coroutine_handle<> task) {
  _tasks.emplace_back(task);
}

void Scheduler::schedule() {
  for (auto task : _tasks) {
    _enqueue(task);
  }
}

auto Scheduler::suspend() { return std::suspend_always{}; }

void Scheduler::wait() {
  for (auto& w : _workers) {
    w.join();
  }
}

void Scheduler::_process(std::coroutine_handle<> task) {
  task.resume();

  if (!task.done()) {
    _enqueue(task);
  } else {
    task.destroy();
    if (_finished.fetch_add(1) + 1 == _tasks.size()) {
      {
        std::unique_lock<std::mutex> lock(_mtx);
        _stop = true;
      }
      _cv.notify_all();
    }
  }
}

void Scheduler::_enqueue(std::coroutine_handle<> task) {
  {
    std::unique_lock<std::mutex> lock(_mtx);
    _pending_tasks.push(task);
  }
  _cv.notify_one();
}

void run() {
  Scheduler sch;

  using scheduler_task::TaskCoroutine;
  sch.emplace(TaskCoroutine<Scheduler>(sch, "A").get_handle());
  sch.emplace(TaskCoroutine<Scheduler>(sch, "B").get_handle());

  util::tid_cout() << "Start multithread scheduling...\n";

  sch.schedule();
  sch.wait();
}

}  // namespace multithread_scheduler