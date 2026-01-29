# Chaos Experiment Design

## Chaos Experiment Template

```
┌─────────────────────────────────────────────────────────────────────┐
│                    CHAOS EXPERIMENT DESIGN                          │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  Experiment Name: [Descriptive name]                                │
│                                                                     │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │  1. HYPOTHESIS                                                │  │
│  │  "We believe that [system X] will [expected behavior]         │  │
│  │   when [failure condition] occurs"                            │  │
│  │                                                               │  │
│  │  Example:                                                     │  │
│  │  "We believe the order service will continue processing       │  │
│  │   orders with <500ms latency when one of three database       │  │
│  │   replicas becomes unavailable"                               │  │
│  └───────────────────────────────────────────────────────────────┘  │
│                                                                     │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │  2. STEADY STATE                                              │  │
│  │  Define normal system behavior to measure against:            │  │
│  │  • Requests per second: [XXX]                                 │  │
│  │  • P99 latency: [XXX]ms                                       │  │
│  │  • Error rate: [X.X]%                                         │  │
│  │  • CPU utilization: [XX]%                                     │  │
│  └───────────────────────────────────────────────────────────────┘  │
│                                                                     │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │  3. METHOD                                                    │  │
│  │  Failure Injection:                                           │  │
│  │  • What: [Specific failure to inject]                         │  │
│  │  • How: [Tool/command/mechanism]                              │  │
│  │  • Duration: [How long]                                       │  │
│  │  • Scope: [Which components affected]                         │  │
│  └───────────────────────────────────────────────────────────────┘  │
│                                                                     │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │  4. BLAST RADIUS CONTROLS                                     │  │
│  │  • Environment: [Staging/Canary/Production %]                 │  │
│  │  • Duration limit: [Max time before auto-stop]                │  │
│  │  • Kill switch: [How to abort immediately]                    │  │
│  │  • Rollback: [Recovery procedure]                             │  │
│  └───────────────────────────────────────────────────────────────┘  │
│                                                                     │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │  5. OBSERVABILITY                                             │  │
│  │  Metrics to monitor:                                          │  │
│  │  • [Metric 1: Dashboard link]                                 │  │
│  │  • [Metric 2: Dashboard link]                                 │  │
│  │  Alerts configured: [Y/N]                                     │  │
│  │  Recording: [Y/N]                                             │  │
│  └───────────────────────────────────────────────────────────────┘  │
│                                                                     │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │  6. RESULTS                                                   │  │
│  │  Hypothesis confirmed: [Y/N/Partial]                          │  │
│  │  Observations:                                                │  │
│  │  • [Finding 1]                                                │  │
│  │  • [Finding 2]                                                │  │
│  │  Action items:                                                │  │
│  │  • [Improvement 1]                                            │  │
│  │  • [Improvement 2]                                            │  │
│  └───────────────────────────────────────────────────────────────┘  │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

## Complete Example Experiment

```
┌─────────────────────────────────────────────────────────────────────┐
│  EXPERIMENT: Database Failover Under Load                           │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  HYPOTHESIS:                                                        │
│  "The order processing system will maintain <1% error rate          │
│   and <500ms P99 latency during primary database failover           │
│   while under normal production load"                               │
│                                                                     │
│  STEADY STATE:                                                      │
│  • Orders/second: 150                                               │
│  • P99 latency: 180ms                                               │
│  • Error rate: 0.01%                                                │
│  • Active connections: 50                                           │
│                                                                     │
│  METHOD:                                                            │
│  1. Generate load: 150 orders/sec using load generator              │
│  2. Wait 2 minutes for steady state                                 │
│  3. Trigger database failover via cloud API                         │
│  4. Monitor for 5 minutes post-failover                             │
│  5. Verify replica promotion completed                              │
│                                                                     │
│  BLAST RADIUS:                                                      │
│  • Environment: Staging (mirrored production config)                │
│  • Duration: Max 10 minutes total                                   │
│  • Kill switch: Script to promote replica immediately               │
│                                                                     │
│  RESULTS:                                                           │
│  ✗ HYPOTHESIS DISPROVED                                             │
│  • Error rate spiked to 12% for 45 seconds                          │
│  • P99 latency reached 2.3s during failover                         │
│  • Connection pool exhausted (observed queuing)                     │
│                                                                     │
│  ACTION ITEMS:                                                      │
│  1. Implement connection pool pre-warming                           │
│  2. Add retry with backoff for DB connections                       │
│  3. Configure shorter health check intervals                        │
│  4. Re-run experiment after fixes                                   │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

## Experiment Design Best Practices

### 1. Start with a Clear Hypothesis
Your hypothesis should be specific, measurable, and testable. Avoid vague statements like "the system should be resilient." Instead, define concrete metrics.

**Bad**: "The system should handle database failures"
**Good**: "The system will maintain <1% error rate and <500ms P99 latency when the primary database fails"

### 2. Define Steady State Precisely
Establish baseline metrics before the experiment. You need to know what "normal" looks like to detect deviations.

Key metrics to track:
- Throughput (requests/second)
- Latency (P50, P95, P99)
- Error rate
- Resource utilization
- Custom business metrics

### 3. Plan the Method in Detail
Document exactly how you'll inject the failure:
- What tool or mechanism will you use?
- How long will the failure persist?
- What scope will it affect?
- When will you trigger it?

### 4. Set Up Observability First
Before running any experiment:
- Ensure all relevant metrics are instrumented
- Set up dashboards to visualize impact
- Configure alerts for abnormal behavior
- Arrange to record the experiment for post-analysis

### 5. Document Results Thoroughly
Capture not just whether the hypothesis was proven, but:
- Unexpected behaviors observed
- Side effects and cascading impacts
- Time to recovery
- Action items for improvement
- Lessons learned

### 6. Iterate and Improve
Failed hypotheses are valuable! They reveal weaknesses. After fixing issues:
- Re-run the experiment to verify the fix
- Expand the experiment scope
- Design new experiments based on learnings
