# Coverage Strategy

This document provides detailed guidance on test coverage metrics, goals, and strategies including mutation testing.

## Coverage Metrics Hierarchy

```
┌─────────────────────────────────────────────────────────────────────┐
│                    COVERAGE METRICS                                 │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  1. Requirement Coverage (Highest Value)                            │
│     "Are all requirements tested?"                                  │
│     └── Each user story has acceptance tests                        │
│     └── Each business rule has verification                         │
│                                                                     │
│  2. Risk Coverage                                                   │
│     "Are high-risk areas well tested?"                              │
│     └── Payment processing has extensive tests                      │
│     └── Security-critical code has thorough coverage                │
│                                                                     │
│  3. Branch Coverage                                                 │
│     "Are all decision paths tested?"                                │
│     └── if/else branches                                            │
│     └── switch cases                                                │
│     └── try/catch paths                                             │
│                                                                     │
│  4. Line Coverage (Lowest Value)                                    │
│     "Is this line executed by tests?"                               │
│     └── Easy to game                                                │
│     └── Doesn't guarantee correctness                               │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘

Coverage Anti-Pattern:
  "We have 95% line coverage!"
  But: No tests verify the output is correct
  Just: Tests execute code without meaningful assertions
```

## Understanding Coverage Metrics

### 1. Requirement Coverage (Most Important)

**Definition**: Percentage of requirements that have automated tests verifying them.

**How to Measure**:
- Track requirements in a traceability matrix
- Link each test to one or more requirements
- Calculate: (Requirements with tests / Total requirements) × 100

**Example**:
```
Requirement: "Users can add items to cart"
Tests:
  ✓ test_add_single_item_to_cart
  ✓ test_add_multiple_items_to_cart
  ✓ test_add_out_of_stock_item_shows_error
  ✓ test_cart_total_updates_after_adding_item
```

**Why It Matters**: This is the only metric that directly measures whether you're testing what you built.

### 2. Risk Coverage

**Definition**: Testing effort allocated based on risk assessment.

**Risk Factors**:
- Business impact (revenue, reputation)
- Complexity (cyclomatic complexity, dependencies)
- Change frequency (code churn)
- Historical defect density
- Criticality (safety, security, compliance)

**Risk-Based Testing Formula**:
```
Test Investment = Risk Score × Coverage Multiplier

Risk Score = (Impact × Likelihood × Complexity) / Change Stability

Where:
  Impact: 1-5 (1=low, 5=critical)
  Likelihood: 1-5 (1=rare, 5=frequent)
  Complexity: 1-5 (1=simple, 5=complex)
  Change Stability: 1-5 (1=changes frequently, 5=stable)
```

**Example Risk Matrix**:
```
┌─────────────────┬────────┬────────────┬──────────┬─────────────┐
│ Component       │ Impact │ Likelihood │ Risk     │ Coverage    │
├─────────────────┼────────┼────────────┼──────────┼─────────────┤
│ Payment         │ 5      │ 4          │ Critical │ 95%+        │
│ Authentication  │ 5      │ 3          │ High     │ 90%+        │
│ Search          │ 3      │ 4          │ Medium   │ 75%+        │
│ User Profile    │ 2      │ 2          │ Low      │ 60%+        │
│ Admin Dashboard │ 2      │ 1          │ Low      │ 50%+        │
└─────────────────┴────────┴────────────┴──────────┴─────────────┘
```

### 3. Branch Coverage

**Definition**: Percentage of decision branches (if/else, switch) executed by tests.

**Formula**: (Executed Branches / Total Branches) × 100

**Example**:
```python
def calculate_discount(price, customer_type):
    if customer_type == "premium":      # Branch 1
        return price * 0.8
    elif customer_type == "regular":    # Branch 2
        return price * 0.9
    else:                               # Branch 3
        return price

# Full branch coverage requires testing all 3 paths
test_premium_customer()    # Covers branch 1
test_regular_customer()    # Covers branch 2
test_guest_customer()      # Covers branch 3
```

**Branch vs Line Coverage**:
```python
# This has 100% line coverage but only 50% branch coverage:
def divide(a, b):
    if b != 0:              # Only true branch tested
        return a / b
    return None

test_divide_success()       # Line coverage: 100%
                            # Branch coverage: 50%

# Adding this test achieves 100% branch coverage:
test_divide_by_zero()       # Branch coverage: 100%
```

### 4. Line Coverage

**Definition**: Percentage of code lines executed by tests.

**Formula**: (Executed Lines / Total Executable Lines) × 100

**Why It's Misleading**:
```python
# Example: 100% line coverage, but no assertions
def test_user_creation():
    user = create_user("Alice", "alice@example.com")
    # Line is executed, but we don't verify it worked!
    # This test has coverage but provides zero value
```

**Better Approach**:
```python
def test_user_creation():
    user = create_user("Alice", "alice@example.com")
    assert user.name == "Alice"
    assert user.email == "alice@example.com"
    assert user.id is not None
    # Now we're actually testing behavior, not just executing code
```

## Coverage Goals by Component

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
│ Generated Code         │ Skip     │ Not worth testing              │
└────────────────────────┴──────────┴────────────────────────────────┘

Key Principle: High coverage in high-value areas > uniform coverage
```

### Component-Specific Strategies

#### Business Logic/Domain (90%+ target)

**What to Test**:
- All business rules and calculations
- State transitions
- Domain invariants
- Complex algorithms

**Example**:
```typescript
// Order domain logic - requires 90%+ coverage
class Order {
  calculateTotal(): Money {
    // Complex tax calculations
    // Discount application logic
    // Shipping cost algorithms
  }

  canBeShipped(): boolean {
    // Multiple business rules
  }
}

// Every branch, edge case, and business rule must be tested
```

#### API Endpoints (80%+ target)

**What to Test**:
- Request validation
- Response format
- Status codes
- Error handling
- Authentication/authorization

**Example**:
```python
# Test all endpoint behaviors
def test_create_order_success():
    response = client.post("/api/orders", json=valid_order)
    assert response.status_code == 201
    assert "id" in response.json()

def test_create_order_invalid_data():
    response = client.post("/api/orders", json=invalid_order)
    assert response.status_code == 400
    assert "error" in response.json()

def test_create_order_unauthorized():
    response = client.post("/api/orders")  # No auth token
    assert response.status_code == 401
```

#### Data Access Layer (70-80% target)

**What to Test**:
- CRUD operations
- Transactions
- Constraints
- Query correctness

**Why Not Higher**: Integration tests are more valuable than unit tests for database code.

```python
# Integration test (preferred for DAL)
def test_user_repository_save_and_retrieve(db_session):
    user = User(name="Alice", email="alice@example.com")
    repository.save(user)

    retrieved = repository.find_by_id(user.id)
    assert retrieved.name == "Alice"
    assert retrieved.email == "alice@example.com"
```

#### Controllers/Routes (60-70% target)

**Why Lower**: Controllers are often thin routing logic, tested indirectly via API integration tests.

```typescript
// Controller (thin, may not need direct unit tests)
@Controller("/users")
class UserController {
  async create(req, res) {
    const user = await this.userService.create(req.body);
    return res.status(201).json(user);
  }
}

// Test via integration test instead:
// POST /users → verify service is called → verify response
```

#### UI Components (50-70% target)

**What to Test**:
- Component behavior (events, state)
- Accessibility
- Edge cases

**Why Not Higher**: Visual appearance often requires manual/visual testing.

```jsx
// Test component behavior
test("button click increments counter", () => {
  render(<Counter />);
  const button = screen.getByRole("button");
  fireEvent.click(button);
  expect(screen.getByText("Count: 1")).toBeInTheDocument();
});

// Visual appearance tested via:
// - Visual regression testing (Chromatic, Percy)
// - Manual QA
```

#### Configuration/Setup (30-50% target)

**Why Lower**: Configuration is often environment-specific and difficult to test in isolation.

```python
# Configuration validation
def test_config_has_required_values():
    config = load_config()
    assert config.DATABASE_URL is not None
    assert config.API_KEY is not None

# Don't over-test configuration parsing
# Focus on validation and defaults
```

## Mutation Testing

```
┌─────────────────────────────────────────────────────────────────────┐
│                      MUTATION TESTING                               │
│              (Testing the quality of your tests)                    │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  How it works:                                                      │
│  1. Automatically mutate code (flip operators, remove lines)        │
│  2. Run tests against mutated code                                  │
│  3. If tests still pass → test suite has gaps                       │
│  4. If tests fail → test suite detected the mutation                │
│                                                                     │
│  Example Mutations:                                                 │
│  ┌────────────────────────┬──────────────────────────────┐          │
│  │ Original               │ Mutated                      │          │
│  ├────────────────────────┼──────────────────────────────┤          │
│  │ if (x > 10)            │ if (x >= 10)                 │          │
│  │ return a + b           │ return a - b                 │          │
│  │ return true            │ return false                 │          │
│  │ array.push(item)       │ // removed                   │          │
│  └────────────────────────┴──────────────────────────────┘          │
│                                                                     │
│  Mutation Score = Killed Mutants / Total Mutants                    │
│  Target: 70-80% mutation score                                      │
│                                                                     │
│  Tools: Stryker (JS), PITest (Java), mutmut (Python)                │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

### Understanding Mutation Testing

**Purpose**: Verify that your tests actually test something meaningful, not just execute code.

**Example Problem**:
```python
# Original code
def calculate_total(items):
    return sum(item.price for item in items)

# Bad test (passes but doesn't verify result)
def test_calculate_total():
    items = [Item(price=10), Item(price=20)]
    calculate_total(items)  # No assertion!
    # Coverage: 100% ✓
    # Mutation score: 0% ✗
```

**Mutation introduces bugs**:
```python
# Mutated code (mutation: sum → len)
def calculate_total(items):
    return len(item.price for item in items)  # BUG INTRODUCED

# Bad test still passes (no assertions)
test_calculate_total()  # ✓ PASSES (BAD!)
```

**Good test (catches mutation)**:
```python
def test_calculate_total():
    items = [Item(price=10), Item(price=20)]
    total = calculate_total(items)
    assert total == 30  # Assertion catches mutated code
    # Coverage: 100% ✓
    # Mutation score: 100% ✓
```

### Mutation Operators

**Arithmetic Operator Replacement**:
```
+ → -, ×, ÷
- → +, ×, ÷
× → +, -, ÷
÷ → +, -, ×
```

**Relational Operator Replacement**:
```
>  → >=, <, <=, ==, !=
<  → <=, >, >=, ==, !=
>= → >, <, <=, ==, !=
<= → <, >, >=, ==, !=
== → !=, <, >, <=, >=
!= → ==, <, >, <=, >=
```

**Logical Operator Replacement**:
```
&& → ||
|| → &&
!  → (remove)
```

**Statement Deletion**:
```
Remove return statements
Remove method calls
Remove variable assignments
```

**Constant Replacement**:
```
0 → 1, -1
1 → 0, 2
true → false
false → true
"" → "mutation"
```

### Interpreting Mutation Scores

**Mutation Score Ranges**:

| Score | Interpretation | Action |
|-------|----------------|--------|
| 90-100% | Excellent | Maintain quality |
| 80-89% | Good | Minor improvements |
| 70-79% | Acceptable | Identify gaps |
| 60-69% | Needs work | Add assertions |
| <60% | Poor | Major test gaps |

**Common Causes of Low Mutation Scores**:

1. **Missing Assertions**:
```python
# Low mutation score
def test_user_login():
    user = login("alice", "password")
    # Missing: assert user is not None

# High mutation score
def test_user_login():
    user = login("alice", "password")
    assert user is not None
    assert user.username == "alice"
```

2. **Weak Assertions**:
```python
# Low mutation score
def test_calculate_discount():
    result = calculate_discount(100, 0.1)
    assert result > 0  # Too weak

# High mutation score
def test_calculate_discount():
    result = calculate_discount(100, 0.1)
    assert result == 90.0  # Precise assertion
```

3. **Untested Edge Cases**:
```python
# Tests only happy path → low mutation score
def test_divide():
    assert divide(10, 2) == 5

# Tests edge cases → high mutation score
def test_divide():
    assert divide(10, 2) == 5
    assert divide(10, 3) == pytest.approx(3.33)
    assert divide(10, 0) is None  # Edge case
```

### Mutation Testing Workflow

```
1. Establish Baseline
   ├─ Run existing tests
   ├─ Verify all tests pass
   └─ Measure current coverage

2. Run Mutation Testing
   ├─ Generate mutants
   ├─ Run tests against each mutant
   └─ Calculate mutation score

3. Analyze Survivors
   ├─ Identify mutants that survived (tests didn't catch)
   ├─ Review why tests didn't fail
   └─ Categorize: missing tests, weak assertions, or equivalent mutants

4. Improve Tests
   ├─ Add missing test cases
   ├─ Strengthen assertions
   └─ Cover edge cases

5. Re-run and Verify
   ├─ Run mutation testing again
   ├─ Verify mutation score improved
   └─ Commit improved tests
```

### Equivalent Mutants Problem

**Issue**: Some mutations don't change program behavior (equivalent mutants).

**Example**:
```python
# Original
def is_positive(x):
    return x > 0

# Mutated (equivalent - no behavior change)
def is_positive(x):
    return x >= 1  # Logically equivalent for integers
```

**Solution**: Manually mark equivalent mutants or accept slightly lower mutation score.

### Mutation Testing Tools

**JavaScript/TypeScript**:
- **Stryker**: `npm install --save-dev @stryker-mutator/core`
- Configuration: `stryker.conf.json`
- Supports: Jest, Jasmine, Mocha

**Python**:
- **mutmut**: `pip install mutmut`
- Usage: `mutmut run`
- Reports: `mutmut results`, `mutmut html`

**Java**:
- **PITest**: Maven/Gradle plugin
- Industry standard for Java mutation testing

**C#**:
- **Stryker.NET**: `dotnet tool install -g dotnet-stryker`

**Go**:
- **go-mutesting**: `go get github.com/zimmski/go-mutesting`

### Mutation Testing Best Practices

1. **Start with Critical Code**: Run mutation testing on high-risk components first
2. **Incremental Adoption**: Don't try to achieve 80% mutation score overnight
3. **CI Integration**: Run mutation testing in CI (but may be slow)
4. **Parallel Execution**: Use parallel execution to speed up mutation testing
5. **Focus on Surviving Mutants**: Prioritize fixing tests that let mutants survive
6. **Document Equivalent Mutants**: Keep a list of known equivalent mutants

## Coverage Strategy Checklist

- [ ] Define requirement coverage tracking mechanism
- [ ] Perform risk assessment for all components
- [ ] Set component-specific coverage targets
- [ ] Establish baseline line/branch coverage
- [ ] Introduce mutation testing for critical paths
- [ ] Review coverage reports in code reviews
- [ ] Track coverage trends over time
- [ ] Don't sacrifice test quality for coverage numbers

## Further Reading

- Martin Fowler - "TestCoverage" article
- Robert C. Martin - "The Clean Coder" (Chapter 7: Acceptance Testing)
- Mutation Testing: "PITest" documentation
- IEEE - "Mutation Testing: Challenges and Opportunities"
