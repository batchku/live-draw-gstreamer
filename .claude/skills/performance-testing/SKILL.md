---
name: performance-testing
description: Designs and implements performance tests including load testing, stress testing, benchmarking, and profiling. Use when validating system performance under load, identifying bottlenecks, establishing performance baselines, capacity planning, or ensuring systems meet SLAs and performance requirements.
allowed-tools: Read Glob Grep
---

# Performance Testing Skill

This skill helps test engineers design and implement performance tests that validate system behavior under load. Use it to ensure systems meet performance requirements and identify bottlenecks before they impact users.

## ⛔ CRITICAL: Test Duration Restriction

All tests you create MUST complete execution in under 1 minute total runtime.
Tests MUST be short, focused, and fast-executing.
NEVER create tests that:
- Require long-running operations (>60 seconds)

If comprehensive testing requires longer duration, break into multiple focused test files that each run quickly.
Tests exceeding 1 minute duration will cause task FAILURE.

**For performance testing specifically**:
- Use synthetic, accelerated load patterns that complete within 60 seconds
- Focus on quick smoke tests that validate performance characteristics
- Use minimal ramp-up periods (5-10 seconds max)
- Limit sustained load periods to 30-40 seconds max
- Design rapid spike tests (10-second spike + 10-second recovery)
- For endurance/soak testing, validate the test harness only, not actual long-duration runs

## When to Use This Skill

Use this skill when you need to:
- Design load tests to validate system capacity
- Identify performance bottlenecks
- Establish performance baselines and benchmarks
- Plan for capacity and scalability
- Validate SLA compliance under load
- Stress test to find breaking points
- Profile applications for optimization opportunities
- Perform soak/endurance testing for memory leaks

## Purpose

This skill provides a systematic approach to performance testing within the 60-second constraint:

1. **Load Testing**: Validate behavior under expected load (30-45 second tests)
2. **Stress Testing**: Find initial breaking indicators (40-50 second progressive tests)
3. **Endurance Testing**: Design (but not execute) long-duration tests; validate test harness only
4. **Spike Testing**: Validate handling of sudden load increases (20-30 second tests)
5. **Benchmarking**: Establish baselines and compare performance (quick comparative tests)
6. **Profiling**: Identify code-level performance issues (short profiling runs)

## Performance Testing Overview

### Test Types Quick Reference (All Under 60 Seconds)

**Load Test**: Validate system handles expected production load (30-45 seconds sustained load after 10-second ramp)
**Stress Test**: Find initial breaking indicators (progressive increase over 40-50 seconds)
**Spike Test**: Validate handling of sudden load spikes (10-second spike + 10-second recovery + margins)
**Soak/Endurance Test**: Validate test harness only (NOT actual long-duration runs - design but don't execute)
**Capacity Test**: Determine initial capacity indicators (step-wise increase over 50 seconds max)

See [Test Types Reference](references/test-types.md) for detailed information on each test type, load patterns, and when to use them.

### Key Metrics

**Latency Metrics**: P50, P90, P95, P99, P99.9 (use percentiles, not averages)
**Throughput Metrics**: RPS (requests/sec), TPS (transactions/sec), concurrent users
**Error Metrics**: Error rate, timeout rate, 5xx rate, 4xx rate
**Resource Metrics**: CPU, memory, disk I/O, network I/O, connection pools, thread pools

See [Metrics Reference](references/metrics.md) for detailed metric definitions, targets, and interpretation.

### Tool Selection

**Developer-friendly**: k6 (JavaScript), Locust (Python)
**High-performance**: Gatling (Scala), wrk/wrk2 (C)
**GUI-based**: JMeter
**Quick YAML tests**: Artillery

See [Tools Reference](references/tools.md) for tool comparisons, specific examples, and selection guidance.

## Workflow

**CRITICAL REMINDER**: Every test you create must complete in under 60 seconds total. Design all test plans with this constraint in mind.

### 1. Define Test Objectives

Start with clear, measurable objectives:
- What questions are you answering?
- What are the success criteria?
- What are the SLAs or performance requirements?
- What is the expected peak load?

Example: "Validate the system handles 10,000 concurrent users with P99 latency < 500ms and error rate < 0.1%"

### 2. Design Test Plan

Create a comprehensive test plan covering:

**Objectives**: What you're testing and why
**Scope**: What's in scope and out of scope (keep focused for 60-second limit)
**Workload Model**: User types, actions, and transaction mix (critical paths only)
**Load Profile**: Ramp pattern (5-10s max), duration (30-45s max), think time (0.5-2s)
**Success Criteria**: Specific metric thresholds with priorities
**Test Environment**: Configuration, data volume, isolation
**Duration Budget**: Ensure total test time < 60 seconds (calculate: ramp + sustain + ramp-down)

See [Test Design Reference](references/test-design.md) for detailed test plan template and workload modeling guidance.

### 3. Prepare Environment

**Environment Requirements**:
- Production-like configuration (same hardware, software, network)
- Realistic data volume (minimum 50% of production)
- Network isolation (prevent test traffic affecting production)
- Monitoring enabled (APM, dashboards, log aggregation)

**Pre-Test Checklist**:
- Environment matches production configuration
- Database populated with realistic data
- Caches warmed up (or intentionally cold)
- No other tests running on shared resources
- Monitoring accessible and collecting data

### 4. Create Test Scripts

**Key Elements (Remember: 60-second total limit)**:
- Minimal think time (0.5-2 seconds between actions max)
- Focused transaction mix (test critical paths only)
- User behavior variance (not all users identical)
- Proper authentication and session handling
- Response validation (check status codes and content)
- Total test duration calculation to ensure < 60 seconds

**Example Pattern** (k6) - MUST complete in under 60 seconds:
```javascript
export const options = {
  stages: [
    { duration: '10s', target: 100 },   // Warm up
    { duration: '30s', target: 100 },   // Measure
    { duration: '10s', target: 200 },   // Peak load
    { duration: '5s', target: 0 },      // Ramp down
  ],
  thresholds: {
    http_req_duration: ['p(95)<200', 'p(99)<500'],
    http_req_failed: ['rate<0.01'],
  },
};
// Total duration: 55 seconds (within 60-second limit)
```

### 5. Execute Tests

**Execution Steps**:
1. Run smoke test with minimal load (validate script works)
2. Start monitoring dashboards
3. Execute main test with gradual ramp-up
4. Monitor continuously for warning signs
5. Cool down and verify system recovery

**Watch For**:
- Response time trends (stable is good, increasing is concerning)
- Error rate (should stay consistently low)
- Resource utilization (< 80% is healthy)
- Queue depths and connection pools

See [Execution Reference](references/execution.md) for detailed pre-test checklist, monitoring guidance, and post-test procedures.

### 6. Analyze Results

**Analysis Steps**:
1. Compare results against success criteria
2. Identify trends and patterns
3. Correlate metrics (when response time increased, what else changed?)
4. Identify bottlenecks

**Report Structure**:
- Executive Summary (Pass/Fail, key findings)
- Test Configuration
- Results vs Targets (table format)
- Response Time Distribution (percentiles)
- Resource Utilization
- Bottlenecks Identified
- Recommendations

See [Bottlenecks Reference](references/bottlenecks.md) for common bottleneck patterns, investigation workflows, and typical solutions.

### 7. Report and Iterate

**Deliverables**:
- Test results report with clear pass/fail status
- Performance baseline data for future regression testing
- Bottleneck analysis with recommendations
- Raw metrics and logs archived for reference

**Follow-Up**:
- Fix identified bottlenecks
- Re-test to validate improvements
- Establish performance baseline
- Schedule regular regression tests

## Common Bottleneck Patterns

**Database**: High query latency, connection pool exhaustion, lock contention
→ Add indexes, optimize queries, add caching, scale database

**CPU**: High utilization (>80%), response time degrades with load
→ Optimize hot code paths, add caching, horizontal scaling

**Memory**: Growing usage over time, frequent GC, OOM errors
→ Fix memory leaks, optimize data structures, tune GC

**Network**: High latency, bandwidth saturation, timeouts
→ Compress payloads, reduce chatty calls, use CDN

**Concurrency**: Thread pool exhaustion, queuing, lock contention
→ Tune thread pools, use async I/O, reduce lock granularity

See [Bottlenecks Reference](references/bottlenecks.md) for detailed investigation steps and solutions.

## Best Practices

1. **Stay Under 60 Seconds**: ALL tests must complete in under 1 minute - design accordingly
2. **Test in Production-Like Environment**: Same hardware, realistic data volume, production configuration
3. **Use Realistic Workload**: Base on production patterns, minimal think time, focused transaction types
4. **Minimal Warm Up**: 5-10 seconds max for cache population and connection establishment
5. **Monitor Everything**: Client metrics, server metrics, infrastructure, dependencies
6. **Establish Baselines**: Run regularly with quick smoke tests, detect regressions early
7. **Test Incrementally**: Start low, increase rapidly (within 60-second window)
8. **Automate**: Include in CI/CD with fast-executing tests only
9. **Document and Share**: Report findings clearly, track improvements, share learnings
10. **Break Into Focused Tests**: If comprehensive testing needed, create multiple 60-second test files

See [Best Practices Reference](references/best-practices.md) for detailed guidance on environment setup, automation, and avoiding common pitfalls.

## Question Strategy

### For Test Planning

Ask these questions to design effective tests:

1. "What are the SLAs or performance requirements?"
2. "What is the expected peak load (concurrent users or RPS)?"
3. "What are the critical user journeys?"
4. "What does production traffic pattern look like?"
5. "How close is the test environment to production?"

### For Bottleneck Analysis

When investigating performance issues:

1. "Which resource is saturated first under load?" (CPU, memory, I/O, connections)
2. "Where is the time being spent in slow requests?" (use profiling)
3. "What correlates with performance degradation?" (when latency increased, what changed?)
4. "Are there trends over time?" (memory growing, response time increasing)

### For Environment Validation

Before running tests:

1. "Does test environment match production configuration?"
2. "Is test data volume realistic (at least 50% of production)?"
3. "Are all dependencies included and realistic?"
4. "Is monitoring capturing all necessary metrics?"

## References

This skill includes detailed reference documentation for deeper technical information:

- **[Test Types](references/test-types.md)**: Detailed descriptions of load, stress, spike, soak, and capacity tests with load patterns and selection guidance
- **[Metrics](references/metrics.md)**: Comprehensive metric definitions, percentile explanations, target setting, and interpretation guidance
- **[Tools](references/tools.md)**: Tool comparisons, specific examples for k6/Locust/Gatling/JMeter, and selection criteria
- **[Test Design](references/test-design.md)**: Test plan templates, workload modeling, transaction mix design, and success criteria definition
- **[Bottlenecks](references/bottlenecks.md)**: Common bottleneck patterns, investigation workflows, profiling techniques, and typical solutions
- **[Execution](references/execution.md)**: Pre-test checklists, monitoring during tests, results analysis, and post-test procedures
- **[Best Practices](references/best-practices.md)**: Environment setup, automation, documentation, regression testing, and common pitfalls to avoid

## Additional Resources

Industry references for performance testing:
- "The Art of Capacity Planning" by John Allspaw
- "High Performance Browser Networking" by Ilya Grigorik
- Google SRE Book - Chapter on Load Testing
- k6, Gatling, and Locust official documentation
