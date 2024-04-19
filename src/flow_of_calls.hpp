#include <coroutine>
#include <iostream>
#include <thread>

// Modified from https://zhuanlan.zhihu.com/p/497224333

namespace flow_of_calls {

struct RetWrapper {
  struct promise_type {
    promise_type() { std::cout << "1.  RetWrapper::promise::ctor\n"; }
    ~promise_type() { std::cout << "17. RetWrapper::promise::dtor\n"; }

    RetWrapper get_return_object() {
      std::cout
          << "2.  RetWrapper::promise::get_return_object: create coroutine "
             "return object, and the coroutine is created now\n";
      return {
          std::coroutine_handle<RetWrapper::promise_type>::from_promise(*this)};
    }

    std::suspend_never initial_suspend() {
      std::cout
          << "3.  RetWrapper::promise::initial_suspend: <suspend the current "
             "coroutine?>\n";
      std::cout << "4.  RetWrapper::promise::initial_suspend: don't suspend "
                   "because return std::suspend_never, so continue to execute "
                   "coroutine body\n";
      return {};
    }

    std::suspend_never final_suspend() noexcept {
      std::cout
          << "15. RetWrapper::promise::final_suspend: coroutine body "
             "finished <do you want to susupend the current coroutine?>\n";
      std::cout << "16. RetWrapper::promise::final_suspend: don't suspend "
                   "because return std::suspend_never, and the continue will "
                   "be automatically destroyed\n";
      return {};
    }
    void return_void() {
      std::cout << "14. RetWrapper::promise::return_void: coroutine don't "
                   "return value, so return_void is called\n";
    }
    void unhandled_exception() {}
  };
  using HandleToCoroutine = std::coroutine_handle<RetWrapper::promise_type>;
  HandleToCoroutine handle_;
};

struct Awaiter {
  bool await_ready() {
    std::cout << "6.  Awaiter::await_ready: <do you want to suspend current "
                 "coroutine?>\n";
    std::cout << "7.  Awaiter::await_ready: yes, suspend because "
                 "Awaiter.await_ready() return false\n";
    return false;
  }
  void await_suspend(RetWrapper::HandleToCoroutine handle) {
    std::cout
        << "8.  Awaiter::await_suspend<RetWrapper::HandleToCoroutine> called\n";
    std::thread([handle]() mutable {
      // handle();
      std::cout << "11. handle.resume() called on tid="
                << std::this_thread::get_id() << "\n";
      handle.resume();
    }).detach();
    std::cout << "9.  Awaiter::await_suspend<RetWrapper::HandleToCoroutine>: "
                 "new thread launched, and will return back to caller\n";
  }
  void await_resume() {
    std::cout << "12. Awaiter::await_resume() on tid="
              << std::this_thread::get_id() << "\n";
  }
};

RetWrapper CallACoroutine() {
  std::cout << "5.  ---Coroutine body---[tid=" << std::this_thread::get_id()
            << "] Begin to execute coroutine body\n";
  co_await Awaiter{};
  std::cout << "13. ---Coroutine body---[tid=" << std::this_thread::get_id()
            << "] Resume back to coroutine body, from "
               "Awaiter's suspend.\n";
}

void run() {
  std::cout << "============================\n";
  CallACoroutine();
  std::cout << "============================\n";
  std::cout << "10. Caller of coroutine: come back to caller because of "
               "co_await Awaiter. Going to sleep 1s on tid="
            << std::this_thread::get_id() << "\n";
  std::this_thread::sleep_for(std::chrono::seconds(1));
  std::cout << "---end---\n";
}

}  // namespace flow_of_calls