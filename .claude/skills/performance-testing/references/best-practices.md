# Performance Testing Best Practices

This reference covers proven best practices and guidelines for effective performance testing.

## Best Practices Overview

```
┌─────────────────────────────────────────────────────────────────────┐
│                    PERFORMANCE TESTING BEST PRACTICES                │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  1. TEST IN PRODUCTION-LIKE ENVIRONMENT                             │
│     • Same hardware/cloud resources                                 │
│     • Realistic data volumes                                        │
│     • Production configuration                                      │
│                                                                     │
│  2. USE REALISTIC WORKLOAD                                          │
│     • Base on production traffic patterns                           │
│     • Include think time                                            │
│     • Mix of transaction types                                      │
│                                                                     │
│  3. WARM UP BEFORE MEASURING                                        │
│     • Let caches populate                                           │
│     • JIT compilation complete                                      │
│     • Connection pools establish                                    │
│                                                                     │
│  4. MONITOR EVERYTHING                                              │
│     • Client-side metrics (load generator)                          │
│     • Server-side metrics (APM)                                     │
│     • Infrastructure metrics (CPU, memory, network)                 │
│     • Dependency metrics (database, cache, queue)                   │
│                                                                     │
│  5. ESTABLISH BASELINES                                             │
│     • Run baseline tests regularly                                  │
│     • Detect regressions early                                      │
│     • Track trends over time                                        │
│                                                                     │
│  6. TEST INCREMENTALLY                                              │
│     • Start with low load                                           │
│     • Increase gradually                                            │
│     • Identify breaking point                                       │
│                                                                     │
│  7. AUTOMATE                                                        │
│     • Include in CI/CD pipeline                                     │
│     • Scheduled regression tests                                    │
│     • Automated result analysis                                     │
│                                                                     │
│  8. DOCUMENT AND SHARE                                              │
│     • Report findings clearly                                       │
│     • Track improvements                                            │
│     • Share learnings with team                                     │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

## Environment Best Practices

### Production-Like Environment

**Why It Matters**:
Results from dev laptop ≠ production performance

**Requirements**:
- Same OS and versions
- Same hardware specs (or proportional)
- Same network topology
- Same database version and configuration
- Same external dependencies

**Acceptable Compromises**:
```
Production: 10 app servers, 1000 GB database
Test Environment: 7 app servers (70%), 700 GB database

Apply scaling factor: Test results × (10/7) ≈ Production estimate
```

### Realistic Data Volume

**Why It Matters**:
Query performance on 1000 rows ≠ performance on 1M rows

**Data Volume Guidelines**:

| Data Type | Minimum Test Volume | Ideal Test Volume |
|-----------|-------------------|-------------------|
| User records | 50% of production | 100% of production |
| Transaction data | 50% of production | 100% of production |
| Time-series data | Last 90 days | Last 365 days |
| Binary/blob data | Representative sample | Full dataset |

**Data Characteristics to Match**:
- Distribution (popular vs unpopular items)
- Relationships (foreign keys)
- Indexes
- Data locality

### Network Isolation

**Why It Matters**:
Test traffic leaking to production can cause incidents

**Isolation Strategies**:

1. **Separate VPC/Network**
```
Production VPC: 10.0.0.0/16
Test VPC: 10.1.0.0/16
```

2. **DNS-based routing**
```
prod.api.example.com → Production
test.api.example.com → Test Environment
```

3. **Load Balancer Rules**
```
Header: X-Test-Traffic: true → Route to test backends
```

## Workload Best Practices

### Model Real User Behavior

**Bad: Constant hammering without think time**
```python
# This doesn't represent real users
while True:
    api.call()  # Immediately call again
```

**Good: Realistic user behavior**
```python
# Login
response = api.login(username, password)
time.sleep(random.uniform(2, 5))  # Read response

# Browse products
products = api.get_products()
time.sleep(random.uniform(5, 10))  # Browse listings

# View product detail
product = api.get_product(product_id)
time.sleep(random.uniform(10, 30))  # Read details

# 70% exit, 30% add to cart
if random.random() < 0.30:
    api.add_to_cart(product_id)
```

### Use Production Traffic Patterns

**Analyze Production**:
```sql
-- Get endpoint frequency distribution
SELECT
    endpoint,
    COUNT(*) as requests,
    COUNT(*) * 100.0 / SUM(COUNT(*)) OVER () as percentage
FROM access_logs
WHERE timestamp > NOW() - INTERVAL '7 days'
GROUP BY endpoint
ORDER BY requests DESC;
```

**Model in Load Test**:
```python
class UserBehavior(HttpUser):
    @task(40)  # 40% of requests
    def browse_products(self):
        self.client.get("/api/products")

    @task(20)  # 20% of requests
    def view_product(self):
        self.client.get(f"/api/products/{random.randint(1, 1000)}")

    @task(15)  # 15% of requests
    def search(self):
        self.client.get(f"/api/search?q={random_search_term()}")

    @task(10)  # 10% of requests
    def add_to_cart(self):
        self.client.post("/api/cart", json={"product_id": random.randint(1, 1000)})
```

### Include Variance

**Bad: All users do exactly the same thing**
```python
# Everyone follows identical path
def user_flow():
    login()
    view_products()
    view_products()
    view_products()
    logout()
```

**Good: Variable behavior**
```python
def user_flow():
    login()

    # Variable number of product views
    for _ in range(random.randint(1, 10)):
        view_products()

    # Some search, some don't
    if random.random() < 0.6:
        search()

    # Variable conversion rate
    if random.random() < 0.1:
        checkout()

    logout()
```

## Test Execution Best Practices

### Always Warm Up

**Why**: Cold start performance ≠ steady-state performance

**Cold System Issues**:
- Empty caches
- JIT not compiled
- Connection pools empty
- Auto-scaling not triggered

**Warm-Up Pattern**:
```javascript
export const options = {
  stages: [
    { duration: '5m', target: 50 },    // Warm up (not measured)
    { duration: '15m', target: 100 },  // Measure at normal load
    { duration: '10m', target: 200 },  // Measure at peak load
    { duration: '2m', target: 0 },     // Ramp down
  ],
};
```

### Ramp Gradually

**Why**: Sudden load spikes can cause cascading failures that hide real bottlenecks

**Bad: Instant load**
```javascript
// 0 to 1000 users instantly
{ duration: '10m', target: 1000 }
```

**Good: Gradual ramp**
```javascript
// Gradual increase allows system to adapt
{ duration: '2m', target: 100 },
{ duration: '3m', target: 300 },
{ duration: '5m', target: 500 },
{ duration: '10m', target: 1000 },
```

### Run Long Enough

**Minimum Test Durations**:

| Test Type | Minimum Duration | Recommended Duration |
|-----------|-----------------|---------------------|
| Smoke Test | 1 minute | 2 minutes |
| Load Test | 10 minutes | 30-60 minutes |
| Stress Test | Until failure | N/A |
| Soak Test | 4 hours | 8-24 hours |
| Spike Test | 5 minutes | 15 minutes |

**Why Long Tests Matter**:
- Garbage collection patterns emerge
- Memory leaks become visible
- Cache effects stabilize
- Connection pool behavior visible

### Monitor Continuously

**Client-Side Metrics** (from load generator):
- Response time
- Error rate
- Throughput

**Server-Side Metrics** (from APM):
- CPU utilization
- Memory usage
- Database queries
- External API calls

**Why Both?**: Client sees end-to-end, server sees internals

**Example**:
```
Client: P99 = 1000ms (slow!)
Server: API response = 50ms (fast?)

→ Network latency or load generator bottleneck
```

## Baseline and Regression Testing

### Establish Performance Baselines

**Create Baseline After Stable Release**:
```json
{
  "version": "v2.1.0",
  "date": "2024-01-15",
  "environment": "staging",
  "load": {
    "concurrent_users": 1000,
    "duration": "30m"
  },
  "metrics": {
    "p50_ms": 45,
    "p95_ms": 156,
    "p99_ms": 312,
    "error_rate": 0.02,
    "throughput_rps": 5234,
    "cpu_avg": 45,
    "memory_avg_mb": 2048
  }
}
```

### Regression Detection

**Automated Comparison**:
```python
def detect_regression(current, baseline, threshold=0.10):
    """Detect performance regression"""
    regressions = []

    # Check P95 latency
    p95_change = (current['p95'] - baseline['p95']) / baseline['p95']
    if p95_change > threshold:
        regressions.append({
            'metric': 'P95 Latency',
            'change': f"{p95_change*100:.1f}%",
            'baseline': f"{baseline['p95']}ms",
            'current': f"{current['p95']}ms",
        })

    # Check throughput (should not decrease)
    throughput_change = (current['throughput'] - baseline['throughput']) / baseline['throughput']
    if throughput_change < -threshold:
        regressions.append({
            'metric': 'Throughput',
            'change': f"{throughput_change*100:.1f}%",
            'baseline': f"{baseline['throughput']} RPS",
            'current': f"{current['throughput']} RPS",
        })

    return regressions
```

### Track Trends Over Time

**Performance Trend Tracking**:
```
P95 Latency Trend
▲
│  ●
│    ●
│      ●  ●
│         ●    ●
│              ●
└─────────────────────────► Release Version
  2.0  2.1  2.2  2.3  2.4  2.5
```

**Trend Analysis**:
- Is performance improving or degrading?
- Which releases introduced regressions?
- What is the rate of change?

## Automation Best Practices

### CI/CD Integration

**Performance Gates in Pipeline**:
```yaml
# .gitlab-ci.yml
performance_test:
  stage: test
  script:
    - k6 run --out json=results.json load-test.js
    - python analyze_results.py results.json
  artifacts:
    reports:
      performance: results.json
  rules:
    - if: $CI_COMMIT_BRANCH == "main"
```

**Analysis Script**:
```python
# analyze_results.py
import json
import sys

def analyze(results_file):
    with open(results_file) as f:
        data = json.load(f)

    # Check thresholds
    p95 = data['metrics']['http_req_duration']['p(95)']
    error_rate = data['metrics']['http_req_failed']['rate']

    failed = False

    if p95 > 500:  # P95 should be < 500ms
        print(f"❌ FAIL: P95 latency {p95}ms > 500ms")
        failed = True

    if error_rate > 0.01:  # Error rate should be < 1%
        print(f"❌ FAIL: Error rate {error_rate*100}% > 1%")
        failed = True

    if failed:
        sys.exit(1)
    else:
        print("✅ PASS: Performance requirements met")
        sys.exit(0)

if __name__ == '__main__':
    analyze(sys.argv[1])
```

### Scheduled Tests

**Run Regular Baseline Tests**:
```yaml
# cron schedule: Daily at 2 AM
performance_baseline:
  schedule: "0 2 * * *"
  script:
    - k6 run baseline-test.js
    - python compare_to_baseline.py
  only:
    - schedules
```

**Why Scheduled Tests**:
- Catch performance regressions early
- Track long-term trends
- Validate infrastructure changes
- Monitor third-party dependencies

## Documentation Best Practices

### Document Test Design

**Test Plan Documentation**:
```markdown
# Load Test Plan: Checkout API

## Objectives
- Validate checkout handles 1000 concurrent users
- P99 latency < 1000ms
- Error rate < 0.1%

## Scope
- In scope: /api/checkout endpoint
- Out of scope: Admin endpoints

## Workload Model
- 60% payment by credit card
- 30% payment by PayPal
- 10% payment by gift card

## Load Profile
- Ramp: 5 minutes to 1000 users
- Sustain: 30 minutes at 1000 users
- Ramp down: 2 minutes

## Success Criteria
- P99 < 1000ms: CRITICAL
- Error rate < 0.1%: CRITICAL
- CPU < 80%: IMPORTANT
```

### Document Results

**Results Should Be**:
- **Reproducible**: Enough detail to re-run
- **Actionable**: Clear next steps
- **Comparable**: Use consistent format

**Results Template**:
```markdown
# Performance Test Results - v2.1.0

## Summary
✅ PASS - All targets met

## Metrics
| Metric | Target | Actual | Status |
|--------|--------|--------|--------|
| P95 Latency | < 500ms | 312ms | ✅ |
| Error Rate | < 0.1% | 0.02% | ✅ |
| Throughput | > 5000 RPS | 5234 RPS | ✅ |

## Environment
- Version: v2.1.0
- Date: 2024-01-15
- Environment: Staging
- Load: 1000 concurrent users, 30 minutes

## Observations
- CPU peaked at 72% (adequate headroom)
- No memory leaks detected
- Cache hit rate: 98.5%

## Recommendations
1. Add 20% capacity for Black Friday
2. Optimize `/api/search` query (200ms → 150ms potential)

## Files
- Test script: `tests/load-test.js`
- Raw results: `results/2024-01-15/raw.json`
- Dashboard: [Link to Grafana]
```

### Share Knowledge

**Create Runbooks**:
```markdown
# Performance Testing Runbook

## How to Run Load Test

1. **Prepare environment**
   ```bash
   ./scripts/prepare-test-env.sh
   ```

2. **Run test**
   ```bash
   k6 run tests/load-test.js
   ```

3. **Analyze results**
   ```bash
   python scripts/analyze-results.py results.json
   ```

## Troubleshooting

### High Error Rate
- Check logs: `kubectl logs -f deployment/api`
- Check database connections
- Verify test data valid

### Low Throughput
- Check load generator CPU
- Verify network connectivity
- Check for rate limiting
```

## Common Pitfalls to Avoid

### Pitfall 1: Testing Without Warm-Up

```
❌ Bad: Measure cold start performance
✅ Good: Warm up, then measure steady state
```

### Pitfall 2: Unrealistic Think Time

```
❌ Bad: No delay between requests
✅ Good: 2-10 second delays simulating real users
```

### Pitfall 3: Ignoring Variance

```
❌ Bad: All users follow identical path
✅ Good: Variable behavior with randomization
```

### Pitfall 4: Testing in Dev Environment

```
❌ Bad: Laptop with 100 row database
✅ Good: Production-like environment with realistic data
```

### Pitfall 5: Only Measuring Averages

```
❌ Bad: Average response time = 50ms
✅ Good: P50=45ms, P95=156ms, P99=312ms
```

### Pitfall 6: Ignoring Client-Side Bottlenecks

```
❌ Bad: Load generator CPU at 100% (bottleneck!)
✅ Good: Distributed load generation, monitor load generator
```

### Pitfall 7: No Baseline Comparison

```
❌ Bad: "P95 is 200ms" (is that good?)
✅ Good: "P95 is 200ms (baseline: 150ms, +33% regression)"
```

### Pitfall 8: Testing Only Happy Path

```
❌ Bad: All requests succeed
✅ Good: Include error scenarios (auth failures, invalid input)
```

## Performance Testing Workflow

1. **Define Objectives**: What questions are we answering?
2. **Model Workload**: Create realistic user behavior model
3. **Design Tests**: Choose test types and create scripts
4. **Prepare Environment**: Set up production-like test environment
5. **Execute Tests**: Run tests with proper monitoring
6. **Analyze Results**: Compare against targets, identify issues
7. **Report Findings**: Document results and recommendations
8. **Iterate**: Fix issues and re-test

## Question Strategy

When designing performance tests, ask:

### For Test Planning:
1. "What are the SLAs or performance requirements?"
2. "What is the expected peak load?"
3. "What are the critical user journeys?"
4. "What does production traffic pattern look like?"

### For Bottleneck Analysis:
1. "Which resource is saturated first under load?"
2. "Where is the time being spent in slow requests?"
3. "What correlates with performance degradation?"

### For Environment:
1. "How close is test environment to production?"
2. "What is the test data volume vs production?"
3. "Are all dependencies included?"
