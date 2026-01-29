# Performance Testing Tools

This reference covers popular performance testing tools, their strengths, and practical examples.

## Tool Comparison Matrix

```
┌─────────────────────────────────────────────────────────────────────┐
│                    PERFORMANCE TESTING TOOLS                         │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  LOAD GENERATION TOOLS                                              │
│  ┌─────────────────┬────────────────────────────────────────────┐   │
│  │ Tool            │ Best For                                   │   │
│  ├─────────────────┼────────────────────────────────────────────┤   │
│  │ k6              │ Developer-friendly, JavaScript scripting   │   │
│  │ Gatling         │ High-performance, Scala-based, CI-friendly │   │
│  │ JMeter          │ GUI-based, extensive protocol support      │   │
│  │ Locust          │ Python scripting, distributed testing      │   │
│  │ Artillery       │ YAML config, easy to start                 │   │
│  │ wrk/wrk2        │ Simple HTTP benchmarking                   │   │
│  │ hey             │ Simple HTTP load testing                   │   │
│  │ vegeta          │ Constant rate load testing                 │   │
│  └─────────────────┴────────────────────────────────────────────┘   │
│                                                                     │
│  APM & PROFILING TOOLS                                              │
│  ┌─────────────────┬────────────────────────────────────────────┐   │
│  │ Tool            │ Best For                                   │   │
│  ├─────────────────┼────────────────────────────────────────────┤   │
│  │ Datadog APM     │ Full-stack observability, traces           │   │
│  │ New Relic       │ Application performance monitoring         │   │
│  │ Jaeger          │ Distributed tracing (open source)          │   │
│  │ Zipkin          │ Distributed tracing (open source)          │   │
│  │ py-spy          │ Python profiling                           │   │
│  │ async-profiler  │ JVM profiling                              │   │
│  │ perf            │ Linux system profiling                     │   │
│  │ Chrome DevTools │ Frontend performance                       │   │
│  └─────────────────┴────────────────────────────────────────────┘   │
│                                                                     │
│  MONITORING DURING TESTS                                            │
│  ┌─────────────────┬────────────────────────────────────────────┐   │
│  │ Tool            │ Best For                                   │   │
│  ├─────────────────┼────────────────────────────────────────────┤   │
│  │ Grafana         │ Dashboards, visualization                  │   │
│  │ Prometheus      │ Metrics collection                         │   │
│  │ InfluxDB        │ Time-series data                           │   │
│  │ Telegraf        │ System metrics collection                  │   │
│  │ htop/top        │ Real-time resource monitoring              │   │
│  │ iotop           │ Disk I/O monitoring                        │   │
│  │ nethogs         │ Network bandwidth per process              │   │
│  └─────────────────┴────────────────────────────────────────────┘   │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

## Load Generation Tools

### k6

**Strengths**:
- Modern, developer-friendly JavaScript API
- Built-in metrics and thresholds
- Excellent for CI/CD integration
- Good documentation and community
- Cloud option available (k6 Cloud)

**Use Cases**:
- API load testing
- Microservices testing
- CI/CD performance gates
- SLA validation

**Example Script**:

```javascript
// k6 Load Test Example
import http from 'k6/http';
import { check, sleep } from 'k6';

export const options = {
  stages: [
    { duration: '2m', target: 100 },   // Ramp up to 100 users
    { duration: '5m', target: 100 },   // Stay at 100 users
    { duration: '2m', target: 200 },   // Ramp up to 200 users
    { duration: '5m', target: 200 },   // Stay at 200 users
    { duration: '2m', target: 0 },     // Ramp down to 0
  ],
  thresholds: {
    http_req_duration: ['p(95)<200', 'p(99)<500'],  // SLA thresholds
    http_req_failed: ['rate<0.01'],                  // Error rate < 1%
  },
};

export default function () {
  const res = http.get('https://api.example.com/orders');

  check(res, {
    'status is 200': (r) => r.status === 200,
    'response time < 200ms': (r) => r.timings.duration < 200,
  });

  sleep(1);  // Think time between requests
}
```

**Running**:
```bash
k6 run script.js
```

### Locust

**Strengths**:
- Python-based scripting (flexible and powerful)
- Web UI for monitoring
- Distributed load generation
- Easy to extend

**Use Cases**:
- Complex user scenarios
- Python ecosystem integration
- Custom protocols
- Distributed load testing

**Example Script**:

```python
# Locust Load Test Example
from locust import HttpUser, task, between

class OrderUser(HttpUser):
    wait_time = between(1, 3)  # Think time 1-3 seconds

    @task(3)  # Weight: 3x more likely than other tasks
    def view_orders(self):
        self.client.get("/api/orders")

    @task(1)
    def create_order(self):
        self.client.post("/api/orders", json={
            "product_id": "123",
            "quantity": 1
        })

    def on_start(self):
        # Login on user start
        self.client.post("/api/login", json={
            "username": "testuser",
            "password": "testpass"
        })
```

**Running**:
```bash
# Web UI mode
locust -f locustfile.py --host=https://api.example.com

# Headless mode
locust -f locustfile.py --host=https://api.example.com \
  --users 100 --spawn-rate 10 --run-time 10m --headless
```

### Gatling

**Strengths**:
- High performance (Scala/Akka based)
- Beautiful HTML reports
- Excellent for high-throughput scenarios
- Strong community and enterprise support

**Use Cases**:
- High-load scenarios (10k+ RPS)
- Enterprise performance testing
- Complex simulation scenarios
- CI/CD integration

**Example Script** (Scala):

```scala
import io.gatling.core.Predef._
import io.gatling.http.Predef._
import scala.concurrent.duration._

class OrderSimulation extends Simulation {

  val httpProtocol = http
    .baseUrl("https://api.example.com")
    .acceptHeader("application/json")

  val scn = scenario("Order API Load")
    .exec(http("Get Orders")
      .get("/api/orders")
      .check(status.is(200))
      .check(responseTimeInMillis.lte(200)))
    .pause(1)
    .exec(http("Create Order")
      .post("/api/orders")
      .body(StringBody("""{"product_id":"123","quantity":1}"""))
      .check(status.is(201)))
    .pause(2)

  setUp(
    scn.inject(
      rampUsers(100) during (2 minutes),
      constantUsersPerSec(50) during (5 minutes)
    )
  ).protocols(httpProtocol)
   .assertions(
     global.responseTime.percentile3.lte(500),
     global.successfulRequests.percent.gte(99)
   )
}
```

### JMeter

**Strengths**:
- Mature and feature-rich
- GUI for test creation
- Extensive protocol support (HTTP, FTP, JDBC, SOAP, LDAP, etc.)
- Large plugin ecosystem

**Use Cases**:
- Multi-protocol testing
- Teams preferring GUI tools
- Complex enterprise scenarios
- Recording browser interactions

**When to Use**:
- Non-developer testers
- Need GUI test builder
- Testing non-HTTP protocols
- Legacy systems

**Limitations**:
- Resource-intensive
- Can be complex to configure
- GUI can be overwhelming

### Artillery

**Strengths**:
- YAML-based configuration (no coding required)
- Quick to get started
- Modern and well-designed
- Good for continuous testing

**Use Cases**:
- Quick API tests
- CI/CD integration
- Teams without coding experience
- Synthetic monitoring

**Example Config**:

```yaml
config:
  target: "https://api.example.com"
  phases:
    - duration: 60
      arrivalRate: 10
      name: "Warm up"
    - duration: 300
      arrivalRate: 50
      name: "Sustained load"

scenarios:
  - name: "Order workflow"
    flow:
      - get:
          url: "/api/products"
      - think: 2
      - post:
          url: "/api/orders"
          json:
            product_id: "123"
            quantity: 1
      - think: 1
      - get:
          url: "/api/orders"
```

### wrk/wrk2

**Strengths**:
- Extremely lightweight
- High performance C implementation
- Lua scripting for customization
- Precise constant throughput (wrk2)

**Use Cases**:
- Simple HTTP benchmarking
- Microbenchmarks
- Constant-rate load testing (wrk2)
- Low-overhead testing

**Example Usage**:

```bash
# Basic test
wrk -t 12 -c 400 -d 30s https://api.example.com/orders

# With Lua script for POST
wrk -t 12 -c 400 -d 30s -s post.lua https://api.example.com/orders
```

**Lua Script** (post.lua):
```lua
wrk.method = "POST"
wrk.body   = '{"product_id":"123","quantity":1}'
wrk.headers["Content-Type"] = "application/json"
```

**wrk2 for constant rate**:
```bash
# Constant 1000 RPS for 30 seconds
wrk2 -t 12 -c 400 -d 30s -R 1000 https://api.example.com/orders
```

## APM and Profiling Tools

### Datadog APM

**Strengths**:
- Full-stack observability
- Distributed tracing
- Automatic instrumentation
- Excellent dashboards

**Use Cases**:
- Production monitoring
- Performance investigation
- Distributed system tracing
- Real-time alerting

### Jaeger / Zipkin

**Strengths**:
- Open-source distributed tracing
- Visualize request flows
- Identify latency sources
- Service dependency mapping

**Use Cases**:
- Microservices tracing
- Latency analysis
- Debugging distributed systems
- Understanding service dependencies

### Language-Specific Profilers

**Python: py-spy**
```bash
# Profile running process
py-spy top --pid <PID>

# Generate flame graph
py-spy record -o profile.svg --pid <PID>
```

**Java: async-profiler**
```bash
# Profile Java process
./profiler.sh -d 30 -f flamegraph.html <PID>
```

**Linux: perf**
```bash
# Profile CPU usage
perf record -F 99 -p <PID> -g -- sleep 30
perf report
```

## Monitoring Stack

### Prometheus + Grafana

**Setup for Performance Testing**:

1. **Prometheus**: Collect metrics from application
2. **Grafana**: Visualize metrics during tests
3. **Exporters**: Node exporter, JMX exporter, etc.

**Key Dashboards**:
- Response time percentiles
- Request rate
- Error rate
- Resource utilization (CPU, memory, disk, network)
- JVM metrics (GC, heap, threads)
- Database connections

### Real-Time Monitoring

**htop**: CPU and memory by process
```bash
htop
```

**iotop**: Disk I/O by process
```bash
sudo iotop
```

**nethogs**: Network usage by process
```bash
sudo nethogs
```

## Tool Selection Guide

| Requirement | Recommended Tool |
|-------------|------------------|
| Developer-friendly scripting | k6, Locust |
| High throughput (10k+ RPS) | Gatling, wrk2 |
| GUI test creation | JMeter |
| Quick YAML-based tests | Artillery |
| Simple HTTP benchmark | wrk, hey |
| Python ecosystem | Locust |
| Enterprise support | Gatling, JMeter |
| CI/CD integration | k6, Gatling, Artillery |
| Distributed load | Locust, Gatling |
| Multi-protocol | JMeter |

## Cloud Load Testing Services

### Cloud Options

- **k6 Cloud**: Managed k6 with global load zones
- **BlazeMeter**: Cloud JMeter/Gatling execution
- **Artillery Cloud**: Managed Artillery
- **AWS Load Testing**: CloudWatch Synthetics
- **Azure Load Testing**: Managed JMeter

**When to Use Cloud Services**:
- Need load from multiple geographic regions
- Don't want to manage infrastructure
- Testing from outside your network
- Simulating distributed users
