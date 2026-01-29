---
name: analyze-codebase
description: Produces onboarding and analysis documentation for existing codebases, covering architecture, build/run/test guidance, extension points, bugfix workflows, refactoring guidance, and required diagrams.
allowed-tools: Read Write Edit Glob Bash
---

# Analyze Codebase Skill

Use this skill when onboarding to an existing codebase or refining a mature project. The output documents what the system does, how to run it, and how to extend or maintain it.

## Required Outputs

- `docs/analysis/ONBOARDING.md` based on the template in `templates/analysis-template.md`
- `docs/analysis/diagrams/` containing one or more `.puml` diagrams as needed
  - Include component and/or sequence diagrams that match the architecture scope
  - Use multiple diagrams when the system has multiple key flows or subsystems

## Instructions

1. **Read key project documentation**
   - Start with `README.md`, `docs/`, and build/test scripts.
   - Identify the primary entry points, main modules, and runtime flows.

2. **Follow the template**
   - Use `templates/analysis-template.md` as the structure for the onboarding document.
   - Keep headings intact and fill in each section with project-specific details.

3. **Document build, run, and test steps**
   - Provide concrete commands for setup, build, run, and test workflows.
   - Call out required tooling, environment variables, or external services.

4. **Surface extension points**
   - Identify extension mechanisms (plugins, configs, SDKs, hooks, APIs, etc.).
   - Provide examples or file references where available.

5. **Generate diagrams**
   - Use the `c4-diagrams` skill for PlantUML guidance.
   - Create as many component and sequence diagrams as needed for key subsystems and flows.
   - Keep each diagram small and focused on a specific runtime path or subsystem.

6. **Capture maintenance guidance**
   - Explain how to add tests, fix bugs, and perform refactors safely.
   - Note coding standards and documentation expectations.

## Output Format

Follow the response schema and list all created or updated files in `artifacts_changed`.
