# TTL Organization Strategies

The parser supports TWO organizational strategies. Choose based on project complexity:

## Flat Structure (Simple Projects)

Use flat structure when:
- Phases have fewer than 10 tasks
- Tasks within a phase are conceptually similar
- No clear sub-groupings exist

**Example:**
```markdown
## Phase 1: Setup

**Goal**: Establish project foundation

| Task ID | Description | SDD Ref | PRD Ref |
|---------|-------------|---------|---------|
| T-1.1   | Create project structure | §8 | - |
| T-1.2   | Setup dev environment | §8 | - |
| T-1.3   | Write setup tests | §10 | - |
```

## Hierarchical Structure (Complex Projects)

Use hierarchical structure when:
- Phases have 10+ tasks
- Tasks naturally group into categories (e.g., Setup, Implementation, Testing)
- You want to improve readability and organization

**Example:**
```markdown
## Phase 1: Foundation and Core Features

**Goal**: Establish foundation and implement core functionality

### Infrastructure Setup
| Task ID | Description | SDD Ref | PRD Ref |
|---------|-------------|---------|---------|
| T-1.1   | Create project structure | §8 | - |
| T-1.2   | Setup development environment | §8 | - |

### Core Implementation
| Task ID | Description | SDD Ref | PRD Ref |
|---------|-------------|---------|---------|
| T-1.3   | Implement main module | §4.1 | §3.1 |
| T-1.4   | Add error handling | §4.2 | §3.1 |

### Testing
| Task ID | Description | SDD Ref | PRD Ref |
|---------|-------------|---------|---------|
| T-1.5   | Test project setup | §10 | - |
| T-1.6   | Test main module | §10 | §3.1 |
```

## Hierarchical Structure Rules

**CRITICAL RULES for Hierarchical Structure:**
- Task group headers MUST be exactly ONE level deeper than phase headers
- If phases use `##`, task groups must use `###`
- If phases use `###`, task groups must use `####`
- Task group names are flexible (e.g., "Setup", "Implementation", "Testing", "Component A", "Module B")
- Task IDs remain T-X.Y format (NOT T-X.Y.Z) - they still sequence across all groups in the phase
- Task sequence validation applies across ALL groups within a phase (T-1.1, T-1.2, T-1.3... regardless of which group they're in)

**CORRECT STRUCTURE:**
```markdown
## Phase 1: Implementation        ← Phase at level ##

### Infrastructure Setup          ← Task group at level ### (one deeper)
| Task ID | Description | SDD Ref | PRD Ref |
|---------|-------------|---------|---------|
| T-1.1   | Task one | - | - |

### Core Features                 ← Task group at level ### (same as other groups)
| Task ID | Description | SDD Ref | PRD Ref |
|---------|-------------|---------|---------|
| T-1.2   | Task two | - | - |
```

**INCORRECT Hierarchical Usage (will fail validation):**
```markdown
## Phase 1: Implementation        ← Phase at level ##
### Component A                   ← Task group at ### (correct)
#### Sub-component A.1            ← INVALID! Too deep - no sub-sub-groups

## Phase 1: Another Section       ← INVALID! Duplicate phase number
```

## Common Structure Validation Errors

**ABSOLUTELY FORBIDDEN - THESE WILL CAUSE VALIDATION FAILURES**:
- ❌ **Mixed phase header levels** (e.g., `## Phase 1:` and `### Phase 2:` in the same document - all phases must use the same level)
- ❌ **Duplicate phase numbers** (e.g., two headers both containing "Phase 1:")
- ❌ **Decimal numbering in phase numbers** (e.g., "## Phase 1.1:" is invalid - use "## Phase 1:" or "## Phase 2:")
- ❌ **Inconsistent task group levels** (if using task groups, all must be exactly one level deeper than phases)

## Alternative Accepted Formats

Any consistent header level works:

```markdown
## Sprint 1: Initial Setup
## Iteration 1: Core Features
## Milestone 1: MVP Release
```

Or with a different level (just be consistent):
```markdown
### Stage 1: Infrastructure
### Stage 2: Core Features
### Stage 3: Integration
```

## ⚠️ CRITICAL WARNING: Orphan Task Tables

**MOST COMMON MISTAKE** that causes parsing failures:

**ORPHAN TABLE**: A task table placed directly under a phase header when task groups also exist in that phase.

**Example of INVALID structure (will fail validation):**
```markdown
## Phase 4: Polish and Documentation

**Goal**: Finalize documentation and testing

| Task ID | Description | SDD Ref | PRD Ref |          ← ORPHAN TABLE (WRONG!)
|---------|-------------|---------|---------|
| T-4.1   | Add docstrings | §7 | - |
| T-4.2   | Create README | §9 | - |

### Task Group 4.1: Documentation                 ← Task groups exist!

| Task ID | Description | SDD Ref | PRD Ref |          ← Valid table
|---------|-------------|---------|---------|
| T-4.1   | Add docstrings | §7 | - |              ← DUPLICATE IDs!
| T-4.2   | Create README | §9 | - |
```

**Why this fails:**
- Parser sees task groups exist, so expects ALL tables under task groups
- The orphan table at the top creates duplicate task IDs
- Results in "No task table found in phase" or "Duplicate task ID" errors

**THE FIX - Choose ONE of these:**

**Option 1: Use ONLY task groups (DELETE the orphan table):**
```markdown
## Phase 4: Polish and Documentation

**Goal**: Finalize documentation and testing

### Task Group 4.1: Documentation

| Task ID | Description | SDD Ref | PRD Ref |
|---------|-------------|---------|---------|
| T-4.1   | Add docstrings | §7 | - |
| T-4.2   | Create README | §9 | - |

### Task Group 4.2: Testing

| Task ID | Description | SDD Ref | PRD Ref |
|---------|-------------|---------|---------|
| T-4.3   | Run full test suite | §10 | - |
```

**Option 2: Use flat structure (DELETE all task group headers):**
```markdown
## Phase 4: Polish and Documentation

**Goal**: Finalize documentation and testing

| Task ID | Description | SDD Ref | PRD Ref |
|---------|-------------|---------|---------|
| T-4.1   | Add docstrings | §7 | - |
| T-4.2   | Create README | §9 | - |
| T-4.3   | Run full test suite | §10 | - |
```

## Choosing Between Flat and Hierarchical

**Use Flat Structure when:**
- Simple, small phases (< 10 tasks)
- Tasks are all similar type of work
- No natural groupings emerge

**Use Hierarchical Structure when:**
- Complex phases (10+ tasks)
- Clear task categories exist (Setup, Implementation, Testing)
- You want improved navigation and readability
- Tasks naturally group by component or concern

**Important**: Once you choose hierarchical, ALL tasks in that phase must be under task groups. No mixing!
