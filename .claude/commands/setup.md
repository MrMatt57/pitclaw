---
allowed-tools: Bash(gh *), Bash(git *), Bash(ls *), Bash(mkdir *), Read, Write, Glob, Grep, AskUserQuestion
---

# /setup — One-time GitHub repo best practices setup

You are setting up a GitHub repository with best practices for a solo developer. This command is designed to run once. Follow these steps precisely.

## Step 1: Detect Repo

Run `gh repo view --json nameWithOwner,defaultBranch --jq '{repo: .nameWithOwner, branch: .defaultBranch}'` to identify the repo.

If this fails, tell the user they need to be in a git repo with a GitHub remote.

## Step 2: Audit Current State

Run these commands to understand what's already configured:

```bash
# Repo settings
gh api repos/{owner}/{repo} --jq '{
  topics: .topics,
  has_issues: .has_issues,
  has_wiki: .has_wiki,
  has_discussions: .has_discussions,
  allow_merge_commit: .allow_merge_commit,
  allow_squash_merge: .allow_squash_merge,
  allow_rebase_merge: .allow_rebase_merge,
  delete_branch_on_merge: .delete_branch_on_merge,
  allow_auto_merge: .allow_auto_merge
}'

# Branch protection
gh api repos/{owner}/{repo}/branches/main/protection 2>&1

# Existing labels
gh label list --json name --jq '.[].name'

# Check for existing files
ls .github/dependabot.yml 2>/dev/null
ls .github/ISSUE_TEMPLATE/ 2>/dev/null
ls .github/PULL_REQUEST_TEMPLATE.md 2>/dev/null
ls SECURITY.md 2>/dev/null
```

## Step 3: Present Summary

Based on the audit, present a clear summary to the user. Group changes into categories and mark items that are already configured:

```
Here's what I'll set up for {repo}:

**Branch Protection (main):**
  {✓ already | → will set} Required CI status checks (strict: up-to-date)
  {✓ already | → will set} Remove required PR reviews (solo dev — no reviewer needed)
  {✓ already | → will set} Require linear history (clean squash merges)
  {✓ already | → will set} Block force pushes
  {✓ already | → will set} Block branch deletion

**Repo Settings:**
  → Add topics: esp32, esp32-s3, bbq, temperature-controller, pid-controller, smoker, iot, arduino, lvgl, platformio, pwm
  → Enable vulnerability alerts

**Files to Create:**
  {✓ exists | → create} .github/dependabot.yml
  {✓ exists | → create} .github/ISSUE_TEMPLATE/bug_report.yml
  {✓ exists | → create} .github/ISSUE_TEMPLATE/feature_request.yml
  {✓ exists | → create} .github/ISSUE_TEMPLATE/config.yml
  {✓ exists | → create} .github/PULL_REQUEST_TEMPLATE.md
  {✓ exists | → create} SECURITY.md

**Labels to Add:**
  {list only labels that don't already exist}
  firmware, web-ui, touchscreen, simulator, enclosure, hardware, pid, ci-cd
```

Use AskUserQuestion to confirm:
> Ready to apply these changes?

If the user declines, stop.

## Step 4: Apply Branch Protection

Update the branch protection rule on `main`. This is a single PUT that replaces the full protection config — include ALL settings (both existing and new):

```bash
gh api repos/{owner}/{repo}/branches/main/protection \
  --method PUT \
  --input - <<'EOF'
{
  "required_status_checks": {
    "strict": true,
    "contexts": ["Firmware Build & Test"]
  },
  "enforce_admins": false,
  "required_pull_request_reviews": null,
  "restrictions": null,
  "required_linear_history": true,
  "allow_force_pushes": false,
  "allow_deletions": false
}
EOF
```

**Important:** Setting `required_pull_request_reviews` to `null` removes the PR review requirement entirely. This is intentional for a solo dev workflow.

## Step 5: Apply Repo Settings

```bash
# Set topics
gh api repos/{owner}/{repo}/topics --method PUT --input - <<'EOF'
{"names":["esp32","esp32-s3","bbq","temperature-controller","pid-controller","smoker","iot","arduino","lvgl","platformio","pwm"]}
EOF

# Enable vulnerability alerts
gh api repos/{owner}/{repo}/vulnerability-alerts --method PUT
```

## Step 6: Create Dependabot Config

Only create if `.github/dependabot.yml` doesn't exist.

Write `.github/dependabot.yml`:

```yaml
version: 2
updates:
  - package-ecosystem: "github-actions"
    directory: "/"
    schedule:
      interval: "weekly"
    commit-message:
      prefix: "ci"

  - package-ecosystem: "pip"
    directory: "/firmware"
    schedule:
      interval: "weekly"
    commit-message:
      prefix: "deps"
```

## Step 7: Create Issue Templates

Only create files that don't already exist.

Write `.github/ISSUE_TEMPLATE/bug_report.yml`:

```yaml
name: Bug Report
description: Report a bug or unexpected behavior
labels: ["bug"]
body:
  - type: markdown
    attributes:
      value: Thanks for reporting a bug! Please fill out the details below.
  - type: textarea
    id: description
    attributes:
      label: Description
      description: A clear description of the bug
    validations:
      required: true
  - type: textarea
    id: steps
    attributes:
      label: Steps to Reproduce
      description: How can we reproduce this?
      placeholder: |
        1. Go to '...'
        2. Click on '...'
        3. See error
    validations:
      required: true
  - type: textarea
    id: expected
    attributes:
      label: Expected Behavior
      description: What did you expect to happen?
    validations:
      required: true
  - type: dropdown
    id: component
    attributes:
      label: Component
      options:
        - Firmware
        - Web UI
        - Touchscreen UI
        - Simulator
        - Enclosure
        - Other
    validations:
      required: true
  - type: input
    id: version
    attributes:
      label: Firmware Version
      description: Check Settings screen or web UI footer
      placeholder: e.g. 2026.2.0
  - type: textarea
    id: context
    attributes:
      label: Additional Context
      description: Logs, screenshots, browser/OS info, etc.
```

Write `.github/ISSUE_TEMPLATE/feature_request.yml`:

```yaml
name: Feature Request
description: Suggest a new feature or improvement
labels: ["enhancement"]
body:
  - type: textarea
    id: problem
    attributes:
      label: Problem
      description: What problem does this solve? Or what would be improved?
    validations:
      required: true
  - type: textarea
    id: solution
    attributes:
      label: Proposed Solution
      description: How should it work?
    validations:
      required: true
  - type: textarea
    id: alternatives
    attributes:
      label: Alternatives Considered
      description: Any other approaches you've thought about?
  - type: dropdown
    id: component
    attributes:
      label: Component
      options:
        - Firmware
        - Web UI
        - Touchscreen UI
        - Simulator
        - Enclosure
        - Other
    validations:
      required: true
```

Write `.github/ISSUE_TEMPLATE/config.yml`:

```yaml
blank_issues_enabled: false
```

## Step 8: Create PR Template

Only create if `.github/PULL_REQUEST_TEMPLATE.md` doesn't exist.

Write `.github/PULL_REQUEST_TEMPLATE.md`:

```markdown
## Summary

-

## Test plan

- [ ]
```

## Step 9: Create Security Policy

Only create if `SECURITY.md` doesn't exist.

Write `SECURITY.md`:

```markdown
# Security Policy

## Supported Versions

| Version | Supported |
|---------|-----------|
| Latest release | Yes |
| Older releases | No |

## Reporting a Vulnerability

If you discover a security vulnerability, please report it responsibly:

1. **Do not** open a public issue
2. Email the maintainer or use [GitHub's private vulnerability reporting](https://github.com/{owner}/{repo}/security/advisories/new)
3. Include steps to reproduce and potential impact
4. Allow reasonable time for a fix before public disclosure

## Scope

This is a hobbyist IoT device for BBQ temperature control. It runs on a local Wi-Fi network and does not handle sensitive personal data. Security concerns are primarily:

- Web UI served over HTTP (local network only)
- WebSocket protocol (unauthenticated, local network only)
- OTA firmware updates (local network only)
```

Replace `{owner}/{repo}` with the actual repo identifier detected in Step 1.

## Step 10: Add Project Labels

For each label below, only create it if it doesn't already exist:

```bash
gh label create "firmware" --color "1d76db" --description "ESP32 firmware code" --force
gh label create "web-ui" --color "0e8a16" --description "Web UI (PWA)" --force
gh label create "touchscreen" --color "5319e7" --description "LVGL touchscreen UI" --force
gh label create "simulator" --color "fbca04" --description "Desktop simulator" --force
gh label create "enclosure" --color "b60205" --description "3D printed enclosure and hardware design" --force
gh label create "hardware" --color "e99695" --description "Hardware, wiring, carrier board" --force
gh label create "pid" --color "f9d0c4" --description "PID controller and tuning" --force
gh label create "ci-cd" --color "c5def5" --description "CI/CD pipeline and automation" --force
```

The `--force` flag makes this idempotent — it updates the label if it already exists.

## Step 11: Report

Print a summary of everything that was configured:

```
Repo setup complete!

  Branch protection: CI required, linear history, no PR reviews needed
  Topics: esp32, bbq, pid-controller, ...
  Vulnerability alerts: enabled
  Dependabot: GitHub Actions + PlatformIO deps (weekly)
  Issue templates: bug report + feature request
  PR template: Summary + Test plan format
  Security policy: SECURITY.md
  Labels: firmware, web-ui, touchscreen, simulator, enclosure, hardware, pid, ci-cd

Note: This command is designed to run once. Re-running is safe (idempotent)
but shouldn't be necessary.
```
