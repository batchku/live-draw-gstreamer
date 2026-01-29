# Performance Test Design

This reference covers how to design effective performance tests, including test planning, workload modeling, and scenario creation.

## Test Design Template

```
┌─────────────────────────────────────────────────────────────────────┐
│                    PERFORMANCE TEST PLAN                             │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  1. OBJECTIVES                                                      │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │ □ Validate system handles expected load (10,000 users)        │  │
│  │ □ Identify bottlenecks before production launch               │  │
│  │ □ Establish performance baseline for regression testing       │  │
│  │ □ Verify SLA compliance (P99 < 500ms, error rate < 0.1%)      │  │
│  └───────────────────────────────────────────────────────────────┘  │
│                                                                     │
│  2. SCOPE                                                           │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │ In Scope:                                                     │  │
│  │ • API endpoints: /orders, /products, /checkout                │  │
│  │ • User workflows: browse → search → add to cart → checkout    │  │
│  │                                                               │  │
│  │ Out of Scope:                                                 │  │
│  │ • Admin endpoints                                             │  │
│  │ • Batch processing jobs                                       │  │
│  └───────────────────────────────────────────────────────────────┘  │
│                                                                     │
│  3. WORKLOAD MODEL                                                  │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │ User Type       │ % of Users │ Actions                        │  │
│  ├─────────────────┼────────────┼────────────────────────────────┤  │
│  │ Browser         │ 60%        │ View products, search          │  │
│  │ Buyer           │ 30%        │ Add to cart, checkout          │  │
│  │ Power user      │ 10%        │ Rapid browsing, multiple carts │  │
│  └─────────────────┴────────────┴────────────────────────────────┘  │
│                                                                     │
│  4. LOAD PROFILE                                                    │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │ Scenario        │ Users │ Duration │ Think Time │ Ramp        │  │
│  ├─────────────────┼───────┼──────────┼────────────┼─────────────┤  │
│  │ Normal Load     │ 1000  │ 30 min   │ 3-5 sec    │ 5 min       │  │
│  │ Peak Load       │ 3000  │ 15 min   │ 2-4 sec    │ 10 min      │  │
│  │ Stress Test     │ 5000+ │ Until    │ 1-2 sec    │ Continuous  │  │
│  │                 │       │ failure  │            │ increase    │  │
│  └─────────────────┴───────┴──────────┴────────────┴─────────────┘  │
│                                                                     │
│  5. SUCCESS CRITERIA                                                │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │ Metric              │ Threshold        │ Priority             │  │
│  ├─────────────────────┼──────────────────┼──────────────────────┤  │
│  │ P95 Response Time   │ < 200ms          │ Critical             │  │
│  │ P99 Response Time   │ < 500ms          │ Critical             │  │
│  │ Error Rate          │ < 0.1%           │ Critical             │  │
│  │ Throughput          │ > 5000 RPS       │ Important            │  │
│  │ CPU Utilization     │ < 80%            │ Important            │  │
│  │ Memory Utilization  │ < 85%            │ Important            │  │
│  └─────────────────────┴──────────────────┴──────────────────────┘  │
│                                                                     │
│  6. TEST ENVIRONMENT                                                │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │ • Environment: Production-like (75% production capacity)      │  │
│  │ • Database: Populated with realistic data volume              │  │
│  │ • Network: Isolated to prevent test traffic leakage           │  │
│  │ • Monitoring: Grafana dashboards, APM enabled                 │  │
│  └───────────────────────────────────────────────────────────────┘  │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

## Step 1: Define Test Objectives

### Good Objectives

Clear, measurable, and actionable:

✅ "Validate the system handles 10,000 concurrent users with P99 latency < 500ms"
✅ "Identify bottlenecks causing response time degradation above 5,000 RPS"
✅ "Establish performance baseline for /api/checkout endpoint"
✅ "Verify system recovers gracefully after traffic spike from 1k to 10k users"

### Poor Objectives

Vague or unmeasurable:

❌ "Test if the system is fast enough"
❌ "See how many users we can handle"
❌ "Check performance"

### Objective Categories

1. **Validation**: Confirm system meets requirements
   - "Validate P99 < 500ms under normal load"
   - "Confirm error rate < 0.1% at peak load"

2. **Discovery**: Find limits and issues
   - "Identify breaking point"
   - "Determine maximum sustainable throughput"

3. **Regression**: Detect performance changes
   - "Establish baseline for future comparisons"
   - "Detect performance regressions before deployment"

4. **Capacity Planning**: Inform infrastructure decisions
   - "Determine scaling requirements for 3x growth"
   - "Validate current infrastructure for expected Black Friday load"

## Step 2: Define Test Scope

### What to Include

**Critical User Paths**:
- Most frequent operations
- Revenue-generating flows
- Operations with SLAs

**Key Endpoints**:
- Public API endpoints
- Highest-traffic endpoints
- Operations with known performance concerns

### What to Exclude

**Out of Scope** (document why):
- Admin endpoints (low volume)
- Batch jobs (different testing approach)
- Features not in production yet
- Third-party dependencies (mock or test separately)

### Scope Definition Example

```
In Scope:
- User authentication (/api/login)
- Product catalog (/api/products, /api/search)
- Shopping cart (/api/cart)
- Checkout (/api/checkout, /api/payment)

Out of Scope:
- Admin panel (< 1% of traffic)
- Analytics endpoints (async, not customer-facing)
- Scheduled batch jobs (tested separately)
```

## Step 3: Workload Modeling

```
┌─────────────────────────────────────────────────────────────────────┐
│                    WORKLOAD MODELING                                 │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  TRANSACTION MIX (Based on Production Analytics)                    │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │                                                               │  │
│  │  Transaction              Weight   Avg Response   Target      │  │
│  │  ─────────────────────────────────────────────────────────    │  │
│  │  GET /api/products        40%      50ms          <100ms       │  │
│  │  GET /api/products/{id}   20%      30ms          <50ms        │  │
│  │  GET /api/search          15%      200ms         <300ms       │  │
│  │  POST /api/cart           10%      80ms          <150ms       │  │
│  │  POST /api/checkout       5%       500ms         <1000ms      │  │
│  │  GET /api/orders          10%      100ms         <200ms       │  │
│  │                                                               │  │
│  └───────────────────────────────────────────────────────────────┘  │
│                                                                     │
│  USER BEHAVIOR MODEL                                                │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │                                                               │  │
│  │  Session Flow:                                                │  │
│  │                                                               │  │
│  │  ┌─────────┐    ┌─────────┐    ┌─────────┐    ┌─────────┐     │  │
│  │  │ Login   │───►│ Browse  │───►│ Search  │───►│ View    │     │  │
│  │  │         │    │ (3-5x)  │    │ (1-2x)  │    │ Product │     │  │
│  │  └─────────┘    └─────────┘    └─────────┘    └────┬────┘     │  │
│  │                                                    │          │  │
│  │       70% exit ◄───────────────────────────────────┤          │  │
│  │                                                    │          │  │
│  │                                               ┌────▼────┐     │  │
│  │                                               │ Add to  │     │  │
│  │                                               │ Cart    │     │  │
│  │                                               └────┬────┘     │  │
│  │       20% exit ◄───────────────────────────────────┤          │  │
│  │                                                    │          │  │
│  │                                               ┌────▼────┐     │  │
│  │                                               │ Checkout│     │  │
│  │                                               │ (10%)   │     │  │
│  │                                               └─────────┘     │  │
│  │                                                               │  │
│  └───────────────────────────────────────────────────────────────┘  │
│                                                                     │
│  THINK TIME DISTRIBUTION                                            │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │ • Between page views: 3-10 seconds (normal distribution)      │  │
│  │ • Reading product details: 10-30 seconds                      │  │
│  │ • Filling checkout form: 30-60 seconds                        │  │
│  │                                                               │  │
│  │ Note: Think time prevents unrealistic load. Real users        │  │
│  │       don't click as fast as scripts can execute.             │  │
│  └───────────────────────────────────────────────────────────────┘  │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

### Building a Transaction Mix

**Step 1: Analyze Production Traffic**

Use analytics or logs to determine:
- What are the most frequent API calls?
- What is the ratio between different operations?
- What are typical user journeys?

**Step 2: Create Realistic Weights**

Example from e-commerce site:
```
Browse products:      40% (users browse frequently)
View product detail:  20% (narrowing search)
Search:               15% (using search)
Add to cart:          10% (considering purchase)
Checkout:             5%  (completing purchase)
View orders:          10% (checking status)
```

**Step 3: Model User Behavior**

Not just API calls, but user flows:
```
1. User logs in
2. Browses 3-5 product listings
3. Searches 1-2 times
4. Views 2-3 product details
5. 70% abandon, 30% add to cart
6. Of those in cart, 33% complete checkout
```

### Think Time

**Why Think Time Matters**:
Without think time, tests generate unrealistic load. Real users:
- Read content (5-30 seconds)
- Fill forms (10-60 seconds)
- Navigate between pages (2-10 seconds)

**Setting Think Time**:

```python
# Random think time
wait_time = between(2, 5)  # 2-5 seconds

# Fixed think time
wait_time = constant(3)    # Always 3 seconds

# Custom distribution
def custom_wait():
    return random.normalvariate(5, 2)  # Normal: mean=5s, stddev=2s
```

**Think Time by Action Type**:
- Quick navigation: 1-3 seconds
- Reading content: 5-15 seconds
- Filling forms: 15-60 seconds
- Decision-making: 10-30 seconds

## Step 4: Load Profile Design

### Load Profile Patterns

**Ramp-Up Pattern** (Most Common):
```
Users ▲
      │         ┌──────────────┐
      │        /                └──────
      │       /
      │      /
      └─────────────────────────────► Time
        Ramp  Sustain   Ramp-down
```

**Step Pattern** (Capacity Testing):
```
Users ▲
      │              ┌────────┐
      │        ┌─────┤        │
      │   ┌────┤     │        │
      │───┘    │     │        │
      └─────────────────────────────► Time
```

**Spike Pattern** (Spike Testing):
```
Users ▲
      │      ┌──┐
      │      │  │
      │──────┘  └──────
      └─────────────────────────────► Time
```

### Calculating Target Load

**From Expected Users**:
```
Given:
- Expected concurrent users: 10,000
- Average session duration: 10 minutes
- Average think time: 5 seconds
- Average response time: 100ms

RPS = Concurrent Users / (Think Time + Response Time)
RPS = 10,000 / (5 + 0.1)
RPS ≈ 1,960 requests/second
```

**From Business Metrics**:
```
Given:
- Expected daily users: 1,000,000
- Peak traffic is 3x average
- Users make 10 requests per session
- Peak hour gets 10% of daily traffic

Daily requests = 1,000,000 × 10 = 10M
Peak hour requests = 10M × 0.10 = 1M
Peak RPS (with 3x factor) = (1M / 3600) × 3 ≈ 833 RPS
```

## Step 5: Success Criteria Definition

### SMART Criteria

Success criteria should be:
- **Specific**: Exact metric and endpoint
- **Measurable**: Numeric threshold
- **Achievable**: Realistic given constraints
- **Relevant**: Aligns with objectives
- **Time-bound**: At what point in test

### Example Criteria

**Good**:
✅ "P95 response time for /api/checkout < 500ms at 5,000 RPS"
✅ "Error rate < 0.1% during entire 30-minute sustained load"
✅ "CPU utilization < 80% at peak load of 10,000 concurrent users"

**Poor**:
❌ "System should be fast"
❌ "No errors"
❌ "Good performance"

### Criteria Priority Levels

**Critical** (Must Pass):
- P95/P99 response time SLAs
- Error rate thresholds
- System stability (no crashes)

**Important** (Should Pass):
- Resource utilization targets
- Throughput minimums
- P50/P90 response times

**Nice to Have** (Informational):
- P99.9 latency
- Optimal resource usage
- Capacity headroom

## Step 6: Test Environment

### Environment Parity

**Production-Like Environment**:
- Same hardware/instance types
- Same software versions
- Same configuration
- Realistic data volumes

**Acceptable Deviations**:
- Smaller scale (e.g., 75% of production)
- Isolated network (no real traffic)
- Test databases (cloned from prod)

**Scaling Test Results**:
If test environment is 75% of production:
```
If test handles 7,500 RPS at P95=200ms
Then production should handle ~10,000 RPS at P95=200ms
```

### Data Volume

**Critical for Realistic Tests**:
- Database size affects query performance
- Cache hit rates depend on data distribution
- Index performance varies with data volume

**Minimum Data Requirements**:
- At least 50% of production data volume
- Representative data distribution
- Realistic data relationships

### Network Isolation

**Why It Matters**:
- Prevent test traffic from affecting production
- Ensure consistent network conditions
- Control external dependencies

**Isolation Strategies**:
- Separate VPC/network
- Use test subdomains
- Mock external services
- Load balancer rules to route test traffic

## Test Design Checklist

- [ ] Objectives clearly defined and measurable
- [ ] Scope documented (in and out)
- [ ] Workload model based on production analytics
- [ ] Transaction mix reflects real user behavior
- [ ] Think time configured realistically
- [ ] Load profile appropriate for test type
- [ ] Success criteria specific and measurable
- [ ] Test environment production-like
- [ ] Data volume realistic
- [ ] Network isolated
- [ ] Monitoring configured
- [ ] Stakeholders informed of test schedule
