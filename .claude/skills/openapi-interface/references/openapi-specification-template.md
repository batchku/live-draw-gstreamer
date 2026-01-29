# OpenAPI Specification Template

## Minimal OpenAPI 3.1 Template

Use this template as a starting point for creating OpenAPI specifications. It includes all essential sections with common patterns.

```yaml
openapi: 3.1.0
info:
  title: API Name
  version: 1.0.0
  description: API description and purpose
  contact:
    name: API Support
    email: support@example.com
  license:
    name: Apache 2.0
    url: https://www.apache.org/licenses/LICENSE-2.0.html

servers:
  - url: https://api.example.com/v1
    description: Production server
  - url: https://staging-api.example.com/v1
    description: Staging server
  - url: http://localhost:8080/v1
    description: Local development

paths:
  /resource:
    get:
      summary: Brief operation summary
      description: Detailed operation description
      operationId: getResource
      tags:
        - Resources
      parameters:
        - name: limit
          in: query
          description: Number of items to return
          required: false
          schema:
            type: integer
            default: 20
            minimum: 1
            maximum: 100
        - name: offset
          in: query
          description: Number of items to skip
          required: false
          schema:
            type: integer
            default: 0
            minimum: 0
      responses:
        '200':
          description: Successful response
          content:
            application/json:
              schema:
                $ref: '#/components/schemas/ResourceList'
              examples:
                default:
                  value:
                    data:
                      - id: "123e4567-e89b-12d3-a456-426614174000"
                        name: "Example Resource"
                        createdAt: "2025-10-17T10:30:00Z"
                    pagination:
                      total: 1
                      limit: 20
                      offset: 0
        '400':
          $ref: '#/components/responses/BadRequest'
        '401':
          $ref: '#/components/responses/Unauthorized'
        '500':
          $ref: '#/components/responses/InternalServerError'
      security:
        - bearerAuth: []

    post:
      summary: Create new resource
      description: Creates a new resource with the provided data
      operationId: createResource
      tags:
        - Resources
      requestBody:
        required: true
        content:
          application/json:
            schema:
              $ref: '#/components/schemas/ResourceInput'
            examples:
              default:
                value:
                  name: "New Resource"
      responses:
        '201':
          description: Resource created successfully
          headers:
            Location:
              description: URL of the created resource
              schema:
                type: string
                format: uri
          content:
            application/json:
              schema:
                $ref: '#/components/schemas/Resource'
        '400':
          $ref: '#/components/responses/BadRequest'
        '401':
          $ref: '#/components/responses/Unauthorized'
        '422':
          $ref: '#/components/responses/UnprocessableEntity'
      security:
        - bearerAuth: []

  /resource/{resourceId}:
    parameters:
      - name: resourceId
        in: path
        required: true
        description: Unique resource identifier
        schema:
          type: string
          format: uuid

    get:
      summary: Get resource by ID
      description: Retrieves a specific resource by its unique identifier
      operationId: getResourceById
      tags:
        - Resources
      responses:
        '200':
          description: Resource found
          content:
            application/json:
              schema:
                $ref: '#/components/schemas/Resource'
        '404':
          $ref: '#/components/responses/NotFound'
        '401':
          $ref: '#/components/responses/Unauthorized'
      security:
        - bearerAuth: []

    put:
      summary: Replace resource
      description: Replaces the entire resource with new data
      operationId: replaceResource
      tags:
        - Resources
      requestBody:
        required: true
        content:
          application/json:
            schema:
              $ref: '#/components/schemas/ResourceInput'
      responses:
        '200':
          description: Resource replaced successfully
          content:
            application/json:
              schema:
                $ref: '#/components/schemas/Resource'
        '404':
          $ref: '#/components/responses/NotFound'
        '400':
          $ref: '#/components/responses/BadRequest'
        '401':
          $ref: '#/components/responses/Unauthorized'
      security:
        - bearerAuth: []

    patch:
      summary: Update resource
      description: Partially updates the resource
      operationId: updateResource
      tags:
        - Resources
      requestBody:
        required: true
        content:
          application/json:
            schema:
              $ref: '#/components/schemas/ResourcePatch'
      responses:
        '200':
          description: Resource updated successfully
          content:
            application/json:
              schema:
                $ref: '#/components/schemas/Resource'
        '404':
          $ref: '#/components/responses/NotFound'
        '400':
          $ref: '#/components/responses/BadRequest'
        '401':
          $ref: '#/components/responses/Unauthorized'
      security:
        - bearerAuth: []

    delete:
      summary: Delete resource
      description: Permanently deletes the resource
      operationId: deleteResource
      tags:
        - Resources
      responses:
        '204':
          description: Resource deleted successfully
        '404':
          $ref: '#/components/responses/NotFound'
        '401':
          $ref: '#/components/responses/Unauthorized'
      security:
        - bearerAuth: []

components:
  schemas:
    Resource:
      type: object
      required:
        - id
        - name
        - createdAt
      properties:
        id:
          type: string
          format: uuid
          description: Unique resource identifier
          readOnly: true
        name:
          type: string
          minLength: 1
          maxLength: 255
          description: Resource name
        description:
          type: string
          maxLength: 1000
          description: Optional resource description
        createdAt:
          type: string
          format: date-time
          description: Creation timestamp
          readOnly: true
        updatedAt:
          type: string
          format: date-time
          description: Last update timestamp
          readOnly: true
      example:
        id: "123e4567-e89b-12d3-a456-426614174000"
        name: "Example Resource"
        description: "This is an example resource"
        createdAt: "2025-10-17T10:30:00Z"
        updatedAt: "2025-10-17T10:30:00Z"

    ResourceInput:
      type: object
      required:
        - name
      properties:
        name:
          type: string
          minLength: 1
          maxLength: 255
          description: Resource name
        description:
          type: string
          maxLength: 1000
          description: Optional resource description
      example:
        name: "New Resource"
        description: "A newly created resource"

    ResourcePatch:
      type: object
      properties:
        name:
          type: string
          minLength: 1
          maxLength: 255
          description: Resource name
        description:
          type: string
          maxLength: 1000
          description: Optional resource description
      example:
        name: "Updated Name"

    ResourceList:
      type: object
      required:
        - data
        - pagination
      properties:
        data:
          type: array
          items:
            $ref: '#/components/schemas/Resource'
        pagination:
          $ref: '#/components/schemas/Pagination'

    Pagination:
      type: object
      required:
        - total
        - limit
        - offset
      properties:
        total:
          type: integer
          description: Total number of items available
          minimum: 0
        limit:
          type: integer
          description: Maximum items per page
          minimum: 1
        offset:
          type: integer
          description: Number of items skipped
          minimum: 0
        next:
          type: string
          format: uri
          description: Link to next page (null if last page)
        previous:
          type: string
          format: uri
          description: Link to previous page (null if first page)

    Error:
      type: object
      required:
        - code
        - message
      properties:
        code:
          type: string
          description: Error code for programmatic handling
        message:
          type: string
          description: Human-readable error message
        details:
          type: array
          description: Additional error details
          items:
            type: object
            properties:
              field:
                type: string
                description: Field that caused the error
              issue:
                type: string
                description: Description of the issue
      example:
        code: "VALIDATION_ERROR"
        message: "The request contains invalid data"
        details:
          - field: "name"
            issue: "Name is required and cannot be empty"

  parameters:
    PageLimit:
      name: limit
      in: query
      description: Maximum number of items to return
      required: false
      schema:
        type: integer
        default: 20
        minimum: 1
        maximum: 100

    PageOffset:
      name: offset
      in: query
      description: Number of items to skip
      required: false
      schema:
        type: integer
        default: 0
        minimum: 0

    ResourceId:
      name: resourceId
      in: path
      required: true
      description: Unique resource identifier
      schema:
        type: string
        format: uuid

  responses:
    BadRequest:
      description: Invalid request parameters or malformed request
      content:
        application/json:
          schema:
            $ref: '#/components/schemas/Error'
          example:
            code: "BAD_REQUEST"
            message: "The request is malformed or contains invalid parameters"

    Unauthorized:
      description: Authentication required or authentication failed
      content:
        application/json:
          schema:
            $ref: '#/components/schemas/Error'
          example:
            code: "UNAUTHORIZED"
            message: "Valid authentication credentials are required"

    Forbidden:
      description: Insufficient permissions to access the resource
      content:
        application/json:
          schema:
            $ref: '#/components/schemas/Error'
          example:
            code: "FORBIDDEN"
            message: "You do not have permission to access this resource"

    NotFound:
      description: Resource not found
      content:
        application/json:
          schema:
            $ref: '#/components/schemas/Error'
          example:
            code: "NOT_FOUND"
            message: "The requested resource does not exist"

    UnprocessableEntity:
      description: Request validation failed
      content:
        application/json:
          schema:
            $ref: '#/components/schemas/Error'
          example:
            code: "VALIDATION_ERROR"
            message: "Request validation failed"
            details:
              - field: "email"
                issue: "Must be a valid email address"

    InternalServerError:
      description: Internal server error
      content:
        application/json:
          schema:
            $ref: '#/components/schemas/Error'
          example:
            code: "INTERNAL_ERROR"
            message: "An unexpected error occurred"

  securitySchemes:
    bearerAuth:
      type: http
      scheme: bearer
      bearerFormat: JWT
      description: JWT bearer token authentication

    apiKey:
      type: apiKey
      in: header
      name: X-API-Key
      description: API key authentication

    oauth2:
      type: oauth2
      description: OAuth 2.0 authentication
      flows:
        authorizationCode:
          authorizationUrl: https://auth.example.com/oauth/authorize
          tokenUrl: https://auth.example.com/oauth/token
          scopes:
            read: Read access to resources
            write: Write access to resources
            admin: Administrative access

security:
  - bearerAuth: []
```

---

## JSON Schema Types Reference

### Primitives
- **string** - Text data
- **number** - Numeric (float/double)
- **integer** - Whole numbers
- **boolean** - true/false
- **null** - Null value

### Structured
- **object** - Key-value pairs
- **array** - Ordered list

### String Formats
- **date** - Full date (e.g., "2025-10-17")
- **date-time** - Date and time with timezone (e.g., "2025-10-17T10:30:00Z")
- **email** - Email address
- **uri** - URI/URL
- **uuid** - UUID (e.g., "123e4567-e89b-12d3-a456-426614174000")
- **ipv4** - IPv4 address
- **ipv6** - IPv6 address

### Validation Keywords
- **required** - Required properties in object
- **minLength** / **maxLength** - String length constraints
- **minimum** / **maximum** - Numeric bounds
- **pattern** - Regex pattern for strings
- **enum** - Fixed set of allowed values
- **format** - Semantic format hint
