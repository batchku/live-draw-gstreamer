# DDD Strategic Design Patterns

Strategic design focuses on high-level domain decomposition, defining bounded contexts, and establishing integration patterns between contexts.

## 1. Bounded Context

**Definition**: An explicit boundary within which a domain model is defined and applicable. Each bounded context has its own ubiquitous language and model.

**Structure**:
```
┌────────────────────────────────────────────────────────────────┐
│                      BOUNDED CONTEXT                           │
├────────────────────────────────────────────────────────────────┤
│                                                                │
│  ┌────────────────────────────────────────────────────────┐    │
│  │         Ubiquitous Language (Domain Model)             │    │
│  │                                                        │    │
│  │  Customer means "Buyer with shopping cart"             │    │
│  │  Product means "Catalog item with price & description" │    │
│  │  Order means "Customer purchase request"               │    │
│  └────────────────────────────────────────────────────────┘    │
│                                                                │
│  ┌───────────────────┐  ┌───────────────────┐                  │
│  │  Domain Model     │  │  Business Rules   │                  │
│  │  - Entities       │  │  - Validations    │                  │
│  │  - Value Objects  │  │  - Invariants     │                  │
│  │  - Aggregates     │  │  - Workflows      │                  │
│  └───────────────────┘  └───────────────────┘                  │
│                                                                │
│  ┌─────────────────────────────────────────────────────┐       │
│  │          Published Interface (API/Events)           │       │
│  └─────────────────────────────────────────────────────┘       │
│                                                                │
└────────────────────────────────────────────────────────────────┘
        ▲                                          ▲
        │ Clear boundary                           │ External access
        │                                          │ only through interface
```

**When to Use**:
- Separating different meanings of the same concept
- Isolating different teams or subsystems
- Preventing model corruption
- Enabling independent evolution of models

**Key Characteristics**:
- Clear boundaries with explicit interfaces
- Single unified model within the context
- Own ubiquitous language
- Autonomous team ownership (ideally)

**Example - E-commerce System**:
```
┌─────────────────────┐       ┌─────────────────────┐
│  Sales Context      │       │  Shipping Context   │
├─────────────────────┤       ├─────────────────────┤
│ Customer:           │       │ Shipment            │
│   "Person buying"   │       │ DeliveryAddress     │
│ Order:              │       │ Package             │
│   "Purchase request"│       │ Carrier             │
│ Product:            │       │ TrackingNumber      │
│   "Catalog item"    │       │                     │
│ ShoppingCart        │       │                     │
└──────────┬──────────┘       └──────────┬──────────┘
           │                             │
           │   OrderPlaced Event         │
           └─────────────────────────────┘

┌─────────────────────┐       ┌─────────────────────┐
│  Inventory Context  │       │  Billing Context    │
├─────────────────────┤       ├─────────────────────┤
│ Product:            │       │ Invoice             │
│   "Stock item"      │       │ Payment             │
│ StockLevel          │       │ BillingAccount      │
│ Warehouse           │       │ Transaction         │
│ Reservation         │       │ Customer:           │
│ SKU                 │       │   "Bill payer"      │
└─────────────────────┘       └─────────────────────┘

Note: "Product" and "Customer" have different meanings
in each context - this is intentional and correct!
```

---

## 2. Context Mapping Patterns

**Context Map Overview**:
```
┌───────────────────────────────────────────────────────────────────┐
│                        CONTEXT MAP                                │
│                 (System-wide integration view)                    │
├───────────────────────────────────────────────────────────────────┤
│                                                                   │
│   ┌─────────────┐                                                 │
│   │  Context A  │───────── Shared Kernel ─────────┐               │
│   └──────┬──────┘                                 │               │
│          │                                  ┌─────▼──────┐        │
│          │ Customer/Supplier                │  Context B │        │
│          │ Relationship                     └─────┬──────┘        │
│          ▼                                        │               │
│   ┌─────────────┐                                 │ Conformist    │
│   │  Context C  │                                 ▼               │
│   └──────┬──────┘                           ┌────────────┐        │
│          │                                  │  Context D │        │
│          │ Anti-Corruption                  └────────────┘        │
│          │ Layer                                                  │
│          ▼                                                        │
│   ┌─────────────┐                                                 │
│   │   Legacy    │                                                 │
│   │   System    │                                                 │
│   └─────────────┘                                                 │
│                                                                   │
└───────────────────────────────────────────────────────────────────┘
```

### 2.1 Shared Kernel Pattern

Two contexts share a subset of the domain model.

```
┌──────────────────────────┐         ┌──────────────────────────┐
│  Sales Context           │         │  Inventory Context       │
│                          │         │                          │
│  ┌────────────────────┐  │         │  ┌────────────────────┐  │
│  │  SHARED KERNEL     │◄─┼─────────┼─►│  SHARED KERNEL     │  │
│  │  ┌──────────────┐  │  │         │  │  ┌──────────────┐  │  │
│  │  │ Money        │  │  │ Shared  │  │  │ Money        │  │  │
│  │  │ Currency     │  │  │   Code  │  │  │ Currency     │  │  │
│  │  │ ProductId    │  │  │         │  │  │ ProductId    │  │  │
│  │  └──────────────┘  │  │         │  │  └──────────────┘  │  │
│  └────────────────────┘  │         │  └────────────────────┘  │
│                          │         │                          │
│  ┌────────────────────┐  │         │  ┌────────────────────┐  │
│  │  Order             │  │         │  │  StockLevel        │  │
│  │  ShoppingCart      │  │         │  │  Warehouse         │  │
│  └────────────────────┘  │         │  └────────────────────┘  │
│                          │         │                          │
└──────────────────────────┘         └──────────────────────────┘

Advantages:
  • Reduces duplication
  • Ensures consistency
  • Shared understanding

Disadvantages:
  • Tight coupling
  • Coordination required
  • Changes affect both teams
```

**When to Use**:
- Teams are closely coordinated
- Shared subset is small and stable
- Changes are infrequent

### 2.2 Customer-Supplier Pattern

Downstream context (customer) depends on upstream context (supplier).

```
┌─────────────────────────────────────┐
│   Order Management Context          │ ← UPSTREAM (Supplier)
│   (Supplier)                        │
│                                     │
│  ┌─────────────────────────────┐    │
│  │  Order aggregate            │    │
│  │  OrderSubmitted event       │    │
│  └─────────────────────────────┘    │
│                                     │
└──────────────┬──────────────────────┘
               │
               │ Published Interface:
               │  - REST API
               │  - Events: OrderSubmitted, OrderCancelled
               │  - Contract negotiated with downstream
               │
               ▼
┌─────────────────────────────────────┐
│   Shipping Context                  │ ← DOWNSTREAM (Customer)
│   (Customer)                        │
│                                     │
│  ┌─────────────────────────────┐    │
│  │  Subscribes to:             │    │
│  │  - OrderSubmitted           │    │
│  │  - OrderCancelled           │    │
│  │                             │    │
│  │  Creates:                   │    │
│  │  - Shipment                 │    │
│  │  - DeliverySchedule         │    │
│  └─────────────────────────────┘    │
│                                     │
└─────────────────────────────────────┘

Responsibilities:
  Upstream (Supplier):
    • Define and publish interface
    • Maintain API compatibility
    • Consider downstream needs
    • Communicate changes

  Downstream (Customer):
    • Depend on published interface
    • Provide input on interface design
    • Adapt to interface changes
```

**When to Use**:
- Clear upstream/downstream relationship
- Downstream needs influence upstream priorities
- Formal integration contract

### 2.3 Conformist Pattern

Downstream conforms to upstream model without modification.

```
┌─────────────────────────────────────┐
│   Third-Party Service               │ ← UPSTREAM
│   (Stripe Payment API)              │   (Dominant, won't change)
│                                     │
│  PaymentIntent {                    │
│    id, amount, currency,            │
│    status, customer, ...            │
│  }                                  │
│                                     │
└──────────────┬──────────────────────┘
               │
               │ Use their model as-is
               │ No translation
               │
               ▼
┌─────────────────────────────────────┐
│   Your Billing Context              │ ← DOWNSTREAM
│   (Conformist - accepts upstream)   │   (Adapts to upstream)
│                                     │
│  // Use Stripe's model directly     │
│  PaymentIntent                      │
│  (same structure as Stripe)         │
│                                     │
│  PaymentService {                   │
│    createPayment(PaymentIntent)     │
│    // Works with Stripe model       │
│  }                                  │
│                                     │
└─────────────────────────────────────┘

When to use:
  ✓ Upstream won't change for you
  ✓ Integration is critical
  ✓ Translation cost is too high
  ✓ Upstream model is "good enough"

Trade-offs:
  Advantages:
    • Simple integration
    • No translation overhead
    • Faster development

  Disadvantages:
    • Your model depends on external model
    • Loss of domain purity
    • Vulnerable to upstream changes
```

**When to Use**:
- Upstream won't accommodate downstream needs
- Integration is more important than model purity
- Cost of translation layer is too high

### 2.4 Anti-Corruption Layer (ACL) Pattern

Translate between contexts to prevent external model from corrupting your model.

```
┌──────────────────────────────────────────┐
│  Legacy CRM System                       │ ← UPSTREAM
│  (Poor model, can't change)              │   (External/Legacy)
├──────────────────────────────────────────┤
│  LegacyCustomer {                        │
│    cust_num: integer                     │
│    fname, lname: string                  │
│    em: string                            │
│    stat: integer (0=inactive, 1=active)  │
│    addr_ln1, addr_ln2, city, ...         │
│  }                                       │
└────────────────┬─────────────────────────┘
                 │
                 │
                 ▼
┌──────────────────────────────────────────┐
│  ANTI-CORRUPTION LAYER                   │ ← Translation Layer
│  (Protects domain from legacy)           │
├──────────────────────────────────────────┤
│  CustomerACL {                           │
│                                          │
│    toDomain(legacy): Customer {          │
│      return Customer(                    │
│        id: CustomerId(legacy.cust_num)   │
│        name: FullName(                   │
│          first: legacy.fname,            │
│          last: legacy.lname              │
│        ),                                │
│        email: EmailAddress(legacy.em)    │
│        status: mapStatus(legacy.stat)    │
│        address: Address(...)             │
│      )                                   │
│    }                                     │
│                                          │
│    toLegacy(customer): Legacy {          │
│      // Reverse translation              │
│    }                                     │
│  }                                       │
└────────────────┬─────────────────────────┘
                 │
                 │ Clean domain model
                 │ protected from legacy
                 ▼
┌──────────────────────────────────────────┐
│  Your Domain Model                       │ ← DOWNSTREAM
│  (Clean, domain-focused)                 │   (Protected)
├──────────────────────────────────────────┤
│  Customer {                              │
│    id: CustomerId                        │
│    name: FullName                        │
│    email: EmailAddress                   │
│    status: CustomerStatus                │
│    address: Address                      │
│  }                                       │
│                                          │
│  FullName {                              │
│    first, last: string                   │
│    fullName(): string                    │
│  }                                       │
│                                          │
│  CustomerStatus: enum {                  │
│    ACTIVE, INACTIVE, SUSPENDED           │
│  }                                       │
└──────────────────────────────────────────┘

Benefits:
  • Domain model stays clean
  • Isolated from legacy changes
  • Can evolve independently
  • Clear translation responsibility
```

**When to Use**:
- Integrating with legacy systems
- External system has poor or incompatible model
- Protecting your bounded context from external changes
- Multiple upstream systems with different models

### 2.5 Open Host Service + Published Language

Define a well-documented service for multiple consumers.

```
    ┌─────────────┐    ┌─────────────┐    ┌─────────────┐
    │  Context A  │    │  Context B  │    │  Context C  │
    │ (Consumer)  │    │ (Consumer)  │    │ (Consumer)  │
    └──────┬──────┘    └──────┬──────┘    └──────┬──────┘
           │                  │                  │
           │    HTTP/REST     │    gRPC          │  Message Queue
           └─────────┬────────┴──────────┬───────┘
                     │                   │
                     ▼                   ▼
        ┌────────────────────────────────────────────┐
        │      OPEN HOST SERVICE                     │
        │      (Well-defined public API)             │
        ├────────────────────────────────────────────┤
        │                                            │
        │  REST API          gRPC            Events  │
        │  ┌─────────┐      ┌─────────┐    ┌──────┐  │
        │  │GET /ord │      │GetOrder │    │Order │  │
        │  │POST/ord │      │Submit   │    │Submit│  │
        │  │DELETE/..│      │Cancel   │    │Cancel│  │
        │  └─────────┘      └─────────┘    └──────┘  │
        │                                            │
        │  ┌──────────────────────────────────────┐  │
        │  │     PUBLISHED LANGUAGE               │  │
        │  │     (Standardized format)            │  │
        │  ├──────────────────────────────────────┤  │
        │  │  OrderDTO {                          │  │
        │  │    orderId: string                   │  │
        │  │    customerId: string                │  │
        │  │    items: OrderItem[]                │  │
        │  │    status: "pending"|"submitted"     │  │
        │  │    total: Money                      │  │
        │  │  }                                   │  │
        │  │                                      │  │
        │  │  Money {                             │  │
        │  │    amount: decimal                   │  │
        │  │    currency: string                  │  │
        │  │  }                                   │  │
        │  └──────────────────────────────────────┘  │
        │                                            │
        └────────────────┬───────────────────────────┘
                         │
                         ▼
            ┌────────────────────────┐
            │  Order Context         │
            │  (Domain Model)        │
            └────────────────────────┘

Characteristics:
  • Multiple protocols supported
  • Well-documented API
  • Versioned interface
  • Published schema/contracts
  • Stable integration points
```

---

## 3. Subdomain Types

```
┌────────────────────────────────────────────────────────────────────┐
│                         DOMAIN                                     │
│                    (Entire business)                               │
├────────────────────────────────────────────────────────────────────┤
│                                                                    │
│  ┌───────────────────────────────────────────────────────────────┐ │
│  │  CORE DOMAIN                                                  │ │
│  │  (Your competitive advantage - INVEST HERE)                   │ │
│  ├───────────────────────────────────────────────────────────────┤ │
│  │  • Most important to business                                 │ │
│  │  • Differentiates you from competitors                        │ │
│  │  • High complexity                                            │ │
│  │  • Best developers work here                                  │ │
│  │  • Custom implementation required                             │ │
│  │                                                               │ │
│  │  Examples:                                                    │ │
│  │    Netflix: Recommendation algorithm                          │ │
│  │    Amazon: Fulfillment optimization                           │ │
│  │    Google: Search ranking                                     │ │
│  └───────────────────────────────────────────────────────────────┘ │
│                                                                    │
│  ┌───────────────────────────────────────────────────────────────┐ │
│  │  SUPPORTING SUBDOMAIN                                         │ │
│  │  (Supports core but not differentiating)                      │ │
│  ├───────────────────────────────────────────────────────────────┤ │
│  │  • Important but not unique                                   │ │
│  │  • Specific to your business                                  │ │
│  │  • Moderate complexity                                        │ │
│  │  • Can be custom or outsourced                                │ │
│  │                                                               │ │
│  │  Examples:                                                    │ │
│  │    Netflix: Billing/subscription management                   │ │
│  │    Amazon: Warehouse management                               │ │
│  │    Google: Ad serving infrastructure                          │ │
│  └───────────────────────────────────────────────────────────────┘ │
│                                                                    │
│  ┌───────────────────────────────────────────────────────────────┐ │
│  │  GENERIC SUBDOMAIN                                            │ │
│  │  (Common across all businesses - BUY DON'T BUILD)             │ │
│  ├───────────────────────────────────────────────────────────────┤ │
│  │  • Not specific to your business                              │ │
│  │  • Low competitive value                                      │ │
│  │  • Well-understood problem                                    │ │
│  │  • Buy off-the-shelf or use open source                       │ │
│  │                                                               │ │
│  │  Examples:                                                    │ │
│  │    • Authentication/authorization                             │ │
│  │    • Email sending                                            │ │
│  │    • Logging and monitoring                                   │ │
│  │    • Payment processing                                       │ │
│  └───────────────────────────────────────────────────────────────┘ │
│                                                                    │
└────────────────────────────────────────────────────────────────────┘

Investment Strategy:
  Core Domain       → Best team, most time, custom code
  Supporting        → Good team, moderate time, custom or outsource
  Generic           → Minimal team, minimal time, buy/use existing
```

---

## Strategic Design Workflow

1. **Identify Subdomains**
   - Analyze business capabilities
   - Classify as Core, Supporting, or Generic
   - Prioritize investment based on classification

2. **Define Bounded Contexts**
   - Identify linguistic boundaries
   - Group models by team ownership
   - Define context boundaries

3. **Map Context Relationships**
   - Identify upstream/downstream relationships
   - Choose integration patterns
   - Document context map

4. **Establish Ubiquitous Language**
   - Create glossary per context
   - Use business domain terminology
   - Avoid technical jargon

---

## Questions for Strategic Design

When working with users on strategic design, ask:

1. "What are the main business capabilities or areas of your domain?"
2. "Which capabilities are core to your competitive advantage?"
3. "What teams will work on different parts of the system?"
4. "Where do concepts have different meanings in different contexts?"
5. "How should different contexts integrate with each other?"
6. "What events are significant to the business?"
