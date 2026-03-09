# 019: Inset Screen Enclosure Variant

**Branch**: `feature/inset-enclosure`
**Created**: 2026-03-09
**Status**: Implemented

## Summary

Alternative enclosure design where the WT32-SC01 Plus drops INTO the box from the top, with walls surrounding the screen for protection. Unlike the flush-mount design in `bbq-case.scad`, this variant has the PCB sitting inside the cavity with the screen recessed 1mm below the wall top. Screws from below through the floor hold the PCB to internal standoffs. Includes a test ring part for quick fitment checks without printing the full box.

Standoff height was corrected +5/16" based on test ring print — the original posts were too low. Test ring slimmed from 25mm to 15mm for faster print iteration.

## Requirements

- PCB drops into box from the top; walls surround the screen on all sides
- Screen recessed 1mm below wall top for protection
- M3 screws from below through floor and standoffs into PCB mounting holes
- Counterbored screw heads flush with bottom of box
- Tighter PCB tolerance (0.3mm per side vs 0.5mm in flush version)
- Same panel-mount holes as flush version: 3x probe jacks, DC barrel jack, RJ45 cutout
- Same carrier board standoffs and ventilation slots
- Kickstand with hinge slot compatible with flush version
- Test ring part for quick PCB fitment printing (~15mm tall, captures standoff tops + PCB pocket + screen lip)
- Standoff height adjusted +5/16" per physical test ring print

## Design

### Key Differences from Flush Version (bbq-case.scad)

| Parameter | Flush (`bbq-case.scad`) | Inset (`bbq-case-inset.scad`) |
|-----------|------------------------|-------------------------------|
| PCB mounting | Bezel lip frames screen from front | PCB drops into box from top |
| Tolerance | 0.5mm per side | 0.3mm per side |
| Wall relationship | Front bezel + rear shell (2 parts) | Single box, open top |
| Screen protection | Bezel lip only | Full wall surround, 1mm recess |
| Parts | 3 (front bezel, rear shell, kickstand) | 3 (box, kickstand, test ring) |

### Parametric Variables (differences from flush version)

```scad
tolerance = 0.3;          // Tighter than flush version (0.5mm)
screen_recess = 1;        // Wall height above screen glass (mm)
rj45_w = 16;              // RJ45 panel-mount cutout (replaces USB-C cutout)
rj45_h = 14;

// Standoff height corrected per test ring fit check
standoff_h = inner_d - wt32_post_h + 0.5 + 25.4*5/16;  // +5/16"
```

### Test Ring

Slices the top 15mm of the box — just enough to capture the standoff tops, full PCB pocket, and screen lip. Print this first to verify PCB fitment before committing to the full box print.

## Files to Modify

| File | Change |
|------|--------|
| `enclosure/bbq-case-inset.scad` | New file — inset screen enclosure variant |
| `enclosure/stl/bbq-case-inset-box.stl` | Exported STL for the inset box |
| `enclosure/stl/bbq-case-inset-kickstand.stl` | Exported STL for the kickstand |
| `enclosure/stl/bbq-case-inset-test-ring.stl` | Exported STL for the test ring |

## Test Plan

- [x] bbq-case-inset.scad renders all three parts without errors in OpenSCAD
- [x] Standoff height includes +5/16" correction from test ring print
- [x] Test ring height reduced to 15mm for faster print iteration
- [x] STLs exported for box, kickstand, and test ring
