# Resilience Patterns to Validate

## Overview

Chaos engineering experiments should validate that your resilience patterns work as designed. These patterns protect your system from cascading failures and enable graceful degradation.

## Core Resilience Patterns

```
┌─────────────────────────────────────────────────────────────────────┐
│                    RESILIENCE PATTERNS TO VALIDATE                  │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  1. CIRCUIT BREAKER                                                 │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │ Test: Inject repeated failures to downstream service          │  │
│  │ Verify:                                                       │  │
│  │   • Circuit opens after threshold failures                    │  │
│  │   • Requests fail fast when circuit open                      │  │
│  │   • Circuit closes after successful health check              │  │
│  └───────────────────────────────────────────────────────────────┘  │
│                                                                     │
│  2. RETRY WITH BACKOFF                                              │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │ Test: Inject intermittent failures (50% error rate)           │  │
│  │ Verify:                                                       │  │
│  │   • Retries occur with increasing delays                      │  │
│  │   • Successful after retry                                    │  │
│  │   • No retry storm under load                                 │  │
│  └───────────────────────────────────────────────────────────────┘  │
│                                                                     │
│  3. TIMEOUT HANDLING                                                │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │ Test: Add latency exceeding timeout threshold                 │  │
│  │ Verify:                                                       │  │
│  │   • Request times out, doesn't hang indefinitely              │  │
│  │   • Resources released after timeout                          │  │
│  │   • Appropriate error returned to caller                      │  │
│  └───────────────────────────────────────────────────────────────┘  │
│                                                                     │
│  4. BULKHEAD                                                        │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │ Test: Exhaust one resource pool, verify others unaffected     │  │
│  │ Verify:                                                       │  │
│  │   • Failure in one bulkhead doesn't cascade                   │  │
│  │   • Other request types continue functioning                  │  │
│  │   • Thread pools isolated correctly                           │  │
│  └───────────────────────────────────────────────────────────────┘  │
│                                                                     │
│  5. FALLBACK                                                        │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │ Test: Make primary service unavailable                        │  │
│  │ Verify:                                                       │  │
│  │   • Fallback behavior activates                               │  │
│  │   • Gracefully degraded response returned                     │  │
│  │   • User experience acceptable                                │  │
│  └───────────────────────────────────────────────────────────────┘  │
│                                                                     │
│  6. GRACEFUL DEGRADATION                                            │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │ Test: Disable non-critical features                           │  │
│  │ Verify:                                                       │  │
│  │   • Core functionality continues                              │  │
│  │   • Non-critical features fail silently                       │  │
│  │   • No cascading failures                                     │  │
│  └───────────────────────────────────────────────────────────────┘  │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

## Pattern Details

### 1. Circuit Breaker

**Purpose**: Prevent cascading failures by failing fast when a downstream service is unhealthy.

**How it works**:
- **Closed state**: Normal operation, requests flow through
- **Open state**: After threshold failures, circuit opens and requests fail immediately
- **Half-open state**: After timeout, allow test requests to check if service recovered

**Chaos tests to run**:
```python
def test_circuit_breaker_opens_on_failures():
    """Circuit should open after N consecutive failures"""
    # Generate failures until circuit opens
    for i in range(threshold + 1):
        response = call_service()
        if i < threshold:
            assert response.circuit_state == "CLOSED"
        else:
            assert response.circuit_state == "OPEN"

def test_circuit_breaker_fails_fast():
    """When circuit is open, requests should fail immediately"""
    # Open the circuit
    force_circuit_open()

    # Verify fast failure
    start = time.now()
    response = call_service()
    duration = time.now() - start

    assert response.error == "CircuitOpen"
    assert duration < 100  # Should be near-instant
```

### 2. Retry with Exponential Backoff

**Purpose**: Handle transient failures without overwhelming the failing service.

**How it works**:
- Retry failed requests with increasing delays
- Common pattern: wait 100ms, 200ms, 400ms, 800ms
- Add jitter to prevent thundering herd

**Chaos tests to run**:
```python
def test_retry_with_backoff():
    """Verify retries happen with exponential backoff"""
    with intermittent_failures(error_rate=0.5):
        start = time.now()
        response = call_with_retry(max_attempts=4)
        duration = time.now() - start

        # Should have retried with backoff
        assert response.success
        assert response.attempts > 1
        assert duration > 100  # Evidence of backoff

def test_no_retry_storm():
    """Under load, retries shouldn't overwhelm the system"""
    with service_failures():
        # Generate concurrent load
        results = concurrent_requests(count=100)

        # Verify retries are distributed over time
        assert max_concurrent_retries < 20  # Backoff prevents storm
```

### 3. Timeout Handling

**Purpose**: Prevent requests from hanging indefinitely.

**How it works**:
- Set reasonable timeout for each operation
- Release resources when timeout occurs
- Return error to caller

**Chaos tests to run**:
```python
def test_timeout_prevents_hanging():
    """Slow responses should timeout"""
    with latency_injection(delay_ms=10000):
        start = time.now()
        response = call_service(timeout=1000)
        duration = time.now() - start

        assert response.error == "Timeout"
        assert 1000 <= duration <= 1100  # Timed out as expected

def test_resources_released_on_timeout():
    """Connections should be cleaned up after timeout"""
    initial_connections = count_active_connections()

    with latency_injection(delay_ms=10000):
        response = call_service(timeout=1000)

    # Wait for cleanup
    time.sleep(100)

    final_connections = count_active_connections()
    assert final_connections == initial_connections
```

### 4. Bulkhead Pattern

**Purpose**: Isolate resources to prevent one failure from consuming all resources.

**How it works**:
- Separate thread pools for different operations
- Limit resources per operation type
- Failure in one pool doesn't affect others

**Chaos tests to run**:
```python
def test_bulkhead_isolation():
    """Exhausting one bulkhead shouldn't affect others"""
    # Exhaust the "reports" bulkhead
    with slow_reports_service():
        # Flood reports endpoint
        reports_threads = generate_load(endpoint="/reports", count=100)

        # Verify orders endpoint still works
        order_response = call_service("/orders")
        assert order_response.success
        assert order_response.latency < 500
```

### 5. Fallback Pattern

**Purpose**: Provide degraded but acceptable service when primary fails.

**How it works**:
- Define fallback behavior for each operation
- Use cached data, default values, or simplified responses
- Maintain core functionality

**Chaos tests to run**:
```python
def test_fallback_on_service_failure():
    """Should use fallback when primary service fails"""
    with service_unavailable(service="recommendations"):
        response = get_product_recommendations(user_id=123)

        assert response.source == "fallback"
        assert response.recommendations  # Some recommendations returned
        assert response.personalized == False  # But not personalized

def test_fallback_performance():
    """Fallback should be faster than waiting for timeout"""
    with service_unavailable(service="recommendations"):
        start = time.now()
        response = get_product_recommendations(user_id=123)
        duration = time.now() - start

        assert duration < 500  # Fast fallback, not timeout
```

### 6. Graceful Degradation

**Purpose**: Continue providing core functionality when parts of the system fail.

**How it works**:
- Identify critical vs. non-critical features
- Disable non-critical features during partial outages
- Maintain core user journeys

**Chaos tests to run**:
```python
def test_graceful_degradation():
    """Core features work even when ancillary services fail"""
    with service_failures(services=["analytics", "recommendations", "reviews"]):
        # Core checkout should still work
        result = checkout_flow(
            cart_items=[{"id": 1, "qty": 2}],
            payment_method="card"
        )

        assert result.success
        assert result.order_id  # Order created
        # But ancillary features are missing:
        assert result.recommendations == None
        assert result.analytics_tracked == False
```

## Pattern Combinations

Real systems use multiple patterns together. Test these combinations:

```python
def test_circuit_breaker_with_fallback():
    """Circuit breaker should trigger fallback"""
    with repeated_failures(service="external-api"):
        # Circuit should open
        response = call_service()

        # Should use fallback
        assert response.circuit_state == "OPEN"
        assert response.source == "fallback"

def test_retry_with_circuit_breaker():
    """Retry should respect circuit breaker"""
    force_circuit_open()

    # Should NOT retry when circuit is open
    response = call_with_retry(max_attempts=3)

    assert response.attempts == 1  # No retries when circuit open
    assert response.error == "CircuitOpen"
```

## Anti-Patterns to Detect

Chaos experiments can also reveal resilience anti-patterns:

1. **Retry storms**: Too many concurrent retries overwhelming the system
2. **Cascading timeouts**: Timeout chains causing slow failures
3. **Resource leaks**: Connections/threads not released on failure
4. **Silent failures**: Failures not logged or monitored
5. **Tight coupling**: One service failure cascading to many others
