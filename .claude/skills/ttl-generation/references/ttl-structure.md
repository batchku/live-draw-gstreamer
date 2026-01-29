# TTL Structure Reference

## Output Format

The TTL is a **pure markdown document**. No embedded JSON is required - the ledger is generated automatically by parsing the markdown structure.

## Document Header

Start with a clear title and project metadata:

```markdown
# Technical Task List (TTL): Project Name
```

## Overview Section

```markdown
## Document Overview

This Technical Task List provides a comprehensive breakdown of all implementation tasks.

### Organization Principles

- **Phased Approach**: Tasks organized into project-appropriate phases (count and naming chosen based on scope; phases execute sequentially)
- **Test-Driven**: Unit and integration tests at each phase
- **Sequential Order**: Within each phase, tasks execute in TTL table order (top to bottom)
```

## Phase Structure

**CRITICAL RULE - CONSISTENCY REQUIRED**: All phase headers must use the SAME markdown header level (e.g., all `##` or all `###`). The specific level doesn't matter, but inconsistency will cause validation failures.

- **Phase headers**: `## Phase N: Phase Name` (or synonyms: Sprint, Iteration, Milestone, Stage, Deliverable, Release, Increment, Work Package)
- **Accepted levels**: `#`, `##`, `###`, etc. - just be consistent across ALL phases

Phase count, phase names, and goal content are project-specific; choose them based on the SDD/PRD. The required structure still includes the header format, Goal line, and task tables.

### Phase Header Format

**Flexible formats accepted:**
- Synonyms accepted: Phase, Sprint, Iteration, Milestone, Stage, Task Group
- Separators accepted: colon, dash, or space
- Examples: `## Phase 1: Setup`, `## Sprint 1 - Core`, `## Task Group 2 Implementation`

**Example phase with tasks:**
```markdown
## Phase 1: Project Foundation

**Goal**: Establish project foundation

| Task ID | Description | SDD Ref | PRD Ref |
|---------|-------------|---------|---------|
| T-1.1 | Create project directory structure | §8 | - |
| T-1.2 | Create main entry point file | §4.1 | §3.1 |
| T-1.3 | Add configuration files | §5 | - |
| T-1.4 | Create test framework setup | §10 | - |
| T-1.5 | Write unit tests for core modules | §10.2 | - |
```

## Task Tables

### CRITICAL: Exact Table Header Format

**The task table header MUST be exactly this format (case-insensitive but columns must match):**

```markdown
| Task ID | Description | SDD Ref | PRD Ref |
|---------|-------------|---------|---------|
```

### MANDATORY Phase Template

**STRICT REQUIREMENT**: Before generating any TTL, you MUST read the phase template file:

```
templates/phase-template.md
```

**ENFORCEMENT RULES:**

1. **READ FIRST**: Always read `templates/phase-template.md` before generating any phase
2. **COPY EXACTLY**: For EVERY phase, copy the template structure exactly and fill in the placeholders
3. **NO DEVIATIONS**: Do not add columns, change column order, or modify the table format
4. **FILL ALL PHASES**: Apply the template to every single phase in the TTL without exception

**VALIDATION RULES:**
1. **Exactly 4 columns**: Task ID, Description, SDD Ref, PRD Ref
2. **No extra columns** - Adding "Status", "Priority", or other columns will cause validation failure
3. **Column names must match exactly** - "Task ID" not "ID", "SDD Ref" not "SDD Reference"
4. **Separator row required** - Must have `|---|---|---|---|` immediately after header

**INVALID TABLE FORMATS (will fail validation):**
```markdown
❌ | Task ID | Description | SDD Ref | PRD Ref | Status |   <-- Extra column!
❌ | ID | Desc | SDD | PRD |                              <-- Wrong column names!
❌ | Task ID | Description | SDD Ref | PRD Ref |
   | T-1.1 | ...                                          <-- Missing separator row!
```

### Execution Order

Tasks execute sequentially within each phase in TTL table order (top to bottom).
Place tasks in the exact order they should run.

**CORRECT Example:**
```markdown
| Task ID | Description | SDD Ref | PRD Ref |
|---------|-------------|---------|---------|
| T-1.1 | Create project structure | §8 | - |
| T-1.2 | Create main entry point | §4.1 | §3.1 |
| T-1.3 | Add configuration loader | §5 | - |
| T-1.4 | Implement core logic | §4.2 | §3.2 |
| T-1.5 | Test core logic | §10.2 | - |
```

## Test Task Identification

Test tasks are identified by having "test" in the task description. The system automatically assigns these tasks to the appropriate testing agent.

```markdown
## Phase 2: Core Logic Implementation

**Goal**: Implement and test core functionality

| Task ID | Description | SDD Ref | PRD Ref |
|---------|-------------|---------|---------|
| T-2.1 | Implement input validation | §5.1 | §3.3 |
| T-2.2 | Implement error handling | §5.2 | §3.4 |
| T-2.3 | Test input validation with automated assertions | §10.2 | - |
| T-2.4 | Test error handling with automated assertions | §10.2 | - |
```

## Quality Standards

### Task Quality
- Task descriptions are specific and actionable
- Each task has a clear deliverable
- File paths specified where applicable

### Phase Coverage
- All SDD components have implementation tasks
- Each phase should ideally have both implementation and test tasks

### Traceability
- Tasks reference SDD sections where applicable
- Tasks reference PRD requirements where applicable
- Clear mapping from requirements to implementation

## Minimum Requirements

- **Phase headers** with phase number and name (e.g., `## Phase 1: Project Setup`)
- **Task tables** with 4-column format: `| Task ID | Description | SDD Ref | PRD Ref |`
- **Task IDs** in T-X.Y format matching phase numbers (T-1.1, T-1.2 for Phase 1)
- **Unique task IDs** (no duplicates)
