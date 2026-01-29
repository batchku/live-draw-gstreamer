# Creational Patterns

Creational patterns deal with object creation mechanisms, trying to create objects in a manner suitable to the situation.

---

## 1. Singleton Pattern

**Category**: Creational

**Intent**: Ensure a class has only one instance and provide a global point of access to it.

**When to Use**:
- Exactly one instance of a class is needed (e.g., database connection, logger)
- Controlled access to a single instance is required
- The instance should be extensible by subclassing

**Structure**:
```
┌─────────────────────────┐
│      Singleton          │
├─────────────────────────┤
│ - instance: Singleton   │  (static)
├─────────────────────────┤
│ + getInstance(): Self   │  (static)
│ - Singleton()           │  (private constructor)
│ + operation()           │
└─────────────────────────┘
```

**Key Participants**:
- **Singleton**: Defines getInstance() operation that lets clients access unique instance

**Example Scenario**:
A logging system where all components of an application must write to the same log file. The Logger class ensures only one instance exists, preventing file access conflicts and maintaining consistent logging across the application.

---

## 2. Factory Method Pattern

**Category**: Creational

**Intent**: Define an interface for creating an object, but let subclasses decide which class to instantiate.

**When to Use**:
- A class can't anticipate the type of objects it must create
- A class wants its subclasses to specify the objects it creates
- Classes delegate responsibility to helper subclasses

**Structure**:
```
┌───────────────────┐                  ┌─────────────────┐
│     Creator       │                  │     Product     │
├───────────────────┤                  └────────△────────┘
│ + factoryMethod() │───────returns─────────────┘
│ + operation()     │
└────────△──────────┘
         │
         │ inherits
         │
┌────────┴──────────┐                  ┌─────────────────┐
│ ConcreteCreator   │                  │ ConcreteProduct │
├───────────────────┤                  ├─────────────────┤
│ + factoryMethod() │──────creates────►│                 │
└───────────────────┘                  └─────────────────┘
```

**Key Participants**:
- **Product**: Interface for objects the factory method creates
- **ConcreteProduct**: Implements the Product interface
- **Creator**: Declares factory method returning Product
- **ConcreteCreator**: Overrides factory method to return ConcreteProduct

**Example Scenario**:
A document editor that can create different types of documents (Text, Spreadsheet, Presentation). The Application class defines a createDocument() factory method, and each specific application (TextEditor, SpreadsheetEditor) implements it to create the appropriate document type.

---

## 3. Abstract Factory Pattern

**Category**: Creational

**Intent**: Provide an interface for creating families of related or dependent objects without specifying their concrete classes.

**When to Use**:
- System should be independent of how products are created
- System should be configured with multiple families of products
- Family of related products must be used together
- You want to reveal interfaces, not implementations

**Structure**:
```
                    ┌──────────────────────┐
                    │  AbstractFactory     │
                    ├──────────────────────┤
                    │ + createProductA()   │
                    │ + createProductB()   │
                    └───────────△──────────┘
                               │
              ┌────────────────┴────────────────┐
              │                                 │
    ┌─────────┴──────────┐           ┌──────────┴─────────┐
    │ ConcreteFactory1   │           │ ConcreteFactory2   │
    ├────────────────────┤           ├────────────────────┤
    │ + createProductA() │           │ + createProductA() │
    │ + createProductB() │           │ + createProductB() │
    └─────────┬──────────┘           └─────────┬──────────┘
              │                                │
         ┌────┴────┐                      ┌────┴────┐
         │         │                      │         │
      creates   creates                creates   creates
         │         │                      │         │
         ▼         ▼                      ▼         ▼
  ┌──────────┐ ┌──────────┐        ┌──────────┐ ┌──────────┐
  │ProductA1 │ │ProductB1 │        │ProductA2 │ │ProductB2 │
  └─────△────┘ └─────△────┘        └─────△────┘ └─────△────┘
        │            │                   │            │
        └──────┬─────┘                   └──────┬─────┘
               │                                │
    ┌──────────┴──────────┐          ┌──────────┴──────────┐
    │  AbstractProductA   │          │  AbstractProductB   │
    └─────────────────────┘          └─────────────────────┘
```

**Key Participants**:
- **AbstractFactory**: Interface for creating abstract products
- **ConcreteFactory**: Implements operations to create concrete products
- **AbstractProduct**: Interface for a type of product
- **ConcreteProduct**: Product object created by corresponding factory
- **Client**: Uses only interfaces declared by AbstractFactory and AbstractProduct

**Example Scenario**:
A UI toolkit supporting multiple look-and-feels (Windows, Mac, Linux). Each factory creates a family of related widgets (Button, ScrollBar, Window) that work together visually. The WindowsFactory creates Windows-style widgets, while MacFactory creates Mac-style widgets.

---

## 4. Builder Pattern

**Category**: Creational

**Intent**: Separate the construction of a complex object from its representation so the same construction process can create different representations.

**When to Use**:
- Algorithm for creating complex object should be independent of parts and assembly
- Construction process must allow different representations
- Step-by-step construction is needed
- Immutable objects require many parameters

**Structure**:
```
┌───────────────┐        ┌──────────────────┐
│  Director     │        │     Builder      │
├───────────────┤        ├──────────────────┤
│ - builder     │───────►│ + buildPart()    │
├───────────────┤        │ + getResult()    │
│ + construct() │        └────────△─────────┘
└───────────────┘                 │
                                  │
                    ┌─────────────┴──────────┐
                    │                        │
          ┌─────────┴─────────┐    ┌─────────┴─────────┐
          │ ConcreteBuilderA  │    │ ConcreteBuilderB  │
          ├───────────────────┤    ├───────────────────┤
          │ + buildPart()     │    │ + buildPart()     │
          │ + getResult()     │    │ + getResult()     │
          └─────────┬─────────┘    └─────────┬─────────┘
                    │                        │
                    │ creates                │ creates
                    │                        │
                    ▼                        ▼
            ┌───────────────┐        ┌───────────────┐
            │   ProductA    │        │   ProductB    │
            └───────────────┘        └───────────────┘
```

**Key Participants**:
- **Builder**: Abstract interface for creating Product parts
- **ConcreteBuilder**: Constructs and assembles Product parts
- **Director**: Constructs object using Builder interface
- **Product**: Complex object under construction

**Example Scenario**:
Building HTML documents with different formats. The HTMLBuilder constructs HTML with specific tags, while MarkdownBuilder creates Markdown format. The DocumentDirector orchestrates the building steps (addHeader, addParagraph, addFooter), and each builder produces its specific representation.

---

## 5. Prototype Pattern

**Category**: Creational

**Intent**: Specify kinds of objects to create using a prototypical instance, and create new objects by copying this prototype.

**When to Use**:
- Classes to instantiate are specified at runtime
- Avoiding building class hierarchies of factories
- Instances of a class have few different state combinations
- Object creation is expensive

**Structure**:
```
┌─────────────┐
│   Client    │
└──────┬──────┘
       │
       │ creates
       ▼
┌─────────────────┐
│   Prototype     │
├─────────────────┤
│ + clone()       │
└────────△────────┘
         │
         │ implements
         │
    ┌────┴────────────────┐
    │                     │
┌───┴──────────────┐  ┌───┴──────────────┐
│ ConcreteProto1   │  │ ConcreteProto2   │
├──────────────────┤  ├──────────────────┤
│ + clone()        │  │ + clone()        │
│   return copy    │  │   return copy    │
│   of self        │  │   of self        │
└──────────────────┘  └──────────────────┘
```

**Key Participants**:
- **Prototype**: Interface for cloning itself
- **ConcretePrototype**: Implements cloning operation
- **Client**: Creates new object by asking prototype to clone itself

**Example Scenario**:
A graphics editor with predefined shapes. Instead of creating shapes from scratch, users can clone existing shapes (circles, rectangles) and modify them. Each shape knows how to copy itself, preserving its properties (color, size, position) in the clone.
