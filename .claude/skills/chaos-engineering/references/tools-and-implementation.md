# Chaos Engineering Tools and Implementation

## Tool Categories

```
┌─────────────────────────────────────────────────────────────────────┐
│                    CHAOS ENGINEERING TOOLS                          │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  INFRASTRUCTURE CHAOS                                               │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │ Tool              │ Capabilities                              │  │
│  ├───────────────────┼───────────────────────────────────────────┤  │
│  │ Chaos Monkey      │ Random instance termination               │  │
│  │ Chaos Kong        │ Entire region/zone failures               │  │
│  │ Litmus            │ Kubernetes-native chaos experiments       │  │
│  │ Chaos Mesh        │ K8s pod/network/I/O fault injection       │  │
│  │ AWS FIS           │ AWS-native fault injection service        │  │
│  │ Azure Chaos       │ Azure-native chaos experiments            │  │
│  │ GCP Chaos         │ Fault injection for GCP resources         │  │
│  └───────────────────┴───────────────────────────────────────────┘  │
│                                                                     │
│  APPLICATION CHAOS                                                  │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │ Tool              │ Capabilities                              │  │
│  ├───────────────────┼───────────────────────────────────────────┤  │
│  │ Gremlin           │ Full-featured SaaS chaos platform         │  │
│  │ Pumba             │ Docker container chaos                    │  │
│  │ Toxiproxy         │ TCP proxy for simulating network issues   │  │
│  │ Chaos Toolkit     │ Open-source chaos experiment framework    │  │
│  │ Steadybit         │ Agent-based chaos engineering             │  │
│  └───────────────────┴───────────────────────────────────────────┘  │
│                                                                     │
│  NETWORK CHAOS                                                      │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │ Tool              │ Capabilities                              │  │
│  ├───────────────────┼───────────────────────────────────────────┤  │
│  │ tc (traffic ctrl) │ Linux kernel traffic shaping              │  │
│  │ iptables          │ Packet filtering and manipulation         │  │
│  │ Comcast           │ Network degradation simulation            │  │
│  │ Muxy              │ Network/protocol mocking proxy            │  │
│  └───────────────────┴───────────────────────────────────────────┘  │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

## Tool Selection Guide

### Chaos Monkey
- **Best for**: AWS/cloud infrastructure instance termination
- **Use when**: Testing instance-level resilience and auto-scaling
- **Maturity**: Production-ready, battle-tested at Netflix

### Chaos Mesh
- **Best for**: Kubernetes environments
- **Use when**: Testing pod failures, network issues, and I/O faults in K8s
- **Maturity**: CNCF project, production-ready

### Litmus
- **Best for**: Kubernetes chaos with built-in experiment catalog
- **Use when**: Need predefined chaos scenarios for K8s workloads
- **Maturity**: CNCF project, extensive experiment library

### Gremlin
- **Best for**: Enterprise chaos with comprehensive features
- **Use when**: Need SaaS platform with support and extensive capabilities
- **Maturity**: Commercial, production-ready with enterprise support

### Toxiproxy
- **Best for**: Network condition simulation
- **Use when**: Testing latency, packet loss, and connection issues
- **Maturity**: Lightweight, stable, easy to integrate

### AWS Fault Injection Simulator (FIS)
- **Best for**: Native AWS resource chaos
- **Use when**: Testing AWS services and infrastructure
- **Maturity**: AWS-managed service, production-ready

### Chaos Toolkit
- **Best for**: Custom, scripted chaos experiments
- **Use when**: Need flexibility and extensibility
- **Maturity**: Open-source, modular, good for custom scenarios

## Implementation Examples

### Python Chaos Test Implementation

```python
# Example: Chaos test with retry verification
class ChaosTest:
    """
    Test that order service handles database failures gracefully
    """

    def test_database_failure_triggers_circuit_breaker(self):
        # Arrange: Set up steady state
        baseline_latency = measure_p99_latency()
        baseline_error_rate = measure_error_rate()

        # Act: Inject database failure
        with DatabaseChaos() as chaos:
            chaos.add_latency(milliseconds=5000)  # 5s delay

            # Generate traffic during failure
            results = generate_load(
                requests_per_second=100,
                duration_seconds=60
            )

        # Assert: Verify graceful degradation
        assert results.circuit_breaker_opened == True
        assert results.fallback_responses > 0
        assert results.error_rate < 0.05  # <5% errors
        assert results.p99_latency < 1000  # <1s (circuit breaker fast-fails)


# Example: Network partition test
def test_network_partition_handling():
    """
    Verify system handles network partition between services
    """
    with NetworkChaos(target="payment-service") as chaos:
        # Simulate network partition
        chaos.partition_from("order-service")

        # Attempt order that requires payment
        result = create_order(requires_payment=True)

        # Should fail gracefully, not hang
        assert result.status == "PENDING_PAYMENT"
        assert result.error_message == "Payment service temporarily unavailable"
        assert result.response_time < 5000  # Timeout, not hung
```

### Using Toxiproxy for Network Chaos

```python
from toxiproxy import Toxiproxy

# Create proxy to database
toxiproxy = Toxiproxy()
proxy = toxiproxy.create(
    name="database",
    upstream="database:5432",
    listen="0.0.0.0:15432"
)

# Add latency toxic
proxy.add_toxic(
    name="latency",
    type="latency",
    attributes={
        "latency": 1000,  # 1 second delay
        "jitter": 100      # ±100ms jitter
    }
)

# Run tests through proxy
# ...

# Clean up
proxy.destroy()
```

### Using Chaos Mesh in Kubernetes

```yaml
# Network delay experiment
apiVersion: chaos-mesh.org/v1alpha1
kind: NetworkChaos
metadata:
  name: network-delay
spec:
  action: delay
  mode: one
  selector:
    namespaces:
      - production
    labelSelectors:
      app: order-service
  delay:
    latency: "100ms"
    correlation: "100"
    jitter: "0ms"
  duration: "5m"
```

### Using AWS FIS

```json
{
  "description": "Stop EC2 instances for chaos testing",
  "targets": {
    "Instances": {
      "resourceType": "aws:ec2:instance",
      "resourceTags": {
        "env": "staging"
      },
      "selectionMode": "COUNT(1)"
    }
  },
  "actions": {
    "StopInstances": {
      "actionId": "aws:ec2:stop-instances",
      "parameters": {
        "startInstancesAfterDuration": "PT5M"
      },
      "targets": {
        "Instances": "Instances"
      }
    }
  },
  "stopConditions": [
    {
      "source": "aws:cloudwatch:alarm",
      "value": "arn:aws:cloudwatch:us-west-2:123456789012:alarm:HighErrorRate"
    }
  ]
}
```

## Building Custom Chaos Infrastructure

For scenarios not covered by existing tools, you can build custom chaos injection:

### Custom Failure Injection Service

```python
class ChaosService:
    """
    Custom service for controlled failure injection
    """

    def __init__(self):
        self.active_experiments = {}
        self.safety_limits = SafetyLimits()

    def inject_latency(self, service_name, duration_ms, duration_seconds):
        """Add artificial latency to service"""
        if not self.safety_limits.allows_experiment():
            raise ChaosAborted("Safety limits exceeded")

        experiment = LatencyExperiment(
            service=service_name,
            latency_ms=duration_ms,
            duration=duration_seconds
        )

        self.active_experiments[experiment.id] = experiment
        experiment.start()

        return experiment.id

    def abort_experiment(self, experiment_id):
        """Immediately stop chaos experiment"""
        if experiment_id in self.active_experiments:
            self.active_experiments[experiment_id].stop()
```

## Integration with CI/CD

Chaos experiments should be part of your continuous deployment pipeline:

```yaml
# GitHub Actions example
name: Chaos Tests
on:
  schedule:
    - cron: '0 */6 * * *'  # Every 6 hours

jobs:
  chaos-test:
    runs-on: ubuntu-latest
    steps:
      - name: Checkout code
        uses: actions/checkout@v2

      - name: Deploy to staging
        run: ./deploy-staging.sh

      - name: Run chaos experiments
        run: |
          pytest tests/chaos/ \
            --environment=staging \
            --max-blast-radius=10

      - name: Report results
        if: failure()
        run: ./notify-slack.sh "Chaos tests failed"
```
