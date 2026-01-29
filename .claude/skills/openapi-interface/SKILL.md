---
name: openapi-interface
description: Designs and documents REST APIs using the OpenAPI Specification (OAS). Use when defining API contracts, designing RESTful endpoints, documenting API operations, specifying request/response schemas, defining authentication requirements, creating API-first designs, or establishing API versioning strategies. Covers CRUD operations, pagination, filtering, error responses, and security schemes for web services and microservices.
allowed-tools: Read Write Bash Edit Glob Grep WebFetch
---

# OpenAPI Interface Design and Documentation Skill

## Purpose

This skill provides comprehensive guidance for designing REST APIs and generating OpenAPI Specification (OAS) compliant YAML documentation. It enables API-first development, contract-driven design, and standardized API documentation following industry best practices.

## When to Use This Skill

Use this skill when you need to:
- **Design new REST APIs** from requirements or specifications
- **Document existing APIs** using OpenAPI Specification (OAS 3.1 or 3.0)
- **Create API contracts** for microservices communication
- **Generate OpenAPI YAML specifications** with complete schemas and examples
- **Design RESTful endpoints** following REST principles and best practices
- **Specify request/response schemas** with JSON Schema validation
- **Document authentication and security** requirements (OAuth 2.0, JWT, API keys)
- **Define API versioning strategies** for evolving APIs
- **Create API mock servers** and contract testing specifications
- **Design API gateways** or GraphQL to REST mappings

## OpenAPI Specification Reference

**Official Specification**: https://spec.openapis.org/oas/latest.html

**Always generate specifications in YAML format** for readability and maintainability.

## Core Principles

### REST API Design
1. **Resource-Oriented URLs** - Use nouns, not verbs (e.g., `/users`, not `/getUsers`)
2. **HTTP Methods** - Use standard verbs appropriately (GET, POST, PUT, PATCH, DELETE)
3. **Stateless Operations** - Each request contains all necessary information
4. **Consistent Naming** - Use lowercase, hyphen-separated paths
5. **Versioning** - Include version in URL (e.g., `/v1/users`) or headers
6. **HATEOAS** - Include links to related resources when appropriate

For detailed REST principles and guidelines, see [references/api-design-principles.md](references/api-design-principles.md).

### OpenAPI Best Practices
1. **Contract-First Design** - Design API before implementation
2. **Component Reusability** - Use `$ref` for schemas, parameters, responses
3. **Comprehensive Descriptions** - Document every operation, parameter, and response
4. **Examples for Clarity** - Provide request/response examples throughout
5. **Security Schemes** - Define authentication/authorization clearly
6. **Validation Rules** - Use JSON Schema constraints (min, max, pattern, enum)
7. **Error Responses** - Document all possible error codes and formats

## API Design Workflow

### 1. Requirements Analysis
Before creating the OpenAPI specification:
- **Identify resources** - What entities does the API expose?
- **Define operations** - What actions can be performed on each resource?
- **Determine relationships** - How do resources relate to each other?
- **Specify authentication** - What security mechanisms are required?
- **Plan versioning** - How will the API evolve over time?

### 2. URL Structure Design
Design URL paths following REST conventions. Use:
- Plural nouns for collections (e.g., `/users`, `/orders`)
- Path parameters for identifiers (e.g., `/users/{userId}`)
- Query parameters for filtering and sorting (e.g., `?status=active&sort=name`)
- Limit nesting to 2 levels maximum (e.g., `/users/{userId}/orders`)

For complete URL design rules and examples, see [references/api-design-principles.md](references/api-design-principles.md).

### 3. Schema Design
Define data models using JSON Schema with:
- `$ref` for reusability (define in `components/schemas`)
- Appropriate data types (string, number, integer, boolean, array, object)
- Validation constraints (required, minLength, maxLength, pattern, enum)
- Field descriptions and examples

### 4. Response Design
Define consistent response structures:
- **Success responses**: 200 OK, 201 Created, 204 No Content
- **Client errors**: 400 Bad Request, 401 Unauthorized, 403 Forbidden, 404 Not Found, 422 Unprocessable Entity
- **Server errors**: 500 Internal Server Error, 503 Service Unavailable
- **Standardized error format** across all endpoints
- **Pagination metadata** for list operations (limit, offset, total, next/prev links)

### 5. Security Definition
Specify authentication and authorization:
- **API Keys** - Header, query parameter, or cookie-based
- **OAuth 2.0** - Authorization code, client credentials, implicit flows
- **OpenID Connect** - Identity layer on OAuth 2.0
- **HTTP Authentication** - Basic or Bearer token (JWT)
- **Security requirements** - Apply globally or per-operation

## OpenAPI Specification Structure

A complete OpenAPI specification includes:
- **info** - API metadata (title, version, description, contact, license)
- **servers** - API server URLs (production, staging, development)
- **paths** - Endpoint definitions with operations (GET, POST, PUT, PATCH, DELETE)
- **components** - Reusable schemas, parameters, responses, security schemes
- **security** - Global security requirements

For a complete, ready-to-use OpenAPI 3.1 template with all sections, see [references/openapi-specification-template.md](references/openapi-specification-template.md).

## Common API Patterns

When designing APIs, use established patterns for consistency and best practices:

1. **CRUD Operations** - Standard create, read, update, delete endpoints
2. **Nested Resources** - Parent-child resource relationships (e.g., `/users/{userId}/orders`)
3. **Search and Filter** - Complex filtering, sorting, and pagination
4. **Batch Operations** - Multiple operations in a single request (207 Multi-Status)
5. **Async Operations** - Long-running operations with status checking (202 Accepted)

For complete code examples of each pattern with full YAML specifications, see [references/common-patterns.md](references/common-patterns.md).

## Validation and Testing

After creating your OpenAPI specification:

### Validate Specifications
```bash
# Using Spectral (recommended)
spectral lint openapi.yaml

# Using Swagger CLI
swagger-cli validate openapi.yaml
```

### Generate Mock Servers
```bash
# Using Prism
prism mock openapi.yaml
```

### Generate Client SDKs and Server Stubs
```bash
# Generate TypeScript client
docker run --rm -v "${PWD}:/local" openapitools/openapi-generator-cli generate \
  -i /local/openapi.yaml -g typescript-fetch -o /local/client
```

For complete validation, testing, and code generation commands, see [references/common-patterns.md](references/common-patterns.md#validation-and-testing).

## Tools and Ecosystem

**Specification Tools**: Swagger Editor, Stoplight Studio, Spectral, OpenAPI Generator
**Documentation Tools**: Swagger UI, ReDoc, RapiDoc, Stoplight Elements
**Testing Tools**: Prism, Dredd, Schemathesis, Postman
**Validation Tools**: Swagger CLI, Spectral

For detailed tool descriptions, installation instructions, and file organization best practices, see [references/tools-and-ecosystem.md](references/tools-and-ecosystem.md).

## Integration with Agents

This skill can be used by any agent for API-related tasks:

### By Software Architects
- Designing system APIs and microservice contracts
- Planning API versioning and evolution strategies
- Defining API gateways and aggregation patterns
- Creating API standards and organizational conventions
- Reviewing API designs for REST compliance

### By Application Engineers
- Implementing REST endpoints following OpenAPI contracts
- Generating API stubs from specifications
- Validating implementations against specifications

### By Test Engineers
- Writing API contract tests from specifications
- Generating test cases from OpenAPI schemas
- Setting up mock servers for testing

### By Technical Writers
- Documenting existing APIs with OpenAPI
- Generating interactive API documentation
- Creating API usage guides and examples

### By Product Managers
- Reviewing API designs for feature completeness
- Defining API requirements and capabilities
- Planning API product roadmaps

## Typical Workflow

### Design-First Approach (Recommended)

1. **Create OpenAPI specification** based on requirements
2. **Validate specification** using Spectral or Swagger CLI
3. **Generate mock server** for early testing (Prism)
4. **Generate server stubs** for implementation scaffolding
5. **Implement API** following the contract
6. **Run contract tests** to validate implementation (Dredd, Schemathesis)
7. **Generate client SDKs** for consumers
8. **Generate documentation** for API consumers (Swagger UI, ReDoc)

### Code-First Approach (Existing APIs)

1. **Generate OpenAPI from code** using framework tools (FastAPI, SpringDoc, NestJS)
2. **Review and enhance** generated specification (add descriptions, examples, validation)
3. **Validate specification** for completeness and correctness
4. **Generate documentation and clients** for distribution

For detailed workflow commands and framework-specific code generation, see [references/tools-and-ecosystem.md](references/tools-and-ecosystem.md#workflow-tools).

## Best Practices Summary

### Design Principles
1. **API-first thinking** - Design contract before implementation
2. **Resource-oriented** - URLs represent resources, not actions
3. **Consistent conventions** - Follow established patterns throughout
4. **Self-documenting** - Clear naming and comprehensive descriptions
5. **Version explicitly** - Plan for API evolution from day one
6. **Secure by default** - Define security requirements clearly

### OpenAPI Specification
1. **Use YAML format** - More readable than JSON
2. **Leverage `$ref`** - Reuse components, avoid duplication
3. **Include examples** - Every schema and response should have examples
4. **Comprehensive descriptions** - Document intent, not just structure
5. **Validation constraints** - Use JSON Schema fully (min, max, pattern, enum)
6. **Error responses** - Document all possible error scenarios
7. **Split large specs** - Use `$ref` to external files for maintainability

### Common Mistakes to Avoid
1. Don't use verbs in URLs - Use HTTP methods instead
2. Don't nest resources too deeply - Limit to 2 levels maximum
3. Don't ignore status codes - Use appropriate codes for each scenario
4. Don't skip examples - Always include request/response examples
5. Don't forget security - Define and apply security schemes consistently
6. Don't duplicate schemas - Use `$ref` for reusability

## Resources

### Official Documentation
- **OpenAPI Specification** - https://spec.openapis.org/oas/latest.html
- **OpenAPI Initiative** - https://www.openapis.org/
- **JSON Schema** - https://json-schema.org/

### Tools
- **Swagger Editor** - https://editor.swagger.io/
- **OpenAPI Generator** - https://openapi-generator.tech/
- **Spectral** - https://stoplight.io/open-source/spectral
- **Prism** - https://stoplight.io/open-source/prism

### Learning Resources
- **OpenAPI Best Practices** - https://oai.github.io/Documentation/best-practices.html
- **REST API Design Rulebook** - https://www.oreilly.com/library/view/rest-api-design/9781449317904/
- **API Design Patterns** - https://microservice-api-patterns.org/

For additional tools, ecosystem information, and quick reference guides, see [references/tools-and-ecosystem.md](references/tools-and-ecosystem.md).

## Support

For questions about OpenAPI Specification:
- Official documentation: https://spec.openapis.org/oas/latest.html
- Stack Overflow: Tag `openapi` or `swagger`
- OpenAPI Community Slack: https://open-api.slack.com/
