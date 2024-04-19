#pragma once

#include <cstdio>

namespace conceptual_coroutine {

// From: https://www.geocities.ws/rand3289/MultiTasking.html
// https://www.reddit.com/r/cpp/comments/18njuow/comment/kebmfw8/?utm_source=share&utm_medium=web3x&utm_name=web3xcss&utm_term=1&utm_content=share_button

#define TASK_INIT() \
  static void* f;   \
  if (f) goto* f;

// The ability for the task to recover its execution state after returning from
// the function is based on the fact that the function's body resides in the
// same memory location across multiple calls.

// When a function is defined in C, its instructions(i.e., the function body)
// are typically stored in a fixed memory location determined by the compiler.As
// a result, when the function is called multiple times, each invocation of the
// function executes the same set of instructions located at the same memory
// address.

// In the case of the TASK_YIELD() macro, the key mechanism enabling the task to
// recover its execution state is the use of function pointers(f) and
// labels(END).When the task yields, it saves the address of the END label in
// the f variable.Later, when the task is resumed, it jumps to the END label
// using the goto statement, effectively continuing execution from that point
// onward.

// Since the function body remains in the same memory location across
// multiple calls, the saved address(f) remains valid even after the
// function returns.This allows the task to resume execution from the point
// where it yielded, achieving the appearance of suspending and resuming
// execution within the same function.

// In C, labels must have a unique name within their scope. Each label serves as
// a target for goto statements, and the compiler ensures that there are no
// duplicate labels within the same scope. In the provided example, END is used
// as a label within the TASK_YIELD() macro. However, since each function
// defines its own scope, the label END within TASK_YIELD() is local to that
// macro and does not conflict with the END label in other functions such as
// testingSub2(). Therefore, there is no redefinition or conflict of labels
// between different functions, and each label is effectively scoped to its
// respective function.

#define TASK_YIELD() \
  {                  \
    __label__ END;   \
    f = &&END;       \
    return;          \
  END:;              \
  }

// Reset the function pointer when goes to end (so the output loops back from
// beginning of code)
#define TASK_END() f = 0;

void testingSub1() {
  TASK_INIT();
  printf("A");
  TASK_YIELD();
  printf("B");
  TASK_YIELD();
  printf("C");
  TASK_END();
}

void testingSub2() {
  TASK_INIT();
  printf("1");
  TASK_YIELD();
  printf("2");
  TASK_YIELD();
  printf("3");
  TASK_YIELD();
  printf("4");
  TASK_END();
}

void testingSub3() {
  TASK_INIT();
  while (1) {
    printf("*");
    TASK_YIELD();
  }
  TASK_END();
}

void testing() {
  testingSub1();
  testingSub2();
  testingSub3();
  printf("|");
}

void run() {
  for (int i = 0; i < 10; ++i) {
    testing();
  }
}

}  // namespace conceptual_coroutine