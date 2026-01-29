---
name: solid-design
description: Systematically applies SOLID principles (Single Responsibility, Open/Closed, Liskov Substitution, Interface Segregation, Dependency Inversion) to software design and code analysis. Use when designing maintainable features, analyzing code for SOLID violations, refactoring for better separation of concerns, improving testability, reducing coupling, or ensuring clean architecture. Identifies code smells, proposes refactoring strategies, and documents design decisions with SOLID justifications.
allowed-tools: Read Write Edit AskUserQuestion Glob Grep
---

# SOLID Design Skill

This skill systematically applies SOLID principles during software design and development. Use it to design maintainable, extensible, and testable code.

## When to Use This Skill

Use this skill when you need to:
- Design new features or components following SOLID principles
- Analyze existing code for SOLID principle violations
- Refactor code to improve maintainability and extensibility
- Review architecture or code for adherence to clean design principles
- Explain SOLID principles with concrete examples
- Document design decisions with SOLID justifications
- Identify technical debt related to poor separation of concerns

## Purpose

This skill provides a systematic approach to applying SOLID principles:

1. **Single Responsibility Principle (SRP)**: A class should have one, and only one, reason to change
2. **Open/Closed Principle (OCP)**: Software entities should be open for extension but closed for modification
3. **Liskov Substitution Principle (LSP)**: Derived classes must be substitutable for their base classes
4. **Interface Segregation Principle (ISP)**: Clients should not be forced to depend on interfaces they don't use
5. **Dependency Inversion Principle (DIP)**: Depend on abstractions, not concretions

## Understanding SOLID Principles

### Single Responsibility Principle (SRP)

**Definition**: A class should have one, and only one, reason to change. Each class should have a single, well-defined responsibility.

**Signs of Violation**:
- Classes with multiple unrelated methods
- Classes that change for different reasons (UI changes, business logic changes, data access changes)
- "God classes" that do too much
- Classes with names like "Manager", "Helper", "Utility" without specific context

**How to Apply**:
- Identify distinct responsibilities within a class
- Extract each responsibility into its own class
- Use composition to combine responsibilities when needed
- Name classes after their single responsibility

**Example**:
```
❌ Violation:
class UserManager {
  validateUser(user)      // Validation responsibility
  saveUser(user)          // Persistence responsibility
  sendEmail(user)         // Notification responsibility
  generateReport(user)    // Reporting responsibility
}

✅ Correct:
class UserValidator {
  validate(user)
}
class UserRepository {
  save(user)
}
class UserNotificationService {
  sendWelcomeEmail(user)
}
class UserReportGenerator {
  generate(user)
}
```

### Open/Closed Principle (OCP)

**Definition**: Software entities should be open for extension but closed for modification. You should be able to add new functionality without changing existing code.

**Signs of Violation**:
- Long if/else or switch statements that grow with new cases
- Modifying existing classes to add new behavior
- Tight coupling to concrete implementations
- No abstraction layer for variation points

**How to Apply**:
- Identify variation points in your design
- Create abstractions (interfaces/abstract classes) for these points
- Use polymorphism, strategy pattern, or plugin architectures
- Rely on dependency injection to provide different implementations

**Example**:
```
❌ Violation:
class ReportGenerator {
  generate(type) {
    if (type === 'PDF') {
      // PDF generation logic
    } else if (type === 'Excel') {
      // Excel generation logic
    } else if (type === 'CSV') {
      // CSV generation logic - requires modifying this class
    }
  }
}

✅ Correct:
interface ReportFormatter {
  format(data): Report
}
class PdfFormatter implements ReportFormatter { ... }
class ExcelFormatter implements ReportFormatter { ... }
class CsvFormatter implements ReportFormatter { ... }

class ReportGenerator {
  constructor(private formatter: ReportFormatter) {}
  generate(data) {
    return formatter.format(data)
  }
}
```

### Liskov Substitution Principle (LSP)

**Definition**: Objects of a superclass should be replaceable with objects of its subclasses without breaking the application. Subtypes must be behaviorally compatible with their base types.

**Signs of Violation**:
- Subclasses throwing exceptions on inherited methods
- Subclasses returning different types than parent
- Subclasses that weaken preconditions or strengthen postconditions
- Code that checks the type before using a polymorphic object

**How to Apply**:
- Ensure subclasses honor the contract of their parent
- Don't throw exceptions in overridden methods if parent doesn't
- Maintain or strengthen postconditions (what the method guarantees)
- Maintain or weaken preconditions (what the method requires)
- Use composition over inheritance when behavior differs significantly

**Example**:
```
❌ Violation:
class Rectangle {
  setWidth(w) { this.width = w }
  setHeight(h) { this.height = h }
  getArea() { return this.width * this.height }
}
class Square extends Rectangle {
  setWidth(w) { this.width = w; this.height = w }  // Violates LSP
  setHeight(h) { this.width = h; this.height = h }  // Violates LSP
}
// rect.setWidth(5); rect.setHeight(10); assert(rect.getArea() === 50)
// This fails if rect is a Square (area would be 100)

✅ Correct:
interface Shape {
  getArea(): number
}
class Rectangle implements Shape {
  constructor(private width, private height) {}
  getArea() { return this.width * this.height }
}
class Square implements Shape {
  constructor(private side) {}
  getArea() { return this.side * this.side }
}
```

### Interface Segregation Principle (ISP)

**Definition**: Clients should not be forced to depend on interfaces they don't use. Create fine-grained, client-specific interfaces rather than monolithic ones.

**Signs of Violation**:
- Fat interfaces with many methods
- Classes implementing interfaces but leaving methods empty or throwing "not supported"
- Clients depending on methods they never call
- Interface with unrelated method groups

**How to Apply**:
- Split large interfaces into smaller, focused ones
- Group methods by client needs, not by object capabilities
- Use role interfaces (what clients need) not header interfaces (what object has)
- Apply Interface Segregation at all levels (class interfaces, service APIs, modules)

**Example**:
```
❌ Violation:
interface Worker {
  work()
  eat()
  sleep()
}
class Robot implements Worker {
  work() { ... }
  eat() { throw new Error("Robots don't eat") }
  sleep() { throw new Error("Robots don't sleep") }
}

✅ Correct:
interface Workable {
  work()
}
interface Eatable {
  eat()
}
interface Sleepable {
  sleep()
}
class Human implements Workable, Eatable, Sleepable { ... }
class Robot implements Workable { ... }
```

### Dependency Inversion Principle (DIP)

**Definition**: High-level modules should not depend on low-level modules. Both should depend on abstractions. Abstractions should not depend on details; details should depend on abstractions.

**Signs of Violation**:
- High-level business logic directly instantiating low-level classes
- Concrete dependencies instead of interface dependencies
- No abstraction layer between layers
- Tight coupling to frameworks, databases, or external services

**How to Apply**:
- Define interfaces in high-level modules
- Have low-level modules implement these interfaces
- Use dependency injection to provide implementations
- Depend on abstractions (interfaces) in all layers
- Invert the dependency direction through abstraction

**Example**:
```
❌ Violation:
class OrderProcessor {
  private db = new MySqlDatabase()  // Concrete dependency
  private email = new SmtpEmailService()  // Concrete dependency

  processOrder(order) {
    db.save(order)
    email.send(order.customerEmail, "Order confirmed")
  }
}

✅ Correct:
interface OrderRepository {
  save(order)
}
interface EmailService {
  send(to, message)
}
class OrderProcessor {
  constructor(
    private repository: OrderRepository,
    private emailService: EmailService
  ) {}

  processOrder(order) {
    repository.save(order)
    emailService.send(order.customerEmail, "Order confirmed")
  }
}
// Inject implementations:
new OrderProcessor(new MySqlOrderRepository(), new SmtpEmailService())
```

## Workflow

### Scenario 1: Analyzing Existing Code

1. **Identify the scope**: Which class, module, or component to analyze
2. **Read the code**: Use Read/Grep tools to understand current implementation
3. **Evaluate against SOLID principles**: Analyze code against each principle
4. **Document violations**: Create detailed analysis of violations found
5. **Prioritize issues**: Identify high-impact violations
6. **Propose refactoring**: Suggest concrete improvements
7. **Create design document**: Document refactoring decisions

### Scenario 2: Designing New Features

1. **Understand requirements**: Gather functional and non-functional requirements
2. **Identify responsibilities**: Break down into single-responsibility components
3. **Define abstractions**: Create interfaces for variation points (OCP, DIP)
4. **Design contracts**: Ensure substitutability (LSP)
5. **Segregate interfaces**: Create client-specific interfaces (ISP)
6. **Apply dependency injection**: Wire dependencies through abstractions (DIP)
7. **Document design**: Use the SOLID design template
8. **Review against principles**: Validate design against SOLID principles

## Question Strategy

When working with users on SOLID design, ask these questions:

### For Analysis Tasks:
1. "Which file/class/module would you like me to analyze for SOLID violations?"
2. "What are the main pain points you're experiencing with this code? (hard to test, frequent bugs, difficult to extend)"
3. "Are there specific SOLID principles you're most concerned about?"
4. "What's your priority: immediate refactoring or documentation of issues?"

### For Design Tasks:
1. "What is the main responsibility of this component/class?"
2. "What aspects of this design are likely to change in the future?"
3. "What different implementations or variations do you anticipate?"
4. "Who are the clients/consumers of this component?"
5. "What dependencies does this component need?"
6. "Are there any existing abstractions or interfaces I should use?"
7. "What testing strategy should inform the design?"

### For Refactoring Tasks:
1. "What triggered the need for refactoring? (adding feature, fixing bugs, improving tests)"
2. "Do you want me to refactor incrementally or propose a comprehensive redesign?"
3. "Are there any constraints? (backward compatibility, performance, deployment)"
4. "Should I create new files or modify existing ones?"

## Usage Examples

### Example 1: Analyze a Class for Violations

**User**: "Analyze UserService.java for SOLID violations"

**Workflow**:
1. Read the file to understand current implementation
2. Evaluate against SOLID principles
3. Identify violations with specific examples
4. Generate solid-analysis.md documenting findings
5. Propose refactoring approach

### Example 2: Design New Feature with SOLID

**User**: "Design a payment processing system following SOLID principles"

**Workflow**:
1. Ask clarifying questions about requirements and variation points
2. Identify core responsibilities (payment validation, processing, notification)
3. Define abstractions (PaymentGateway interface, PaymentValidator interface)
4. Design component structure following SRP
5. Apply DIP for external dependencies (gateways, databases)
6. Use ISP for different client needs (admin vs customer)
7. Document design in solid-design.md

### Example 3: Refactor Existing Code

**User**: "Refactor OrderManager class to follow SOLID principles"

**Workflow**:
1. Analyze current OrderManager for violations
2. Identify multiple responsibilities (validation, persistence, notification)
3. Propose extraction into separate classes
4. Create interfaces for abstractions
5. Implement dependency injection
6. Document refactoring decisions
7. Provide before/after comparison

## Templates

This skill provides the **solid-design-template.md** template in the `templates/` directory for documenting SOLID-compliant designs.

## Integration with Other Skills

- **Code Review**: Apply SOLID principles during code reviews
- **Refactoring**: Guide refactoring efforts with SOLID principles
- **Testing**: Design testable code using DIP and ISP
- **Documentation**: Document design rationale with SOLID principles

## Best Practices

1. **Start with SRP**: It's the foundation - get responsibilities right first
2. **Identify variation points**: Look for where requirements might change (OCP)
3. **Design abstractions early**: Create interfaces before implementations (DIP)
4. **Keep interfaces small**: Resist the urge to create one big interface (ISP)
5. **Test substitutability**: Write tests that verify LSP compliance
6. **Iterate**: SOLID design is iterative - refine as you learn
7. **Balance pragmatism**: Don't over-engineer - apply SOLID where it adds value
8. **Document decisions**: Explain why you chose specific SOLID approaches

## Common Pitfalls to Avoid

- **Over-abstraction**: Not everything needs an interface - apply SOLID where variability exists
- **Premature optimization**: Design for current requirements, refactor when variation emerges
- **Inheritance abuse**: Prefer composition over inheritance to avoid LSP violations
- **Interface explosion**: Don't create interfaces for every class - only for variation points
- **Ignoring context**: SOLID applies differently to different domains (business logic vs utilities)

## Important Notes

1. **Context Matters** - Apply SOLID where it adds value, not dogmatically
2. **Iterative Process** - SOLID design emerges through refactoring and learning
3. **Documentation** - Use SOLID vocabulary to document design decisions
4. **Testing First** - SOLID principles naturally emerge from writing testable code
5. **Documentation** - Always document why SOLID principles were applied
