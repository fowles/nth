#ifndef NTH_IO_PRINTER_H
#define NTH_IO_PRINTER_H

#include <concepts>
#include <string_view>

#include "nth/io/format/format.h"
#include "nth/io/format/interpolation_spec.h"
#include "nth/meta/concepts/core.h"
#include "nth/meta/type.h"
#include "nth/strings/interpolate/string.h"
#include "nth/strings/interpolate/types.h"

namespace nth {
namespace io {

template <typename T, typename P>
concept printable_with = requires(P p, T const &t) {
  NthFormat(p, std::declval<nth::io::format_spec<T>>(), t);
};

template <typename P>
struct printer {
  template <printable_with<P> T>
  static auto print(P p, auto spec, T const &value) {
    if constexpr (nth::type<decltype(spec)> ==
                  nth::type<nth::io::interpolation_spec>) {
      return NthFormat(p, ::nth::format_spec_from<T>(spec), value);
    } else {
      return NthFormat(p, std::move(spec), value);
    }
  }
};

template <typename P>
concept printer_type = std::derived_from<P, printer<P>> and requires(P p) {
  { p.write(std::string_view()) } -> nth::precisely<void>;
};

template <int &..., printer_type P>
void print(P p, auto spec, auto const &value) {
  P::print(std::move(p), std::move(spec), value);
}

// `minimal_printer` is the most trivial type satisfying the `printer_type`
// concept. Its member functions are not implemented as it is intended only for
// use at compile-time with type-traits. `minimal_printer` is intended to be
// used as a stand-in for compile-time checks where a generic printer is needed:
// If a member function can be instantiated with `minimal_printer`, it should be
// instantiatiable with any `printer_type`.
struct minimal_printer : printer<minimal_printer> {
  void write(nth::precisely<std::string_view> auto);
};

}  // namespace io
}  // namespace nth

#endif  // NTH_IO_PRINTER_H
