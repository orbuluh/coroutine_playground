#include <coroutine>
#include <iostream>

// from https://stackoverflow.com/a/70803344/4924135 by Zartaj Majeed

// Coroutines come to mind any time there's a function that needs to maintain
// state across calls. Before C++ coroutines we could use functors or capturing
// lambdas to solve such problems. What coroutines bring to the table is the
// ability to not just maintain the state of local data but also the state of
// local logic.

// So coroutine code can be simpler and have a nicer flow than a normal function
// that examines the state at its entry with every call.

// Beyond keeping state locally, coroutines let a function offer customization
// points or hooks similar to a lambda passed in as a callback.

// Using lambdas as callbacks is an example of inversion of control. A
// coroutine-based design however is the opposite of inversion of control - a
// "reversion of control", if you will, back to client code that decides when to
// co_await and what to do with the result.

namespace parse_with_generator {

struct ReplaceDashes {
  struct Promise;
  using promise_type = Promise;
  std::coroutine_handle<Promise> coro;

  ReplaceDashes(std::coroutine_handle<Promise> h) : coro(h) {}

  ~ReplaceDashes() {
    if (coro) coro.destroy();
  }

  // resume the suspended coroutine
  bool next() {
    coro.resume();
    return !coro.done();
  }

  // return the value yielded by coroutine
  std::string value() const { return coro.promise().output; }

  // set the input std::string and run coroutine
  ReplaceDashes& operator()(std::string* input) {
    *coro.promise().input = input;
    coro.resume();
    return *this;
  }

  struct Promise {
    // address of a pointer to the input std::string
    std::string** input;
    // the transformed output aka yielded value of the coroutine
    std::string output;

    ReplaceDashes get_return_object() {
      return ReplaceDashes{std::coroutine_handle<Promise>::from_promise(*this)};
    }

    // run coroutine immediately to first co_await
    std::suspend_never initial_suspend() noexcept { return {}; }

    // set yielded value to return
    std::suspend_always yield_value(std::string value) {
      output = value;
      return {};
    }
    // set returned value to return
    void return_value(std::string value) { output = value; }

    std::suspend_always final_suspend() noexcept { return {}; }

    void unhandled_exception() noexcept {}

    // intercept co_await on the address of the local variable in
    // the coroutine that points to the input std::string
    std::suspend_always await_transform(std::string** localInput) {
      input = localInput;
      return {};
    }
  };
};

ReplaceDashes replaceDashes() {
  std::string dashes;
  std::string outstr;

  // input is a pointer to a string instead of a string
  // this way input string can be changed cheaply
  std::string* input{};

  // pass a reference to local input string to keep in coroutine promise
  // this way input string can be set from outside coroutine
  co_await &input;

  for (unsigned i = 0;;) {
    char chr = (*input)[i++];
    // string is consumed, return the transformed string
    // or any leftover dashes if this was the final input
    if (chr == '\0') {
      if (i == 1) {
        co_return dashes;
      }
      co_yield outstr;
      // resume to process new input string
      i = 0;
      outstr.clear();
      continue;
    }
    // append non-dash after any accumulated dashes
    if (chr != '-') {
      outstr += dashes;
      outstr += chr;
      dashes.clear();
      continue;
    }
    // accumulate dashes
    if (dashes.length() < 2) {
      dashes += chr;
      continue;
    }
    // replace 3 dashes in a row
    // unicode em dash u+2014 '—' is utf8 e2 80 94
    outstr += "\xe2\x80\x94";
    dashes.clear();
  }
}

struct Charsub {
  ReplaceDashes replacer = replaceDashes();

  std::string parse(std::string& input) { return replacer(&input).value(); }

  std::string flush() {
    replacer.next();
    return replacer.value();
  }
};

void run() {
  Charsub charsub;

  for (std::string line; std::getline(std::cin, line);) {
    if (line.empty()) {
      std::cout << "Input something with dash\n";
      continue;
    }
    std::cout << charsub.parse(line) << "\n";
  }
  std::cout << charsub.flush();
}
}  // namespace parse_with_generator