# Security Testing Types

Comprehensive overview of security testing methodologies and when to use each type.

## Overview

```
┌─────────────────────────────────────────────────────────────────────┐
│                    SECURITY TESTING TYPES                            │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  ┌────────────────────────────────────────────────────────────────┐ │
│  │  STATIC APPLICATION SECURITY TESTING (SAST)                    │ │
│  │  ─────────────────────────────────────────                     │ │
│  │  What: Analyzes source code without executing it               │ │
│  │  When: During development, in CI/CD pipeline                   │ │
│  │  Finds: SQL injection, XSS, buffer overflows, hardcoded creds  │ │
│  │  Tools: Semgrep, SonarQube, CodeQL, Checkmarx, Fortify         │ │
│  │                                                                │ │
│  │  Strengths:                         Weaknesses:                │ │
│  │  ✓ Early detection                  ✗ False positives          │ │
│  │  ✓ Full code coverage               ✗ Can't find runtime issues│ │
│  │  ✓ Fast feedback                    ✗ Language-specific        │ │
│  └────────────────────────────────────────────────────────────────┘ │
│                                                                     │
│  ┌────────────────────────────────────────────────────────────────┐ │
│  │  DYNAMIC APPLICATION SECURITY TESTING (DAST)                   │ │
│  │  ──────────────────────────────────────────                    │ │
│  │  What: Tests running application for vulnerabilities           │ │
│  │  When: Against deployed application (staging/production)       │ │
│  │  Finds: XSS, CSRF, authentication flaws, misconfigurations     │ │
│  │  Tools: OWASP ZAP, Burp Suite, Nuclei, Nikto                   │ │
│  │                                                                │ │
│  │  Strengths:                         Weaknesses:                │ │
│  │  ✓ Tests actual behavior            ✗ Requires running app     │ │
│  │  ✓ Low false positives              ✗ Limited code coverage    │ │
│  │  ✓ Language-agnostic                ✗ Slower feedback          │ │
│  └────────────────────────────────────────────────────────────────┘ │
│                                                                     │
│  ┌────────────────────────────────────────────────────────────────┐ │
│  │  SOFTWARE COMPOSITION ANALYSIS (SCA)                           │ │
│  │  ───────────────────────────────────                           │ │
│  │  What: Scans dependencies for known vulnerabilities            │ │
│  │  When: During build, in CI/CD pipeline                         │ │
│  │  Finds: CVEs in libraries, outdated dependencies, license risk │ │
│  │  Tools: Snyk, Dependabot, OWASP Dependency-Check, Trivy        │ │
│  │                                                                │ │
│  │  Strengths:                         Weaknesses:                │ │
│  │  ✓ Catches known CVEs               ✗ Only finds known issues  │ │
│  │  ✓ Automated updates                ✗ Can't find custom vulns  │ │
│  │  ✓ License compliance               ✗ Dependency version lock  │ │
│  └────────────────────────────────────────────────────────────────┘ │
│                                                                     │
│  ┌────────────────────────────────────────────────────────────────┐ │
│  │  INTERACTIVE APPLICATION SECURITY TESTING (IAST)               │ │
│  │  ───────────────────────────────────────                       │ │
│  │  What: Combines SAST + DAST via instrumentation                │ │
│  │  When: During integration/E2E testing                          │ │
│  │  Finds: Runtime vulnerabilities with code context              │ │
│  │  Tools: Contrast Security, Seeker, Hdiv                        │ │
│  │                                                                │ │
│  │  Strengths:                         Weaknesses:                │ │
│  │  ✓ Low false positives              ✗ Requires instrumentation │ │
│  │  ✓ Code-level context               ✗ Performance overhead     │ │
│  │  ✓ Real vulnerability proof         ✗ More complex setup       │ │
│  └────────────────────────────────────────────────────────────────┘ │
│                                                                     │
│  ┌────────────────────────────────────────────────────────────────┐ │
│  │  PENETRATION TESTING                                           │ │
│  │  ───────────────────                                           │ │
│  │  What: Systematic security assessment using attack simulation  │ │
│  │  When: Before releases, after major changes                    │ │
│  │  Finds: Logic flaws, complex attack chains, business logic     │ │
│  │  Methods: Black box, white box, gray box                       │ │
│  │                                                                │ │
│  │  Strengths:                         Weaknesses:                │ │
│  │  ✓ Finds complex issues             ✗ Resource intensive       │ │
│  │  ✓ Business logic testing           ✗ Point-in-time snapshot   │ │
│  │  ✓ Real-world attack simulation     ✗ Requires deep context    │ │
│  └────────────────────────────────────────────────────────────────┘ │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

## Testing Type Selection Matrix

```
┌─────────────────────────────────────────────────────────────────────┐
│                    WHEN TO USE WHICH TEST TYPE                       │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  ┌──────────────────┬────────┬────────┬────────┬────────┬────────┐  │
│  │ Vulnerability    │ SAST   │ DAST   │ SCA    │ IAST   │ Pentest│  │
│  ├──────────────────┼────────┼────────┼────────┼────────┼────────┤  │
│  │ SQL Injection    │ ✓✓     │ ✓✓     │        │ ✓✓✓    │ ✓✓     │  │
│  │ XSS              │ ✓      │ ✓✓     │        │ ✓✓✓    │ ✓✓     │  │
│  │ CSRF             │ ✓      │ ✓✓     │        │ ✓✓     │ ✓✓     │  │
│  │ Hardcoded Creds  │ ✓✓✓    │        │        │        │ ✓      │  │
│  │ Insecure Config  │ ✓      │ ✓✓     │        │ ✓      │ ✓✓     │  │
│  │ Known CVEs       │        │        │ ✓✓✓    │        │ ✓      │  │
│  │ Auth Bypass      │ ✓      │ ✓✓     │        │ ✓✓     │ ✓✓✓    │  │
│  │ Business Logic   │        │ ✓      │        │ ✓      │ ✓✓✓    │  │
│  │ API Security     │ ✓      │ ✓✓     │        │ ✓✓     │ ✓✓✓    │  │
│  │ Cryptographic    │ ✓✓     │ ✓      │        │ ✓      │ ✓✓     │  │
│  └──────────────────┴────────┴────────┴────────┴────────┴────────┘  │
│                                                                     │
│  ✓ = Can detect    ✓✓ = Good at detecting    ✓✓✓ = Best for        │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

## SAST (Static Application Security Testing)

### When to Use
- During development phase
- In pull request reviews
- As part of CI/CD pipeline (pre-merge)
- For compliance audits requiring code analysis

### Key Characteristics
- Analyzes source code, bytecode, or binaries
- No execution required
- Can scan entire codebase
- Language-specific engines

### Best Practices
- Run on every commit or pull request
- Set severity thresholds for blocking builds
- Tune rules to reduce false positives
- Integrate with IDE for real-time feedback
- Create custom rules for organization-specific patterns

## DAST (Dynamic Application Security Testing)

### When to Use
- Against staging or pre-production environments
- After application is deployed
- For black-box testing without source code access
- To validate runtime security controls

### Key Characteristics
- Tests running application
- No source code access needed
- Finds runtime-specific issues
- Requires deployed application

### Best Practices
- Run against staging/test environments (never production)
- Use authenticated scans for protected endpoints
- Configure scans to avoid destructive tests
- Correlate findings with SAST results
- Schedule regular scans (nightly builds)

## SCA (Software Composition Analysis)

### When to Use
- During dependency updates
- In CI/CD pipeline (build phase)
- For license compliance checking
- When evaluating new dependencies

### Key Characteristics
- Scans dependencies and third-party libraries
- Checks against CVE databases
- Tracks license compliance
- Monitors for outdated dependencies

### Best Practices
- Scan on every build
- Auto-create PRs for dependency updates
- Set severity thresholds (block on critical/high)
- Monitor transitive dependencies
- Review license compatibility

## IAST (Interactive Application Security Testing)

### When to Use
- During integration testing
- For applications where instrumentation is feasible
- When high accuracy is required
- For continuous security monitoring

### Key Characteristics
- Requires application instrumentation
- Combines SAST and DAST benefits
- Provides code-level context for runtime issues
- Lower false positive rate

### Best Practices
- Use in non-production environments
- Monitor performance overhead
- Combine with functional testing
- Correlate with SAST/DAST findings
- Use for deep API security testing

## Penetration Testing

### When to Use
- Before major releases
- After significant architecture changes
- For compliance requirements (PCI, HIPAA)
- For critical/high-value applications

### Key Characteristics
- Manual or semi-automated testing
- Simulates real-world attacks
- Tests business logic flaws
- Requires security expertise

### Testing Approaches

**Black Box Testing**
- No prior knowledge of system
- Tests external attack surface
- Simulates external attacker perspective

**White Box Testing**
- Full access to source code and architecture
- Comprehensive security assessment
- Tests internal security controls

**Gray Box Testing**
- Partial knowledge (typical user credentials)
- Balances coverage and realism
- Most common approach

### Best Practices
- Define clear scope and rules of engagement
- Use qualified security professionals
- Test in production-like environment
- Document all findings with proof-of-concept
- Retest after remediation
- Schedule regular assessments (annually minimum)
