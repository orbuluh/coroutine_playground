#include "scheduler_task.hpp"

namespace scheduler_task {

std::suspend_always Task::promise_type::initial_suspend() noexcept {
  util::tid_cout() << "initial_suspend\n";
  return {};
}
std::suspend_always Task::promise_type::final_suspend() noexcept {
  util::tid_cout() << "final_suspend\n";
  return {};
}

Task Task::promise_type::get_return_object() {
  util::tid_cout() << "get_return_object\n";
  return std::coroutine_handle<promise_type>::from_promise(*this);
}

void Task::promise_type::return_void() {}
void Task::promise_type::unhandled_exception() {}

Task::Task(std::coroutine_handle<promise_type> handle) : handle{handle} {}
Task::~Task() { util::tid_cout() << "~Task()\n"; }
std::coroutine_handle<Task::promise_type> Task::get_handle() { return handle; }

}  // namespace scheduler_task