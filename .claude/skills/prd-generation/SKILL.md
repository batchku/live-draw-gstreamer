---
name: prd-generation
description: Generates Product Requirements Documents (PRDs) from project briefs using a structured template. Use when defining product requirements, writing user stories with acceptance criteria, specifying functional and non-functional requirements, establishing success metrics and KPIs, documenting technical constraints, or capturing product vision and scope. Covers user personas, feature prioritization, and requirement traceability.
allowed-tools: Read Write Grep Glob WebFetch
---

# PRD Generation Skill

This skill generates comprehensive Product Requirements Documents (PRDs) from project briefs or high-level descriptions.

---

## When to Use This Skill

Use this skill when you need to:
- Convert a project brief into a formal PRD
- Document product vision and requirements for a new feature
- Create structured requirements documentation
- Establish a clear product specification before design begins
- Define user stories with acceptance criteria
- Capture functional and non-functional requirements
- Set measurable success metrics for a product
- Establish technical constraints and dependencies

---

## Purpose

This skill provides a systematic approach to PRD creation by:
1. Reading and analyzing the source brief or project description
2. Extracting key product goals, features, and constraints
3. Structuring requirements according to industry-standard PRD format
4. Generating specific, testable user stories with acceptance criteria
5. Defining clear success metrics and KPIs
6. Producing a complete, actionable document with no placeholders

---

## Critical Validation Rules

⚠️ **BEFORE WRITING ANY PRD**, read these validation rules to avoid common failures:

### Brief Keyword Coverage
**Every keyword from the brief must appear in your PRD.** The validation system performs literal text matching - if the brief says "snake", you must write "snake" (not "serpent"). Using synonyms causes validation failures.

📖 **See [references/validation-rules.md](references/validation-rules.md#-critical-brief-keyword-coverage-)** for:
- How keywords are extracted from briefs
- Why literal matching is required
- Common mistakes that cause failures
- Self-check procedures

### User Story Format
**User stories are validated by regex and must follow exact format.** The most common validation failure is incorrect user story formatting.

**Required format:**
```
As a [user type], I want [feature] so that [benefit].
```

**Critical rules:**
- Single line (no line breaks)
- Comma after role: `As a user,` NOT `As a user`
- Plain text only (no bold markdown)
- Must match regex: `As a .+, I want .+ so that`
- Every story requires acceptance criteria (checklist format with `- [ ]`)

📖 **See [references/validation-rules.md](references/validation-rules.md#-critical-user-story-format-)** for:
- Complete formatting rules
- Common mistakes (multi-line bold format, missing comma)
- Correct and incorrect examples
- Acceptance criteria requirements

---

## MVP-First Principles

Structure requirements to support incremental MVP delivery:

### Phase 1: Minimal Viable Product
- Define the simplest proof the application works
- Focus on initialization and basic execution
- Example: "System shall launch and display ready status"

### Phase 2+: Incremental Features
- Build complete features on working base
- Support vertical slicing (full feature stack)
- Each phase delivers demonstrable user value

📖 **See [references/mvp-first-requirements.md](references/mvp-first-requirements.md)** for:
- Detailed MVP-first structuring guidance
- Vertical vs horizontal slicing examples
- Good vs bad requirement structures
- Common pitfalls and how to avoid them

---

## Required PRD Sections

A complete PRD must contain all 9 sections with substantive content:

### 1. Executive Summary
- Vision and purpose statement
- Target users and personas
- Value proposition

### 2. Goals and Objectives
- User goals (what users want to accomplish)
- Technical goals (technical objectives and quality attributes)

### 3. User Stories
- Single-line format: `As a [user type], I want [feature] so that [benefit].`
- Acceptance criteria (checklist with `- [ ]` items)
- Priority (Must/Should/Could/Won't)
- Minimum: 1 user story required

### 4. Functional Requirements
- Feature specifications with detailed descriptions
- User flows (step-by-step workflows)
- Edge cases (exceptional scenarios)
- Business rules (validation and constraints)

### 5. Non-Functional Requirements
- Performance (with specific numbers)
- Security (authentication, authorization, data protection)
- Scalability (load and growth targets)
- Accessibility (WCAG compliance, screen reader support)
- Reliability (uptime targets)
- Compatibility (platform/browser/device support)

### 6. Technical Constraints
- Required technologies and tech stack
- Third-party integrations
- Platform constraints
- Compliance requirements (GDPR, HIPAA, etc.)
- Legacy system integrations

### 7. Assumptions and Dependencies
- Assumptions (what we assume to be true)
- External dependencies (third-party services, APIs)
- Internal dependencies (other teams/systems)
- Risks (potential blockers)

### 8. Out of Scope
- Explicitly excluded features
- Future considerations (deferred features)
- Clear boundaries of what's not included

### 9. Success Metrics
- KPIs with specific targets
- Measurement methods
- Examples: User adoption targets, performance metrics, quality goals

📖 **See [references/prd-template.md](references/prd-template.md)** for:
- Detailed guidance for each section
- Complete PRD markdown template
- Examples for each section type
- Best practices for content structure

---

## Quality Standards

The PRD must meet these criteria:

### Completeness
- All 9 required sections present with substantive content
- All user stories have acceptance criteria
- All technical constraints identified

### Specificity
- **NO placeholder text** like "[TBD]", "[TODO]", "[To be determined]"
- Requirements are specific and measurable
- Performance targets include actual numbers
- Success metrics have quantifiable targets

### Testability
- Requirements can be verified or tested
- Acceptance criteria are clear pass/fail conditions
- Success metrics are measurable

### Clarity
- Professional language with appropriate technical terms
- User stories follow standard format
- Requirements organized logically

### No Time or Effort Estimates
- **NO timeline estimates** (e.g., "Week 1-2", "Q1 2024")
- **NO work effort estimates** (e.g., "40 hours", "2 person-weeks")
- Describe requirements by scope and deliverables, not duration

---

## Generation Process

Follow this systematic process when generating a PRD:

1. **Read the brief** - Thoroughly analyze the source brief or project description
2. **Extract key information** - Identify goals, features, constraints, users, and all keywords
3. **Structure content** - Organize according to the 9-section template
4. **Expand requirements** - Develop detailed functional and non-functional requirements
5. **Create user stories** - Write stories in correct single-line format with acceptance criteria
6. **Define metrics** - Establish specific, measurable success criteria
7. **Verify keyword coverage** - Ensure every brief keyword appears in PRD (no synonyms)
8. **Review completeness** - Check all sections complete with no placeholders
9. **Self-check user stories** - Verify format matches `As a .+, I want .+ so that`
10. **Save document** - Use Write tool to save PRD to specified location

---

## External References Handling

External references (URLs and file paths) are automatically extracted from the brief and provided in the prompt under "📚 Extracted External References" section.

### When References Are Provided

1. Use **WebFetch** for URLs and **Read** for file paths to retrieve content
2. Extract actionable information (APIs, data structures, configuration, patterns)
3. Generate `docs/planning/EXTERNAL.md` documenting relevant information
4. Focus on information useful for downstream agents (design, implementation, testing)

### What to Extract
- API endpoints, authentication, rate limits
- Key classes, functions, configuration options
- UI/UX patterns and branding requirements
- Data formats, schemas, validation rules
- Integration protocols and requirements

### Important Notes
- Only generate EXTERNAL.md if references are provided in the prompt
- Do NOT scan the brief yourself for references
- Focus on actionable implementation details
- Keep summaries concise (1-2 sentences per source)

📖 **See [references/external-references-guide.md](references/external-references-guide.md)** for:
- Complete EXTERNAL.md template
- Detailed extraction guidelines
- Example EXTERNAL.md structure
- Tool usage instructions (WebFetch/Read)

---

## Usage Instructions

To use this skill effectively:

1. **Provide the brief** - Ensure project brief or description is available
2. **Specify output location** - Indicate where PRD should be saved (typically `docs/planning/PRD.md`)
3. **Include context** - Provide any additional constraints or requirements
4. **Review validation rules** - Read validation-rules.md before starting
5. **Validate output** - Use the checklist to verify completeness

**Example invocation:**
```
Generate a PRD from brief.md and save it to docs/planning/PRD.md
```

---

## Common Pitfalls to Avoid

❌ **Don't:**
- Use placeholder text like [TBD], [TODO], [Coming soon]
- Write vague requirements ("fast", "user-friendly", "scalable")
- Skip acceptance criteria on user stories
- Forget to specify metrics with actual numbers
- Leave any required sections empty
- Include timeline or effort estimates
- Format user stories incorrectly (multi-line, bold, missing comma)
- Use synonyms for brief keywords (write exact words from brief)
- Paraphrase or rephrase key terms from the brief

✅ **Do:**
- Be specific and measurable in all requirements
- Include actual numbers for performance targets
- Write complete acceptance criteria for every user story
- Ensure all 9 sections have substantive content
- Follow single-line user story format exactly
- Use EXACT terminology from the brief - every keyword must appear
- Verify all brief concepts are mentioned before finalizing
- Read validation-rules.md before writing user stories

---

## Final Validation Checklist

Before saving the PRD, complete this self-check:

- [ ] All brief keywords appear in PRD (exact words, no synonyms)
- [ ] All 9 required sections present with substantive content
- [ ] Every user story is single-line, plain text, comma after role
- [ ] Every user story matches regex: `As a .+, I want .+ so that`
- [ ] Every Acceptance Criteria section has at least one `- [ ]` item
- [ ] At least one user story exists with priority specified
- [ ] No placeholder text anywhere in document
- [ ] No timeline or work-effort estimates
- [ ] Functional and non-functional requirements are specific/testable
- [ ] Performance targets include numeric values
- [ ] Success metrics are quantifiable
- [ ] Technical constraints, assumptions/dependencies, and out-of-scope sections are complete

📖 **See [references/validation-rules.md](references/validation-rules.md#final-validation-checklist)** for the complete checklist.

---

## Required Response Format

### Completion Notes

- Set `task_id` to "prd_generation" in your response
- List "docs/planning/PRD.md" in `artifacts_changed`
- If EXTERNAL.md was generated, also list "docs/planning/EXTERNAL.md" in `artifacts_changed`
- Set `task_status` to "SUCCESS" when PRD is complete

---

## Notes

- The PRD serves as the single source of truth for product requirements
- The PRD should be updated as requirements evolve
- Use the PRD as input for the SDD generation process
- All detailed validation rules, templates, and examples are in the references/ subdirectory
