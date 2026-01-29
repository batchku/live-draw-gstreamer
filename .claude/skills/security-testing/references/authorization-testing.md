# Authorization Testing Guide

Comprehensive test cases for authorization and access control security.

## Authorization Test Cases Overview

```
┌─────────────────────────────────────────────────────────────────────┐
│                    AUTHORIZATION TEST CASES                          │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  ACCESS CONTROL                                                     │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │ □ Test access to each endpoint without authentication         │  │
│  │ □ Verify each role can only access authorized resources       │  │
│  │ □ Test horizontal privilege escalation (user A → user B data) │  │
│  │ □ Test vertical privilege escalation (user → admin)           │  │
│  │ □ Verify RBAC/ABAC policies enforced consistently             │  │
│  │ □ Test access after permission revocation                     │  │
│  └───────────────────────────────────────────────────────────────┘  │
│                                                                     │
│  INSECURE DIRECT OBJECT REFERENCES (IDOR)                           │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │ Test Vectors:                                                 │  │
│  │                                                               │  │
│  │ Original:  GET /api/users/123/profile                         │  │
│  │ Attack:    GET /api/users/456/profile  (different user)       │  │
│  │                                                               │  │
│  │ Original:  GET /api/orders?user_id=123                        │  │
│  │ Attack:    GET /api/orders?user_id=456                        │  │
│  │                                                               │  │
│  │ Original:  POST /api/files/download {"id": "abc123"}          │  │
│  │ Attack:    POST /api/files/download {"id": "xyz789"}          │  │
│  │                                                               │  │
│  │ □ Test all resource IDs in URLs and request bodies            │  │
│  │ □ Try sequential IDs, UUIDs, predictable patterns             │  │
│  │ □ Test with authenticated but unauthorized users              │  │
│  └───────────────────────────────────────────────────────────────┘  │
│                                                                     │
│  API AUTHORIZATION                                                  │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │ □ Test each API endpoint with different roles                 │  │
│  │ □ Verify JWT claims are validated server-side                 │  │
│  │ □ Test API key scope restrictions                             │  │
│  │ □ Verify GraphQL query authorization                          │  │
│  │ □ Test batch operations for authorization bypass              │  │
│  └───────────────────────────────────────────────────────────────┘  │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

## Access Control Models

### Role-Based Access Control (RBAC)

**Model Overview**:
- Users are assigned roles
- Roles have permissions
- Permissions grant access to resources

**Test Cases**:
```python
def test_rbac_user_role():
    """Test that regular user can only access user endpoints"""
    user_session = login_as_role("user")

    # User can access their own profile
    response = user_session.get("/api/profile")
    assert response.status_code == 200

    # User cannot access admin endpoints
    response = user_session.get("/api/admin/users")
    assert response.status_code == 403

    # User cannot access other users' data
    response = user_session.get("/api/users/999/profile")
    assert response.status_code == 403

def test_rbac_admin_role():
    """Test that admin can access admin endpoints"""
    admin_session = login_as_role("admin")

    # Admin can access admin endpoints
    response = admin_session.get("/api/admin/users")
    assert response.status_code == 200

    # Admin can access user data
    response = admin_session.get("/api/users/123/profile")
    assert response.status_code == 200

def test_rbac_role_hierarchy():
    """Test that role hierarchy is enforced"""
    # Roles: user < moderator < admin

    moderator_session = login_as_role("moderator")

    # Moderator can perform user actions
    response = moderator_session.get("/api/profile")
    assert response.status_code == 200

    # Moderator can moderate content
    response = moderator_session.delete("/api/posts/123")
    assert response.status_code == 200

    # Moderator cannot access admin-only functions
    response = moderator_session.get("/api/admin/settings")
    assert response.status_code == 403
```

### Attribute-Based Access Control (ABAC)

**Model Overview**:
- Access decisions based on attributes (user, resource, environment)
- More fine-grained than RBAC
- Policies evaluate attributes dynamically

**Test Cases**:
```python
def test_abac_department_access():
    """Test that users can only access resources from their department"""
    hr_user = login_as_user(department="HR")

    # HR user can access HR documents
    response = hr_user.get("/api/documents/hr-policy-2024")
    assert response.status_code == 200

    # HR user cannot access Engineering documents
    response = hr_user.get("/api/documents/eng-architecture-2024")
    assert response.status_code == 403

def test_abac_time_based_access():
    """Test that access is restricted based on time"""
    # Regular user with time-based restriction (9am-5pm)
    session = login_as_user(username="timeuser")

    # Access during allowed hours
    with mock_time("2024-01-15 10:00:00"):
        response = session.get("/api/sensitive-data")
        assert response.status_code == 200

    # Access outside allowed hours
    with mock_time("2024-01-15 20:00:00"):
        response = session.get("/api/sensitive-data")
        assert response.status_code == 403

def test_abac_resource_ownership():
    """Test that users can only modify their own resources"""
    user1 = login_as_user(user_id=1)
    user2 = login_as_user(user_id=2)

    # User 1 creates a document
    response = user1.post("/api/documents", json={"title": "My Doc"})
    doc_id = response.json()["id"]

    # User 1 can modify their document
    response = user1.put(f"/api/documents/{doc_id}", json={"title": "Updated"})
    assert response.status_code == 200

    # User 2 cannot modify User 1's document
    response = user2.put(f"/api/documents/{doc_id}", json={"title": "Hacked"})
    assert response.status_code == 403
```

## Privilege Escalation Testing

### Horizontal Privilege Escalation

**Definition**: Accessing resources belonging to another user at the same privilege level.

**Test Vectors**:
```python
def test_horizontal_privilege_escalation_url():
    """Test IDOR in URL parameters"""
    user1 = login_as_user(user_id=123)
    user2 = login_as_user(user_id=456)

    # User 1 accesses their own orders
    response = user1.get("/api/users/123/orders")
    assert response.status_code == 200
    user1_orders = response.json()

    # User 1 tries to access User 2's orders
    response = user1.get("/api/users/456/orders")
    assert response.status_code == 403

    # User 2 accesses their own orders (should be different)
    response = user2.get("/api/users/456/orders")
    assert response.status_code == 200
    user2_orders = response.json()

    assert user1_orders != user2_orders

def test_horizontal_privilege_escalation_query():
    """Test IDOR in query parameters"""
    user1 = login_as_user(user_id=123)

    # User tries to access another user's data via query param
    response = user1.get("/api/profile?user_id=456")
    assert response.status_code == 403

def test_horizontal_privilege_escalation_body():
    """Test IDOR in request body"""
    user1 = login_as_user(user_id=123)

    # User tries to modify another user's profile
    response = user1.put("/api/profile", json={
        "user_id": 456,  # Different user
        "email": "hacked@example.com"
    })
    assert response.status_code == 403

    # Verify original user is unchanged
    user2 = login_as_user(user_id=456)
    response = user2.get("/api/profile")
    assert response.json()["email"] != "hacked@example.com"
```

### Vertical Privilege Escalation

**Definition**: Gaining access to higher privilege functions (e.g., user to admin).

**Test Vectors**:
```python
def test_vertical_privilege_escalation_direct():
    """Test direct access to admin endpoints"""
    user_session = login_as_role("user")

    admin_endpoints = [
        "/api/admin/users",
        "/api/admin/settings",
        "/api/admin/logs",
        "/api/admin/reports",
    ]

    for endpoint in admin_endpoints:
        response = user_session.get(endpoint)
        assert response.status_code == 403, f"User accessed admin endpoint: {endpoint}"

def test_vertical_privilege_escalation_role_manipulation():
    """Test role manipulation attempts"""
    user_session = login_as_role("user")

    # Try to set role to admin via profile update
    response = user_session.put("/api/profile", json={
        "role": "admin"
    })
    assert response.status_code in [400, 403]

    # Verify role is unchanged
    response = user_session.get("/api/profile")
    assert response.json()["role"] != "admin"

def test_vertical_privilege_escalation_jwt():
    """Test JWT manipulation attempts"""
    user_session = login_as_role("user")
    user_token = user_session.cookies.get("jwt")

    # Decode JWT and modify role claim
    decoded = jwt.decode(user_token, options={"verify_signature": False})
    decoded["role"] = "admin"

    # Try to use modified JWT (should fail due to signature)
    modified_token = jwt.encode(decoded, "wrong-secret")
    malicious_session = requests.Session()
    malicious_session.cookies.set("jwt", modified_token)

    response = malicious_session.get("/api/admin/users")
    assert response.status_code == 401  # Invalid signature
```

## Insecure Direct Object References (IDOR)

### Sequential ID Testing

**Attack Scenario**: Predictable resource IDs allow enumeration and unauthorized access.

**Test Cases**:
```python
def test_idor_sequential_enumeration():
    """Test that sequential IDs can't be enumerated"""
    user_session = login_as_user(user_id=100)

    # User accesses their own resource
    response = user_session.get("/api/documents/1")
    if response.status_code == 200:
        # Resource 1 belongs to this user
        accessible_ids = [1]
    else:
        accessible_ids = []

    # Try to enumerate other IDs
    unauthorized_access = []
    for doc_id in range(2, 20):
        response = user_session.get(f"/api/documents/{doc_id}")
        if response.status_code == 200:
            unauthorized_access.append(doc_id)

    # User should not have accessed resources not owned by them
    assert len(unauthorized_access) == 0, \
        f"IDOR vulnerability: accessed IDs {unauthorized_access}"

def test_idor_guid_protection():
    """Test that GUIDs/UUIDs don't prevent IDOR if not checked"""
    user1 = login_as_user(user_id=1)
    user2 = login_as_user(user_id=2)

    # User 2 creates a resource (gets GUID)
    response = user2.post("/api/files", json={"name": "secret.txt"})
    file_guid = response.json()["id"]  # e.g., "550e8400-e29b-41d4-a716-446655440000"

    # User 1 tries to access User 2's file using GUID
    response = user1.get(f"/api/files/{file_guid}")
    assert response.status_code == 403, "IDOR via GUID bypass"
```

### Hidden Parameter IDOR

**Attack Scenario**: Resource owner specified in hidden or unexpected parameters.

**Test Cases**:
```python
def test_idor_hidden_user_id():
    """Test IDOR via hidden user_id parameter"""
    user1 = login_as_user(user_id=123)

    # API might use user_id from token, but also accept it as param
    # User tries to override with another user's ID
    response = user1.get("/api/orders", params={"user_id": 456})
    assert response.status_code == 403

    # Try in request body
    response = user1.post("/api/orders/search", json={
        "user_id": 456,
        "status": "completed"
    })
    assert response.status_code == 403

def test_idor_via_header():
    """Test IDOR via custom headers"""
    user1 = login_as_user(user_id=123)

    # Some APIs use custom headers for user context
    response = user1.get("/api/profile", headers={
        "X-User-ID": "456"
    })
    # Should use authenticated user's ID, not header
    assert response.json()["user_id"] == 123
```

### Batch Operation IDOR

**Attack Scenario**: Batch operations may bypass authorization checks.

**Test Cases**:
```python
def test_idor_batch_delete():
    """Test that batch operations check authorization"""
    user1 = login_as_user(user_id=123)
    user2 = login_as_user(user_id=456)

    # User 2 creates some documents
    doc_ids = []
    for i in range(3):
        response = user2.post("/api/documents", json={"title": f"Doc {i}"})
        doc_ids.append(response.json()["id"])

    # User 1 tries to batch delete User 2's documents
    response = user1.post("/api/documents/batch-delete", json={
        "ids": doc_ids
    })
    assert response.status_code == 403

    # Verify documents still exist
    for doc_id in doc_ids:
        response = user2.get(f"/api/documents/{doc_id}")
        assert response.status_code == 200
```

## API Authorization Testing

### JWT Security

**Test Cases**:
```python
def test_jwt_signature_verification():
    """Test that JWT signature is verified"""
    user_session = login_as_user(user_id=123)
    valid_jwt = user_session.cookies.get("jwt")

    # Decode JWT
    header, payload, signature = valid_jwt.split('.')

    # Modify payload (change user_id)
    import base64
    import json
    decoded_payload = json.loads(base64.urlsafe_b64decode(payload + '=='))
    decoded_payload["user_id"] = 999
    modified_payload = base64.urlsafe_b64encode(
        json.dumps(decoded_payload).encode()
    ).decode().rstrip('=')

    # Create tampered JWT with original signature
    tampered_jwt = f"{header}.{modified_payload}.{signature}"

    # Try to use tampered JWT
    malicious_session = requests.Session()
    malicious_session.cookies.set("jwt", tampered_jwt)
    response = malicious_session.get("/api/profile")
    assert response.status_code == 401  # Signature verification failed

def test_jwt_expiration():
    """Test that expired JWT is rejected"""
    # Create JWT with short expiration
    expired_jwt = create_jwt(user_id=123, expires_in_seconds=1)

    # Wait for expiration
    time.sleep(2)

    # Try to use expired JWT
    session = requests.Session()
    session.cookies.set("jwt", expired_jwt)
    response = session.get("/api/profile")
    assert response.status_code == 401

def test_jwt_algorithm_confusion():
    """Test that 'none' algorithm is rejected"""
    user_session = login_as_user(user_id=123)
    valid_jwt = user_session.cookies.get("jwt")

    # Create JWT with 'none' algorithm
    decoded = jwt.decode(valid_jwt, options={"verify_signature": False})
    malicious_jwt = jwt.encode(decoded, "", algorithm="none")

    # Try to use malicious JWT
    session = requests.Session()
    session.cookies.set("jwt", malicious_jwt)
    response = session.get("/api/profile")
    assert response.status_code == 401
```

### API Key Authorization

**Test Cases**:
```python
def test_api_key_scope():
    """Test that API key scopes are enforced"""
    # Create API key with read-only scope
    readonly_key = create_api_key(scopes=["read"])

    # Read operations should work
    response = requests.get("/api/data", headers={
        "X-API-Key": readonly_key
    })
    assert response.status_code == 200

    # Write operations should be forbidden
    response = requests.post("/api/data", headers={
        "X-API-Key": readonly_key
    }, json={"value": "test"})
    assert response.status_code == 403

def test_api_key_revocation():
    """Test that revoked API keys are rejected"""
    api_key = create_api_key()

    # Key works initially
    response = requests.get("/api/data", headers={
        "X-API-Key": api_key
    })
    assert response.status_code == 200

    # Revoke key
    revoke_api_key(api_key)

    # Key should no longer work
    response = requests.get("/api/data", headers={
        "X-API-Key": api_key
    })
    assert response.status_code == 401
```

### GraphQL Authorization

**Test Cases**:
```python
def test_graphql_field_authorization():
    """Test that GraphQL field-level authorization is enforced"""
    user_session = login_as_role("user")

    # Query that includes admin-only field
    query = """
    query {
        user(id: 123) {
            name
            email
            ssn  # Admin-only field
        }
    }
    """

    response = user_session.post("/graphql", json={"query": query})

    # Should return data but null for unauthorized field
    data = response.json()["data"]
    assert data["user"]["name"] is not None
    assert data["user"]["email"] is not None
    assert data["user"]["ssn"] is None  # Not authorized

def test_graphql_query_depth():
    """Test that query depth limits prevent DoS"""
    deeply_nested_query = """
    query {
        user { friends { friends { friends { friends { friends {
            name
        }}}}}}
    }
    """

    response = requests.post("/graphql", json={"query": deeply_nested_query})
    assert response.status_code == 400
    assert "depth" in response.json()["errors"][0]["message"].lower()
```

## Permission Revocation Testing

**Test Cases**:
```python
def test_permission_revocation_immediate():
    """Test that revoked permissions take effect immediately"""
    user_session = login_as_user(user_id=123)

    # User can access resource initially
    response = user_session.get("/api/sensitive-data")
    assert response.status_code == 200

    # Revoke permission
    admin_revoke_permission(user_id=123, permission="read:sensitive-data")

    # User should no longer have access (immediate effect)
    response = user_session.get("/api/sensitive-data")
    assert response.status_code == 403

def test_role_change_immediate():
    """Test that role changes take effect immediately"""
    user_session = login_as_role("admin")

    # Admin can access admin endpoint
    response = user_session.get("/api/admin/settings")
    assert response.status_code == 200

    # Change role to user
    change_user_role(user_session.user_id, "user")

    # Should no longer have admin access
    response = user_session.get("/api/admin/settings")
    assert response.status_code == 403
```

## Best Practices Summary

1. **Always Validate Authorization Server-Side**
   - Never trust client-side checks
   - Validate on every request
   - Check both authentication and authorization

2. **Use Secure Resource Identifiers**
   - Use UUIDs instead of sequential IDs
   - Still enforce authorization even with UUIDs
   - Validate ownership on every access

3. **Implement Defense in Depth**
   - Multiple layers of authorization checks
   - Validate at API gateway and service level
   - Log all authorization failures

4. **Principle of Least Privilege**
   - Grant minimum necessary permissions
   - Default to deny
   - Require explicit grants

5. **Test All Access Paths**
   - Test URL parameters, query params, body, headers
   - Test GET, POST, PUT, DELETE, PATCH
   - Test batch operations separately
   - Test with different roles and users

6. **Monitor and Alert**
   - Log all authorization failures
   - Alert on suspicious patterns
   - Track privilege escalation attempts
