# Test Pyramid Architecture

This document provides detailed guidance on test pyramid architecture, alternative models, and layer-specific strategies.

## The Classic Test Pyramid

```
                    ┌───────────┐
                    │    E2E    │   ← Slow, expensive, high confidence
                    │   Tests   │     Few in number
                    └─────┬─────┘
                          │
                ┌─────────┴─────────┐
                │    Integration    │   ← Medium speed, medium confidence
                │       Tests       │     Moderate number
                └─────────┬─────────┘
                          │
        ┌─────────────────┴─────────────────┐
        │           Unit Tests              │   ← Fast, cheap, low-level
        │      (Foundation of testing)      │     Many in number
        └───────────────────────────────────┘

Ideal Distribution:
  Unit Tests:        70%  (fast feedback, isolated)
  Integration Tests: 20%  (component boundaries)
  E2E Tests:         10%  (critical user flows)
```

### Historical Context

The test pyramid was introduced by Mike Cohn in his book "Succeeding with Agile" (2009). It addresses a common anti-pattern where teams invested heavily in slow, fragile UI tests while neglecting fast, reliable unit tests. The pyramid shape represents both quantity and cost: more tests at the bottom (cheap, fast) and fewer at the top (expensive, slow).

## Test Types by Layer

### End-to-End (E2E) Tests

```
┌─────────────────────────────────────────────────────────────────────┐
│                      END-TO-END (E2E) TESTS                         │
├─────────────────────────────────────────────────────────────────────┤
│ Purpose: Verify complete user workflows work correctly              │
│ Scope: Entire application stack (UI → Backend → Database)           │
│ Examples:                                                           │
│   • User can complete checkout flow                                 │
│   • Search returns relevant results                                 │
│   • Authentication flow works end-to-end                            │
│ Trade-offs:                                                         │
│   ✓ High confidence in real behavior                                │
│   ✗ Slow execution (seconds to minutes)                             │
│   ✗ Flaky due to timing, network, state                             │
│   ✗ Expensive to maintain                                           │
└─────────────────────────────────────────────────────────────────────┘
```

**When to Write E2E Tests:**
- Critical user journeys that directly impact revenue
- Complex workflows that span multiple systems
- Scenarios that can't be adequately covered by lower-level tests
- Happy path validation for major features

**When NOT to Write E2E Tests:**
- Individual function behavior (use unit tests)
- Component integration (use integration tests)
- Error handling scenarios (use lower-level tests)
- Performance testing (use dedicated performance tests)

**E2E Test Design Principles:**
1. **Focus on User Journeys**: Test from the user's perspective
2. **Minimize Test Count**: Only test critical flows
3. **Avoid Redundancy**: If integration tests cover it, skip E2E
4. **Handle Flakiness**: Use retries, waits, and stable selectors
5. **Isolate Test Data**: Use unique data per test run

### Integration Tests

```
┌─────────────────────────────────────────────────────────────────────┐
│                      INTEGRATION TESTS                              │
├─────────────────────────────────────────────────────────────────────┤
│ Purpose: Verify components work together at boundaries              │
│ Scope: Multiple components/services                                 │
│ Examples:                                                           │
│   • API endpoint returns correct response                           │
│   • Database repository saves and retrieves correctly               │
│   • Message queue consumer processes events                         │
│ Trade-offs:                                                         │
│   ✓ Tests real integrations (databases, APIs)                       │
│   ✓ Faster than E2E                                                 │
│   ✗ Slower than unit tests                                          │
│   ✗ Requires infrastructure setup                                   │
└─────────────────────────────────────────────────────────────────────┘
```

**What to Test at Integration Level:**
- API contract adherence (request/response formats)
- Database queries (CRUD operations, transactions)
- Message queue producers/consumers
- File system operations
- External service integrations (with recorded fixtures)
- Middleware and plugin chains

**Integration Test Setup Strategies:**
- **In-Memory Databases**: SQLite for faster tests when possible
- **Containerized Dependencies**: Docker/Testcontainers for real databases
- **Test Fixtures**: Recorded responses for external services
- **Database Transactions**: Rollback after each test for isolation

### Unit Tests

```
┌─────────────────────────────────────────────────────────────────────┐
│                         UNIT TESTS                                  │
├─────────────────────────────────────────────────────────────────────┤
│ Purpose: Verify individual functions/classes work correctly         │
│ Scope: Single function, method, or class                            │
│ Examples:                                                           │
│   • Calculator.add(2, 3) returns 5                                  │
│   • EmailValidator rejects invalid formats                          │
│   • PriceCalculator applies discounts correctly                     │
│ Trade-offs:                                                         │
│   ✓ Fast execution (milliseconds)                                   │
│   ✓ Easy to write and maintain                                      │
│   ✓ Precise failure localization                                    │
│   ✗ Doesn't test integration                                        │
│   ✗ Can miss emergent behaviors                                     │
└─────────────────────────────────────────────────────────────────────┘
```

**What to Unit Test:**
- Business logic and algorithms
- Data transformations and validation
- Calculation and computation functions
- State management logic
- Error handling and edge cases
- Utility functions

**Unit Test Best Practices:**
1. **Test Behavior, Not Implementation**: Focus on inputs/outputs
2. **One Logical Assertion Per Test**: Test one behavior
3. **Use Descriptive Names**: Test name explains what and why
4. **Avoid Mocking Too Much**: If you need many mocks, use integration test
5. **Fast Execution**: Unit tests should run in milliseconds

## Alternative: The Testing Trophy

```
                    ┌───────────────┐
                    │ E2E/Smoke     │   ← Few critical paths
                    └───────────────┘
                ┌───────────────────────┐
                │    Integration Tests  │   ← EMPHASIS HERE
                │    (Component tests)  │     Most value per test
                └───────────────────────┘
            ┌───────────────────────────────┐
            │      Unit Tests               │   ← Complex logic only
            └───────────────────────────────┘
        ┌───────────────────────────────────────┐
        │        Static Analysis                │   ← Type checking, linting
        └───────────────────────────────────────┘

Key Insight: Integration tests often provide better ROI than
extensive unit tests because they test behavior at boundaries.
```

### When to Use Testing Trophy

The testing trophy model, popularized by Kent C. Dodds, emphasizes integration tests over unit tests. Consider this approach when:

- **Frontend Development**: Integration tests (component tests) catch more bugs
- **API Development**: Integration tests verify actual request/response handling
- **Microservices**: Integration tests validate service boundaries
- **Rapid Prototyping**: Integration tests provide confidence faster

**Trophy vs Pyramid Trade-offs:**

| Aspect | Pyramid | Trophy |
|--------|---------|--------|
| Focus | Unit tests | Integration tests |
| Best for | Backend logic | Frontend/APIs |
| Speed | Faster overall | Slightly slower |
| Confidence | Lower per test | Higher per test |
| Maintenance | More tests to maintain | Fewer, broader tests |

### Static Analysis Layer

The testing trophy includes static analysis at the base:

- **Type Checking**: TypeScript, mypy, Flow
- **Linting**: ESLint, Pylint, RuboCop
- **Code Formatting**: Prettier, Black, gofmt
- **Security Scanning**: Bandit, safety, npm audit

**Benefits:**
- Instant feedback (editor integration)
- Catches common errors before runtime
- Enforces code style consistency
- Zero execution time (compile-time checks)

## Test Pyramid with External Services

```
┌─────────────────────────────────────────────────────────────────────┐
│            TEST PYRAMID FOR EXTERNAL SERVICE INTEGRATION            │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│                    ┌───────────────┐                                │
│                    │  Live Service │  ← At least 1 per external API │
│                    │    Smoke      │    (validates contract)        │
│                    └───────┬───────┘                                │
│                            │                                        │
│                ┌───────────┴───────────┐                            │
│                │   Mock Validation     │  ← Runs every CI build     │
│                │   (drift detection)   │    (catches mock drift)    │
│                └───────────┬───────────┘                            │
│                            │                                        │
│            ┌───────────────┴───────────────┐                        │
│            │  Integration Tests            │  ← Uses RECORDED       │
│            │  (recorded fixtures)          │    responses from API  │
│            └───────────────┬───────────────┘                        │
│                            │                                        │
│    ┌───────────────────────┴───────────────────────┐                │
│    │           Unit Tests                          │  ← Synthetic   │
│    │    (synthetic mocks, validated above)         │    mocks OK    │
│    └───────────────────────────────────────────────┘                │
│                                                                     │
│  KEY: The top layer (live smoke) is NON-NEGOTIABLE.                 │
│  Without it, all layers below can pass while the app is broken.     │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

### External Service Integration Layers

1. **Live Service Smoke Tests** (Top Layer)
   - At least ONE test calls the REAL external service
   - Validates response structure matches implementation
   - Run before releases, can skip in routine CI
   - Catches: field renames, structure changes, deprecations
   - Marker: `@pytest.mark.live_api` or similar

2. **Mock Validation Tests** (Second Layer)
   - Compare synthetic mocks against recorded real responses
   - Flag when mock fields don't exist in real API
   - Detect type mismatches (string vs int, etc.)
   - Run in CI to prevent mock drift

3. **Integration Tests with Recorded Fixtures** (Third Layer)
   - Use real API responses captured as test fixtures
   - Test your code's handling of actual API structure
   - Faster than live tests, more realistic than synthetic mocks

4. **Unit Tests with Validated Mocks** (Base Layer)
   - Use synthetic mocks, but only if validated above
   - Fast, isolated tests for business logic
   - Safe because mock accuracy is verified

See [trade-offs.md](./trade-offs.md) for detailed guidance on external API contract testing.

## Choosing Your Distribution

### Factors to Consider

1. **Application Type**
   - Backend API: 70/20/10 (pyramid)
   - Frontend SPA: 50/40/10 (trophy-ish)
   - Microservices: 60/30/10 (pyramid)
   - Monolith: 65/25/10 (pyramid)

2. **Team Experience**
   - Junior team: More integration tests (easier to write)
   - Senior team: More unit tests (better design)

3. **Change Frequency**
   - High change: More unit tests (faster feedback)
   - Stable system: More integration tests (confidence)

4. **Risk Tolerance**
   - Low risk tolerance: More E2E tests
   - High risk tolerance: Fewer E2E tests

### Red Flags

**Too Many E2E Tests (Ice Cream Cone):**
```
┌───────────────────────────────────────┐
│         E2E Tests (Many)              │ ← PROBLEM: Too many
└─────────────────────────────────────┬─┘
    ┌─────────────────────────────────┼───┐
    │     Integration (Some)          │   │
    └─────────────────────────────────┼───┘
        ┌─────────────────────────────┼───────┐
        │         Unit (Few)          │       │ ← PROBLEM: Too few
        └─────────────────────────────┴───────┘
```

**Symptoms:**
- Test suite takes > 15 minutes
- Frequent false failures (flaky tests)
- Tests break on unrelated changes
- Difficult to identify failure root cause

**Too Few Integration Tests (Hourglass):**
```
        ┌─────────────────────────────────────┐
        │      E2E Tests (Some)               │
        └─────────────────────────────────────┘
                      ┌─┐
                      │ │ ← PROBLEM: Missing integration layer
                      └─┘
        ┌─────────────────────────────────────┐
        │      Unit Tests (Many)              │
        └─────────────────────────────────────┘
```

**Symptoms:**
- Unit tests pass but E2E tests fail
- Integration bugs discovered late
- Mocks drift from reality
- False confidence from high unit test coverage

## Tools by Layer

### E2E Test Tools

- **Web Applications**: Playwright, Cypress, Selenium
- **Mobile Applications**: Appium, Detox, XCUITest
- **APIs**: Postman, REST Assured, Supertest
- **Desktop**: TestComplete, WinAppDriver

### Integration Test Tools

- **Test Containers**: Testcontainers (Java/Python/Node)
- **Database**: In-memory databases, Docker containers
- **API Testing**: Supertest, HTTPie, requests
- **Message Queues**: Embedded brokers, Docker containers

### Unit Test Tools

- **JavaScript/TypeScript**: Jest, Vitest, Mocha
- **Python**: pytest, unittest
- **Java**: JUnit, TestNG
- **C#**: NUnit, xUnit
- **Go**: testing package, testify
- **Ruby**: RSpec, Minitest

## Further Reading

- Mike Cohn - "Succeeding with Agile" (original pyramid concept)
- Martin Fowler - "TestPyramid" article
- Kent C. Dodds - "The Testing Trophy" blog post
- Google Testing Blog - "Just Say No to More End-to-End Tests"
