---
name: ttl-sizing
description: Right-sizes Technical Task Lists (TTLs) by scoring project complexity across multiple dimensions and adjusting task granularity to match scope. Use when a TTL has too many tasks for a simple project (over-engineering), too few tasks for a complex project (under-specification), or when task count needs validation against project size. Scores projects as Micro/Tiny/Small/Medium/Large/Enterprise and provides target phase and task counts. Prevents scope mismatch between project complexity and task breakdown detail.
allowed-tools: Read Write Grep Glob
---

# TTL Sizing Skill

Right-size a Technical Task List by scoring the project's complexity and adjusting tasks to match.

---

## CRITICAL: Follow This Exact Process

### Step 1: Score the Project (MANDATORY)

You MUST complete this scoring table with evidence from the brief BEFORE any TTL work:

```markdown
## Size Assessment

| Dimension | Score | Evidence |
|-----------|-------|----------|
| Product surface (0=one flow, 5=several, 10=many) | | |
| Artifacts/platforms (0=one, 5=FE+BE, 10=multi-client/multi-service) | | |
| Data model (0=none, 5=several entities, 10=complex/multi-tenant) | | |
| Integrations (0=none, 5=1-2 APIs, 10=multiple/critical) | | |
| Security/compliance (0=none, 5=roles+PII, 10=regulated/SSO) | | |
| Ops/reliability (0=local, 5=multi-env+monitoring, 10=HA/SLO) | | |
| Performance/scale (0=negligible, 5=clear constraints, 10=strict SLOs) | | |
| UX/design complexity (0=CLI/basic UI, 5=multi-screen, 10=custom UX) | | |
| Migration/legacy impact (0=greenfield, 5=light migration, 10=legacy/zero-downtime) | | |
| Uncertainty buffer (0=clear, 5=some unknowns, 10=high risk) | | |
| **TOTAL (0–100)** | | |

**Band**: [from table below]
**Target Phases**: [from table below]
**Target Tasks**: [from table below]
```

### Step 2: Map Score to Band and Targets

| Score | Band | Phases | Total Tasks |
|------:|------|-------:|------------:|
| 0–10 | **Micro** | 1 | 1–5 |
| 11–22 | **Tiny** | 2–4 | 5–25 |
| 23–34 | **Small** | 4–7 | 25–60 |
| 35–46 | **Medium** | 7–12 | 60–130 |
| 47–58 | **Large** | 12–20 | 130–240 |
| 59–70 | **Mega** | 20–30 | 240–360 |
| 71–85 | **Huge** | 30–45 | 360–500 |
| 86–100 | **Enterprise** | 50+ | 500+ |

### Step 3: Validate Your Score

**STOP. Check these rules before proceeding:**

- If brief says "single script", "simple", "tiny", "minimal" → score should be 0–10 (Micro)
- If score is 0–10 → you MUST produce exactly 1 phase with 1–5 tasks
- If score is 11–22 → you MUST produce 2–4 phases with 5–25 total tasks
- If your task count exceeds the target range, YOU ARE WRONG → re-score or re-consolidate

### Step 4: Adjust the TTL

Based on your band:

**Micro (0–10)**: Maximum consolidation
- Collapse ALL phases into 1
- Merge ALL tasks into 1–5 total
- Tests can be inline or one separate task

**Tiny (11–22)**: Aggressive consolidation  
- Merge setup/config into one task
- One test task per phase maximum
- 2–4 phases, 5–25 total tasks

**Small+ (23+)**: Targeted consolidation
- Merge by deliverable, not by file
- Keep cross-cutting concerns explicit
- Refer to `references/domain-examples.md` for patterns

---

## Validation Gate (REQUIRED before saving)

Answer these questions. If ANY answer is NO, fix the TTL:

1. Did I fill out the scoring table with evidence? 
2. Does my total task count fall within the target range for my band?
3. Does my phase count match the band target?
4. If the brief mentions "simple/tiny/minimal/single", is my score ≤10?

---

## Common Failure: Over-Engineering Simple Projects

### WRONG: Word Counter
Brief: "Count words in a text file. Single Python script. Standard library only."

❌ Agent scored 30 → "Small" → produced 27 tasks, 3 phases

✅ Correct scoring:
- Product surface: 0 (one tiny flow: read→count→print)
- Artifacts: 0 (single script)
- Data model: 0 (no persistence)
- Integrations: 0 (stdlib only)
- Security: 0 (no auth)
- Ops: 0 (local only)
- Performance: 0 (negligible)
- UX/design: 0 (CLI output only)
- Migration/legacy: 0 (greenfield)
- Uncertainty: 0 (crystal clear)
- **TOTAL: 0 → Micro → 1 phase, 1-5 tasks**

Correct TTL:
```markdown
## Phase 1: Implementation
| T-1.1 | Create word_counter.py with CLI, file reading, and word counting |
| T-1.2 | Add error handling and create test file for verification |
| T-1.3 | Add unit tests |
```

### WRONG: Temperature Converter
Brief: "Python script to convert Celsius to Fahrenheit and vice versa."

❌ Agent produced 24 tasks across 5 phases

✅ Correct: Score 0 → Micro → 1 phase, 2 tasks

---

## Output Format

**CRITICAL**: All phase headers must use the SAME markdown level (e.g., all `##` or all `###`). Inconsistent levels will cause validation failures.

```markdown
# Technical Task List (TTL): [Project Name]

## Size Assessment
[Your completed scoring table from Step 1]

## Phase 1: [Name]

**Goal**: [One sentence]

| Task ID | Description | SDD Ref | PRD Ref |
|---------|-------------|---------|---------|
| T-1.1 | [Task] | §X | FR-Y |

## Phase 2: [Name]   <-- Same level as Phase 1

[Additional phases only if band requires them]
```

---

## Quick Reference

For detailed scoring guidance: `references/size-rubric.md`
For domain-specific examples: `references/domain-examples.md`

**Key Rules:**
- Score FIRST, consolidate SECOND
- Brief says "simple" → score near 0
- Task count MUST match band target
- When in doubt, score LOWER (prefer under-engineering to over-engineering)
- ALL phase headers must use the same markdown level (consistency required)
