---
name: testing-strategies
description: Designs comprehensive test strategies including test pyramid architecture, coverage goals, trade-offs, and testing methodologies (TDD, BDD). Use when planning test architecture, defining coverage targets, designing test pyramid distribution, choosing testing approaches (unit vs integration vs E2E), establishing testing standards, or evaluating testing trade-offs between speed and coverage.
allowed-tools: Read Glob Grep
---

# Testing Strategies Skill

This skill helps test engineers and developers design comprehensive testing strategies that balance coverage, speed, maintainability, and confidence. Use it to architect test suites that catch bugs efficiently while enabling rapid development.

## ⛔ CRITICAL: Test Duration Restriction

All tests you create MUST complete execution in under 1 minute total runtime.
Tests MUST be short, focused, and fast-executing.
NEVER create tests that:
- Require long-running operations (>60 seconds)

If comprehensive testing requires longer duration, break into multiple focused test files that each run quickly.
Tests exceeding 1 minute duration will cause task FAILURE.

**For test strategy design specifically**:
- Ensure test pyramid layers all respect the 60-second limit
- Design E2E tests as focused, critical-path-only tests (30-45 seconds max)
- Plan integration tests with fast test doubles and fixtures (10-30 seconds)
- Advocate for test suite parallelization to keep CI pipelines fast
- When estimating test execution time, always budget for < 60 seconds per test file

## When to Use This Skill

Use this skill when you need to:
- Design a test strategy for a new project or feature
- Evaluate and improve existing test architecture
- Define test coverage goals and metrics
- Choose between testing approaches (TDD, BDD, ATDD)
- Balance test pyramid distribution (unit, integration, E2E)
- Establish testing standards and best practices
- Analyze testing trade-offs (speed vs coverage, isolation vs realism)
- Plan test automation infrastructure
- Design contract testing for external service integrations

## Purpose

This skill provides a systematic approach to test strategy design:

1. **Test Pyramid Architecture**: Design optimal distribution of unit, integration, and E2E tests
2. **Coverage Strategy**: Define meaningful coverage goals beyond line coverage
3. **Testing Methodologies**: Apply TDD, BDD, or ATDD appropriately
4. **Trade-off Analysis**: Balance competing concerns (speed, coverage, maintenance)
5. **Risk-Based Testing**: Prioritize testing effort based on risk and impact
6. **Contract Testing**: Verify external service integrations against reality

---

## Quick Reference

### Test Pyramid Overview

```
                    ┌───────────┐
                    │    E2E    │   ← 10%: Critical user flows
                    └─────┬─────┘
                ┌─────────┴─────────┐
                │   Integration     │   ← 20%: Component boundaries
                └─────────┬─────────┘
        ┌───────────────┴───────────────┐
        │        Unit Tests              │   ← 70%: Business logic
        └───────────────────────────────┘
```

**See [references/test-pyramid.md](./references/test-pyramid.md) for**:
- Detailed layer descriptions and trade-offs
- Testing Trophy alternative model
- Test pyramid with external services
- Tools by layer
- Anti-patterns (Ice Cream Cone)

### Coverage Goals

```
┌────────────────────────┬──────────┬────────────────────────────────┐
│ Component Type         │ Target   │ Rationale                      │
├────────────────────────┼──────────┼────────────────────────────────┤
│ Business Logic/Domain  │ 90%+     │ Core value, must be reliable   │
│ API Endpoints          │ 80%+     │ Public interface, high impact  │
│ Data Access Layer      │ 70-80%   │ Integration tests more valuable│
│ Controllers/Routes     │ 60-70%   │ Often thin, tested via E2E     │
│ UI Components          │ 50-70%   │ Visual testing often better    │
│ Configuration/Setup    │ 30-50%   │ Environment-dependent          │
└────────────────────────┴──────────┴────────────────────────────────┘
```

**See [references/coverage-strategy.md](./references/coverage-strategy.md) for**:
- Coverage metrics hierarchy (requirement, risk, branch, line)
- Mutation testing detailed guide
- Coverage anti-patterns
- Component-specific strategies

### Testing Methodologies

**Test-Driven Development (TDD)**:
- Red → Green → Refactor cycle
- Best for: Business logic, algorithms, bug fixes
- See [references/methodologies.md](./references/methodologies.md) for detailed TDD workflow

**Behavior-Driven Development (BDD)**:
- Gherkin syntax (Given/When/Then)
- Best for: User-facing features, cross-functional communication
- See [references/methodologies.md](./references/methodologies.md) for BDD examples and tools

**Acceptance Test-Driven Development (ATDD)**:
- Discuss → Distill → Develop
- Best for: Complex requirements, regulatory compliance
- See [references/methodologies.md](./references/methodologies.md) for ATDD process details

### Trade-offs

**Speed vs Confidence**:
- Unit tests: Fast, narrow confidence
- Integration tests: Medium speed, good confidence
- E2E tests: Slow, high confidence

**Isolation vs Realism**:
- More isolation: Fast, deterministic, may not reflect reality
- More realism: Slower, higher confidence, harder to control

**See [references/trade-offs.md](./references/trade-offs.md) for**:
- Speed vs confidence matrix
- Isolation vs realism detailed analysis
- Maintenance cost factors
- Decision frameworks and selection matrices

### External Service Integration

**Critical: External API contract testing prevents mock drift**

```
                    ┌───────────────┐
                    │  Live Smoke   │  ← At least 1 per external API
                    └───────┬───────┘
                ┌───────────┴───────────┐
                │  Mock Validation      │  ← Runs every CI build
                └───────────┬───────────┘
            ┌───────────────┴───────────────┐
            │  Recorded Fixtures            │  ← Real responses
            └───────────────┬───────────────┘
    ┌───────────────────────┴───────────────────┐
    │  Unit Tests (validated mocks)             │
    └───────────────────────────────────────────┘
```

**The Mock Drift Problem**:
- Developer assumes field name from docs
- Creates mock matching assumption
- Tests pass (100% coverage)
- Production breaks (real API uses different field name)

**Solution**: See [references/trade-offs.md](./references/trade-offs.md) for comprehensive external API contract testing strategies including:
- Live service smoke tests
- Recorded response fixtures
- Mock validation tests
- Schema validation approaches

---

## Workflow

When designing a test strategy, follow this workflow:

### 1. Analyze Requirements and Risk

**Questions to ask**:
- What are the critical user journeys?
- Which areas have historically had bugs?
- What are the business-critical components?
- What external services does the system integrate with?

**Output**: Risk matrix identifying high-priority areas for testing

### 2. Design Test Pyramid Distribution

**Steps**:
1. Identify component types (business logic, API, data access, UI)
2. Assign coverage targets based on risk and component type
3. Plan test distribution (typically 70% unit, 20% integration, 10% E2E)
4. Consider Testing Trophy for frontend-heavy applications

**Output**: Planned test distribution across pyramid layers

### 3. Address External Service Integration

**If system integrates with external APIs/services**:
1. Inventory all external services
2. Plan live smoke tests (at least 1 per service)
3. Set up response fixture capture mechanism
4. Design mock validation tests
5. Document pre-release verification steps

**See [references/trade-offs.md](./references/trade-offs.md)** for complete external service testing strategy.

**Output**: Contract testing plan for external services

### 4. Select Testing Methodologies

**Choose based on context**:
- **TDD**: Complex business logic, algorithms
- **BDD**: User-facing features, stakeholder collaboration
- **ATDD**: Sprint planning, acceptance criteria definition

**Output**: Methodology selection for different component types

### 5. Define Coverage Goals

**Steps**:
1. Set component-specific coverage targets
2. Define primary metric (requirement coverage > branch > line)
3. Plan mutation testing for critical paths
4. Establish coverage tracking and reporting

**Output**: Coverage goals and measurement strategy

### 6. Analyze Trade-offs

**Consider**:
- Speed vs confidence balance
- Isolation vs realism for different dependencies
- Maintenance cost implications
- CI pipeline time constraints

**Use decision frameworks from [references/trade-offs.md](./references/trade-offs.md)**

**Output**: Trade-off decisions documented with rationale

### 7. Document Strategy

Create test strategy document including:
- Scope (what's tested, what's not)
- Test pyramid distribution
- Coverage goals by component
- Testing methodologies
- Test types and frameworks
- Risk-based priorities
- Environment strategy
- External service contract testing plan
- Test data strategy

**Template**: See "Test Strategy Template" section below

### 8. Implement and Iterate

**Steps**:
1. Implement tests following the strategy
2. Measure actual coverage and test execution time
3. Review metrics in code reviews
4. Adjust strategy based on findings
5. Track trends over time

**Output**: Living test strategy that evolves with the system

---

## Test Strategy Template

Use this template to document your test strategy:

```markdown
# Test Strategy: [Project/Feature Name]

## 1. Scope
- **In Scope**: [Components/features to test]
- **Out of Scope**: [What won't be tested and why]

## 2. Test Pyramid Distribution
- Unit Tests: [X]%
- Integration Tests: [X]%
- E2E Tests: [X]%
- Rationale: [Why this distribution]

## 3. Coverage Goals
┌────────────────────┬────────┬───────────────────┐
│ Component          │ Target │ Rationale         │
├────────────────────┼────────┼───────────────────┤
│ Business Logic     │ 90%    │ Core domain rules │
│ API Layer          │ 80%    │ Public interface  │
│ Data Access        │ 70%    │ Integration tests │
└────────────────────┴────────┴───────────────────┘

## 4. Testing Methodology
- [ ] TDD for core business logic
- [ ] BDD for user-facing features
- [ ] ATDD for acceptance criteria
- [ ] Exploratory testing for edge cases

## 5. Test Types and Frameworks
- Unit Tests: [Framework, e.g., pytest, Jest]
- Integration Tests: [Framework + tools, e.g., pytest + Testcontainers]
- E2E Tests: [Framework, e.g., Playwright, Cypress]
- Contract Tests: [If applicable]
- Performance Tests: [If applicable]

## 6. Risk-Based Priorities
┌───────────────────┬────────────┬─────────────────┐
│ Feature           │ Risk Level │ Test Investment │
├───────────────────┼────────────┼─────────────────┤
│ Payment           │ Critical   │ Extensive       │
│ Authentication    │ High       │ High            │
│ User Profiles     │ Medium     │ Moderate        │
└───────────────────┴────────────┴─────────────────┘

## 7. External Service Integration Strategy

### External Services Inventory
┌────────────────────┬─────────────┬───────────────┬──────────────────┐
│ Service Name       │ Type        │ Auth Required │ Has OpenAPI Spec │
├────────────────────┼─────────────┼───────────────┼──────────────────┤
│ [Service 1]        │ REST API    │ Yes/No        │ Yes/No           │
│ [Service 2]        │ GraphQL     │ Yes/No        │ Yes/No           │
└────────────────────┴─────────────┴───────────────┴──────────────────┘

### Contract Testing Approach
- [ ] Live smoke tests: At least 1 per external service
- [ ] Recorded fixtures: Real responses captured from each service
- [ ] Mock validation: Synthetic mocks verified against recorded
- [ ] Schema validation: Implemented where specs available

### Pre-Release Verification
- [ ] All live smoke tests pass against real services
- [ ] Recorded fixtures exist for all external services
- [ ] Mock validation tests pass
- [ ] API version compatibility confirmed

## 8. Environment Strategy
- Local: [Setup details, e.g., Docker Compose for dependencies]
- CI: [Pipeline details, when to run which test types]
- Staging: [Pre-production testing approach]

## 9. Test Data Strategy
- Factories: [Approach, e.g., Factory Boy, Faker]
- Fixtures: [Static vs generated]
- Production-like data: [Anonymization approach]

## 10. CI/CD Integration
- Unit/Integration: Run on every commit
- Contract validation: Run on every build
- Live smoke tests: Run before releases
- E2E tests: [When to run]
- Performance tests: [When to run]

## 11. Success Metrics
- Coverage target: [X]% overall
- Test execution time: < [X] minutes
- Flaky test rate: < [X]%
- Defect escape rate: < [X]%
```

---

## Best Practices

### Test Writing

1. **Arrange-Act-Assert (AAA)**: Structure tests clearly
2. **Test Naming**: Use descriptive names ("should X when Y")
3. **One Behavior Per Test**: Focus each test on one thing
4. **Test Independence**: Each test runs in isolation
5. **Avoid Duplication**: Use parameterized tests and fixtures

### Common Anti-Patterns to Avoid

```
❌ Ice Cream Cone: Too many E2E tests, too few unit tests
❌ Testing Implementation: Tests break on refactoring
❌ Flaky Tests: Tests pass/fail non-deterministically
❌ Slow Test Suites: Taking > 10 minutes
❌ Commented-Out Tests: Delete or fix, don't disable
❌ Mock-Only External API Testing: No reality verification
```

See [references/trade-offs.md](./references/trade-offs.md) for detailed anti-pattern analysis.

---

## Question Strategy

When developing test strategies, ask these questions:

### Strategy Design
1. "What are the critical user journeys that must always work?"
2. "Which areas have historically had the most bugs?"
3. "What are the performance and reliability requirements?"
4. "What testing tools and frameworks are already in use?"
5. "What is the acceptable test execution time for CI?"

### Coverage Decisions
1. "What happens if this component fails in production?"
2. "How often does this code change?"
3. "Is this business logic or infrastructure code?"
4. "What types of bugs are we trying to prevent?"

### Trade-off Analysis
1. "What is more important: fast feedback or high confidence?"
2. "How much effort can we invest in test maintenance?"
3. "Should we use mocks or real dependencies here?"

### External Service Integration (CRITICAL)
1. "Does this system integrate with any external APIs or services?"
2. "How will we verify our mocks match the real API response structure?"
3. "Do we have at least one live smoke test per external service?"
4. "Where are recorded response fixtures stored?"
5. "What happens if the external API changes field names?"
6. "Is an OpenAPI/Swagger spec available for schema validation?"
7. "How will we detect mock drift before it reaches production?"

---

## Integration with Other Skills

- **Performance Testing**: Design load tests as part of E2E strategy
- **Security Testing**: Include security tests in test pyramid
- **Chaos Engineering**: Add resilience tests for critical paths
- **Code Review**: Validate test coverage and quality during reviews
- **SDD Generation**: Test strategy informs architecture decisions
- **TTL Generation**: Test tasks included in technical task lists

---

## Reference Documentation

For detailed information, see these reference documents:

- **[test-pyramid.md](./references/test-pyramid.md)**: Comprehensive guide to test pyramid architecture, alternative models, layer details, and tools
- **[coverage-strategy.md](./references/coverage-strategy.md)**: Coverage metrics, goals by component, mutation testing detailed guide
- **[methodologies.md](./references/methodologies.md)**: TDD, BDD, ATDD detailed workflows with examples
- **[trade-offs.md](./references/trade-offs.md)**: Trade-off analysis, decision frameworks, external API contract testing, maintenance strategies

---

## Further Reading

- Martin Fowler - "TestPyramid" and "UnitTest" articles
- Kent Beck - "Test-Driven Development: By Example"
- "Growing Object-Oriented Software, Guided by Tests" by Freeman & Pryce
- Testing Trophy concept by Kent C. Dodds
- Mike Cohn - "Succeeding with Agile" (original pyramid concept)
- J.B. Rainsberger - "Integrated Tests Are A Scam" (talk on trade-offs)
