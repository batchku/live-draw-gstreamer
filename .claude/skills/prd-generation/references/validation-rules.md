# PRD Validation Rules

This document contains critical validation rules that PRDs must pass. Read this carefully before writing your PRD.

---

## ⛔⛔⛔ CRITICAL: BRIEF KEYWORD COVERAGE ⛔⛔⛔
## THIS IS A TOP CAUSE OF PRD VALIDATION FAILURES

**═══════════════════════════════════════════════════════════════════════════**
**║  EVERY KEYWORD EXTRACTED FROM THE BRIEF MUST APPEAR IN YOUR PRD         ║**
**║  VALIDATION WILL FAIL IF ANY BRIEF CONCEPT IS MISSING FROM THE PRD      ║**
**═══════════════════════════════════════════════════════════════════════════**

### What is Brief Coverage?

The brief is analyzed using NLP to extract key concepts:
- **Nouns** → Potential classes/entities (e.g., "snake", "game", "food", "score")
- **Verbs** → Operations/actions (e.g., "move", "eat", "grow", "display")
- **Technical terms** → Domain-specific words (e.g., "terminal", "curses", "ASCII")

**ALL of these extracted keywords MUST appear somewhere in your PRD text.**

### Why Does This Matter?

The validation system performs a **literal text search** for each keyword:
```python
for keyword in brief_keywords:
    if keyword.lower() not in prd_text.lower():
        FAIL("Missing brief concept: " + keyword)
```

If the brief says "snake" and you write "serpent", validation FAILS.
If the brief says "terminal" and you write "console", validation FAILS.

### How to Ensure Coverage

1. **If provided with extracted keywords** (in the prompt under "Extracted Keywords"):
   - Use EVERY keyword from the list at least once in your PRD
   - Do NOT substitute synonyms - use the EXACT terms provided
   - Check your PRD against the keyword list before finalizing

2. **If NOT provided with keywords**:
   - Read the brief carefully and identify all nouns, verbs, and technical terms
   - Include each identified term at least once in your PRD
   - When in doubt, use the brief's exact wording

### ❌ COMMON MISTAKES THAT CAUSE KEYWORD FAILURES:

┌───────────────┬─────────────────┬─────────────────────────────────┐
│ Brief Says    │ PRD Says        │ Result                          │
├───────────────┼─────────────────┼─────────────────────────────────┤
│ "snake"       │ "serpent"       │ ❌ FAIL - synonym not accepted  │
│ "terminal"    │ "console"       │ ❌ FAIL - synonym not accepted  │
│ "food pellet" │ "collectible"   │ ❌ FAIL - rephrased             │
│ "high score"  │ "best result"   │ ❌ FAIL - rephrased             │
└───────────────┴─────────────────┴─────────────────────────────────┘

### ✅ CORRECT APPROACH:

┌───────────────┬────────────────────────────────┬──────────┐
│ Brief Says    │ PRD Says                       │ Result   │
├───────────────┼────────────────────────────────┼──────────┤
│ "snake"       │ "The snake will move..."       │ ✅ PASS  │
│ "terminal"    │ "...runs in the terminal..."   │ ✅ PASS  │
│ "food pellet" │ "...collects food pellets..."  │ ✅ PASS  │
│ "high score"  │ "...tracks the high score..."  │ ✅ PASS  │
└───────────────┴────────────────────────────────┴──────────┘

### Self-Check Before Submission

Before writing the final PRD:
1. List all nouns, verbs, and technical terms from the brief
2. Search your PRD for each term (case-insensitive)
3. If ANY term is missing, add it naturally to an appropriate section
4. Re-verify until ALL terms are present

**═══════════════════════════════════════════════════════════════════════════**
**║  REMEMBER: Use the EXACT words from the brief. NO synonyms. NO          ║**
**║  paraphrasing. If the brief says it, your PRD must say it too.          ║**
**═══════════════════════════════════════════════════════════════════════════**

---

## ⛔⛔⛔ CRITICAL: USER STORY FORMAT ⛔⛔⛔
## THIS IS THE #1 CAUSE OF PRD VALIDATION FAILURES

**═══════════════════════════════════════════════════════════════════════════**
**║  USER STORIES ARE VALIDATED BY REGEX AND WILL FAIL IF FORMATTED WRONG   ║**
**═══════════════════════════════════════════════════════════════════════════**

### ✅ REQUIRED FORMAT (Use this EXACT pattern for EVERY user story):

```
As a [user type], I want [feature] so that [benefit].
```

### 🔴 CRITICAL RULES (Breaking ANY rule = validation failure):

1. **SINGLE LINE** - The entire user story MUST be on ONE continuous line
2. **COMMA AFTER ROLE** - Write `As a user,` NOT `As a user` (comma is mandatory)
3. **PLAIN TEXT ONLY** - NO markdown formatting: `As a` NOT `**As a**`
4. **VALIDATION REGEX**: `As a .+, I want .+ so that` (your story must match this)
5. **ACCEPTANCE CRITERIA REQUIRED** - Every user story must include acceptance criteria
6. **ACCEPTANCE CRITERIA CHECKLIST REQUIRED** - The `**Acceptance Criteria:**` section must be a checklist using `- [ ]` and include at least one item. Empty sections or non-checklist bullets fail validation.

### ❌ COMMON MISTAKES THAT FAIL VALIDATION:

**Mistake #1: Multi-line bold format (MOST COMMON ERROR)**
```markdown
**As a** terminal user
**I want to** launch the game
**So that** I can play
```
❌ FAILS: Multi-line, uses bold

**Mistake #2: No comma after role**
```markdown
As a terminal user I want to launch the game so that I can play
```
❌ FAILS: Missing comma after "terminal user"

**Mistake #3: Bold formatting**
```markdown
**As a** terminal user, **I want** to launch the game **so that** I can play
```
❌ FAILS: Uses bold markdown

### ✅ CORRECT EXAMPLES (These PASS validation):

```markdown
As a terminal user, I want the game to launch with a simple command so that I can start playing quickly.
```

```markdown
As a player, I want the snake to grow when eating food so that the game becomes more challenging.
```

```markdown
As a developer, I want clear error messages when the game crashes so that I can debug issues efficiently.
```

### Complete User Story Example:
```markdown
### §3.1: Game Launch (Priority: Must)

As a terminal user, I want the game to launch with a simple command so that I can start playing quickly.

**Acceptance Criteria:**
- [ ] Game launches with `python3 game.py`
- [ ] Initialization completes within 1 second
- [ ] Clear error message if Python version < 3.8
```

**═══════════════════════════════════════════════════════════════════════════**
**║  BEFORE WRITING: Verify each user story is single-line, plain text,     ║**
**║  with comma after role, and matches: As a .+, I want .+ so that         ║**
**═══════════════════════════════════════════════════════════════════════════**

---

## Final Validation Checklist

Before generating or saving the PRD artifact, complete this self-check:

- [ ] Re-read the brief, list all nouns/verbs/technical terms, and confirm every exact keyword appears in the PRD (case-insensitive literal match; no synonyms).
- [ ] Confirm all 9 required sections are present and each has substantive content (no empty headers).
- [ ] Validate every user story is single-line plain text, includes the comma after role, and matches `As a .+, I want .+ so that` with acceptance criteria for each.
- [ ] Confirm every `Acceptance Criteria` block contains at least one checklist item (`- [ ]`). Empty or missing checklists fail.
- [ ] Verify at least one user story exists and priorities are specified.
- [ ] Ensure no placeholder text and no timeline or work-effort estimates appear anywhere.
- [ ] Check functional and non-functional requirements are specific/testable and performance targets include numeric values.
- [ ] Confirm success metrics are quantifiable and technical constraints, assumptions/dependencies, and out-of-scope sections are complete.
