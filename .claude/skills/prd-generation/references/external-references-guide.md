# External References Guide

This document explains how to handle external references (URLs and file paths) when generating PRDs.

---

## Overview

External references are automatically extracted from the brief and provided to you in the prompt under the "📚 Extracted External References" section. Your job is to fetch this content and document it for downstream agents.

---

## Using Provided References

When external references are provided in your prompt:

1. **Review the provided references** - URLs and absolute file paths have been extracted for you
2. **Fetch the content** - Use WebFetch for URLs, Read for file paths
3. **Generate EXTERNAL.md** - Document relevant information at `docs/planning/EXTERNAL.md`
4. **Focus on actionable information** - Extract APIs, data structures, configuration options, patterns

---

## Accessing External Content

### For URLs
Use the **WebFetch** tool to retrieve web content:

```markdown
WebFetch(
    url="https://example.com/api-docs",
    prompt="Extract API endpoints, authentication requirements, and rate limits"
)
```

### For File Paths
Use the **Read** tool to access local files:

```markdown
Read(file_path="/absolute/path/to/reference.md")
```

---

## What to Extract from External References

Focus on information that will help downstream agents (SDD generation, implementation, testing):

### API Documentation
- Endpoint URLs and HTTP methods
- Request/response formats and schemas
- Authentication and authorization mechanisms
- Rate limits and quotas
- Error codes and handling
- Example requests and responses

### Library/Framework Documentation
- Installation and setup instructions
- Key classes, functions, and methods
- Configuration options and defaults
- Common patterns and best practices
- Integration requirements
- Version compatibility

### Design Guidelines
- UI/UX patterns to follow
- Branding requirements (colors, fonts, logos)
- Accessibility standards
- Component libraries to use

### Data Specifications
- Data formats (JSON, XML, CSV, etc.)
- Schema definitions
- Validation rules
- Data transformation requirements

### External System Integration
- Connection protocols and endpoints
- Authentication credentials (references, not actual secrets)
- Data synchronization requirements
- Error handling and retry logic

---

## EXTERNAL.md Template

Create `docs/planning/EXTERNAL.md` with the following structure:

```markdown
# External Context

This document contains context gathered from external references in the project brief.

## [Source Name/URL]

**Source:** [URL or file path]

**Summary:** [1-2 sentence description of the source]

**Relevant Information:**
- [Key API endpoints, data structures, or configuration options]
- [Important patterns or conventions to follow]
- [Dependencies or integration requirements]
- [Version requirements or compatibility notes]

**Example Usage:**
[If applicable, include code snippets or configuration examples]

---

## [Next Source]

**Source:** [URL or file path]

**Summary:** [Description]

**Relevant Information:**
- [Key information]

---
```

---

## Example EXTERNAL.md

Here's an example of a well-structured EXTERNAL.md:

```markdown
# External Context

This document contains context gathered from external references in the project brief.

## Python Curses Library Documentation

**Source:** https://docs.python.org/3/library/curses.html

**Summary:** Official Python documentation for the curses library, used for terminal-based UI programming.

**Relevant Information:**
- **Initialization**: Use `curses.wrapper(main)` for automatic setup/teardown
- **Input**: `stdscr.getch()` for keyboard input, `stdscr.nodelay(1)` for non-blocking
- **Colors**: Initialize with `curses.start_color()`, define pairs with `curses.init_pair()`
- **Window Management**: `stdscr.clear()`, `stdscr.refresh()`, `stdscr.addstr(y, x, str)`
- **Compatibility**: Standard on Unix/Linux/macOS, requires `windows-curses` on Windows

**Example Usage:**
```python
import curses

def main(stdscr):
    curses.curs_set(0)  # Hide cursor
    stdscr.nodelay(1)   # Non-blocking input
    stdscr.timeout(100) # 100ms timeout

    while True:
        key = stdscr.getch()
        if key == ord('q'):
            break
        stdscr.addstr(0, 0, f"Key pressed: {key}")
        stdscr.refresh()

curses.wrapper(main)
```

---

## Snake Game Reference Implementation

**Source:** https://github.com/example/snake-reference

**Summary:** Reference implementation of terminal-based Snake game demonstrating movement algorithms and collision detection.

**Relevant Information:**
- **Movement Algorithm**: Deque-based approach for O(1) segment addition/removal
- **Collision Detection**: Set-based lookup for O(1) self-collision checking
- **Direction Handling**: Queue single direction change to prevent double-tap issues
- **Game Loop**: Fixed 100ms tick rate using time-based delta calculation

**Key Code Pattern:**
```python
from collections import deque

class Snake:
    def __init__(self):
        self.segments = deque([(10, 10)])  # Start position
        self.direction = (0, 1)  # Moving right

    def move(self):
        head_y, head_x = self.segments[0]
        dy, dx = self.direction
        new_head = (head_y + dy, head_x + dx)
        self.segments.appendleft(new_head)
        self.segments.pop()  # Remove tail
```

---

## Color Scheme Reference

**Source:** /home/user/Projects/snake/design/colors.md

**Summary:** Terminal color palette specification for game UI.

**Relevant Information:**
- **Background**: Black (curses.COLOR_BLACK)
- **Snake Body**: Green (curses.COLOR_GREEN)
- **Snake Head**: Bright Green (curses.COLOR_GREEN + curses.A_BOLD)
- **Food**: Red (curses.COLOR_RED)
- **Border**: White (curses.COLOR_WHITE)
- **Score Display**: Yellow (curses.COLOR_YELLOW)

**Color Pair Initialization:**
```python
curses.init_pair(1, curses.COLOR_GREEN, curses.COLOR_BLACK)  # Snake
curses.init_pair(2, curses.COLOR_RED, curses.COLOR_BLACK)    # Food
curses.init_pair(3, curses.COLOR_YELLOW, curses.COLOR_BLACK) # Score
```

---
```

---

## Important Notes

### Only Generate EXTERNAL.md When References Are Provided

- **Do NOT** scan the brief yourself for references
- Only create EXTERNAL.md if the prompt includes "📚 Extracted External References"
- If no references are provided, skip EXTERNAL.md generation entirely

### Focus on Actionable Information

Extract information that answers these questions:
- **For SDD agent**: What technical approaches should be used?
- **For implementation**: What APIs, libraries, or patterns are required?
- **For testing**: What validation criteria or test data is needed?

Avoid:
- Marketing copy or promotional content
- Redundant information already in the brief
- Tangential topics not relevant to implementation

### Keep Summaries Concise

- 1-2 sentences per source summary
- Bullet points for relevant information
- Code snippets only when they illustrate key patterns
- Focus on "what" and "how", not "why" (that's in the PRD)

---

## Workflow Summary

1. Check prompt for "📚 Extracted External References" section
2. If present, use WebFetch/Read to retrieve each reference
3. Extract actionable information relevant to implementation
4. Create `docs/planning/EXTERNAL.md` with structured content
5. Add "docs/planning/EXTERNAL.md" to `artifacts_changed` in response
6. If no references provided, skip EXTERNAL.md entirely
