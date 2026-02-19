# 003: Spec Test Plan Verification & PR Alignment

**Branch**: `feature/spec-test-verification`
**Created**: 2026-02-18

## Summary

Update the `/feature` and `/ship` lifecycle commands so that test plan items in the spec are checked off during implementation, and the PR body pulls its test plan directly from the spec rather than generating a new one. This ensures the spec remains the single source of truth throughout the lifecycle.

## Requirements

- During `/feature` Step 8 (Implement), after verifying each test plan item, update the spec file to check off the item (`- [ ]` → `- [x]`)
- During `/ship` Step 4 (Create PR), read the spec's test plan and summary and use them directly in the PR body instead of writing new ones
- The PR summary should come from the spec's Summary section
- The PR test plan should be the spec's Test Plan section (already checked off from implementation)
- If a test plan item genuinely wasn't verified, it should remain unchecked

## Design

### `feature.md` Changes (Step 8)

Add explicit instructions to update the spec file's test plan checkboxes as each item is verified. After running builds/tests and confirming requirements, edit the spec file (in the worktree) to mark each passing item as `[x]`. This makes the spec a living document that reflects verification status.

### `ship.md` Changes (Step 4)

Replace the hardcoded PR body template with instructions to:
1. Read the spec file matching the branch name
2. Extract the Summary section for the PR summary
3. Extract the Test Plan section (with its checked/unchecked checkboxes) for the PR test plan
4. Use these directly in the PR body

## Files to Modify

| File | Change |
|------|--------|
| `.claude/commands/feature.md` | Update Step 8 to check off test plan items in the spec file after verification |
| `.claude/commands/ship.md` | Update Step 4 to pull summary and test plan from the spec file |

## Test Plan

- [x] `feature.md` Step 8 includes instructions to check off test plan items in the spec
- [x] `ship.md` Step 4 includes instructions to read the spec's Summary and Test Plan
- [x] `ship.md` Step 4 PR body template uses spec content instead of hardcoded example
- [x] Both files have valid markdown and no syntax errors
