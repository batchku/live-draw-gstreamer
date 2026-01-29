# PRD Template and Section Guidance

This document provides the complete PRD template structure and detailed guidance for each section.

---

## Required PRD Sections

A complete PRD must contain all 9 of the following sections with substantive content.

### 1. Executive Summary

The Executive Summary provides a high-level overview of the product.

**Subsections:**
- **Vision and Purpose**: Clear statement of what the product is and why it exists
- **Target Users**: Who will use this product (personas, user segments)
- **Value Proposition**: What unique value does this provide to users

**Example:**
```markdown
## 1. Executive Summary

### Vision and Purpose
SnakeGame is a terminal-based implementation of the classic Snake game, designed to provide quick entertainment in a command-line environment without requiring graphical dependencies.

### Target Users
- Terminal users and system administrators who want quick entertainment
- Developers learning Python and terminal graphics
- Retro gaming enthusiasts who appreciate ASCII art games

### Value Proposition
SnakeGame delivers nostalgic gaming entertainment that runs anywhere Python is installed, with no GUI dependencies, making it ideal for remote sessions and minimal environments.
```

---

### 2. Goals and Objectives

Define what the product aims to achieve from both user and technical perspectives.

**Subsections:**
- **User Goals**: What users want to accomplish with the product
- **Technical Goals**: Technical objectives and quality attributes

**Example:**
```markdown
## 2. Goals and Objectives

### User Goals
- Play a complete Snake game with smooth controls
- See their score increase as they collect food
- Experience progressively challenging gameplay as the snake grows

### Technical Goals
- Implement clean, maintainable Python code using OOP principles
- Achieve responsive input handling with minimal latency
- Provide cross-platform terminal compatibility
```

---

### 3. User Stories

⚠️ **CRITICAL**: See [validation-rules.md](validation-rules.md) for required user story format.

Each user story must include:
- **The story itself**: Using the single-line format: `As a [user type], I want [feature] so that [benefit].`
- **Acceptance Criteria**: Specific, testable conditions (checklist format with at least one `- [ ]` item)
- **Priority**: Must/Should/Could/Won't (MoSCoW) or High/Medium/Low

**Minimum**: 1 user story required (more for complex projects)

**Example:**
```markdown
## 3. User Stories

### §3.1: Game Launch (Priority: Must)

As a terminal user, I want the game to launch with a simple command so that I can start playing quickly.

**Acceptance Criteria:**
- [ ] Game launches with `python3 snake.py`
- [ ] Initialization completes within 1 second
- [ ] Clear error message if Python version < 3.8

### §3.2: Snake Movement (Priority: Must)

As a player, I want to control the snake using arrow keys so that I can navigate toward food.

**Acceptance Criteria:**
- [ ] Arrow keys change snake direction
- [ ] Snake moves smoothly at consistent speed
- [ ] Direction cannot reverse instantly (no 180-degree turns)
```

---

### 4. Functional Requirements

Detailed description of each feature, including user flows, edge cases, and business rules.

**Structure for each requirement:**
- **Description**: What the feature does
- **User Flow**: Step-by-step workflow
- **Edge Cases**: Exceptional scenarios
- **Business Rules**: Validation rules and constraints

**Example:**
```markdown
## 4. Functional Requirements

### §4.1: Snake Movement System

**Description**: The snake continuously moves in the current direction, with the player able to change direction using keyboard input.

**User Flow**:
1. Game initializes with snake at center, moving right
2. Player presses arrow key to change direction
3. Snake head turns to new direction on next frame
4. Snake body follows head's path segment by segment

**Edge Cases**:
- Player presses opposite direction (e.g., left while moving right): Ignore input
- Player presses multiple keys rapidly: Process only the last valid direction change
- Snake reaches screen boundary: Trigger collision detection

**Business Rules**:
- Snake cannot reverse direction instantly (180-degree turn prevention)
- Direction changes queue: maximum 1 pending direction change
- Movement speed: 10 frames per second initially, increasing by 1 fps per food eaten (max 20 fps)
```

---

### 5. Non-Functional Requirements

Specify quality attributes with measurable targets.

**Categories:**
- **Performance**: Response times, throughput, resource usage
- **Security**: Authentication, authorization, data protection
- **Scalability**: Load capacity, growth handling
- **Accessibility**: WCAG compliance, assistive technology support
- **Reliability**: Uptime targets, error rates
- **Compatibility**: Platform/browser/device support

**Example:**
```markdown
## 5. Non-Functional Requirements

### Performance
- Frame rate: 10-20 fps depending on difficulty level
- Input latency: < 50ms from keypress to direction change
- Memory usage: < 10MB RAM
- Startup time: < 1 second on modern hardware

### Security
- No network access required or allowed
- No file system access beyond configuration file read
- Safe handling of terminal control sequences

### Scalability
- Support terminal sizes from 24x80 to 120x200 characters
- Handle snake length up to 1000 segments without performance degradation

### Accessibility
- Terminal-based display compatible with screen readers
- High-contrast color scheme option
- Configurable game speed for different skill levels

### Reliability
- Graceful handling of terminal resize events
- Clean exit on Ctrl+C with terminal state restoration
- Error recovery from corrupted configuration files

### Compatibility
- Python 3.8+ on Linux, macOS, Windows
- Support for standard terminal emulators (xterm, gnome-terminal, iTerm2, Windows Terminal)
```

---

### 6. Technical Constraints

Document required technologies, integrations, platforms, and compliance requirements.

**Example:**
```markdown
## 6. Technical Constraints

- **Programming Language**: Python 3.8 or higher (required for walrus operator and typing improvements)
- **Terminal Library**: `curses` module (standard library on Unix, requires `windows-curses` on Windows)
- **Dependency Management**: Poetry for packaging and dependency resolution
- **Testing Framework**: pytest with minimum 80% code coverage
- **Platform Support**: Linux, macOS, Windows 10+
- **No External Services**: Game must run entirely offline with no network dependencies
```

---

### 7. Assumptions and Dependencies

Document what you're assuming to be true and what external factors the project depends on.

**Subsections:**
- **Assumptions**: What we believe to be true
- **External Dependencies**: Third-party services, APIs, data sources
- **Internal Dependencies**: Other teams, systems, or projects
- **Risks**: Potential blockers or challenges

**Example:**
```markdown
## 7. Assumptions and Dependencies

### Assumptions
- Users have Python 3.8+ installed
- Users are familiar with basic terminal navigation
- Terminal supports ANSI color codes and cursor positioning
- Keyboard input is available (not running in non-interactive mode)

### Dependencies
- Python standard library `curses` module (Unix/Linux/macOS)
- `windows-curses` package for Windows compatibility
- Terminal emulator with minimum 24x80 character display

### Risks
- Terminal compatibility issues across different emulators
- Windows `curses` implementation may have subtle behavioral differences
- Very small terminal sizes may make the game unplayable
- Input latency on remote/slow terminal connections
```

---

### 8. Out of Scope

Explicitly state what is NOT included in this version.

**Example:**
```markdown
## 8. Out of Scope

The following are explicitly excluded from this version:

- Multiplayer functionality
- High score persistence across sessions
- Level editor or custom map creation
- Power-ups or special items beyond basic food
- Sound effects or audio feedback
- Graphical user interface (GUI) version
- Mobile or web versions
- Network leaderboards
- Replay or demo recording features
- Difficulty customization beyond speed adjustment
```

---

### 9. Success Metrics

Define measurable KPIs with specific targets and measurement methods.

**Format**: Use a table with Metric, Target, and Measurement Method columns.

**Example:**
```markdown
## 9. Success Metrics

┌─────────────────────────────┬──────────────────┬────────────────────────────────────┐
│ Metric                      │ Target           │ Measurement Method                 │
├─────────────────────────────┼──────────────────┼────────────────────────────────────┤
│ Installation Success Rate   │ > 95%            │ Test across 10 different systems   │
│ Game Startup Time           │ < 1 second       │ Profiling with cProfile            │
│ Code Coverage               │ > 80%            │ pytest-cov reporting               │
│ Critical Bugs               │ < 3 per release  │ GitHub Issues tagged "critical"    │
│ Average Session Length      │ > 5 minutes      │ User testing feedback              │
│ Frame Rate Consistency      │ ±1 fps variance  │ Frame timing measurements          │
└─────────────────────────────┴──────────────────┴────────────────────────────────────┘
```

---

## Complete PRD Template

```markdown
# Product Requirements Document: [Product Name]

## 1. Executive Summary

### Vision and Purpose
[Clear statement of what the product is and why it exists]

### Target Users
[Description of user personas and segments]

### Value Proposition
[Unique value provided to users]

## 2. Goals and Objectives

### User Goals
- [What users want to accomplish]

### Technical Goals
- [Technical objectives and deliverables]

## 3. User Stories

⚠️ CRITICAL: Each user story MUST be single-line, plain text format.
DO NOT use multi-line bold format like "**As a** user **I want to**..."

### §3.1: [Title] (Priority: Must)
As a [user type], I want [feature] so that [benefit].

**Acceptance Criteria:**
- [ ] Criterion 1
- [ ] Criterion 2
- [ ] Criterion 3

### §3.2: [Title] (Priority: Should)
As a [user type], I want [feature] so that [benefit].

**Acceptance Criteria:**
- [ ] Criterion 1
- [ ] Criterion 2

[Repeat for all user stories - remember: single-line format for each story]

## 4. Functional Requirements

### §4.1: [Feature Name]
**Description**: [Detailed description]
**User Flow**:
1. Step 1
2. Step 2
3. Step 3

**Edge Cases**:
- [Edge case 1]
- [Edge case 2]

[Repeat for all features]

## 5. Non-Functional Requirements

### Performance
- API response time: < 200ms for 95th percentile
- Page load time: < 2 seconds
- Database query time: < 100ms

### Security
- [Security requirements]

### Scalability
- [Scalability requirements]

### Accessibility
- [Accessibility requirements]

## 6. Technical Constraints

- **Technology stack**: [Required technologies]
- **Integrations**: [Third-party services]
- **Platform**: [Deployment environments]
- **Compliance**: [Regulatory requirements]

## 7. Assumptions and Dependencies

### Assumptions
- [Assumption 1]
- [Assumption 2]

### Dependencies
- [Dependency 1]
- [Dependency 2]

### Risks
- [Risk 1]
- [Risk 2]

## 8. Out of Scope

The following are explicitly excluded from this version:
- [Excluded feature 1]
- [Excluded feature 2]

## 9. Success Metrics

┌─────────┬─────────┬────────────────────┐
│ Metric  │ Target  │ Measurement Method │
├─────────┼─────────┼────────────────────┤
│ [KPI 1] │ [Value] │ [How]              │
│ [KPI 2] │ [Value] │ [How]              │
└─────────┴─────────┴────────────────────┘
```
