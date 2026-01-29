# CI/CD Security Integration

Comprehensive guide to integrating security testing into DevSecOps pipelines.

## DevSecOps Pipeline Overview

```
┌─────────────────────────────────────────────────────────────────────┐
│                    DEVSECOPS PIPELINE                                │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  ┌─────────────────────────────────────────────────────────────┐    │
│  │                    DEVELOPMENT PHASE                        │    │
│  │  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐     │    │
│  │  │  IDE     │  │ Pre-     │  │ Commit   │  │ Pull     │     │    │
│  │  │ Plugins  │─►│ commit   │─►│ Signing  │─►│ Request  │     │    │
│  │  │ (SAST)   │  │ Hooks    │  │          │  │ Checks   │     │    │
│  │  └──────────┘  └──────────┘  └──────────┘  └──────────┘     │    │
│  │      │             │             │              │           │    │
│  │      ▼             ▼             ▼              ▼           │    │
│  │  Lint rules   Secret scan   Git history    SAST + SCA      │    │
│  │  Security     Hardcoded     Verified       Code review      │    │
│  │  patterns     credentials   commits        Security gates   │    │
│  └─────────────────────────────────────────────────────────────┘    │
│                              │                                      │
│                              ▼                                      │
│  ┌─────────────────────────────────────────────────────────────┐    │
│  │                    BUILD PHASE                              │    │
│  │  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐     │    │
│  │  │ SAST     │  │ SCA      │  │ Container│  │ IaC      │     │    │
│  │  │ Scan     │─►│ Scan     │─►│ Scan     │─►│ Scan     │     │    │
│  │  └──────────┘  └──────────┘  └──────────┘  └──────────┘     │    │
│  │      │             │             │              │           │    │
│  │      ▼             ▼             ▼              ▼           │    │
│  │  Source code   Dependencies   Base images   Terraform      │    │
│  │  vulnerabilities CVEs        CVEs + misconfig security     │    │
│  │                                                             │    │
│  │  GATE: Block build if critical/high vulnerabilities found   │    │
│  └─────────────────────────────────────────────────────────────┘    │
│                              │                                      │
│                              ▼                                      │
│  ┌─────────────────────────────────────────────────────────────┐    │
│  │                    TEST PHASE                               │    │
│  │  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐     │    │
│  │  │ DAST     │  │ IAST     │  │ Security │  │ API      │     │    │
│  │  │ Scan     │─►│ (opt)    │─►│ Unit     │─►│ Security │     │    │
│  │  │ (ZAP)    │  │          │  │ Tests    │  │ Tests    │     │    │
│  │  └──────────┘  └──────────┘  └──────────┘  └──────────┘     │    │
│  │      │             │             │              │           │    │
│  │      ▼             ▼             ▼              ▼           │    │
│  │  Running app   Instrumented Auth/Authz    OpenAPI spec     │    │
│  │  scanning     analysis     tests          validation       │    │
│  │                                                             │    │
│  │  GATE: Block deployment if security tests fail              │    │
│  └─────────────────────────────────────────────────────────────┘    │
│                              │                                      │
│                              ▼                                      │
│  ┌─────────────────────────────────────────────────────────────┐    │
│  │                    DEPLOY PHASE                             │    │
│  │  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐     │    │
│  │  │ Config   │  │ Runtime  │  │ Secrets  │  │ Compliance│    │    │
│  │  │ Audit    │─►│ Protect  │─►│ Mgmt     │─►│ Check     │    │    │
│  │  └──────────┘  └──────────┘  └──────────┘  └──────────┘     │    │
│  │      │             │             │              │           │    │
│  │      ▼             ▼             ▼              ▼           │    │
│  │  Prod configs  RASP/WAF    Vault/KMS      PCI/SOC2         │    │
│  │  verification  deployment  integration    validation       │    │
│  └─────────────────────────────────────────────────────────────┘    │
│                              │                                      │
│                              ▼                                      │
│  ┌─────────────────────────────────────────────────────────────┐    │
│  │                    OPERATE PHASE                            │    │
│  │  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐     │    │
│  │  │ Security │  │ Threat   │  │ Vuln     │  │ Incident │     │    │
│  │  │ Monitor  │─►│ Detection│─►│ Mgmt     │─►│ Response │     │    │
│  │  └──────────┘  └──────────┘  └──────────┘  └──────────┘     │    │
│  │      │             │             │              │           │    │
│  │      ▼             ▼             ▼              ▼           │    │
│  │  SIEM/logging  IDS/IPS      Patching       Playbooks       │    │
│  │  security      anomaly      schedule       automated        │    │
│  │  metrics       detection    tracking       response         │    │
│  └─────────────────────────────────────────────────────────────┘    │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

## GitHub Actions Security Pipeline

### Complete Example

```yaml
name: Security Scan

on:
  push:
    branches: [main, develop]
  pull_request:
    branches: [main, develop]
  schedule:
    - cron: '0 0 * * 0'  # Weekly on Sunday

jobs:
  secrets-scan:
    name: Secret Detection
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
        with:
          fetch-depth: 0  # Full history for secret scanning

      - name: Run GitLeaks
        uses: gitleaks/gitleaks-action@v2
        env:
          GITHUB_TOKEN: ${{ secrets.GITHUB_TOKEN }}

  sast:
    name: Static Analysis
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4

      - name: Run Semgrep
        uses: semgrep/semgrep-action@v1
        with:
          config: >-
            p/security-audit
            p/owasp-top-ten
            p/cwe-top-25
        env:
          SEMGREP_RULES: auto

      - name: Upload SARIF results
        uses: github/codeql-action/upload-sarif@v2
        with:
          sarif_file: semgrep.sarif

  sca:
    name: Dependency Scanning
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4

      - name: Setup Node.js
        uses: actions/setup-node@v4
        with:
          node-version: '18'

      - name: Install dependencies
        run: npm ci

      - name: Run Snyk
        uses: snyk/actions/node@master
        env:
          SNYK_TOKEN: ${{ secrets.SNYK_TOKEN }}
        with:
          args: --severity-threshold=high --fail-on=upgradable
          command: test

      - name: Run npm audit
        run: npm audit --audit-level=high

  container-scan:
    name: Container Security
    runs-on: ubuntu-latest
    if: github.event_name == 'push'
    steps:
      - uses: actions/checkout@v4

      - name: Build image
        run: docker build -t myapp:${{ github.sha }} .

      - name: Run Trivy
        uses: aquasecurity/trivy-action@master
        with:
          image-ref: myapp:${{ github.sha }}
          format: 'sarif'
          output: 'trivy-results.sarif'
          severity: 'CRITICAL,HIGH'
          exit-code: '1'

      - name: Upload Trivy results
        uses: github/codeql-action/upload-sarif@v2
        with:
          sarif_file: 'trivy-results.sarif'

  dast:
    name: Dynamic Analysis
    runs-on: ubuntu-latest
    needs: [sast, sca]
    if: github.event_name == 'pull_request'
    steps:
      - uses: actions/checkout@v4

      - name: Start application
        run: |
          docker-compose up -d
          sleep 10  # Wait for app to start

      - name: Wait for app
        run: |
          timeout 60 bash -c 'until curl -f http://localhost:8080/health; do sleep 2; done'

      - name: Run OWASP ZAP
        uses: zaproxy/action-full-scan@v0.8.0
        with:
          target: 'http://localhost:8080'
          rules_file_name: '.zap/rules.tsv'
          cmd_options: '-a'
          allow_issue_writing: false

      - name: Stop application
        if: always()
        run: docker-compose down

  security-gate:
    name: Security Gate
    runs-on: ubuntu-latest
    needs: [secrets-scan, sast, sca, container-scan]
    if: always()
    steps:
      - name: Check security results
        run: |
          if [ "${{ needs.secrets-scan.result }}" != "success" ] || \
             [ "${{ needs.sast.result }}" != "success" ] || \
             [ "${{ needs.sca.result }}" != "success" ] || \
             [ "${{ needs.container-scan.result }}" != "success" ]; then
            echo "Security checks failed"
            exit 1
          fi
```

## GitLab CI Security Pipeline

```yaml
stages:
  - security-scan
  - build
  - test
  - deploy

variables:
  SECURE_ANALYZERS_PREFIX: "registry.gitlab.com/gitlab-org/security-products/analyzers"

secret-detection:
  stage: security-scan
  image: $SECURE_ANALYZERS_PREFIX/secrets:latest
  script:
    - /analyzer run
  artifacts:
    reports:
      secret_detection: gl-secret-detection-report.json

sast:
  stage: security-scan
  image: $SECURE_ANALYZERS_PREFIX/semgrep:latest
  script:
    - /analyzer run
  artifacts:
    reports:
      sast: gl-sast-report.json
  rules:
    - if: $CI_PIPELINE_SOURCE == "merge_request_event"
    - if: $CI_COMMIT_BRANCH == $CI_DEFAULT_BRANCH

dependency-scanning:
  stage: security-scan
  image: $SECURE_ANALYZERS_PREFIX/gemnasium:latest
  script:
    - /analyzer run
  artifacts:
    reports:
      dependency_scanning: gl-dependency-scanning-report.json

container-scanning:
  stage: test
  image: $SECURE_ANALYZERS_PREFIX/trivy:latest
  variables:
    IMAGE: $CI_REGISTRY_IMAGE:$CI_COMMIT_SHA
  script:
    - /analyzer run
  artifacts:
    reports:
      container_scanning: gl-container-scanning-report.json

dast:
  stage: test
  image: registry.gitlab.com/gitlab-org/security-products/dast:latest
  variables:
    DAST_WEBSITE: http://staging.example.com
  script:
    - /analyze
  artifacts:
    reports:
      dast: gl-dast-report.json
  only:
    - schedules
```

## Jenkins Pipeline

```groovy
pipeline {
    agent any

    environment {
        SNYK_TOKEN = credentials('snyk-api-token')
        DOCKER_IMAGE = "myapp:${env.BUILD_NUMBER}"
    }

    stages {
        stage('Checkout') {
            steps {
                checkout scm
            }
        }

        stage('Secret Scanning') {
            steps {
                sh '''
                    docker run --rm -v $(pwd):/src \
                    zricethezav/gitleaks:latest \
                    detect --source /src --no-git
                '''
            }
        }

        stage('SAST') {
            parallel {
                stage('Semgrep') {
                    steps {
                        sh '''
                            docker run --rm -v $(pwd):/src \
                            returntocorp/semgrep \
                            --config=auto /src
                        '''
                    }
                }
                stage('SonarQube') {
                    steps {
                        withSonarQubeEnv('SonarQube') {
                            sh 'mvn sonar:sonar'
                        }
                    }
                }
            }
        }

        stage('SCA') {
            steps {
                sh '''
                    npm install -g snyk
                    snyk test --severity-threshold=high
                '''
            }
        }

        stage('Build') {
            steps {
                sh "docker build -t ${DOCKER_IMAGE} ."
            }
        }

        stage('Container Scan') {
            steps {
                sh """
                    docker run --rm -v /var/run/docker.sock:/var/run/docker.sock \
                    aquasec/trivy image --severity HIGH,CRITICAL \
                    --exit-code 1 ${DOCKER_IMAGE}
                """
            }
        }

        stage('Deploy to Test') {
            steps {
                sh 'docker-compose -f docker-compose.test.yml up -d'
            }
        }

        stage('DAST') {
            steps {
                sh '''
                    docker run --rm -t \
                    -v $(pwd):/zap/wrk:rw \
                    owasp/zap2docker-stable \
                    zap-baseline.py \
                    -t http://test.example.com \
                    -r zap-report.html
                '''
            }
        }
    }

    post {
        always {
            sh 'docker-compose -f docker-compose.test.yml down'
            publishHTML([
                reportDir: '.',
                reportFiles: 'zap-report.html',
                reportName: 'ZAP Security Report'
            ])
        }
        failure {
            mail to: 'security@example.com',
                 subject: "Security Scan Failed: ${env.JOB_NAME} - ${env.BUILD_NUMBER}",
                 body: "Security vulnerabilities detected. Check ${env.BUILD_URL}"
        }
    }
}
```

## CircleCI Configuration

```yaml
version: 2.1

orbs:
  snyk: snyk/snyk@1.5.0

jobs:
  security-scan:
    docker:
      - image: cimg/node:18.0
    steps:
      - checkout

      # Secret scanning
      - run:
          name: GitLeaks scan
          command: |
            wget https://github.com/gitleaks/gitleaks/releases/download/v8.18.0/gitleaks_8.18.0_linux_x64.tar.gz
            tar -xzf gitleaks_8.18.0_linux_x64.tar.gz
            ./gitleaks detect --source . --no-git

      # SAST
      - run:
          name: Semgrep scan
          command: |
            pip install semgrep
            semgrep --config=auto --json --output=semgrep-report.json

      # SCA
      - snyk/scan:
          severity-threshold: high
          fail-on-issues: true

  container-scan:
    machine: true
    steps:
      - checkout
      - run:
          name: Build image
          command: docker build -t myapp:${CIRCLE_SHA1} .
      - run:
          name: Trivy scan
          command: |
            docker run --rm -v /var/run/docker.sock:/var/run/docker.sock \
            aquasec/trivy image --severity HIGH,CRITICAL \
            myapp:${CIRCLE_SHA1}

  dast-scan:
    docker:
      - image: cimg/base:stable
    steps:
      - checkout
      - run:
          name: Start app
          command: docker-compose up -d
      - run:
          name: ZAP scan
          command: |
            docker run --network container:app \
            owasp/zap2docker-stable \
            zap-baseline.py -t http://localhost:8080

workflows:
  security-pipeline:
    jobs:
      - security-scan
      - container-scan:
          requires:
            - security-scan
      - dast-scan:
          requires:
            - container-scan
```

## Pre-commit Hooks

### .pre-commit-config.yaml

```yaml
repos:
  - repo: https://github.com/gitleaks/gitleaks
    rev: v8.18.0
    hooks:
      - id: gitleaks

  - repo: https://github.com/Yelp/detect-secrets
    rev: v1.4.0
    hooks:
      - id: detect-secrets
        args: ['--baseline', '.secrets.baseline']

  - repo: https://github.com/returntocorp/semgrep
    rev: v1.45.0
    hooks:
      - id: semgrep
        args: ['--config=auto', '--error']

  - repo: https://github.com/PyCQA/bandit
    rev: 1.7.5
    hooks:
      - id: bandit
        args: ['-c', '.bandit', '-r', '.']
        files: \.py$

  - repo: https://github.com/pre-commit/mirrors-eslint
    rev: v8.54.0
    hooks:
      - id: eslint
        args: ['--plugin', 'security']
        files: \.(js|jsx|ts|tsx)$
```

## Security Gate Configuration

### Quality Gates by Severity

```yaml
# Example: security-gates.yml
gates:
  # Block on critical vulnerabilities
  critical:
    action: block
    threshold: 0
    tools:
      - sast
      - sca
      - dast
      - container

  # Block on high vulnerabilities (with exceptions)
  high:
    action: block
    threshold: 3
    tools:
      - sast
      - sca
      - dast
    exceptions:
      - CVE-2023-12345  # Has compensating controls
      - GHSA-xxxx-yyyy  # Not applicable to our use case

  # Warn on medium vulnerabilities
  medium:
    action: warn
    threshold: 10
    tools:
      - sast
      - sca

  # Track but don't block on low
  low:
    action: track
    tools:
      - sast
```

## Best Practices

### Development Phase
1. **IDE Integration**: Install security plugins (e.g., SonarLint, Snyk)
2. **Pre-commit Hooks**: Scan for secrets and basic security issues
3. **Commit Signing**: Require GPG-signed commits
4. **Branch Protection**: Require security checks to pass before merge

### Build Phase
1. **Fast Feedback**: Run SAST and SCA on every PR
2. **Fail Fast**: Block builds on critical/high vulnerabilities
3. **Parallel Scans**: Run security tools in parallel
4. **Cache Results**: Cache dependencies and scan results

### Test Phase
1. **Staging Environment**: Run DAST against staging, not production
2. **Authenticated Scans**: Test protected endpoints
3. **API Security**: Include API-specific security tests
4. **Security Unit Tests**: Test security controls (auth, authz)

### Deploy Phase
1. **Configuration Validation**: Scan production configs
2. **Secret Management**: Verify secrets from vault/KMS
3. **Runtime Protection**: Deploy WAF/RASP
4. **Compliance Checks**: Verify regulatory requirements

### Operate Phase
1. **Continuous Monitoring**: Monitor for new vulnerabilities
2. **Incident Response**: Automate security alerting
3. **Patch Management**: Track and apply security patches
4. **Security Metrics**: Measure MTTR (Mean Time To Remediation)

## Common Pitfalls

### Avoid
- Running DAST against production
- Ignoring false positives without triage
- Setting thresholds too high (blocks everything)
- Not updating security tools regularly
- Scanning without fixing (alert fatigue)

### Do
- Start with warnings, gradually enforce blocking
- Document exceptions and compensating controls
- Regularly tune rules to reduce false positives
- Track security metrics over time
- Educate developers on security findings
