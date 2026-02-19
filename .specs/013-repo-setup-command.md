# 013: Repo Setup Command

**Branch**: `feature/repo-setup-command`
**Created**: 2026-02-18

## Summary

Create a `/setup` Claude Code slash command that configures the GitHub repo with best practices for a solo developer. It audits current settings, shows a summary of proposed changes, asks for confirmation, then executes all configuration via `gh` CLI and file creation. Designed to be run once per repo.

## Requirements

- Create `.claude/commands/setup.md` as a Claude Code slash command
- The command must:
  1. Audit current repo state (branch protection, settings, labels, templates, etc.)
  2. Display a clear summary of what will be changed/created
  3. Ask user for confirmation before making any changes
  4. Execute all changes via `gh` API calls and file creation
  5. Report what was done
- Solo-dev-friendly: no required PR reviews, no team-oriented friction
- Idempotent where possible (safe to re-run, skips things already configured)

## Design

The command is a Claude Code slash command (`.claude/commands/setup.md`) with allowed tools for `gh` API calls, file creation, and bash commands. It runs interactively — Claude audits, presents a plan, confirms, then executes.

### What it configures:

#### 1. Branch Protection (update `main`)
- **Required status checks**: keep existing (`Firmware Build & Test`), enable strict (up-to-date before merge)
- **Required PR reviews**: remove (set to 0) — solo dev, no reviewer
- **Required linear history**: enable — enforces squash/rebase (matches squash-only setting)
- **Force pushes**: keep blocked
- **Branch deletion**: keep blocked
- **Enforce admins**: keep false — allows admin bypass in emergencies

#### 2. Repo Settings
- **Topics**: `esp32`, `esp32-s3`, `bbq`, `temperature-controller`, `pid-controller`, `smoker`, `iot`, `arduino`, `lvgl`, `platformio`, `pwm`
- **Vulnerability alerts**: enable

#### 3. Dependabot (`.github/dependabot.yml`)
- GitHub Actions: weekly updates
- PlatformIO/pip: weekly updates (pip ecosystem for PlatformIO)

#### 4. Issue Templates
- `.github/ISSUE_TEMPLATE/bug_report.yml` — YAML form: description, steps to reproduce, expected behavior, environment (firmware version, browser, etc.)
- `.github/ISSUE_TEMPLATE/feature_request.yml` — YAML form: problem, proposed solution, alternatives considered
- `.github/ISSUE_TEMPLATE/config.yml` — disable blank issues, link to discussions if enabled

#### 5. PR Template
- `.github/PULL_REQUEST_TEMPLATE.md` — Summary (bullet points) + Test Plan (checklist) format matching the `/ship` workflow

#### 6. Security Policy
- `SECURITY.md` — responsible disclosure policy, supported versions, contact info

#### 7. Project Labels
Add these project-specific labels (keep existing defaults):
| Label | Color | Description |
|-------|-------|-------------|
| `firmware` | `#1d76db` | ESP32 firmware code |
| `web-ui` | `#0e8a16` | Web UI (PWA) |
| `touchscreen` | `#5319e7` | LVGL touchscreen UI |
| `simulator` | `#fbca04` | Desktop simulator |
| `enclosure` | `#b60205` | 3D printed enclosure/hardware design |
| `hardware` | `#e99695` | Hardware, wiring, carrier board |
| `pid` | `#f9d0c4` | PID controller and tuning |
| `ci-cd` | `#c5def5` | CI/CD pipeline and automation |

### Command Flow

```
/setup

Claude:
1. Runs gh api calls to audit current state
2. Checks which files already exist
3. Presents summary:
   "Here's what I'll set up for your repo:

   Branch Protection:
   ✓ Required CI checks (already set)
   → Remove required PR reviews (currently 1)
   → Enable linear history

   Repo Settings:
   → Add topics: esp32, bbq, ...
   → Enable vulnerability alerts

   Files to Create:
   → .github/dependabot.yml
   → .github/ISSUE_TEMPLATE/bug_report.yml
   → .github/ISSUE_TEMPLATE/feature_request.yml
   → .github/ISSUE_TEMPLATE/config.yml
   → .github/PULL_REQUEST_TEMPLATE.md
   → SECURITY.md

   Labels to Add:
   → firmware, web-ui, touchscreen, simulator, enclosure, hardware, pid, ci-cd

   Proceed?"
4. On confirmation, executes everything
5. Reports results
```

## Files to Modify

| File | Change |
|------|--------|
| `.claude/commands/setup.md` | Create — the slash command definition |
| `.claude/settings.json` | May need to add `Skill(setup)` to permissions if needed |

## Files Created by the Command (when run)

These files are NOT created during implementation — they are created when the `/setup` command is executed by the user:

| File | Purpose |
|------|---------|
| `.github/dependabot.yml` | Dependabot configuration |
| `.github/ISSUE_TEMPLATE/bug_report.yml` | Bug report template |
| `.github/ISSUE_TEMPLATE/feature_request.yml` | Feature request template |
| `.github/ISSUE_TEMPLATE/config.yml` | Issue template config |
| `.github/PULL_REQUEST_TEMPLATE.md` | PR template |
| `SECURITY.md` | Security policy |

## Test Plan

- [x] `.claude/commands/setup.md` exists and is well-formed
- [x] Command includes `allowed-tools` frontmatter with necessary permissions (`Bash(gh *)`, `Write`, `Read`, `Glob`, `AskUserQuestion`)
- [x] Command audits current repo state before proposing changes
- [x] Command shows a clear summary of all proposed changes
- [x] Command asks for user confirmation before executing
- [x] Command handles already-configured items gracefully (skip or note as "already set")
- [x] All `gh` API calls in the command use correct endpoints and payloads
- [x] File templates (issue, PR, security, dependabot) follow GitHub's expected format
- [x] Labels use valid hex colors
- [x] Command is documented as one-time-use
