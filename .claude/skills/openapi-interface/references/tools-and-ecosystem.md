# OpenAPI Tools and Ecosystem

## Specification Tools

### Swagger Editor
- **URL**: https://editor.swagger.io/
- **Purpose**: Online YAML/JSON editor with real-time validation
- **Features**: Syntax highlighting, live preview, error detection
- **Use when**: Quick editing and validation in browser

### Stoplight Studio
- **URL**: https://stoplight.io/studio
- **Purpose**: Visual OpenAPI designer with form-based and code views
- **Features**: Visual design, mock servers, Git integration
- **Use when**: Collaborative API design with non-technical stakeholders

### Spectral
- **URL**: https://stoplight.io/open-source/spectral
- **Purpose**: OpenAPI linter with customizable rules
- **Features**: Style guide enforcement, custom rulesets, CI/CD integration
- **Use when**: Enforcing organizational API standards

```bash
# Install Spectral
npm install -g @stoplight/spectral-cli

# Lint with default rules
spectral lint openapi.yaml

# Use custom ruleset
spectral lint openapi.yaml --ruleset .spectral.yaml
```

### OpenAPI Generator
- **URL**: https://openapi-generator.tech/
- **Purpose**: Multi-language code generation from OpenAPI specs
- **Features**: 50+ language generators, templates, customization
- **Use when**: Generating client SDKs or server stubs

```bash
# List available generators
docker run --rm openapitools/openapi-generator-cli list

# Generate client
docker run --rm -v "${PWD}:/local" openapitools/openapi-generator-cli generate \
  -i /local/openapi.yaml \
  -g typescript-fetch \
  -o /local/client
```

---

## Documentation Tools

### Swagger UI
- **URL**: https://swagger.io/tools/swagger-ui/
- **Purpose**: Interactive API documentation
- **Features**: Try-it-out functionality, authentication, customizable themes
- **Use when**: Developer-facing interactive documentation

```bash
# Run with Docker
docker run -p 8080:8080 \
  -e SWAGGER_JSON=/openapi.yaml \
  -v ${PWD}:/openapi \
  swaggerapi/swagger-ui
```

### ReDoc
- **URL**: https://redocly.com/redoc
- **Purpose**: Clean, responsive API documentation
- **Features**: Three-panel layout, search, code samples
- **Use when**: Public-facing documentation with emphasis on aesthetics

```bash
# Generate standalone HTML
npx redoc-cli bundle openapi.yaml -o docs/api.html

# Serve with hot reload
npx redoc-cli serve openapi.yaml --watch
```

### RapiDoc
- **URL**: https://rapidocweb.com/
- **Purpose**: Modern, lightweight API documentation web component
- **Features**: Customizable themes, schema explorer, markdown support
- **Use when**: Embedding documentation in existing web applications

```html
<!DOCTYPE html>
<html>
<head>
  <script type="module" src="https://unpkg.com/rapidoc/dist/rapidoc-min.js"></script>
</head>
<body>
  <rapi-doc spec-url="openapi.yaml"></rapi-doc>
</body>
</html>
```

### Stoplight Elements
- **URL**: https://stoplight.io/open-source/elements
- **Purpose**: Embeddable API docs with mock server
- **Features**: React components, try-it functionality, responsive
- **Use when**: Building custom documentation portals

---

## Testing Tools

### Prism
- **URL**: https://stoplight.io/open-source/prism
- **Purpose**: OpenAPI mock server and validation proxy
- **Features**: Dynamic response generation, validation, contract testing
- **Use when**: Testing before implementation or validating API contracts

```bash
# Install
npm install -g @stoplight/prism-cli

# Mock server
prism mock openapi.yaml

# Validation proxy
prism proxy openapi.yaml http://localhost:8080
```

### Dredd
- **URL**: https://dredd.org/
- **Purpose**: HTTP API testing framework for OpenAPI
- **Features**: Contract testing, hooks for custom logic, CI/CD integration
- **Use when**: Automated API contract testing in CI pipeline

```bash
# Install
npm install -g dredd

# Run tests
dredd openapi.yaml http://localhost:8080
```

### Schemathesis
- **URL**: https://schemathesis.readthedocs.io/
- **Purpose**: Property-based testing for APIs
- **Features**: Automatic test generation, fuzzing, hypothesis testing
- **Use when**: Discovering edge cases and validating API robustness

```bash
# Install
pip install schemathesis

# Run tests
schemathesis run openapi.yaml --base-url http://localhost:8080
```

### Postman
- **URL**: https://www.postman.com/
- **Purpose**: API development and testing platform
- **Features**: Import OpenAPI, collection tests, automation
- **Use when**: Manual testing and exploratory API testing

**Import OpenAPI**: File > Import > Select openapi.yaml

---

## Validation Tools

### Swagger CLI
- **URL**: https://www.npmjs.com/package/@apidevtools/swagger-cli
- **Purpose**: Command-line validation and bundling
- **Features**: Validate, bundle multi-file specs, convert formats
- **Use when**: CI/CD validation and bundling split specs

```bash
# Install
npm install -g @apidevtools/swagger-cli

# Validate
swagger-cli validate openapi.yaml

# Bundle multi-file spec
swagger-cli bundle -o bundled.yaml -t yaml index.yaml
```

### openapi-generator-cli validate
- **Purpose**: Validate OpenAPI specs before generation
- **Use when**: Ensuring spec is valid before code generation

```bash
docker run --rm -v "${PWD}:/local" \
  openapitools/openapi-generator-cli validate \
  -i /local/openapi.yaml
```

---

## File Organization

Store OpenAPI specifications in your project using one of these patterns:

### Single File (Small APIs)
```
project/
├── api/
│   └── openapi.yaml              # Complete specification
├── docs/
│   └── api/                      # Generated docs
└── tests/
    └── api/                      # API tests
```

### Multi-File (Large APIs)
```
project/
├── api/
│   ├── openapi.yaml              # Main file with $ref
│   └── openapi/
│       ├── info.yaml             # API metadata
│       ├── servers.yaml          # Server definitions
│       ├── paths/                # Split path definitions
│       │   ├── users.yaml
│       │   ├── orders.yaml
│       │   └── products.yaml
│       └── components/           # Split component definitions
│           ├── schemas/
│           │   ├── User.yaml
│           │   ├── Order.yaml
│           │   └── Product.yaml
│           ├── responses/
│           │   ├── Error.yaml
│           │   └── NotFound.yaml
│           ├── parameters/
│           │   ├── PageLimit.yaml
│           │   └── PageOffset.yaml
│           └── securitySchemes/
│               └── bearerAuth.yaml
├── docs/
│   ├── api/                      # Generated docs
│   └── api-design.md             # Design decisions
└── tests/
    └── api/
        └── contract/             # Contract tests
```

### Bundling Multi-File Specs

When using multi-file specifications, bundle them for distribution:

```bash
# Bundle with Swagger CLI
swagger-cli bundle -o dist/openapi.yaml -t yaml api/openapi.yaml

# Bundle with Redocly CLI
npm install -g @redocly/cli
redocly bundle api/openapi.yaml -o dist/openapi.yaml
```

---

## Workflow Tools

### Design-First Workflow

```bash
# 1. Create specification
# Edit api/openapi.yaml

# 2. Validate
spectral lint api/openapi.yaml

# 3. Generate mock server
prism mock api/openapi.yaml &

# 4. Generate server stubs
openapi-generator-cli generate -i api/openapi.yaml -g spring -o server/

# 5. Implement API following contract

# 6. Validate implementation
dredd api/openapi.yaml http://localhost:8080

# 7. Generate client SDKs
openapi-generator-cli generate -i api/openapi.yaml -g typescript-fetch -o client/

# 8. Generate documentation
redoc-cli bundle api/openapi.yaml -o docs/api.html
```

### Code-First Workflow

Use framework tools to generate OpenAPI from existing code:

**FastAPI (Python)**:
```python
from fastapi import FastAPI

app = FastAPI()

# FastAPI automatically generates OpenAPI
# Access at http://localhost:8000/openapi.json
```

**SpringDoc (Java/Spring)**:
```xml
<!-- Add SpringDoc dependency -->
<dependency>
    <groupId>org.springdoc</groupId>
    <artifactId>springdoc-openapi-ui</artifactId>
    <version>1.7.0</version>
</dependency>
```

**NestJS (Node.js/TypeScript)**:
```typescript
import { SwaggerModule, DocumentBuilder } from '@nestjs/swagger';

const config = new DocumentBuilder()
  .setTitle('API')
  .setVersion('1.0')
  .build();
const document = SwaggerModule.createDocument(app, config);
```

---

## CI/CD Integration

### Validation in CI Pipeline

**GitHub Actions Example**:
```yaml
name: OpenAPI Validation

on: [push, pull_request]

jobs:
  validate:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3

      - name: Validate OpenAPI Spec
        uses: docker://openapitools/openapi-generator-cli
        with:
          args: validate -i api/openapi.yaml

      - name: Lint with Spectral
        run: |
          npm install -g @stoplight/spectral-cli
          spectral lint api/openapi.yaml
```

### Breaking Change Detection

**Optic**:
```bash
npm install -g @useoptic/optic

# Track API changes
optic diff api/openapi.yaml --base main --head feature-branch
```

---

## Quick Reference

### HTTP Methods
- **GET** - Retrieve resource(s), idempotent, safe, cacheable
- **POST** - Create resource, non-idempotent, unsafe
- **PUT** - Replace resource, idempotent, unsafe
- **PATCH** - Update resource partially, non-idempotent, unsafe
- **DELETE** - Remove resource, idempotent, unsafe

### Status Codes
**Success (2xx)**:
- 200 OK - Successful GET, PUT, PATCH
- 201 Created - Successful POST with resource creation
- 204 No Content - Successful DELETE or POST without response body
- 207 Multi-Status - Batch operations with mixed results

**Redirection (3xx)**:
- 301 Moved Permanently - Resource moved to new URL
- 304 Not Modified - Cached version is still valid

**Client Error (4xx)**:
- 400 Bad Request - Invalid request format or parameters
- 401 Unauthorized - Authentication required or failed
- 403 Forbidden - Insufficient permissions
- 404 Not Found - Resource doesn't exist
- 409 Conflict - Resource state conflict
- 422 Unprocessable Entity - Validation error
- 429 Too Many Requests - Rate limit exceeded

**Server Error (5xx)**:
- 500 Internal Server Error - Unexpected server error
- 502 Bad Gateway - Invalid upstream response
- 503 Service Unavailable - Service temporarily unavailable
- 504 Gateway Timeout - Upstream timeout

### JSON Schema Types
**Primitives**:
- `string` - Text data
- `number` - Numeric (float/double)
- `integer` - Whole numbers
- `boolean` - true/false
- `null` - Null value

**Structured**:
- `object` - Key-value pairs
- `array` - Ordered list

**String Formats**:
- `date` - Full date (2025-10-17)
- `date-time` - Date and time with timezone (2025-10-17T10:30:00Z)
- `email` - Email address
- `uri` / `url` - URI/URL
- `uuid` - UUID (123e4567-e89b-12d3-a456-426614174000)
- `ipv4` / `ipv6` - IP addresses

**Validation Keywords**:
- `required` - Required properties in object
- `minLength` / `maxLength` - String length constraints
- `minimum` / `maximum` - Numeric bounds
- `pattern` - Regex pattern for strings
- `enum` - Fixed set of allowed values
- `format` - Semantic format hint

---

## Additional Resources

### Official Documentation
- **OpenAPI Specification**: https://spec.openapis.org/oas/latest.html
- **OpenAPI Initiative**: https://www.openapis.org/
- **JSON Schema**: https://json-schema.org/

### Learning Resources
- **Swagger Tutorial**: https://swagger.io/docs/specification/about/
- **OpenAPI Best Practices**: https://oai.github.io/Documentation/best-practices.html
- **REST API Design Rulebook**: https://www.oreilly.com/library/view/rest-api-design/9781449317904/
- **API Design Patterns**: https://microservice-api-patterns.org/

### Community
- **OpenAPI GitHub**: https://github.com/OAI/OpenAPI-Specification
- **Stack Overflow**: Tag `openapi` or `swagger`
- **OpenAPI Community Slack**: https://open-api.slack.com/
