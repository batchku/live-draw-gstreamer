# TTL Validation Checklist

Before generating or saving the TTL artifact, the agent MUST complete this final self-check:

## Required (Validation Will Fail Without These)

- [ ] **Phase headers** with number and name (e.g., `## Phase 1: Setup`)
- [ ] **Consistent header levels** for all phases (all `##` or all `###`, no mixing)
- [ ] **No duplicate phase numbers** (each phase has unique number)
- [ ] **Goal statement** immediately after each phase header (format: `**Goal**: description`)
- [ ] **Task tables** with 4-column format: `| Task ID | Description | SDD Ref | PRD Ref |`
- [ ] **Separator row** immediately after each table header (format: `|---|---|---|---|`)
- [ ] **Task IDs** in `T-X.Y` format matching phase number (T-1.x for Phase 1)
- [ ] **No three-level task IDs** (e.g., T-1.1.1 is invalid)
- [ ] **Unique task IDs** - no duplicates across entire document
- [ ] **Non-empty descriptions** for each task
- [ ] **Sequential task IDs** within each phase (T-1.1, T-1.2, T-1.3 with no gaps)
- [ ] **SDD Ref and PRD Ref columns present** (use `-` for empty references)

## Structural Validation

- [ ] **All phases at same header level** (consistent throughout document)
- [ ] **If using task groups**: all task groups exactly one level deeper than phases
- [ ] **If using task groups**: no orphan tables (tables directly under phase when groups exist)
- [ ] **If using task groups**: task IDs still use T-X.Y format (not T-X.Y.Z)
- [ ] **If using flat structure**: no task group headers in the phase
- [ ] **Phase template followed** for every single phase (read from `templates/phase-template.md`)

## Content Validation

- [ ] **All SDD components have implementation tasks** (complete coverage)
- [ ] **Each phase includes test tasks** (identified by "test" in description)
- [ ] **Task descriptions are specific and actionable** (not vague)
- [ ] **Tasks in execution order** (top to bottom in table)
- [ ] **SDD references accurate** (e.g., §4.1, §5.2)
- [ ] **PRD references accurate** (e.g., §3.1, §2.4)
- [ ] **Each phase ends with runnable increment** (MVP-first requirement)

## Common Errors to Check

### Phase Header Errors
- ❌ Mixed header levels (e.g., `## Phase 1:` and `### Phase 2:`)
- ❌ Duplicate phase numbers (e.g., two "Phase 1" headers)
- ❌ Decimal phase numbers (e.g., "Phase 1.1" instead of "Phase 1" or "Phase 2")
- ❌ Missing colon or separator after phase number

### Task Table Errors
- ❌ Extra columns (e.g., Status, Priority, Assignee)
- ❌ Wrong column names (e.g., "ID" instead of "Task ID")
- ❌ Missing separator row after header
- ❌ Malformed table borders (missing pipes)

### Task ID Errors
- ❌ Three-level IDs (e.g., T-1.1.1)
- ❌ Task IDs not matching phase number (e.g., T-2.1 in Phase 1)
- ❌ Duplicate task IDs across document
- ❌ Non-sequential IDs (e.g., T-1.1, T-1.3, missing T-1.2)
- ❌ Gaps in task numbering

### Hierarchical Structure Errors
- ❌ Orphan tables (table under phase header when task groups exist)
- ❌ Task groups not exactly one level deeper than phases
- ❌ Mixed structure (some tasks in groups, some directly under phase)
- ❌ Sub-sub-groups (e.g., #### under ### task groups)

### Content Errors
- ❌ Empty task descriptions
- ❌ Missing SDD/PRD references (should use `-` if none)
- ❌ No test tasks in phase
- ❌ Phase doesn't end with runnable increment
- ❌ Vague task descriptions (e.g., "Implement feature")

## Pre-Save Verification Steps

1. **Read the entire generated TTL document**
2. **Count phases** and verify sequential numbering (1, 2, 3...)
3. **Check all task IDs** match pattern `T-X.Y` (no T-X.Y.Z)
4. **Verify all phase headers at same level** (count `#` symbols)
5. **If task groups exist**: verify all are exactly one level deeper
6. **Check for orphan tables** (tables under phase when groups exist)
7. **Verify task ID uniqueness** (no duplicates)
8. **Check task sequencing** (T-1.1, T-1.2, T-1.3... in order)
9. **Confirm test tasks exist** (look for "test" in descriptions)
10. **Validate MVP-first compliance** (each phase ends with runnable increment)

## Quick Validation Commands

After generating the TTL, you can use these checks:

```bash
# Check for three-level task IDs (should return nothing)
grep -E 'T-[0-9]+\.[0-9]+\.[0-9]+' TTL.md

# Count phases
grep -E '^##+ Phase [0-9]+:' TTL.md | wc -l

# Check phase header consistency (all should have same # count)
grep -E '^##+ Phase [0-9]+:' TTL.md

# Find duplicate task IDs
grep -oE 'T-[0-9]+\.[0-9]+' TTL.md | sort | uniq -d
```

## Error Messages and Fixes

| Error Message | Likely Cause | Fix |
|---------------|-------------|-----|
| "Mixed phase header levels" | Some phases use `##`, others use `###` | Make all phases use same level |
| "Duplicate phase number" | Two phases both labeled "Phase 1" | Renumber phases sequentially |
| "Invalid task ID format" | Task ID is T-1.1.1 or similar | Change to T-X.Y format |
| "Orphan task table" | Table directly under phase when groups exist | Move table under a task group |
| "Missing separator row" | No `\|---\|` row after header | Add separator row |
| "No task table found" | Phase missing table or table malformed | Check table structure |

## Final Check Before Completion

Before marking TTL generation as complete:

1. ✅ Document passes all required validation checks
2. ✅ Document passes all structural validation checks
3. ✅ Document passes all content validation checks
4. ✅ No validation errors when parsed
5. ✅ All phases follow phase template exactly
6. ✅ MVP-first requirement met for every phase

If ANY check fails, fix the issue before saving the TTL.
