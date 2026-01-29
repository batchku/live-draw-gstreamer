# DDD Examples and Case Studies

This reference provides comprehensive examples of applying Domain-Driven Design patterns to real-world scenarios.

## E-Commerce Order Management System

A comprehensive example showing both strategic and tactical DDD patterns.

### Bounded Contexts

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
```

### Tactical Patterns

**Entities**:
- Order (aggregate root)
- Customer (aggregate root)
- Product (aggregate root in catalog context)
- OrderItem (within Order aggregate)

**Value Objects**:
```
Money {
  amount: decimal
  currency: string (ISO 4217 code)

  + add(other: Money): Money
  + multiply(factor: decimal): Money
  + isGreaterThan(other: Money): boolean
}

Address {
  street: string
  city: string
  state: string
  postalCode: string
  country: string

  + toString(): string
  + isInternational(): boolean
}

EmailAddress {
  value: string

  + isValid(): boolean
  + domain(): string
}

OrderId {
  value: UUID

  + toString(): string
}
```

**Aggregates**:

```
Order Aggregate (Root: Order)
┌──────────────────────────────────────┐
│ Order                                │
│ ─────                                │
│ + id: OrderId                        │
│ + customerId: CustomerId             │
│ + items: OrderItem[]                 │
│ + status: OrderStatus                │
│ + shippingAddress: Address           │
│ + total: Money                       │
│                                      │
│ PUBLIC METHODS:                      │
│ + addItem(product, quantity, price)  │
│ + removeItem(itemId)                 │
│ + updateShippingAddress(address)     │
│ + submit()                           │
│ + cancel()                           │
│ + calculateTotal(): Money            │
│                                      │
│ INVARIANTS:                          │
│ • Must have at least 1 item          │
│ • Total must be > $10                │
│ • Max 10 items per order             │
│ • Submitted orders cannot be changed │
│ • Must have shipping address         │
└──────────────────────────────────────┘
         │
         │ owns
         ▼
┌──────────────────────┐
│ OrderItem            │
│ ─────────            │
│ + id: local ID       │
│ + productId: string  │
│ + productName: string│
│ + quantity: int      │
│ + unitPrice: Money   │
│                      │
│ + subtotal(): Money  │
└──────────────────────┘

Customer Aggregate (Root: Customer)
┌──────────────────────────────────────┐
│ Customer                             │
│ ────────                             │
│ + id: CustomerId                     │
│ + name: FullName                     │
│ + email: EmailAddress                │
│ + tier: CustomerTier                 │
│ + shippingAddresses: Address[]       │
│                                      │
│ PUBLIC METHODS:                      │
│ + addShippingAddress(address)        │
│ + removeShippingAddress(addressId)   │
│ + getDiscount(orderTotal): Money     │
│ + upgradeToVIP()                     │
│                                      │
│ INVARIANTS:                          │
│ • Must have valid email              │
│ • Must have at least one address     │
│ • VIP customers get 10% discount     │
└──────────────────────────────────────┘
```

**Repositories**:
```
interface OrderRepository {
  + findById(orderId: OrderId): Order?
  + findByCustomer(customerId: CustomerId): Order[]
  + findPendingOrders(): Order[]
  + findByDateRange(start: Date, end: Date): Order[]
  + save(order: Order): void
  + delete(order: Order): void
}

interface CustomerRepository {
  + findById(customerId: CustomerId): Customer?
  + findByEmail(email: EmailAddress): Customer?
  + findVIPCustomers(): Customer[]
  + save(customer: Customer): void
}
```

**Domain Services**:
```
class OrderPricingService {
  - taxCalculator: TaxCalculator
  - discountPolicy: DiscountPolicy

  + calculateFinalPrice(
      order: Order,
      customer: Customer,
      promotionCode: PromotionCode?
    ): OrderPrice {

    // Get base total from order
    baseTotal = order.calculateTotal()

    // Apply customer discount (tier-based)
    customerDiscount = customer.getDiscount(baseTotal)

    // Apply promotion code if provided
    promoDiscount = promotionCode
      ? applyPromotion(promotionCode, baseTotal)
      : Money.zero()

    // Calculate subtotal after discounts
    subtotal = baseTotal - customerDiscount - promoDiscount

    // Calculate tax
    tax = taxCalculator.calculate(subtotal, order.shippingAddress)

    // Final total
    finalTotal = subtotal + tax

    return OrderPrice {
      baseTotal,
      customerDiscount,
      promoDiscount,
      subtotal,
      tax,
      finalTotal
    }
  }
}
```

**Domain Events**:
```
OrderSubmitted {
  eventId: UUID
  occurredAt: DateTime
  aggregateId: OrderId
  customerId: CustomerId
  orderTotal: Money
  itemsCount: int
  shippingAddress: Address
}

OrderCancelled {
  eventId: UUID
  occurredAt: DateTime
  aggregateId: OrderId
  reason: string
}

PaymentProcessed {
  eventId: UUID
  occurredAt: DateTime
  orderId: OrderId
  paymentId: PaymentId
  amount: Money
  paymentMethod: string
}
```

**Event Handlers**:
```
class OrderSubmittedHandler {
  - inventoryService: InventoryService
  - shippingService: ShippingService
  - emailService: EmailService

  + handle(event: OrderSubmitted) {
    // Reserve inventory
    inventoryService.reserve(
      event.aggregateId,
      event.itemsCount
    )

    // Create shipment
    shippingService.createShipment(
      event.aggregateId,
      event.shippingAddress
    )

    // Send confirmation email
    emailService.sendOrderConfirmation(
      event.customerId,
      event.aggregateId
    )
  }
}
```

---

## Banking System Example

Demonstrates money transfers between accounts using domain services.

### Bounded Contexts

```
┌─────────────────────┐       ┌─────────────────────┐
│  Account Context    │       │  Transaction        │
├─────────────────────┤       │  Context            │
│ Account             │       ├─────────────────────┤
│ AccountNumber       │       │ TransactionLog      │
│ Balance             │       │ TransactionHistory  │
│ AccountHolder       │       │ AuditTrail          │
└─────────────────────┘       └─────────────────────┘
           │                           │
           │  MoneyTransferred Event   │
           └───────────────────────────┘
```

### Tactical Patterns

**Entities**:
```
Account (Aggregate Root)
┌──────────────────────────────────────┐
│ Account                              │
│ ───────                              │
│ + id: AccountId                      │
│ + accountNumber: AccountNumber       │
│ + balance: Money                     │
│ + holder: AccountHolder              │
│ + status: AccountStatus              │
│ + transactions: Transaction[]        │
│                                      │
│ PUBLIC METHODS:                      │
│ + deposit(amount: Money)             │
│ + withdraw(amount: Money)            │
│ + freeze()                           │
│ + close()                            │
│                                      │
│ INVARIANTS:                          │
│ • Balance cannot be negative         │
│ • Frozen accounts cannot transact    │
│ • Closed accounts cannot be reopened │
└──────────────────────────────────────┘
         │
         │ owns
         ▼
┌──────────────────────┐
│ Transaction          │
│ ───────────          │
│ + id: local ID       │
│ + timestamp: DateTime│
│ + type: TxType       │
│ + amount: Money      │
│ + description: string│
└──────────────────────┘
```

**Value Objects**:
```
Money {
  amount: decimal
  currency: Currency

  + add(other: Money): Money
  + subtract(other: Money): Money
  + isGreaterThan(other: Money): boolean
  + isNegative(): boolean
}

AccountNumber {
  value: string

  + isValid(): boolean
  + format(): string  // "XXXX-XXXX-XXXX-1234"
}

AccountHolder {
  firstName: string
  lastName: string
  taxId: string

  + fullName(): string
}
```

**Domain Service**:
```
class MoneyTransferService {
  - accountRepository: AccountRepository
  - eventBus: EventBus

  + transfer(
      fromAccountId: AccountId,
      toAccountId: AccountId,
      amount: Money
    ) {

    // Load both accounts
    fromAccount = accountRepository.findById(fromAccountId)
    toAccount = accountRepository.findById(toAccountId)

    // Validate transfer
    if (!fromAccount || !toAccount) {
      throw AccountNotFoundException()
    }

    if (fromAccount.balance < amount) {
      throw InsufficientFundsException()
    }

    if (fromAccount.status != ACTIVE || toAccount.status != ACTIVE) {
      throw InactiveAccountException()
    }

    // Execute transfer
    fromAccount.withdraw(amount)
    toAccount.deposit(amount)

    // Save both accounts
    accountRepository.save(fromAccount)
    accountRepository.save(toAccount)

    // Publish event
    eventBus.publish(MoneyTransferred {
      fromAccountId,
      toAccountId,
      amount,
      occurredAt: now()
    })
  }
}
```

---

## Inventory Management Example

Shows aggregate size considerations and eventual consistency.

### Aggregates

```
❌ WRONG - Aggregate Too Large:
┌─────────────────────────────────────┐
│ Warehouse Aggregate (TOO LARGE)     │
│ ├─ Warehouse (root)                 │
│ ├─ Location []                      │
│ ├─ Product []  ← Don't include all! │
│ │   ├─ SKU                          │
│ │   ├─ StockLevel                   │
│ │   ├─ ReorderPoint                 │
│ │   └─ Supplier                     │
│ ├─ Employee []                       │
│ └─ Order []                          │
└─────────────────────────────────────┘

✅ CORRECT - Separate Aggregates:
┌──────────────────┐  ┌──────────────────┐  ┌──────────────────┐
│ Warehouse        │  │ Product          │  │ StockReservation │
│ ├─ Warehouse     │  │ ├─ Product       │  │ ├─ Reservation   │
│ └─ Location []   │  │ ├─ SKU           │  │ ├─ ProductId     │
└──────────────────┘  │ └─ Description   │  │ ├─ Quantity      │
                      └──────────────────┘  │ └─ OrderId       │
                                            └──────────────────┘
        ▲                      ▲                      ▲
        └──────────────────────┴──────────────────────┘
              Reference by ID (eventual consistency)
```

**Domain Service**:
```
class InventoryReservationService {
  - productRepository: ProductRepository
  - reservationRepository: ReservationRepository

  + reserve(
      orderId: OrderId,
      productId: ProductId,
      quantity: int
    ) {

    // Load product
    product = productRepository.findById(productId)

    if (!product) {
      throw ProductNotFoundException()
    }

    if (!product.hasAvailableStock(quantity)) {
      throw InsufficientStockException()
    }

    // Create reservation (separate aggregate)
    reservation = new Reservation {
      id: generateId(),
      orderId,
      productId,
      quantity,
      expiresAt: now() + 15.minutes
    }

    // Save reservation
    reservationRepository.save(reservation)

    // Update product stock (eventual consistency)
    product.reserveStock(quantity)
    productRepository.save(product)

    return reservation
  }
}
```

---

## Healthcare System Example

Shows complex domain modeling with multiple aggregates.

### Entities and Aggregates

```
Patient Aggregate (Root: Patient)
┌──────────────────────────────────────┐
│ Patient                              │
│ ───────                              │
│ + id: PatientId                      │
│ + name: PersonName                   │
│ + dateOfBirth: Date                  │
│ + medicalRecordNumber: MRN           │
│ + contactInfo: ContactInfo           │
│                                      │
│ PUBLIC METHODS:                      │
│ + updateContactInfo(info)            │
│ + age(): int                         │
└──────────────────────────────────────┘

Appointment Aggregate (Root: Appointment)
┌──────────────────────────────────────┐
│ Appointment                          │
│ ───────────                          │
│ + id: AppointmentId                  │
│ + patientId: PatientId (reference)   │
│ + providerId: ProviderId (reference) │
│ + scheduledTime: DateTimeRange       │
│ + status: AppointmentStatus          │
│ + reason: string                     │
│                                      │
│ PUBLIC METHODS:                      │
│ + schedule()                         │
│ + reschedule(newTime: DateTimeRange) │
│ + cancel(reason: string)             │
│ + checkIn()                          │
│ + complete()                         │
│                                      │
│ INVARIANTS:                          │
│ • Scheduled time must be in future   │
│ • Cannot reschedule < 24hr before    │
│ • Completed appointments cannot      │
│   be cancelled                       │
└──────────────────────────────────────┘

MedicalRecord Aggregate (Root: MedicalRecord)
┌──────────────────────────────────────┐
│ MedicalRecord                        │
│ ─────────────                        │
│ + id: MedicalRecordId                │
│ + patientId: PatientId (reference)   │
│ + visits: Visit[]                    │
│ + diagnoses: Diagnosis[]             │
│ + medications: Medication[]          │
│                                      │
│ PUBLIC METHODS:                      │
│ + addVisit(visit: Visit)             │
│ + addDiagnosis(diagnosis: Diagnosis) │
│ + prescribeMedication(med)           │
└──────────────────────────────────────┘
```

**Domain Events**:
```
AppointmentScheduled {
  eventId: UUID
  occurredAt: DateTime
  appointmentId: AppointmentId
  patientId: PatientId
  providerId: ProviderId
  scheduledTime: DateTimeRange
}

AppointmentCancelled {
  eventId: UUID
  occurredAt: DateTime
  appointmentId: AppointmentId
  reason: string
  cancelledBy: UserId
}

PatientCheckedIn {
  eventId: UUID
  occurredAt: DateTime
  appointmentId: AppointmentId
  patientId: PatientId
  checkInTime: DateTime
}
```

---

## Common DDD Patterns by Scenario

### Scenario: Task Management System

**Core Domain**: Task workflow and assignment logic

**Entities**:
- Task (aggregate root)
- Project (aggregate root)
- User (aggregate root)

**Value Objects**:
- TaskStatus (enum: TODO, IN_PROGRESS, DONE)
- Priority (enum: LOW, MEDIUM, HIGH, CRITICAL)
- DueDate
- TaskDescription

**Aggregates**:
```
Task Aggregate
├─ Task (root)
├─ Comment[]
├─ Attachment[]
└─ ActivityLog[]

Project Aggregate
├─ Project (root)
└─ Milestone[]
```

**Domain Services**:
- TaskAssignmentService (assigns based on workload)
- TaskPrioritizationService (auto-prioritizes based on rules)

**Domain Events**:
- TaskCreated
- TaskAssigned
- TaskCompleted
- TaskOverdue

---

### Scenario: Subscription Management

**Core Domain**: Subscription lifecycle and billing

**Entities**:
- Subscription (aggregate root)
- Customer (aggregate root)
- Plan (aggregate root)

**Value Objects**:
- BillingPeriod (MONTHLY, YEARLY)
- SubscriptionStatus (TRIAL, ACTIVE, CANCELLED, EXPIRED)
- Money
- PaymentMethod

**Aggregates**:
```
Subscription Aggregate
├─ Subscription (root)
├─ BillingHistory[]
└─ UsageMetrics
```

**Domain Services**:
- SubscriptionRenewalService
- BillingCalculationService
- UpgradeDowngradeService

**Domain Events**:
- SubscriptionStarted
- SubscriptionRenewed
- SubscriptionCancelled
- SubscriptionExpired
- PaymentFailed

---

## Best Practices Illustrated

### 1. Keep Aggregates Small

```
❌ BAD: Large aggregate with performance issues
Order Aggregate (10,000 lines in memory)
├─ Order
├─ OrderItem[] (1000 items)
├─ PaymentHistory[] (500 payments)
├─ ShipmentHistory[] (100 shipments)
└─ ActivityLog[] (5000 entries)

Problem: Loading this aggregate is slow, concurrency conflicts

✅ GOOD: Small, focused aggregates
Order Aggregate
├─ Order
└─ OrderItem[] (max 10 items)

Payment Aggregate (separate)
├─ Payment
└─ PaymentTransaction[]

Shipment Aggregate (separate)
├─ Shipment
└─ TrackingEvent[]
```

### 2. Reference Aggregates by ID

```
✅ CORRECT:
class Order {
  customerId: CustomerId  // Reference by ID

  // Don't hold direct reference:
  // customer: Customer  ← BAD!
}

✅ CORRECT: Use repository to navigate
order = orderRepo.findById(orderId)
customer = customerRepo.findById(order.customerId)
```

### 3. Use Value Objects for Validation

```
✅ GOOD: Encapsulate validation in value object
class EmailAddress {
  value: string

  constructor(email: string) {
    if (!isValidEmail(email)) {
      throw InvalidEmailException()
    }
    this.value = email
  }
}

// Usage:
email = new EmailAddress("user@example.com")  // Validates
customer.updateEmail(email)  // Already validated!

❌ BAD: Validate everywhere
class Customer {
  email: string

  updateEmail(email: string) {
    if (!isValidEmail(email)) {  // Duplicate validation
      throw InvalidEmailException()
    }
    this.email = email
  }
}
```

### 4. Protect Invariants in Aggregate

```
✅ GOOD: Methods enforce invariants
class Order {
  private items: OrderItem[]
  private status: OrderStatus

  addItem(item: OrderItem) {
    if (status == SUBMITTED) {
      throw OrderAlreadySubmittedException()
    }
    if (items.length >= 10) {
      throw TooManyItemsException()
    }
    items.add(item)
  }
}

❌ BAD: Direct field access bypasses validation
order.items.add(item)  // Bypasses validation!
order.status = SUBMITTED  // Bypasses business logic!
```

---

## References and Further Reading

- "Domain-Driven Design: Tackling Complexity in the Heart of Software" by Eric Evans
- "Implementing Domain-Driven Design" by Vaughn Vernon
- "Domain-Driven Design Distilled" by Vaughn Vernon
- "Patterns, Principles, and Practices of Domain-Driven Design" by Scott Millett
