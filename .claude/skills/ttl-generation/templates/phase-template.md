# TTL Phase Template

**Required elements:**
- Phase header with number and name
- 4-column task table
- Task IDs matching phase number (T-1.x for Phase 1)

---

## Phase N: [Phase Name]

| Task ID | Description | SDD Ref | PRD Ref |
|---------|-------------|---------|---------|
| T-N.1   | [Specific actionable task description] | §X.X | §X.X |
| T-N.2   | [Specific actionable task description] | §X.X | - |
| T-N.3   | Test [feature/component] with automated assertions | §X.X | - |

---

## Format Rules

| Element | Format | Notes |
|---------|--------|-------|
| Phase Header | `## Phase N: [Name]` | Number required, format flexible |
| Task Table Header | `\| Task ID \| Description \| SDD Ref \| PRD Ref \|` | Exactly 4 columns |
| Separator Row | `\|---------|-------------|---------|---------|` | Must follow header |
| Task ID | `T-N.Y` | N = phase number, Y = sequential |
| Empty Reference | `-` | Use dash, not blank |

**Phase header synonyms:** Phase, Sprint, Iteration, Milestone, Stage, Task Group
**Phase header separators:** colon, dash, or space

## Example

```markdown
## Phase 1: Project Foundation

| Task ID | Description | SDD Ref | PRD Ref |
|---------|-------------|---------|---------|
| T-1.1   | Create project directory structure | §8 | - |
| T-1.2   | Create main entry point file | §4.1 | §3.1 |
| T-1.3   | Add configuration loader | §5 | - |
| T-1.4   | Test application starts successfully | §10.2 | - |

## Phase 2: Core Features

| Task ID | Description | SDD Ref | PRD Ref |
|---------|-------------|---------|---------|
| T-2.1   | Implement input validation module | §5.1 | §3.3 |
| T-2.2   | Add error handling framework | §5.2 | §3.4 |
| T-2.3   | Test input validation with automated assertions | §10.2 | - |
```
