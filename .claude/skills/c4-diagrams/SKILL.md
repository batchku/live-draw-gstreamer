---
name: c4-diagrams
description: Generates C4 architecture diagrams at all four levels (Context, Container, Component, Code). Use when visualizing system architecture, creating architecture documentation, drawing component relationships, illustrating data flow between systems, documenting system boundaries, or showing interactions between containers and external systems. Creates both static structure diagrams and dynamic sequence diagrams for system behavior.
allowed-tools: Read Write Bash Edit Glob
---

# C4 Architecture Diagrams Skill

This skill helps generate C4 architecture diagrams using PlantUML with a focus on simple, well-supported syntax compatible with older PlantUML versions.

## When to Use This Skill

Use this skill when you need to:
- Create C4 Context diagrams (system landscape)
- Create C4 Container diagrams (high-level technology choices)
- Create C4 Component diagrams (component-level architecture)
- Create C4 Code diagrams (class/component relationships and sequences)
- Visualize dynamic interactions with sequence diagrams
- Generate architecture documentation diagrams

## Instructions

### 1. Read the C4 Reference

Before creating diagrams, read the `references/c4-reference.md` file to understand C4 model principles and levels.

### 2. Determine Diagram Type

Choose the appropriate C4 level:
- **Level 1 - Context**: System in context with users and external systems
- **Level 2 - Container**: Applications and data stores within the system
- **Level 3 - Component**: Components within a container
- **Level 4 - Code**: Use UML component or sequence diagrams for implementation details

### 3. Generate PlantUML Files

For **static structure** (components, classes, packages):
- Use standard PlantUML component syntax
- Keep it black and white (no themes/colors)
- Use simple, well-supported syntax
- Reference: `templates/component-example.puml`

For **dynamic behavior** (interactions, flows):
- Use PlantUML sequence diagrams
- Show interactions between components
- Reference: `templates/sequence-example.puml`

### 4. Generate Diagrams

Use the unified PlantUML script (automatically downloads and caches PlantUML):
```bash
./scripts/plantuml.sh input.puml           # Generates SVG (default)
./scripts/plantuml.sh -f png input.puml    # Generates PNG
./scripts/plantuml.sh -h                   # Show help
```

## Best Practices

1. **Keep it Simple**: Use basic PlantUML syntax without advanced features
2. **No Colors/Themes**: Ensure compatibility and printability
3. **Clear Labels**: Use descriptive names for all elements
4. **Layer Appropriately**: Match diagram detail to C4 level
5. **Prefer Sequence for Code Level**: Use sequence diagrams to show dynamic behavior
6. **Prefer Component for Structure**: Use component diagrams for static relationships

## File Organization

Store generated diagrams in:
- `docs/architecture/diagrams/` for documentation
- `docs/c4/` for dedicated C4 architecture folder
- Co-locate with related markdown documentation

## Dependencies

The `plantuml.sh` script automatically downloads and caches PlantUML in `/tmp`.

Required:
- Java Runtime (for running PlantUML)
- wget or curl (for downloading PlantUML)

Optional:
- Graphviz (for complex layouts)

Install on Ubuntu/Debian: `sudo apt-get install default-jre wget`

## Integration with Agents

This skill can be used by any agent when creating architecture documentation. The agent should:

1. **Determine the appropriate C4 level** based on the architecture scope (Context, Container, Component, or Code)
2. **Read the C4 reference** documentation to understand modeling principles
3. **Choose diagram type**:
   - Static structure → Component diagrams
   - Dynamic behavior → Sequence diagrams
4. **Generate PlantUML files** using simple, well-supported syntax
5. **Execute the plantuml.sh script** to generate SVG/PNG outputs
6. **Organize diagrams** in appropriate documentation folders

### Typical Workflow

1. **Understand Requirements**
   - What level of architecture needs visualization?
   - Static structure or dynamic behavior?
   - What level of technical detail is appropriate?

2. **Create PlantUML Source**
   ```bash
   # Read examples first
   cat templates/component-example.puml
   cat templates/sequence-example.puml

   # Write diagram source
   # Keep syntax simple, avoid advanced features
   ```

3. **Generate Diagrams**
   ```bash
   ./scripts/plantuml.sh architecture.puml
   ```

4. **Integrate with Documentation**
   - Place diagrams in `docs/architecture/diagrams/`
   - Include in SDD (System Design Documents)

## Examples

See `templates/` directory for:
- Component diagram examples
- Sequence diagram examples
- Common patterns and structures

## Support

For questions about PlantUML:
- Official documentation: https://plantuml.com/
- PlantUML guide: https://plantuml.com/guide
- Component diagrams: https://plantuml.com/component-diagram
- Sequence diagrams: https://plantuml.com/sequence-diagram
- GitHub repository: https://github.com/plantuml/plantuml
- Community forum: https://forum.plantuml.net/
- Stack Overflow: Tag `plantuml`

For C4 model concepts:
- C4 Model official site: https://c4model.com/
- C4-PlantUML library: https://github.com/plantuml-stdlib/C4-PlantUML
