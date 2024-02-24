#include "nth/utility/early_exit.h"

#include <string>
#include <variant>

#include "nth/test/test.h"
#include "nth/utility/early_exit_test_cases.h"

namespace nth {
namespace {

struct Error {
  using promise_type = early_exit_promise_type<Error>;

  explicit Error(int n) : value_(n) {}
  ~Error();
  explicit Error(early_exit_constructor_t, Error*& v) { v = this; };
  friend bool operator==(Error, Error) = default;
  explicit operator bool() const { return value_ >= 0; }

  int value() const { return value_; }

 private:
  int value_;
};

template <typename T>
struct ErrorOr {
  using promise_type = early_exit_promise_type<ErrorOr>;

  explicit ErrorOr(Error e) : value_(e) {}
  explicit ErrorOr(T t) : value_(t) {}
  ~ErrorOr();
  explicit ErrorOr(early_exit_constructor_t, ErrorOr*& v) { v = this; };

  explicit operator Error const&() const { return std::get<Error>(value_); }

  friend bool operator==(ErrorOr const&, ErrorOr const&) = default;
  explicit operator bool() const { return std::holds_alternative<T>(value_); }
  T const& operator*() const { return std::get<T>(value_); }

 private:
  std::variant<Error, T> value_ = Error(0);
};

// TODO: For some reason ASAN complains if `Error` is marked as
// default-destructible inside the class body, but not if it is marked so out of
// line. Investigate if this is a real memory issue or a miscompile with Clang.
Error::~Error()     = default;
template <typename T>
ErrorOr<T>::~ErrorOr() = default;

static_assert(supports_early_exit<Error>);
static_assert(supports_early_exit<ErrorOr<int>>);
static_assert(supports_early_exit<ErrorOr<std::string>>);
static_assert(early_exit_type<Error> == nth::type<void>);
static_assert(early_exit_type<ErrorOr<int>> == nth::type<int>);
static_assert(early_exit_type<ErrorOr<std::string>> == nth::type<std::string>);

NTH_INVOKE_TEST("nth/utility/early-exit") {
  co_yield nth::type<Error>;
  co_yield nth::type<ErrorOr<int>>;
  co_yield nth::type<ErrorOr<std::string>>;
}

NTH_TEST("early_exit/return-void/continue") {
  int count = 0;

  Error v = [&]() -> Error {
    ++count;
    co_await Error(5);
    ++count;
    co_return Error(3);
  }();

  NTH_EXPECT(v == Error(3));
  NTH_EXPECT(count == 2);
}

NTH_TEST("early_exit/return-void/exit") {
  int count = 0;

  Error v = [&]() -> Error {
    ++count;
    co_await Error(-3);
    ++count;
    co_return Error(3);
  }();

  NTH_EXPECT(v == Error(-3));
  NTH_EXPECT(count == 1);
}

NTH_TEST("early_exit/return-value/continue") {
  int count = 0;

  ErrorOr v = [&]() -> ErrorOr<std::string> {
    ++count;
    std::string value = co_await ErrorOr<std::string>("hello");
    ++count;
    co_return ErrorOr(value + ", world");
  }();

  NTH_EXPECT(v == ErrorOr<std::string>("hello, world"));
  NTH_EXPECT(count == 2);
}

NTH_TEST("early_exit/return-value/exit") {
  int count = 0;

  ErrorOr<std::string> v = [&]() -> ErrorOr<std::string> {
    ++count;
    co_await ErrorOr<std::string>(Error(-3));
    ++count;
    co_return ErrorOr<std::string>("ok");
  }();

  NTH_EXPECT(v == ErrorOr<std::string>(Error(-3)));
  NTH_EXPECT(count == 1);
}

NTH_TEST("early_exit/return-value/convert/continue") {
  int count = 0;

  Error v = [&]() -> Error {
    ++count;
    std::string s = co_await ErrorOr<std::string>("hello");
    NTH_EXPECT(s == "hello");
    ++count;
    co_return Error(3);
  }();

  NTH_EXPECT(v == Error(3));
  NTH_EXPECT(count == 2);
}

NTH_TEST("early_exit/return-value/convert/exit") {
  int count = 0;

  Error v = [&]() -> Error {
    ++count;
    co_await ErrorOr<std::string>(Error(-3));
    ++count;
    co_return Error(3);
  }();

  NTH_EXPECT(v == Error(-3));
  NTH_EXPECT(count == 1);
}

NTH_TEST("early_exit/on-exit/return-void/take-parameter/continue") {
  int count              = 0;
  int exit_handler_total = 0;

  Error v = [&]() -> Error {
    ++count;
    co_await on_exit(ErrorOr<std::string>("hello"),
                     [&](ErrorOr<std::string> const& e) {
                       exit_handler_total += static_cast<Error>(e).value();
                     });
    ++count;
    co_await on_exit(ErrorOr<std::string>("hello"),
                     [&](ErrorOr<std::string> const& e) {
                       exit_handler_total += static_cast<Error>(e).value();
                     });
    ++count;
    co_return Error(3);
  }();

  NTH_EXPECT(v == Error(3));
  NTH_EXPECT(count == 3);
  NTH_EXPECT(exit_handler_total == 0);
}

NTH_TEST("early_exit/on-exit/return-void/take-parameter/exit") {
  int count              = 0;
  int exit_handler_total = 0;

  Error v = [&]() -> Error {
    ++count;
    co_await on_exit(ErrorOr<std::string>("hello"),
                     [&](ErrorOr<std::string> const& e) {
                       exit_handler_total += static_cast<Error>(e).value();
                     });
    ++count;
    co_await on_exit(ErrorOr<std::string>(Error(-3)),
                     [&](ErrorOr<std::string> const& e) {
                       exit_handler_total += static_cast<Error>(e).value();
                     });
    ++count;
    co_return Error(3);
  }();

  NTH_EXPECT(v == Error(-3));
  NTH_EXPECT(count == 2);
  NTH_EXPECT(exit_handler_total == -3);
}

NTH_TEST("early_exit/on-exit/return-void/take-void/continue") {
  int count              = 0;
  int exit_handler_count = 0;

  Error v = [&]() -> Error {
    ++count;
    co_await on_exit(ErrorOr<std::string>("hello"),
                     [&] { ++exit_handler_count; });
    ++count;
    co_await on_exit(ErrorOr<std::string>("hello"),
                     [&] { ++exit_handler_count; });
    ++count;
    co_return Error(3);
  }();

  NTH_EXPECT(v == Error(3));
  NTH_EXPECT(count == 3);
  NTH_EXPECT(exit_handler_count == 0);
}

NTH_TEST("early_exit/on-exit/return-void/take-void/exit") {
  int count              = 0;
  int exit_handler_count = 0;

  Error v = [&]() -> Error {
    ++count;
    co_await on_exit(ErrorOr<std::string>("hello"),
                     [&] { ++exit_handler_count; });
    ++count;
    co_await on_exit(ErrorOr<std::string>(Error(-3)),
                     [&] { ++exit_handler_count; });
    ++count;
    co_return Error(3);
  }();

  NTH_EXPECT(v == Error(-3));
  NTH_EXPECT(count == 2);
  NTH_EXPECT(exit_handler_count == 1);
}

NTH_TEST("early_exit/on-exit/return-value/take-parameter/continue") {
  int count              = 0;
  int exit_handler_total = 0;

  Error v = [&]() -> Error {
    ++count;
    co_await on_exit(ErrorOr<std::string>("hello"),
                     [&](ErrorOr<std::string> const& e) {
                       exit_handler_total += static_cast<Error>(e).value();
                       return Error(-3);
                     });
    ++count;
    co_await on_exit(ErrorOr<std::string>("hello"),
                     [&](ErrorOr<std::string> const& e) {
                       exit_handler_total += static_cast<Error>(e).value();
                       return Error(-4);
                     });
    ++count;
    co_return Error(3);
  }();

  NTH_EXPECT(v == Error(3));
  NTH_EXPECT(count == 3);
  NTH_EXPECT(exit_handler_total == 0);
}

NTH_TEST("early_exit/on-exit/return-value/take-parameter/exit") {
  int count              = 0;
  int exit_handler_total = 0;

  Error v = [&]() -> Error {
    ++count;
    co_await on_exit(ErrorOr<std::string>("hello"),
                     [&](ErrorOr<std::string> const& e) {
                       exit_handler_total += static_cast<Error>(e).value();
                       return Error(-3);
                     });
    ++count;
    co_await on_exit(ErrorOr<std::string>(Error(-2)),
                     [&](ErrorOr<std::string> const& e) {
                       exit_handler_total += static_cast<Error>(e).value();
                       return Error(-4);
                     });
    ++count;
    co_return Error(3);
  }();

  NTH_EXPECT(v == Error(-4));
  NTH_EXPECT(count == 2);
  NTH_EXPECT(exit_handler_total == -2);
}

NTH_TEST("early_exit/on-exit/return-value/take-void/continue") {
  int count              = 0;
  int exit_handler_count = 0;

  Error v = [&]() -> Error {
    ++count;
    co_await on_exit(ErrorOr<std::string>("hello"), [&] {
      ++exit_handler_count;
      return Error(-3);
    });
    ++count;
    co_await on_exit(ErrorOr<std::string>("hello"), [&] {
      ++exit_handler_count;
      return Error(-4);
    });
    ++count;
    co_return Error(3);
  }();

  NTH_EXPECT(v == Error(3));
  NTH_EXPECT(count == 3);
  NTH_EXPECT(exit_handler_count == 0);
}

NTH_TEST("early_exit/on-exit/return-value/take-void/exit") {
  int count              = 0;
  int exit_handler_count = 0;

  Error v = [&]() -> Error {
    ++count;
    co_await on_exit(ErrorOr<std::string>("hello"), [&] {
      ++exit_handler_count;
      return Error(-3);
    });
    ++count;
    co_await on_exit(ErrorOr<std::string>(Error(-3)), [&] {
      ++exit_handler_count;
      return Error(-4);
    });
    ++count;
    co_return Error(3);
  }();

  NTH_EXPECT(v == Error(-4));
  NTH_EXPECT(count == 2);
  NTH_EXPECT(exit_handler_count == 1);
}

}  // namespace
}  // namespace nth
