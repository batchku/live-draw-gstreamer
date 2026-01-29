# DDD Tactical Design Patterns

Tactical design focuses on implementation patterns within a bounded context. These patterns help you build a rich domain model with well-encapsulated business logic.

## 1. Entity

**Definition**: An object defined by its identity rather than its attributes. Entities have continuity through time and across different states.

**Structure**:
```
┌─────────────────────────────────────────────────────────┐
│                        ENTITY                           │
│                 (Identity-based equality)               │
├─────────────────────────────────────────────────────────┤
│                                                         │
│  ┌───────────────────────────────────────────────────┐  │
│  │           IDENTITY (Immutable)                    │  │
│  │  ┌─────────────────────────────────────────────┐  │  │
│  │  │  id: OrderId (unique, never changes)        │  │  │
│  │  └─────────────────────────────────────────────┘  │  │
│  └───────────────────────────────────────────────────┘  │
│                                                         │
│  ┌───────────────────────────────────────────────────┐  │
│  │           ATTRIBUTES (Mutable)                    │  │
│  │  ┌─────────────────────────────────────────────┐  │  │
│  │  │  customerId: CustomerId                     │  │  │
│  │  │  items: OrderItem[]                         │  │  │
│  │  │  status: OrderStatus ──────┐                │  │  │
│  │  │  createdAt: DateTime       │  Can change    │  │  │
│  │  │  shippingAddress: Address  │  over time     │  │  │
│  │  │  total: Money ─────────────┘                │  │  │
│  │  └─────────────────────────────────────────────┘  │  │
│  └───────────────────────────────────────────────────┘  │
│                                                         │
│  ┌───────────────────────────────────────────────────┐  │
│  │           BEHAVIOR (Business logic)               │  │
│  │  ┌─────────────────────────────────────────────┐  │  │
│  │  │  + addItem(product, quantity, price)        │  │  │
│  │  │  + removeItem(itemId)                       │  │  │
│  │  │  + submit()                                 │  │  │
│  │  │  + cancel()                                 │  │  │
│  │  │  + calculateTotal(): Money                  │  │  │
│  │  └─────────────────────────────────────────────┘  │  │
│  └───────────────────────────────────────────────────┘  │
│                                                         │
│  Equality: Two orders are same if id is same            │
│            (regardless of attribute values)             │
│                                                         │
│  Order(id=123, total=$50) == Order(id=123, total=$100)  │
│  True - same identity!                                  │
│                                                         │
└─────────────────────────────────────────────────────────┘

    Order(id=123)           Order(id=456)
         │                       │
         │  Same identity?       │
         └───────────┬───────────┘
                     │
              ┌──────▼─────┐
              │  id == id? │
              └──────┬─────┘
                     │
            ┌────────┴────────┐
            │                 │
          Yes               No
       (Equal)          (Not equal)
```

**When to Use**:
- Object needs to be tracked over time
- Object changes state but remains "the same thing"
- Need to distinguish between instances with same attributes

**Characteristics**:
- Has unique identity (ID)
- Mutable
- Identity remains constant even as attributes change
- Implements equality based on identity, not attributes

**Lifecycle**:
```
┌──────────┐    ┌──────────┐    ┌──────────┐    ┌──────────┐
│ Created  │───►│ Modified │───►│ Modified │───►│ Deleted  │
│ id=123   │    │ id=123   │    │ id=123   │    │ id=123   │
│ total=$0 │    │ total=$50│    │ total=$75│    │          │
└──────────┘    └──────────┘    └──────────┘    └──────────┘
     │               │               │               │
     └───────────────┴───────────────┴───────────────┘
                 Same Entity!
          (Identity preserved through changes)
```

---

## 2. Value Object

**Definition**: An object defined by its attributes rather than identity. Value objects are immutable and interchangeable.

**Structure**:
```
┌─────────────────────────────────────────────────────────┐
│                    VALUE OBJECT                         │
│               (Attribute-based equality)                │
├─────────────────────────────────────────────────────────┤
│                                                         │
│  NO IDENTITY - identified by attributes                 │
│                                                         │
│  ┌───────────────────────────────────────────────────┐  │
│  │         ATTRIBUTES (Immutable)                    │  │
│  │  ┌─────────────────────────────────────────────┐  │  │
│  │  │  amount: decimal (readonly)                 │  │  │
│  │  │  currency: string (readonly)                │  │  │
│  │  │                                             │  │  │
│  │  │  Once created, CANNOT be changed            │  │  │
│  │  └─────────────────────────────────────────────┘  │  │
│  └───────────────────────────────────────────────────┘  │
│                                                         │
│  ┌───────────────────────────────────────────────────┐  │
│  │         VALIDATION (On construction)              │  │
│  │  ┌─────────────────────────────────────────────┐  │  │
│  │  │  constructor(amount, currency) {            │  │  │
│  │  │    if (amount < 0)                          │  │  │
│  │  │      throw "Invalid"                        │  │  │
│  │  │    if (currency.length != 3)                │  │  │
│  │  │      throw "Invalid"                        │  │  │
│  │  │  }                                          │  │  │
│  │  └─────────────────────────────────────────────┘  │  │
│  └───────────────────────────────────────────────────┘  │
│                                                         │
│  ┌───────────────────────────────────────────────────┐  │
│  │         OPERATIONS (Return new instances)         │  │
│  │  ┌─────────────────────────────────────────────┐  │  │
│  │  │  + add(other: Money): Money                 │  │  │
│  │  │    return new Money(...)  // New instance!  │  │  │
│  │  │                                             │  │  │
│  │  │  + multiply(factor): Money                  │  │  │
│  │  │    return new Money(...)  // New instance!  │  │  │
│  │  └─────────────────────────────────────────────┘  │  │
│  └───────────────────────────────────────────────────┘  │
│                                                         │
│  Equality: Based on ALL attribute values                │
│                                                         │
│  Money($50, USD) == Money($50, USD)  → True             │
│  Money($50, USD) == Money($75, USD)  → False            │
│                                                         │
└─────────────────────────────────────────────────────────┘

Immutability Example:
─────────────────────
money1 = Money($50, USD)
         │
         │ add($25)
         ▼
money2 = Money($75, USD)  ← NEW instance created

money1 still = Money($50, USD)  ← Original unchanged!

Sharing is Safe:
────────────────
┌─────────┐      ┌─────────┐
│ Order A │─────►│ Money   │◄────┐
└─────────┘      │ ($50)   │     │
                 └─────────┘     │
┌─────────┐                      │
│ Order B │──────────────────────┘
└─────────┘

Both orders can safely share the same Money instance
because it's immutable - neither can change it!
```

**When to Use**:
- Modeling descriptive aspects of the domain
- Object has no life cycle
- Object is interchangeable with others with same attributes
- Encapsulating validation and business rules

**Characteristics**:
- No identity
- Immutable
- Equality based on attributes
- Can be shared/copied freely

**Common Value Objects**:
```
┌─────────────────┐  ┌─────────────────┐  ┌─────────────────┐
│  Money          │  │  Address        │  │  EmailAddress   │
├─────────────────┤  ├─────────────────┤  ├─────────────────┤
│ - amount        │  │ - street        │  │ - value         │
│ - currency      │  │ - city          │  │                 │
│                 │  │ - state         │  │ + isValid()     │
│ + add()         │  │ - postalCode    │  │ + domain()      │
│ + multiply()    │  │ - country       │  │                 │
└─────────────────┘  └─────────────────┘  └─────────────────┘

┌─────────────────┐  ┌─────────────────┐  ┌─────────────────┐
│  DateRange      │  │  PhoneNumber    │  │  Quantity       │
├─────────────────┤  ├─────────────────┤  ├─────────────────┤
│ - start         │  │ - countryCode   │  │ - value         │
│ - end           │  │ - number        │  │ - unit          │
│                 │  │                 │  │                 │
│ + contains()    │  │ + formatted()   │  │ + add()         │
│ + overlaps()    │  │ + isValid()     │  │ + compare()     │
└─────────────────┘  └─────────────────┘  └─────────────────┘
```

---

## 3. Aggregate

**Definition**: A cluster of entities and value objects with defined boundaries and a single root entity.

**Structure**:
```
┌──────────────────────────────────────────────────────────────────┐
│                           AGGREGATE                              │
│                    (Consistency Boundary)                        │
├──────────────────────────────────────────────────────────────────┤
│                                                                  │
│  ╔═══════════════════════════════════════════════════════════╗   │
│  ║             AGGREGATE ROOT (Entity)                       ║   │
│  ║  ┌─────────────────────────────────────────────────────┐  ║   │
│  ║  │  Order                                              │  ║   │
│  ║  │  ─────                                              │  ║   │
│  ║  │  + id: OrderId (global identity)                    │  ║   │
│  ║  │  + customerId: CustomerId                           │  ║   │
│  ║  │  + status: OrderStatus                              │  ║   │
│  ║  │  + shippingAddress: Address                         │  ║   │
│  ║  │                                                     │  ║   │
│  ║  │  PUBLIC INTERFACE (Only entry point):               │  ║   │
│  ║  │  + addItem(...)       ┐                             │  ║   │
│  ║  │  + removeItem(...)    │ Enforces                    │  ║   │
│  ║  │  + submit()           │ invariants                  │  ║   │
│  ║  │  + cancel()           ┘                             │  ║   │
│  ║  └─────────────────────────────────────────────────────┘  ║   │
│  ╚═══════════════════════════════════════════════════════════╝   │
│                              │                                   │
│                              │ owns                              │
│                              ▼                                   │
│  ┌──────────────────────────────────────────────────────────┐    │
│  │         INTERNAL ENTITIES (Not accessible outside)       │    │
│  │  ┌────────────────┐  ┌────────────────┐  ┌────────────┐  │    │
│  │  │ OrderItem      │  │ OrderItem      │  │ OrderItem  │  │    │
│  │  │ ────────       │  │ ────────       │  │ ──────     │  │    │
│  │  │ id: local ID   │  │ id: local ID   │  │ id: local  │  │    │
│  │  │ productId      │  │ productId      │  │ productId  │  │    │
│  │  │ quantity       │  │ quantity       │  │ quantity   │  │    │
│  │  │ unitPrice      │  │ unitPrice      │  │ unitPrice  │  │    │
│  │  └────────────────┘  └────────────────┘  └────────────┘  │    │
│  └──────────────────────────────────────────────────────────┘    │
│                              │                                   │
│                              │ uses                              │
│                              ▼                                   │
│  ┌──────────────────────────────────────────────────────────┐    │
│  │              VALUE OBJECTS (Immutable)                   │    │
│  │  ┌────────────┐  ┌────────────┐  ┌────────────────────┐  │    │
│  │  │ Money      │  │ Address    │  │ OrderStatus        │  │    │
│  │  │ ($50, USD) │  │ (123 Main) │  │ (PENDING)          │  │    │
│  │  └────────────┘  └────────────┘  └────────────────────┘  │    │
│  └──────────────────────────────────────────────────────────┘    │
│                                                                  │
│  ┌──────────────────────────────────────────────────────────┐    │
│  │                    INVARIANTS                            │    │
│  │  • Order total must be > $10                             │    │
│  │  • Maximum 10 items per order                            │    │
│  │  • Submitted order cannot be modified                    │    │
│  │  • Order must have shipping address to submit            │    │
│  └──────────────────────────────────────────────────────────┘    │
│                                                                  │
│  ┌──────────────────────────────────────────────────────────┐    │
│  │              TRANSACTIONAL BOUNDARY                      │    │
│  │  Save/Load entire aggregate as atomic unit               │    │
│  │  All invariants checked within single transaction        │    │
│  └──────────────────────────────────────────────────────────┘    │
│                                                                  │
└──────────────────────────────────────────────────────────────────┘

External Access Rules:
──────────────────────
┌──────────┐                  ┌─────────────────┐
│ External │─────────────────►│ Order (Root)    │ ✓ Allowed
│ Client   │    references    └─────────────────┘
└──────────┘                           │
                                       │ owns
                    ┌──────────────────┼──────────────┐
                    ▼                  ▼              ▼
              ┌──────────┐       ┌──────────┐  ┌──────────┐
              │OrderItem │       │OrderItem │  │OrderItem │
              └──────────┘       └──────────┘  └──────────┘
                    ▲                   ▲             ▲
                    │                   │             │
┌──────────┐        │                   │             │
│ External │────────┘                   │             │
│ Client   │  Direct access ✗ FORBIDDEN │             │
└──────────┘                            │             │
                                        │             │
┌──────────┐                            │             │
│ External │────────────────────────────┘             │
│ Client   │  Direct access ✗ FORBIDDEN               │
└──────────┘                                          │
                                                      │
┌──────────┐                                          │
│ External │──────────────────────────────────────────┘
│ Client   │  Direct access ✗ FORBIDDEN
└──────────┘

Reference Pattern:
─────────────────
┌──────────┐    holds ID      ┌──────────┐
│ Order A  │─────────────────►│ Order B  │
│ id: 123  │  customerId=456  │ id: 789  │
└──────────┘                  └──────────┘
     │                             ▲
     │                             │
     └─────── NOT ─────────────────┘
         Direct object reference
         (would couple aggregates)
```

**Invariant Rules**:
- Root entity has global identity
- Non-root entities have local identity within aggregate
- External objects can only hold references to root
- Only root can be obtained through queries
- Internal objects can hold references to each other
- Delete removes everything in the aggregate

**When to Use**:
- Need to enforce invariants across multiple objects
- Group of objects that change together
- Define transactional consistency boundaries

**Aggregate Size Guidelines**:
```
❌ TOO LARGE:
┌─────────────────────────────────────────────┐
│  Customer Aggregate                         │
│  ├─ Customer (root)                         │
│  ├─ Address []                              │
│  ├─ Order []  ← Don't include!              │
│  │   ├─ OrderItem []                        │
│  │   └─ Payment                             │
│  ├─ ShoppingCart                            │
│  │   └─ CartItem []                         │
│  └─ Wishlist []                             │
│      └─ WishlistItem []                     │
└─────────────────────────────────────────────┘
Problems:
 • Too many objects in one transaction
 • Performance issues
 • Concurrency conflicts
 • Difficult to maintain invariants

✅ BETTER (Separate Aggregates):
┌──────────────────┐  ┌──────────────────┐  ┌──────────────────┐
│  Customer        │  │  Order           │  │  ShoppingCart    │
│  ├─ Customer     │  │  ├─ Order        │  │  ├─ ShoppingCart │
│  └─ Address []   │  │  ├─ OrderItem [] │  │  └─ CartItem []  │
└──────────────────┘  │  └─ Payment      │  └──────────────────┘
        ▲             └──────────────────┘           ▲
        │                      ▲                     │
        └─────── Reference by ID ────────────────────┘

Benefits:
 • Smaller transactional boundaries
 • Better performance
 • Fewer conflicts
 • Clear invariant boundaries
 • Eventual consistency between aggregates
```

---

## 4. Repository

**Definition**: Provides collection-like interface for accessing aggregates. Mediates between domain and data mapping layers.

**Structure**:
```
┌─────────────────────────────────────────────────────────────────┐
│                        DOMAIN LAYER                             │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  ┌───────────────────────────────────────────────────────────┐  │
│  │              OrderRepository (Interface)                  │  │
│  │              Abstract/Contract                            │  │
│  ├───────────────────────────────────────────────────────────┤  │
│  │  + findById(orderId): Order?                              │  │
│  │  + findByCustomer(customerId): Order[]                    │  │
│  │  + findPendingOrders(): Order[]                           │  │
│  │  + save(order: Order): void                               │  │
│  │  + delete(order: Order): void                             │  │
│  └───────────────────────────────────────────────────────────┘  │
│                              ▲                                  │
│                              │ implements                       │
│                              │                                  │
└──────────────────────────────┼──────────────────────────────────┘
                               │
┌──────────────────────────────┼──────────────────────────────────┐
│                INFRASTRUCTURE LAYER                             │
├──────────────────────────────┼──────────────────────────────────┤
│                              │                                  │
│  ┌───────────────────────────┴───────────────────────────────┐  │
│  │        SqlOrderRepository (Concrete Implementation)       │  │
│  ├───────────────────────────────────────────────────────────┤  │
│  │  - dbConnection: Database                                 │  │
│  ├───────────────────────────────────────────────────────────┤  │
│  │  + findById(orderId): Order? {                            │  │
│  │      // SQL query to load order                           │  │
│  │      row = db.query("SELECT * FROM orders WHERE...")      │  │
│  │      return reconstituteOrder(row)                        │  │
│  │    }                                                      │  │
│  │                                                           │  │
│  │  + save(order: Order): void {                             │  │
│  │      // Save aggregate atomically                         │  │
│  │      transaction {                                        │  │
│  │        saveOrder(order)                                   │  │
│  │        saveOrderItems(order.items)                        │  │
│  │        dispatchEvents(order.domainEvents)                 │  │
│  │      }                                                    │  │
│  │    }                                                      │  │
│  └───────────────────────────────────────────────────────────┘  │
│                              │                                  │
│                              ▼                                  │
│  ┌───────────────────────────────────────────────────────────┐  │
│  │                    DATABASE                               │  │
│  │  ┌─────────────┐  ┌─────────────────┐  ┌──────────────┐   │  │
│  │  │ orders      │  │ order_items     │  │ order_events │   │  │
│  │  │ table       │  │ table           │  │ table        │   │  │
│  │  └─────────────┘  └─────────────────┘  └──────────────┘   │  │
│  └───────────────────────────────────────────────────────────┘  │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘

Collection Illusion:
────────────────────
Repository acts like in-memory collection:

  orders = OrderRepository

  order = orders.findById(123)  // Like: collection[123]
  order.submit()                // Modify in memory
  orders.save(order)            // Like: collection.add(order)

  // Client doesn't know about:
  //   - SQL queries
  //   - Database transactions
  //   - ORM mapping
  //   - Caching
```

**One Repository Per Aggregate Root**:
```
✓ CORRECT:
┌──────────────────┐      ┌──────────────────────┐
│ Order            │◄─────│ OrderRepository      │
│ (Aggregate Root) │      └──────────────────────┘
│  ├─ OrderItem    │
│  └─ Payment      │
└──────────────────┘

✓ CORRECT:
┌──────────────────┐      ┌──────────────────────┐
│ Customer         │◄─────│ CustomerRepository   │
│ (Aggregate Root) │      └──────────────────────┘
│  └─ Address []   │
└──────────────────┘

✗ WRONG:
┌──────────────────┐       ┌──────────────────────┐
│ Order            │       │ OrderRepository      │
│ (Aggregate Root) │       └──────────────────────┘
│  ├─ OrderItem    │       ┌──────────────────────┐
│  │               │  ◄────│ OrderItemRepository  │ ✗ No!
│  │               │       └──────────────────────┘
│  └─ Payment      │       ┌──────────────────────┐
│                  │  ◄────│ PaymentRepository    │ ✗ No!
└──────────────────┘       └──────────────────────┘

OrderItems and Payments are accessed through Order only!
```

**When to Use**:
- Need to retrieve aggregates from storage
- Want to isolate domain from persistence concerns
- Implementing data access patterns

**Characteristics**:
- One repository per aggregate root
- Provides illusion of in-memory collection
- Hides data access details from domain
- Supports querying by ID and domain-specific criteria

---

## 5. Domain Service

**Definition**: Stateless operation that doesn't naturally belong to an entity or value object. Operates on domain objects.

**When to Use Domain Service**:
```
✓ USE Domain Service when:
  1. Logic spans multiple aggregates
     ┌────────┐              ┌────────┐
     │Order   │─────────────►│Customer│
     └────────┘   Pricing    └────────┘
                  logic
                  uses both

  2. No natural home in any entity
     "Which aggregate should calculate
      transfer between two accounts?"
     → Neither! Use TransferService

  3. Operation is stateless
     calculatePrice(order, customer)
     No state stored in service

✗ DON'T USE Domain Service when:
  1. Logic belongs to one aggregate
     order.calculateTotal() ← In Order
     Not: OrderService.calculateTotal(order)

  2. Logic is about single value object
     money.add(other) ← In Money
     Not: MoneyService.add(money1, money2)
```

**Example Comparison**:
```
❌ WRONG (Anemic model):
  class Order {
    items: OrderItem[]
    // No behavior, just data
  }

  class OrderService {
    calculateTotal(order) {
      // Business logic in service
      return sum(order.items.map(i => i.total))
    }
  }

✅ CORRECT (Rich model):
  class Order {
    items: OrderItem[]

    calculateTotal() {
      // Business logic in entity
      return sum(this.items.map(i => i.total))
    }
  }

✅ CORRECT (Domain service for multi-aggregate):
  class OrderPricingService {
    calculateFinalPrice(order, customer) {
      // Needs both aggregates
      base = order.calculateTotal()
      discount = customer.getDiscount(base)
      tax = calculateTax(base - discount)
      return base - discount + tax
    }
  }
```

**Domain Service vs Application Service**:
```
┌────────────────────────────────────────────────────────────────┐
│                    APPLICATION SERVICE                         │
│                (Orchestration, not domain logic)               │
├────────────────────────────────────────────────────────────────┤
│  class PlaceOrderAppService {                                  │
│    handle(command: PlaceOrderCommand) {                        │
│      // 1. Load aggregates                                     │
│      order = orderRepo.findById(command.orderId)               │
│      customer = customerRepo.findById(command.customerId)      │
│                                                                │
│      // 2. Call domain service (business logic)                │
│      price = pricingService.calculateFinalPrice(               │
│        order, customer, command.promoCode                      │
│      )                                                         │
│                                                                │
│      // 3. Update aggregate                                    │
│      order.submit(price)                                       │
│                                                                │
│      // 4. Persist                                             │
│      orderRepo.save(order)                                     │
│                                                                │
│      // 5. Send notifications (infrastructure)                 │
│      emailService.sendConfirmation(customer.email)             │
│    }                                                           │
│  }                                                             │
└────────────────────────────────────────────────────────────────┘
                               │
                               │ uses
                               ▼
┌────────────────────────────────────────────────────────────────┐
│                     DOMAIN SERVICE                             │
│                   (Business logic)                             │
├────────────────────────────────────────────────────────────────┤
│  class OrderPricingService {                                   │
│    calculateFinalPrice(order, customer, promoCode) {           │
│      // Pure business logic                                    │
│      // No infrastructure concerns                             │
│      base = order.calculateTotal()                             │
│      discount = customer.calculateDiscount(base)               │
│      promo = applyPromotion(promoCode, base)                   │
│      tax = calculateTax(base - discount - promo)               │
│      return base - discount - promo + tax                      │
│    }                                                           │
│  }                                                             │
└────────────────────────────────────────────────────────────────┘

Key Differences:
───────────────
Application Service:
  • Orchestrates use case
  • Loads/saves aggregates
  • Calls infrastructure
  • Transaction boundaries
  • No business logic

Domain Service:
  • Business logic only
  • Works with domain objects
  • No persistence
  • No infrastructure
  • Stateless operations
```

---

## 6. Factory

**Definition**: Encapsulates complex object creation logic.

**When to Use**:
- Object construction is complex
- Requires multiple steps or validation
- Need to enforce invariants during creation
- Hide implementation details of construction

**Factory Patterns**:
```
┌────────────────────────────────────────────────────────────┐
│              FACTORY METHOD PATTERN                        │
├────────────────────────────────────────────────────────────┤
│  Abstract factory method in base class/interface           │
│                                                            │
│  interface OrderFactory {                                  │
│    createOrder(...): Order                                 │
│  }                                                         │
│                                                            │
│  class StandardOrderFactory implements OrderFactory {      │
│    createOrder(...): Order {                               │
│      // Standard order creation                            │
│    }                                                       │
│  }                                                         │
│                                                            │
│  class BulkOrderFactory implements OrderFactory {          │
│    createOrder(...): Order {                               │
│      // Bulk order creation with discounts                 │
│    }                                                       │
│  }                                                         │
└────────────────────────────────────────────────────────────┘

┌────────────────────────────────────────────────────────────┐
│              ABSTRACT FACTORY PATTERN                      │
├────────────────────────────────────────────────────────────┤
│  Create families of related objects                        │
│                                                            │
│  interface EcommerceFactory {                              │
│    createOrder(): Order                                    │
│    createPayment(): Payment                                │
│    createShipment(): Shipment                              │
│  }                                                         │
│                                                            │
│  class B2CFactory implements EcommerceFactory {            │
│    // Create B2C versions                                  │
│  }                                                         │
│                                                            │
│  class B2BFactory implements EcommerceFactory {            │
│    // Create B2B versions                                  │
│  }                                                         │
└────────────────────────────────────────────────────────────┘
```

---

## 7. Domain Events

**Definition**: Something significant that happened in the domain that matters to the business.

**Structure**:
```
┌─────────────────────────────────────────────────────────────────┐
│                        DOMAIN EVENT                             │
│                   (Immutable fact)                              │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  ┌───────────────────────────────────────────────────────────┐  │
│  │  OrderSubmitted                                           │  │
│  │  (Named in past tense - already happened)                 │  │
│  ├───────────────────────────────────────────────────────────┤  │
│  │  Identity:                                                │  │
│  │  - eventId: "evt_789"                                     │  │
│  │  - occurredAt: 2025-01-15 10:30:00 UTC                    │  │
│  │  - aggregateId: "order_456"                               │  │
│  │                                                           │  │
│  │  Event Data (Relevant information):                       │  │
│  │  - customerId: "cust_123"                                 │  │
│  │  - orderTotal: Money($50, USD)                            │  │
│  │  - itemsCount: 3                                          │  │
│  │  - shippingAddress: Address(...)                          │  │
│  │                                                           │  │
│  │  IMMUTABLE - Cannot be changed after creation             │  │
│  └───────────────────────────────────────────────────────────┘  │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘

Cross-Context Communication:
────────────────────────────
┌─────────────────────┐      Event       ┌─────────────────────┐
│  Order Context      │─────────────────►│ Shipping Context    │
│  (Publisher)        │ OrderSubmitted   │ (Subscriber)        │
│                     │                  │                     │
│  Order.submit()     │                  │ Creates:            │
│    raises event     │                  │  - Shipment         │
└─────────────────────┘                  │  - DeliverySchedule │
                                         └─────────────────────┘

┌─────────────────────┐      Event       ┌─────────────────────┐
│  Order Context      │─────────────────►│ Inventory Context   │
│  (Publisher)        │ OrderSubmitted   │ (Subscriber)        │
│                     │                  │                     │
│  Order.submit()     │                  │ Reserves:           │
│    raises event     │                  │  - Stock            │
└─────────────────────┘                  │  - Items            │
                                         └─────────────────────┘

Benefits:
  • Decouples bounded contexts
  • Enables eventual consistency
  • Provides audit trail
  • Triggers workflows
  • Supports event sourcing
```

**When to Use**:
- Modeling significant business occurrences
- Decoupling bounded contexts
- Eventual consistency between aggregates
- Audit trail and event sourcing
- Triggering workflows or side effects

**Characteristics**:
- Named in past tense (OrderSubmitted, PaymentProcessed)
- Immutable
- Contains relevant data about what happened
- Timestamp of occurrence
- Can trigger side effects in other parts of system

**Event Naming Conventions**:
```
✓ GOOD (Past tense, business language):
  - OrderSubmitted
  - PaymentProcessed
  - CustomerRegistered
  - InventoryReserved
  - ShipmentDispatched
  - AccountClosed

✗ BAD (Present/future tense, technical):
  - SubmitOrder         (command, not event)
  - ProcessingPayment   (not completed)
  - OrderUpdate         (vague)
  - DataChanged         (technical)
  - RecordInserted      (implementation detail)
```

---

## Tactical Design Workflow

1. **Model Aggregates**
   - Identify transactional boundaries
   - Design aggregate roots
   - Enforce invariants

2. **Design Entities and Value Objects**
   - Identify objects with identity (Entities)
   - Identify objects without identity (Value Objects)
   - Make value objects immutable

3. **Define Repositories**
   - One repository per aggregate root
   - Domain-specific query methods
   - Hide persistence details

4. **Identify Domain Services**
   - Operations spanning multiple aggregates
   - Business logic without natural home
   - Stateless operations

5. **Model Domain Events**
   - Identify significant occurrences
   - Name in past tense
   - Capture relevant data

---

## Questions for Tactical Design

When working with users on tactical design, ask:

1. "Which objects need to be tracked over time? (Entities)"
2. "Which objects are interchangeable and have no identity? (Value Objects)"
3. "What invariants must be enforced within a transaction? (Aggregates)"
4. "What operations don't naturally belong to an entity? (Domain Services)"
5. "What are the business rules for this concept?"
6. "What significant things happen in the domain? (Domain Events)"

For refactoring:
1. "Is this an anemic domain model (just data, no behavior)?"
2. "Are business rules scattered across services or in the database?"
3. "Are there transaction scripts that should be domain logic?"
4. "Is domain logic leaking into application or infrastructure layers?"
