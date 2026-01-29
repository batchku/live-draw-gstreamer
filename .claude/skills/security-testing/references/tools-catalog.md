# Security Testing Tools Catalog

Comprehensive reference of security testing tools organized by category.

## Tool Categories Overview

```
┌─────────────────────────────────────────────────────────────────────┐
│                    SECURITY TESTING TOOLS                            │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  STATIC ANALYSIS (SAST)                                             │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │ Tool            │ Languages        │ Best For                 │  │
│  ├─────────────────┼──────────────────┼──────────────────────────┤  │
│  │ Semgrep         │ Multi-language   │ Custom rules, fast       │  │
│  │ SonarQube       │ Multi-language   │ Code quality + security  │  │
│  │ CodeQL          │ Multi-language   │ Deep semantic analysis   │  │
│  │ Bandit          │ Python           │ Python-specific issues   │  │
│  │ ESLint-security │ JavaScript       │ JS security rules        │  │
│  │ Brakeman        │ Ruby             │ Rails security           │  │
│  │ gosec           │ Go               │ Go security linting      │  │
│  └─────────────────┴──────────────────┴──────────────────────────┘  │
│                                                                     │
│  DYNAMIC ANALYSIS (DAST)                                            │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │ Tool            │ Type             │ Best For                 │  │
│  ├─────────────────┼──────────────────┼──────────────────────────┤  │
│  │ OWASP ZAP       │ Web scanner      │ Free, CI/CD integration  │  │
│  │ Burp Suite      │ Web scanner      │ Comprehensive scanning   │  │
│  │ Nuclei          │ Vuln scanner     │ Template-based scanning  │  │
│  │ Nikto           │ Web scanner      │ Quick web server scan    │  │
│  │ SQLMap          │ SQL injection    │ Automated SQLi testing   │  │
│  │ XSStrike        │ XSS scanner      │ Advanced XSS detection   │  │
│  └─────────────────┴──────────────────┴──────────────────────────┘  │
│                                                                     │
│  DEPENDENCY SCANNING (SCA)                                          │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │ Tool            │ Ecosystems       │ Best For                 │  │
│  ├─────────────────┼──────────────────┼──────────────────────────┤  │
│  │ Snyk            │ Multi-ecosystem  │ Developer-friendly       │  │
│  │ Dependabot      │ GitHub           │ Auto-PRs for updates     │  │
│  │ OWASP Dep-Check │ Java, .NET       │ OWASP database           │  │
│  │ Trivy           │ Containers, IaC  │ Container + infra        │  │
│  │ npm audit       │ Node.js          │ Native npm scanning      │  │
│  │ safety          │ Python           │ Python dependency check  │  │
│  └─────────────────┴──────────────────┴──────────────────────────┘  │
│                                                                     │
│  SECRET DETECTION                                                   │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │ Tool            │ Features         │ Best For                 │  │
│  ├─────────────────┼──────────────────┼──────────────────────────┤  │
│  │ GitLeaks        │ Git history scan │ Pre-commit, CI/CD        │  │
│  │ TruffleHog      │ Entropy analysis │ Deep secret detection    │  │
│  │ detect-secrets  │ Allowlisting     │ Low false positives      │  │
│  │ git-secrets     │ AWS patterns     │ AWS credential detection │  │
│  └─────────────────┴──────────────────┴──────────────────────────┘  │
│                                                                     │
│  CONTAINER/INFRASTRUCTURE                                           │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │ Tool            │ Target           │ Best For                 │  │
│  ├─────────────────┼──────────────────┼──────────────────────────┤  │
│  │ Trivy           │ Containers, IaC  │ All-in-one scanning      │  │
│  │ Clair           │ Container images │ CVE detection            │  │
│  │ Checkov         │ IaC (Terraform)  │ Infrastructure security  │  │
│  │ kube-bench      │ Kubernetes       │ CIS benchmark            │  │
│  │ Prowler         │ AWS              │ Cloud security           │  │
│  └─────────────────┴──────────────────┴──────────────────────────┘  │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

## SAST Tools

### Semgrep

**Overview**: Fast, open-source static analysis with customizable rules.

**Languages**: Python, JavaScript, Java, Go, Ruby, PHP, C, C++, TypeScript, and more.

**Key Features**:
- Custom rule creation with simple syntax
- Fast analysis (typically seconds)
- CI/CD integration
- IDE plugins available
- Rule registry with 2000+ rules

**Installation**:
```bash
pip install semgrep
# or
brew install semgrep
```

**Usage**:
```bash
# Run with community rules
semgrep --config=auto .

# Run specific rule sets
semgrep --config=p/security-audit .
semgrep --config=p/owasp-top-ten .

# CI/CD mode
semgrep ci
```

**Best For**: Custom security patterns, fast feedback, CI/CD integration.

### SonarQube

**Overview**: Comprehensive code quality and security platform.

**Languages**: 25+ languages including Java, C#, JavaScript, Python, Go.

**Key Features**:
- Code quality metrics
- Security hotspot tracking
- Quality gates
- Historical trending
- IDE integration

**Best For**: Organizations needing combined code quality and security analysis.

### CodeQL

**Overview**: Semantic code analysis engine by GitHub.

**Languages**: C/C++, C#, Go, Java, JavaScript, Python, Ruby, TypeScript.

**Key Features**:
- Deep semantic analysis
- Query-based analysis
- GitHub Advanced Security integration
- Custom query creation
- Variant analysis

**Best For**: Deep analysis, finding complex vulnerabilities, GitHub workflows.

### Language-Specific SAST Tools

**Bandit (Python)**
```bash
pip install bandit
bandit -r /path/to/code
```

**ESLint with security plugins (JavaScript)**
```bash
npm install --save-dev eslint-plugin-security
```

**Brakeman (Ruby/Rails)**
```bash
gem install brakeman
brakeman /path/to/rails/app
```

**gosec (Go)**
```bash
go install github.com/securego/gosec/v2/cmd/gosec@latest
gosec ./...
```

## DAST Tools

### OWASP ZAP

**Overview**: Free, open-source web application security scanner.

**Key Features**:
- Active and passive scanning
- API scanning (OpenAPI/Swagger)
- Authenticated scanning
- Automation framework
- CI/CD integration

**Installation**:
```bash
# Docker
docker pull zaproxy/zap-stable

# Standalone
wget https://github.com/zaproxy/zaproxy/releases/download/v2.14.0/ZAP_2.14.0_Linux.tar.gz
```

**Usage**:
```bash
# Quick scan
zap-cli quick-scan http://localhost:8080

# Full scan
zap-cli active-scan http://localhost:8080

# API scan
zap-cli open-url http://localhost:8080/api
zap-cli spider http://localhost:8080/api
zap-cli active-scan http://localhost:8080/api
```

**Best For**: Free, comprehensive scanning, CI/CD integration.

### Burp Suite

**Overview**: Industry-standard web security testing platform.

**Editions**: Community (free), Professional, Enterprise.

**Key Features**:
- Manual testing tools (Repeater, Intruder)
- Scanner (Pro/Enterprise)
- Extensive plugin ecosystem
- Collaboration features
- REST API

**Best For**: Manual security testing, comprehensive scanning (Pro).

### Nuclei

**Overview**: Fast vulnerability scanner based on templates.

**Key Features**:
- Template-based scanning
- 5000+ community templates
- Fast parallel scanning
- Custom template creation
- CI/CD friendly

**Installation**:
```bash
go install github.com/projectdiscovery/nuclei/v2/cmd/nuclei@latest
```

**Usage**:
```bash
# Scan with all templates
nuclei -u http://example.com

# Scan with specific templates
nuclei -u http://example.com -t cves/

# Scan multiple targets
nuclei -l urls.txt -o results.txt
```

**Best For**: Fast scanning, CVE detection, custom vulnerability checks.

### SQLMap

**Overview**: Automated SQL injection testing tool.

**Installation**:
```bash
git clone https://github.com/sqlmapproject/sqlmap.git
cd sqlmap
python sqlmap.py
```

**Usage**:
```bash
# Test URL parameter
sqlmap -u "http://example.com/page?id=1"

# Test POST data
sqlmap -u "http://example.com/login" --data "user=admin&pass=test"

# With authentication
sqlmap -u "http://example.com/api" --cookie "session=abc123"
```

**Best For**: SQL injection detection and exploitation.

## SCA Tools

### Snyk

**Overview**: Developer-first security platform for dependencies.

**Ecosystems**: npm, PyPI, Maven, NuGet, RubyGems, Go modules, Docker.

**Key Features**:
- Fix PRs automatically
- License compliance
- Container scanning
- IaC scanning
- Real-time monitoring

**Installation**:
```bash
npm install -g snyk
snyk auth
```

**Usage**:
```bash
# Test for vulnerabilities
snyk test

# Monitor project
snyk monitor

# Fix vulnerabilities
snyk fix
```

**Best For**: Developer experience, auto-fix PRs, comprehensive coverage.

### Dependabot

**Overview**: GitHub-native dependency update tool.

**Ecosystems**: Most major package managers.

**Key Features**:
- Automatic PR creation
- Security updates
- Version updates
- Native GitHub integration

**Configuration** (.github/dependabot.yml):
```yaml
version: 2
updates:
  - package-ecosystem: "npm"
    directory: "/"
    schedule:
      interval: "weekly"
    open-pull-requests-limit: 10
```

**Best For**: GitHub users, automatic updates, zero configuration.

### OWASP Dependency-Check

**Overview**: Free SCA tool for detecting known vulnerabilities.

**Ecosystems**: Java, .NET, JavaScript, Ruby, Python.

**Installation**:
```bash
# Maven
mvn org.owasp:dependency-check-maven:check

# Gradle
./gradlew dependencyCheckAnalyze

# CLI
wget https://github.com/jeremylong/DependencyCheck/releases/download/v8.0.0/dependency-check-8.0.0-release.zip
```

**Best For**: Free SCA, OWASP ecosystem, Java/.NET projects.

### Trivy

**Overview**: Comprehensive vulnerability scanner for containers and more.

**Scan Targets**: Container images, filesystems, Git repositories, IaC.

**Installation**:
```bash
# macOS
brew install trivy

# Linux
wget https://github.com/aquasecurity/trivy/releases/download/v0.48.0/trivy_0.48.0_Linux-64bit.deb
dpkg -i trivy_0.48.0_Linux-64bit.deb
```

**Usage**:
```bash
# Scan container image
trivy image nginx:latest

# Scan filesystem
trivy fs /path/to/project

# Scan IaC
trivy config ./terraform
```

**Best For**: Container security, multi-purpose scanning, DevOps workflows.

## Secret Detection Tools

### GitLeaks

**Overview**: Git-focused secret scanner.

**Key Features**:
- Git history scanning
- Pre-commit hooks
- Custom rule support
- Fast scanning

**Installation**:
```bash
brew install gitleaks
```

**Usage**:
```bash
# Scan repo
gitleaks detect --source /path/to/repo

# Pre-commit hook
gitleaks protect --staged

# Scan specific commits
gitleaks detect --log-opts="--since='2023-01-01'"
```

### TruffleHog

**Overview**: High-entropy secret scanner.

**Installation**:
```bash
pip install truffleHog
```

**Usage**:
```bash
# Scan repo
truffleHog git https://github.com/user/repo

# Scan filesystem
truffleHog filesystem /path/to/files
```

**Best For**: High-entropy detection, deep history scans.

## Container and Infrastructure Tools

### Trivy

See SCA Tools section above.

### Checkov

**Overview**: IaC security scanner for Terraform, CloudFormation, Kubernetes, etc.

**Installation**:
```bash
pip install checkov
```

**Usage**:
```bash
# Scan Terraform
checkov -d /path/to/terraform

# Scan Kubernetes
checkov -f kubernetes-deployment.yaml

# Scan CloudFormation
checkov -f cloudformation-template.yaml
```

**Best For**: IaC security, Terraform/CloudFormation, policy-as-code.

### kube-bench

**Overview**: Kubernetes CIS benchmark scanner.

**Installation**:
```bash
kubectl apply -f https://raw.githubusercontent.com/aquasecurity/kube-bench/main/job.yaml
```

**Best For**: Kubernetes security, CIS compliance.

### Prowler

**Overview**: AWS security assessment tool.

**Installation**:
```bash
pip install prowler
```

**Usage**:
```bash
# Scan AWS account
prowler aws

# Specific checks
prowler aws --checks check123,check456
```

**Best For**: AWS security, compliance checks (CIS, PCI-DSS, GDPR).

## Tool Selection Guide

### By Project Size

**Small Projects**: Semgrep, npm audit, GitLeaks
**Medium Projects**: SonarQube, OWASP ZAP, Snyk, Trivy
**Large Projects**: CodeQL, Burp Suite Enterprise, Snyk Enterprise

### By Language

**JavaScript/TypeScript**: Semgrep, ESLint-security, Snyk, npm audit
**Python**: Bandit, Semgrep, safety, pip-audit
**Java**: SonarQube, SpotBugs, OWASP Dependency-Check
**Go**: gosec, Semgrep, govulncheck
**Ruby**: Brakeman, bundler-audit
**C/C++**: CodeQL, Cppcheck, Clang Static Analyzer

### By Budget

**Free**: Semgrep, OWASP ZAP, GitLeaks, Trivy, Dependabot
**Paid**: Snyk, SonarQube Enterprise, Burp Suite Pro, Checkmarx

### By CI/CD Integration Priority

**Best CI/CD Integration**: Semgrep, Snyk, OWASP ZAP, GitLeaks, Trivy
**Requires Configuration**: SonarQube, Burp Suite, CodeQL
