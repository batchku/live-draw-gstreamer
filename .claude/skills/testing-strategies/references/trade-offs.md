# Testing Trade-offs and Decision Frameworks

This document provides detailed guidance on testing trade-offs, decision frameworks, and test type selection strategies, including comprehensive coverage of external API contract testing.

## Speed vs Confidence Matrix

```
                        High Speed
                            │
                            │
             Unit Tests     │     Static Analysis
             ┌─────────┐    │     ┌─────────┐
             │ Fast    │    │     │ Instant │
             │ Narrow  │    │     │ Limited │
             └─────────┘    │     └─────────┘
                            │
   Low ─────────────────────┼───────────────────── High
   Confidence               │                   Confidence
                            │
             Contract       │     E2E Tests
             Tests          │     ┌─────────┐
             ┌─────────┐    │     │ Slow    │
             │ Medium  │    │     │ Broad   │
             └─────────┘    │     └─────────┘
                            │
                            │
                        Low Speed
```

### Understanding the Trade-off

**The Core Tension**: Fast tests provide quick feedback but limited confidence. Comprehensive tests provide high confidence but slow feedback.

**Optimal Strategy**: Balance across the spectrum based on risk and criticality.

| Test Type | Speed | Confidence | When to Use |
|-----------|-------|------------|-------------|
| Static Analysis | Instant | Low | Always (zero cost) |
| Unit Tests | Milliseconds | Medium | Business logic, algorithms |
| Contract Tests | Seconds | Medium-High | External service boundaries |
| Integration Tests | Seconds | High | Component interactions |
| E2E Tests | Minutes | Very High | Critical user journeys |

### Speed Considerations

**Unit Tests**:
- Target: <1ms per test
- 1000 unit tests should run in <1 second
- Optimize by avoiding: I/O, network, external dependencies

**Integration Tests**:
- Target: <100ms per test
- Acceptable: <1s per test with database
- Optimize with: in-memory databases, testcontainers, fixtures

**E2E Tests**:
- Target: <30s per test
- Acceptable: <2 minutes per critical flow
- Optimize with: parallel execution, smart waits, headless browsers

### Confidence Considerations

**Low Confidence** (Unit Tests):
- Tests individual components in isolation
- May miss integration issues
- Can give false confidence with too much mocking

**Medium Confidence** (Integration Tests):
- Tests components working together
- Catches boundary issues
- May miss UI/UX problems

**High Confidence** (E2E Tests):
- Tests entire system as user would experience
- Catches all types of issues
- Can be flaky (timing, state, dependencies)

## Isolation vs Realism Trade-off

```
┌─────────────────────────────────────────────────────────────────────┐
│                    ISOLATION VS REALISM                             │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  More Isolation (Mocks/Stubs)                                       │
│  ┌─────────────────────────────────────────────────────────────┐    │
│  │ ✓ Fast execution                                            │    │
│  │ ✓ Deterministic                                             │    │
│  │ ✓ Easy failure localization                                 │    │
│  │ ✗ May not reflect real behavior                             │    │
│  │ ✗ Mocks can drift from reality                              │    │
│  │ ✗ False confidence                                          │    │
│  └─────────────────────────────────────────────────────────────┘    │
│                                                                     │
│  More Realism (Real Dependencies)                                   │
│  ┌─────────────────────────────────────────────────────────────┐    │
│  │ ✓ Tests real behavior                                       │    │
│  │ ✓ Catches integration issues                                │    │
│  │ ✓ Higher confidence                                         │    │
│  │ ✗ Slower execution                                          │    │
│  │ ✗ Harder to control state                                   │    │
│  │ ✗ May be flaky                                              │    │
│  └─────────────────────────────────────────────────────────────┘    │
│                                                                     │
│  Strategy:                                                          │
│  - Use mocks for external services (APIs, payment providers)        │
│  - Use real dependencies for owned components when feasible         │
│  - Use containers (Docker, Testcontainers) for databases            │
│  - Use contract tests to verify mock accuracy                       │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

### When to Use Mocks

**Use Mocks For**:
1. **External Services You Don't Control**:
   - Payment gateways (Stripe, PayPal)
   - Email providers (SendGrid, Mailgun)
   - SMS services (Twilio)
   - Third-party APIs (weather, maps)

2. **Slow Operations**:
   - File system operations
   - Network calls
   - Heavy computations

3. **Non-Deterministic Behavior**:
   - Random number generation
   - Current time/date
   - External API rate limits

4. **Unavailable Dependencies**:
   - Services not available in test environment
   - Legacy systems
   - Expensive resources (cloud services)

**Example (Python)**:
```python
# Mock external payment service
def test_checkout_success():
    with mock.patch('payment_service.charge') as mock_charge:
        mock_charge.return_value = {"status": "success", "transaction_id": "tx_123"}

        order = checkout(cart, credit_card)

        assert order.status == "completed"
        mock_charge.assert_called_once()
```

### When to Use Real Dependencies

**Use Real Dependencies For**:
1. **Components You Own and Control**:
   - Internal services
   - Your own modules
   - Business logic

2. **Databases** (with containers):
   - Use real database when possible
   - Testcontainers make this easy
   - More confidence than in-memory

3. **File System** (with temp directories):
   - Test real file operations
   - Use pytest tmpdir or similar

4. **Simple, Fast Operations**:
   - Utility functions
   - Data transformations
   - Calculations

**Example (Python with Testcontainers)**:
```python
from testcontainers.postgres import PostgresContainer

def test_user_repository_with_real_database():
    with PostgresContainer("postgres:14") as postgres:
        # Real database, isolated per test
        db = connect(postgres.get_connection_url())

        user_repo = UserRepository(db)
        user = user_repo.create(name="Alice", email="alice@example.com")

        retrieved = user_repo.find_by_id(user.id)
        assert retrieved.name == "Alice"
```

### The Mock Drift Problem

**Issue**: Mocks can become outdated when the real dependency changes.

```python
# Real API changed "user_data" to "userData"
# But mock still uses old field name

# Mock (outdated)
mock_response = {
    "user_data": {"name": "Alice"}  # Old field name
}

# Real API now returns
real_response = {
    "userData": {"name": "Alice"}  # New field name
}

# Tests pass (using mock) but production breaks (using real API)
```

**Solution**: Use contract testing (see below).

## Maintenance Cost Factors

```
┌─────────────────────────────────────────────────────────────────────┐
│                    TEST MAINTENANCE COST                            │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  High Maintenance Cost:                                             │
│  ┌─────────────────────────────────────────────────────────────┐    │
│  │ • Tests coupled to implementation details                   │    │
│  │ • UI tests using fragile selectors                          │    │
│  │ • Tests with complex setup/teardown                         │    │
│  │ • Shared mutable state between tests                        │    │
│  │ • Tests that verify internal methods directly               │    │
│  └─────────────────────────────────────────────────────────────┘    │
│                                                                     │
│  Low Maintenance Cost:                                              │
│  ┌─────────────────────────────────────────────────────────────┐    │
│  │ • Tests that verify behavior, not implementation            │    │
│  │ • Semantic selectors (data-testid, roles)                   │    │
│  │ • Reusable test fixtures and factories                      │    │
│  │ • Independent, isolated tests                               │    │
│  │ • Tests against public APIs only                            │    │
│  └─────────────────────────────────────────────────────────────┘    │
│                                                                     │
│  Maintenance Pyramid:                                               │
│           ┌─────────┐                                               │
│           │ E2E     │ High maintenance (UI changes)                 │
│       ┌───┴─────────┴───┐                                           │
│       │   Integration   │ Medium maintenance                        │
│   ┌───┴─────────────────┴───┐                                       │
│   │       Unit Tests        │ Low maintenance (when done right)     │
│   └─────────────────────────┘                                       │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

### Reducing Maintenance Cost

#### 1. Test Behavior, Not Implementation

```python
# ❌ HIGH MAINTENANCE: Coupled to implementation
def test_user_service_saves_via_repository():
    service = UserService(mock_repository)
    service.create_user("Alice", "alice@example.com")

    # Test verifies internal implementation detail
    mock_repository.save.assert_called_once()

# ✓ LOW MAINTENANCE: Tests behavior
def test_user_service_creates_user():
    service = UserService(real_repository)
    user = service.create_user("Alice", "alice@example.com")

    # Test verifies observable outcome
    retrieved = service.get_user(user.id)
    assert retrieved.name == "Alice"
```

#### 2. Use Stable Selectors in UI Tests

```javascript
// ❌ HIGH MAINTENANCE: Fragile CSS selectors
await page.click('.header > div:nth-child(2) > button.btn-primary');

// ✓ LOW MAINTENANCE: Semantic test IDs
await page.click('[data-testid="login-button"]');

// ✓✓ EVEN BETTER: Accessibility roles
await page.click('role=button[name="Log in"]');
```

#### 3. Use Test Fixtures and Factories

```python
# ❌ HIGH MAINTENANCE: Setup duplicated across tests
def test_user_can_place_order():
    user = User(name="Alice", email="alice@example.com")
    user.save()
    product = Product(name="Widget", price=10.00)
    product.save()
    # ... 20 more lines of setup

# ✓ LOW MAINTENANCE: Reusable fixtures
@pytest.fixture
def user():
    return UserFactory.create()

@pytest.fixture
def product():
    return ProductFactory.create()

def test_user_can_place_order(user, product):
    order = Order.create(user=user, items=[product])
    assert order.total == product.price
```

#### 4. Keep Tests Independent

```python
# ❌ HIGH MAINTENANCE: Tests depend on order
class TestUserWorkflow:
    user_id = None

    def test_1_create_user(self):
        self.user_id = create_user("Alice").id

    def test_2_update_user(self):
        update_user(self.user_id, email="alice@example.com")

    def test_3_delete_user(self):
        delete_user(self.user_id)

# ✓ LOW MAINTENANCE: Independent tests
class TestUserWorkflow:
    def test_create_user(self):
        user = create_user("Alice")
        assert user.name == "Alice"

    def test_update_user(self):
        user = create_user("Alice")
        updated = update_user(user.id, email="alice@example.com")
        assert updated.email == "alice@example.com"

    def test_delete_user(self):
        user = create_user("Alice")
        delete_user(user.id)
        assert find_user(user.id) is None
```

## External API Contract Testing

### The Mock Drift Problem

```
┌─────────────────────────────────────────────────────────────────────┐
│                    THE MOCK DRIFT PROBLEM                           │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  TIMELINE OF A SILENT FAILURE:                                      │
│                                                                     │
│  1. Developer reads API docs    →  Assumes field is "user_data"     │
│  2. Developer writes code       →  response.get("user_data")        │
│  3. Developer writes mock       →  {"user_data": {...}}             │
│  4. Tests pass (100% coverage)  →  ✓ All green!                     │
│  5. Production deployment       →  💥 API returns "userData"        │
│                                                                     │
│  THE MOCK MATCHED THE IMPLEMENTATION, NOT REALITY                   │
│                                                                     │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │  Coverage metrics: 95% ✓                                      │  │
│  │  All tests passing: YES ✓                                     │  │
│  │  Application status: BROKEN ✗                                 │  │
│  └───────────────────────────────────────────────────────────────┘  │
│                                                                     │
│  This is NOT a coverage problem.                                    │
│  This is a CONTRACT VERIFICATION problem.                           │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

**Why This Happens**:
1. **Documentation Lag**: API docs may be outdated or inaccurate
2. **Assumption-Based Mocks**: Developers create mocks based on assumptions, not reality
3. **No Reality Check**: Tests never call the real API to verify contract
4. **False Confidence**: High coverage metrics mask the contract mismatch

### Contract Testing Strategies for External Services

```
┌─────────────────────────────────────────────────────────────────────┐
│                    CONTRACT TESTING APPROACHES                      │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  1. LIVE SERVICE SMOKE TESTS (Highest Confidence)                   │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │ • At least ONE test calls the REAL external service           │  │
│  │ • Validates response structure matches implementation         │  │
│  │ • Run before releases, can skip in routine CI                 │  │
│  │ • Catches: field renames, structure changes, deprecations     │  │
│  │ • Marker: @pytest.mark.live_api or similar                    │  │
│  └───────────────────────────────────────────────────────────────┘  │
│                                                                     │
│  2. RECORDED RESPONSE FIXTURES (Contract Anchors)                   │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │ • Capture and store REAL API responses as test fixtures       │  │
│  │ • Capture BEFORE writing implementation (establish baseline)  │  │
│  │ • Use for integration tests instead of hand-written mocks     │  │
│  │ • Catches: developer assumptions vs actual API behavior       │  │
│  │ • Re-capture when API provider announces breaking changes     │  │
│  └───────────────────────────────────────────────────────────────┘  │
│                                                                     │
│  3. MOCK VALIDATION TESTS (Drift Detection)                         │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │ • Compare synthetic mocks against recorded real responses     │  │
│  │ • Flag when mock fields don't exist in real API               │  │
│  │ • Detect type mismatches (string vs int, etc.)                │  │
│  │ • Run in CI to prevent mock drift                             │  │
│  │ • Catches: synthetic mocks diverging from reality             │  │
│  └───────────────────────────────────────────────────────────────┘  │
│                                                                     │
│  4. SCHEMA VALIDATION (If API Spec Available)                       │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │ • Validate against OpenAPI/Swagger/JSON Schema/GraphQL SDL    │  │
│  │ • Use published API specifications from provider              │  │
│  │ • Automate schema updates when provider publishes changes     │  │
│  │ • Catches: any contract violation against official spec       │  │
│  └───────────────────────────────────────────────────────────────┘  │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

### 1. Live Service Smoke Tests

**Purpose**: Verify the real API contract at least once.

**Implementation** (Python):
```python
import pytest
import os

# Mark tests that call real external services
@pytest.mark.live_api
def test_weather_api_returns_expected_structure():
    """
    Calls REAL Weather API to verify response structure.
    Run before releases: pytest -m live_api
    Skip in routine CI: pytest -m "not live_api"
    """
    api_key = os.getenv("WEATHER_API_KEY")
    if not api_key:
        pytest.skip("WEATHER_API_KEY not set")

    # Call REAL API
    response = requests.get(
        "https://api.weatherapi.com/v1/current.json",
        params={"key": api_key, "q": "London"}
    )

    # Verify contract
    assert response.status_code == 200
    data = response.json()

    # Verify expected fields exist
    assert "location" in data
    assert "current" in data
    assert data["location"]["name"] == "London"
    assert "temp_c" in data["current"]
    assert isinstance(data["current"]["temp_c"], (int, float))

    # If this test fails, our assumptions about the API are wrong!
```

**When to Run**:
- Before every release/deployment (mandatory)
- During development when changing API integration
- Periodically (weekly) to catch API changes early

**CI Configuration**:
```yaml
# .github/workflows/ci.yml
name: CI

on: [push, pull_request]

jobs:
  unit-tests:
    runs-on: ubuntu-latest
    steps:
      - run: pytest -m "not live_api"  # Skip live tests in routine CI

  pre-release:
    runs-on: ubuntu-latest
    if: github.ref == 'refs/heads/main'
    steps:
      - run: pytest -m live_api  # Run live tests before release
        env:
          WEATHER_API_KEY: ${{ secrets.WEATHER_API_KEY }}
```

### 2. Recorded Response Fixtures

**Purpose**: Capture real API responses to use as test fixtures.

**Workflow**:
1. Call real API
2. Save response to fixture file
3. Use fixture in tests instead of hand-written mocks

**Implementation** (Python):
```python
# scripts/capture_api_fixtures.py
import requests
import json
import os

def capture_weather_api_response():
    """Capture real API response to use as fixture"""
    api_key = os.getenv("WEATHER_API_KEY")
    response = requests.get(
        "https://api.weatherapi.com/v1/current.json",
        params={"key": api_key, "q": "London"}
    )

    # Save real response
    fixture_path = "tests/fixtures/recorded/weather_api_london.json"
    os.makedirs(os.path.dirname(fixture_path), exist_ok=True)

    with open(fixture_path, "w") as f:
        json.dump(response.json(), f, indent=2)

    print(f"Captured response to {fixture_path}")

if __name__ == "__main__":
    capture_weather_api_response()
```

**Using Recorded Fixtures in Tests**:
```python
# tests/test_weather_service.py
import json

def test_weather_service_parses_api_response():
    # Load REAL recorded response
    with open("tests/fixtures/recorded/weather_api_london.json") as f:
        api_response = json.load(f)

    # Test our code against REAL response structure
    service = WeatherService()
    weather = service.parse_response(api_response)

    assert weather.city == "London"
    assert weather.temperature is not None
    assert isinstance(weather.temperature, float)
```

**Benefits**:
- Tests use real response structure, not assumptions
- Catches field name mismatches immediately
- No need to guess API structure from documentation
- Easy to update when API changes (re-capture)

### 3. Mock Validation Tests

**Purpose**: Verify that hand-written mocks match recorded real responses.

**Implementation** (Python):
```python
# tests/validation/test_mock_accuracy.py
import json
import pytest

def load_recorded_fixture(name):
    """Load recorded real API response"""
    with open(f"tests/fixtures/recorded/{name}.json") as f:
        return json.load(f)

def load_synthetic_mock(name):
    """Load hand-written mock"""
    from tests.fixtures.synthetic import mocks
    return getattr(mocks, name)

def validate_structure(recorded, synthetic, path=""):
    """Recursively validate mock structure matches recorded"""
    errors = []

    # Check synthetic has no extra fields
    for key in synthetic:
        if key not in recorded:
            errors.append(f"{path}.{key} exists in mock but not in real API")

    # Check synthetic has all required fields
    for key in recorded:
        if key not in synthetic:
            errors.append(f"{path}.{key} exists in real API but not in mock")
        elif isinstance(recorded[key], dict) and isinstance(synthetic[key], dict):
            # Recursively validate nested objects
            errors.extend(validate_structure(
                recorded[key],
                synthetic[key],
                f"{path}.{key}"
            ))

    return errors

def test_weather_api_mock_matches_recorded():
    """Verify synthetic mock structure matches real API"""
    recorded = load_recorded_fixture("weather_api_london")
    synthetic = load_synthetic_mock("WEATHER_RESPONSE_LONDON")

    errors = validate_structure(recorded, synthetic)

    if errors:
        pytest.fail("Mock drift detected:\n" + "\n".join(errors))
```

**Example Failure**:
```
FAILED tests/validation/test_mock_accuracy.py::test_weather_api_mock_matches_recorded

Mock drift detected:
.current.temp_c exists in real API but not in mock
.current.temperature_celsius exists in mock but not in real API
```

### 4. Schema Validation

**Purpose**: Validate against official API specifications (if available).

**Implementation** (Python with OpenAPI):
```python
# tests/validation/test_api_schema.py
import json
from openapi_spec_validator import validate_spec
from openapi_core import create_spec
from openapi_core.validation.response import openapi_response_validator

def test_weather_api_response_matches_openapi_spec():
    # Load OpenAPI spec from provider
    with open("tests/schemas/weatherapi_openapi.yaml") as f:
        spec_dict = yaml.safe_load(f)

    spec = create_spec(spec_dict)

    # Load recorded real response
    with open("tests/fixtures/recorded/weather_api_london.json") as f:
        response_data = json.load(f)

    # Validate response against spec
    result = openapi_response_validator.validate(
        spec,
        request=...,
        response=response_data
    )

    # Will raise exception if validation fails
    result.raise_for_errors()
```

### Test Pyramid with External Services

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

### Anti-Pattern: Mock-Only External Service Testing

```
┌─────────────────────────────────────────────────────────────────────┐
│              ANTI-PATTERN: MOCK-ONLY EXTERNAL API TESTING           │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  ❌ DANGEROUS: Testing external API integration with only mocks     │
│                                                                     │
│     tests/                                                          │
│     ├── unit/test_api_client.py     # Uses hand-written mocks       │
│     ├── integration/test_service.py # Uses hand-written mocks       │
│     └── fixtures/mock_responses.py  # Matches implementation        │
│                                                                     │
│     Problem: Mock field names match implementation assumptions,     │
│     NOT the real API. 100% coverage achieved. App broken.           │
│                                                                     │
│  ─────────────────────────────────────────────────────────────────  │
│                                                                     │
│  ✓ CORRECT: Layered approach with reality anchoring                 │
│                                                                     │
│     tests/                                                          │
│     ├── smoke/                                                      │
│     │   └── test_live_services.py   # Calls REAL APIs (≥1 test)     │
│     ├── validation/                                                 │
│     │   └── test_mock_accuracy.py   # Mocks match recorded          │
│     ├── fixtures/                                                   │
│     │   ├── recorded/               # REAL responses captured       │
│     │   │   └── api_response.json   # From actual API call          │
│     │   └── synthetic/              # Hand-written edge cases       │
│     │       └── mock_responses.py   # Validated against recorded    │
│     ├── integration/                                                │
│     │   └── test_service.py         # Uses recorded fixtures        │
│     └── unit/                                                       │
│         └── test_api_client.py      # Uses validated mocks          │
│                                                                     │
│     Result: Mock drift detected before production deployment.       │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

### Coverage Metrics for External Service Integration

```
┌─────────────────────────────────────────────────────────────────────┐
│           COVERAGE METRICS: WHAT THEY DO AND DON'T CATCH            │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  Traditional Metrics (INSUFFICIENT for external services):          │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │ • Line Coverage: 95%     ← Doesn't catch wrong field names    │  │
│  │ • Branch Coverage: 90%   ← Doesn't catch wrong field names    │  │
│  │ • Mutation Score: 80%    ← Doesn't catch wrong field names    │  │
│  │ • Function Coverage: 95% ← Doesn't catch wrong field names    │  │
│  └───────────────────────────────────────────────────────────────┘  │
│                                                                     │
│  Additional Metrics REQUIRED for external services:                 │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │ • Contract Coverage: % of API fields validated against real   │  │
│  │ • Live Test Coverage: ≥1 live test per external service       │  │
│  │ • Mock Validation: All synthetic mocks verified vs recorded   │  │
│  │ • Schema Compliance: % validated against API specification    │  │
│  │ • Field Accuracy: % of mock fields verified in real response  │  │
│  └───────────────────────────────────────────────────────────────┘  │
│                                                                     │
│  Key Insight:                                                       │
│  You can achieve 100% line coverage with every test passing         │
│  while the application fails on its first real API call.            │
│  Contract verification requires testing against REALITY.            │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

### When to Apply External API Contract Testing

Apply this approach when your system integrates with:

- Third-party REST APIs (payment processors, social login, maps, weather, etc.)
- External GraphQL services
- Partner or vendor APIs
- Cloud provider APIs (AWS, GCP, Azure services)
- SaaS platform APIs (Stripe, Twilio, SendGrid, etc.)
- Any service you don't control and can't run locally

**Do NOT skip contract testing because**:
- "The API is well-documented" — Documentation can be outdated or wrong
- "We have high coverage" — Coverage doesn't verify field names match reality
- "The mock looks right" — Mocks based on assumptions inherit those assumptions
- "It worked in development" — Development may use different API versions

## Decision Framework

### Test Type Selection Matrix

```
┌─────────────────┬──────────┬──────────┬──────────┬──────────┬──────────┐
│ Question        │ Unit     │ Int.     │ Contract │ E2E      │ Manual   │
├─────────────────┼──────────┼──────────┼──────────┼──────────┼──────────┤
│ Tests logic?    │ ✓✓✓      │ ✓        │          │          │          │
│ Tests boundary? │          │ ✓✓✓      │ ✓✓       │ ✓        │          │
│ Tests UI?       │          │          │          │ ✓✓✓      │ ✓✓       │
│ Tests external? │          │ ✓✓       │ ✓✓✓      │ ✓        │          │
│ Fast feedback?  │ ✓✓✓      │ ✓✓       │ ✓        │          │          │
│ High confidence?│          │ ✓✓       │ ✓✓       │ ✓✓✓      │ ✓✓✓      │
│ Easy to write?  │ ✓✓✓      │ ✓✓       │ ✓        │          │ ✓✓✓      │
│ Easy to maintain│ ✓✓✓      │ ✓✓       │ ✓✓       │          │          │
│ Catches bugs?   │ ✓        │ ✓✓       │ ✓✓       │ ✓✓✓      │ ✓✓✓      │
└─────────────────┴──────────┴──────────┴──────────┴──────────┴──────────┘

✓✓✓ = Excellent    ✓✓ = Good    ✓ = Acceptable    (blank) = Poor
```

### Testing Decision Tree

```
Does it involve external services you don't control?
├─ YES → Use contract testing (live smoke + recorded fixtures)
│        + integration tests with recorded fixtures
│        + unit tests with validated mocks
└─ NO  → Continue...

Is it business logic or algorithms?
├─ YES → Use unit tests (TDD if complex)
└─ NO  → Continue...

Does it cross component boundaries?
├─ YES → Use integration tests
└─ NO  → Continue...

Is it a critical user workflow?
├─ YES → Use E2E tests (minimal, just critical paths)
└─ NO  → Skip E2E, rely on integration tests

Is it UI appearance/layout?
├─ YES → Use visual regression testing + manual QA
└─ NO  → Continue...

Is it a bug fix?
├─ YES → Write test that reproduces bug (lowest level possible)
│        Fix bug, verify test passes
└─ NO  → Apply risk-based testing approach
```

## Further Reading

- Martin Fowler - "TestPyramid", "UnitTest", "Mocks Aren't Stubs"
- Michael Feathers - "Working Effectively with Legacy Code"
- Kent Beck - "Test-Driven Development: By Example"
- J.B. Rainsberger - "Integrated Tests Are A Scam" (talk)
- Sam Newman - "Testing Strategies in a Microservice Architecture"
