#ifndef NTH_BASE_INDESTRUCTIBLE_H
#define NTH_BASE_INDESTRUCTIBLE_H

#include "nth/base/core.h"
#include "nth/memory/address.h"
#include "nth/memory/buffer.h"

namespace nth {

// An `indestructible<T>` is a wrapper around an object of type `T` which does
// not invoke the destructor of `T` when it is destroyed. The primary motivation
// for such a type is for non-trivial static globals; such types frequently do
// not need to have their destructors run (as resources are returned to the
// operating system on exit). Moreover running these destructors is typically
// fraught, if references to these object are held by multiple threads.
template <typename T>
struct indestructible : private buffer<sizeof(T), alignof(T)> {
  template <typename... Args>
  constexpr indestructible(Args &&...args)
      : buffer<sizeof(T), alignof(T)>(buffer_construct<T>,
                                      NTH_FWD(args)...) {}

  T &operator*() & { return this->template as<T>(); }
  T const &operator*() const & { return this->template as<T>(); }
  T &&operator*() && { return NTH_MOVE(*this).template as<T>(); }
  T const &&operator*() const && { return NTH_MOVE(*this).template as<T>(); }

  T const *operator->() const { return nth::address(this->template as<T>()); }
  T *operator->() { return nth::address(this->template as<T>()); }
};

template <typename T>
indestructible(T) -> indestructible<T>;

}  // namespace nth

#endif  // NTH_BASE_INDESTRUCTIBLE_H
