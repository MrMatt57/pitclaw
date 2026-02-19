# 017: Update README Screenshots

**Branch**: `feature/readme-screenshots`
**Created**: 2026-02-18

## Summary

Update the README to showcase both the hardware device and the web UI. Add a second screenshot for the embedded touchscreen device and update the existing web UI screenshot reference.

## Requirements

- Add a hardware/device screenshot placeholder in `docs/media/` (filename: `pitclaw-device.png`)
- Keep the existing web UI screenshot (`docs/media/pitclaw.png`) — user will replace the file with an updated version
- Update `README.md` to display both screenshots prominently near the top

## Design

Update the hero section of the README (after the badges, before Features) to show two screenshots side by side or stacked:
1. Hardware device photo (`pitclaw-device.png`) — placeholder file for now
2. Web UI screenshot (`pitclaw.png`) — existing file, user will update the actual image

Use a simple markdown table or inline images to display both.

## Files to Modify

| File | Change |
|------|--------|
| `README.md` | Update screenshot section to show both device and web UI images |

## Test Plan

- [x] README renders correctly on GitHub with both image references
- [x] Placeholder filename `pitclaw-device.png` is referenced correctly
- [x] Existing `pitclaw.png` reference is preserved
