#ifndef NTH_UTILITY_ITERATOR_RANGE_H
#define NTH_UTILITY_ITERATOR_RANGE_H

#include <type_traits>
#include <utility>

namespace nth {
namespace internal_iterator_range {

template <typename Iterator, int Tag>
struct Base : private Iterator {
  explicit constexpr Base(Iterator&& iterator)
      : Iterator(std::move(iterator)) {}
  explicit constexpr Base(Iterator const& iterator) : Iterator(iterator) {}

  constexpr Iterator const& iterator() const {
    return static_cast<Iterator const&>(*this);
  }
};

}  // namespace internal_iterator_range

template <typename B, typename E>
struct iterator_range : private internal_iterator_range::Base<B, 0>,
                        private internal_iterator_range::Base<E, 1> {
  using value_type     = std::decay_t<decltype(*std::declval<B>())>;
  using const_iterator = B;

  iterator_range(B b, E e)
      : internal_iterator_range::Base<B, 0>(std::move(b)),
        internal_iterator_range::Base<E, 1>(std::move(e)) {}

  auto begin() const {
    return static_cast<internal_iterator_range::Base<B, 0> const&>(*this)
        .iterator();
  }
  auto end() const {
    return static_cast<internal_iterator_range::Base<E, 1> const&>(*this)
        .iterator();
  }

  bool empty() const { return begin() == end(); }
};

template <typename B, typename E>
iterator_range(B, E) -> iterator_range<B, E>;

}  // namespace nth

#endif  // NTH_UTILITY_ITERATOR_RANGE_H
