# Pattern Relationships and Selection Guide

## Pattern Relationships

### Common Combinations:
- **Abstract Factory + Factory Method**: Factory methods implement operations in Abstract Factory
- **Composite + Iterator**: Iterate over Composite structures
- **Composite + Visitor**: Apply operations over Composite structures
- **Decorator + Strategy**: Decorator changes object's skin, Strategy changes its guts
- **Facade + Singleton**: Facade object often a Singleton
- **Observer + Mediator**: Mediator can use Observer to notify colleagues
- **Strategy + Template Method**: Strategy uses delegation, Template Method uses inheritance

## Pattern Selection Guide

### Creating Objects:
- Single instance needed → **Singleton**
- Defer to subclass → **Factory Method**
- Family of products → **Abstract Factory**
- Complex construction → **Builder**
- Copy existing → **Prototype**

### Structuring Code:
- Interface mismatch → **Adapter**
- Separate abstraction/implementation → **Bridge**
- Part-whole hierarchy → **Composite**
- Add responsibilities → **Decorator**
- Simplify interface → **Facade**
- Share objects → **Flyweight**
- Control access → **Proxy**

### Defining Behavior:
- Chain of handlers → **Chain of Responsibility**
- Encapsulate request → **Command**
- Language grammar → **Interpreter**
- Traverse collection → **Iterator**
- Centralize interactions → **Mediator**
- Capture state → **Memento**
- Notify dependents → **Observer**
- State-dependent behavior → **State**
- Interchangeable algorithms → **Strategy**
- Algorithm skeleton → **Template Method**
- Operations on structure → **Visitor**

## Usage Guidelines

When learning or teaching patterns:

1. **Start with Problem**: Understand the design problem before the pattern
2. **Study Structure**: Analyze the diagrams to understand relationships
3. **Apply Incrementally**: Don't over-engineer; use patterns when appropriate
4. **Refactor to Patterns**: Often better to refactor toward patterns than design with them initially
5. **Combine Wisely**: Patterns work together but can create complexity
6. **Know Trade-offs**: Each pattern has benefits and costs

## Anti-Patterns to Avoid

- **Pattern Overload**: Using patterns when simple code would suffice
- **Golden Hammer**: Forcing favorite pattern into inappropriate situations
- **Premature Patterns**: Applying patterns before understanding requirements
- **Pattern Spaghetti**: Combining too many patterns creating complexity
