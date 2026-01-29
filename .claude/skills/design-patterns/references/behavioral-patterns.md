# Behavioral Patterns

Behavioral patterns characterize the ways in which classes or objects interact and distribute responsibility.

---

## 13. Chain of Responsibility Pattern

**Category**: Behavioral

**Intent**: Avoid coupling the sender of a request to its receiver by giving more than one object a chance to handle the request. Chain receiving objects and pass request along until an object handles it.

**When to Use**:
- More than one object may handle request, handler unknown a priori
- Issue request to one of several objects without specifying receiver explicitly
- Set of objects that can handle request should be specified dynamically

**Structure**:
```
┌─────────────┐
│   Client    │
└──────┬──────┘
       │
       ▼
┌───────────────────┐
│    Handler        │
├───────────────────┤
│ - successor       │───┐ (next Handler)
├───────────────────┤   │
│ + handleRequest() │◄──┘
└────────△──────────┘
         │
    ┌────┴───────────────────┐
    │                        │
┌───┴───────────────┐   ┌────┴──────────────┐
│ ConcreteHandler1  │   │ ConcreteHandler2  │
├───────────────────┤   ├───────────────────┤
│ + handleRequest() │   │ + handleRequest() │
│   if can handle   │   │   if can handle   │
│     process it    │   │     process it    │
│   else            │   │   else            │
│     pass to       │   │     pass to       │
│     successor     │   │     successor     │
└───────────────────┘   └───────────────────┘
```

**Key Participants**:
- **Handler**: Interface for handling requests, optional link to successor
- **ConcreteHandler**: Handles requests it's responsible for, can access successor
- **Client**: Initiates request to ConcreteHandler in chain

**Example Scenario**:
A support ticket system with three levels: Level1Support, Level2Support, and Manager. A ticket is first sent to Level1. If they can't resolve it, they pass it to Level2. If Level2 can't handle it, it goes to Manager. Each handler decides to process or forward.

---

## 14. Command Pattern

**Category**: Behavioral

**Intent**: Encapsulate a request as an object, thereby letting you parameterize clients with different requests, queue or log requests, and support undoable operations.

**When to Use**:
- Parameterize objects by action to perform
- Specify, queue, and execute requests at different times
- Support undo
- Support logging changes for crash recovery
- Structure system around high-level operations built on primitive operations

**Structure**:
```
┌─────────────┐         ┌──────────────┐
│   Client    │────────►│   Receiver   │
└──────┬──────┘         ├──────────────┤
       │                │ + action()   │
       │ creates        └──────────────┘
       ▼                       ▲
┌──────────────┐               │ calls
│   Command    │               │
├──────────────┤               │
│ + execute()  │               │
└──────△───────┘               │
       │                       │
       │                       │
┌──────┴──────────────┐        │
│ ConcreteCommand     │        │
├─────────────────────┤        │
│ - receiver          │────────┘
│ - state             │
├─────────────────────┤
│ + execute()         │
│   receiver.action() │
└─────────────────────┘
       ▲
       │ invokes
┌──────┴──────┐
│  Invoker    │
├─────────────┤
│ - command   │
├─────────────┤
│ + invoke()  │
└─────────────┘
```

**Key Participants**:
- **Command**: Interface for executing operation
- **ConcreteCommand**: Binds Receiver with action
- **Invoker**: Asks command to carry out request
- **Receiver**: Knows how to perform operations
- **Client**: Creates ConcreteCommand and sets its Receiver

**Example Scenario**:
A text editor with undo/redo. Each action (InsertTextCommand, DeleteTextCommand) is encapsulated as a command object. Commands are stored in a history stack. Undo executes the inverse operation. The menu/toolbar (Invoker) triggers commands without knowing implementation details.

---

## 15. Interpreter Pattern

**Category**: Behavioral

**Intent**: Given a language, define a representation for its grammar along with an interpreter that uses the representation to interpret sentences in the language.

**When to Use**:
- Grammar is simple
- Efficiency is not critical concern
- Want to interpret sentences in a language

**Structure**:
```
┌─────────────┐         ┌────────────────────┐
│   Client    │────────►│     Context        │
└──────┬──────┘         └────────────────────┘
       │                        ▲
       │ builds syntax tree     │
       ▼                        │ uses
┌──────────────────────┐        │
│ AbstractExpression   │        │
├──────────────────────┤        │
│ + interpret(context) │────────┘
└──────△───────────────┘
       │
   ┌───┴──────────────────────┐
   │                          │
┌──┴───────────────────┐  ┌───┴───────────────────┐
│ TerminalExpression   │  │ NonterminalExpression │
├──────────────────────┤  ├───────────────────────┤
│ + interpret(context) │  │ + interpret(context)  │
│                      │  │ - children[]          │
└──────────────────────┘  └───────────────────────┘
```

**Key Participants**:
- **AbstractExpression**: Interface for interpret operation
- **TerminalExpression**: Implements interpret for terminal symbols
- **NonterminalExpression**: Maintains instances for each rule R ::= R1...Rn
- **Context**: Contains global information for interpreter
- **Client**: Builds abstract syntax tree, invokes interpret

**Example Scenario**:
A calculator for expressions like "3 + 5 * 2". NumberExpression is terminal (represents numbers), AddExpression and MultiplyExpression are nonterminal (combine sub-expressions). The interpreter traverses the syntax tree, evaluating each expression node recursively to compute the result.

---

## 16. Iterator Pattern

**Category**: Behavioral

**Intent**: Provide a way to access elements of an aggregate object sequentially without exposing its underlying representation.

**When to Use**:
- Access aggregate's contents without exposing internal structure
- Support multiple traversals of aggregate objects
- Provide uniform interface for traversing different aggregate structures

**Structure**:
```
┌─────────────┐
│   Client    │
└──────┬──────┘
       │
       ▼
┌──────────────────┐          ┌──────────────────┐
│    Aggregate     │          │    Iterator      │
├──────────────────┤          ├──────────────────┤
│ + createIterator │─────────►│ + first()        │
│   () : Iterator  │ returns  │ + next()         │
└────────△─────────┘          │ + isDone()       │
         │                    │ + current()      │
         │                    └────────△─────────┘
         │                             │
         │                             │
┌────────┴──────────┐         ┌────────┴─────────┐
│ ConcreteAggregate │◄────────│ ConcreteIterator │
├───────────────────┤ accesses├──────────────────┤
│ + createIterator  │─────────│ - aggregate      │
│   ()              │ creates │ + first()        │
└───────────────────┘         │ + next()         │
                              │ + isDone()       │
                              │ + current()      │
                              └──────────────────┘
```

**Key Participants**:
- **Iterator**: Interface for accessing and traversing elements
- **ConcreteIterator**: Implements Iterator, keeps track of current position
- **Aggregate**: Interface for creating Iterator
- **ConcreteAggregate**: Implements Iterator creation returning ConcreteIterator

**Example Scenario**:
A social media feed viewer that can iterate through posts from different sources (Facebook, Twitter, Instagram). Each platform has its own ConcreteIterator that knows how to traverse its specific data structure, while the client uses a uniform Iterator interface (next(), hasNext(), current()).

---

## 17. Mediator Pattern

**Category**: Behavioral

**Intent**: Define an object that encapsulates how a set of objects interact. Mediator promotes loose coupling by keeping objects from referring to each other explicitly.

**When to Use**:
- Set of objects communicate in well-defined but complex ways
- Reusing object is difficult because it refers to many others
- Behavior distributed between several classes should be customizable without subclassing

**Structure**:
```
┌──────────────────┐
│     Mediator     │
├──────────────────┤
│ + notify()       │
└────────△─────────┘
         │
         │
┌────────┴──────────┐
│ ConcreteMediator  │
├───────────────────┤
│ - colleague1      │───┐
│ - colleague2      │   │
├───────────────────┤   │
│ + notify()        │   │
└───────────────────┘   │
         ▲              │
         │ knows        │
         │              │
         │              │
┌────────┴─────────┐    │
│   Colleague      │    │
├──────────────────┤    │
│ - mediator       │◄───┘
├──────────────────┤
│ + send()         │
│ + receive()      │
└────────△─────────┘
         │
    ┌────┴──────────────────┐
    │                       │
┌───┴────────────────┐ ┌────┴───────────────┐
│ ConcreteColleague1 │ │ ConcreteColleague2 │
└────────────────────┘ └────────────────────┘
```

**Key Participants**:
- **Mediator**: Interface for communicating with Colleague objects
- **ConcreteMediator**: Coordinates Colleague objects, knows and maintains colleagues
- **Colleague**: Each Colleague class knows its Mediator, communicates via Mediator

**Example Scenario**:
An air traffic control system where aircraft don't communicate directly with each other. Instead, they all communicate through the ControlTower (mediator). When a plane wants to land, it notifies the tower, which coordinates with other planes to ensure safe landing. This prevents tight coupling between aircraft.

---

## 18. Memento Pattern

**Category**: Behavioral

**Intent**: Without violating encapsulation, capture and externalize an object's internal state so the object can be restored to this state later.

**When to Use**:
- Snapshot of object's state must be saved for later restoration
- Direct interface to obtain state would expose implementation details

**Structure**:
```
┌─────────────┐         ┌──────────────┐
│ Caretaker   │         │  Originator  │
├─────────────┤         ├──────────────┤
│ - memento   │◄────────│ - state      │
└─────────────┘ creates ├──────────────┤
                        │ + create     │
                        │   Memento()  │───┐
                        │ + restore    │   │
                        │   (Memento)  │   │
                        └──────────────┘   │
                                           │
                        ┌──────────────┐   │
                        │  Memento     │◄──┘
                        ├──────────────┤
                        │ - state      │
                        ├──────────────┤
                        │ + getState() │
                        │ + setState() │
                        └──────────────┘
```

**Key Participants**:
- **Memento**: Stores internal state of Originator, protects against access by others
- **Originator**: Creates memento containing snapshot, uses memento to restore state
- **Caretaker**: Responsible for memento's safekeeping, never operates on or examines contents

**Example Scenario**:
A game with save points. The GameState (Originator) creates a SavePoint (Memento) containing player position, health, inventory. The SaveManager (Caretaker) stores these. When player dies, the SaveManager provides the memento to restore GameState. The SaveManager doesn't know the memento's internal structure.

---

## 19. Observer Pattern

**Category**: Behavioral

**Intent**: Define a one-to-many dependency between objects so that when one object changes state, all its dependents are notified and updated automatically.

**When to Use**:
- Abstraction has two aspects, one dependent on the other
- Change to one object requires changing others, number unknown
- Object should notify others without assumptions about who they are

**Structure**:
```
┌──────────────────┐         ┌──────────────┐
│     Subject      │         │   Observer   │
├──────────────────┤         ├──────────────┤
│ - observers[]    │────────►│ + update()   │
├──────────────────┤         └──────△───────┘
│ + attach(obs)    │                │
│ + detach(obs)    │                │
│ + notify()       │                │
└────────△─────────┘                │
         │                          │
         │                          │
┌────────┴─────────┐         ┌──────┴───────────┐
│ ConcreteSubject  │         │ ConcreteObserver │
├──────────────────┤         ├──────────────────┤
│ - state          │         │ - subject        │
├──────────────────┤         ├──────────────────┤
│ + getState()     │◄────────│ + update()       │
│ + setState()     │ observes│   get subject    │
└──────────────────┘         │   state          │
                             └──────────────────┘
```

**Key Participants**:
- **Subject**: Knows its observers, provides interface for attaching/detaching
- **Observer**: Interface for objects to be notified of changes
- **ConcreteSubject**: Stores state, sends notifications when state changes
- **ConcreteObserver**: Maintains reference to ConcreteSubject, implements update

**Example Scenario**:
A weather station (Subject) with multiple displays (Observers): CurrentConditionsDisplay, StatisticsDisplay, ForecastDisplay. When weather data changes, the station notifies all displays, which pull new data and update their presentation. Displays can be added or removed without modifying the weather station.

---

## 20. State Pattern

**Category**: Behavioral

**Intent**: Allow an object to alter its behavior when its internal state changes. The object will appear to change its class.

**When to Use**:
- Object behavior depends on its state and must change at runtime
- Operations have large conditional statements depending on object state

**Structure**:
```
┌─────────────┐         ┌──────────────┐
│  Context    │────────►│    State     │
├─────────────┤         ├──────────────┤
│ - state     │         │ + handle()   │
├─────────────┤         └──────△───────┘
│ + request() │                │
└─────────────┘                │
       │                       │
       │ delegates             │
       │                   ┌───┴────────────┐
       │                   │                │
       │            ┌──────┴───────┐ ┌──────┴───────┐
       │            │ConcreteStateA│ │ConcreteStateB│
       │            ├──────────────┤ ├──────────────┤
       └───────────►│ + handle()   │ │ + handle()   │
         forwards   │   change     │ │   change     │
         to state   │   context    │ │   context    │
                    │   state      │ │   state      │
                    └──────────────┘ └──────────────┘
```

**Key Participants**:
- **Context**: Defines client interface, maintains ConcreteState instance
- **State**: Interface for encapsulating behavior associated with Context state
- **ConcreteState**: Each subclass implements behavior associated with state of Context

**Example Scenario**:
A TCP connection with states: Listening, Established, Closed. Each state handles operations (open, close, acknowledge) differently. In Listening state, acknowledge is ignored; in Established state, it processes data. The Connection delegates to current State object, which changes the Connection's state as needed.

---

## 21. Strategy Pattern

**Category**: Behavioral

**Intent**: Define a family of algorithms, encapsulate each one, and make them interchangeable. Strategy lets the algorithm vary independently from clients that use it.

**When to Use**:
- Many related classes differ only in behavior
- Need different variants of algorithm
- Algorithm uses data clients shouldn't know about
- Class defines many behaviors appearing as multiple conditional statements

**Structure**:
```
┌────────────────┐         ┌───────────────┐
│  Context       │────────►│  Strategy     │
├────────────────┤         ├───────────────┤
│ - strategy     │         │ + algorithm() │
├────────────────┤         └──────△────────┘
│ + operation()  │                │
│   strategy     │                │
│   .algorithm() │                │
└────────────────┘       ┌────────┴──────────────┐
                         │                       │
                  ┌──────┴────────────┐ ┌────────┴──────────┐
                  │ ConcreteStrategyA │ │ ConcreteStrategyB │
                  ├───────────────────┤ ├───────────────────┤
                  │ + algorithm()     │ │ + algorithm()     │
                  └───────────────────┘ └───────────────────┘
```

**Key Participants**:
- **Strategy**: Common interface for all supported algorithms
- **ConcreteStrategy**: Implements algorithm using Strategy interface
- **Context**: Configured with ConcreteStrategy, maintains reference to Strategy

**Example Scenario**:
A navigation app with different routing strategies: ShortestRoute, FastestRoute, ScenicRoute. The Navigator (Context) accepts a RouteStrategy and calculates path using that algorithm. Users can switch strategies at runtime without changing the Navigator code.

---

## 22. Template Method Pattern

**Category**: Behavioral

**Intent**: Define the skeleton of an algorithm in an operation, deferring some steps to subclasses. Template Method lets subclasses redefine certain steps without changing the algorithm's structure.

**When to Use**:
- Implement invariant parts of algorithm once, leave varying parts to subclasses
- Common behavior among subclasses should be factored and localized
- Control subclass extensions (hooks)

**Structure**:
```
┌──────────────────────┐
│  AbstractClass       │
├──────────────────────┤
│ + templateMethod()   │────┐ (final/sealed)
│   primitiveOp1()     │    │
│   primitiveOp2()     │    │
│   hook()             │    │
├──────────────────────┤    │
│ + primitiveOp1()     │◄───┘ (abstract)
│ + primitiveOp2()     │      (abstract)
│ + hook()             │      (optional override)
└──────────△───────────┘
           │
     ┌─────┴─────────┐
     │               │
┌────┴───────────┐ ┌─┴──────────────┐
│ ConcreteClassA │ │ ConcreteClassB │
├────────────────┤ ├────────────────┤
│ + primitiveOp1 │ │ + primitiveOp1 │
│ + primitiveOp2 │ │ + primitiveOp2 │
│ + hook()       │ │ + hook()       │
└────────────────┘ └────────────────┘
```

**Key Participants**:
- **AbstractClass**: Defines template method, abstract primitive operations
- **ConcreteClass**: Implements primitive operations for specific steps

**Example Scenario**:
A data processor with templateMethod: read(), process(), write(). AbstractDataProcessor defines the workflow, while CSVProcessor and JSONProcessor implement read() and write() for their formats. The process() step is common. All subclasses follow the same workflow but with format-specific I/O.

---

## 23. Visitor Pattern

**Category**: Behavioral

**Intent**: Represent an operation to be performed on elements of an object structure. Visitor lets you define a new operation without changing the classes of the elements on which it operates.

**When to Use**:
- Object structure contains many classes with differing interfaces
- Many distinct operations need to be performed on objects in structure
- Classes defining object structure rarely change but operations do
- Add new operations without changing element classes

**Structure**:
```
┌─────────────┐
│   Client    │
└──────┬──────┘
       │
       ▼
┌─────────────────┐                 ┌────────────────────┐
│ ObjectStructure │                 │     Visitor        │
├─────────────────┤                 ├────────────────────┤
│ - elements[]    │                 │ + visitConcreteA() │
└─────────────────┘                 │ + visitConcreteB() │
       │                            └─────────△──────────┘
       │                                      │
       │                                      │
       │                            ┌─────────┴──────────┐
       │                            │ ConcreteVisitor    │
       ▼                            ├────────────────────┤
┌───────────────────┐               │ + visitConcreteA() │
│   Element         │               │ + visitConcreteB() │
├───────────────────┤               └────────────────────┘
│ + accept(Visitor) │                         ▲
└──────△────────────┘                         │
       │                                      │
   ┌───┴───────────────────┐                  │
   │                       │                  │
┌──┴───────────────┐  ┌────┴─────────────┐    │
│ ConcreteElementA │  │ ConcreteElementB │    │
├──────────────────┤  ├──────────────────┤    │
│ + accept(v)      │  │ + accept(v)      │    │
│   v.visit        │  │   v.visit        │    │
│   ConcreteA      │  │   ConcreteB      │────┘
│   (this)         │  │   (this)         │
└──────────────────┘  └──────────────────┘
```

**Key Participants**:
- **Visitor**: Interface declaring visit operation for each ConcreteElement class
- **ConcreteVisitor**: Implements each operation declared by Visitor
- **Element**: Defines accept operation taking Visitor as argument
- **ConcreteElement**: Implements accept operation
- **ObjectStructure**: Can enumerate elements, provides high-level interface for visitor

**Example Scenario**:
A company organization with Employee and Contractor elements. Different visitors perform different operations: SalaryCalculatorVisitor computes compensation, VacationDaysVisitor calculates time off, TaxReportVisitor generates tax documents. Each visitor knows how to work with each element type without modifying the Employee/Contractor classes.
