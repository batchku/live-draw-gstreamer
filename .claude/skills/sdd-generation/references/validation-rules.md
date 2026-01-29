# SDD Validation Rules and Quality Standards

## ⛔⛔⛔ CRITICAL: KEYWORD PRESERVATION FROM PRD ⛔⛔⛔
## THIS IS A TOP CAUSE OF SDD VALIDATION FAILURES

**═══════════════════════════════════════════════════════════════════════════**
**║  ALL USER STORY ACTIONS FROM THE PRD MUST BE ADDRESSED IN THE SDD      ║**
**║  VALIDATION CHECKS THAT EACH "I want [X]" IS COVERED BY YOUR DESIGN    ║**
**═══════════════════════════════════════════════════════════════════════════**

### What is PRD Coverage?

The validator extracts user story actions from the PRD using this pattern:
```
"I want [action phrase]"
```

Each action phrase is checked against your SDD. If the first 3 words of any action phrase are not found in your SDD, validation FAILS.

### Examples:

| PRD User Story | Action Phrase | SDD Must Contain |
|----------------|---------------|------------------|
| "I want the game to launch quickly" | "the game to launch quickly" | "game to launch" (at minimum) |
| "I want the snake to move smoothly" | "the snake to move smoothly" | "snake to move" (at minimum) |
| "I want to see my score displayed" | "to see my score displayed" | "to see my" or "see my score" |

### How to Ensure Coverage

1. **Read all user stories from the PRD** before starting design
2. **For each "I want [X]"**, ensure your SDD sections (components when applicable, interfaces, workflows, or features) explicitly address [X]
3. **Use the PRD's exact terminology** - don't rephrase or use synonyms
4. If PRD says "snake", your SDD says "snake" (not "player entity")
5. If PRD says "food", your SDD says "food" (not "collectible item")

### Self-Check Before Submission

1. Extract all "I want [action]" phrases from the PRD
2. Search your SDD for the first 3 words of each action
3. If missing, add the concept to an appropriate component or section
4. Verify until ALL user story actions are addressed

**═══════════════════════════════════════════════════════════════════════════**

---

## Quality Standards

The SDD must meet these quality criteria:

### Completeness
- [ ] All 8 required sections are present
- [ ] Each section has substantive, detailed content
- [ ] Minimum 1 component is specified
- [ ] All components have public interfaces defined
- [ ] Data models and storage approach for all relevant entities are included
- [ ] If applicable, API endpoints for relevant user stories are specified
- [ ] Directory structure is complete

### Implementation-Ready
- [ ] **NO placeholder text** like "[TBD]", "[TODO]", "[Implementation details]"
- [ ] Code examples are syntactically valid
- [ ] File paths are specific and complete
- [ ] Technology versions are specified
- [ ] Component interfaces have type hints
- [ ] Storage schemas/definitions are specified when persistence is used

### Traceability
- [ ] All PRD requirements are addressed
- [ ] User stories map to components
- [ ] Functional requirements map to components and interfaces (APIs when applicable)
- [ ] Non-functional requirements addressed in architecture
- [ ] Technical constraints are honored

### Technical Quality
- [ ] Architecture follows established patterns
- [ ] SOLID principles are applied
- [ ] Separation of concerns is clear
- [ ] Error handling is comprehensive
- [ ] Security considerations are addressed when relevant

---

## Validation Criteria

Before generating or saving the SDD artifact, the agent MUST complete this final self-check:

- [ ] Re-read the PRD user stories, extract each "I want [action]" phrase, and verify the first 3 words of every action appear in the SDD (exact terminology, no synonyms).
- [ ] Confirm all 8 required sections are present with substantive content.
- [ ] Verify at least 1 component specification exists and each lists purpose, exact file path, dependencies, error handling, and a public interface with type hints and code.
- [ ] Ensure data models cover all entities with type hints, and the storage approach/schema is specified when persistence is used.
- [ ] If an API Specifications section is included, validate method/path, request/response formats, status codes, and error scenarios.
- [ ] Confirm technology stack table includes versions, directory structure is complete, and the architecture diagram shows data flow.
- [ ] Check there is no placeholder text and all code examples are syntactically valid.
- [ ] Verify all PRD requirements and constraints are addressed and mapped to components and interfaces (APIs when applicable).

---

## Common Pitfalls to Avoid

❌ **Don't:**
- Use placeholder text or vague descriptions
- Skip component interface definitions
- Forget to specify file paths
- Use pseudocode instead of real code examples
- Leave out technology versions
- Miss PRD requirements in the design
- Create incomplete directory structures
- **Rephrase or use synonyms for PRD terminology** - causes validation failures
- Ignore user story action phrases from the PRD

✅ **Do:**
- Include working code examples with proper syntax
- Specify exact file paths for every component
- Include type hints on all function signatures
- Provide storage schemas/definitions when persistence is used
- Map all PRD requirements to components
- Include version numbers for all technologies
- Create comprehensive architecture diagrams
- **Preserve EXACT terminology from PRD** - use the same words for entities, actions, features
- **Address every "I want [X]" from PRD user stories** in your component designs
