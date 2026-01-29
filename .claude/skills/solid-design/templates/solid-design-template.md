# SOLID Design Document

## Component: [Component/Feature Name]

**Date**: [YYYY-MM-DD]
**Author**: [Your Name]
**Status**: [Draft | In Review | Approved]

---

## Overview

### Purpose
[Brief description of what this component does and why it exists]

### Scope
[What this component is responsible for and what it is NOT responsible for]

---

## SOLID Principles Application

### Single Responsibility Principle (SRP)

**Primary Responsibility**: [One clear responsibility]

**Rationale**:
[Explain why this is the single responsibility and how it avoids multiple reasons to change]

**What This Component Does NOT Do**:
- [Responsibility delegated to component X]
- [Responsibility delegated to component Y]

---

### Open/Closed Principle (OCP)

**Extension Points**:
[Identify where this design allows extension without modification]

**Abstractions Used**:
- **Interface/Abstract Class**: `[Name]`
  - **Purpose**: [Why this abstraction exists]
  - **Implementations**: [List current or planned implementations]

**Future Extensions**:
[Describe how new functionality can be added without modifying existing code]

---

### Liskov Substitution Principle (LSP)

**Inheritance Hierarchy** (if applicable):
```
[BaseClass/Interface]
├── [Implementation1]
├── [Implementation2]
└── [Implementation3]
```

**Behavioral Contract**:
- **Preconditions**: [What must be true before calling methods]
- **Postconditions**: [What is guaranteed after calling methods]
- **Invariants**: [What remains true throughout object lifetime]

**Substitutability Guarantee**:
[Explain how any subtype can replace the base type without breaking clients]

---

### Interface Segregation Principle (ISP)

**Interfaces Defined**:

1. **Interface**: `[InterfaceName]`
   - **Client**: [Who uses this interface]
   - **Methods**:
     - `[method1()]`: [Purpose]
     - `[method2()]`: [Purpose]
   - **Rationale**: [Why this interface is client-specific]

2. **Interface**: `[AnotherInterfaceName]`
   - **Client**: [Who uses this interface]
   - **Methods**:
     - `[method1()]`: [Purpose]
   - **Rationale**: [Why this interface is separate]

**Why Not a Single Interface**:
[Explain why interfaces are segregated instead of combined]

---

### Dependency Inversion Principle (DIP)

**Abstractions Depended Upon**:

| Dependency | Abstraction | Concrete Implementation | Injection Method |
|------------|-------------|------------------------|------------------|
| [Database] | `Repository` interface | `SqlRepository` | Constructor injection |
| [Email] | `NotificationService` interface | `SmtpNotificationService` | Constructor injection |

**Dependency Flow**:
```
[High-level module] → depends on → [Abstraction] ← implements ← [Low-level module]
```

**Rationale**:
[Explain why dependencies are inverted and how it improves testability/flexibility]

---

## Component Structure

### Class Diagram
```
[Provide a simple ASCII or markdown class diagram showing main classes and relationships]
```

### Key Classes/Interfaces

#### [ClassName1]
- **Responsibility**: [Single responsibility]
- **Dependencies**: [List interfaces it depends on]
- **Implements**: [Interface(s) if applicable]
- **Key Methods**:
  - `method1()`: [Purpose]
  - `method2()`: [Purpose]

#### [ClassName2]
- **Responsibility**: [Single responsibility]
- **Dependencies**: [List interfaces it depends on]
- **Implements**: [Interface(s) if applicable]

---

## Usage Example

### Construction and Injection
```[language]
// Example of how components are instantiated and wired together
[Code example showing dependency injection]
```

### Typical Usage
```[language]
// Example of how clients use this component
[Code example showing typical usage]
```

### Extension Example
```[language]
// Example of how to extend this design (OCP)
[Code example showing how to add new functionality]
```

---

## Design Decisions

### Decision 1: [Title]
**Context**: [What problem or requirement drove this decision]
**Decision**: [What was decided]
**SOLID Principle**: [Which principle(s) this supports]
**Rationale**: [Why this decision was made]
**Consequences**: [Trade-offs and implications]

### Decision 2: [Title]
**Context**: [What problem or requirement drove this decision]
**Decision**: [What was decided]
**SOLID Principle**: [Which principle(s) this supports]
**Rationale**: [Why this decision was made]
**Consequences**: [Trade-offs and implications]

---

## Testing Strategy

### Unit Testing
[How SOLID design enables unit testing]
- [Which components can be tested in isolation]
- [Which dependencies can be mocked/stubbed]

### Integration Testing
[How components are tested together]

### Test Doubles
[Which interfaces allow test doubles to be injected]

---

## Future Considerations

### Anticipated Changes
[What changes are expected and how the design accommodates them]

### Extension Scenarios
[Concrete examples of how this design can be extended]

### Potential Refactoring
[Areas that might need refactoring as requirements evolve]

---

## References

- [Link to requirements]
- [Link to external resources]
