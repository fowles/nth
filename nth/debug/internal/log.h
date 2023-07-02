#ifndef NTH_DEBUG_INTERNAL_LOG_H
#define NTH_DEBUG_INTERNAL_LOG_H

#include <array>
#include <functional>
#include <iostream>

#include "nth/base/attributes.h"
#include "nth/base/macros.h"
#include "nth/configuration/verbosity.h"
#include "nth/debug/source_location.h"
#include "nth/io/universal_print.h"
#include "nth/strings/format.h"

namespace nth::internal_log {

struct cerr_printer {
  void write(char c) { std::cerr << c; }
  void write(std::string_view s) { std::cerr << s; }
};

template <nth::Printer P, int N>
struct LogEntry {
  template <typename... Ts>
  LogEntry(NTH_ATTRIBUTE(lifetimebound) Ts const&... ts)  //
      requires(sizeof...(Ts) == N)
      : entries_{Erased(ts)...} {}

  auto const& entries() const { return entries_; };

 private:
  struct Erased {
    template <typename T>
    Erased(NTH_ATTRIBUTE(lifetimebound) T const& t)
        : object_(std::addressof(t)), log_([](P& p, void const* t) {
            nth::universal_formatter(p, *reinterpret_cast<T const*>(t));
          }) {}

    friend void NthPrint(P& p, Erased e) { e.log_(p, e.object_); }

    void const* object_;
    void (*log_)(P& p, void const*);
  };

  std::array<Erased, N> entries_;
};

template <nth::FormatString Fmt>
struct LogFormat {
  explicit constexpr LogFormat(source_location location)
      : source_location_(location) {}

  friend void operator<<=(
      LogFormat const& f,
      NTH_ATTRIBUTE(lifetimebound)
          LogEntry<cerr_printer, Fmt.placeholders()> const& entry) {
    static cerr_printer cerr_printer_impl;
    nth::Format<"\x1b[0;34m{} {}:{}]\x1b[0m ">(
        cerr_printer_impl, nth::universal_formatter,
        f.source_location_.file_name(), f.source_location_.function_name(),
        f.source_location_.line());
    std::apply(
        [&](auto... entries) {
          nth::Format<Fmt>(cerr_printer_impl, nth::universal_formatter,
                           entries...);
        },
        entry.entries());

    nth::Format<"\n">(cerr_printer_impl, nth::universal_formatter);
  }

 private:
  source_location source_location_;
};

}  // namespace nth::internal_log

#define NTH_DEBUG_INTERNAL_LOG(...)                                            \
  NTH_DEBUG_INTERNAL_LOG_WITH_VERBOSITY(                                       \
      (::nth::config::default_log_verbosity_requirement), __VA_ARGS__)

#define NTH_DEBUG_INTERNAL_LOG_WITH_VERBOSITY(verbosity, ...)                  \
  switch (0)                                                                   \
  default:                                                                     \
    ([&] {                                                                     \
      [[maybe_unused]] constexpr auto& v = ::nth::debug_verbosity;             \
      return not(verbosity)(::nth::source_location::current());                \
    }())                                                                       \
        ? (void)0                                                              \
        : ::nth::internal_log::LogFormat<__VA_ARGS__>(                         \
              ::nth::source_location::current())


#endif // NTH_DEBUG_INTERNAL_LOG_H
