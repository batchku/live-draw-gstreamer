# MVP-First Requirements Structuring

This document explains how to structure requirements to support incremental MVP delivery.

---

## MVP-First Requirement Structure

When defining requirements, structure them to support **incremental MVP delivery**:

### Phase 1 Requirements: The Minimal Working Version

Define the absolute minimum that proves the application works:

- **What is the simplest proof the application works?**
- **What demonstrates the application executes successfully?**
- **What is the minimum viable proof of concept?**

**Key Characteristics:**
- Focus on basic initialization and execution
- Prove the core technology stack works
- Demonstrate the application can start and stop cleanly
- Minimal but complete end-to-end flow

**Example Phase 1 Requirements:**
- System shall initialize and exit cleanly with status code 0
- System shall display version information on startup
- System shall respond to basic health check with "OK" status
- System shall load configuration from default settings

---

### Phase 2+ Requirements: Incremental Feature Building

Build incrementally on the working base:

- Each phase adds complete, demonstrable features
- Requirements should support vertical slicing (full feature stack, not just layers)
- Each phase delivers user-facing value
- Phases build upon previous working functionality

**Key Characteristics:**
- Complete features that users can interact with
- Full stack implementation (UI → business logic → data layer)
- Testable and demonstrable outcomes
- Progressive complexity

---

## Examples: Good vs Bad Requirement Structuring

### ✅ GOOD: MVP-First (Vertical Slicing)

Requirements structured for incremental delivery:

```
Phase 1: Proof of Concept
§4.1: Application shall launch and display "Ready" status message
§4.2: Application shall respond to health check with version info

Phase 2: Basic Functionality
§4.3: Application shall accept user input and store in database
§4.4: Application shall retrieve stored user data via API endpoint

Phase 3: Enhanced Features
§4.5: Application shall validate user input against schema
§4.6: Application shall support filtering retrieved data by criteria
```

**Why this works:**
- Phase 1 delivers a working (though minimal) application
- Phase 2 adds complete user-facing features
- Each phase builds on the previous one
- Users can see progress at each phase
- Each phase is independently testable

---

### ❌ BAD: Layer-First (Horizontal Slicing)

Requirements structured by technical layers:

```
Phase 1: Data Layer
§4.1: Application shall define all data models
§4.2: Application shall initialize database schema
§4.3: Application shall implement data access layer

Phase 2: Business Logic Layer
§4.4: Application shall implement business logic layer
§4.5: Application shall validate business rules

Phase 3: API Layer
§4.6: Application shall expose API endpoints
§4.7: Application shall handle API requests
```

**Why this fails:**
- No working application until Phase 3 is complete
- Cannot demonstrate value to users until the end
- Difficult to test in isolation
- High risk of late-stage integration issues
- No incremental value delivery

---

## Vertical Slicing Guidelines

When writing requirements, follow these principles:

### 1. Define End-to-End Flows
Each requirement should describe a complete user flow:
```
✅ "User shall create account, receive confirmation email, and login"
❌ "System shall implement user registration database table"
```

### 2. Prioritize User-Facing Value
Focus on what users can see and do:
```
✅ "Player shall see score increase when collecting items"
❌ "System shall implement score calculation algorithm"
```

### 3. Build in Thin Slices
Start with minimal implementation, then enhance:
```
Phase 1: "System shall save user data to local JSON file"
Phase 2: "System shall save user data to SQLite database"
Phase 3: "System shall save user data to PostgreSQL with replication"
```

### 4. Each Slice Must Be Demonstrable
Every requirement should produce something you can show:
```
✅ "Game shall display animated snake moving on screen"
❌ "Game shall implement snake movement logic"
```

---

## Common Pitfalls

### Pitfall 1: Infrastructure-First Thinking
```
❌ Bad:
§4.1: Set up database infrastructure
§4.2: Configure caching layer
§4.3: Implement API gateway
§4.4: Add user-facing features

✅ Good:
§4.1: Implement basic feature with hardcoded data
§4.2: Add database persistence to feature
§4.3: Add caching to improve feature performance
```

### Pitfall 2: Complete Feature Requirements Too Early
```
❌ Bad:
§4.1: User authentication with OAuth2, MFA, SSO, and password recovery

✅ Good:
§4.1 (Phase 1): User authentication with username/password
§4.2 (Phase 2): Add password recovery via email
§4.3 (Phase 3): Add multi-factor authentication
§4.4 (Phase 4): Add OAuth2 social login
```

### Pitfall 3: Technical Details Instead of User Outcomes
```
❌ Bad:
§4.1: Implement RESTful API endpoints using Flask
§4.2: Set up SQLAlchemy ORM models
§4.3: Configure JWT token authentication

✅ Good:
§4.1: API shall allow users to create and retrieve their profile information
§4.2: API shall authenticate users and maintain session state
§4.3: API shall protect user data from unauthorized access
```

---

## Writing MVP-First Requirements

Use this template when writing requirements:

```markdown
## Phase 1: Minimal Viable Product
[Focus: Prove the application works at the most basic level]

§4.1: [Simplest initialization requirement]
§4.2: [Basic health check or status indication]
§4.3: [Minimal core functionality - one complete flow]

## Phase 2: Essential Features
[Focus: Add the most critical user-facing features]

§4.4: [First complete user feature - full stack]
§4.5: [Second complete user feature - full stack]

## Phase 3: Enhanced Functionality
[Focus: Improve and expand on core features]

§4.6: [Feature enhancement or additional capability]
§4.7: [Feature enhancement or additional capability]
```

---

## Checklist for MVP-First Requirements

Before finalizing requirements, verify:

- [ ] Phase 1 delivers a working (though minimal) application
- [ ] Each requirement describes a complete user flow, not just a technical layer
- [ ] Requirements are ordered by user value, not technical convenience
- [ ] Each phase builds incrementally on the previous one
- [ ] Every requirement is independently testable and demonstrable
- [ ] No requirement depends on "future" functionality to be useful
- [ ] Users can see tangible progress at the end of each phase
