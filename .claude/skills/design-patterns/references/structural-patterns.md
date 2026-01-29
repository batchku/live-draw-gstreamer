# Structural Patterns

Structural patterns concern class and object composition, using inheritance to compose interfaces and define ways to compose objects to obtain new functionality.

---

## 6. Adapter Pattern

**Category**: Structural

**Intent**: Convert the interface of a class into another interface clients expect. Adapter lets classes work together that couldn't otherwise because of incompatible interfaces.

**When to Use**:
- Use an existing class with incompatible interface
- Create reusable class that cooperates with unrelated classes
- Need to use several existing subclasses but impractical to adapt by subclassing

**Structure** (Object Adapter):
```
┌─────────────┐         ┌──────────────┐
│   Client    │────────►│    Target    │
└─────────────┘         ├──────────────┤
                        │ + request()  │
                        └──────△───────┘
                               │
                               │ implements
                        ┌──────┴───────┐
                        │   Adapter    │
                        ├──────────────┤
                        │ - adaptee    │───────┐
                        ├──────────────┤       │
                        │ + request()  │       │
                        └──────────────┘       │ delegates
                                               │
                                               ▼
                                    ┌──────────────────────┐
                                    │   Adaptee            │
                                    ├──────────────────────┤
                                    │ + specificRequest()  │
                                    └──────────────────────┘
```

**Key Participants**:
- **Target**: Domain-specific interface Client uses
- **Adapter**: Adapts Adaptee interface to Target interface
- **Adaptee**: Existing interface needing adaptation
- **Client**: Collaborates with objects conforming to Target interface

**Example Scenario**:
Integrating a third-party payment library into your e-commerce system. Your system expects a PaymentProcessor interface with processPayment() method, but the library provides PayPalAPI with sendPayment(). The PayPalAdapter implements PaymentProcessor and internally calls PayPalAPI.sendPayment().

---

## 7. Bridge Pattern

**Category**: Structural

**Intent**: Decouple an abstraction from its implementation so the two can vary independently.

**When to Use**:
- Avoid permanent binding between abstraction and implementation
- Both abstractions and implementations should be extensible by subclassing
- Changes in implementation shouldn't impact clients
- Share implementation among multiple objects

**Structure**:
```
┌──────────────────┐               ┌───────────────────┐
│   Abstraction    │──────────────►│ Implementor       │
├──────────────────┤   bridge      ├───────────────────┤
│ - implementor    │               │ + operationImpl() │
├──────────────────┤               └────────△──────────┘
│ + operation()    │                        │
└────────△─────────┘                        │
         │                          ┌───────┴────────────┐
         │                          │                    │
┌────────┴───────────┐    ┌─────────┴────────────┐  ┌────┴─────────────────┐
│ RefinedAbstraction │    │ ConcreteImplementorA │  │ ConcreteImplementorB │
├────────────────────┤    ├──────────────────────┤  ├──────────────────────┤
│ + operation()      │    │ + operationImpl()    │  │ + operationImpl()    │
└────────────────────┘    └──────────────────────┘  └──────────────────────┘
```

**Key Participants**:
- **Abstraction**: Defines abstraction interface, maintains Implementor reference
- **RefinedAbstraction**: Extends Abstraction interface
- **Implementor**: Interface for implementation classes
- **ConcreteImplementor**: Implements Implementor interface

**Example Scenario**:
A graphics library supporting multiple rendering engines (OpenGL, DirectX, Vulkan). The Shape abstraction (Circle, Rectangle) is separate from the rendering implementation. Shapes can work with any renderer, and you can add new shapes or renderers independently.

---

## 8. Composite Pattern

**Category**: Structural

**Intent**: Compose objects into tree structures to represent part-whole hierarchies. Composite lets clients treat individual objects and compositions uniformly.

**When to Use**:
- Represent part-whole hierarchies of objects
- Clients should ignore difference between compositions and individual objects
- Structure can have any level of complexity and is dynamic

**Structure**:
```
┌─────────────┐
│   Client    │
└──────┬──────┘
       │
       ▼
┌───────────────┐
│  Component    │◄────────────┐
├───────────────┤             │
│ + operation() │             │
│ + add()       │             │
│ + remove()    │             │
└──────△────────┘             │
       │                      │
       │                      │
   ┌───┴──────┐               │
   │          │               │
┌──┴─────┐ ┌──┴─────────┐     │
│ Leaf   │ │Composite   │     │
├────────┤ ├────────────┤     │
│ + op() │ │ - children │─────┘ (children: Component[])
└────────┘ ├────────────┤
           │ + op()     │
           │ + add()    │
           │ + remove() │
           └────────────┘
```

**Key Participants**:
- **Component**: Interface for objects in composition
- **Leaf**: Leaf object with no children, implements Component
- **Composite**: Component with children, implements child-related operations
- **Client**: Manipulates objects via Component interface

**Example Scenario**:
A file system where folders and files are treated uniformly. Both File (leaf) and Folder (composite) implement the FileSystemComponent interface with methods like getSize(). A Folder contains Files and other Folders, and getSize() returns the sum of all contents.

---

## 9. Decorator Pattern

**Category**: Structural

**Intent**: Attach additional responsibilities to an object dynamically. Decorators provide a flexible alternative to subclassing for extending functionality.

**When to Use**:
- Add responsibilities to individual objects dynamically and transparently
- Responsibilities can be withdrawn
- Extension by subclassing is impractical
- Need many independent extensions

**Structure**:
```
┌─────────────┐
│   Client    │
└──────┬──────┘
       │
       ▼
┌───────────────┐
│  Component    │
├───────────────┤
│ + operation() │
└──────△────────┘
       │
   ┌───┴──────────────────┐
   │                      │
┌──┴────────────┐  ┌──────┴────────┐
│  Concrete     │  │  Decorator    │
│ Component     │  ├───────────────┤
├───────────────┤  │ - component   │──┐ (wraps Component)
│ + operation() │  ├───────────────┤  │
└───────────────┘  │ + operation() │◄─┘
                   └──────△────────┘
                          │
                 ┌────────┴──────────┐
                 │  Concrete         │
                 │  Decorator        │
                 ├───────────────────┤
                 │ + operation()     │
                 │ + addedBehavior() │
                 └───────────────────┘
```

**Key Participants**:
- **Component**: Interface for objects that can have responsibilities added
- **ConcreteComponent**: Object to which responsibilities can be added
- **Decorator**: Maintains reference to Component, conforms to Component interface
- **ConcreteDecorator**: Adds responsibilities to component

**Example Scenario**:
A coffee shop application where beverages can have various add-ons. A basic Coffee is decorated with Milk, Sugar, or Whip. Each decorator adds its cost to the base beverage. You can stack decorators: Coffee with Milk with Sugar with Whip.

---

## 10. Facade Pattern

**Category**: Structural

**Intent**: Provide a unified interface to a set of interfaces in a subsystem. Facade defines a higher-level interface that makes the subsystem easier to use.

**When to Use**:
- Provide simple interface to complex subsystem
- Decouple subsystem from clients and other subsystems
- Layer your subsystems

**Structure**:
```
┌─────────────┐
│   Client    │
└──────┬──────┘
       │
       ▼
┌──────────────────────┐
│      Facade          │
├──────────────────────┤
│ + simpleOperation()  │────┐
└──────────────────────┘    │
                            │ uses
       ┌────────────────────┼────────────────────┐
       ▼                    ▼                    ▼
┌───────────────┐    ┌───────────────┐    ┌───────────────┐
│ SubsystemA    │    │ SubsystemB    │    │ SubsystemC    │
├───────────────┤    ├───────────────┤    ├───────────────┤
│ + operationA1 │    │ + operationB1 │    │ + operationC1 │
│ + operationA2 │    │ + operationB2 │    │ + operationC2 │
└───────────────┘    └───────────────┘    └───────────────┘
```

**Key Participants**:
- **Facade**: Knows which subsystem classes handle requests, delegates to subsystems
- **Subsystem classes**: Implement subsystem functionality, handle work assigned by Facade
- **Client**: Uses Facade instead of subsystem objects directly

**Example Scenario**:
A home theater system with DVD player, projector, amplifier, and lights. Instead of the client calling each component (player.on(), player.load(), projector.on(), amp.setVolume(), lights.dim()), a HomeTheaterFacade provides watchMovie() which coordinates all components.

---

## 11. Flyweight Pattern

**Category**: Structural

**Intent**: Use sharing to support large numbers of fine-grained objects efficiently.

**When to Use**:
- Application uses large number of objects
- Storage costs are high due to quantity of objects
- Most object state can be made extrinsic
- Many groups of objects can be replaced by few shared objects
- Application doesn't depend on object identity

**Structure**:
```
┌─────────────┐
│   Client    │
└──────┬──────┘
       │ extrinsicState
       ▼
┌──────────────────────┐        ┌────────────────────────┐
│ FlyweightFactory     │        │  Flyweight             │
├──────────────────────┤        ├────────────────────────┤
│ - flyweights: Map    │───────►│ + operation(extrinsic) │
├──────────────────────┤        │                        │
│ + getFlyweight(key)  │        └──────△─────────────────┘
└──────────────────────┘               │
                                ┌──────┴────────────┐
                                │                   │
                        ┌───────┴──────────┐  ┌─────┴──────────┐
                        │  Concrete        │  │ Unshared       │
                        │  Flyweight       │  │ Flyweight      │
                        ├──────────────────┤  ├────────────────┤
                        │ - intrinsicState │  │ - allState     │
                        │ + operation()    │  │ + operation()  │
                        └──────────────────┘  └────────────────┘
```

**Key Participants**:
- **Flyweight**: Interface through which flyweights receive and act on extrinsic state
- **ConcreteFlyweight**: Implements Flyweight, stores intrinsic state
- **FlyweightFactory**: Creates and manages flyweight objects, ensures proper sharing
- **Client**: Maintains references to flyweights, computes or stores extrinsic state

**Example Scenario**:
A text editor rendering characters. Instead of creating an object for each character instance, characters share font and style data (intrinsic state), while position and color (extrinsic state) are passed when rendering. The letter 'A' with Arial 12pt is one shared flyweight used hundreds of times.

---

## 12. Proxy Pattern

**Category**: Structural

**Intent**: Provide a surrogate or placeholder for another object to control access to it.

**When to Use**:
- Remote proxy: represent object in different address space
- Virtual proxy: create expensive objects on demand
- Protection proxy: control access to original object
- Smart reference: additional actions when object is accessed

**Structure**:
```
┌─────────────┐
│   Client    │
└──────┬──────┘
       │
       ▼
┌──────────────┐
│   Subject    │
├──────────────┤
│ + request()  │
└──────△───────┘
       │
   ┌───┴───────────────┐
   │                   │
┌──┴──────────┐   ┌────┴──────────┐
│  Real       │◄──│    Proxy      │
│ Subject     │   ├───────────────┤
├─────────────┤   │ - realSubject │
│ + request() │   ├───────────────┤
└─────────────┘   │ + request()   │
                  └───────────────┘
                   (Proxy controls access to RealSubject)
```

**Key Participants**:
- **Subject**: Common interface for RealSubject and Proxy
- **RealSubject**: Real object that proxy represents
- **Proxy**: Maintains reference to RealSubject, controls access, may create/delete it

**Example Scenario**:
A document viewer with lazy image loading. The ImageProxy displays a placeholder initially. When the user scrolls to view the image, the proxy loads the RealImage from disk. This virtual proxy delays expensive loading until necessary, improving initial page load time.
