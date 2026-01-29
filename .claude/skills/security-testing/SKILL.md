---
name: security-testing
description: Designs and implements security testing including SAST, DAST, dependency scanning, and penetration testing methodologies. Use when validating application security, identifying vulnerabilities, designing security test automation, implementing secure development lifecycle practices, or ensuring compliance with security standards.
allowed-tools: Read Glob Grep
---

# Security Testing Skill

This skill helps test engineers design and implement security testing practices to identify vulnerabilities before they reach production. Use it to ensure applications are secure by design and resistant to common attack vectors.

## ⛔ CRITICAL: Test Duration Restriction

All tests you create MUST complete execution in under 1 minute total runtime.
Tests MUST be short, focused, and fast-executing.
NEVER create tests that:
- Require long-running operations (>60 seconds)

If comprehensive testing requires longer duration, break into multiple focused test files that each run quickly.
Tests exceeding 1 minute duration will cause task FAILURE.

**For security testing specifically**:
- Use fast SAST scanners with targeted rulesets
- Run DAST scans against minimal test surfaces (single endpoint or small scope)
- Use pre-recorded vulnerability samples instead of full penetration tests
- Design quick authentication/authorization validation tests (5-10 seconds each)
- Break comprehensive security test suites into multiple fast-running test files
- For full vulnerability scans, design the test harness only - don't execute long scans

## When to Use This Skill

Use this skill when you need to:
- Design security testing strategies for applications
- Implement automated security scanning (SAST, DAST, SCA)
- Plan and execute penetration testing
- Validate security controls and defenses
- Ensure compliance with security standards (OWASP, etc.)
- Integrate security testing into CI/CD pipelines
- Assess application vulnerabilities and risks
- Test authentication and authorization mechanisms

## Purpose

This skill provides a systematic approach to security testing:

1. **SAST (Static Analysis)**: Find vulnerabilities in source code before execution
2. **DAST (Dynamic Analysis)**: Test running applications for security flaws
3. **SCA (Software Composition Analysis)**: Identify vulnerable dependencies
4. **Penetration Testing**: Systematic attack simulation and security assessment
5. **Security Automation**: Integrate security into DevSecOps pipelines
6. **Compliance Validation**: Ensure adherence to security standards

---

## Security Testing Overview

### Core Testing Types

**Static Analysis (SAST)**
- Analyzes source code without execution
- Detects SQL injection, XSS, hardcoded credentials
- Fast feedback during development
- Tools: Semgrep, SonarQube, CodeQL, Bandit

**Dynamic Analysis (DAST)**
- Tests running applications for vulnerabilities
- Finds runtime issues, misconfigurations, authentication flaws
- Language-agnostic, black-box testing
- Tools: OWASP ZAP, Burp Suite, Nuclei

**Software Composition Analysis (SCA)**
- Scans dependencies for known CVEs
- License compliance checking
- Automated vulnerability updates
- Tools: Snyk, Dependabot, Trivy, OWASP Dependency-Check

**Penetration Testing**
- Manual/semi-automated security assessment
- Simulates real-world attacks
- Tests business logic and complex vulnerabilities
- Methods: Black box, white box, gray box

**Interactive Testing (IAST)**
- Combines SAST + DAST via instrumentation
- Low false positives, code-level context
- Runtime vulnerability detection
- Tools: Contrast Security, Seeker

### When to Use Each Type

| Vulnerability Type   | SAST | DAST | SCA | IAST | Pentest |
|---------------------|------|------|-----|------|---------|
| SQL Injection       | ✓✓   | ✓✓   |     | ✓✓✓  | ✓✓      |
| XSS                 | ✓    | ✓✓   |     | ✓✓✓  | ✓✓      |
| Hardcoded Secrets   | ✓✓✓  |      |     |      | ✓       |
| Known CVEs          |      |      | ✓✓✓ |      | ✓       |
| Auth Bypass         | ✓    | ✓✓   |     | ✓✓   | ✓✓✓     |
| Business Logic      |      | ✓    |     | ✓    | ✓✓✓     |

**See**: `references/testing-types.md` for detailed comparisons, strengths/weaknesses, and best practices.

---

## OWASP Top 10 Testing

The OWASP Top 10 represents the most critical web application security risks. Every security testing strategy should cover these vulnerabilities:

1. **A01: Broken Access Control** - Test IDOR, privilege escalation
2. **A02: Cryptographic Failures** - Verify TLS, password hashing, encryption
3. **A03: Injection** - Test SQL, command, LDAP injection
4. **A04: Insecure Design** - Validate rate limiting, business logic
5. **A05: Security Misconfiguration** - Check headers, defaults, error handling
6. **A06: Vulnerable Components** - Scan dependencies for CVEs
7. **A07: Authentication Failures** - Test password policy, session management, MFA
8. **A08: Integrity Failures** - Check deserialization, code signing
9. **A09: Logging Failures** - Verify security event logging
10. **A10: SSRF** - Test internal resource access prevention

**See**: `references/owasp-top-10.md` for comprehensive test cases, attack vectors, and validation steps for each vulnerability type.

---

## Security Testing Tools

### Tool Selection by Category

**SAST Tools**
- **Semgrep**: Multi-language, custom rules, fast CI/CD integration
- **SonarQube**: Code quality + security, comprehensive reporting
- **CodeQL**: Deep semantic analysis, GitHub integration
- **Language-specific**: Bandit (Python), gosec (Go), Brakeman (Ruby)

**DAST Tools**
- **OWASP ZAP**: Free, CI/CD friendly, API scanning
- **Burp Suite**: Industry standard, comprehensive features
- **Nuclei**: Template-based, fast scanning, 5000+ templates
- **Specialized**: SQLMap (SQL injection), XSStrike (XSS detection)

**SCA Tools**
- **Snyk**: Developer-friendly, auto-fix PRs, multi-ecosystem
- **Dependabot**: GitHub native, automatic updates
- **Trivy**: Container + dependency scanning, IaC support
- **OWASP Dependency-Check**: Free, Java/.NET focused

**Secret Detection**
- **GitLeaks**: Pre-commit hooks, git history scanning
- **TruffleHog**: Entropy-based detection, deep scans
- **detect-secrets**: Low false positives, allowlisting

**Container/Infrastructure**
- **Trivy**: All-in-one container and IaC scanning
- **Checkov**: Terraform, CloudFormation, Kubernetes security
- **kube-bench**: Kubernetes CIS benchmark
- **Prowler**: AWS security assessment

**See**: `references/tools-catalog.md` for detailed tool comparisons, installation instructions, usage examples, and selection guidance.

---

## CI/CD Security Integration

### DevSecOps Pipeline Phases

**1. Development Phase**
- IDE security plugins (real-time feedback)
- Pre-commit hooks (secret scanning, linting)
- Commit signing (integrity verification)
- Pull request security checks (SAST, SCA)

**2. Build Phase**
- SAST scan (source code vulnerabilities)
- SCA scan (dependency CVEs)
- Container scan (base image vulnerabilities)
- IaC scan (infrastructure security)
- **Gate**: Block on critical/high vulnerabilities

**3. Test Phase**
- DAST scan (running application)
- IAST instrumentation (optional)
- Security unit tests (auth/authz)
- API security tests (OpenAPI validation)
- **Gate**: Block deployment on security test failures

**4. Deploy Phase**
- Configuration audit (production settings)
- Secrets management (Vault/KMS integration)
- Runtime protection (WAF/RASP deployment)
- Compliance checks (PCI, SOC2, GDPR)

**5. Operate Phase**
- Security monitoring (SIEM integration)
- Threat detection (IDS/IPS)
- Vulnerability management (patching schedule)
- Incident response (automated playbooks)

### Integration Examples

**GitHub Actions Pipeline**
```yaml
jobs:
  security-scan:
    steps:
      - name: Secret Scanning
        uses: gitleaks/gitleaks-action@v2
      - name: SAST
        uses: semgrep/semgrep-action@v1
      - name: SCA
        uses: snyk/actions/node@master
      - name: Container Scan
        uses: aquasecurity/trivy-action@master
```

**GitLab CI Pipeline**
```yaml
include:
  - template: Security/SAST.gitlab-ci.yml
  - template: Security/Dependency-Scanning.gitlab-ci.yml
  - template: Security/Secret-Detection.gitlab-ci.yml
```

**See**: `references/cicd-integration.md` for complete pipeline configurations, Jenkins/CircleCI examples, pre-commit hook setup, and security gate patterns.

---

## Core Security Test Cases

### Authentication Testing

**Critical Test Areas**:
- Password security (complexity, hashing, storage)
- Brute force protection (rate limiting, lockouts, CAPTCHA)
- Session management (timeout, regeneration, secure flags)
- Multi-factor authentication (bypass prevention, code expiration)
- Credential recovery (token security, user enumeration prevention)

**Key Test Scenarios**:
```python
# Test account lockout
for i in range(max_attempts):
    login(username, "wrong_password")
assert next_login_attempt() == 429  # Too Many Requests

# Test session regeneration
initial_session = get_session_id()
login(username, password)
new_session = get_session_id()
assert initial_session != new_session
```

**See**: `references/authentication-testing.md` for complete test cases, code examples, and security best practices.

### Authorization Testing

**Critical Test Areas**:
- Access control (RBAC, ABAC enforcement)
- Horizontal privilege escalation (user A accessing user B data)
- Vertical privilege escalation (user accessing admin functions)
- IDOR vulnerabilities (insecure direct object references)
- API authorization (JWT validation, API key scopes)

**Key Test Scenarios**:
```python
# Test IDOR prevention
user1 = login_as_user(user_id=123)
response = user1.get("/api/users/456/orders")
assert response.status_code == 403

# Test vertical escalation prevention
user_session = login_as_role("user")
response = user_session.get("/api/admin/users")
assert response.status_code == 403
```

**See**: `references/authorization-testing.md` for RBAC/ABAC testing, privilege escalation vectors, and JWT security tests.

### Input Validation Testing

**Critical Test Areas**:
- SQL injection (classic, union-based, blind)
- Cross-site scripting (reflected, stored, DOM-based)
- Command injection (OS command execution)
- Path traversal (directory traversal, file access)
- XML injection (XXE, XPath injection)

**Key Test Payloads**:
```python
# SQL injection test
sql_payloads = [
    "' OR '1'='1' --",
    "'; DROP TABLE users; --",
    "' UNION SELECT username, password FROM users --"
]

# XSS test
xss_payloads = [
    "<script>alert('XSS')</script>",
    "<img src=x onerror=alert('XSS')>",
    "<svg onload=alert('XSS')>"
]
```

**See**: `references/input-validation-testing.md` for comprehensive injection payloads, test scripts, and validation best practices.

---

## Vulnerability Management

### Severity Classification (CVSS)

**Critical (9.0-10.0)**
- Remote code execution, authentication bypass
- **Priority**: Emergency patch (24-48 hours)
- **Action**: Block deployment immediately

**High (7.0-8.9)**
- Stored XSS, privilege escalation, IDOR with PII
- **Priority**: Scheduled patch (7 days)
- **Action**: Block PR merge

**Medium (4.0-6.9)**
- Reflected XSS, information disclosure, missing headers
- **Priority**: Backlog (30 days)
- **Action**: Warning, track for remediation

**Low (0.1-3.9)**
- Version disclosure, verbose errors, best practices
- **Priority**: Technical debt (90 days)
- **Action**: Informational, document

### Vulnerability Workflow

```
1. Discovery → 2. Validation → 3. Severity Classification → 4. Impact Analysis
                                                                      ↓
8. Verification ← 7. Tracking ← 6. Assignment ← 5. Prioritization
```

### Key Metrics

- **MTTD (Mean Time to Detect)**: Time from introduction to detection
- **MTTR (Mean Time to Remediate)**: Time from detection to fix
- **SLA Compliance**: % fixed within target timeframes
- **Vulnerability Density**: Vulnerabilities per 1000 LOC
- **Recurrence Rate**: % of vulnerabilities that reappear

**See**: `references/vulnerability-management.md` for CVSS calculation, triage processes, tracking templates, and remediation verification.

---

## Workflow

When designing and implementing security testing:

1. **Threat Modeling**: Identify assets, threats, and attack vectors
   - What sensitive data needs protection?
   - Who are potential attackers?
   - What are the trust boundaries?

2. **Tool Selection**: Choose appropriate security testing tools
   - Match tools to tech stack and languages
   - Consider CI/CD integration requirements
   - Balance coverage vs. performance

3. **Pipeline Integration**: Integrate security into CI/CD
   - Development: IDE plugins, pre-commit hooks
   - Build: SAST, SCA, container scanning
   - Test: DAST, security unit tests
   - Deploy: Configuration audits, compliance checks

4. **Test Automation**: Write automated security test cases
   - Authentication and authorization tests
   - Input validation tests (injection, XSS)
   - API security tests
   - Security regression tests

5. **Triage and Prioritize**: Classify vulnerabilities by severity
   - Validate findings (eliminate false positives)
   - Calculate CVSS scores
   - Assess business impact
   - Set remediation priorities

6. **Remediate**: Fix vulnerabilities based on severity
   - Critical: Emergency patch (24-48 hours)
   - High: Scheduled fix (7 days)
   - Medium: Backlog (30 days)
   - Low: Technical debt (90 days)

7. **Verify**: Confirm vulnerabilities are resolved
   - Retest with original exploit
   - Run security scanners
   - Add regression tests
   - Update documentation

8. **Monitor**: Continuous security monitoring
   - Track new vulnerabilities in dependencies
   - Monitor for attack patterns
   - Measure security metrics (MTTR, SLA compliance)
   - Regular security assessments

---

## Question Strategy

When designing security tests, ask:

### Threat Assessment
1. "What sensitive data does this application handle?"
2. "What are the most critical user workflows?"
3. "What external systems does the application integrate with?"
4. "What are the trust boundaries in the system?"
5. "What compliance requirements apply (PCI, HIPAA, GDPR)?"

### Test Planning
1. "What security tools are already in use?"
2. "What is the current test coverage for security?"
3. "What is the acceptable false positive rate?"
4. "What is the response SLA for different severity levels?"
5. "How are security findings tracked and remediated?"

### Tool Configuration
1. "Which languages and frameworks are used?"
2. "What is the deployment environment (cloud, on-prem, hybrid)?"
3. "What authentication mechanisms are in place?"
4. "Are there existing security baselines or policies?"
5. "What is the budget for security tools?"

---

## Best Practices

1. **Shift Left**: Integrate security testing early in development
   - IDE plugins for real-time feedback
   - Pre-commit hooks for secret scanning
   - SAST on every pull request

2. **Automate**: Include security scans in every CI/CD pipeline run
   - Run on every commit/PR
   - Set appropriate severity thresholds
   - Fast feedback loops

3. **Layer Defense**: Use multiple testing types (SAST + DAST + SCA)
   - Complementary coverage
   - Reduce false negatives
   - Defense in depth

4. **Prioritize**: Focus on high-impact vulnerabilities first
   - Use CVSS as baseline
   - Consider business context
   - Track MTTR by severity

5. **Track Metrics**: Measure mean time to remediation (MTTR)
   - Monitor SLA compliance
   - Track vulnerability trends
   - Measure security debt

6. **Educate**: Train developers on secure coding practices
   - Security awareness training
   - Secure coding guidelines
   - Vulnerability case studies

7. **Stay Current**: Keep tools and vulnerability databases updated
   - Regular tool updates
   - CVE database refreshes
   - Follow security advisories

8. **Validate Fixes**: Always verify that vulnerabilities are truly fixed
   - Retest with original exploit
   - Add regression tests
   - Document remediation

---

## Reference Documentation

This skill includes detailed reference documentation for in-depth guidance:

- **`references/testing-types.md`**: Comprehensive comparison of SAST, DAST, SCA, IAST, and penetration testing with strengths, weaknesses, and use cases
- **`references/owasp-top-10.md`**: Complete test cases for all OWASP Top 10 vulnerabilities with attack vectors and validation steps
- **`references/tools-catalog.md`**: Security testing tools organized by category with installation, usage, and selection guidance
- **`references/cicd-integration.md`**: DevSecOps pipeline integration patterns for GitHub Actions, GitLab CI, Jenkins, and CircleCI
- **`references/authentication-testing.md`**: Comprehensive authentication test cases including password security, session management, and MFA
- **`references/authorization-testing.md`**: Authorization test cases covering RBAC, ABAC, IDOR, and privilege escalation
- **`references/input-validation-testing.md`**: Input validation test cases with injection payloads for SQL, XSS, command injection, and path traversal

Consult these references for detailed technical guidance, code examples, and comprehensive testing strategies.
