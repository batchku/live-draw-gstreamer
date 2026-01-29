# Testing Methodologies

This document provides detailed guidance on Test-Driven Development (TDD), Behavior-Driven Development (BDD), and Acceptance Test-Driven Development (ATDD).

## Test-Driven Development (TDD)

```
┌─────────────────────────────────────────────────────────────────────┐
│                    TDD CYCLE (Red-Green-Refactor)                   │
└─────────────────────────────────────────────────────────────────────┘

        ┌──────────────┐
        │   1. RED     │     Write a failing test first
        │  (Write test)│     Test should fail because
        └───────┬──────┘     feature doesn't exist yet
                │
                ▼
        ┌──────────────┐
        │  2. GREEN    │     Write minimal code to pass
        │ (Make pass)  │     Don't optimize, just pass
        └───────┬──────┘
                │
                ▼
        ┌──────────────┐
        │ 3. REFACTOR  │     Clean up code while tests pass
        │   (Clean)    │     Improve design, remove duplication
        └───────┬──────┘
                │
                └────────────────────────────────────┐
                                                     │
                                              ┌──────▼──────┐
                                              │   REPEAT    │
                                              │             │
                                              └─────────────┘

TDD Benefits:
  ✓ Tests exist before code
  ✓ Forces testable design
  ✓ High coverage naturally
  ✓ Documentation through tests
  ✓ Confidence in changes

TDD Challenges:
  ✗ Slower initial development
  ✗ Learning curve
  ✗ Over-testing implementation details
  ✗ Requires discipline
```

### The Red-Green-Refactor Cycle

#### 1. RED: Write a Failing Test

**Purpose**: Define expected behavior before implementation.

**Rules**:
- Test should fail for the right reason (feature not implemented)
- Test should be minimal and focused
- Test describes one specific behavior

**Example (Python)**:
```python
# Step 1: RED - Write failing test
def test_calculate_total_with_tax():
    cart = ShoppingCart()
    cart.add_item(Item(name="Book", price=10.00))

    # This will fail - calculate_total_with_tax doesn't exist yet
    total = cart.calculate_total_with_tax(tax_rate=0.1)

    assert total == 11.00


# Run test → FAIL ✗
# AttributeError: 'ShoppingCart' object has no attribute 'calculate_total_with_tax'
```

#### 2. GREEN: Write Minimal Code to Pass

**Purpose**: Make the test pass with the simplest possible implementation.

**Rules**:
- Write just enough code to pass the test
- Don't optimize or add extra features
- Ignore code quality temporarily (will refactor next)

**Example (Python)**:
```python
# Step 2: GREEN - Make test pass
class ShoppingCart:
    def __init__(self):
        self.items = []

    def add_item(self, item):
        self.items.append(item)

    def calculate_total_with_tax(self, tax_rate):
        subtotal = sum(item.price for item in self.items)
        return subtotal * (1 + tax_rate)


# Run test → PASS ✓
```

**Common Mistake**: Don't implement more than needed.
```python
# ❌ DON'T: Adding features not tested
def calculate_total_with_tax(self, tax_rate, discount=0):  # Discount not tested!
    # ...

# ✓ DO: Only what's tested
def calculate_total_with_tax(self, tax_rate):
    # ...
```

#### 3. REFACTOR: Improve Code Quality

**Purpose**: Improve code design while tests ensure correctness.

**Rules**:
- Tests must remain green during refactoring
- Remove duplication
- Improve names and structure
- Apply design patterns if beneficial

**Example (Python)**:
```python
# Step 3: REFACTOR - Improve design
class ShoppingCart:
    def __init__(self):
        self.items = []

    def add_item(self, item):
        self.items.append(item)

    def calculate_subtotal(self):
        """Extract method for reusability"""
        return sum(item.price for item in self.items)

    def calculate_total_with_tax(self, tax_rate):
        """Use extracted method"""
        return self.calculate_subtotal() * (1 + tax_rate)


# Run tests → PASS ✓ (refactoring didn't break anything)
```

### TDD in Practice

#### Full TDD Example: Password Validator

**Requirement**: Implement password validator that requires:
- At least 8 characters
- At least one uppercase letter
- At least one number

**Iteration 1: Length Requirement**
```python
# RED
def test_password_too_short():
    validator = PasswordValidator()
    assert not validator.is_valid("abc123")  # Only 6 characters

# GREEN
class PasswordValidator:
    def is_valid(self, password):
        return len(password) >= 8

# Test passes ✓
```

**Iteration 2: Uppercase Requirement**
```python
# RED
def test_password_missing_uppercase():
    validator = PasswordValidator()
    assert not validator.is_valid("abcdefgh")  # No uppercase

# GREEN
class PasswordValidator:
    def is_valid(self, password):
        has_uppercase = any(c.isupper() for c in password)
        return len(password) >= 8 and has_uppercase

# Test passes ✓
```

**Iteration 3: Number Requirement**
```python
# RED
def test_password_missing_number():
    validator = PasswordValidator()
    assert not validator.is_valid("Abcdefgh")  # No number

# GREEN
class PasswordValidator:
    def is_valid(self, password):
        has_uppercase = any(c.isupper() for c in password)
        has_number = any(c.isdigit() for c in password)
        return len(password) >= 8 and has_uppercase and has_number

# Test passes ✓
```

**Iteration 4: REFACTOR**
```python
# REFACTOR - Extract validation rules
class PasswordValidator:
    MIN_LENGTH = 8

    def is_valid(self, password):
        return (
            self._has_min_length(password) and
            self._has_uppercase(password) and
            self._has_number(password)
        )

    def _has_min_length(self, password):
        return len(password) >= self.MIN_LENGTH

    def _has_uppercase(self, password):
        return any(c.isupper() for c in password)

    def _has_number(self, password):
        return any(c.isdigit() for c in password)

# All tests still pass ✓
```

### When to Use TDD

**TDD Works Best For**:
- Business logic and algorithms
- Complex conditional logic
- Data transformations
- Utility functions
- Bug fixes (write test that reproduces bug, then fix)

**TDD Is Challenging For**:
- UI/visual components (hard to specify precisely)
- Exploratory coding (you don't know requirements yet)
- Third-party API integration (external dependencies)
- Performance-critical code (optimization conflicts with simplicity)

### TDD Anti-Patterns

#### 1. Testing Implementation Details

**Problem**: Tests break when refactoring, even though behavior is unchanged.

```python
# ❌ BAD: Testing internal implementation
def test_user_repository_uses_database_connection():
    repo = UserRepository()
    user = repo.find_by_id(1)
    assert repo._db_connection.execute.called  # Tied to implementation

# ✓ GOOD: Testing behavior
def test_user_repository_finds_user_by_id():
    repo = UserRepository()
    user = repo.find_by_id(1)
    assert user.id == 1
    assert user.name == "Alice"
```

#### 2. One Test at a Time Violation

**Problem**: Writing multiple tests before making any pass.

```python
# ❌ BAD: Writing all tests first
def test_add(): ...
def test_subtract(): ...
def test_multiply(): ...
def test_divide(): ...
# Now implement all four operations

# ✓ GOOD: One test at a time
def test_add(): ...
# Implement add()
# Move to next test
def test_subtract(): ...
# Implement subtract()
# Continue...
```

#### 3. Large Test Jumps

**Problem**: Test is too ambitious, requires too much implementation.

```python
# ❌ BAD: Test requires entire system
def test_user_can_complete_checkout():
    # Requires: cart, payment, inventory, email, database...
    # Too much to implement in one step

# ✓ GOOD: Incremental tests
def test_cart_accepts_items():
    cart = Cart()
    cart.add(item)
    assert len(cart) == 1

# Then incrementally add more functionality
```

---

## Behavior-Driven Development (BDD)

```
┌─────────────────────────────────────────────────────────────────────┐
│                    BDD (Gherkin Syntax)                             │
└─────────────────────────────────────────────────────────────────────┘

Feature: Shopping Cart
  As a customer
  I want to add items to my cart
  So that I can purchase them later

  Scenario: Add single item to empty cart
    Given I have an empty shopping cart
    When I add a "Widget" priced at $10.00
    Then my cart should contain 1 item
    And my cart total should be $10.00

  Scenario: Add multiple items
    Given I have an empty shopping cart
    When I add a "Widget" priced at $10.00
    And I add a "Gadget" priced at $20.00
    Then my cart should contain 2 items
    And my cart total should be $30.00

┌─────────────────────────────────────────────────────────────────────┐
│ BDD Mapping:                                                        │
│   Given → Setup (Arrange)                                           │
│   When  → Action (Act)                                              │
│   Then  → Assertion (Assert)                                        │
└─────────────────────────────────────────────────────────────────────┘

BDD Benefits:
  ✓ Living documentation
  ✓ Shared language with stakeholders
  ✓ Focus on behavior, not implementation
  ✓ Scenarios drive development

BDD Tools:
  • Cucumber (multi-language)
  • SpecFlow (.NET)
  • Behave (Python)
  • Jest-Cucumber (JavaScript)
```

### Gherkin Syntax

**Structure**:
```gherkin
Feature: [Feature name]
  [Optional description]

  Scenario: [Scenario name]
    Given [precondition]
    When [action]
    Then [expected result]
    And [additional condition/result]
```

**Keywords**:
- **Feature**: High-level description of functionality
- **Scenario**: Specific example of feature behavior
- **Given**: Sets up initial state
- **When**: Performs action
- **Then**: Verifies outcome
- **And/But**: Chains additional steps

### BDD Example: User Authentication

**Feature File** (`features/authentication.feature`):
```gherkin
Feature: User Authentication
  As a registered user
  I want to log in to the application
  So that I can access my account

  Scenario: Successful login with valid credentials
    Given I am a registered user with username "alice" and password "secret123"
    When I enter username "alice" and password "secret123"
    And I click the "Login" button
    Then I should see the homepage
    And I should see "Welcome, Alice"

  Scenario: Failed login with invalid password
    Given I am a registered user with username "alice" and password "secret123"
    When I enter username "alice" and password "wrongpassword"
    And I click the "Login" button
    Then I should see an error message "Invalid credentials"
    And I should remain on the login page

  Scenario: Failed login with non-existent user
    Given there is no user with username "bob"
    When I enter username "bob" and password "anypassword"
    And I click the "Login" button
    Then I should see an error message "Invalid credentials"
```

**Step Definitions** (Python/Behave):
```python
from behave import given, when, then

@given('I am a registered user with username "{username}" and password "{password}"')
def step_registered_user(context, username, password):
    context.user = User.create(username=username, password=password)

@when('I enter username "{username}" and password "{password}"')
def step_enter_credentials(context, username, password):
    context.login_form = LoginForm()
    context.login_form.username = username
    context.login_form.password = password

@when('I click the "{button}" button')
def step_click_button(context, button):
    context.response = context.login_form.submit()

@then('I should see the homepage')
def step_see_homepage(context):
    assert context.response.url == "/home"

@then('I should see "{text}"')
def step_see_text(context, text):
    assert text in context.response.body
```

### BDD Best Practices

#### 1. Use Business Language, Not Technical Terms

```gherkin
# ❌ BAD: Technical implementation details
Given the database table "users" has a row with id=1
When the API endpoint "/api/users/1" receives a GET request
Then the response status code should be 200

# ✓ GOOD: Business language
Given a user named "Alice" exists in the system
When I view Alice's profile
Then I should see Alice's information
```

#### 2. Focus on Behavior, Not Data

```gherkin
# ❌ BAD: Too much data detail
Given a product with SKU "WIDGET-001", name "Blue Widget", price 10.99, category "Widgets"
When I add the product to cart
Then the cart contains product SKU "WIDGET-001"

# ✓ GOOD: Focus on behavior
Given a product is available for purchase
When I add the product to my cart
Then my cart should contain the product
```

#### 3. One Scenario Per Behavior

```gherkin
# ❌ BAD: Multiple unrelated behaviors
Scenario: User operations
  Given I am logged in
  When I update my profile
  Then my profile is saved
  When I log out
  Then I am logged out
  When I try to access my profile
  Then I am redirected to login

# ✓ GOOD: Separate scenarios
Scenario: Update profile
  Given I am logged in
  When I update my profile
  Then my profile is saved

Scenario: Access profile requires login
  Given I am not logged in
  When I try to access my profile
  Then I am redirected to login
```

#### 4. Use Scenario Outlines for Examples

```gherkin
# ❌ BAD: Duplicate scenarios for different inputs
Scenario: Discount for $100 order
  Given my cart total is $100
  When I apply a 10% discount
  Then my total should be $90

Scenario: Discount for $200 order
  Given my cart total is $200
  When I apply a 10% discount
  Then my total should be $180

# ✓ GOOD: Scenario Outline with examples
Scenario Outline: Apply discount to cart
  Given my cart total is <original_price>
  When I apply a <discount>% discount
  Then my total should be <final_price>

  Examples:
    | original_price | discount | final_price |
    | $100          | 10       | $90         |
    | $200          | 10       | $180        |
    | $150          | 20       | $120        |
```

### When to Use BDD

**BDD Works Best For**:
- User-facing features
- Complex business rules
- Cross-functional team communication
- Acceptance criteria definition
- Regulatory compliance (traceable requirements)

**BDD Is Overkill For**:
- Internal utility functions
- Low-level technical components
- Simple CRUD operations
- Performance optimizations

---

## Acceptance Test-Driven Development (ATDD)

```
┌─────────────────────────────────────────────────────────────────────┐
│                    ATDD WORKFLOW                                    │
└─────────────────────────────────────────────────────────────────────┘

┌───────────────┐     ┌───────────────┐     ┌───────────────┐
│   DISCUSS     │────►│   DISTILL     │────►│   DEVELOP     │
│               │     │               │     │               │
│ Collaborate   │     │ Write tests   │     │ Implement     │
│ on examples   │     │ from examples │     │ to pass tests │
└───────────────┘     └───────────────┘     └───────────────┘

Participants:
  • Developer (implementation)
  • Tester (edge cases, quality)
  • Business (requirements, acceptance)

Example Workflow:
  1. DISCUSS: Team reviews user story
     "User can filter products by price range"

  2. DISTILL: Create acceptance tests
     - Filter shows products within range
     - Filter excludes products outside range
     - Empty result shows helpful message
     - Edge: min > max shows error

  3. DEVELOP: Implement feature
     - Code written to satisfy tests
     - Tests serve as specification
```

### ATDD Process in Detail

#### Phase 1: DISCUSS (Collaborative Example Definition)

**Participants**: Product Owner, Developer, Tester

**Activities**:
1. Review user story
2. Identify acceptance criteria
3. Create concrete examples
4. Discover edge cases
5. Clarify ambiguities

**Example Discussion**:
```
Story: "Users can search for products"

PO: "Users should be able to search by product name"
Dev: "Should it be case-sensitive?"
PO: "No, case-insensitive"
Tester: "What if search term is empty?"
PO: "Show all products"
Tester: "What if no results found?"
PO: "Show 'No products found' message"
Dev: "Should we support partial matching?"
PO: "Yes, substring matching"

Examples agreed:
1. Search "widget" finds "Blue Widget"
2. Search "WIDGET" finds "Blue Widget" (case-insensitive)
3. Search "" returns all products
4. Search "xyz999" shows "No products found"
5. Search "wid" finds "Blue Widget" (partial match)
```

#### Phase 2: DISTILL (Convert to Acceptance Tests)

**Activities**:
1. Convert examples to executable tests
2. Define test data
3. Specify expected outcomes
4. Automate acceptance criteria

**Acceptance Tests** (Python):
```python
class TestProductSearch:
    def test_search_finds_product_by_exact_name(self):
        # Setup
        create_product(name="Blue Widget")

        # Execute
        results = search_products("Blue Widget")

        # Verify
        assert len(results) == 1
        assert results[0].name == "Blue Widget"

    def test_search_is_case_insensitive(self):
        create_product(name="Blue Widget")

        results = search_products("blue widget")

        assert len(results) == 1

    def test_empty_search_returns_all_products(self):
        create_product(name="Widget A")
        create_product(name="Widget B")

        results = search_products("")

        assert len(results) == 2

    def test_no_results_shows_helpful_message(self):
        results = search_products("xyz999")

        assert len(results) == 0
        # UI would display "No products found"

    def test_partial_match_finds_products(self):
        create_product(name="Blue Widget")

        results = search_products("wid")

        assert len(results) == 1
```

#### Phase 3: DEVELOP (Implement to Pass Tests)

**Activities**:
1. Run tests (all fail - RED)
2. Implement minimal code to pass
3. Refactor
4. Verify all acceptance tests pass

**Implementation**:
```python
def search_products(query: str) -> List[Product]:
    """Search products by name (case-insensitive, partial match)"""
    if query == "":
        return Product.objects.all()

    return Product.objects.filter(
        name__icontains=query  # Case-insensitive partial match
    )
```

### ATDD vs TDD vs BDD

| Aspect | TDD | BDD | ATDD |
|--------|-----|-----|------|
| Focus | Code design | Behavior specification | Acceptance criteria |
| Audience | Developers | Developers + Stakeholders | Entire team |
| Language | Code | Business language (Gherkin) | Both |
| Granularity | Unit level | Feature level | Story level |
| When | During coding | Before/during coding | Before coding (sprint planning) |
| Output | Unit tests | Executable specs | Acceptance tests |

### ATDD Example: E-commerce Checkout

**Sprint Planning (DISCUSS)**:
```
Story: "Customer can complete checkout with credit card"

Acceptance Criteria Discussed:
1. Valid card is accepted
2. Invalid card is rejected
3. Insufficient funds are handled
4. Order is created on success
5. Email confirmation is sent

Edge Cases Identified:
- Expired card
- Card declined
- Network timeout
- Duplicate submission (idempotency)
```

**Acceptance Tests (DISTILL)**:
```python
class TestCheckout:
    def test_valid_card_completes_checkout(self):
        cart = create_cart_with_items()
        card = valid_credit_card()

        order = checkout(cart, card)

        assert order.status == "completed"
        assert order.total == cart.total
        assert_email_sent(order.customer_email)

    def test_invalid_card_rejects_checkout(self):
        cart = create_cart_with_items()
        card = invalid_credit_card()

        with pytest.raises(PaymentError):
            checkout(cart, card)

    def test_expired_card_rejects_checkout(self):
        cart = create_cart_with_items()
        card = expired_credit_card()

        with pytest.raises(PaymentError) as exc:
            checkout(cart, card)
        assert "expired" in str(exc.value).lower()

    def test_declined_card_rejects_checkout(self):
        cart = create_cart_with_items()
        card = valid_credit_card()

        with mock.patch('payment_gateway.charge', side_effect=DeclinedError):
            with pytest.raises(PaymentError) as exc:
                checkout(cart, card)
            assert "declined" in str(exc.value).lower()

    def test_duplicate_submission_is_idempotent(self):
        cart = create_cart_with_items()
        card = valid_credit_card()

        order1 = checkout(cart, card, idempotency_key="key123")
        order2 = checkout(cart, card, idempotency_key="key123")

        assert order1.id == order2.id  # Same order returned
        assert_charged_once(card)       # Not charged twice
```

**Implementation (DEVELOP)**:
```python
def checkout(cart, card, idempotency_key=None):
    # Check for duplicate submission
    if idempotency_key:
        existing_order = Order.objects.filter(
            idempotency_key=idempotency_key
        ).first()
        if existing_order:
            return existing_order

    # Validate card
    if not card.is_valid():
        raise PaymentError("Invalid card")

    if card.is_expired():
        raise PaymentError("Card expired")

    # Process payment
    try:
        payment_gateway.charge(card, cart.total)
    except DeclinedError:
        raise PaymentError("Card declined")

    # Create order
    order = Order.objects.create(
        customer=cart.customer,
        items=cart.items,
        total=cart.total,
        status="completed",
        idempotency_key=idempotency_key
    )

    # Send confirmation
    send_email(order.customer_email, "Order Confirmation", order)

    return order
```

### ATDD Best Practices

1. **Write Acceptance Tests Before Implementation**: Tests define "done"
2. **Involve All Three Roles**: Developer, Tester, Business
3. **Use Concrete Examples**: Real data, not abstract descriptions
4. **Automate Acceptance Tests**: Manual checks don't scale
5. **Keep Tests Independent**: Each test can run alone
6. **Review Tests in Sprint Planning**: Ensure shared understanding

### When to Use ATDD

**ATDD Works Best For**:
- Complex business requirements
- Cross-functional features
- Regulatory/compliance requirements
- High-risk features
- Features with unclear requirements

**Skip ATDD For**:
- Simple CRUD operations
- Internal refactoring
- Technical debt cleanup
- Performance improvements

## Choosing a Methodology

```
┌───────────────────┬────────────┬────────────┬────────────┐
│ Scenario          │ Use TDD    │ Use BDD    │ Use ATDD   │
├───────────────────┼────────────┼────────────┼────────────┤
│ Algorithm logic   │ ✓✓✓        │            │            │
│ Business rules    │ ✓✓         │ ✓✓✓        │ ✓✓         │
│ User workflows    │            │ ✓✓✓        │ ✓✓✓        │
│ API development   │ ✓✓         │ ✓✓         │ ✓          │
│ UI features       │            │ ✓✓         │ ✓✓✓        │
│ Acceptance criteria│           │ ✓✓         │ ✓✓✓        │
│ Sprint planning   │            │            │ ✓✓✓        │
└───────────────────┴────────────┴────────────┴────────────┘

✓✓✓ = Best fit
✓✓  = Good fit
✓   = Workable
(blank) = Not recommended
```

## Further Reading

- Kent Beck - "Test-Driven Development: By Example"
- Dan North - "Introducing BDD" (original BDD article)
- Gojko Adzic - "Specification by Example" (ATDD)
- Martin Fowler - "Mocks Aren't Stubs"
- Robert C. Martin - "The Three Rules of TDD"
