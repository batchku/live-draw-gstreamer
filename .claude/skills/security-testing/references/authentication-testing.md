# Authentication Testing Guide

Comprehensive test cases for authentication security.

## Authentication Test Cases Overview

```
┌─────────────────────────────────────────────────────────────────────┐
│                    AUTHENTICATION TEST CASES                         │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  PASSWORD SECURITY                                                  │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │ □ Test password complexity requirements enforced              │  │
│  │ □ Test minimum length requirement (12+ chars recommended)     │  │
│  │ □ Verify passwords are hashed (bcrypt, Argon2)                │  │
│  │ □ Test password history enforcement                           │  │
│  │ □ Verify no password hints stored                             │  │
│  │ □ Test password change requires current password              │  │
│  └───────────────────────────────────────────────────────────────┘  │
│                                                                     │
│  BRUTE FORCE PROTECTION                                             │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │ □ Test account lockout after N failed attempts                │  │
│  │ □ Verify rate limiting on login endpoint                      │  │
│  │ □ Test CAPTCHA triggers after failures                        │  │
│  │ □ Verify lockout applies across different IPs                 │  │
│  │ □ Test unlock mechanism (time-based, admin, etc.)             │  │
│  └───────────────────────────────────────────────────────────────┘  │
│                                                                     │
│  SESSION MANAGEMENT                                                 │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │ □ Test session timeout on inactivity                          │  │
│  │ □ Verify session invalidation on logout                       │  │
│  │ □ Test concurrent session handling                            │  │
│  │ □ Verify session token is regenerated after login             │  │
│  │ □ Test session fixation prevention                            │  │
│  │ □ Verify secure cookie flags (HttpOnly, Secure, SameSite)     │  │
│  │ □ Test session token entropy (unpredictable)                  │  │
│  └───────────────────────────────────────────────────────────────┘  │
│                                                                     │
│  MULTI-FACTOR AUTHENTICATION                                        │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │ □ Test MFA bypass attempts                                    │  │
│  │ □ Verify MFA codes are time-limited                           │  │
│  │ □ Test MFA code reuse prevention                              │  │
│  │ □ Verify backup codes work correctly                          │  │
│  │ □ Test MFA recovery process security                          │  │
│  └───────────────────────────────────────────────────────────────┘  │
│                                                                     │
│  CREDENTIAL RECOVERY                                                │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │ □ Test password reset token expiration                        │  │
│  │ □ Verify reset token is single-use                            │  │
│  │ □ Test for user enumeration via reset flow                    │  │
│  │ □ Verify reset link sent to registered email only             │  │
│  │ □ Test security questions resistance to social engineering    │  │
│  └───────────────────────────────────────────────────────────────┘  │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

## Password Security Testing

### Password Complexity Requirements

**Test Cases**:
1. Minimum length enforcement (recommended: 12+ characters)
2. Character type requirements (upper, lower, digits, special)
3. Dictionary word rejection
4. Common password rejection (e.g., "Password123!")
5. Password strength meter accuracy

**Test Script Example**:
```python
def test_password_complexity():
    weak_passwords = [
        "pass",           # Too short
        "password",       # Dictionary word
        "12345678",       # Only digits
        "Password123",    # Common pattern
        "qwerty123",      # Keyboard pattern
    ]

    for pwd in weak_passwords:
        response = create_account(password=pwd)
        assert response.status_code == 400
        assert "password" in response.json()["errors"]

def test_strong_password_accepted():
    strong_pwd = "MyS3cur3P@ssw0rd!2024"
    response = create_account(password=strong_pwd)
    assert response.status_code == 201
```

### Password Storage

**Verification Steps**:
1. Confirm passwords are never stored in plaintext
2. Verify use of strong hashing algorithm (bcrypt, Argon2, scrypt)
3. Check for proper salting (unique per password)
4. Verify work factor/cost parameter is adequate
5. Confirm passwords are not logged

**Database Inspection**:
```sql
-- Verify password column contains hashes, not plaintext
SELECT id, username, password_hash
FROM users
LIMIT 5;

-- Check hash format (bcrypt starts with $2a$, $2b$, or $2y$)
-- Argon2 starts with $argon2
```

**Test for bcrypt**:
```python
import bcrypt

def test_password_hashing():
    password = "MySecurePassword123!"
    # Password should be hashed
    user = create_user(password=password)

    # Verify hash format
    assert user.password_hash.startswith('$2b$')

    # Verify password verification works
    assert bcrypt.checkpw(
        password.encode('utf-8'),
        user.password_hash.encode('utf-8')
    )
```

### Password Change Security

**Test Cases**:
1. Require current password to change password
2. Prevent reuse of recent passwords (password history)
3. Force logout of all sessions after password change
4. Send notification email on password change
5. Require re-authentication for sensitive operations

**Test Script**:
```python
def test_password_change_requires_current():
    session = login("user@example.com", "OldPassword123!")

    # Attempt to change without current password
    response = session.post("/api/password/change", json={
        "new_password": "NewPassword456!"
    })
    assert response.status_code == 400

    # Change with current password
    response = session.post("/api/password/change", json={
        "current_password": "OldPassword123!",
        "new_password": "NewPassword456!"
    })
    assert response.status_code == 200

    # Verify old password no longer works
    response = login("user@example.com", "OldPassword123!")
    assert response.status_code == 401
```

## Brute Force Protection

### Account Lockout Testing

**Test Cases**:
1. Lock account after N failed attempts (typically 5-10)
2. Verify lockout applies across different IPs
3. Test time-based unlock (e.g., 30 minutes)
4. Verify admin can manually unlock
5. Check that lockout doesn't reveal if account exists

**Test Script**:
```python
def test_account_lockout():
    username = "testuser@example.com"
    max_attempts = 5

    # Attempt failed logins
    for i in range(max_attempts):
        response = login(username, "WrongPassword")
        assert response.status_code == 401

    # Next attempt should be locked
    response = login(username, "WrongPassword")
    assert response.status_code == 429  # Too Many Requests
    assert "locked" in response.json()["error"].lower()

    # Even correct password should be locked
    response = login(username, "CorrectPassword")
    assert response.status_code == 429

def test_lockout_across_ips():
    username = "testuser@example.com"

    # Failed attempts from IP 1
    for i in range(3):
        login(username, "wrong", source_ip="192.168.1.1")

    # Failed attempts from IP 2
    for i in range(3):
        login(username, "wrong", source_ip="192.168.1.2")

    # Should be locked from any IP
    response = login(username, "wrong", source_ip="192.168.1.3")
    assert response.status_code == 429
```

### Rate Limiting

**Test Cases**:
1. Verify rate limit on login endpoint (e.g., 10 attempts per minute)
2. Test rate limit by IP address
3. Test rate limit by username
4. Verify rate limit headers (X-RateLimit-Remaining)
5. Test exponential backoff

**Test Script**:
```python
import time

def test_login_rate_limit():
    username = "testuser@example.com"
    rate_limit = 10  # attempts per minute

    # Make requests up to limit
    for i in range(rate_limit):
        response = login(username, "password")
        assert response.status_code in [200, 401]

    # Next request should be rate limited
    response = login(username, "password")
    assert response.status_code == 429
    assert "X-RateLimit-Limit" in response.headers
    assert "X-RateLimit-Reset" in response.headers
```

### CAPTCHA Implementation

**Test Cases**:
1. Verify CAPTCHA triggers after N failed attempts
2. Test that CAPTCHA is required for subsequent attempts
3. Verify CAPTCHA tokens are single-use
4. Test CAPTCHA bypass attempts
5. Check accessibility (audio CAPTCHA alternative)

## Session Management Testing

### Session Lifecycle

**Test Cases**:
1. Session created on successful login
2. Session contains unpredictable token (high entropy)
3. Session regenerated after login (prevent fixation)
4. Session invalidated on logout
5. Session expires after inactivity timeout

**Test Script**:
```python
def test_session_regeneration_on_login():
    session = requests.Session()

    # Get initial session (anonymous)
    session.get("http://example.com/")
    initial_session_id = session.cookies.get("session_id")

    # Login
    session.post("/api/login", json={
        "username": "testuser",
        "password": "password"
    })
    post_login_session_id = session.cookies.get("session_id")

    # Session ID should have changed
    assert initial_session_id != post_login_session_id

def test_session_invalidation_on_logout():
    session = login("testuser", "password")
    session_id = session.cookies.get("session_id")

    # Logout
    session.post("/api/logout")

    # Try to use old session
    old_session = requests.Session()
    old_session.cookies.set("session_id", session_id)
    response = old_session.get("/api/profile")
    assert response.status_code == 401
```

### Session Cookie Security

**Cookie Flags to Verify**:
- `HttpOnly`: Prevents JavaScript access
- `Secure`: Only sent over HTTPS
- `SameSite`: Prevents CSRF attacks

**Test Script**:
```python
def test_secure_cookie_flags():
    response = login("testuser", "password")

    session_cookie = response.cookies.get("session_id")
    assert session_cookie is not None

    # Check cookie attributes
    cookie_header = response.headers.get("Set-Cookie")
    assert "HttpOnly" in cookie_header
    assert "Secure" in cookie_header
    assert "SameSite=Strict" in cookie_header or \
           "SameSite=Lax" in cookie_header
```

### Session Fixation Prevention

**Attack Scenario**:
1. Attacker gets a session ID from the app
2. Attacker tricks victim into using that session ID
3. Victim logs in with the attacker's session
4. Attacker now has access to victim's account

**Test for Protection**:
```python
def test_session_fixation_prevention():
    # Attacker gets a session
    attacker_session = requests.Session()
    attacker_session.get("http://example.com/")
    attacker_session_id = attacker_session.cookies.get("session_id")

    # Victim logs in with attacker's session ID
    victim_session = requests.Session()
    victim_session.cookies.set("session_id", attacker_session_id)
    victim_session.post("/api/login", json={
        "username": "victim",
        "password": "password"
    })
    victim_session_id = victim_session.cookies.get("session_id")

    # Session ID should have changed (preventing fixation)
    assert attacker_session_id != victim_session_id

    # Attacker's session should not have access
    attacker_session.cookies.set("session_id", attacker_session_id)
    response = attacker_session.get("/api/profile")
    assert response.status_code == 401
```

### Session Timeout

**Test Cases**:
1. Absolute timeout (e.g., 12 hours max)
2. Idle timeout (e.g., 30 minutes of inactivity)
3. Configurable timeouts based on sensitivity
4. Warning before session expires
5. Automatic session renewal on activity

**Test Script**:
```python
import time

def test_session_idle_timeout():
    session = login("testuser", "password")

    # Access protected resource
    response = session.get("/api/profile")
    assert response.status_code == 200

    # Wait beyond idle timeout (e.g., 31 minutes)
    # In test, we'd mock time or use shorter timeout
    time.sleep(31 * 60)

    # Session should be expired
    response = session.get("/api/profile")
    assert response.status_code == 401
```

## Multi-Factor Authentication (MFA)

### MFA Bypass Testing

**Test Cases**:
1. Verify MFA cannot be skipped
2. Test MFA with invalid code
3. Test MFA with expired code
4. Test MFA code reuse
5. Test MFA backup codes

**Test Script**:
```python
def test_mfa_required():
    # Login with correct password
    response = requests.post("/api/login", json={
        "username": "user@example.com",
        "password": "CorrectPassword123!"
    })
    assert response.status_code == 200
    assert response.json()["mfa_required"] == True

    # Try to access protected resource without MFA
    session = requests.Session()
    session.cookies.set("temp_session", response.cookies.get("temp_session"))
    response = session.get("/api/profile")
    assert response.status_code == 401

def test_mfa_code_expiration():
    session = login_step1("user@example.com", "password")

    # Wait for code to expire (e.g., 5 minutes)
    time.sleep(5 * 60)

    # Attempt to use expired code
    response = session.post("/api/mfa/verify", json={
        "code": "123456"
    })
    assert response.status_code == 400
    assert "expired" in response.json()["error"].lower()

def test_mfa_code_reuse():
    session = login_step1("user@example.com", "password")
    code = get_mfa_code_from_authenticator()

    # Use code once
    response = session.post("/api/mfa/verify", json={"code": code})
    assert response.status_code == 200

    # Try to reuse same code
    session2 = login_step1("user@example.com", "password")
    response = session2.post("/api/mfa/verify", json={"code": code})
    assert response.status_code == 400
```

## Credential Recovery Testing

### Password Reset Flow

**Test Cases**:
1. Password reset token expiration (e.g., 1 hour)
2. Single-use reset tokens
3. No user enumeration (same response for valid/invalid email)
4. Reset link sent to registered email only
5. Old password no longer works after reset

**Test Script**:
```python
def test_password_reset_token_expiration():
    # Request password reset
    response = requests.post("/api/password/reset-request", json={
        "email": "user@example.com"
    })
    assert response.status_code == 200

    # Get reset token from email
    reset_token = get_reset_token_from_email()

    # Wait for token to expire (e.g., 61 minutes)
    time.sleep(61 * 60)

    # Try to use expired token
    response = requests.post("/api/password/reset", json={
        "token": reset_token,
        "new_password": "NewPassword123!"
    })
    assert response.status_code == 400
    assert "expired" in response.json()["error"].lower()

def test_no_user_enumeration():
    # Reset request for existing user
    response1 = requests.post("/api/password/reset-request", json={
        "email": "existing@example.com"
    })

    # Reset request for non-existing user
    response2 = requests.post("/api/password/reset-request", json={
        "email": "nonexisting@example.com"
    })

    # Responses should be identical
    assert response1.status_code == response2.status_code == 200
    assert response1.json() == response2.json()
```

## OAuth/SSO Testing

### OAuth 2.0 Security

**Test Cases**:
1. Verify state parameter prevents CSRF
2. Test redirect URI validation (exact match)
3. Verify authorization code is single-use
4. Test PKCE implementation (public clients)
5. Verify token expiration and refresh

**Test Script**:
```python
def test_oauth_state_parameter():
    # Initiate OAuth flow
    state = "random_state_value_12345"
    auth_url = f"/oauth/authorize?state={state}&redirect_uri=..."
    response = requests.get(auth_url)

    # Verify state is included in callback
    callback_url = response.url
    assert f"state={state}" in callback_url

def test_redirect_uri_validation():
    # Legitimate redirect URI
    response = requests.get("/oauth/authorize", params={
        "redirect_uri": "https://myapp.com/callback",
        "state": "xyz"
    })
    assert response.status_code == 200

    # Malicious redirect URI (different domain)
    response = requests.get("/oauth/authorize", params={
        "redirect_uri": "https://attacker.com/steal",
        "state": "xyz"
    })
    assert response.status_code == 400
```

## Best Practices Summary

1. **Password Security**
   - Minimum 12 characters
   - Use bcrypt or Argon2
   - Enforce password history
   - Never log passwords

2. **Brute Force Protection**
   - Account lockout after 5-10 attempts
   - Rate limiting per IP and per user
   - CAPTCHA after multiple failures
   - Time-based lockout release

3. **Session Management**
   - Regenerate session ID on login
   - Use HttpOnly, Secure, SameSite flags
   - Implement idle and absolute timeouts
   - Invalidate sessions on logout

4. **MFA**
   - Enforce MFA for sensitive accounts
   - Time-limited codes (30 seconds typical)
   - Prevent code reuse
   - Secure backup codes

5. **Password Reset**
   - Single-use, time-limited tokens
   - Prevent user enumeration
   - Send to registered email only
   - Invalidate old sessions after reset
