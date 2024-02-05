#include <cstdint>
#include <string>
#include <vector>

#include "nth/io/deserialize.h"
#include "nth/io/serialize.h"
#include "nth/io/string_reader.h"
#include "nth/io/string_writer.h"
#include "nth/test/test.h"

namespace nth::io {
namespace {

NTH_TEST("round-trip/integer", auto n) {
  decltype(n) m;
  std::string s;

  string_writer w(s);
  NTH_ASSERT(serialize_integer(w, n));

  string_reader r(s);
  NTH_ASSERT(deserialize_integer(r, m));

  NTH_ASSERT(r.size() == 0u);
  NTH_EXPECT((int)m == (int)n);
}

NTH_INVOKE_TEST("round-trip/integer") {
  for (int8_t n : {
           int8_t{0},
           int8_t{1},
           int8_t{-1},
           int8_t{10},
           int8_t{-10},
           std::numeric_limits<int8_t>::max(),
           std::numeric_limits<int8_t>::lowest(),
       }) {
    co_yield n;
  }

  for (uint8_t n : {
           uint8_t{0},
           uint8_t{1},
           uint8_t{10},
           std::numeric_limits<uint8_t>::max(),
       }) {
    co_yield n;
  }

  for (int32_t n : {
           int32_t{0},
           int32_t{1},
           int32_t{-1},
           int32_t{10},
           int32_t{-10},
           int32_t{256},
           int32_t{-256},
           int32_t{257},
           int32_t{-257},
           int32_t{1'000'000},
           int32_t{-1'000'000},
           std::numeric_limits<int32_t>::max(),
           std::numeric_limits<int32_t>::lowest(),
       }) {
    co_yield n;
  }

  for (uint32_t n : {
           uint32_t{0},
           uint32_t{1},
           uint32_t{10},
           uint32_t{256},
           uint32_t{257},
           uint32_t{1'000'000},
           std::numeric_limits<uint32_t>::max(),
       }) {
    co_yield n;
  }
}

struct Thing {
  int n;
  friend bool NthSerialize(auto &s, Thing const &t) {
    return serialize_integer(s, t.n);
  }
  friend bool NthDeserialize(auto &d, Thing &t) {
    return deserialize_integer(d, t.n);
  }
  friend bool operator==(Thing, Thing) = default;
};

decltype(auto) NthEmplace(std::vector<Thing> &v) { return v.emplace_back(); }

NTH_TEST("round-trip/sequence", std::vector<Thing> const & v) {
  std::vector<Thing> round_tripped;
  std::string s;

  string_writer w(s);
  NTH_ASSERT(serialize_sequence(w, v));

  string_reader r(s);
  NTH_ASSERT(deserialize_sequence(r, round_tripped));

  NTH_ASSERT(r.size() == 0u);
  NTH_EXPECT(v == round_tripped);
}

NTH_INVOKE_TEST("round-trip/sequence") {
  co_yield std::vector<Thing>{};
  co_yield std::vector<Thing>{Thing{0}};
  co_yield std::vector<Thing>{Thing{0}, Thing{1}};
  co_yield std::vector<Thing>{Thing{0}, Thing{1}, Thing{2}, Thing{3}, Thing{4}, Thing{5}};
}

}  // namespace
}  // namespace nth::io
