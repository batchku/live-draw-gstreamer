# Failure Injection Types

## Infrastructure Failures

```
┌─────────────────────────────────────────────────────────────────────┐
│                    INFRASTRUCTURE FAILURES                          │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  1. Server/Instance Failures                                        │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │ Type           │ Method                │ Validates            │  │
│  ├────────────────┼───────────────────────┼──────────────────────┤  │
│  │ Kill process   │ kill -9 <pid>         │ Process recovery     │  │
│  │ Terminate VM   │ Cloud API terminate   │ Instance replacement │  │
│  │ CPU stress     │ stress --cpu 4        │ Degraded performance │  │
│  │ Memory exhaust │ stress --vm 1 --vm-b  │ OOM handling         │  │
│  │ Disk fill      │ dd if=/dev/zero       │ Disk full handling   │  │
│  └────────────────┴───────────────────────┴──────────────────────┘  │
│                                                                     │
│  2. Network Failures                                                │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │ Type           │ Method                │ Validates            │  │
│  ├────────────────┼───────────────────────┼──────────────────────┤  │
│  │ Latency        │ tc qdisc add delay    │ Timeout handling     │  │
│  │ Packet loss    │ tc qdisc add loss     │ Retry logic          │  │
│  │ Partition      │ iptables DROP         │ Split-brain handling │  │
│  │ DNS failure    │ Block DNS resolution  │ DNS caching/fallback │  │
│  │ Bandwidth      │ tc qdisc add rate     │ Throughput limits    │  │
│  └────────────────┴───────────────────────┴──────────────────────┘  │
│                                                                     │
│  3. Storage Failures                                                │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │ Type           │ Method                │ Validates            │  │
│  ├────────────────┼───────────────────────┼──────────────────────┤  │
│  │ Disk failure   │ Detach volume         │ Data redundancy      │  │
│  │ Slow I/O       │ fio with delays       │ I/O timeout handling │  │
│  │ Corrupted data │ Flip bits in files    │ Data validation      │  │
│  │ Full disk      │ Fill filesystem       │ Graceful degradation │  │
│  └────────────────┴───────────────────────┴──────────────────────┘  │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

## Application Failures

```
┌──────────────────────────────────────────────────────────────────────┐
│                    APPLICATION FAILURES                              │
├──────────────────────────────────────────────────────────────────────┤
│                                                                      │
│  1. Service Failures                                                 │
│  ┌────────────────────────────────────────────────────────────────┐  │
│  │ Type              │ Injection Method       │ Tests             │  │
│  ├───────────────────┼────────────────────────┼───────────────────┤  │
│  │ Service crash     │ Kill container/process │ Failover          │  │
│  │ Slow responses    │ Add artificial delay   │ Timeout handling  │  │
│  │ Error responses   │ Return 500 errors      │ Error handling    │  │
│  │ Partial failure   │ Fail some requests     │ Retry logic       │  │
│  │ Memory leak       │ Allocate unbounded     │ Resource limits   │  │
│  └───────────────────┴────────────────────────┴───────────────────┘  │
│                                                                      │
│  2. Dependency Failures                                              │
│  ┌────────────────────────────────────────────────────────────────┐  │
│  │ Dependency        │ Failure Mode           │ Expected Behavior │  │
│  ├───────────────────┼────────────────────────┼───────────────────┤  │
│  │ Database          │ Connection refused     │ Circuit breaker   │  │
│  │ Cache (Redis)     │ Timeout                │ Cache miss path   │  │
│  │ Message queue     │ Unavailable            │ Retry/dead letter │  │
│  │ External API      │ 503 Service Unavail.   │ Fallback behavior │  │
│  │ Auth service      │ Slow response          │ Token caching     │  │
│  └───────────────────┴────────────────────────┴───────────────────┘  │
│                                                                      │
│  3. Data Failures                                                    │
│  ┌────────────────────────────────────────────────────────────────┐  │
│  │ Type              │ Injection              │ Tests             │  │
│  ├───────────────────┼────────────────────────┼───────────────────┤  │
│  │ Invalid data      │ Inject malformed data  │ Validation logic  │  │
│  │ Missing data      │ Delete records         │ Null handling     │  │
│  │ Stale data        │ Return old cached data │ Freshness checks  │  │
│  │ Duplicate events  │ Replay messages        │ Idempotency       │  │
│  └───────────────────┴────────────────────────┴───────────────────┘  │
│                                                                      │
└──────────────────────────────────────────────────────────────────────┘
```

## Distributed System Failures

```
┌─────────────────────────────────────────────────────────────────────┐
│                    DISTRIBUTED SYSTEM FAILURES                      │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  1. Consensus Failures                                              │
│     • Leader election during partition                              │
│     • Split-brain scenarios                                         │
│     • Quorum loss                                                   │
│     • Clock skew between nodes                                      │
│                                                                     │
│  2. Replication Failures                                            │
│     • Replica lag                                                   │
│     • Replication failure                                           │
│     • Inconsistent replicas                                         │
│     • Failover during write                                         │
│                                                                     │
│  3. Load Balancer Failures                                          │
│     • LB failover                                                   │
│     • Health check failures                                         │
│     • Uneven load distribution                                      │
│     • Session stickiness issues                                     │
│                                                                     │
│  4. Cross-Region Failures                                           │
│     • Region outage                                                 │
│     • Cross-region latency spike                                    │
│     • DNS failover                                                  │
│     • Data synchronization lag                                      │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

## Choosing Failure Types

When selecting which failures to inject, consider:

1. **Real-world likelihood**: Focus on failures that actually happen in production
2. **Impact severity**: Test failures that would cause the most user pain
3. **System dependencies**: Inject failures in critical path dependencies
4. **Recovery mechanisms**: Test all your resilience patterns
5. **Cascading potential**: Test failures that could trigger cascades
