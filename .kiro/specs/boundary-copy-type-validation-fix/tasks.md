# Implementation Plan

- [-] 1. Write bug condition exploration test
  - **Property 1: Bug Condition** - Stdlib Function Type Mismatch Not Detected
  - **CRITICAL**: This test MUST FAIL on unfixed code - failure confirms the bug exists
  - **DO NOT attempt to fix the test or the code when it fails**
  - **NOTE**: This test encodes the expected behavior - it will validate the fix when it passes after implementation
  - **GOAL**: Surface counterexamples that demonstrate the bug exists
  - **Scoped PBT Approach**: For this deterministic bug, scope the property to the concrete failing case: `concurrent.copy_text(value)` where `value` is of type `Snapshot` (struct)
  - Test that when `concurrent.copy_text(value)` is called where `value` is of type `Snapshot` (struct), the type checker generates a diagnostic error (either `ZT_DIAG_INVALID_CALL` or `ZT_DIAG_TYPE_MISMATCH`)
  - The test should verify that exactly 1 diagnostic is generated for this type mismatch
  - Run test on UNFIXED code
  - **EXPECTED OUTCOME**: Test FAILS (this is correct - it proves the bug exists: 0 diagnostics generated instead of 1)
  - Document counterexamples found: the specific case where `concurrent.copy_text(value)` with `Snapshot` type fails to generate a diagnostic
  - Mark task complete when test is written, run, and failure is documented
  - _Requirements: 1.1, 1.2_

- [~] 2. Write preservation property tests (BEFORE implementing fix)
  - **Property 2: Preservation** - Valid Type Checking Behavior Preserved
  - **IMPORTANT**: Follow observation-first methodology
  - Observe behavior on UNFIXED code for non-buggy inputs:
    - `concurrent.copy_text(value)` where `value` is of type `text` should NOT generate diagnostics
    - `concurrent.copy_int(value)` where `value` is of type `int` should NOT generate diagnostics
    - Other stdlib function calls with correct types should NOT generate diagnostics
    - Non-stdlib function calls with type mismatches should generate diagnostics
  - Write property-based tests capturing observed behavior patterns:
    - Property: For all stdlib function calls with correct argument types, no diagnostics are generated
    - Property: For all non-stdlib function calls with type mismatches, appropriate diagnostics are generated
  - Property-based testing generates many test cases for stronger guarantees
  - Run tests on UNFIXED code
  - **EXPECTED OUTCOME**: Tests PASS (this confirms baseline behavior to preserve)
  - Mark task complete when tests are written, run, and passing on unfixed code
  - _Requirements: 3.1, 3.2, 3.3, 3.4_

- [ ] 3. Fix for stdlib function parameter type validation

  - [~] 3.1 Implement the fix
    - Locate the type checking logic for function calls in the semantic analyzer
    - Identify why stdlib function calls (imported from modules) are not being type-checked
    - Add or fix type validation for stdlib function parameters to ensure argument types match expected parameter types
    - Ensure the fix generates appropriate diagnostics (`ZT_DIAG_INVALID_CALL` or `ZT_DIAG_TYPE_MISMATCH`) when type mismatches occur
    - _Bug_Condition: isBugCondition(call) where call is `concurrent.copy_text(value)` AND value.type is `Snapshot` (struct) AND expected parameter type is `text`_
    - _Expected_Behavior: When isBugCondition(call) is true, the system SHALL generate exactly 1 type mismatch diagnostic (ZT_DIAG_INVALID_CALL or ZT_DIAG_TYPE_MISMATCH)_
    - _Preservation: Valid stdlib calls with correct types (3.1), valid non-stdlib calls (3.2, 3.3), and existing type checking for non-stdlib calls (3.4) must continue to work without generating diagnostics_
    - _Requirements: 1.1, 1.2, 2.1, 2.2, 3.1, 3.2, 3.3, 3.4_

  - [~] 3.2 Verify bug condition exploration test now passes
    - **Property 1: Expected Behavior** - Type Mismatch Diagnostic Generated
    - **IMPORTANT**: Re-run the SAME test from task 1 - do NOT write a new test
    - The test from task 1 encodes the expected behavior
    - When this test passes, it confirms the expected behavior is satisfied
    - Run bug condition exploration test from step 1
    - **EXPECTED OUTCOME**: Test PASSES (confirms bug is fixed: 1 diagnostic is now generated)
    - _Requirements: 2.1, 2.2_

  - [~] 3.3 Verify preservation tests still pass
    - **Property 2: Preservation** - Valid Type Checking Behavior Preserved
    - **IMPORTANT**: Re-run the SAME tests from task 2 - do NOT write new tests
    - Run preservation property tests from step 2
    - **EXPECTED OUTCOME**: Tests PASS (confirms no regressions)
    - Confirm all tests still pass after fix (no regressions in valid calls or existing type checking)

- [~] 4. Checkpoint - Ensure all tests pass
  - Ensure all tests pass, ask the user if questions arise.
