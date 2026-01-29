---
name: design-patterns
description: Comprehensive reference for the 23 Gang of Four (GoF) design patterns. Use when applying Creational patterns (Singleton, Factory, Builder, Prototype), Structural patterns (Adapter, Bridge, Composite, Decorator, Facade, Proxy), or Behavioral patterns (Observer, Strategy, Command, State, Template Method). Covers pattern selection, structure diagrams, implementation guidance, and when to use each pattern for common software design problems.
allowed-tools: Read Glob Grep
---

# Design Patterns Skill

## Description
This skill provides comprehensive reference and teaching material for the 23 Gang of Four (GoF) design patterns. Use this skill when you need to understand, explain, or apply classic object-oriented design patterns in software development.

## When to Use This Skill
- Explaining design patterns for implementation
- Selecting the appropriate pattern for a design problem
- Explaining pattern structures and relationships
- Refactoring code to use established patterns
- Architectural decision-making
- Code review guidance and design analysis

---

# Gang of Four Design Patterns

The 23 GoF patterns are organized into three categories: Creational, Structural, and Behavioral.

---

## Pattern Categories

### Creational Patterns
Creational patterns deal with object creation mechanisms, trying to create objects in a manner suitable to the situation.

**Patterns:**
1. **Singleton** - Ensure a class has only one instance
2. **Factory Method** - Define interface for creating objects, let subclasses decide
3. **Abstract Factory** - Create families of related objects
4. **Builder** - Separate construction from representation
5. **Prototype** - Create objects by copying prototypes

**Detailed Reference:** See [references/creational-patterns.md](references/creational-patterns.md)

---

### Structural Patterns
Structural patterns concern class and object composition, using inheritance to compose interfaces and define ways to compose objects to obtain new functionality.

**Patterns:**
6. **Adapter** - Convert interface to what clients expect
7. **Bridge** - Decouple abstraction from implementation
8. **Composite** - Compose objects into tree structures
9. **Decorator** - Attach additional responsibilities dynamically
10. **Facade** - Provide unified interface to subsystem
11. **Flyweight** - Use sharing to support many objects efficiently
12. **Proxy** - Provide surrogate or placeholder

**Detailed Reference:** See [references/structural-patterns.md](references/structural-patterns.md)

---

### Behavioral Patterns
Behavioral patterns characterize the ways in which classes or objects interact and distribute responsibility.

**Patterns:**
13. **Chain of Responsibility** - Pass request along chain of handlers
14. **Command** - Encapsulate request as object
15. **Interpreter** - Define grammar representation and interpreter
16. **Iterator** - Access elements sequentially without exposing representation
17. **Mediator** - Encapsulate how objects interact
18. **Memento** - Capture and restore object state
19. **Observer** - Define one-to-many dependency for state changes
20. **State** - Alter behavior when internal state changes
21. **Strategy** - Define family of interchangeable algorithms
22. **Template Method** - Define algorithm skeleton, defer steps to subclasses
23. **Visitor** - Define new operations without changing element classes

**Detailed Reference:** See [references/behavioral-patterns.md](references/behavioral-patterns.md)

---

## Quick Pattern Selection

### When Creating Objects:
- Need single instance → **Singleton**
- Defer creation to subclass → **Factory Method**
- Create families of products → **Abstract Factory**
- Complex step-by-step construction → **Builder**
- Clone existing objects → **Prototype**

### When Structuring Code:
- Interface incompatibility → **Adapter**
- Separate interface from implementation → **Bridge**
- Tree structure with uniform treatment → **Composite**
- Add responsibilities dynamically → **Decorator**
- Simplify complex subsystem → **Facade**
- Share many similar objects → **Flyweight**
- Control access to object → **Proxy**

### When Defining Behavior:
- Chain of request handlers → **Chain of Responsibility**
- Encapsulate requests for undo/queue → **Command**
- Parse language or expressions → **Interpreter**
- Traverse collection → **Iterator**
- Centralize object interactions → **Mediator**
- Save/restore object state → **Memento**
- Notify multiple dependents → **Observer**
- Behavior depends on state → **State**
- Interchangeable algorithms → **Strategy**
- Define algorithm template → **Template Method**
- Add operations to structure → **Visitor**

**Detailed Selection Guide:** See [references/pattern-relationships.md](references/pattern-relationships.md)

---

## Pattern Structure Elements

Each pattern in the reference documents includes:

- **Intent**: What the pattern does
- **When to Use**: Situations where the pattern applies
- **Structure**: ASCII diagram showing relationships
- **Key Participants**: Classes/objects and their roles
- **Example Scenario**: Real-world application

---

## Usage Guidelines

### Learning Patterns
1. **Start with Problem**: Understand the design problem first
2. **Study Structure**: Analyze diagrams to understand relationships
3. **Apply Incrementally**: Use patterns when appropriate, don't over-engineer
4. **Refactor to Patterns**: Often better to evolve toward patterns than design with them initially

### Applying Patterns
1. **Identify the Problem**: What design challenge are you facing?
2. **Select Pattern**: Use the quick selection guide above
3. **Study Details**: Read the full pattern specification in references
4. **Adapt to Context**: Modify pattern as needed for your situation
5. **Document Decision**: Explain why you chose this pattern

### Teaching Patterns
1. **Present Problem First**: Show the pain point before the solution
2. **Use Visual Diagrams**: Structure diagrams clarify relationships
3. **Provide Real Examples**: Connect to familiar scenarios
4. **Discuss Trade-offs**: Every pattern has costs and benefits
5. **Show Combinations**: Demonstrate how patterns work together

---

## Common Anti-Patterns to Avoid

- **Pattern Overload**: Using patterns when simple code would suffice
- **Golden Hammer**: Forcing favorite pattern into inappropriate situations
- **Premature Patterns**: Applying patterns before understanding requirements
- **Pattern Spaghetti**: Combining too many patterns creating unnecessary complexity

---

## References

This skill is based on "Design Patterns: Elements of Reusable Object-Oriented Software" by Gamma, Helm, Johnson, and Vlissides (Gang of Four), 1994.

### Detailed Documentation

- **Creational Patterns**: [references/creational-patterns.md](references/creational-patterns.md)
- **Structural Patterns**: [references/structural-patterns.md](references/structural-patterns.md)
- **Behavioral Patterns**: [references/behavioral-patterns.md](references/behavioral-patterns.md)
- **Relationships & Selection**: [references/pattern-relationships.md](references/pattern-relationships.md)

---
