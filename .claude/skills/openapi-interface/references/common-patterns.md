# Common OpenAPI Patterns

## 1. CRUD Operations Pattern

```yaml
paths:
  /items:
    get:
      summary: List items
      operationId: listItems
      parameters:
        - $ref: '#/components/parameters/PageLimit'
        - $ref: '#/components/parameters/PageOffset'
      responses:
        '200':
          description: List of items
          content:
            application/json:
              schema:
                $ref: '#/components/schemas/ItemList'
    post:
      summary: Create item
      operationId: createItem
      requestBody:
        required: true
        content:
          application/json:
            schema:
              $ref: '#/components/schemas/ItemInput'
      responses:
        '201':
          description: Item created
          headers:
            Location:
              schema:
                type: string
                format: uri
          content:
            application/json:
              schema:
                $ref: '#/components/schemas/Item'

  /items/{itemId}:
    parameters:
      - name: itemId
        in: path
        required: true
        schema:
          type: string
          format: uuid
    get:
      summary: Get item
      operationId: getItem
      responses:
        '200':
          description: Item details
          content:
            application/json:
              schema:
                $ref: '#/components/schemas/Item'
        '404':
          $ref: '#/components/responses/NotFound'
    put:
      summary: Replace item
      operationId: replaceItem
      requestBody:
        required: true
        content:
          application/json:
            schema:
              $ref: '#/components/schemas/ItemInput'
      responses:
        '200':
          description: Item updated
          content:
            application/json:
              schema:
                $ref: '#/components/schemas/Item'
    patch:
      summary: Update item
      operationId: updateItem
      requestBody:
        required: true
        content:
          application/json:
            schema:
              $ref: '#/components/schemas/ItemPatch'
      responses:
        '200':
          description: Item updated
          content:
            application/json:
              schema:
                $ref: '#/components/schemas/Item'
    delete:
      summary: Delete item
      operationId: deleteItem
      responses:
        '204':
          description: Item deleted
        '404':
          $ref: '#/components/responses/NotFound'
```

---

## 2. Nested Resources Pattern

```yaml
paths:
  /users/{userId}/orders:
    parameters:
      - name: userId
        in: path
        required: true
        schema:
          type: string
    get:
      summary: List user orders
      operationId: listUserOrders
      responses:
        '200':
          description: User's orders
          content:
            application/json:
              schema:
                type: array
                items:
                  $ref: '#/components/schemas/Order'
    post:
      summary: Create order for user
      operationId: createUserOrder
      requestBody:
        required: true
        content:
          application/json:
            schema:
              $ref: '#/components/schemas/OrderInput'
      responses:
        '201':
          description: Order created
          content:
            application/json:
              schema:
                $ref: '#/components/schemas/Order'
```

---

## 3. Search and Filter Pattern

```yaml
paths:
  /products:
    get:
      summary: Search products
      operationId: searchProducts
      parameters:
        - name: q
          in: query
          description: Search query
          schema:
            type: string
        - name: category
          in: query
          description: Filter by category
          schema:
            type: string
            enum: [electronics, clothing, books]
        - name: minPrice
          in: query
          description: Minimum price filter
          schema:
            type: number
            format: double
            minimum: 0
        - name: maxPrice
          in: query
          description: Maximum price filter
          schema:
            type: number
            format: double
        - name: sort
          in: query
          description: Sort order
          schema:
            type: string
            enum: [price_asc, price_desc, name_asc, name_desc, newest]
            default: newest
        - name: limit
          in: query
          schema:
            type: integer
            default: 20
            minimum: 1
            maximum: 100
        - name: offset
          in: query
          schema:
            type: integer
            default: 0
            minimum: 0
      responses:
        '200':
          description: Search results
          content:
            application/json:
              schema:
                type: object
                properties:
                  data:
                    type: array
                    items:
                      $ref: '#/components/schemas/Product'
                  pagination:
                    $ref: '#/components/schemas/Pagination'
                  facets:
                    type: object
                    description: Available filters with counts
```

---

## 4. Batch Operations Pattern

```yaml
paths:
  /items/batch:
    post:
      summary: Create multiple items
      operationId: batchCreateItems
      requestBody:
        required: true
        content:
          application/json:
            schema:
              type: object
              properties:
                items:
                  type: array
                  items:
                    $ref: '#/components/schemas/ItemInput'
                  minItems: 1
                  maxItems: 100
      responses:
        '207':
          description: Multi-status response
          content:
            application/json:
              schema:
                type: object
                properties:
                  results:
                    type: array
                    items:
                      type: object
                      properties:
                        status:
                          type: integer
                          description: HTTP status code for this item
                        item:
                          $ref: '#/components/schemas/Item'
                        error:
                          $ref: '#/components/schemas/Error'
```

---

## 5. Async Operations Pattern

```yaml
paths:
  /exports:
    post:
      summary: Request data export
      operationId: createExport
      requestBody:
        required: true
        content:
          application/json:
            schema:
              type: object
              properties:
                format:
                  type: string
                  enum: [csv, json, xlsx]
                filters:
                  type: object
      responses:
        '202':
          description: Export accepted for processing
          headers:
            Location:
              description: URL to check export status
              schema:
                type: string
                format: uri
          content:
            application/json:
              schema:
                type: object
                properties:
                  exportId:
                    type: string
                    format: uuid
                  status:
                    type: string
                    enum: [pending, processing, completed, failed]
                  statusUrl:
                    type: string
                    format: uri

  /exports/{exportId}:
    get:
      summary: Get export status
      operationId: getExportStatus
      parameters:
        - name: exportId
          in: path
          required: true
          schema:
            type: string
            format: uuid
      responses:
        '200':
          description: Export status
          content:
            application/json:
              schema:
                type: object
                properties:
                  exportId:
                    type: string
                  status:
                    type: string
                    enum: [pending, processing, completed, failed]
                  progress:
                    type: integer
                    minimum: 0
                    maximum: 100
                  downloadUrl:
                    type: string
                    format: uri
                    description: Available when status is completed
                  error:
                    $ref: '#/components/schemas/Error'
```

---

## Validation and Testing

### Validate OpenAPI Specifications

Validate your OpenAPI specification files to ensure they are syntactically correct and follow best practices.

```bash
# Using openapi-generator-cli (Docker)
docker run --rm -v "${PWD}:/local" openapitools/openapi-generator-cli validate -i /local/openapi.yaml

# Using Swagger CLI (npm)
npm install -g @apidevtools/swagger-cli
swagger-cli validate openapi.yaml

# Using Spectral for linting with custom rules
npm install -g @stoplight/spectral-cli
spectral lint openapi.yaml

# Spectral with custom ruleset
spectral lint openapi.yaml --ruleset .spectral.yaml
```

### Generate Mock Servers

Create mock servers from your OpenAPI specification for testing and development.

```bash
# Using Prism (recommended)
npm install -g @stoplight/prism-cli
prism mock openapi.yaml

# Run Prism on a specific port
prism mock -p 4010 openapi.yaml

# With dynamic response generation
prism mock --dynamic openapi.yaml

# Using openapi-mock-server
npx @open-api-tools/openapi-mock-server openapi.yaml

# Using Docker
docker run --init --rm -v ${PWD}:/tmp -p 4010:4010 stoplight/prism:4 mock -h 0.0.0.0 /tmp/openapi.yaml
```

### Generate Client SDKs

Generate client libraries in various languages from your OpenAPI specification.

```bash
# Generate TypeScript/Fetch client
docker run --rm -v "${PWD}:/local" openapitools/openapi-generator-cli generate \
  -i /local/openapi.yaml \
  -g typescript-fetch \
  -o /local/generated-client/typescript

# Generate Python client
docker run --rm -v "${PWD}:/local" openapitools/openapi-generator-cli generate \
  -i /local/openapi.yaml \
  -g python \
  -o /local/generated-client/python

# Generate Java client
docker run --rm -v "${PWD}:/local" openapitools/openapi-generator-cli generate \
  -i /local/openapi.yaml \
  -g java \
  -o /local/generated-client/java

# Generate Go client
docker run --rm -v "${PWD}:/local" openapitools/openapi-generator-cli generate \
  -i /local/openapi.yaml \
  -g go \
  -o /local/generated-client/go

# List all available generators
docker run --rm openapitools/openapi-generator-cli list
```

### Generate Server Stubs

Generate server-side code stubs from your OpenAPI specification.

```bash
# Generate Node.js/Express server
docker run --rm -v "${PWD}:/local" openapitools/openapi-generator-cli generate \
  -i /local/openapi.yaml \
  -g nodejs-express-server \
  -o /local/generated-server

# Generate Python/FastAPI server
docker run --rm -v "${PWD}:/local" openapitools/openapi-generator-cli generate \
  -i /local/openapi.yaml \
  -g python-fastapi \
  -o /local/generated-server

# Generate Spring Boot server
docker run --rm -v "${PWD}:/local" openapitools/openapi-generator-cli generate \
  -i /local/openapi.yaml \
  -g spring \
  -o /local/generated-server
```

### Contract Testing

Validate that your API implementation matches your OpenAPI specification.

```bash
# Using Dredd for contract testing
npm install -g dredd
dredd openapi.yaml http://localhost:8080

# Using Schemathesis for property-based testing
pip install schemathesis
schemathesis run openapi.yaml --base-url http://localhost:8080

# Run specific test cases
schemathesis run openapi.yaml --base-url http://localhost:8080 --checks all

# Using Postman (import OpenAPI and run collection tests)
newman run postman_collection.json --environment postman_environment.json
```

### Documentation Generation

Generate interactive API documentation from your OpenAPI specification.

```bash
# Using Redoc (standalone HTML)
npx redoc-cli bundle openapi.yaml -o docs/api.html

# Using Swagger UI (serve locally)
docker run -p 8080:8080 -e SWAGGER_JSON=/openapi.yaml -v ${PWD}:/openapi swaggerapi/swagger-ui

# Using RapiDoc
# Create an HTML file that references your openapi.yaml with RapiDoc web component
```

---

## Best Practices Summary

### When to Use Each Pattern

1. **CRUD Operations**: Standard resource management endpoints
2. **Nested Resources**: When resources have clear parent-child relationships (limit to 2 levels)
3. **Search and Filter**: For list endpoints requiring complex filtering and sorting
4. **Batch Operations**: When clients need to perform multiple operations in a single request
5. **Async Operations**: For long-running operations that cannot complete synchronously

### Common Mistakes to Avoid

1. **Don't use verbs in URLs** - Use HTTP methods instead (GET, POST, etc.)
2. **Don't nest resources too deeply** - Limit to 2 levels maximum
3. **Don't ignore status codes** - Use appropriate HTTP status codes for each scenario
4. **Don't skip examples** - Always include request/response examples
5. **Don't forget security** - Define security schemes and apply them consistently
6. **Don't duplicate schemas** - Use `$ref` to reuse component definitions
7. **Don't forget validation** - Add JSON Schema constraints for all inputs
