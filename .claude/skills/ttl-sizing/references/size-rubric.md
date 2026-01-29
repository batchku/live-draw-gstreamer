# Size Score Rubric (0–100)

Use this for detailed scoring guidance. Score 10 dimensions from 0–10 each, then total to 0–100.

**Rule of thumb**: If you're "hoping" something will be easy, score higher.

---

## Quick Scoring Guide

| Dimension | 0 | 2 | 4 | 6 | 8 | 10 |
|-----------|---|---|---|---|---|----|
| **Product surface** | one tiny flow | 1-2 flows | 3-5 flows | 6-10 flows, roles | >10 flows, admin | enterprise-scale workflows |
| **Artifacts** | single script | +packaging | FE+BE | 3-4 artifacts | multi-client | multi-org/multi-region stack |
| **Data model** | none/in-memory | single table | 2-4 entities | 5-8 entities | complex relations | multi-tenant + large migrations |
| **Integrations** | none | 1 simple API | 1 serious or 2 simple | multiple serious | payments/SSO | regulated/mission-critical partners |
| **Security** | no auth | API key/basic | roles | RBAC+audit | regulated | formal compliance + external audits |
| **Ops** | local only | simple deploy | multi-env | dashboards+alerts | HA/SLOs | multi-region + DR requirements |
| **Performance** | negligible | "fast enough" | clear constraints | real-time/streaming | strict SLOs | ultra-low latency/high throughput |
| **UX/design** | CLI/basic UI | few screens | multi-screen UI | complex workflows | custom design system | highly polished/accessibility-first |
| **Migration/legacy** | greenfield | light import | minor migration | backfills/dual-write | zero-downtime migration | legacy replacement/regulatory cutover |
| **Uncertainty** | crystal clear | minor TBDs | key unknowns | many unknowns | high risk | discovery-heavy/unknown unknowns |

---

## Detailed Dimension Guidance

### Product Surface (user journeys + workflows)
- **0**: Single command, single endpoint, one tiny flow
- **2**: 1–2 straightforward flows
- **4**: 3–5 flows; mild branching; a couple validations
- **6**: 6–10 flows; multiple roles or complex states
- **8**: >10 flows; admin/ops flows; lots of conditional behavior
- **10**: Enterprise-scale workflows across multiple personas

**Watch for**: "admin UI", "settings", "audit view", "import/export" — if mentioned, count them.

### Artifacts/Platforms
- **0**: One artifact (single script/service/library)
- **2**: One artifact + packaging/distribution
- **4**: Two artifacts (API + web UI; service + worker)
- **6**: 3–4 artifacts
- **8**: Multi-client (web + iOS/Android), multi-service
- **10**: Multi-org, multi-region stack with dedicated pipelines

**Watch for**: "just a cron job" still needs deploy, env config, secrets, retries, alerting.

### Data Model
- **0**: No persistence; in-memory only
- **2**: Single table/collection/file; simple fields
- **4**: 2–4 entities with basic relationships
- **6**: 5–8 entities; migrations; history/audit
- **8**: Complex relational, multi-tenancy, large backfills
- **10**: High-volume data, multiple schemas, ongoing migrations

**Watch for**: "import CSVs", "sync with X", "keep history", "undo", "audit trail".

### Integrations
- **0**: None (libraries don't count unless fragile)
- **2**: 1 simple integration (read-only, stable SDK)
- **4**: 1 serious (OAuth, webhooks) OR 2 simple
- **6**: Multiple serious, event-driven
- **8**: Payments/SSO/enterprise APIs, strict SLAs
- **10**: Regulated/mission-critical integrations with strict contracts

**Hidden work**: auth, token refresh, retries, rate limits, idempotency, webhook verification.

### Security/Compliance
- **0**: No auth; no sensitive data
- **2**: Basic auth (single user, API key)
- **4**: Multiple roles; basic authorization
- **6**: RBAC/ABAC, audit logging, PII handling
- **8**: Regulated (HIPAA/PCI/GDPR), SSO/SAML
- **10**: Formal compliance, external audits, complex policies

### Ops/Reliability
- **0**: Local-only; no real deployment
- **2**: Simple deploy, basic CI, basic logs
- **4**: Multi-env (staging/prod), health checks
- **6**: Dashboards + alerting, runbooks, backup/restore
- **8**: HA, multi-region, zero-downtime, strict SLOs
- **10**: Multi-region active-active, DR exercises, 24/7 on-call

### Performance/Scale
- **0**: None stated; small usage
- **2**: "Should be fast enough"
- **4**: Clear non-trivial constraints; background processing
- **6**: Real-time, concurrency, streaming, large data
- **8**: Strict SLOs, very high throughput, complex caching
- **10**: Ultra-low latency, massive scale, tight capacity bounds

### UX/Design Complexity
- **0**: CLI or basic UI; no custom design
- **2**: Few screens; standard components
- **4**: Multi-screen UI; moderate interaction design
- **6**: Complex workflows; custom components or navigation
- **8**: Design system work; accessibility requirements
- **10**: Highly polished UX, multi-device, extensive usability work

### Migration/Legacy Impact
- **0**: Greenfield; no legacy constraints
- **2**: Light import/migration; small data volume
- **4**: Minor migration with backfills
- **6**: Dual-write or sync during transition
- **8**: Zero-downtime migration; legacy dependencies
- **10**: Legacy replacement with regulatory/contractual cutover

### Uncertainty Buffer
- **0**: Brief is precise (flows, constraints, data, integrations, success criteria)
- **2**: Minor unknowns (a couple TBDs)
- **4**: Key unknowns (data model unclear; integration details missing)
- **6**: Many unknowns; "we'll figure it out" items
- **8**: High risk/ambiguity; needs discovery/spikes
- **10**: Discovery-heavy; requirements likely to shift

---

## Domain Modifiers

Interpret evidence in domain-appropriate ways:

- **Web SaaS**: Product surface + Security + UX/design dominate
- **Mobile**: Artifacts jumps (iOS/Android), higher Ops (distribution, device QA)
- **Data pipelines**: Data model + Ops + Performance dominate
- **ML systems**: Treat training/eval/deploy as artifacts; add uncertainty
- **Infra/DevOps**: Ops is usually 4–8 by default
- **Embedded/IoT**: Performance often higher; add testing harness
- **Migration-heavy**: Migration/legacy drives size; account for cutovers

---

## Score → Band Reference

| Score | Band | Phases | Tasks |
|------:|------|-------:|------:|
| 0–10 | Micro | 1 | 1–5 |
| 11–22 | Tiny | 2–4 | 5–25 |
| 23–34 | Small | 4–7 | 25–60 |
| 35–46 | Medium | 7–12 | 60–130 |
| 47–58 | Large | 12–20 | 130–240 |
| 59–70 | Mega | 20–30 | 240–360 |
| 71–85 | Huge | 30–45 | 360–500 |
| 86–100 | Enterprise | 50+ | 500+ |
