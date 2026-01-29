# Bottleneck Identification and Resolution

This reference covers common performance bottlenecks, how to identify them, and typical solutions.

## Common Bottleneck Patterns

```
┌─────────────────────────────────────────────────────────────────────┐
│                    COMMON BOTTLENECK PATTERNS                        │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  1. DATABASE BOTTLENECK                                             │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │ Symptoms:                                                     │  │
│  │ • High query latency under load                               │  │
│  │ • Connection pool exhaustion                                  │  │
│  │ • Lock contention (deadlocks)                                 │  │
│  │ • High CPU on database server                                 │  │
│  │                                                               │  │
│  │ Investigation:                                                │  │
│  │ • Check slow query logs                                       │  │
│  │ • Analyze query execution plans                               │  │
│  │ • Monitor connection pool utilization                         │  │
│  │ • Check for missing indexes                                   │  │
│  │                                                               │  │
│  │ Typical Fixes:                                                │  │
│  │ • Add indexes                                                 │  │
│  │ • Optimize queries                                            │  │
│  │ • Add read replicas                                           │  │
│  │ • Implement caching                                           │  │
│  │ • Connection pool tuning                                      │  │
│  └───────────────────────────────────────────────────────────────┘  │
│                                                                     │
│  2. CPU BOTTLENECK                                                  │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │ Symptoms:                                                     │  │
│  │ • CPU utilization > 80%                                       │  │
│  │ • Response time degrades linearly with load                   │  │
│  │ • All cores saturated                                         │  │
│  │                                                               │  │
│  │ Investigation:                                                │  │
│  │ • Profile CPU usage per function                              │  │
│  │ • Check for tight loops                                       │  │
│  │ • Look for unnecessary computation                            │  │
│  │                                                               │  │
│  │ Typical Fixes:                                                │  │
│  │ • Optimize hot code paths                                     │  │
│  │ • Add caching for computed values                             │  │
│  │ • Horizontal scaling (more instances)                         │  │
│  │ • Async processing for heavy computation                      │  │
│  └───────────────────────────────────────────────────────────────┘  │
│                                                                     │
│  3. MEMORY BOTTLENECK                                               │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │ Symptoms:                                                     │  │
│  │ • High memory usage                                           │  │
│  │ • Frequent garbage collection                                 │  │
│  │ • OOM errors                                                  │  │
│  │ • Memory usage grows over time                                │  │
│  │                                                               │  │
│  │ Investigation:                                                │  │
│  │ • Heap dump analysis                                          │  │
│  │ • GC log analysis                                             │  │
│  │ • Memory profiling                                            │  │
│  │                                                               │  │
│  │ Typical Fixes:                                                │  │
│  │ • Fix memory leaks                                            │  │
│  │ • Optimize data structures                                    │  │
│  │ • Tune GC settings                                            │  │
│  │ • Stream large datasets instead of loading all                │  │
│  └───────────────────────────────────────────────────────────────┘  │
│                                                                     │
│  4. NETWORK BOTTLENECK                                              │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │ Symptoms:                                                     │  │
│  │ • High network latency                                        │  │
│  │ • Bandwidth saturation                                        │  │
│  │ • Packet loss under load                                      │  │
│  │ • Timeouts on remote calls                                    │  │
│  │                                                               │  │
│  │ Investigation:                                                │  │
│  │ • Network bandwidth monitoring                                │  │
│  │ • Packet analysis                                             │  │
│  │ • Check payload sizes                                         │  │
│  │                                                               │  │
│  │ Typical Fixes:                                                │  │
│  │ • Compress payloads                                           │  │
│  │ • Reduce chatty calls (batch requests)                        │  │
│  │ • CDN for static assets                                       │  │
│  │ • Connection pooling                                          │  │
│  └───────────────────────────────────────────────────────────────┘  │
│                                                                     │
│  5. THREAD/CONCURRENCY BOTTLENECK                                   │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │ Symptoms:                                                     │  │
│  │ • Thread pool exhaustion                                      │  │
│  │ • Request queuing                                             │  │
│  │ • Low CPU but high latency                                    │  │
│  │ • Contention on shared resources                              │  │
│  │                                                               │  │
│  │ Investigation:                                                │  │
│  │ • Thread dump analysis                                        │  │
│  │ • Monitor queue depths                                        │  │
│  │ • Check for lock contention                                   │  │
│  │                                                               │  │
│  │ Typical Fixes:                                                │  │
│  │ • Tune thread pool sizes                                      │  │
│  │ • Use async I/O                                               │  │
│  │ • Reduce lock granularity                                     │  │
│  │ • Non-blocking algorithms                                     │  │
│  └───────────────────────────────────────────────────────────────┘  │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

## Database Bottlenecks

### Symptom: Slow Queries Under Load

**Investigation Steps**:

1. **Enable Slow Query Log**
```sql
-- MySQL
SET GLOBAL slow_query_log = 'ON';
SET GLOBAL long_query_time = 1;  -- Log queries > 1 second
```

2. **Analyze Query Plans**
```sql
EXPLAIN SELECT * FROM orders WHERE user_id = 123;
```

3. **Check for Missing Indexes**
Look for:
- Full table scans in EXPLAIN output
- WHERE clauses on non-indexed columns
- JOIN conditions without indexes

**Common Solutions**:

**Add Indexes**:
```sql
-- Before: Full table scan
SELECT * FROM orders WHERE user_id = 123;

-- Add index
CREATE INDEX idx_orders_user_id ON orders(user_id);

-- After: Index lookup
```

**Optimize Queries**:
```sql
-- Before: N+1 query problem
for user in users:
    orders = db.query("SELECT * FROM orders WHERE user_id = ?", user.id)

-- After: Single query with JOIN
orders = db.query("""
    SELECT u.*, o.*
    FROM users u
    LEFT JOIN orders o ON u.id = o.user_id
""")
```

**Add Caching**:
```python
# Cache frequently accessed data
@cache.memoize(timeout=300)
def get_product(product_id):
    return db.query("SELECT * FROM products WHERE id = ?", product_id)
```

### Symptom: Connection Pool Exhaustion

**Investigation**:
```python
# Monitor pool metrics
print(f"Active: {pool.active_connections}")
print(f"Idle: {pool.idle_connections}")
print(f"Total: {pool.total_connections}")
print(f"Waiting: {pool.waiting_requests}")
```

**Solutions**:

1. **Increase Pool Size** (if database can handle it):
```python
# Before
pool = ConnectionPool(max_connections=10)

# After
pool = ConnectionPool(max_connections=50)
```

2. **Fix Connection Leaks**:
```python
# Before: Connection not released on error
conn = pool.get_connection()
result = conn.execute(query)
conn.close()  # Not called if execute() throws

# After: Use context manager
with pool.get_connection() as conn:
    result = conn.execute(query)
# Connection automatically returned to pool
```

3. **Reduce Connection Hold Time**:
```python
# Before: Long transaction
with db.transaction():
    data = fetch_from_api()  # Slow external call
    db.insert(data)

# After: Minimize transaction scope
data = fetch_from_api()  # Do this outside transaction
with db.transaction():
    db.insert(data)  # Quick insert only
```

### Symptom: Lock Contention

**Investigation**:
```sql
-- MySQL: Show current locks
SELECT * FROM information_schema.innodb_locks;
SELECT * FROM information_schema.innodb_lock_waits;

-- PostgreSQL: Show blocking queries
SELECT blocked_locks.pid AS blocked_pid,
       blocking_locks.pid AS blocking_pid,
       blocked_activity.query AS blocked_statement
FROM pg_locks blocked_locks
JOIN pg_locks blocking_locks ON blocking_locks.locktype = blocked_locks.locktype;
```

**Solutions**:

1. **Reduce Transaction Scope**:
```python
# Before: Large transaction
with db.transaction():
    for item in large_list:  # Holds lock too long
        db.update(item)

# After: Smaller transactions
for item in large_list:
    with db.transaction():
        db.update(item)
```

2. **Optimize Lock Order** (prevent deadlocks):
```python
# Before: Inconsistent lock order → deadlock risk
def transfer(from_id, to_id, amount):
    lock_account(from_id)
    lock_account(to_id)
    # ...

# After: Consistent lock order
def transfer(from_id, to_id, amount):
    first_id = min(from_id, to_id)
    second_id = max(from_id, to_id)
    lock_account(first_id)
    lock_account(second_id)
    # ...
```

## CPU Bottlenecks

### Symptom: High CPU Utilization

**Investigation with Profiling**:

**Python (py-spy)**:
```bash
# Find hot functions
py-spy top --pid <PID>

# Generate flame graph
py-spy record -o profile.svg --pid <PID> --duration 60
```

**Java (async-profiler)**:
```bash
./profiler.sh -d 60 -f flamegraph.html <PID>
```

**Node.js (--prof)**:
```bash
node --prof app.js
node --prof-process isolate-*.log > processed.txt
```

**Common Issues Found**:

### Issue: Inefficient Algorithms

```python
# Before: O(n²) - quadratic time
def find_duplicates(items):
    duplicates = []
    for i in range(len(items)):
        for j in range(i+1, len(items)):
            if items[i] == items[j]:
                duplicates.append(items[i])
    return duplicates

# After: O(n) - linear time
def find_duplicates(items):
    seen = set()
    duplicates = set()
    for item in items:
        if item in seen:
            duplicates.add(item)
        seen.add(item)
    return list(duplicates)
```

### Issue: Unnecessary Computation

```python
# Before: Recomputing on every request
def get_recommendations(user_id):
    all_users = db.get_all_users()  # Expensive
    similarity_matrix = compute_similarity(all_users)  # Very expensive
    return recommend(user_id, similarity_matrix)

# After: Precompute and cache
@cache.memoize(timeout=3600)
def get_similarity_matrix():
    all_users = db.get_all_users()
    return compute_similarity(all_users)

def get_recommendations(user_id):
    similarity_matrix = get_similarity_matrix()
    return recommend(user_id, similarity_matrix)
```

### Issue: Serialization Overhead

```python
# Before: JSON serialization in hot path
def process_request(data):
    json_str = json.dumps(data)  # Slow
    process(json_str)

# After: Pass objects directly
def process_request(data):
    process(data)  # No serialization needed
```

## Memory Bottlenecks

### Symptom: Growing Memory Usage

**Investigation**:

**Python Memory Profiling**:
```python
from memory_profiler import profile

@profile
def my_function():
    large_list = [i for i in range(1000000)]
    # ...
```

**Java Heap Dump**:
```bash
# Generate heap dump
jmap -dump:format=b,file=heap.hprof <PID>

# Analyze with Eclipse MAT or VisualVM
```

**Common Memory Leaks**:

### Issue: Unbounded Caches

```python
# Before: Cache grows forever
cache = {}
def get_user(user_id):
    if user_id not in cache:
        cache[user_id] = db.get_user(user_id)
    return cache[user_id]

# After: LRU cache with size limit
from functools import lru_cache

@lru_cache(maxsize=1000)
def get_user(user_id):
    return db.get_user(user_id)
```

### Issue: Loading Large Datasets

```python
# Before: Load all into memory
def process_orders():
    all_orders = db.query("SELECT * FROM orders")  # Could be millions
    for order in all_orders:
        process(order)

# After: Stream with cursor
def process_orders():
    cursor = db.cursor()
    cursor.execute("SELECT * FROM orders")
    for order in cursor.fetchmany(size=1000):  # Batch of 1000
        process(order)
```

### Issue: Event Handlers Not Released

```python
# Before: Memory leak in observer pattern
class Publisher:
    def __init__(self):
        self.subscribers = []

    def subscribe(self, callback):
        self.subscribers.append(callback)  # Never removed!

# After: Weak references
import weakref

class Publisher:
    def __init__(self):
        self.subscribers = []

    def subscribe(self, callback):
        self.subscribers.append(weakref.ref(callback))
```

## Network Bottlenecks

### Symptom: High Network Latency

**Investigation**:
```bash
# Monitor network bandwidth
iftop

# Check packet loss
ping <host>

# Network latency
traceroute <host>
```

**Common Issues**:

### Issue: N+1 API Calls

```python
# Before: N+1 problem
def get_orders_with_products():
    orders = fetch("/api/orders")  # 1 call
    for order in orders:
        order.product = fetch(f"/api/products/{order.product_id}")  # N calls
    return orders

# After: Batch API call
def get_orders_with_products():
    orders = fetch("/api/orders")
    product_ids = [order.product_id for order in orders]
    products = fetch(f"/api/products?ids={','.join(product_ids)}")  # 1 call
    # Map products to orders
    return orders
```

### Issue: Large Payloads

```python
# Before: Sending huge payloads
response = {
    "users": get_all_users(),  # 10MB of data
    "timestamp": time.time()
}

# After: Pagination + compression
@app.route("/api/users")
@gzip_compress
def get_users():
    page = request.args.get('page', 1)
    per_page = 100
    users = get_users_page(page, per_page)
    return jsonify(users)
```

## Thread/Concurrency Bottlenecks

### Symptom: Low CPU, High Latency

**This indicates waiting, not computation**

**Investigation**:
```bash
# Java thread dump
jstack <PID>

# Look for threads in WAITING or BLOCKED state
```

**Common Issues**:

### Issue: Thread Pool Too Small

```python
# Before: Small thread pool → requests queued
executor = ThreadPoolExecutor(max_workers=4)

# After: Sized for workload
import os
executor = ThreadPoolExecutor(max_workers=os.cpu_count() * 2)
```

### Issue: Blocking I/O

```python
# Before: Blocking I/O in thread pool
def handle_request():
    result = requests.get("http://api.example.com")  # Blocks thread
    return result

# After: Async I/O
async def handle_request():
    async with aiohttp.ClientSession() as session:
        async with session.get("http://api.example.com") as response:
            return await response.text()
```

## Bottleneck Investigation Workflow

```
┌─────────────────────────────────────────────────────────────────────┐
│            BOTTLENECK INVESTIGATION WORKFLOW                         │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  1. OBSERVE                                                         │
│     │                                                               │
│     ├─→ Check response time trends                                  │
│     ├─→ Monitor error rates                                         │
│     └─→ Review resource utilization (CPU, memory, I/O)              │
│                                                                     │
│  2. IDENTIFY                                                        │
│     │                                                               │
│     ├─→ Which resource is saturated first?                          │
│     ├─→ Which component shows degradation?                          │
│     └─→ What correlates with performance drop?                      │
│                                                                     │
│  3. ISOLATE                                                         │
│     │                                                               │
│     ├─→ Test components independently                               │
│     ├─→ Remove/mock dependencies                                    │
│     └─→ Profile specific code paths                                 │
│                                                                     │
│  4. MEASURE                                                         │
│     │                                                               │
│     ├─→ Establish baseline metrics                                  │
│     ├─→ Quantify impact of bottleneck                               │
│     └─→ Define improvement target                                   │
│                                                                     │
│  5. FIX                                                             │
│     │                                                               │
│     ├─→ Implement targeted fix                                      │
│     ├─→ Avoid premature optimization                                │
│     └─→ Fix highest impact issues first                             │
│                                                                     │
│  6. VERIFY                                                          │
│     │                                                               │
│     ├─→ Re-run load tests                                           │
│     ├─→ Compare against baseline                                    │
│     └─→ Confirm no regression                                       │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

## Quick Diagnosis Decision Tree

```
High Response Time?
├─ Yes → Check resource utilization
│  ├─ CPU > 80%? → CPU bottleneck (profile code)
│  ├─ Memory > 85%? → Memory bottleneck (heap dump)
│  ├─ DB connections exhausted? → Database bottleneck
│  ├─ Thread pool exhausted? → Concurrency bottleneck
│  └─ Network saturated? → Network bottleneck
│
└─ No → Check error rates
   ├─ Errors increasing? → Investigate error types
   └─ No errors? → May be capacity limit, run stress test
```
