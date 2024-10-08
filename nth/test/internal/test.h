#ifndef NTH_TEST_INTERNAL_TEST_H
#define NTH_TEST_INTERNAL_TEST_H

#include <functional>
#include <type_traits>

#include "nth/base/macros.h"
#include "nth/meta/stateful.h"
#include "nth/registration/static.h"
#include "nth/strings/glob.h"
#include "nth/test/internal/arguments.h"
#include "nth/test/internal/invocation.h"

// The stateful compile-time sequence of types which will be appended to once
// for each parameterized test encountered. Despite not being in an internal
// namespace, this object is for internal use only and must not be used directly
// outside of this library.
NTH_DEFINE_MUTABLE_COMPILE_TIME_SEQUENCE(NthInternalParameterizedTestSequence);

// The stateful compile-time sequence of types which will be appended to once
// for each parameterized test invocation encountered. Despite not being in an
// internal namespace, this object is for internal use only and must not be used
// directly outside of this library.
NTH_DEFINE_MUTABLE_COMPILE_TIME_SEQUENCE(
    NthInternalParameterizedTestInvocationSequence);

#define NTH_INTERNAL_IMPLEMENT_ASSERT(verbosity_path, ...)                     \
  NTH_INTERNAL_CONTRACTS_CHECK("NTH_ASSERT", verbosity_path, return,           \
                               __VA_ARGS__)

#define NTH_INTERNAL_IMPLEMENT_EXPECT(verbosity_path, ...)                     \
  NTH_INTERNAL_CONTRACTS_CHECK("NTH_EXPECT", verbosity_path, , __VA_ARGS__)

namespace nth::test {

void RegisterTestInvocation(std::string_view categorization,
                            std::function<void()> f);

namespace internal_test {

// Accepts an invocation type as a template argument and a sequence of all test
// types seen so far during compilation. For each test, if the test's category
// matches the invocation's glob, the invocation is instantiated for the test
// and registered.
template <typename TestInvocationType>
void RegisterTestsMatching(nth::Sequence auto seq) {
  seq.each([&](auto t) {
    using type = nth::type_t<t>;
    if constexpr (nth::GlobMatches(TestInvocationType::categorization(),
                                   type::categorization())) {
      RegisterTestInvocation(type::categorization(), [] {
        TestInvocationType::template Invocation<type>();
      });
    }
  });
}

// Accepts a test type as a template argument and a sequence of all test
// invocations seen so far during compilation. For each invocation, if the
// test's category matches the invocation's glob, the invocation is instantiated
// for the test and registered.
template <typename Test>
void RegisterInvocationsMatching(nth::Sequence auto seq) {
  seq.each([&](auto t) {
    using type = nth::type_t<t>;
    if constexpr (nth::GlobMatches(type::categorization(),
                                   Test::categorization())) {
      RegisterTestInvocation(Test::categorization(),
                             [] { type::template Invocation<Test>(); });
    }
  });
}

}  // namespace internal_test

#define NTH_INTERNAL_TEST_IMPL(name, initializer, categorization, ...)         \
  NTH_IF(NTH_IS_EMPTY(__VA_ARGS__), NTH_INTERNAL_TEST_IMPL_,                   \
         NTH_INTERNAL_TEST_PARAMETERIZED_IMPL_)                                \
  (name, initializer, categorization, __VA_ARGS__)

#define NTH_INTERNAL_TEST_IMPL_(test_name, ignored_initializer, cat, ...)      \
  struct test_name {                                                           \
    static std::string_view categorization() { return cat; }                   \
    static void InvokeTest();                                                  \
                                                                               \
   private:                                                                    \
    static nth::registration_token const registration_token;                   \
    [[maybe_unused]] static constexpr ::nth::odr_use<&registration_token>      \
        RegisterTokenUse;                                                      \
  };                                                                           \
  inline nth::registration_token const test_name::registration_token = [] {    \
    ::nth::test::RegisterTestInvocation(categorization(),                      \
                                        &test_name::InvokeTest);               \
  };                                                                           \
  void test_name::InvokeTest()

#define NTH_INTERNAL_TEST_PARAMETERIZED_IMPL_(test_name, initializer, cat,     \
                                              ...)                             \
  struct test_name {                                                           \
    static constexpr std::string_view categorization() { return cat; }         \
    static void InvokeTest(__VA_ARGS__);                                       \
                                                                               \
   private:                                                                    \
    static nth::registration_token const registration_token;                   \
    [[maybe_unused]] static constexpr ::nth::odr_use<&registration_token>      \
        RegisterTokenUse;                                                      \
  };                                                                           \
  inline nth::registration_token const test_name::registration_token = [] {    \
    ::nth::test::internal_test::RegisterInvocationsMatching<test_name>(        \
        *::NthInternalParameterizedTestInvocationSequence);                    \
    ::NthInternalParameterizedTestSequence.append<nth::type<test_name>>();     \
  };                                                                           \
  void test_name::InvokeTest(__VA_ARGS__)

#define NTH_INTERNAL_INVOKE_TEST_IMPL(invocation_name, initializer, cat)       \
  struct invocation_name {                                                     \
    static constexpr std::string_view categorization() { return cat; }         \
                                                                               \
    template <typename T>                                                      \
    static ::nth::test::internal_invocation::TestInvocation<T> Invocation();   \
                                                                               \
   private:                                                                    \
    static nth::registration_token const registration_token;                   \
    [[maybe_unused]] static constexpr ::nth::odr_use<&registration_token>      \
        RegisterTokenUse;                                                      \
  };                                                                           \
  inline nth::registration_token const invocation_name::registration_token =   \
      [] {                                                                     \
        ::NthInternalParameterizedTestInvocationSequence                       \
            .append<nth::type<invocation_name>.decayed()>();                   \
        ::nth::test::internal_test::RegisterTestsMatching<invocation_name>(    \
            *::NthInternalParameterizedTestSequence);                          \
      };                                                                       \
  template <typename T>                                                        \
  ::nth::test::internal_invocation::TestInvocation<T>                          \
  invocation_name::Invocation()

}  // namespace nth::test

#endif  // NTH_TEST_INTERNAL_TEST_H
