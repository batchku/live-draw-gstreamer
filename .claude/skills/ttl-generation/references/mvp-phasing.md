# MVP-First Phasing Strategy

## Core Requirement

Each phase MUST end with a runnable, executable increment (Minimum Viable Product). Phase count, names, and goal content are project-specific, but the MVP requirement applies to every phase.

## Phase Completion Criteria

Every phase MUST end with:
- A **runnable application** that executes without errors
- A **demonstrable feature** that shows visible progress
- **Passing tests** that verify the working functionality
- **Clear evidence** the software works (output, status, or behavior)

## Phase 1: Minimal Working Application (Example)

**Example Goal**: Create the simplest possible working application that executes successfully.

**Example MVP Deliverable**: An executable program that:
- Can be run from command line
- Executes without errors
- Returns success exit code 0
- Demonstrates the application is "alive"

### MVP Deliverables by Project Type

- **CLI Application**: Runs successfully and exits with code 0
- **Web API**: Server starts, binds to port, and responds to health check endpoint
- **Desktop App**: Window opens and renders without crashing
- **Library**: Can be imported without errors and basic function executes
- **Daemon/Service**: Process starts, initializes resources, and enters main loop

**Key Point**: The MVP should prove the application runs (exit code 0).

### Example Implementation Tasks (Phase 1)

```markdown
## Phase 1: Minimal Working Application

**Goal**: Create the simplest possible working application that executes successfully.

| Task ID | Description | SDD Ref | PRD Ref |
|---------|-------------|---------|---------|
| T-1.1 | Create project directory structure | §8 | - |
| T-1.2 | Create main entry point with basic execution | §4.1 | §3.1 |
| T-1.3 | Implement minimal startup routine that exits cleanly | §4.1 | §3.1 |
| T-1.4 | Add basic configuration loader (minimal defaults) | §5 | - |
| T-1.5 | Create test framework setup | §10 | - |
| T-1.6 | Test application launches without errors | §10.2 | - |
| T-1.7 | Test application exits with code 0 | §10.2 | - |
```

## Phase 2: Core Feature MVP (Example)

**Example Goal**: Add the primary feature in simplest possible form.

**Example MVP Deliverable**:
- Phase 1 MVP + one working core feature
- Feature can be demonstrated/tested
- Feature executes successfully (observable via tests or exit code)

### Example Additions by Project Type

- **CLI Tool**: Add one command that performs basic function
- **Web API**: Add one POST endpoint that accepts input and returns result
- **Data Processor**: Process one simple input file and produce output

## Phase 3+: Incremental Feature Addition (Example)

**Example Goal**: Each phase adds new features to working MVP.

**Example MVP Deliverable**:
- Previous phase MVP + new feature(s)
- All features remain working
- New tests verify new functionality

## Anti-Patterns to Avoid

**Consider avoiding phases like this when they delay a runnable increment:**
```markdown
## Phase 1: Data Models
## Phase 2: Database Setup
## Phase 3: Business Logic
## Phase 4: API Layer
## Phase 5: Tests
```
*Problem: Nothing works until Phase 5!*

**If MVP-first delivery is desired, consider phases like this instead:**
```markdown
## Phase 1: Minimal Working Application
- Entry point that runs successfully
- Status output showing application is alive
- Basic tests verifying startup

## Phase 2: Core Feature MVP
- One complete feature (e.g., user creation)
- Database connection for that feature
- API endpoint exposing that feature
- Tests for the complete flow

## Phase 3: Additional Features
- Add more features to working app
- Each feature fully integrated
- Tests for new features
```
*Success: Working app at end of every phase!*

## Vertical Slice Architecture

MVP-first phasing promotes **vertical slice delivery**:
- Each phase delivers a thin, complete slice through all layers
- Database → Business Logic → API → UI (if applicable)
- Feature is complete and testable end-to-end

**Benefits:**
- Early feedback on working software
- Risk reduction (integration happens continuously)
- Clear progress demonstration
- Easier debugging (smaller increments)
- Team morale (frequent wins)

## Testing in MVP Phases

Each phase must include tests that verify the MVP works:

**Phase 1 Tests (Example):**
- Application launches without errors
- Application exits cleanly with code 0
- Configuration loads successfully

**Phase 2 Tests (Example):**
- Core feature accepts valid input
- Core feature produces expected output
- Core feature handles invalid input gracefully

**Phase 3+ Tests (Example):**
- New feature works correctly
- New feature integrates with existing features
- All previous tests still pass (regression)

## Goal Statement Guidelines

Each phase goal should clearly state:
1. What will work by the end of this phase
2. What can be demonstrated or tested
3. What incremental value this adds

**Good Goal Examples:**
- "Create a working CLI that accepts user input and exits successfully"
- "Add database persistence for user data with CRUD operations"
- "Implement authentication with login/logout functionality"

**Poor Goal Examples:**
- "Set up the database schema" (not runnable)
- "Implement business logic" (too vague)
- "Add models" (no deliverable to demonstrate)

## MVP vs. Polish

**MVP phases focus on:**
- Core functionality that works
- Minimal feature set
- Proving the concept
- Getting to runnable quickly

**Polish phases add:**
- Error handling improvements
- User experience enhancements
- Performance optimizations
- Documentation and help text
- Edge case handling

Both are important, but MVP comes first to establish the working foundation.
