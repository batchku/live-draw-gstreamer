---
name: sdd-generation
description: Generates Software Design Documents (SDDs) by analyzing Product Requirements Documents (PRDs) and converting the "why" into the "how". Use when translating product requirements into technical design specifications, defining system architecture, specifying component interfaces, designing data models, selecting technology stacks, creating directory structures, or producing implementation-ready specifications. Covers architecture diagrams, error handling strategies, and testing approaches.
allowed-tools: Read Write Grep Glob
---

# SDD Generation Skill

This skill generates detailed Software Design Documents (SDDs) from Product Requirements Documents (PRDs). It transforms product requirements and user stories into concrete technical designs, system architectures, and implementation specifications that autonomous agents can execute.

---

## When to Use This Skill

Use this skill when you need to:
- Convert a PRD into a technical design document
- Define system architecture for a new feature or application
- Create implementation-ready specifications for autonomous agents
- Document component interfaces and data models
- Design API specifications and endpoints when the system exposes an API
- Establish directory structure and code organization
- Define technology stack and architectural patterns
- Create architectural diagrams and system overviews

## Purpose

This skill provides a systematic approach to SDD creation by:
1. Reading and analyzing the source PRD
2. Translating user stories into system components
3. Designing data models and storage
4. Specifying component interfaces with code examples
5. Defining integration points and service interfaces (API endpoints when applicable)
6. Selecting appropriate technologies and patterns
7. Creating architecture diagrams and directory structures
8. Producing implementation-ready technical specifications

---

## Critical Requirements

### PRD Coverage Validation

**⛔ CRITICAL**: All user story actions from the PRD MUST be addressed in the SDD. The validator extracts "I want [action]" phrases from the PRD and verifies that the first 3 words of each action appear in your SDD using exact terminology (no synonyms).

**Example**: If the PRD says "I want the snake to move smoothly", your SDD must contain "snake to move" (at minimum).

For detailed validation rules and self-check procedures, see [references/validation-rules.md](references/validation-rules.md).

### Automated Testing Only

**⛔ CRITICAL**: All tests specified in the Testing Strategy section MUST be fully automated and executable without human interaction. Do NOT specify manual GUI testing, interactive user testing, or any testing requiring human observation.

**Required**: All tests must be executable via command-line (e.g., `pytest`, `npm test`) with programmatic assertions.

---

## MVP-First Architecture Design

Design the system to support **incremental vertical slicing**:

### Phase 1 Design: Minimal Working Application

The Phase 1 architecture must enable a **runnable, testable application**:
- **Entry Point**: Clear main execution path that can be invoked
- **Minimal Bootstrap**: Simplest initialization to get application running
- **Status Output**: Mechanism to demonstrate application is working
- **Basic Infrastructure**: Only what's needed for Phase 1 to run

### Vertical Slice Architecture

Design components as **complete vertical slices**, not horizontal layers:

✅ **Good**: Phase 2 delivers complete user registration feature (data model + database + service + API + tests)

❌ **Bad**: Phase 1 = all data models, Phase 2 = all business logic, Phase 3 = all APIs (nothing works until Phase 3)

For detailed MVP-First principles and examples, see [references/sdd-best-practices.md](references/sdd-best-practices.md).

---

## Required SDD Sections

A complete SDD must contain these 8 sections (API Specifications section is optional, include when system exposes network APIs):

1. **System Overview**
   - Architecture style and system boundaries
   - Key architectural decisions and rationale
   - Design principles (SOLID, DRY, etc.)

2. **Architecture Diagram**
   - ASCII art or Mermaid diagram showing all major components
   - Data flow between components
   - External integrations

3. **Component Specifications** (Optional)
   - Include only when system has discrete components warranting separate specs
   - For each component: purpose, exact file path, public interface with type hints, dependencies, error handling, and code examples

4. **Data Models and Storage**
   - Models with type hints and field descriptions
   - Storage approach (SQL, NoSQL, files, in-memory)
   - Schemas/migrations when persistence is used
   - Relationships and validation rules

5. **Technology Stack**
   - Table with layer, technology, version, and purpose
   - Justification for each technology choice

6. **Directory Structure**
   - Complete tree structure with all directories and key files
   - Organization conventions and special directories

7. **Error Handling Strategy**
   - Exception hierarchy with custom exception classes
   - Error propagation and logging approach
   - User feedback and recovery mechanisms

8. **Testing Strategy**
   - Automated unit, integration, and E2E tests only
   - Test coverage targets and organization
   - Mocking strategy and CI/CD integration
   - Command to run all tests

**Optional Section**:

- **API Specifications** (when applicable)
  - Include when system exposes network APIs or external integration points
  - For each endpoint: method/path, request/response formats, status codes, authentication, error scenarios

For detailed section templates with comprehensive examples, see [references/sdd-template.md](references/sdd-template.md).

---

## Quality Standards

The SDD must meet these criteria:

### Completeness
- All required sections present with substantive content
- At least 1 component specified (when applicable) with complete interface
- Data models and storage approach for all entities
- Directory structure is complete

### Implementation-Ready
- **NO placeholder text** like "[TBD]", "[TODO]", "[Implementation details]"
- Code examples are syntactically valid
- File paths are specific and complete
- Technology versions are specified
- Component interfaces have type hints

### Traceability
- All PRD requirements addressed in components/interfaces
- User stories map to components
- Technical constraints honored

### Technical Quality
- Architecture follows established patterns
- SOLID principles applied
- Clear separation of concerns
- Comprehensive error handling

For detailed quality checklists and validation criteria, see [references/validation-rules.md](references/validation-rules.md).

---

## Generation Process

When generating an SDD, follow this systematic approach:

1. **Read the PRD**: Thoroughly analyze requirements, user stories, and constraints
2. **Design architecture**: Choose appropriate architectural style and patterns
3. **Identify components**: Break system into logical components based on user stories
4. **Design data models and storage**: Define entities and persistence approach
5. **Specify interfaces**: Define public APIs for each component with type hints
6. **Select technologies**: Choose tech stack based on PRD constraints
7. **Create diagrams**: Draw architecture diagram showing components and data flow
8. **Define directory structure**: Organize code according to architecture
9. **Document cross-cutting concerns**: Error handling, testing, security
10. **Review completeness**: Ensure all PRD requirements are addressed
11. **Validate**: Complete self-check against validation criteria
12. **Save document**: Use Write tool to save the SDD

For detailed generation process steps and best practices, see [references/sdd-best-practices.md](references/sdd-best-practices.md).

---

## Output Format

The SDD should be formatted as a Markdown document following the 8-section structure outlined above, adding the optional API Specifications section when applicable.

**File location**: Typically saved to `docs/planning/SDD.md` unless otherwise specified.

---

## Usage Instructions

To use this skill effectively:

1. **Read the PRD first**: Ensure you understand all requirements
2. **Extract user story actions**: List all "I want [X]" phrases to ensure coverage
3. **Be specific**: Include actual code examples with proper syntax, not pseudocode
4. **Specify locations**: Use exact file paths for all components
5. **Preserve terminology**: Use exact PRD terminology (no synonyms or rephrasing)
6. **Think implementation**: Design should be directly implementable
7. **Validate before saving**: Complete self-check against validation criteria

**Example invocation:**
```
Generate an SDD from docs/planning/PRD.md and save it to docs/planning/SDD.md
```

---

## Quick Reference

### Key Validation Rules
- Preserve exact PRD terminology (no synonyms)
- Address every "I want [X]" from PRD user stories
- No placeholder text or TBD markers
- All code examples must be syntactically valid
- Specify exact file paths for all components
- Include version numbers for all technologies

### Common Pitfalls
- Rephrasing PRD terminology (causes validation failures)
- Skipping component interface definitions
- Using vague or manual testing descriptions
- Missing storage schemas when persistence is used
- Incomplete directory structures
- Placeholder text instead of real specifications

For comprehensive lists of what to do and what to avoid, see [references/validation-rules.md](references/validation-rules.md).

---

## Final Self-Check Before Submission

Before saving the SDD artifact, complete this validation:

- [ ] All "I want [action]" phrases from PRD are addressed in SDD (exact terminology)
- [ ] All 8 required sections present with substantive content
- [ ] At least 1 component specification exists with complete interface (when applicable)
- [ ] Data models cover all entities with type hints and storage approach
- [ ] Technology stack includes versions, directory structure is complete
- [ ] Architecture diagram shows data flow
- [ ] No placeholder text, all code examples are syntactically valid
- [ ] All PRD requirements and constraints are addressed

For detailed validation checklist, see [references/validation-rules.md](references/validation-rules.md).

---

## Required Response Format

### Completion Notes

- Set `task_id` to "sdd_generation" in your response
- List "docs/planning/SDD.md" in `artifacts_changed`
- Set `task_status` to "SUCCESS" when SDD is complete

### Notes

- The SDD bridges product requirements and implementation
- It should be detailed enough for developers to start coding immediately
- All architectural decisions should be justified
- The SDD serves as the blueprint for the Technical Task List (TTL)
- Update the SDD as design evolves during implementation

---

## References

For detailed information, see:
- [references/validation-rules.md](references/validation-rules.md) - Comprehensive validation rules, quality standards, and common pitfalls
- [references/sdd-template.md](references/sdd-template.md) - Detailed section templates with extensive code examples
- [references/sdd-best-practices.md](references/sdd-best-practices.md) - MVP-First design principles and generation process details
