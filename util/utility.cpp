#include "utility.hpp"

#include <atomic>
#include <mutex>
#include <thread>
#include <unordered_map>

namespace {
std::mutex cout_lock;
}

namespace util {

std::ostream& tid_cout() {
  static std::unordered_map<std::thread::id, int> seen;
  static int idx = 0;
  auto get_tid_desc = [&]() {
    const auto tid = std::this_thread::get_id();
    if (seen.find(tid) == seen.end()) {
      seen[tid] = idx++;
    }
    return seen[tid];
  };
  std::lock_guard<std::mutex> _{cout_lock};
  return std::cout << "[thread_" << get_tid_desc() << "]: ";
}
}  // namespace util