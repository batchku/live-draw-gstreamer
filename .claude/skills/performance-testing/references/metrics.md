# Performance Metrics

This reference covers key performance metrics to measure, how to interpret them, and what targets to set.

## Key Metrics Categories

```
┌─────────────────────────────────────────────────────────────────────┐
│                    PERFORMANCE METRICS                               │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  LATENCY METRICS                                                    │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │ Metric        │ Description                │ Target Example   │  │
│  ├───────────────┼────────────────────────────┼──────────────────┤  │
│  │ P50 (median)  │ 50% of requests faster     │ < 100ms          │  │
│  │ P90           │ 90% of requests faster     │ < 200ms          │  │
│  │ P95           │ 95% of requests faster     │ < 300ms          │  │
│  │ P99           │ 99% of requests faster     │ < 500ms          │  │
│  │ P99.9         │ 99.9% of requests faster   │ < 1000ms         │  │
│  │ Max           │ Slowest request            │ < 5000ms         │  │
│  └───────────────┴────────────────────────────┴──────────────────┘  │
│                                                                     │
│  Why Percentiles Matter:                                            │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │ Average latency = 50ms (looks good!)                          │  │
│  │ P99 latency = 5000ms (5% of users waiting 5+ seconds!)        │  │
│  │                                                               │  │
│  │ Averages hide outliers. High percentiles reveal real UX.      │  │
│  └───────────────────────────────────────────────────────────────┘  │
│                                                                     │
│  THROUGHPUT METRICS                                                 │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │ Metric             │ Description              │ Example        │  │
│  ├────────────────────┼──────────────────────────┼────────────────┤  │
│  │ Requests/sec (RPS) │ Requests handled/second  │ 10,000 RPS     │  │
│  │ Transactions/sec   │ Complete txns/second     │ 500 TPS        │  │
│  │ Concurrent users   │ Simultaneous active      │ 5,000 users    │  │
│  │ Throughput (bytes) │ Data transferred/second  │ 100 MB/s       │  │
│  └────────────────────┴──────────────────────────┴────────────────┘  │
│                                                                     │
│  ERROR METRICS                                                      │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │ Metric          │ Description                │ Target         │  │
│  ├─────────────────┼────────────────────────────┼────────────────┤  │
│  │ Error rate      │ % of failed requests       │ < 0.1%         │  │
│  │ Timeout rate    │ % of timed out requests    │ < 0.01%        │  │
│  │ 5xx rate        │ % of server errors         │ < 0.1%         │  │
│  │ 4xx rate        │ % of client errors         │ Context-dep.   │  │
│  └─────────────────┴────────────────────────────┴────────────────┘  │
│                                                                     │
│  RESOURCE METRICS                                                   │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │ Metric          │ What to Watch                │ Warning Sign  │  │
│  ├─────────────────┼──────────────────────────────┼───────────────┤  │
│  │ CPU utilization │ Processing capacity          │ > 80%         │  │
│  │ Memory usage    │ RAM consumption              │ > 85%         │  │
│  │ Disk I/O        │ Read/write operations        │ High wait     │  │
│  │ Network I/O     │ Bandwidth usage              │ Saturation    │  │
│  │ Connection pool │ DB/cache connections         │ Exhaustion    │  │
│  │ Thread pool     │ Worker thread availability   │ Queuing       │  │
│  │ GC pause time   │ Garbage collection duration  │ > 100ms       │  │
│  └─────────────────┴──────────────────────────────┴───────────────┘  │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

## Latency Metrics Deep Dive

### Why Percentiles Over Averages

Averages are misleading for latency measurement:

**Example Problem**:
- 95% of requests complete in 50ms
- 5% of requests complete in 5000ms
- **Average**: 297ms (looks reasonable)
- **Reality**: 5% of users experience terrible performance

**Solution**: Use percentiles to understand the full distribution.

### Understanding Percentile Targets

| Percentile | What It Means | Why It Matters |
|------------|---------------|----------------|
| P50 (Median) | Half of requests are faster | Typical user experience |
| P90 | 90% of requests are faster | Most users' experience |
| P95 | 95% of requests are faster | Good coverage of normal cases |
| P99 | 99% of requests are faster | Catches most outliers |
| P99.9 | 99.9% of requests are faster | Tail latency - critical for large scale |
| Max | Absolute worst case | Debugging extreme outliers |

### Latency Distribution Visualization

```
┌─────────────────────────────────────────────────────────────────────┐
│                    LATENCY HISTOGRAM                                 │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  Requests                                                           │
│  ▲                                                                  │
│  │                                                                  │
│  │  ████                                                            │
│  │  ████                                                            │
│  │  ████████                                                        │
│  │  ████████                                                        │
│  │  ████████████                                                    │
│  │  ████████████                                                    │
│  │  ████████████████                                                │
│  │  ████████████████████                                            │
│  │  ████████████████████████                                        │
│  │  ████████████████████████████████                    ▌           │
│  └──────────────────────────────────────────────────────────► ms    │
│     │  P50  │   P90  │  P95 │   P99  │         P99.9    │           │
│    50ms   100ms   150ms  200ms  300ms           1000ms              │
│                                                                     │
│  Most requests fast (P50=50ms)                                      │
│  Long tail causes P99.9=1000ms                                      │
│  Investigate: What causes those slow outliers?                      │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

### Setting Latency Targets

Consider these factors when setting targets:

1. **User Experience Requirements**
   - Interactive: < 100ms feels instant
   - Quick: < 300ms feels responsive
   - Acceptable: < 1000ms is tolerable
   - Slow: > 1000ms feels sluggish

2. **Business Context**
   - Financial trading: microseconds matter
   - E-commerce checkout: < 500ms optimal
   - Content browsing: < 1000ms acceptable
   - Background jobs: seconds are fine

3. **Technical Constraints**
   - Database query time
   - External API calls
   - Network latency
   - Processing complexity

## Throughput Metrics

### Requests Per Second (RPS)

**Definition**: Number of requests the system handles per second.

**How to Calculate**:
```
RPS = Total Requests / Test Duration (seconds)
```

**What Good Looks Like**:
- RPS should scale linearly with load (up to capacity)
- Sudden drop in RPS indicates saturation
- RPS should remain stable during sustained load

### Concurrent Users vs RPS

These are related but different:

**Concurrent Users**: Number of active users at any moment
**RPS**: Rate of requests being processed

**Relationship**:
```
RPS = (Concurrent Users) / (Average Think Time + Average Response Time)
```

**Example**:
- 1000 concurrent users
- 2 second think time
- 100ms average response time
- RPS ≈ 1000 / (2 + 0.1) ≈ 476 RPS

## Error Metrics

### Error Rate Calculation

```
Error Rate % = (Failed Requests / Total Requests) × 100
```

### Error Rate Targets

| System Type | Typical Target | Aggressive Target |
|-------------|----------------|-------------------|
| Public API | < 1% | < 0.1% |
| Internal Service | < 0.5% | < 0.01% |
| Critical Path | < 0.1% | < 0.001% |

### Types of Errors to Track

1. **5xx Errors** (Server-side)
   - 500 Internal Server Error
   - 502 Bad Gateway
   - 503 Service Unavailable
   - 504 Gateway Timeout

2. **4xx Errors** (Client-side)
   - 400 Bad Request (expected from bad input)
   - 401 Unauthorized (expected without auth)
   - 429 Too Many Requests (rate limiting)

3. **Timeouts**
   - Request timeout before completion
   - Often indicates backend overload

### When Errors Are Expected

Not all errors indicate problems:
- 401 errors during auth flow testing
- 400 errors during input validation testing
- 429 errors during rate limit testing

Separate expected vs unexpected errors in your analysis.

## Resource Metrics

### CPU Utilization

**Target**: < 80% during normal load

**Why 80%?**
- Leaves headroom for traffic spikes
- Prevents CPU saturation degradation
- Allows room for background tasks

**Warning Signs**:
- Sustained > 80%: Risk of degradation
- > 90%: Likely performance issues
- 100%: CPU-bound, immediate action needed

### Memory Utilization

**Target**: < 85% during normal load

**What to Monitor**:
- Heap usage (application memory)
- Non-heap usage (JVM metadata, etc.)
- Native memory
- Memory leak trends

**Warning Signs**:
- Growing memory over time (leak)
- Frequent garbage collection
- Out of Memory errors
- Swapping to disk

### Connection Pools

**Database Connection Pool**:
- Monitor: Active connections vs pool size
- Warning: > 80% utilization
- Crisis: Pool exhaustion (requests waiting)

**Thread Pool**:
- Monitor: Active threads vs pool size
- Warning: Queue depth growing
- Crisis: Rejected tasks

### Disk I/O

**Metrics to Track**:
- Read/write operations per second
- Disk utilization %
- I/O wait time
- Queue depth

**Warning Signs**:
- High I/O wait time
- Disk utilization > 80%
- Growing queue depth

## Composite Metrics

### Apdex Score

Application Performance Index - user satisfaction metric:

```
Apdex = (Satisfied + (Tolerating / 2)) / Total Samples

Where:
- Satisfied: Response time <= T
- Tolerating: Response time <= 4T
- Frustrated: Response time > 4T
```

**Example** (T = 100ms):
- Satisfied: <= 100ms
- Tolerating: <= 400ms
- Frustrated: > 400ms

**Apdex Score Interpretation**:
- 1.0 = Perfect
- 0.94 - 1.0 = Excellent
- 0.85 - 0.94 = Good
- 0.70 - 0.85 = Fair
- 0.50 - 0.70 = Poor
- < 0.50 = Unacceptable

## Metric Collection Best Practices

1. **Consistent Time Intervals**: Collect at regular intervals (1s, 5s, 10s)
2. **Full Distribution**: Capture percentiles, not just averages
3. **Tag Metadata**: Include endpoint, region, instance ID
4. **Client + Server**: Measure from both perspectives
5. **Retention**: Keep raw data for 7 days, aggregated for 90 days
