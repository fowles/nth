#ifndef NTH_DEBUG_INTERNAL_TRACE_H
#define NTH_DEBUG_INTERNAL_TRACE_H

#include <concepts>
#include <iomanip>
#include <iostream>
#include <memory>

#include "nth/io/string_printer.h"
#include "nth/io/universal_print.h"
#include "nth/meta/sequence.h"
#include "nth/meta/type.h"

namespace nth::internal_trace {

// Base class from which all `Traced` objects inherit.
struct TracedBase {};

// A wrapper around a value of type `T` that was computed via some tracing
// mechanism.
template <typename T>
struct TracedValue : TracedBase {
  using type = T;

 protected:
  // Constructs the traced value by invoking `f` with the arguments `ts...`.
  constexpr TracedValue(auto f, auto const &...ts) : value_(f(ts...)) {}

 private:
  // Friend function template abstracting over a `TracedValue<T>` and a `T`.
  template <typename U>
  friend constexpr decltype(auto) Evaluate(U const &value);

  type value_;
};

// `Traced` is the primary workhorse of this library. It takes an `Action`
// template parameter indicating how the value is computed, and a collection of
// `Ts` which are input types to that action. It stores the computed value as
// well as pointers to each of the values used in the computation (which must
// remain valid for the lifetime of this object.
template <typename Action, typename... Ts>
struct Traced : TracedValue<typename Action::template invoke_type<Ts...>> {
  using action_type                    = Action;
  static constexpr auto argument_types = nth::type_sequence<Ts...>;

  constexpr Traced(NTH_ATTRIBUTE(lifetimebound) Ts const &...ts)
      : TracedValue<typename action_type::template invoke_type<Ts...>>(
            [&](auto const &...vs) -> decltype(auto) {
              return action_type::invoke(vs...);
            },
            ts...),
        ptrs_{std::addressof(ts)...} {}

 private:
  friend struct TracedTraversal;

  void const *ptrs_[sizeof...(Ts)];
};

// A concept matching any type deriving from `TracedBase`. Only `TracedValue`
// instantiations are allowed to inherit from `TracedBase` so this concept
// matches any "traced" type.
template <typename T>
concept TracedImpl = std::derived_from<T, internal_trace::TracedBase>;

template <typename T, typename U>
concept TracedEvaluatingTo =
    std::derived_from<T, nth::internal_trace::TracedValue<U>>;

template <nth::CompileTimeString S>
struct Identity {
  static constexpr auto name = S;

  template <typename T>
  using invoke_type = T;

  template <typename T>
  static constexpr decltype(auto) invoke(T const &t) {
    return t;
  }
};

// Function template which, when `value` is a traced object extracts a constant
// reference to the underlying value, and otherwise returns a constant reference
// to the passed in `value` unmodified.
template <typename T>
decltype(auto) constexpr Evaluate(NTH_ATTRIBUTE(lifetimebound) T const &value) {
  if constexpr (nth::internal_trace::TracedImpl<T>) {
    return (value.value_);
  } else {
    return value;
  }
}

template <typename PreFn, std::invocable<> auto PostFn>
struct Responder {
  constexpr bool set(char const *expression,
                     TracedEvaluatingTo<bool> auto const &b,
                     char const *file_name, int line_number) {
    value_ = Evaluate(b);
    if (not value_) { PreFn{}(expression, b, file_name, line_number); }
    return value_;
  }

  constexpr ~Responder() {
    if (not value_) { PostFn(); }
  }

 private:
  bool value_;
};

struct TracedTraversal {
  template <typename T>
  void operator()(T const &trace) {
    if constexpr (nth::internal_trace::TracedImpl<T>) {
      if constexpr (requires { T::action_type::name; } and
                    nth::type<typename T::action_type> ==
                        nth::type<nth::internal_trace::Identity<
                            T::action_type::name>>) {
        std::string s(indentation, ' ');
        nth::StringPrinter p(s);
        nth::UniversalPrint(p, nth::internal_trace::Evaluate(trace));
        if constexpr (T::action_type::name.empty()) {
          std::cerr << s << " [traced value]\n";
        } else {
          std::cerr << s << " [traced value "
                    << std::quoted(T::action_type::name.data()) << "]\n";
        }
      } else {
        std::string s;
        nth::StringPrinter p(s);
        nth::UniversalPrint(p, nth::internal_trace::Evaluate(trace));

        T::argument_types.reduce([&](auto... ts) {
          std::cerr << std::string(indentation, ' ') << T::action_type::name
                    << std::string(
                           40 - indentation - sizeof(T::action_type::name),
                           ' ');
          std::cerr << "(= " << s << ")\n";
          indentation += 2;
          size_t i = 0;
          ((*this)(*static_cast<::nth::type_t<ts> const *>(trace.ptrs_[i++])),
           ...);
          indentation -= 2;
        });
      }
    } else {
      std::string s;
      nth::StringPrinter p(s);
      nth::UniversalPrint(p, trace);
      std::cerr << std::string(indentation, ' ') << s << '\n';
    }
  }

  int indentation = 0;
};

struct Explain {
  void operator()(char const *expression,
                  TracedEvaluatingTo<bool> auto const &b, char const *file_name,
                  int line_number) {
    std::cerr << "\033[31m"
                 "NTH_ASSERT failed at "
              << "\033[1m" << file_name << ':' << line_number << ":\033[m\n  "
              << expression << "\n\n";

    TracedTraversal traverser;
    traverser(b);
  }
};

struct TraceInjector {};
template <typename T>
constexpr decltype(auto) operator->*(TraceInjector, T const &value) {
  if constexpr (nth::internal_trace::TracedImpl<T>) {
    return value;
  } else {
    return Traced<Identity<"">, T>(value);
  }
}
template <typename T>
constexpr decltype(auto) operator->*(T const &value, TraceInjector) {
  if constexpr (nth::internal_trace::TracedImpl<T>) {
    return value;
  } else {
    return Traced<Identity<"">, T>(value);
  }
}

}  // namespace nth::internal_trace

#define NTH_DEBUG_INTERNAL_TRACE_EXPECT(...)                                   \
  if (::nth::internal_trace::Responder<::nth::internal_trace::Explain, [] {}>  \
          responder;                                                           \
      responder.set(                                                           \
          (#__VA_ARGS__),                                                      \
          (::nth::internal_trace::TraceInjector{}                              \
               ->*__VA_ARGS__->*::nth::internal_trace::TraceInjector{}),       \
          __FILE__, __LINE__)) {}

#define NTH_DEBUG_INTERNAL_TRACE_ASSERT(...)                                   \
  if (::nth::internal_trace::Responder<::nth::internal_trace::Explain,         \
                                       std::abort>                             \
          responder;                                                           \
      responder.set(                                                           \
          (#__VA_ARGS__),                                                      \
          (::nth::internal_trace::TraceInjector{}                              \
               ->*__VA_ARGS__->*::nth::internal_trace::TraceInjector{}),       \
          __FILE__, __LINE__)) {}

#endif  // NTH_DEBUG_INTERNAL_TRACE_H