---
name: ttl-generation
description: Generates Technical Task Lists (TTLs) by analyzing Software Design Documents (SDDs) and Product Requirements Documents (PRDs) to create detailed, trackable implementation task breakdowns. Use when converting design specifications into actionable development tasks, creating phased implementation plans, defining task dependencies and sequencing, breaking down complex systems into manageable work units, or establishing milestones with testing boundaries. Supports MVP-first vertical slice delivery.
allowed-tools: Read Write Grep Glob
---

## When to Use This Skill

Use this skill when you need to:
- Convert an SDD into an executable task breakdown
- Create a phased implementation plan for a project
- Define task sequencing and execution order
- Break down complex systems into manageable work units
- Create test tasks for each implementation phase
- Establish clear milestones and deliverables

## Purpose

This skill provides a systematic approach to TTL creation by:
1. Reading and analyzing the SDD and PRD
2. Identifying all components and features to implement
3. Breaking down implementation into logical phases
4. Creating specific, actionable tasks for each component
5. Creating test tasks for each implementation phase
6. Organizing tasks under phase headers (flat) or task groups (hierarchical) based on complexity

## Quick Start

### 1. Read the Phase Template (MANDATORY)

**FIRST STEP**: Before generating any TTL content, read the phase template:

```
templates/phase-template.md
```

This template provides the exact structure you must follow for every phase. Copy it exactly and fill in the placeholders.

### 2. Generate the TTL

Follow this high-level workflow:
1. Read SDD and PRD to understand requirements
2. Identify logical phases (MVP-first approach)
3. Extract all components from SDD
4. For each phase, apply the phase template
5. Create specific, actionable tasks in execution order
6. Add test tasks for each phase
7. Link tasks to SDD/PRD sections
8. Validate structure and format

### 3. Key Structure Requirements

**Phase Header Format:**
```markdown
## Phase N: Phase Name

**Goal**: Description of runnable increment delivered by this phase
```

**Task Table Format (exactly 4 columns):**
```markdown
| Task ID | Description | SDD Ref | PRD Ref |
|---------|-------------|---------|---------|
| T-N.1   | Specific task description | §X.Y | §A.B |
| T-N.2   | Another task | §X.Z | - |
```

**Task ID Format:**
- Required: `T-X.Y` (e.g., T-1.1, T-2.5)
- X = phase number
- Y = sequential task number within phase
- Never use three-level IDs (T-X.Y.Z is invalid)

## Output Format

The TTL is a **pure markdown document**. The task ledger JSON is generated automatically by parsing the markdown structure.

## Critical Rules (Validation Will Fail Without These)

1. **Phase Template**: Always read and use `templates/phase-template.md` for every phase
2. **Consistent Headers**: All phase headers must use the same markdown level (e.g., all `##`)
3. **Task ID Format**: Always use `T-X.Y` format, never `T-X.Y.Z`
4. **Exact Table Format**: Task tables must have exactly 4 columns with exact names
5. **Separator Row**: Must appear immediately after table header
6. **Unique IDs**: No duplicate phase numbers or task IDs
7. **MVP-First**: Each phase must end with a runnable, testable increment
8. **Sequential IDs**: Task IDs must be sequential within each phase (no gaps)

## Organization Strategies

Choose based on phase complexity:

**Flat Structure** (< 10 tasks per phase):
- Tasks directly under phase header
- Simple, straightforward phases

**Hierarchical Structure** (10+ tasks per phase):
- Tasks organized under task group headers
- Task groups exactly one level deeper than phases
- No orphan tables (if using groups, ALL tasks must be in groups)

See `references/organizational-strategies.md` for detailed guidance and examples.

## Reference Documentation

For detailed information, see the reference files:

- **`references/ttl-structure.md`** - Complete structural requirements, phase headers, task tables, formatting rules, quality standards
- **`references/task-id-format.md`** - Task ID format rules, validation requirements, sequencing, examples
- **`references/organizational-strategies.md`** - Flat vs hierarchical structure, task groups, orphan table warnings, common errors
- **`references/mvp-phasing.md`** - MVP-first strategy, phase completion criteria, vertical slices, anti-patterns, examples by project type
- **`references/generation-workflow.md`** - Complete step-by-step generation process, from reading template to final validation
- **`references/validation-checklist.md`** - Pre-save validation checklist, common errors, quick validation commands, error messages

## MVP-First Requirement

**Every phase MUST end with a runnable increment:**
- A working application that executes without errors
- Demonstrable features showing visible progress
- Passing tests verifying functionality
- Clear evidence the software works

Example Phase 1 MVP: An executable program that runs successfully and exits with code 0.

See `references/mvp-phasing.md` for detailed examples and anti-patterns.

## Test Tasks

Test tasks are automatically identified by having "test" in the task description. The system assigns these to test engineers automatically.

**Example:**
```markdown
| T-1.5 | Test application launches without errors | §10.2 | - |
| T-2.3 | Test user login with valid credentials | §10.3 | §3.2 |
```

## Generation Workflow Summary

1. **Read phase template** (`templates/phase-template.md`) - MANDATORY FIRST STEP
2. **Read SDD and PRD** - understand all components and requirements
3. **Identify phases** - group work into MVP-first increments
4. **Choose organization strategy** - flat or hierarchical per phase
5. **Apply phase template** - copy structure exactly for each phase
6. **Create tasks** - specific, actionable, in execution order
7. **Add test tasks** - verify each phase works
8. **Add references** - link to SDD/PRD sections (use `-` if none)
9. **Validate structure** - run through validation checklist
10. **Save and confirm** - ensure no validation errors

See `references/generation-workflow.md` for detailed step-by-step instructions.

## Common Pitfalls to Avoid

1. **Skipping phase template** - ALWAYS read `templates/phase-template.md` first
2. **Three-level task IDs** - Use T-X.Y, never T-X.Y.Z
3. **Mixed phase header levels** - All phases must use same level
4. **Orphan tables** - With task groups, ALL tables must be under groups
5. **Extra table columns** - Exactly 4 columns required
6. **Non-MVP phases** - Each phase must end with runnable increment
7. **Missing test tasks** - Every phase needs test tasks
8. **Vague descriptions** - Be specific about deliverables

## Validation Before Saving

Before saving the TTL, verify:
- [ ] Phase template followed for all phases
- [ ] All phase headers at same level
- [ ] All task IDs in T-X.Y format
- [ ] No duplicate phase numbers or task IDs
- [ ] Task tables have exactly 4 columns
- [ ] Separator rows present
- [ ] No orphan tables if using task groups
- [ ] Each phase ends with runnable increment
- [ ] Test tasks present in each phase

See `references/validation-checklist.md` for complete checklist.

## Example Phase Structure

**Using Phase Template:**
```markdown
## Phase 1: Minimal Working Application

**Goal**: Create the simplest possible working application that executes successfully.

| Task ID | Description | SDD Ref | PRD Ref |
|---------|-------------|---------|---------|
| T-1.1 | Create project directory structure | §8 | - |
| T-1.2 | Create main entry point with basic execution | §4.1 | §3.1 |
| T-1.3 | Implement minimal startup routine that exits cleanly | §4.1 | §3.1 |
| T-1.4 | Add basic configuration loader | §5 | - |
| T-1.5 | Create test framework setup | §10 | - |
| T-1.6 | Test application launches without errors | §10.2 | - |
| T-1.7 | Test application exits with code 0 | §10.2 | - |
```

## Getting Help

If you encounter issues:
1. Check the specific reference file for detailed guidance
2. Review `references/validation-checklist.md` for common errors
3. Verify you've read and applied the phase template
4. Ensure MVP-first requirement is met for each phase
5. Validate task ID format (T-X.Y only)

## Quality Standards

Your TTL should have:
- **Clear tasks**: Specific, actionable descriptions with deliverables
- **Complete coverage**: All SDD components have implementation tasks
- **Logical sequencing**: Tasks in execution order (dependencies first)
- **Traceability**: Tasks linked to SDD/PRD sections
- **Testability**: Test tasks for each phase
- **MVP compliance**: Each phase delivers runnable increment

The TTL serves as the implementation blueprint. Invest time in making it clear, complete, and correct.
