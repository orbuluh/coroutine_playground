#include <iostream>

#include "conceptual_coroutine.hpp"
#include "flow_of_calls.hpp"
#include "multithread_scheduler.hpp"
#include "parse_with_generator.hpp"
#include "singlethread_scheduler.hpp"

int main() {
  // conceptual_coroutine::run();
  // flow_of_calls::run();
  // singlethread_scheduler::run();
  // multithread_scheduler::run();
  parse_with_generator::run();
}