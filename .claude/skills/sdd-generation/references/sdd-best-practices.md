# SDD Best Practices and Generation Process

## MVP-First Architecture Design

Design the system to support **incremental vertical slicing**:

### Phase 1 Design: Minimal Working Application

The Phase 1 architecture must enable a **runnable, testable application**:

- **Entry Point**: Clear main execution path that can be invoked
- **Minimal Bootstrap**: Simplest initialization to get application running
- **Status Output**: Mechanism to demonstrate application is working
- **Basic Infrastructure**: Only what's needed for Phase 1 to run

**Example Phase 1 Components**:
```python
# src/main.py - Entry point that runs successfully
def main():
    """Application entry point - Phase 1 MVP"""
    config = load_config()
    app = Application(config)
    status = app.startup()
    print(f"Application v{VERSION} started successfully")
    print(f"Status: {status}")
    return 0

if __name__ == "__main__":
    sys.exit(main())
```

### Vertical Slice Architecture

Design components as **complete vertical slices**, not horizontal layers:

✅ **Good (Vertical Slice - Complete Feature)**:
```
Phase 2: User Registration Feature
├── Data Model: User class
├── Database: users table creation
├── Service: UserService.register()
├── API: POST /users endpoint
└── Tests: Full registration flow test
```
*Result: Working user registration feature*

❌ **Bad (Horizontal Layers - No Complete Feature)**:
```
Phase 1: All Data Models and Storage
Phase 2: All Business Logic
Phase 3: All API Endpoints
Phase 4: All Tests
```
*Result: Nothing works until Phase 4*

### Design Principles

- **Runnable First**: Every phase design must produce executable code
- **Testable Always**: Include test strategy for each phase's deliverables
- **Demonstrable Progress**: Each phase adds visible functionality
- **Incremental Complexity**: Start simple, add sophistication in later phases

---

## Generation Process

When generating an SDD, follow this process:

1. **Read the PRD**: Thoroughly analyze requirements, user stories, and constraints
2. **Design architecture**: Choose appropriate architectural style and patterns
3. **Identify components**: Break system into logical components based on user stories
4. **Design data models and storage**: Define entities and the persistence approach
5. **Specify interfaces**: Define public APIs for each component with type hints
6. **Select technologies**: Choose tech stack based on PRD constraints
7. **Create diagrams**: Draw architecture diagram showing components and data flow
8. **Define directory structure**: Organize code according to architecture
9. **Document cross-cutting concerns**: Error handling, testing, and security when relevant
10. **Review completeness**: Ensure all PRD requirements are addressed
11. **Save document**: Use Write tool to save the SDD

---

## Usage Guidelines

To use this skill effectively:

1. **Read the PRD first**: Ensure you understand all requirements
2. **Consider constraints**: Honor technical constraints from the PRD
3. **Be specific**: Include actual code examples, not pseudocode
4. **Specify locations**: Use exact file paths for all components
5. **Think implementation**: Design should be directly implementable

**Example invocation:**
```
Generate an SDD from docs/planning/PRD.md and save it to docs/planning/SDD.md
```

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
