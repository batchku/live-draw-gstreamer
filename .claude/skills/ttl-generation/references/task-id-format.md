# Task ID Format Reference

## CRITICAL REQUIREMENT - THIS MUST BE FOLLOWED EXACTLY

**REQUIRED Format:** `T-X.Y` where:
- X = phase number (1, 2, 3, ...)
- Y = sequential task number within phase (1, 2, 3, ...)

**Valid Examples:** `T-1.1`, `T-1.2`, `T-2.1`, `T-3.15`, `T-1.25`

**Pattern:** Must match exactly `\bT-\d+\.\d+\b` (only TWO numeric parts)

## Validation Rules

**ABSOLUTELY FORBIDDEN - THESE WILL CAUSE VALIDATION FAILURES**:
- ❌ **Three-level task IDs** (e.g., "T-1.1.1" is INVALID - always use T-X.Y even with task groups)
- ❌ **Decimal phase numbers in IDs** (e.g., "T-1.5.1" suggests Phase 1.5 - phases must be whole numbers)
- ❌ **Duplicate task IDs** (each task ID must be unique across the entire document)

## Enforcement

**Before saving the TTL**, you MUST verify that ALL task IDs in ALL tables use exactly the T-X.Y format with NO three-level IDs.

## Task ID Sequencing

Within each phase, task IDs must:
1. Match the phase number (T-1.x for Phase 1, T-2.x for Phase 2)
2. Be sequential (T-1.1, T-1.2, T-1.3... with no gaps)
3. Be unique (no duplicate IDs)
4. Follow table order (tasks listed in execution order)

**Example:**
```markdown
## Phase 1: Setup

| Task ID | Description | SDD Ref | PRD Ref |
|---------|-------------|---------|---------|
| T-1.1   | First task | - | - |
| T-1.2   | Second task | - | - |
| T-1.3   | Third task | - | - |

## Phase 2: Implementation

| Task ID | Description | SDD Ref | PRD Ref |
|---------|-------------|---------|---------|
| T-2.1   | First task of phase 2 | - | - |
| T-2.2   | Second task of phase 2 | - | - |
```

## Task IDs with Hierarchical Structure

**CRITICAL**: Even when using task groups (hierarchical structure), task IDs remain T-X.Y format.

**CORRECT (with task groups):**
```markdown
## Phase 1: Implementation

### Infrastructure Setup
| Task ID | Description | SDD Ref | PRD Ref |
|---------|-------------|---------|---------|
| T-1.1   | Setup task one | - | - |

### Core Features
| Task ID | Description | SDD Ref | PRD Ref |
|---------|-------------|---------|---------|
| T-1.2   | Implementation task two | - | - |
```

**INCORRECT (will fail validation):**
```markdown
## Phase 1: Implementation

### Infrastructure Setup
| Task ID | Description | SDD Ref | PRD Ref |
|---------|-------------|---------|---------|
| T-1.1.1 | ❌ WRONG - three levels | - | - |
```

Task IDs sequence across ALL groups within a phase (T-1.1, T-1.2, T-1.3...) regardless of which task group they're in.
