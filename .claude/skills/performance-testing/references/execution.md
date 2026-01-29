# Performance Test Execution

This reference covers the practical aspects of executing performance tests, from preparation through results analysis.

## Pre-Test Checklist

```
┌─────────────────────────────────────────────────────────────────────┐
│                    PRE-TEST CHECKLIST                                │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  Environment                                                        │
│  □ Test environment matches production configuration                │
│  □ Database populated with realistic data volume                    │
│  □ Caches warmed up (or intentionally cold)                         │
│  □ No other tests running on shared resources                       │
│  □ Network isolation verified                                       │
│                                                                     │
│  Monitoring                                                         │
│  □ APM/monitoring enabled and collecting                            │
│  □ Dashboards accessible                                            │
│  □ Log aggregation working                                          │
│  □ Alerting disabled (to avoid alert fatigue)                       │
│                                                                     │
│  Test Scripts                                                       │
│  □ Scripts validated with small load                                │
│  □ Think times configured realistically                             │
│  □ Assertions checking correct behavior                             │
│  □ Data parameterization working                                    │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

## Environment Preparation

### Step 1: Verify Environment Parity

**Configuration Check**:
```bash
# Compare configurations
diff production.config test.config

# Verify versions
app_version=$(curl http://test-api/version)
prod_version=$(curl http://prod-api/version)
```

**Resource Verification**:
```bash
# Check instance specs
cat /proc/cpuinfo | grep "model name" | head -1
free -h
df -h
```

### Step 2: Data Preparation

**Database Seeding**:
```python
# Seed with realistic data
def seed_database():
    # Match production data volume
    target_users = 1_000_000
    target_orders = 10_000_000

    print(f"Seeding {target_users} users...")
    seed_users(target_users)

    print(f"Seeding {target_orders} orders...")
    seed_orders(target_orders)

    print("Creating indexes...")
    create_indexes()

    print("Analyzing tables...")
    analyze_tables()
```

**Data Distribution**:
Ensure data matches production patterns:
- User activity distribution (active vs inactive)
- Order status distribution
- Product popularity distribution
- Geographic distribution

### Step 3: Cache Warming

**Why Cache Warming Matters**:
Cold caches give unrealistic results. Real production systems have warm caches.

**Cache Warming Script**:
```python
def warm_caches():
    # Warm application cache
    for product_id in popular_products:
        requests.get(f"/api/products/{product_id}")

    # Warm database query cache
    db.execute("SELECT * FROM products WHERE featured = true")

    # Warm Redis cache
    for key in important_keys:
        redis.get(key)
```

### Step 4: Monitoring Setup

**Essential Dashboards**:

1. **Request Metrics**:
   - Request rate (RPS)
   - Response time percentiles (P50, P95, P99)
   - Error rate

2. **Resource Metrics**:
   - CPU utilization
   - Memory usage
   - Disk I/O
   - Network I/O

3. **Application Metrics**:
   - Database connection pool
   - Thread pool usage
   - Cache hit rate
   - Queue depths

4. **External Dependencies**:
   - Database query latency
   - External API latency
   - Message queue depth

## Test Script Validation

### Smoke Test Before Full Load

**Run with minimal load first**:
```bash
# k6: Run with 1 virtual user for 1 minute
k6 run --vus 1 --duration 1m script.js

# Locust: Run with 10 users
locust -f locustfile.py --users 10 --run-time 1m --headless
```

**Verify**:
- [ ] No errors in test execution
- [ ] Correct endpoints called
- [ ] Authentication working
- [ ] Response validation passing
- [ ] Think times applied

### Gradual Load Increase

**Don't jump to peak load immediately**:

```javascript
// Bad: Immediate peak load
export const options = {
  vus: 1000,  // Jump straight to 1000
  duration: '10m',
};

// Good: Gradual ramp
export const options = {
  stages: [
    { duration: '5m', target: 100 },   // Warm up
    { duration: '10m', target: 500 },  // Intermediate
    { duration: '15m', target: 1000 }, // Peak
    { duration: '5m', target: 0 },     // Cool down
  ],
};
```

## During Test Monitoring

```
┌─────────────────────────────────────────────────────────────────────┐
│                    DURING TEST MONITORING                            │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  REAL-TIME DASHBOARD                                                │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │                                                               │  │
│  │  Requests/sec    Response Time (P99)    Error Rate            │  │
│  │  ┌──────────┐    ┌──────────────────┐   ┌──────────┐          │  │
│  │  │  5,234   │    │     156ms        │   │   0.02%  │          │  │
│  │  │ ▲        │    │ ▲                │   │ ▼        │          │  │
│  │  └──────────┘    └──────────────────┘   └──────────┘          │  │
│  │                                                               │  │
│  │  ────────────────────────────────────────────────────────     │  │
│  │                                                               │  │
│  │  CPU Utilization      Memory Usage       Active Threads       │  │
│  │  ┌──────────┐         ┌──────────┐       ┌──────────┐         │  │
│  │  │   45%    │         │   62%    │       │   150    │         │  │
│  │  │ █████░░░ │         │ ██████░░ │       │ ████████ │         │  │
│  │  └──────────┘         └──────────┘       └──────────┘         │  │
│  │                                                               │  │
│  │  ────────────────────────────────────────────────────────     │  │
│  │                                                               │  │
│  │  DB Connections       Cache Hit Rate     Queue Depth          │  │
│  │  ┌──────────┐         ┌──────────┐       ┌──────────┐         │  │
│  │  │  45/100  │         │   98.5%  │       │    12    │         │  │
│  │  │ ████░░░░ │         │ █████████│       │ █░░░░░░░ │         │  │
│  │  └──────────┘         └──────────┘       └──────────┘         │  │
│  │                                                               │  │
│  └───────────────────────────────────────────────────────────────┘  │
│                                                                     │
│  WARNING SIGNS TO WATCH FOR:                                        │
│  ⚠️  Response time trending up                                      │
│  ⚠️  Error rate increasing                                          │
│  ⚠️  Resource utilization > 80%                                     │
│  ⚠️  Queue depths growing                                           │
│  ⚠️  Connection pool exhaustion                                     │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

### What to Watch During Tests

**Healthy Patterns**:
- ✅ Response time stable or slightly increasing
- ✅ Error rate consistently low (< 0.1%)
- ✅ CPU/Memory utilization stable
- ✅ Throughput scales with load

**Warning Signs**:
- ⚠️ Response time suddenly jumps
- ⚠️ Error rate spikes
- ⚠️ Resource utilization > 80%
- ⚠️ Throughput plateaus despite increasing load

**Critical Issues**:
- 🚨 System becomes unresponsive
- 🚨 Error rate > 5%
- 🚨 Memory leak (continuous growth)
- 🚨 Cascading failures

### When to Stop a Test Early

**Stop immediately if**:
1. Error rate exceeds 10% (system failing)
2. Response times > 10x normal (severe degradation)
3. System becomes unresponsive
4. Risk of data corruption
5. Test environment affecting production

**Example Stop Criteria**:
```javascript
// k6: Abort on threshold breach
export const options = {
  thresholds: {
    http_req_failed: ['rate<0.1'],  // < 10% errors
    http_req_duration: ['p(99)<5000'],  // P99 < 5s
  },
  // Abort test if thresholds fail
  abortOnFail: true,
};
```

## Test Execution Workflow

### Execution Steps

1. **Pre-Test Verification** (10 minutes)
   - [ ] Run checklist
   - [ ] Start monitoring
   - [ ] Verify baseline metrics

2. **Smoke Test** (5 minutes)
   - [ ] Run with minimal load
   - [ ] Verify no errors
   - [ ] Check all endpoints working

3. **Main Test Execution** (varies)
   - [ ] Start load generation
   - [ ] Monitor dashboards continuously
   - [ ] Take notes on observations
   - [ ] Capture screenshots of anomalies

4. **Cool Down** (5 minutes)
   - [ ] Gradually reduce load
   - [ ] Verify system recovery
   - [ ] Check for memory leaks

5. **Post-Test Data Collection** (10 minutes)
   - [ ] Export metrics
   - [ ] Collect logs
   - [ ] Save test results
   - [ ] Document observations

## Results Analysis

### Results Report Template

```markdown
# Performance Test Results

## Executive Summary
- **Test Type**: Load Test / Stress Test / etc.
- **Result**: PASS / FAIL
- **Key Finding**: [One sentence summary]

## Test Configuration
- Duration: X minutes
- Peak Load: X users / Y RPS
- Think Time: X-Y seconds

## Results vs Targets

| Metric | Target | Actual | Status |
|--------|--------|--------|--------|
| P95 Response Time | <200ms | 156ms | ✅ PASS |
| P99 Response Time | <500ms | 312ms | ✅ PASS |
| Error Rate | <0.1% | 0.02% | ✅ PASS |
| Throughput | >5000 RPS | 5234 RPS | ✅ PASS |

## Response Time Distribution

| Percentile | Value |
|------------|-------|
| P50 | 45ms |
| P75 | 89ms |
| P90 | 134ms |
| P95 | 156ms |
| P99 | 312ms |
| P99.9 | 890ms |

## Resource Utilization

| Resource | Peak | Average |
|----------|------|---------|
| CPU | 72% | 45% |
| Memory | 68% | 62% |
| DB Connections | 78/100 | 45/100 |

## Bottlenecks Identified
1. [Description of bottleneck]
   - Impact: [quantified impact]
   - Recommendation: [fix suggestion]

## Recommendations
1. [Recommendation 1]
2. [Recommendation 2]

## Appendix
- [Link to detailed metrics]
- [Link to test scripts]
- [Link to monitoring dashboards]
```

### Analyzing Results

**Step 1: Compare Against Targets**

```python
def analyze_results(results, targets):
    """Compare results against targets"""
    analysis = {
        'passed': [],
        'failed': [],
        'warnings': []
    }

    # Check P95 latency
    if results['p95'] < targets['p95']:
        analysis['passed'].append('P95 latency met')
    else:
        analysis['failed'].append(f"P95 latency: {results['p95']}ms > {targets['p95']}ms")

    # Check error rate
    if results['error_rate'] < targets['error_rate']:
        analysis['passed'].append('Error rate acceptable')
    else:
        analysis['failed'].append(f"Error rate: {results['error_rate']}% > {targets['error_rate']}%")

    return analysis
```

**Step 2: Identify Trends**

Look for:
- Response time degradation over test duration
- Throughput plateau
- Error rate spikes
- Resource utilization trends

**Step 3: Correlate Metrics**

```
When response time increased:
- Did error rate increase? → Capacity issue
- Did CPU spike? → CPU bottleneck
- Did DB connections saturate? → Database bottleneck
- Did memory grow? → Memory leak
```

### Visualizing Results

**Response Time Over Time**:
```
Response Time (ms)
▲
│                     ╱
│                   ╱
│                 ╱
│               ╱
│  ───────────╱
│
└──────────────────────────► Time
  Stable    Ramp  Degradation
```

**Throughput vs Response Time**:
```
Response Time (ms)
▲
│                    ╱
│                  ╱
│                ╱
│  ────────────╱
│            Knee (capacity limit)
└──────────────────────────► Throughput (RPS)
```

### Common Result Patterns

**Pattern 1: Linear Degradation**
```
Response time increases linearly with load
→ Likely CPU-bound, needs horizontal scaling
```

**Pattern 2: Sudden Cliff**
```
Good performance until sudden spike
→ Resource exhaustion (connections, memory, threads)
```

**Pattern 3: Gradual Decline**
```
Slow degradation over time
→ Memory leak or resource leak
```

**Pattern 4: Oscillation**
```
Response time oscillates
→ GC pauses, cache eviction, or batch processing interference
```

## Post-Test Activities

### Data Archival

**Save for Future Reference**:
```bash
# Create results archive
mkdir -p results/2024-01-15-load-test
cp test-output.json results/2024-01-15-load-test/
cp screenshots/*.png results/2024-01-15-load-test/
cp logs/*.log results/2024-01-15-load-test/

# Compress for long-term storage
tar -czf results-2024-01-15.tar.gz results/2024-01-15-load-test/
```

**What to Save**:
- Test scripts
- Configuration files
- Raw metrics data
- Log samples
- Screenshots of dashboards
- Analysis report

### Baseline Establishment

**Create Performance Baseline**:
```json
{
  "baseline_date": "2024-01-15",
  "version": "v2.1.0",
  "environment": "staging",
  "metrics": {
    "p50_latency": 45,
    "p95_latency": 156,
    "p99_latency": 312,
    "max_throughput": 5234,
    "error_rate": 0.02
  }
}
```

**Use for Regression Detection**:
```python
def check_regression(current, baseline, threshold=0.1):
    """Check if performance regressed vs baseline"""
    p95_delta = (current['p95'] - baseline['p95']) / baseline['p95']

    if p95_delta > threshold:
        print(f"⚠️ REGRESSION: P95 latency increased by {p95_delta*100:.1f}%")
        return True
    return False
```

### Stakeholder Communication

**Results Summary for Different Audiences**:

**For Engineers**:
- Detailed metrics and percentiles
- Bottlenecks identified
- Technical recommendations
- Resource utilization details

**For Management**:
- Pass/Fail status
- Capacity headroom
- Risk assessment
- Timeline for improvements

**Example Executive Summary**:
```
✅ PASS: System meets performance requirements

Key Findings:
- Handles 5,000 RPS with P99 < 500ms (target met)
- CPU utilization at 72% (adequate headroom)
- No critical bottlenecks identified

Recommendations:
- Add 20% capacity for Black Friday (est. 6,000 RPS)
- Optimize slow queries (15ms improvement potential)
```

## Troubleshooting Common Issues

### Issue: Test Results Inconsistent

**Causes**:
- Background processes interfering
- Network variability
- Insufficient warm-up time
- Shared test environment

**Solutions**:
- Isolate test environment
- Run multiple iterations
- Increase warm-up period
- Use dedicated resources

### Issue: Load Generator Bottleneck

**Symptoms**:
- CPU maxed on load generator
- Unable to achieve target load
- Load generator errors

**Solutions**:
```bash
# Distribute load across multiple machines
k6 run --out cloud script.js  # Use k6 Cloud

# Or run distributed Locust
locust -f locustfile.py --master &
locust -f locustfile.py --worker --master-host=localhost &
locust -f locustfile.py --worker --master-host=localhost &
```

### Issue: Test Environment Too Different

**Problem**: Results don't translate to production

**Solutions**:
- Document environment differences
- Apply scaling factors to results
- Test in production (dark traffic, shadowing)
- Use production-identical staging
