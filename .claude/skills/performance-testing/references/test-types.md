# Performance Test Types

This reference provides detailed information about different types of performance tests, their purposes, and when to use each.

## Test Type Overview

```
┌─────────────────────────────────────────────────────────────────────┐
│                    PERFORMANCE TEST TYPES                            │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  LOAD TEST                                                          │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │ Purpose: Validate system under expected production load       │  │
│  │ Duration: 15-60 minutes                                       │  │
│  │ Load: Normal expected traffic                                 │  │
│  │ Key Question: "Can we handle expected demand?"                │  │
│  │                                                               │  │
│  │  Load ▲                                                       │  │
│  │       │    ┌────────────────────────┐                         │  │
│  │       │    │ Sustained normal load  │                         │  │
│  │       │ ───┘                        └────                     │  │
│  │       └──────────────────────────────────► Time               │  │
│  └───────────────────────────────────────────────────────────────┘  │
│                                                                     │
│  STRESS TEST                                                        │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │ Purpose: Find system breaking point and failure behavior      │  │
│  │ Duration: Until failure or defined limit                      │  │
│  │ Load: Progressively increasing beyond capacity                │  │
│  │ Key Question: "Where does it break and how?"                  │  │
│  │                                                               │  │
│  │  Load ▲                          Breaking                     │  │
│  │       │                            point                      │  │
│  │       │                              ╳                        │  │
│  │       │                          ╱                            │  │
│  │       │                      ╱                                │  │
│  │       │                  ╱                                    │  │
│  │       │              ╱                                        │  │
│  │       │          ╱                                            │  │
│  │       └──────────────────────────────────► Time               │  │
│  └───────────────────────────────────────────────────────────────┘  │
│                                                                     │
│  SPIKE TEST                                                         │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │ Purpose: Validate handling of sudden load increases           │  │
│  │ Duration: 5-15 minutes per spike                              │  │
│  │ Load: Sudden dramatic increase, then decrease                 │  │
│  │ Key Question: "How do we handle traffic spikes?"              │  │
│  │                                                               │  │
│  │  Load ▲                                                       │  │
│  │       │         ┌────┐                                        │  │
│  │       │         │    │    ┌──┐                                │  │
│  │       │         │    │    │  │                                │  │
│  │       │ ────────┘    └────┘  └────────                        │  │
│  │       └──────────────────────────────────► Time               │  │
│  └───────────────────────────────────────────────────────────────┘  │
│                                                                     │
│  SOAK/ENDURANCE TEST                                                │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │ Purpose: Detect memory leaks, resource exhaustion over time   │  │
│  │ Duration: 4-24+ hours                                         │  │
│  │ Load: Moderate sustained load                                 │  │
│  │ Key Question: "Are there time-based degradations?"            │  │
│  │                                                               │  │
│  │  Load ▲                                                       │  │
│  │       │ ──────────────────────────────────                    │  │
│  │       │ Sustained moderate load for extended period           │  │
│  │       │                                                       │  │
│  │       └──────────────────────────────────► Time (hours)       │  │
│  └───────────────────────────────────────────────────────────────┘  │
│                                                                     │
│  CAPACITY TEST                                                      │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │ Purpose: Determine maximum sustainable load                   │  │
│  │ Duration: Step-wise increase until SLA breach                 │  │
│  │ Load: Incremental increases                                   │  │
│  │ Key Question: "What's our maximum capacity?"                  │  │
│  │                                                               │  │
│  │  Load ▲                              Capacity                 │  │
│  │       │                              limit                    │  │
│  │       │                        ┌──────┤                       │  │
│  │       │                  ┌─────┘                              │  │
│  │       │            ┌─────┘                                    │  │
│  │       │      ┌─────┘                                          │  │
│  │       │ ─────┘                                                │  │
│  │       └──────────────────────────────────► Time               │  │
│  └───────────────────────────────────────────────────────────────┘  │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

## Load Testing

**Purpose**: Validate that the system can handle expected production load levels.

**When to Use**:
- Before production releases
- After infrastructure changes
- To validate SLA compliance
- As part of regression testing

**Duration**: 15-60 minutes typically

**Load Pattern**: Sustained normal load matching expected production traffic

**Success Criteria**:
- Response times meet SLAs under expected load
- Error rates remain below threshold
- Resource utilization stays within acceptable bounds
- No degradation over test duration

## Stress Testing

**Purpose**: Find the breaking point of the system and understand failure modes.

**When to Use**:
- To understand system limits
- For capacity planning
- To validate graceful degradation
- To test recovery behavior

**Duration**: Until failure or defined limit reached

**Load Pattern**: Progressively increasing load beyond normal capacity

**What to Observe**:
- At what load level does performance degrade?
- How does the system fail? (graceful vs catastrophic)
- Can the system recover after load reduction?
- What component fails first?

## Spike Testing

**Purpose**: Validate system handling of sudden, dramatic load increases.

**When to Use**:
- Systems with unpredictable traffic patterns
- E-commerce during flash sales
- News sites during breaking events
- Systems with auto-scaling

**Duration**: 5-15 minutes per spike

**Load Pattern**: Sudden increase from baseline to peak, then drop back

**Success Criteria**:
- System handles spike without crashes
- Response times acceptable during spike
- Auto-scaling triggers appropriately
- Quick recovery to normal after spike

## Soak/Endurance Testing

**Purpose**: Detect memory leaks, resource exhaustion, and degradation over time.

**When to Use**:
- Testing for memory leaks
- Validating long-running stability
- Connection pool leak detection
- File handle exhaustion issues

**Duration**: 4-24+ hours (extended periods)

**Load Pattern**: Moderate sustained load over extended time

**What to Monitor**:
- Memory usage trends (should be stable, not growing)
- Connection pool usage
- File descriptors
- Disk space
- Database connections
- Cache behavior

**Warning Signs**:
- Gradual response time increase
- Memory usage trending upward
- Growing queue depths
- Increasing error rates over time

## Capacity Testing

**Purpose**: Determine the maximum sustainable load the system can handle.

**When to Use**:
- Infrastructure sizing decisions
- Capacity planning
- Cost optimization
- Setting operational limits

**Duration**: Step-wise increase until SLA breach

**Load Pattern**: Incremental load increases with plateaus at each level

**Process**:
1. Start at baseline load
2. Increase by increment (e.g., +500 users)
3. Sustain for observation period (10-15 minutes)
4. Monitor metrics against SLAs
5. Repeat until SLA violation
6. Capacity = last successful load level

**Output**: Maximum sustainable capacity before SLA violations

## Choosing the Right Test Type

| Scenario | Test Type |
|----------|-----------|
| Pre-release validation | Load Test |
| Finding system limits | Stress Test |
| Flash sale preparation | Spike Test |
| Memory leak detection | Soak Test |
| Infrastructure sizing | Capacity Test |
| Black Friday readiness | Load + Spike |
| New deployment validation | Load + Stress |
