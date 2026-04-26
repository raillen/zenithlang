# Bugfix Requirements Document

## Introduction

O teste `test_boundary_copy_rejects_unsupported_transferable_shape` está falhando porque o type checker não está gerando um diagnóstico de erro quando `concurrent.copy_text(value)` é chamado com um argumento do tipo `Snapshot` (struct) em vez do tipo esperado `text`. O teste espera receber 1 diagnóstico `ZT_DIAG_INVALID_CALL`, mas está recebendo 0 diagnósticos, indicando que a validação de tipo não está sendo aplicada corretamente para chamadas de funções importadas de módulos da stdlib.

## Bug Analysis

### Current Behavior (Defect)

1.1 WHEN `concurrent.copy_text(value)` is called where `value` is of type `Snapshot` (a user-defined struct) THEN the system does not generate any type mismatch diagnostic

1.2 WHEN a function from an imported stdlib module expects a parameter of type `text` but receives an argument of an incompatible type (struct) THEN the system fails to validate the argument type and does not emit `ZT_DIAG_INVALID_CALL` or `ZT_DIAG_TYPE_MISMATCH`

### Expected Behavior (Correct)

2.1 WHEN `concurrent.copy_text(value)` is called where `value` is of type `Snapshot` (a user-defined struct) THEN the system SHALL generate a type mismatch diagnostic (either `ZT_DIAG_INVALID_CALL` or `ZT_DIAG_TYPE_MISMATCH`)

2.2 WHEN a function from an imported stdlib module expects a parameter of type `text` but receives an argument of an incompatible type (struct) THEN the system SHALL validate the argument type and emit an appropriate diagnostic indicating the type mismatch

### Unchanged Behavior (Regression Prevention)

3.1 WHEN `concurrent.copy_text(value)` is called where `value` is of type `text` THEN the system SHALL CONTINUE TO accept the call without generating any diagnostics

3.2 WHEN `concurrent.copy_int(value)` is called where `value` is of type `int` THEN the system SHALL CONTINUE TO accept the call without generating any diagnostics

3.3 WHEN other stdlib function calls receive arguments of the correct type THEN the system SHALL CONTINUE TO accept those calls without generating diagnostics

3.4 WHEN type checking is performed for non-stdlib function calls with type mismatches THEN the system SHALL CONTINUE TO generate appropriate type mismatch diagnostics as before
