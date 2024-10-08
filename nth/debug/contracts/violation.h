#ifndef NTH_DEBUG_TRACE_CONTRACTS_VIOLATION_H
#define NTH_DEBUG_TRACE_CONTRACTS_VIOLATION_H

#include <string_view>

#include "nth/base/attributes.h"
#include "nth/debug/contracts/contract.h"
#include "nth/debug/contracts/internal/any_formattable_ref.h"
#include "nth/debug/source_location.h"
#include "nth/registration/registrar.h"

namespace nth {

// `contract_violation`:
//
// Represents a failing result of a computation intended as verification during
// a test or debug-check.
struct contract_violation {
  contract_violation(contract const& c,
                     internal_contracts::any_formattable_ref payload);

  [[nodiscard]] constexpr struct source_location source_location() const {
    return contract_.source_location();
  }

  // The contract which was violated.
  [[nodiscard]] constexpr struct contract const& contract() const {
    return contract_;
  }

  [[nodiscard]] constexpr internal_contracts::any_formattable_ref payload()
      const NTH_ATTRIBUTE(lifetimebound) {
    return payload_;
  }

 private:
  struct contract const& contract_;
  internal_contracts::any_formattable_ref payload_;
};

// `register_contract_violation_handler`:
//
// Registers `handler` to be executed any time an expectation is evaluated (be
// that `NTH_REQUIRE`, `NTH_ENSURE`, `NTH_EXPECT`, or `NTH_ASSERT`). The
// execution order of handlers is unspecified and may be executed
// concurrently. Handlers cannot be un-registered.
void register_contract_violation_handler(
    void (*handler)(contract_violation const&));

// `registered_contract_violation_handlers`:
//
// Returns a view over the globally registered `contract_violation` handlers.
// Note that `ExpecattionResult` handlers may be registered at any time
// including during the execution of this function. The returned range will
// view all handlers registered at a particular instant. If another handler
// registration happens after the function returns, the returned
// `contract_violationHandlerRange` will not contain the newly registered
// handler.
registrar<void (*)(contract_violation const&)>::range_type
registered_contract_violation_handlers();

}  // namespace nth

#endif  // NTH_DEBUG_TRACE_CONTRACTS_VIOLATION_H
