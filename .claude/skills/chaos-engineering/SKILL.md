---
name: chaos-engineering
description: Designs and implements chaos engineering experiments including failure injection and resilience testing. Use when validating system resilience, testing failure handling, designing fault injection tests, verifying graceful degradation, or assessing recovery capabilities.
allowed-tools: Read Glob Grep
---

# Chaos Engineering Skill

This skill helps test engineers and SREs design and implement chaos engineering practices to validate system resilience. Use it to proactively discover weaknesses before they cause production incidents.

## ⛔ CRITICAL: Test Duration Restriction

All tests you create MUST complete execution in under 1 minute total runtime.
Tests MUST be short, focused, and fast-executing.
NEVER create tests that:
- Require long-running operations (>60 seconds)

If comprehensive testing requires longer duration, break into multiple focused test files that each run quickly.
Tests exceeding 1 minute duration will cause task FAILURE.

**For chaos engineering specifically**:
- Use rapid failure injection (inject failure, observe 10-20 seconds, abort)
- Test quick recovery scenarios (fail → recover within 30-40 seconds)
- Design experiments with short observation windows (20-30 seconds max)
- Use synthetic, accelerated failure patterns
- Focus on unit-level resilience tests that run quickly
- For multi-minute chaos experiments, design the experiment plan only - don't execute
- Break comprehensive chaos test suites into multiple fast-running experiments

## When to Use This Skill

Use this skill when you need to:
- Design chaos experiments to test system resilience
- Implement failure injection tests
- Validate fault tolerance and graceful degradation
- Test disaster recovery procedures
- Assess system behavior under adverse conditions
- Build confidence in system reliability
- Identify single points of failure

## Purpose

This skill provides a systematic approach to chaos engineering:

1. **Experiment Design**: Create hypothesis-driven chaos experiments
2. **Failure Injection**: Implement various failure modes safely
3. **Resilience Validation**: Verify systems handle failures gracefully
4. **Blast Radius Control**: Ensure experiments don't cause outages
5. **Learning Culture**: Build knowledge from chaos findings

---

## Core Chaos Engineering Concepts

### Key Principles

Chaos engineering is based on five core principles:

1. **Hypothesis-Driven**: Define expected steady-state behavior and hypothesize how the system will respond to failures
2. **Real-World Events**: Inject failures that actually occur in production (server crashes, network issues, dependency failures)
3. **Production Testing**: Eventually test in production where real behavior emerges (with proper safeguards)
4. **Continuous Practice**: Automate experiments to catch regressions as systems evolve
5. **Minimize Blast Radius**: Start small, expand gradually, always have abort mechanisms ready

See [Principles and Maturity Model](references/principles-and-maturity.md) for detailed guidance on chaos maturity levels and progression.

### Experiment Structure

Every chaos experiment follows this structure:

1. **Hypothesis**: "We believe [system] will [behavior] when [failure] occurs"
2. **Steady State**: Define baseline metrics (latency, error rate, throughput)
3. **Method**: Specify what failure to inject, how, for how long
4. **Blast Radius Controls**: Environment, duration limits, kill switch, rollback plan
5. **Observability**: Metrics to monitor, dashboards, alerts
6. **Results**: Hypothesis confirmed/disproved, findings, action items

See [Experiment Design](references/experiment-design.md) for templates, examples, and detailed best practices.

---

## What Can You Test?

### Failure Categories

Chaos experiments typically test three categories of failures:

**Infrastructure Failures**
- Server/instance termination, CPU/memory exhaustion
- Network latency, packet loss, partitions
- Storage failures, disk full, corrupted data

**Application Failures**
- Service crashes, slow responses, error responses
- Dependency failures (database, cache, message queue, external APIs)
- Data failures (invalid data, missing data, duplicates)

**Distributed System Failures**
- Consensus issues (leader election, split-brain, quorum loss)
- Replication failures (lag, inconsistency, failover)
- Load balancer and cross-region failures

See [Failure Injection Types](references/failure-injection-types.md) for comprehensive catalogs with injection methods.

### Resilience Patterns to Validate

Your chaos experiments should verify that these patterns work:

- **Circuit Breaker**: Opens after threshold failures, fails fast, closes after recovery
- **Retry with Backoff**: Retries with increasing delays, no retry storms
- **Timeout Handling**: Requests timeout appropriately, resources released
- **Bulkhead**: Resource isolation prevents cascading failures
- **Fallback**: Degraded service provided when primary fails
- **Graceful Degradation**: Core functionality continues when ancillary services fail

See [Resilience Patterns](references/resilience-patterns.md) for detailed test scenarios and code examples.

---

## Tools and Implementation

### Tool Selection

Choose tools based on your environment:

- **Kubernetes**: Chaos Mesh, Litmus
- **Cloud (AWS/Azure/GCP)**: Native fault injection services (AWS FIS, Azure Chaos Studio)
- **Containers**: Pumba, Toxiproxy
- **Infrastructure**: Chaos Monkey, Chaos Kong
- **Custom**: Chaos Toolkit for scripted experiments

See [Tools and Implementation](references/tools-and-implementation.md) for detailed tool comparisons, selection criteria, and code examples.

### Implementation Patterns

Chaos tests should be:
- **Automated**: Run as part of your test suite
- **Repeatable**: Same experiment produces consistent results
- **Observable**: Comprehensive metrics and logging
- **Safe**: Proper blast radius controls and abort mechanisms

Example chaos test structure:
```python
def test_database_failover():
    # 1. Establish steady state
    baseline = measure_steady_state()

    # 2. Inject failure
    with chaos_injection():
        results = measure_behavior()

    # 3. Assert graceful degradation
    assert results.meets_expectations(baseline)
```

---

## Safety First

### Blast Radius Control

Always follow the environment progression:
**Development → Staging → Canary → Production**

Start with aggressive chaos in dev, graduate to minimal controlled chaos in production.

### Production Safeguards

When running chaos in production:
- Affect only 1-5% of traffic initially
- Time-bound experiments (auto-stop after N minutes)
- Automatic abort if error rate exceeds threshold
- Kill switch readily accessible
- Run one experiment at a time
- Run during business hours (staff available)

### Abort Criteria

Immediately abort if:
- Error rate exceeds acceptable threshold
- Customer-facing impact detected
- Experiment scope unexpectedly expands
- Recovery doesn't occur as expected
- Monitoring indicates cascading failures

See [Safety and Best Practices](references/safety-and-best-practices.md) for comprehensive safety checklists and emergency response plans.

---

## Workflow

When working on chaos engineering tasks:

1. **Read Codebase Context**
   - Use Read, Glob, Grep to understand system architecture
   - Identify critical paths and dependencies
   - Locate existing resilience patterns (circuit breakers, retries, timeouts)

2. **Assess Current State**
   - Review existing failure handling
   - Identify gaps in resilience
   - Determine chaos maturity level

3. **Design Experiments**
   - Create hypothesis-driven experiments
   - Define steady-state metrics
   - Specify failure injection method
   - Plan blast radius controls

4. **Implement Tests**
   - Write automated chaos tests
   - Set up observability and monitoring
   - Implement safety mechanisms (abort, rollback)

5. **Document and Share**
   - Create experiment documentation
   - Share findings with team
   - Update runbooks based on learnings
   - Create action items for improvements

---

## Question Strategy

When designing chaos experiments, ask:

### For Experiment Design
1. "What is the critical user journey we're protecting?"
2. "What dependencies could fail and how?"
3. "What does graceful degradation look like for this system?"
4. "What is the maximum acceptable blast radius?"
5. "How will we know if the experiment succeeded or failed?"

### For Implementation
1. "What tools are available in this environment?"
2. "What metrics do we need to monitor?"
3. "What are the abort criteria and kill switch mechanisms?"
4. "What is the rollback procedure if things go wrong?"

### For Safety
1. "Is this a safe time to run this experiment?"
2. "Are the safeguards properly configured?"
3. "Who needs to be notified about this experiment?"
4. "What is the blast radius if this goes wrong?"

---

## Best Practices

1. **Start Small**: Begin with minimal blast radius in lower environments, expand gradually
2. **Hypothesis First**: Always have a clear, testable hypothesis before experimenting
3. **Automate Safeguards**: Never rely on manual kill switches alone - implement automatic abort
4. **Blameless Culture**: Focus on system improvement, not blame for failures
5. **Document Everything**: Capture learnings, findings, and action items for future reference
6. **Share Widely**: Chaos findings benefit the whole organization - communicate results
7. **Continuous Chaos**: Regular experiments catch regressions as systems evolve
8. **Production Reality**: Eventually test in production (safely) - it's the only true test

---

## Output Guidelines

When completing chaos engineering tasks, provide:

### For Experiment Design Tasks
- Clear hypothesis statement
- Steady-state metrics definition
- Detailed failure injection method
- Blast radius controls and safeguards
- Observability requirements
- Expected outcomes

### For Implementation Tasks
- Automated test code with proper structure
- Safety mechanisms (abort, rollback)
- Monitoring and alerting setup
- Documentation for running the experiment

### For Analysis Tasks
- Experiment results (hypothesis confirmed/disproved)
- Observed behaviors and metrics
- Unexpected findings
- Action items for system improvements
- Recommendations for next experiments

---

## References

Detailed guidance available in:
- [Principles and Maturity Model](references/principles-and-maturity.md) - Core concepts and maturity progression
- [Failure Injection Types](references/failure-injection-types.md) - Comprehensive failure catalogs
- [Experiment Design](references/experiment-design.md) - Templates and examples
- [Tools and Implementation](references/tools-and-implementation.md) - Tool selection and code examples
- [Resilience Patterns](references/resilience-patterns.md) - Patterns to validate with tests
- [Safety and Best Practices](references/safety-and-best-practices.md) - Safety checklists and procedures

External resources:
- "Chaos Engineering" by Casey Rosenthal & Nora Jones
- Principles of Chaos Engineering (principlesofchaos.org)
- AWS Well-Architected Framework - Reliability Pillar
