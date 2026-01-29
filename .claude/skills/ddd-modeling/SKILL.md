---
name: ddd-modeling
description: Applies Domain-Driven Design (DDD) principles to model complex business domains using strategic and tactical patterns. Use when modeling complex business logic, defining bounded contexts, creating ubiquitous language, designing aggregates and entities, implementing repositories, modeling domain events, establishing context boundaries, or applying DDD strategic patterns (Context Map, Anti-Corruption Layer, Shared Kernel). Covers both strategic design (context mapping, domain decomposition) and tactical design (entities, value objects, aggregates, domain services, repositories, factories).
allowed-tools: Read Glob Grep
---

# Domain-Driven Design (DDD) Modeling Skill

This skill helps developers and architects apply Domain-Driven Design principles to model complex business domains effectively. Use it to create maintainable, expressive models that reflect the business domain.

**Note**: All code examples in this skill use pseudo-code notation to remain language-agnostic. Adapt the concepts to your programming language of choice.

## When to Use This Skill

Use this skill when you need to:
- Model complex business domains with rich domain logic
- Establish ubiquitous language between technical and business teams
- Define bounded contexts and context boundaries
- Design aggregates, entities, and value objects
- Implement repositories and domain services
- Model domain events and event-driven architectures
- Apply strategic DDD patterns for system integration
- Refactor anemic domain models to rich domain models
- Document domain knowledge and business rules
- Decompose large domains into manageable contexts

## Purpose

This skill provides a systematic approach to applying DDD by:

1. **Strategic Design**: Define bounded contexts, context maps, and integration patterns
2. **Tactical Design**: Implement entities, value objects, aggregates, repositories, and services
3. **Ubiquitous Language**: Establish shared vocabulary that reflects the business domain
4. **Domain Events**: Model significant business events that matter to the business
5. **Business Logic Encapsulation**: Keep business rules within the domain model
6. **Context Boundaries**: Prevent model corruption across context boundaries

---

## DDD Overview

### Strategic Design vs Tactical Design

```
┌───────────────────────────────────────────────────────────────────┐
│                     DOMAIN-DRIVEN DESIGN                          │
├───────────────────────────────────────────────────────────────────┤
│                                                                   │
│  ┌──────────────────────────────┐   ┌──────────────────────────┐  │
│  │   STRATEGIC DESIGN           │   │   TACTICAL DESIGN        │  │
│  │   (High-level decomposition) │   │   (Implementation)       │  │
│  ├──────────────────────────────┤   ├──────────────────────────┤  │
│  │                              │   │                          │  │
│  │  • Bounded Contexts          │   │  • Entities              │  │
│  │  • Context Mapping           │   │  • Value Objects         │  │
│  │  • Subdomains                │   │  • Aggregates            │  │
│  │    - Core Domain             │   │  • Repositories          │  │
│  │    - Supporting Domain       │   │  • Domain Services       │  │
│  │    - Generic Domain          │   │  • Factories             │  │
│  │  • Integration Patterns      │   │  • Domain Events         │  │
│  │    - Shared Kernel           │   │                          │  │
│  │    - Customer/Supplier       │   │                          │  │
│  │    - Anti-Corruption Layer   │   │                          │  │
│  │    - Open Host Service       │   │                          │  │
│  │                              │   │                          │  │
│  └──────────────────────────────┘   └──────────────────────────┘  │
│                                                                   │
└───────────────────────────────────────────────────────────────────┘
```

**Strategic Design** - High-level domain decomposition:
- Bounded Contexts: Explicit boundaries for domain models
- Context Mapping: Integration patterns between contexts
- Subdomains: Core, Supporting, and Generic classification
- Domain Events: Significant business occurrences
- Integration patterns: How contexts communicate

**Tactical Design** - Implementation patterns within a bounded context:
- Entities: Objects with identity and lifecycle
- Value Objects: Immutable descriptive objects
- Aggregates: Consistency boundaries with root entities
- Repositories: Persistence abstraction for aggregates
- Domain Services: Stateless operations on domain objects
- Factories: Complex object creation
- Domain Events: Implementation for decoupling

---

## How to Apply DDD

### Step 1: Strategic Design First

Start with high-level domain decomposition before diving into code.

**1.1 Identify Subdomains**
```
┌────────────────────────────────────────────────────────────────────┐
│                         DOMAIN                                     │
│                    (Entire business)                               │
├────────────────────────────────────────────────────────────────────┤
│                                                                    │
│  CORE DOMAIN (Your competitive advantage - INVEST HERE)            │
│    • Most important to business                                    │
│    • Differentiates you from competitors                           │
│    • Best developers work here                                     │
│                                                                    │
│  SUPPORTING SUBDOMAIN (Supports core but not differentiating)      │
│    • Important but not unique                                      │
│    • Can be custom or outsourced                                   │
│                                                                    │
│  GENERIC SUBDOMAIN (Common across businesses - BUY DON'T BUILD)    │
│    • Not specific to your business                                 │
│    • Buy off-the-shelf or use open source                          │
│    • Examples: Auth, Email, Logging, Payment processing            │
│                                                                    │
└────────────────────────────────────────────────────────────────────┘
```

**1.2 Define Bounded Contexts**

Each bounded context has:
- Its own ubiquitous language
- Clear boundaries with explicit interfaces
- Single unified model within the context
- Autonomous team ownership (ideally)

Example: In e-commerce, "Customer" means different things in Sales, Shipping, and Billing contexts.

**1.3 Map Context Relationships**

Choose integration patterns:
- **Shared Kernel**: Share small subset of code (tight coupling)
- **Customer-Supplier**: Downstream depends on upstream API
- **Conformist**: Downstream accepts upstream model as-is
- **Anti-Corruption Layer**: Translate to protect your model
- **Open Host Service**: Well-documented API for multiple consumers

See [Strategic Patterns](references/strategic-patterns.md) for detailed guidance.

---

### Step 2: Tactical Design Implementation

Once contexts are defined, implement the domain model.

**2.1 Identify Entities vs Value Objects**

```
Entity (Identity-based):
  ✓ Has unique identity (ID)
  ✓ Mutable
  ✓ Tracked over time
  Example: Order, Customer, Account

Value Object (Attribute-based):
  ✓ No identity
  ✓ Immutable
  ✓ Interchangeable
  Example: Money, Address, EmailAddress
```

**2.2 Design Aggregates**

Aggregates are consistency boundaries:
- One aggregate root (entity) as entry point
- Root enforces invariants
- External objects reference root by ID only
- Save/load entire aggregate atomically

```
Order Aggregate
╔═══════════════════════════════════╗
║  Order (Root)                     ║ ← Only public interface
║  + addItem()                      ║
║  + submit()                       ║
╚═══════════════════════════════════╝
         │
         │ owns
         ▼
┌───────────────────┐
│ OrderItem[]       │ ← Not accessible outside
└───────────────────┘
```

**Key Rule**: Keep aggregates small. Only include what's needed to enforce invariants.

**2.3 Define Repositories**

One repository per aggregate root:
```
interface OrderRepository {
  + findById(orderId): Order?
  + findByCustomer(customerId): Order[]
  + save(order: Order): void
  + delete(order: Order): void
}
```

**2.4 Identify Domain Services**

Use domain services when:
- Operation spans multiple aggregates
- Logic doesn't naturally belong to any entity
- Operation is stateless

```
✅ GOOD: Multi-aggregate operation
class OrderPricingService {
  calculateFinalPrice(order, customer) {
    // Needs both aggregates
    base = order.calculateTotal()
    discount = customer.getDiscount(base)
    return base - discount + tax
  }
}

❌ BAD: Single aggregate operation
class OrderService {
  calculateTotal(order) {
    // Should be order.calculateTotal()
  }
}
```

**2.5 Model Domain Events**

Events represent significant business occurrences:
- Named in past tense (OrderSubmitted, PaymentProcessed)
- Immutable
- Enable decoupling between contexts
- Support eventual consistency

See [Tactical Patterns](references/tactical-patterns.md) for detailed patterns and diagrams.

---

## DDD Workflow Summary

### Strategic Design Workflow

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

### Tactical Design Workflow

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

## Question Strategy

When working with users on DDD modeling, ask these questions:

### For Strategic Design:
1. "What are the main business capabilities or areas of your domain?"
2. "Which capabilities are core to your competitive advantage?"
3. "What teams will work on different parts of the system?"
4. "Where do concepts have different meanings in different contexts?"
5. "How should different contexts integrate with each other?"
6. "What events are significant to the business?"

### For Tactical Design:
1. "Which objects need to be tracked over time? (Entities)"
2. "Which objects are interchangeable and have no identity? (Value Objects)"
3. "What invariants must be enforced within a transaction? (Aggregates)"
4. "What operations don't naturally belong to an entity? (Domain Services)"
5. "What are the business rules for this concept?"
6. "What significant things happen in the domain? (Domain Events)"

### For Refactoring:
1. "Is this an anemic domain model (just data, no behavior)?"
2. "Are business rules scattered across services or in the database?"
3. "Are there transaction scripts that should be domain logic?"
4. "Is domain logic leaking into application or infrastructure layers?"

---

## Best Practices

1. **Start with Strategic Design**: Define bounded contexts before diving into tactical patterns
2. **Ubiquitous Language First**: Establish shared vocabulary reflecting the business domain
3. **Keep Aggregates Small**: Only include what's needed to enforce invariants
4. **Make Value Objects Immutable**: Prevents bugs and simplifies reasoning
5. **Reference Aggregates by ID**: Avoid direct references between aggregates
6. **Use Domain Events for Decoupling**: Enable eventual consistency between contexts
7. **Avoid Anemic Domain Models**: Put business logic in domain objects, not services
8. **Protect Aggregate Boundaries**: Only modify aggregate through its root
9. **Design for Testability**: Domain logic should be easy to test without infrastructure
10. **Iterate**: DDD is discovery process - model evolves as you learn

---

## Common Pitfalls

❌ **Anemic Domain Model**
- Entities are just data holders
- Business logic in services
- Solution: Move behavior into domain objects

❌ **Large Aggregates**
- Including too much in one aggregate
- Performance and concurrency issues
- Solution: Split into multiple aggregates

❌ **Ignoring Bounded Contexts**
- One model for entire system
- Model becomes corrupted
- Solution: Define explicit context boundaries

❌ **Entity/Value Object Confusion**
- Making everything an entity
- Missing immutability benefits
- Solution: Identify objects without identity as value objects

❌ **Weak Invariant Enforcement**
- Setting fields directly
- Bypassing business logic
- Solution: Use methods, make fields private

❌ **Repository Overuse**
- Repository for every entity
- Breaking aggregate boundaries
- Solution: One repository per aggregate root

---

## Detailed References

For comprehensive details, diagrams, and examples, see:

- **[Strategic Patterns](references/strategic-patterns.md)**: Bounded contexts, context mapping, subdomains, integration patterns
- **[Tactical Patterns](references/tactical-patterns.md)**: Entities, value objects, aggregates, repositories, domain services, factories, domain events
- **[Examples](references/examples.md)**: Complete case studies including e-commerce, banking, inventory, and healthcare systems

---

## Integration with Other Skills

- **Software Architecture**: DDD informs architectural decisions (microservices boundaries = bounded contexts)
- **SOLID Design**: DDD entities and services follow SOLID principles
- **API Design**: Bounded contexts define API boundaries
- **Testing**: Aggregates and value objects are highly testable
- **Event-Driven Architecture**: Domain events enable event-driven systems

---

## Important Notes

1. **DDD is About Learning**: The model evolves as you understand the domain better
2. **Domain Knowledge**: Understanding the business domain deeply is essential for success
3. **Not All Projects Need DDD**: Use for complex domains, not CRUD apps
4. **Start Small**: Apply DDD to core domain first, not entire system
5. **Ubiquitous Language**: Most important concept - shared understanding between technical and business
6. **Language-Agnostic Patterns**: All examples use pseudo-code applicable to any programming language

---

## References

Based on:
- "Domain-Driven Design: Tackling Complexity in the Heart of Software" by Eric Evans
- "Implementing Domain-Driven Design" by Vaughn Vernon
- "Domain-Driven Design Distilled" by Vaughn Vernon
