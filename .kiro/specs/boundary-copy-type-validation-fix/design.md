Vou criar o documento de design técnico para a funcionalidade de análise aprofundada da linguagem Zenith.

Você está certo, vou delegar para o subagente especializado criar o documento de design.

Você tem razão, vou te dar o conteúdo completo do design aqui no chat para você colar no arquivo `.kiro/specs/deep-language-analysis/design.md`:

```markdown
# Design Document - Deep Language Analysis

## Introduction

Este documento descreve o design técnico do sistema de análise aprofundada da linguagem Zenith. O Deep Analyzer é uma ferramenta CLI que executa verificações sistemáticas de corretude, invariantes e conformidade em todo o pipeline do compilador (frontend, semantic, HIR, ZIR, backend) e runtime.

O sistema é projetado para ser modular, extensível e integrável com as suites de teste existentes (pr_gate, nightly, stress) e pipelines de CI/CD.

## High-Level Architecture

### System Overview

```
┌─────────────────────────────────────────────────────────────────┐
│                        Deep Analyzer CLI                         │
│                     (deep_analyzer_main.c)                       │
└────────────────────────────┬────────────────────────────────────┘
                             │
                             ▼
┌─────────────────────────────────────────────────────────────────┐
│                    Analysis Orchestrator                         │
│                  (deep_analyzer_orchestrator.c)                  │
│  • Parses CLI arguments                                          │
│  • Loads configuration                                           │
│  • Schedules analysis tasks                                      │
│  • Aggregates results                                            │
└────────────┬────────────┬────────────┬────────────┬─────────────┘
             │            │            │            │
             ▼            ▼            ▼            ▼
    ┌────────────┐ ┌────────────┐ ┌────────────┐ ┌────────────┐
    │ Invariant  │ │ Property   │ │  Pattern   │ │Conformance │
    │  Checker   │ │ Validator  │ │  Detector  │ │ Validator  │
    └────────────┘ └────────────┘ └────────────┘ └────────────┘
             │            │            │            │
             └────────────┴────────────┴────────────┘
                             │
                             ▼
                    ┌────────────────┐
                    │ Report Builder │
                    │ (JSON/MD/HTML) │
                    └────────────────┘
```

### Component Responsibilities

1. **Deep Analyzer CLI** (`deep_analyzer_main.c`)
   - Entry point for the tool
   - Parses command-line arguments
   - Invokes orchestrator with configuration

2. **Analysis Orchestrator** (`deep_analyzer_orchestrator.c`)
   - Coordinates all analysis components
   - Manages analysis scope (Frontend, Semantic, Backend, Runtime, Full)
   - Handles incremental analysis and caching
   - Aggregates results from all checkers

3. **Invariant Checker** (`invariant_checker.c`)
   - Validates compiler pipeline invariants
   - Checks lexer token spans, AST parent pointers, symbol table consistency
   - Validates HIR/ZIR transformations preserve semantics

4. **Property Validator** (`property_validator.c`)
   - Validates round-trip properties (parse/format, ZIR, JSON)
   - Validates idempotence properties (formatter)
   - Validates type system properties

5. **Pattern Detector** (`pattern_detector.c`)
   - Detects problematic code patterns
   - Identifies null pointer dereferences, memory leaks, race conditions
   - Uses static analysis and pattern matching

6. **Conformance Validator** (`conformance_validator.c`)
   - Validates implementation against specifications
   - Checks Code Maps accuracy
   - Validates test coverage

7. **Report Builder** (`report_builder.c`)
   - Generates structured reports (JSON, Markdown, HTML)
   - Includes bug candidates with severity levels
   - Provides actionable suggestions

### Data Flow

```
Input (Zenith codebase)
    │
    ▼
┌─────────────────────┐
│  File Discovery     │ → Scans compiler/, runtime/, language/stdlib/
└─────────────────────┘
    │
    ▼
┌─────────────────────┐
│  Cache Check        │ → Checks .ztc-tmp/deep-analysis/cache/
└─────────────────────┘
    │
    ▼
┌─────────────────────┐
│  Analysis Execution │ → Runs all enabled checkers
└─────────────────────┘
    │
    ▼
┌─────────────────────┐
│  Result Aggregation │ → Collects bug candidates
└─────────────────────┘
    │
    ▼
┌─────────────────────┐
│  Report Generation  │ → Outputs JSON/MD/HTML
└─────────────────────┘
    │
    ▼
Output (Analysis Report + Exit Code)
```

## Detailed Component Design

### 1. Deep Analyzer CLI

**File:** `tools/deep_analyzer/deep_analyzer_main.c`

**Interface:**
```c
// CLI entry point
int main(int argc, char** argv);

// Command-line options
typedef struct {
    AnalysisScope scope;           // Frontend, Semantic, Backend, Runtime, Full
    const char* output_format;     // json, markdown, html
    const char* output_path;       // Path to output report
    bool enable_cache;             // Enable incremental analysis
    bool verbose;                  // Verbose logging
    SeverityLevel min_severity;    // Minimum severity to report
    const char* config_file;       // Optional config file path
} DeepAnalyzerOptions;

// Parse CLI arguments
DeepAnalyzerOptions parse_arguments(int argc, char** argv);
```

**Usage Examples:**
```bash
# Full analysis with JSON report
./deep_analyzer --scope=full --format=json --output=report.json

# Frontend-only analysis with Markdown report
./deep_analyzer --scope=frontend --format=markdown --output=report.md

# Quick analysis (cached, critical bugs only)
./deep_analyzer --scope=full --cache --min-severity=critical

# Integration with test suite
python run_suite.py --suite=deep_analysis
```

### 2. Analysis Orchestrator

**File:** `tools/deep_analyzer/deep_analyzer_orchestrator.c`

**Interface:**
```c
// Main orchestration function
AnalysisResult run_deep_analysis(const DeepAnalyzerOptions* options);

// Analysis result structure
typedef struct {
    BugCandidate* bugs;           // Array of detected bugs
    size_t bug_count;
    AnalysisStatistics stats;     // Time, files analyzed, etc.
    bool has_critical_bugs;       // Exit code determination
} AnalysisResult;

// Analysis statistics
typedef struct {
    double total_time_seconds;
    size_t files_analyzed;
    size_t files_cached;
    size_t invariants_checked;
    size_t properties_validated;
    size_t patterns_detected;
} AnalysisStatistics;
```

**Orchestration Logic:**
```c
AnalysisResult run_deep_analysis(const DeepAnalyzerOptions* options) {
    // 1. Initialize components
    InvariantChecker* inv_checker = invariant_checker_create();
    PropertyValidator* prop_validator = property_validator_create();
    PatternDetector* pattern_detector = pattern_detector_create();
    ConformanceValidator* conf_validator = conformance_validator_create();
    
    // 2. Discover files based on scope
    FileList* files = discover_files(options->scope);
    
    // 3. Check cache and filter files
    if (options->enable_cache) {
        files = filter_cached_files(files);
    }
    
    // 4. Run analysis components
    BugCandidateList* bugs = bug_candidate_list_create();
    
    if (scope_includes_invariants(options->scope)) {
        run_invariant_checks(inv_checker, files, bugs);
    }
    
    if (scope_includes_properties(options->scope)) {
        run_property_validation(prop_validator, files, bugs);
    }
    
    if (scope_includes_patterns(options->scope)) {
        run_pattern_detection(pattern_detector, files, bugs);
    }
    
    if (scope_includes_conformance(options->scope)) {
        run_conformance_validation(conf_validator, files, bugs);
    }
    
    // 5. Aggregate results
    AnalysisResult result = aggregate_results(bugs, options);
    
    // 6. Update cache
    if (options->enable_cache) {
        update_cache(files, bugs);
    }
    
    return result;
}
```

### 3. Invariant Checker

**File:** `tools/deep_analyzer/invariant_checker.c`

**Interface:**
```c
// Invariant checker instance
typedef struct InvariantChecker InvariantChecker;

// Create/destroy
InvariantChecker* invariant_checker_create(void);
void invariant_checker_destroy(InvariantChecker* checker);

// Check invariants for a specific pipeline phase
void check_lexer_invariants(InvariantChecker* checker, const char* source_file, BugCandidateList* bugs);
void check_parser_invariants(InvariantChecker* checker, const char* source_file, BugCandidateList* bugs);
void check_semantic_invariants(InvariantChecker* checker, const char* source_file, BugCandidateList* bugs);
void check_hir_invariants(InvariantChecker* checker, const char* source_file, BugCandidateList* bugs);
void check_zir_invariants(InvariantChecker* checker, const char* source_file, BugCandidateList* bugs);
void check_backend_invariants(InvariantChecker* checker, const char* source_file, BugCandidateList* bugs);
void check_runtime_invariants(InvariantChecker* checker, const char* source_file, BugCandidateList* bugs);
```

**Invariant Checks:**

1. **Lexer Invariants:**
   - Token spans are contiguous (no gaps)
   - Token spans cover entire source file
   - Token types match lexer specification

2. **Parser Invariants:**
   - AST parent pointers are consistent
   - AST spans are properly nested
   - All nodes have valid source locations

3. **Semantic Invariants:**
   - No duplicate symbols in same scope
   - All symbol references are resolved
   - Type annotations are consistent

4. **HIR Invariants:**
   - HIR lowering preserves AST semantics
   - All HIR nodes have valid types
   - Control flow is well-formed

5. **ZIR Invariants:**
   - All instructions are type-safe
   - Register usage is valid
   - Control flow is valid

6. **Backend Invariants:**
   - No symbol name collisions in generated C
   - All emitted code is valid C
   - Memory management is correct

7. **Runtime Invariants:**
   - Reference counts are correct
   - No memory leaks in simple cases
   - Thread safety in concurrent operations

**Implementation Example:**
```c
void check_lexer_invariants(InvariantChecker* checker, const char* source_file, BugCandidateList* bugs) {
    // Lex the source file
    Lexer* lexer = lexer_create(source_file);
    Token* tokens = lexer_tokenize(lexer);
    
    // Check token span contiguity
    for (size_t i = 0; i < tokens->count - 1; i++) {
        Token* current = &tokens->data[i];
        Token* next = &tokens->data[i + 1];
        
        if (current->span.end != next->span.start) {
            BugCandidate bug = {
                .severity = SEVERITY_HIGH,
                .category = "Lexer Invariant Violation",
                .message = "Token spans are not contiguous",
                .file = source_file,
                .line = current->span.line,
                .column = current->span.column,
                .suggestion = "Check lexer implementation for span calculation"
            };
            bug_candidate_list_add(bugs, &bug);
        }
    }
    
    lexer_destroy(lexer);
}
```

### 4. Property Validator

**File:** `tools/deep_analyzer/property_validator.c`

**Interface:**
```c
// Property validator instance
typedef struct PropertyValidator PropertyValidator;

// Create/destroy
PropertyValidator* property_validator_create(void);
void property_validator_destroy(PropertyValidator* validator);

// Validate properties
void validate_roundtrip_parse_format(PropertyValidator* validator, const char* source_file, BugCandidateList* bugs);
void validate_roundtrip_zir(PropertyValidator* validator, const char* zir_file, BugCandidateList* bugs);
void validate_formatter_idempotence(PropertyValidator* validator, const char* source_file, BugCandidateList* bugs);
void validate_json_serialization(PropertyValidator* validator, BugCandidateList* bugs);
```

**Property Checks:**

1. **Round-Trip Parse/Format:**
   - Property: `parse(format(ast)) ≡ ast`
   - Test: Parse source → format AST → parse again → compare ASTs

2. **Round-Trip Format/Parse:**
   - Property: `format(parse(source))` preserves semantics
   - Test: Parse source → format → parse → compile → compare outputs

3. **Formatter Idempotence:**
   - Property: `format(format(code)) = format(code)`
   - Test: Format code → format again → compare outputs

4. **ZIR Round-Trip:**
   - Property: `parse_zir(print_zir(zir)) ≡ zir`
   - Test: Parse ZIR → pretty print → parse again → compare

5. **JSON Serialization:**
   - Property: `deserialize(serialize(value)) ≡ value`
   - Test: Serialize value → deserialize → compare

**Implementation Example:**
```c
void validate_formatter_idempotence(PropertyValidator* validator, const char* source_file, BugCandidateList* bugs) {
    // Format once
    char* formatted_once = format_file(source_file);
    
    // Format twice
    char* formatted_twice = format_string(formatted_once);
    
    // Compare
    if (strcmp(formatted_once, formatted_twice) != 0) {
        BugCandidate bug = {
            .severity = SEVERITY_CRITICAL,
            .category = "Formatter Idempotence Violation",
            .message = "Formatter is not idempotent",
            .file = source_file,
            .line = 0,
            .column = 0,
            .suggestion = "Check formatter implementation for oscillation"
        };
        bug_candidate_list_add(bugs, &bug);
    }
    
    free(formatted_once);
    free(formatted_twice);
}
```

### 5. Pattern Detector

**File:** `tools/deep_analyzer/pattern_detector.c`

**Interface:**
```c
// Pattern detector instance
typedef struct PatternDetector PatternDetector;

// Create/destroy
PatternDetector* pattern_detector_create(void);
void pattern_detector_destroy(PatternDetector* detector);

// Detect patterns
void detect_null_pointer_dereferences(PatternDetector* detector, const char* source_file, BugCandidateList* bugs);
void detect_memory_leaks(PatternDetector* detector, const char* source_file, BugCandidateList* bugs);
void detect_unbounded_recursion(PatternDetector* detector, const char* source_file, BugCandidateList* bugs);
void detect_buffer_overflows(PatternDetector* detector, const char* source_file, BugCandidateList* bugs);
void detect_integer_overflows(PatternDetector* detector, const char* source_file, BugCandidateList* bugs);
void detect_race_conditions(PatternDetector* detector, const char* source_file, BugCandidateList* bugs);
void detect_reference_cycles(PatternDetector* detector, const char* source_file, BugCandidateList* bugs);
```

**Pattern Detection Strategies:**

1. **Null Pointer Dereferences:**
   - Pattern: `ptr->field` without prior `if (ptr != NULL)`
   - Strategy: Data flow analysis tracking null checks

2. **Memory Leaks:**
   - Pattern: `malloc()` without corresponding `free()`
   - Strategy: Escape analysis and ownership tracking

3. **Unbounded Recursion:**
   - Pattern: Recursive function without depth limit
   - Strategy: Call graph analysis

4. **Buffer Overflows:**
   - Pattern: Array access without bounds check
   - Strategy: Range analysis

5. **Integer Overflows:**
   - Pattern: Arithmetic without overflow check
   - Strategy: Value range analysis

6. **Race Conditions:**
   - Pattern: Shared data access without synchronization
   - Strategy: Lockset analysis

7. **Reference Cycles:**
   - Pattern: Circular references in ARC
   - Strategy: Cycle detection in object graph

**Implementation Example:**
```c
void detect_null_pointer_dereferences(PatternDetector* detector, const char* source_file, BugCandidateList* bugs) {
    // Parse the C source file
    CParser* parser = c_parser_create(source_file);
    CAst* ast = c_parser_parse(parser);
    
    // Traverse AST looking for pointer dereferences
    for (size_t i = 0; i < ast->node_count; i++) {
        CAstNode* node = &ast->nodes[i];
        
        if (node->kind == C_AST_MEMBER_ACCESS && node->is_pointer_access) {
            // Check if there's a null check in the control flow
            if (!has_null_check_in_path(ast, node)) {
                BugCandidate bug = {
                    .severity = SEVERITY_HIGH,
                    .category = "Potential Null Pointer Dereference",
                    .message = "Pointer dereference without null check",
                    .file = source_file,
                    .line = node->line,
                    .column = node->column,
                    .suggestion = "Add null check before dereferencing pointer"
                };
                bug_candidate_list_add(bugs, &bug);
            }
        }
    }
    
    c_parser_destroy(parser);
}
```

### 6. Conformance Validator

**File:** `tools/deep_analyzer/conformance_validator.c`

**Interface:**
```c
// Conformance validator instance
typedef struct ConformanceValidator ConformanceValidator;

// Create/destroy
ConformanceValidator* conformance_validator_create(void);
void conformance_validator_destroy(ConformanceValidator* validator);

// Validate conformance
void validate_lexer_spec_conformance(ConformanceValidator* validator, BugCandidateList* bugs);
void validate_parser_spec_conformance(ConformanceValidator* validator, BugCandidateList* bugs);
void validate_type_system_conformance(ConformanceValidator* validator, BugCandidateList* bugs);
void validate_formatter_spec_conformance(ConformanceValidator* validator, BugCandidateList* bugs);
void validate_diagnostics_spec_conformance(ConformanceValidator* validator, BugCandidateList* bugs);
void validate_runtime_spec_conformance(ConformanceValidator* validator, BugCandidateList* bugs);
void validate_code_maps(ConformanceValidator* validator, BugCandidateList* bugs);
void validate_hardening_tests(ConformanceValidator* validator, BugCandidateList* bugs);
```

**Conformance Checks:**

1. **Lexer Spec Conformance:**
   - Check: All tokens in `language/spec/surface-syntax.md` are implemented
   - Strategy: Parse spec, extract token list, verify in lexer code

2. **Parser Spec Conformance:**
   - Check: All grammar rules are implemented
   - Strategy: Parse spec, extract grammar, verify in parser code

3. **Type System Conformance:**
   - Check: Type rules in `language/spec/type-system.md` are implemented
   - Strategy: Parse spec, extract type rules, verify in type checker

4. **Code Maps Validation:**
   - Check: All files/functions listed in Code Maps exist
   - Check: Line numbers are accurate
   - Strategy: Parse Code Maps, verify against actual code

5. **Hardening Tests Validation:**
   - Check: Determinism tests cover all pipeline phases
   - Check: Roundtrip tests cover all syntax constructs
   - Strategy: Analyze test suite coverage

**Implementation Example:**
```c
void validate_code_maps(ConformanceValidator* validator, BugCandidateList* bugs) {
    // Parse all CODE_MAP.md files
    CodeMap* maps[] = {
        parse_code_map("compiler/CODE_MAP.md"),
        parse_code_map("compiler/driver/DRIVER_MAP.md"),
        parse_code_map("compiler/frontend/FRONTEND_MAP.md"),
        // ... more maps
    };
    
    for (size_t i = 0; i < sizeof(maps) / sizeof(maps[0]); i++) {
        CodeMap* map = maps[i];
        
        // Check each file entry
        for (size_t j = 0; j < map->file_count; j++) {
            CodeMapFile* file = &map->files[j];
            
            // Check if file exists
            if (!file_exists(file->path)) {
                BugCandidate bug = {
                    .severity = SEVERITY_MEDIUM,
                    .category = "Code Map Outdated",
                    .message = "File listed in Code Map does not exist",
                    .file = map->path,
                    .line = file->line_in_map,
                    .column = 0,
                    .suggestion = "Update Code Map or restore missing file"
                };
                bug_candidate_list_add(bugs, &bug);
            }
            
            // Check each function entry
            for (size_t k = 0; k < file->function_count; k++) {
                CodeMapFunction* func = &file->functions[k];
                
                // Check if function exists at specified line
                if (!function_exists_at_line(file->path, func->name, func->line)) {
                    BugCandidate bug = {
                        .severity = SEVERITY_LOW,
                        .category = "Code Map Line Number Mismatch",
                        .message = "Function line number in Code Map is incorrect",
                        .file = map->path,
                        .line = func->line_in_map,
                        .column = 0,
                        .suggestion = "Update Code Map with correct line number"
                    };
                    bug_candidate_list_add(bugs, &bug);
                }
            }
        }
        
        code_map_destroy(map);
    }
}
```

### 7. Report Builder

**File:** `tools/deep_analyzer/report_builder.c`

**Interface:**
```c
// Report builder instance
typedef struct ReportBuilder ReportBuilder;

// Create/destroy
ReportBuilder* report_builder_create(void);
void report_builder_destroy(ReportBuilder* builder);

// Build reports in different formats
void build_json_report(ReportBuilder* builder, const AnalysisResult* result, const char* output_path);
void build_markdown_report(ReportBuilder* builder, const AnalysisResult* result, const char* output_path);
void build_html_report(ReportBuilder* builder, const AnalysisResult* result, const char* output_path);

// Parse existing reports
AnalysisResult* parse_json_report(const char* report_path);
```

**Report Structure (JSON):**
```json
{
  "version": "1.0",
  "timestamp": "2026-04-25T10:30:00Z",
  "analysis_scope": "full",
  "statistics": {
    "total_time_seconds": 45.2,
    "files_analyzed": 234,
    "files_cached": 180,
    "invariants_checked": 1523,
    "properties_validated": 456,
    "patterns_detected": 89
  },
  "bug_candidates": [
    {
      "id": "BUG-001",
      "severity": "critical",
      "category": "Formatter Idempotence Violation",
      "message": "Formatter is not idempotent for nested expressions",
      "file": "compiler/tooling/formatter.c",
      "line": 234,
      "column": 12,
      "suggestion": "Check expression formatting logic for oscillation",
      "code_snippet": "format_expression(expr->nested);",
      "references": [
        "language/spec/formatter-model.md#idempotence"
      ]
    }
  ],
  "summary": {
    "total_bugs": 12,
    "critical": 1,
    "high": 3,
    "medium": 5,
    "low": 3,
    "info": 0
  }
}
```

**Report Structure (Markdown):**
```markdown
# Deep Language Analysis Report

**Generated:** 2026-04-25 10:30:00  
**Scope:** Full  
**Duration:** 45.2 seconds

## Summary

- **Total Bugs:** 12
- **Critical:** 1
- **High:** 3
- **Medium:** 5
- **Low:** 3

## Statistics

- **Files Analyzed:** 234
- **Files Cached:** 180
- **Invariants Checked:** 1,523
- **Properties Validated:** 456
- **Patterns Detected:** 89

## Bug Candidates

### BUG-001: Formatter Idempotence Violation [CRITICAL]

**File:** `compiler/tooling/formatter.c:234:12`

**Message:** Formatter is not idempotent for nested expressions

**Code:**
```c
format_expression(expr->nested);
```

**Suggestion:** Check expression formatting logic for oscillation

**References:**
- [Formatter Model Spec](language/spec/formatter-model.md#idempotence)

---
```

**Report Structure (HTML):**
- Interactive report with navigation
- Filterable by severity
- Searchable by file/category
- Syntax-highlighted code snippets
- Links to specifications and Code Maps

## Data Structures

### Bug Candidate

```c
typedef enum {
    SEVERITY_CRITICAL,  // Blocks compilation or causes crashes
    SEVERITY_HIGH,      // Causes incorrect behavior
    SEVERITY_MEDIUM,    // Potential issue or non-conformance
    SEVERITY_LOW,       // Minor issue or style violation
    SEVERITY_INFO       // Informational finding
} SeverityLevel;

typedef struct {
    char* id;                    // Unique bug ID (e.g., "BUG-001")
    SeverityLevel severity;
    char* category;              // e.g., "Invariant Violation"
    char* message;               // Human-readable description
    char* file;                  // Source file path
    size_t line;
    size_t column;
    char* suggestion;            // Actionable fix suggestion
    char* code_snippet;          // Relevant code snippet
    char** references;           // Links to specs/docs
    size_t reference_count;
} BugCandidate;

typedef struct {
    BugCandidate* bugs;
    size_t count;
    size_t capacity;
} BugCandidateList;
```

### Analysis Scope

```c
typedef enum {
    SCOPE_FRONTEND   = 1 << 0,  // Lexer, parser, AST
    SCOPE_SEMANTIC   = 1 << 1,  // Binder, type system, symbols
    SCOPE_HIR        = 1 << 2,  // HIR lowering
    SCOPE_ZIR        = 1 << 3,  // ZIR generation and verification
    SCOPE_BACKEND    = 1 << 4,  // C emitter
    SCOPE_RUNTIME    = 1 << 5,  // Runtime C code
    SCOPE_STDLIB     = 1 << 6,  // Standard library
    SCOPE_FULL       = 0xFF     // All scopes
} AnalysisScope;
```

### Cache Entry

```c
typedef struct {
    char* file_path;
    char* content_hash;          // SHA-256 hash
    time_t last_modified;
    BugCandidate* cached_bugs;
    size_t cached_bug_count;
} CacheEntry;

typedef struct {
    CacheEntry* entries;
    size_t count;
    size_t capacity;
} AnalysisCache;
```

## Caching Strategy

### Cache Structure

```
.ztc-tmp/deep-analysis/cache/
├── cache_index.json          # Index of all cached files
├── frontend/
│   ├── lexer.c.cache         # Cached results for lexer.c
│   └── parser.c.cache
├── semantic/
│   └── binder.c.cache
└── ...
```

### Cache Index Format

```json
{
  "version": "1.0",
  "entries": [
    {
      "file": "compiler/frontend/lexer/lexer.c",
      "hash": "a1b2c3d4e5f6...",
      "last_modified": "2026-04-25T09:00:00Z",
      "cache_file": ".ztc-tmp/deep-analysis/cache/frontend/lexer.c.cache",
      "bug_count": 0
    }
  ]
}
```

### Cache Invalidation

1. **Content-based:** Recompute SHA-256 hash, compare with cached hash
2. **Time-based:** Check file modification time
3. **Dependency-based:** Invalidate if dependencies changed (future enhancement)

### Cache Update

```c
void update_cache(const FileList* files, const BugCandidateList* bugs) {
    AnalysisCache* cache = load_cache();
    
    for (size_t i = 0; i < files->count; i++) {
        const char* file = files->paths[i];
        
        // Compute hash
        char* hash = compute_sha256(file);
        
        // Find bugs for this file
        BugCandidate* file_bugs = filter_bugs_by_file(bugs, file);
        size_t file_bug_count = count_bugs(file_bugs);
        
        // Update cache entry
        CacheEntry entry = {
            .file_path = strdup(file),
            .content_hash = hash,
            .last_modified = get_file_mtime(file),
            .cached_bugs = file_bugs,
            .cached_bug_count = file_bug_count
        };
        
        cache_add_or_update(cache, &entry);
    }
    
    save_cache(cache);
    cache_destroy(cache);
}
```

## Integration with Test Suites

### CLI Integration

The Deep Analyzer integrates with `run_suite.py`:

```python
# In run_suite.py
SUITES = {
    # ... existing suites
    'deep_analysis': {
        'command': './tools/deep_analyzer/deep_analyzer',
        'args': ['--scope=full', '--format=json', '--output=.ztc-tmp/deep-analysis/report.json'],
        'timeout': 1800,  # 30 minutes
        'critical': True   # Blocks PR if fails
    },
    'deep_analysis_quick': {
        'command': './tools/deep_analyzer/deep_analyzer',
        'args': ['--scope=frontend,semantic', '--cache', '--min-severity=high'],
        'timeout': 300,   # 5 minutes
        'critical': False
    }
}
```

### CI/CD Integration

**GitHub Actions Workflow:**

```yaml
name: Deep Language Analysis

on:
  pull_request:
    branches: [ main ]
  schedule:
    - cron: '0 2 * * *'  # Nightly at 2 AM