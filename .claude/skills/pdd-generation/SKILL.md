---
name: pdd-generation
description: Generates Product Description Documents (PDDs) from project briefs. Use when describing products for stakeholders, defining value propositions and market positioning, documenting target users and market opportunity, establishing go-to-market strategy, or capturing competitive analysis. Focuses on the product's identity, market fit, and strategic positioning rather than detailed implementation requirements.
allowed-tools: Read Write Grep Glob WebFetch
---

# PDD Generation Skill

This skill generates comprehensive Product Description Documents (PDDs) from project briefs (i.e., high-level descriptions).

---

## When to Use This Skill

Use this skill when you need to:
- Describe a product for stakeholders, executives, or investors
- Document value propositions and market positioning
- Define target users and market opportunity (TAM/SAM)
- Establish go-to-market strategy
- Capture competitive analysis and differentiation
- Create a single source of truth for "what the product is"
- Communicate product identity across teams (BD, marketing, sales)

---

## PDD vs PRD: Key Differences

┌──────────────┬─────────────────────────────────────────────────────────────┬─────────────────────────────────────────────────────────────┐
│ Aspect       │ PDD (Product Description Document)                          │ PRD (Product Requirements Document)                         │
├──────────────┼─────────────────────────────────────────────────────────────┼─────────────────────────────────────────────────────────────┤
│ **Purpose**  │ Describe what the product IS                                │ Define what to BUILD                                        │
│ **Audience** │ Stakeholders, executives, BD, sales                         │ Engineering, QA, design teams                               │
│ **Focus**    │ Market positioning, value, identity                         │ Features, requirements, acceptance criteria                 │
│ **Content**  │ Value propositions, market opportunity, competitive analysis│ User stories, functional requirements, technical constraints│
│ **Tone**     │ Strategic, market-facing                                    │ Technical, implementation-focused                           │
│ **Lifespan** │ Evolves with product positioning                            │ Evolves with each release/sprint                            │
└──────────────┴─────────────────────────────────────────────────────────────┴─────────────────────────────────────────────────────────────┘

### When to Use Which

- **Use PDD** when answering: "What is this product and why does it matter?"
- **Use PRD** when answering: "What features should we build and how do we verify them?"

---

## Purpose

This skill provides a systematic approach to PDD creation by:
1. Reading and analyzing the source brief or product concept
2. Extracting key product identity, value propositions, and market positioning
3. Structuring content according to the PDD format
4. Defining target users and market opportunity
5. Capturing competitive landscape and differentiation
6. Producing a complete, stakeholder-ready document

---

## Required PDD Sections

A complete PDD must contain all 15 sections with substantive content:

### 1. Problem Statement
- Clear articulation of the specific problem(s) the product solves
- One sentence, under 50 words

### 2. Product Overview
Three versions at different detail levels:
- **Sound-bite**: Under 10 words
- **Paragraph**: Under 100 words
- **Detailed**: 1-pager with images/diagrams if applicable

### 3. Value Propositions
- Bullet list of 1-5 items
- Each item: Name (under 3 words) and description (under 20 words)
- Focus on unique value delivered to customers/users

### 4. Target Users
- Primary user segments with characteristics
- Needs and pain points addressed
- Both internal and external user types
- Example customers/users for each type

### 5. Market Opportunity
Three sections:
- Overview of market opportunity (under 100 words)
- TAM (Total Addressable Market) description
- SAM (Serviceable Addressable Market) description

### 6. User Stories
- High-level stories capturing key functionality
- Format: "As a [role], I want [goal] so that [benefit]."
- Link to related key capabilities
- Prioritized by business value

### 7. Key Capabilities
- Prioritized list of capabilities/features
- Short descriptions of functionality
- Benefits to users for each capability
- Organized by capability category

### 8. Technical Architecture
- High-level description of software architecture
- Key components and interfaces
- Integration points
- Follows C4 model (Context, Containers, Components, Code)
- Include diagrams with captions (under 100 words each)

### 9. Non-Functional Requirements
- Focus on **critical** NFRs only (keep maintainable)
- Categories: Performance, Security, Reliability, Scalability
- Each requirement: Name (under 5 words), Description (under 50 words)

### 10. Regulatory & Compliance Considerations
- Relevant standards and certifications
- Regulatory requirements for the application domain
- Compliance considerations

### 11. Dependencies & Constraints
- External dependencies
- Technical constraints
- Potential risks to product development

### 12. Development Roadmap
- Timeline of development milestones
- Feature releases with prioritization
- Resource requirements (if known)
- Link to detailed roadmap artifacts

### 13. Success Metrics
- Defined KPIs with specific targets
- Measurement methods for each metric
- Business, user, and technical performance indicators
- Review mechanism (where KPIs are reported)

### 14. Go-to-Market Strategy
- Product positioning
- Pricing strategy (if applicable)
- Promotion and distribution approach
- Target customer acquisition strategy

### 15. Competitive Analysis
- Analysis of competition within the market
- Differentiation points
- Competitive advantages
- Links to detailed competitive analysis documents

### 16. Feedback & Iteration Plan
Three sections:
- What feedback is collected (enumeration)
- How feedback is collected (who and how)
- How feedback is actioned (process)

---

## Quality Standards

The PDD must meet these criteria:

### Completeness
- All 15 required sections present with substantive content
- Value propositions clearly articulated
- Target users well-defined with examples

### Specificity
- **NO placeholder text** like "[TBD]", "[TODO]", "[To be determined]"
- Market opportunity includes actual analysis
- Success metrics have quantifiable targets

### Stakeholder-Readiness
- Language appropriate for executive/stakeholder audience
- Clear value articulation
- Professional presentation quality

### Maintainability
- Sections can be updated independently
- Links to external sources of truth where appropriate
- Clear ownership for ongoing maintenance

### No Time or Effort Estimates
- **NO timeline estimates** in implementation details
- Development roadmap focuses on milestones, not dates (unless known)
- Describe by scope and deliverables, not duration

---

## Generation Process

Follow this systematic process when generating a PDD:

1. **Read the brief** - Analyze the source brief or product concept
2. **Identify the problem** - Articulate the core problem being solved
3. **Define the product** - Create sound-bite, paragraph, and detailed descriptions
4. **Extract value propositions** - Identify unique value delivered
5. **Map target users** - Define user segments and their needs
6. **Analyze market** - Describe TAM/SAM and market opportunity
7. **List capabilities** - Document key features and their benefits
8. **Describe architecture** - High-level technical overview
9. **Capture NFRs** - Critical non-functional requirements only
10. **Document constraints** - Dependencies, risks, compliance
11. **Define success** - KPIs and measurement methods
12. **Outline GTM** - Go-to-market approach
13. **Analyze competition** - Competitive landscape and differentiation
14. **Plan feedback** - How to gather and action user feedback
15. **Review completeness** - Verify all sections complete, no placeholders
16. **Save document** - Use Write tool to save PDD to specified location

---

## Usage Instructions

To use this skill effectively:

1. **Provide the brief** - Ensure product concept or description is available
2. **Specify output location** - Indicate where PDD should be saved (typically `docs/planning/PDD.md`)
3. **Include context** - Provide any market context, competitive info, or constraints
4. **Review output** - Verify stakeholder-readiness and completeness

**Example invocation:**
```
Generate a PDD from product-concept.md and save it to docs/planning/PDD.md
```

---

## Common Pitfalls to Avoid

❌ **Don't:**
- Use placeholder text like [TBD], [TODO], [Coming soon]
- Write vague value propositions ("best in class", "innovative")
- Skip market opportunity analysis
- Focus too heavily on technical details (that's for the PRD/SDD)
- Include exhaustive NFR lists (focus on critical ones)
- Forget competitive differentiation
- Write for engineers instead of stakeholders

✅ **Do:**
- Be specific about value delivered to users
- Include actual market analysis when available
- Keep language stakeholder-appropriate
- Focus on product identity and positioning
- Link to external sources of truth for detailed data
- Ensure every section adds value for the reader
- Keep NFR section focused and maintainable

---

## Relationship to Other Documents

The PDD sits within a documentation ecosystem:

```
PDD (Product Description Document)
 ↓ "What is this product?"
PRD (Product Requirements Document)
 ↓ "What should we build?"
SDD (Software Design Document)
 ↓ "How should we build it?"
TTL (Technical Task List)
 ↓ "What tasks need to be done?"
Implementation
```

- **PDD → PRD**: The PDD informs requirements by establishing product identity
- **PDD → Marketing**: PDD content feeds marketing materials and sales enablement
- **PDD → BD**: Business development uses PDD for customer conversations

---

## Final Validation Checklist

Before saving the PDD, complete this self-check:

- [ ] Problem statement is clear and under 50 words
- [ ] Product overview has all three versions (sound-bite, paragraph, detailed)
- [ ] Value propositions are specific and differentiated (1-5 items)
- [ ] Target users are well-defined with examples
- [ ] Market opportunity includes TAM/SAM analysis
- [ ] User stories follow correct format
- [ ] Key capabilities are prioritized with descriptions
- [ ] Technical architecture follows C4 model
- [ ] NFRs focus on critical requirements only
- [ ] Regulatory/compliance considerations documented
- [ ] Dependencies and constraints identified
- [ ] Success metrics are quantifiable with measurement methods
- [ ] Go-to-market strategy is outlined
- [ ] Competitive analysis includes differentiation
- [ ] Feedback plan covers collection and actioning
- [ ] No placeholder text anywhere in document
- [ ] Language is stakeholder-appropriate

---

## Required Response Format

### Completion Notes

- Set `task_id` to "pdd_generation" in your response
- List "docs/planning/PDD.md" in `artifacts_changed`
- Set `task_status` to "SUCCESS" when PDD is complete

---

## Notes

- The PDD serves as the single source of truth for product identity and positioning
- The PDD should be updated as product strategy evolves
- Product managers own PDD maintenance
- Use the PDD as input for PRD generation and marketing materials

---

## References

📖 **See [references/pdd-template.md](references/pdd-template.md)** for:
- Complete PDD markdown template
- Detailed guidance for each section
- Examples for section content
- Best practices for stakeholder communication
