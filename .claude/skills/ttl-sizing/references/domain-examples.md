# Domain Examples for TTL Sizing

Examples showing score → band → TTL shape. Use for pattern-matching, not copy-paste.

---

## Failure Examples (LEARN FROM THESE)

### ❌ FAILURE: Word Counter Over-Engineering

**Brief**: "Count words in a text file. Single Python script. Standard library only."

**What the agent did wrong**:
- Scored 30/100 → "Small"
- Produced 27 tasks across 3 phases
- Never showed dimension-by-dimension scoring

**Correct scoring**:
| Dimension | Score | Evidence |
|-----------|-------|----------|
| Product surface | 0 | One flow: read→count→print |
| Artifacts | 0 | Single script |
| Data model | 0 | No persistence |
| Integrations | 0 | stdlib only |
| Security | 0 | No auth |
| Ops | 0 | Local only |
| Performance | 0 | Negligible |
| UX/design | 0 | CLI output only |
| Migration/legacy | 0 | Greenfield |
| Uncertainty | 0 | Crystal clear brief |
| **TOTAL** | **0** | **Micro** |

**Correct TTL**: 1 phase, 1-5 tasks

### ❌ FAILURE: Temperature Converter Over-Engineering

**Brief**: "Python script to convert Celsius to Fahrenheit"

**What the agent did wrong**:
- 5 phases: Setup, Core Logic, CLI, Testing, Documentation
- 24 total tasks

**Correct**: Score 0 → Micro → 1 phase, 2 tasks max

### ❌ FAILURE: TODO App Over-Engineering

**Brief**: "Simple TODO list. Add/remove/list items. Save to JSON file."

**What the agent did wrong**:
- Scored as "Small" (23–34)
- 4 phases, 25 tasks

**Correct scoring**:
| Dimension | Score | Evidence |
|-----------|-------|----------|
| Product surface | 2 | 3 simple flows |
| Artifacts | 0 | Single script |
| Data model | 2 | One JSON file |
| Integrations | 0 | None |
| Security | 0 | No auth |
| Ops | 0 | Local only |
| Performance | 0 | Negligible |
| UX/design | 0 | CLI only |
| Migration/legacy | 0 | Greenfield |
| Uncertainty | 0 | Clear |
| **TOTAL** | **4** | **Micro** |

**Correct TTL**: 1 phase, 1-5 tasks

---

## Correct Examples by Band

### Micro (Score 0–10): CLI Utility

**Brief**: "CLI that normalizes a CSV and outputs cleaned file"

**Score**: Product 2, Artifacts 2, Data 2, UX 2, rest 0 → **8 (Micro)**

**TTL** (1 phase, 3 tasks):
```
## Phase 1: Implementation
| T-1.1 | Create CLI with argument parsing and CSV reading |
| T-1.2 | Implement normalization logic and output writing |
| T-1.3 | Add error handling and tests |
```

---

### Tiny (Score 11–22): Personal CRUD App

**Brief**: "Web app to create/list/edit tasks with login; SQLite; simple UI"

**Score**: Product 4, Artifacts 4, Data 4, Security 4, Ops 2, UX 2, Uncertainty 2 → **22 (Tiny)**

**TTL** (2 phases, 8 tasks):
```
## Phase 1: Foundation
| T-1.1 | Project setup with Flask/SQLite and auth |
| T-1.2 | DB schema and migrations |
| T-1.3 | Basic CRUD endpoints |
| T-1.4 | Unit tests for core flows |

## Phase 2: UI & Polish
| T-2.1 | Simple UI for task management |
| T-2.2 | Input validation and error handling |
| T-2.3 | Integration tests |
| T-2.4 | Basic deploy setup |
```

---

### Small (Score 23–34): API Service

**Brief**: "REST API for inventory management with basic auth, PostgreSQL, Docker deploy"

**Score**: Product 6, Artifacts 6, Data 6, Security 6, Ops 6, Uncertainty 4 → **34 (Small)**

**TTL** (4–7 phases, 25-60 tasks):
- Phase 1: Foundation (DB, auth, core models)
- Phase 2: API endpoints + validation
- Phase 3: Admin tooling + data export
- Phase 4: Deploy + observability

---

### Medium (Score 35–46): B2B SaaS

**Brief**: "Multi-tenant dashboard with Admin/Member roles, audit log, CSV import, notifications"

**Score**: Product 6, Artifacts 6, Data 6, Integrations 4, Security 6, Ops 6, Perf 4, UX 4, Uncertainty 4 → **46 (Medium)**

**TTL** (7–12 phases, 60-130 tasks):
- Phase 1: Foundation (tenancy, auth, RBAC)
- Phase 2: Core domain CRUD
- Phase 3: CSV import pipeline
- Phase 4: Notifications + messaging
- Phase 5: Audit logging + compliance
- Phase 6: Admin settings + role management
- Phase 7: Reporting + exports
- Phase 8: Observability + deploy

---

### Large (Score 47–58): Mobile App with Sync

**Brief**: "iOS + Android app, offline-first notes sync, push notifications"

**Score**: Product 8, Artifacts 10, Data 8, Integrations 6, Security 6, Ops 6, Perf 6, UX 8, Migration 0, Uncertainty 0 → **58 (Large)**

**TTL** (12–20 phases, 130-240 tasks):
- App scaffolding + auth
- Local persistence + sync protocol
- Backend sync + conflict handling
- Push notifications
- QA matrix + release pipeline
- Monitoring + rollout

---

## Hidden Work Checklist

When sizing, check if these apply (add tasks if yes):

**Data**: migrations, backfills, seed data, schema evolution
**Integrations**: retries, rate limits, idempotency, webhook verification
**Security**: permission tests, audit schema, access policies
**Ops**: health checks, dashboards, alerts, runbooks, rollback
**Performance**: load testing, caching strategy, capacity planning
