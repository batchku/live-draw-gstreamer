# TTL Generation Workflow

Follow this step-by-step process to generate a complete, valid TTL:

## Step 1: Read the Phase Template

**MANDATORY FIRST STEP**: Read the phase template before generating any content.

```
templates/phase-template.md
```

This template provides the exact structure you must follow for every phase.

## Step 2: Read and Analyze Input Documents

Read the SDD and PRD to understand:
- All components that need implementation
- System architecture and design
- Feature requirements and priorities
- Testing requirements
- Technology stack and constraints

**Key questions to answer:**
- What components are defined in the SDD?
- What features are required by the PRD?
- What is the logical implementation order?
- How can work be broken into MVPs?
- What testing is required at each stage?

## Step 3: Identify Phases

Group related work into logical phases based on:
- **MVP-first principle**: Each phase must end with runnable increment
- **Vertical slices**: Complete features end-to-end
- **Dependencies**: Earlier phases enable later phases
- **Complexity**: Balance phase size (not too large, not too small)
- **Testing**: Each phase includes tests for its work

**Example phase breakdown:**
1. Phase 1: Minimal working application (proves it runs)
2. Phase 2: Core feature MVP (one complete feature)
3. Phase 3: Additional features (build on working foundation)
4. Phase 4: Polish and documentation (improve what works)

## Step 4: Choose Organization Strategy

For each phase, decide between:

**Flat structure** (< 10 tasks, conceptually similar):
```markdown
## Phase 1: Setup

**Goal**: ...

| Task ID | Description | SDD Ref | PRD Ref |
|---------|-------------|---------|---------|
| T-1.1   | ... | - | - |
```

**Hierarchical structure** (10+ tasks, clear groupings):
```markdown
## Phase 1: Setup

**Goal**: ...

### Infrastructure
| Task ID | Description | SDD Ref | PRD Ref |
|---------|-------------|---------|---------|
| T-1.1   | ... | - | - |

### Testing
| Task ID | Description | SDD Ref | PRD Ref |
|---------|-------------|---------|---------|
| T-1.2   | ... | - | - |
```

See `references/organizational-strategies.md` for detailed guidance.

## Step 5: Extract Components from SDD

List all components mentioned in the SDD that require implementation:
- Modules and classes
- APIs and endpoints
- Database schemas
- Configuration systems
- Utilities and helpers
- External integrations

Map each component to an SDD section reference (e.g., §4.1, §5.2).

## Step 6: Create Phase Structure

For each phase, apply the phase template:

1. **Copy the phase template structure exactly**
2. **Fill in phase number and name**
   - Example: `## Phase 1: Minimal Working Application`
3. **Write the Goal statement**
   - Format: `**Goal**: Description of what works at end of phase`
   - Focus on the runnable increment
4. **Create the task table**
   - Use exact header: `| Task ID | Description | SDD Ref | PRD Ref |`
   - Add separator row: `|---------|-------------|---------|---------|`

## Step 7: Create Tasks for Each Component

Break each component into specific, actionable tasks:

**Task characteristics:**
- **Specific**: Clear deliverable (e.g., "Create User class in src/models/user.py")
- **Actionable**: Developer knows exactly what to do
- **Testable**: Can verify completion
- **Scoped**: Completable in reasonable time
- **Sequenced**: Placed in execution order

**Example task breakdown for "User Authentication":**
- T-2.1: Create User model with username and password fields
- T-2.2: Implement password hashing using bcrypt
- T-2.3: Create login endpoint in API
- T-2.4: Add JWT token generation
- T-2.5: Test login with valid credentials
- T-2.6: Test login with invalid credentials

## Step 8: Add Test Tasks

For each phase, add test tasks that verify:
- Implementation works correctly
- Integration points function properly
- MVP is runnable and demonstrable

**Test task identification:**
- Include "test" in task description
- System auto-assigns to test engineer
- Place after related implementation tasks

**Example test tasks:**
```markdown
| T-1.5 | Test application launches without errors | §10.2 | - |
| T-1.6 | Test application exits with code 0 | §10.2 | - |
| T-2.4 | Test user login with valid credentials | §10.3 | §3.2 |
| T-2.5 | Test user login rejects invalid credentials | §10.3 | §3.2 |
```

## Step 9: Add SDD and PRD References

Link each task to source documents:

**SDD References** (design sections):
- Format: `§X` or `§X.Y` (e.g., §4.1, §5.2)
- Links to component/module design
- Use `-` if no SDD reference applies

**PRD References** (requirements):
- Format: `§X` or `§X.Y` (e.g., §3.1, §2.4)
- Links to user requirements or features
- Use `-` if no PRD reference applies

**Example:**
```markdown
| T-2.3 | Implement user login API endpoint | §4.2 | §3.1 |
| T-2.4 | Add password hashing with bcrypt | §4.3 | - |
```

## Step 10: Sequence Tasks in Execution Order

Within each phase, order tasks by:
1. **Dependencies**: Prerequisites before dependents
2. **Build order**: Foundation before features
3. **Logic**: Setup → Implementation → Testing

**Example correct sequence:**
```markdown
| T-1.1 | Create project directory structure | §8 | - |        ← Setup first
| T-1.2 | Create main entry point | §4.1 | §3.1 |            ← Entry point next
| T-1.3 | Implement configuration loader | §5 | - |           ← Config needed
| T-1.4 | Add logging setup | §6 | - |                      ← Logging ready
| T-1.5 | Test application launches | §10.2 | - |            ← Test at end
```

## Step 11: Verify MVP-First Compliance

For each phase, confirm:
- [ ] Phase ends with something runnable
- [ ] Tests verify the increment works
- [ ] Goal statement describes the working deliverable
- [ ] Phase builds on previous working foundation

**Red flags:**
- Phase only sets up infrastructure (not runnable)
- No tests to verify functionality
- Goal is vague or doesn't describe working software

## Step 12: Validate Structure

Run through the validation checklist (`references/validation-checklist.md`):

**Critical checks:**
- [ ] All phase headers at same level
- [ ] Task IDs in T-X.Y format (no T-X.Y.Z)
- [ ] No duplicate phase numbers or task IDs
- [ ] Task tables have exactly 4 columns
- [ ] Separator rows present after all headers
- [ ] No orphan tables if using task groups
- [ ] All phases follow phase template exactly

## Step 13: Review and Refine

Before saving:
1. **Read through entire TTL** as if you're the developer
2. **Check task clarity**: Can developer understand what to do?
3. **Verify coverage**: All SDD components have tasks?
4. **Check sequencing**: Logical order within phases?
5. **Validate MVPs**: Each phase runnable?
6. **Confirm tests**: Each phase has test tasks?

## Step 14: Save and Confirm

1. **Save the TTL document** to specified location
2. **Run validation** (grep checks if needed)
3. **Confirm no errors** in structure or formatting
4. **Report completion** with summary:
   - Number of phases created
   - Total number of tasks
   - Organization strategy used (flat/hierarchical)
   - Any special considerations

## Common Generation Pitfalls

| Pitfall | Impact | Prevention |
|---------|--------|------------|
| Skipping phase template | Invalid structure | ALWAYS read template first |
| Three-level task IDs | Validation failure | Use only T-X.Y format |
| Mixed phase header levels | Parse error | Use same level for all phases |
| Orphan tables with task groups | Parse error | All tables under groups or none |
| Vague task descriptions | Unclear work | Be specific about deliverables |
| Missing test tasks | Untested code | Add tests for each phase |
| Non-MVP phases | Nothing runnable | Ensure each phase has working increment |

## Generation Time Estimates

Based on project complexity:
- **Small project** (3-4 phases, 15-25 tasks): 10-15 minutes
- **Medium project** (4-6 phases, 30-50 tasks): 20-30 minutes
- **Large project** (6-8+ phases, 60-100+ tasks): 40-60 minutes

## Tips for Efficient Generation

1. **Read template first** - saves rework
2. **Work phase by phase** - complete one before next
3. **Use hierarchical structure** for complex phases (10+ tasks)
4. **Keep flat structure** for simple phases (< 10 tasks)
5. **Be specific in descriptions** - include file paths, class names
6. **Reference liberally** - link to SDD/PRD sections
7. **Test continuously** - add test tasks throughout
8. **Validate frequently** - catch errors early

## Post-Generation Workflow

After generating the TTL:
1. TTL is parsed into task ledger JSON
2. Tasks appear in CLI task list
3. Developers execute tasks in sequence
4. System tracks progress and completion
5. Tests verify each phase works

Your TTL is the blueprint for the entire implementation. Invest time in making it clear, complete, and correct.
