#include "nth/debug/trace/trace.h"

#include <vector>

#include "nth/debug/log/stderr_log_sink.h"

namespace {

struct Thing {
  int triple() const { return n * 3; }
  Thing add(int k) const { return Thing{.n = n + k}; }

  int& value() { return n; }
  int const& value() const { return n; }

  bool operator==(Thing const&) const = default;

  int n;
};

template <typename T>
struct S {
  T triple() const { return n * 3; }
  S add(T k) const { return S<T>{.n = n + k}; }

  T& value() { return n; }
  T const& value() const { return n; }

  bool operator==(S const&) const = default;

  T n;
};

}  // namespace

NTH_TRACE_DECLARE_API(Thing, (triple)(add)(value));

template <typename T>
NTH_TRACE_DECLARE_API_TEMPLATE(S<T>, (triple)(add)(value));

static_assert(not nth::Traced<int>);
static_assert(nth::Traced<decltype(nth::Trace<"n">(3))>);

void ComparisonExpectations() {
  int n  = 3;
  auto t = nth::Trace<"n">(n);

  // Comparison expectations
  NTH_EXPECT(t == 3);
  NTH_EXPECT(t <= 4);
  NTH_EXPECT(t < 4);
  NTH_EXPECT(t >= 2);
  NTH_EXPECT(t > 2);
  NTH_EXPECT(t != 2);

  // More complex expression expectations
  NTH_EXPECT(t * 2 == 6);
  NTH_EXPECT(t * 2 + 1 == 7);
  NTH_EXPECT((1 + t) * 2 + 1 == 9);
  NTH_EXPECT(9 == (1 + t) * 2 + 1);

  // Comparison assertions
  NTH_ASSERT(t == 3);
  NTH_ASSERT(t <= 4);
  NTH_ASSERT(t < 4);
  NTH_ASSERT(t >= 2);
  NTH_ASSERT(t > 2);
  NTH_ASSERT(t != 2);
  NTH_ASSERT((t << 2) == 12);

  // More complex expression assertions
  NTH_ASSERT(t * 2 == 6);
  NTH_ASSERT(t * 2 + 1 == 7);
  NTH_ASSERT((1 + t) * 2 + 1 == 9);
  NTH_ASSERT(9 == (1 + t) * 2 + 1);
}

struct Uncopyable {
  Uncopyable()                             = default;
  Uncopyable(Uncopyable const&)            = delete;
  Uncopyable(Uncopyable&&)                 = default;
  Uncopyable& operator=(Uncopyable const&) = delete;
  Uncopyable& operator=(Uncopyable&&)      = default;

  friend bool operator==(Uncopyable const&, Uncopyable const&) { return true; }
};

void MoveOnly() {
  Uncopyable u;
  auto t = nth::Trace<"u">(u);

  NTH_EXPECT(t == t);
  NTH_ASSERT(t == t);
}

void ShortCircuiting() {
  int n  = 3;
  auto t = nth::Trace<"n">(n);

  NTH_EXPECT(t == 0 or (3 / t) == 1);
  NTH_EXPECT(t == 2 or t == 3);
}

int main() {
  static int failure_count = 0;
  nth::RegisterLogSink(nth::stderr_log_sink);
  nth::RegisterExpectationResultHandler(
      [](nth::debug::ExpectationResult const& result) {
        if (not result.success()) { ++failure_count; }
      });
  ComparisonExpectations();
  if (failure_count != 0) { return 1; }
  ShortCircuiting();
  if (failure_count != 0) { return 1; }
  MoveOnly();
  if (failure_count != 0) { return 1; }

  // Declared API
  Thing thing{.n = 5};
  auto traced_thing = nth::Trace<"thing">(thing);
  NTH_EXPECT(traced_thing.triple() == 15);
  NTH_EXPECT(traced_thing.value() == 5);
  NTH_EXPECT(traced_thing.add(3).add(4).add(10) == Thing{.n = 22});
  if (failure_count != 0) { return 1; }

  // Declared API template
  S<int> s{.n = 5};
  auto ts = nth::Trace<"s">(s);
  NTH_EXPECT(ts.triple() == 15);
  NTH_EXPECT(ts.value() == 5);
  NTH_EXPECT((v.always), ts.add(3).add(4).add(10) == S<int>{.n = 22});
  if (failure_count != 0) { return 1; }
  return 0;
}
