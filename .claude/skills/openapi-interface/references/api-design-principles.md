# REST API Design Principles

## Core Principles

### 1. Resource-Oriented URLs
Use nouns, not verbs (e.g., `/users`, not `/getUsers`)

### 2. HTTP Methods
Use standard verbs appropriately:
- **GET** - Retrieve resource(s), idempotent, safe, cacheable
- **POST** - Create resource, non-idempotent, unsafe
- **PUT** - Replace resource, idempotent, unsafe
- **PATCH** - Update resource partially, non-idempotent, unsafe
- **DELETE** - Remove resource, idempotent, unsafe

### 3. Stateless Operations
Each request contains all necessary information

### 4. Consistent Naming
Use lowercase, hyphen-separated paths

### 5. Versioning
Include version in URL (e.g., `/v1/users`) or headers

### 6. HATEOAS
Include links to related resources when appropriate

---

## OpenAPI Best Practices

### 1. Contract-First Design
Start with the API contract before implementation

### 2. Component Reusability
Use `components` section to define schemas, parameters, responses once and reference with `$ref`

### 3. Comprehensive Descriptions
Document every operation, parameter, and response with clear descriptions

### 4. Examples for Clarity
Provide request/response examples for all endpoints

### 5. Security Schemes
Define authentication/authorization clearly and consistently

### 6. Validation Rules
Use JSON Schema constraints (min, max, pattern, enum, format)

### 7. Error Responses
Document all possible error codes and standardized error formats

---

## URL Structure Design

Design URL paths following REST conventions:

```
GET    /v1/users              # List all users
POST   /v1/users              # Create new user
GET    /v1/users/{id}         # Get specific user
PUT    /v1/users/{id}         # Replace user (full update)
PATCH  /v1/users/{id}         # Update user (partial update)
DELETE /v1/users/{id}         # Delete user

GET    /v1/users/{id}/posts   # List user's posts (nested resource)
POST   /v1/users/{id}/posts   # Create post for user
```

### URL Design Rules:
- Use plural nouns for collections
- Path parameters for IDs
- Query parameters for filtering/sorting
- Limit nesting to 2 levels
- Avoid verbs in URLs

---

## Schema Design Guidelines

Define data models using JSON Schema:

### Use `$ref` for Reusability
Define schemas in `components/schemas` and reference them

### Specify Data Types
- string, number, integer, boolean, array, object

### Add Validation
- required fields
- patterns (regex)
- min/max values
- enums

### Include Descriptions
Document every field

### Provide Examples
Show sample data structures

---

## Response Design

Define consistent response structures:

### Success Responses
- **200 OK** - Successful GET, PUT, PATCH
- **201 Created** - Successful POST
- **204 No Content** - Successful DELETE

### Client Errors
- **400 Bad Request** - Invalid request data
- **401 Unauthorized** - Authentication required
- **403 Forbidden** - Insufficient permissions
- **404 Not Found** - Resource doesn't exist
- **409 Conflict** - Resource conflict
- **422 Unprocessable Entity** - Validation error

### Server Errors
- **500 Internal Server Error** - Server error
- **502 Bad Gateway** - Invalid upstream response
- **503 Service Unavailable** - Service temporarily unavailable

### Error Format
Standardize error response schema across all endpoints

### Pagination
Define pagination metadata (limit, offset, total, next/prev links)

---

## Security Definition

Specify authentication and authorization:

### API Keys
Header, query parameter, or cookie-based

### OAuth 2.0
Authorization code, client credentials, implicit flows

### OpenID Connect
Identity layer on OAuth 2.0

### HTTP Authentication
Basic or Bearer token

### Security Requirements
Apply globally in root `security` field or per-operation
