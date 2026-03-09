// ============================================================
// Pit Claw Temperature Controller Enclosure — INSET SCREEN
// WT32-SC01 Plus drops INTO the box; walls surround the screen
// for protection. Screws still from below through floor.
// Open in OpenSCAD, set 'part' variable, render, export STL
// ============================================================

// === PART SELECTOR ===
// Set to "all", "box", "kickstand", or "test_ring" to render
part = "all";

// === Board Dimensions ===
// WT32-SC01 Plus — measure your board with calipers and adjust
wt32_pcb_w = 60;        // PCB width — short edge (mm)
wt32_pcb_h = 92;        // PCB height — long edge (mm)
wt32_pcb_d = 12.1;      // PCB total thickness including screen (mm) — measured 61/128"
carrier_w = 50;          // Carrier perfboard width (mm)
carrier_h = 70;          // Carrier perfboard height (mm)
carrier_d = 15;          // Tallest component on carrier board (mm)

// === WT32 Mounting Holes ===
// 4 holes: 2 near top, 2 near bottom (measured from actual board)
// Hole-to-hole spans: 52mm wide × 75mm tall
wt32_hole_inset_w = 4;   // All holes: inset from width edges (mm) → 52mm span
wt32_top_inset_h = 10.2; // Top 2 holes: inset from top edge (mm)  ┐ 75mm span
wt32_bot_inset_h = 6.8;  // Bottom 2 holes: inset from bottom edge (mm) ┘ shifted ~1.2mm toward bottom
wt32_post_h = 6.5;       // Height of PCB posts below board (~1/4", with clearance)
wt32_post_od = 6;        // Outer diameter of PCB mounting posts (mm)

// === Enclosure ===
wall = 2.5;               // Wall thickness (mm)
corner_r = 4;             // Corner radius (mm)
tolerance = 0.3;          // Extra clearance per side for PCB fitment (mm)
screw_d = 3.0;            // M3 screw shaft diameter (mm)
screw_head_d = 6;         // M3 screw head diameter (mm)
screw_head_recess = 1.5;  // Depth to recess screw heads in floor (mm)
screen_recess = 1;        // Wall height above screen glass (mm)

// === Panel Mount Holes ===
probe_jack_d = 6.2;       // 2.5mm mono jack mounting hole (mm)
barrel_jack_d = 12.2;     // DC barrel jack mounting hole (mm)
rj45_w = 16;              // RJ45 panel-mount cutout width (mm)
rj45_h = 14;              // RJ45 panel-mount cutout height (mm)

// === Derived Dimensions ===
// Inset variant: PCB drops inside, so box is wider/taller by wall+tolerance on each side
outer_w = wt32_pcb_w + tolerance * 2 + wall * 2;  // 60 + 0.6 + 5 = 65.6mm
outer_h = wt32_pcb_h + tolerance * 2 + wall * 2;  // 92 + 0.6 + 5 = 97.6mm
inner_w = wt32_pcb_w + tolerance * 2;              // 60.6mm — PCB-sized cavity
inner_h = wt32_pcb_h + tolerance * 2;              // 92.6mm — PCB-sized cavity

// Box internal height — stacked zones from floor up:
//   1. Carrier standoffs (5mm)
//   2. Carrier board + components (15mm)
//   3. Clearance above carrier (4mm)
//   4. Connector zone for barrel jack / RJ45 bodies (16mm)
carrier_standoff_h = 5;
carrier_clearance = 4;
connector_zone = 16;
inner_d = carrier_standoff_h + carrier_d + carrier_clearance + connector_zone;

// Total box depth: floor + internal cavity + PCB thickness + screen recess
// Z=0 floor bottom, Z=wall floor top, Z=wall+inner_d PCB bottom/ledge,
// Z=wall+inner_d+wt32_pcb_d screen glass, Z=box_d wall top (1mm above glass)
box_d = wall + inner_d + wt32_pcb_d + screen_recess;  // 2.5 + 40 + 12.1 + 1 = 55.6mm

// Standoff dimensions — inset from PCB bottom to leave room for WT32 PCB posts
standoff_d = screw_d + 2;  // 5mm — slim to fit within corner walls
standoff_h = inner_d - wt32_post_h + 0.5 + 25.4*5/16;  // +5/16" per test ring print fit

// Panel-mount hole height — all ports centered at same Z, slightly above midpoint
port_z = wall + inner_d / 2 + 1.5;  // ~24mm — just above center of box

// WT32 mounting positions (relative to enclosure center XY)
wt32_mount = [
    [ (wt32_pcb_w/2 - wt32_hole_inset_w),  (wt32_pcb_h/2 - wt32_top_inset_h)],    // top-right (corner)
    [-(wt32_pcb_w/2 - wt32_hole_inset_w),  (wt32_pcb_h/2 - wt32_top_inset_h)],    // top-left (corner)
    [ (wt32_pcb_w/2 - wt32_hole_inset_w), -(wt32_pcb_h/2 - wt32_bot_inset_h)],    // bottom-right (inset)
    [-(wt32_pcb_w/2 - wt32_hole_inset_w), -(wt32_pcb_h/2 - wt32_bot_inset_h)]     // bottom-left (inset)
];

// Carrier board mounting positions (4 corners of 50x70mm board)
carrier_inset = 3;
carrier_mount = [
    [ (carrier_w/2 - carrier_inset),  (carrier_h/2 - carrier_inset)],
    [-(carrier_w/2 - carrier_inset),  (carrier_h/2 - carrier_inset)],
    [ (carrier_w/2 - carrier_inset), -(carrier_h/2 - carrier_inset)],
    [-(carrier_w/2 - carrier_inset), -(carrier_h/2 - carrier_inset)]
];

// Kickstand dimensions (recalculated for new outer_h)
ks_w = 25;
ks_h = outer_h * 0.65;
ks_thick = 3;
ks_hinge_d = 5;
ks_hinge_slot_w = ks_w + 1;

// $fn for curves
$fn = 40;

// ============================================================
// Utility Modules
// ============================================================

module rounded_rect_2d(w, h, r) {
    offset(r = r)
        square([w - 2*r, h - 2*r], center = true);
}

module rounded_box(w, h, d, r) {
    linear_extrude(height = d)
        rounded_rect_2d(w, h, r);
}

module vent_slot(length, width, depth) {
    translate([-length/2, -width/2, 0])
        cube([length, width, depth + 1]);
}

// ============================================================
// Part 1: Inset Box
// ============================================================

module inset_box() {
    difference() {
        union() {
            // Outer walls with floor — full height including PCB zone
            difference() {
                rounded_box(outer_w, outer_h, box_d, corner_r);
                translate([0, 0, wall])
                    rounded_box(inner_w, inner_h, box_d, max(corner_r - wall, 1));
            }

            // WT32 mounting standoffs — same as flush version
            // Start 0.01 below floor top to avoid coincident faces
            for (pos = wt32_mount) {
                translate([pos[0], pos[1], wall - 0.01])
                    cylinder(d = standoff_d + 0.02, h = standoff_h + 0.01);
            }

            // Buttress ribs — connect each standoff to nearest side wall
            for (pos = wt32_mount) {
                sx = pos[0] > 0 ? 1 : -1;
                bx = sx * (inner_w/2 + 0.5);  // 0.5mm into wall for solid union
                translate([min(pos[0], bx), pos[1] - standoff_d/2, wall - 0.01])
                    cube([abs(bx - pos[0]), standoff_d, standoff_h + 0.01]);
            }

            // Carrier board standoffs (shorter, near floor)
            for (pos = carrier_mount) {
                translate([pos[0], pos[1], wall - 0.01])
                    cylinder(d = screw_d + 2.5, h = carrier_standoff_h + 0.01);
            }
        }

        // --- WT32 through-holes (bottom to top) ---
        // M3 screws from below, through floor and standoff, into PCB posts
        for (pos = wt32_mount) {
            // Screw shaft — all the way through
            translate([pos[0], pos[1], -0.5])
                cylinder(d = screw_d, h = box_d + 1);
            // Counterbore for screw head flush with bottom
            translate([pos[0], pos[1], -0.5])
                cylinder(d = screw_head_d, h = screw_head_recess + 0.5);
        }

        // --- Carrier board pilot holes ---
        for (pos = carrier_mount) {
            translate([pos[0], pos[1], wall - 0.5])
                cylinder(d = screw_d * 0.8, h = carrier_standoff_h + 1);
        }

        // --- Bottom wall: 3x probe jack holes (evenly spaced) ---
        // Wider inner cavity → recalculate spacing
        bottom_y = -outer_h/2;
        probe_spacing = inner_w / 4;   // ~15.25mm — spread across wider wall
        for (i = [0:2]) {
            translate([(i - 1) * probe_spacing, bottom_y, port_z])
                rotate([90, 0, 0])
                    cylinder(d = probe_jack_d, h = wall * 2 + 1, center = true);
        }

        // --- Right wall: DC barrel jack + RJ45 Ethernet (same side) ---
        barrel_y = -15;
        rj45_y = 15;
        translate([outer_w/2, barrel_y, port_z])
            rotate([0, 90, 0])
                cylinder(d = barrel_jack_d, h = wall * 2 + 1, center = true);
        translate([outer_w/2, rj45_y, port_z])
            rotate([0, 90, 0])
                linear_extrude(height = wall * 2 + 1, center = true)
                    square([rj45_h, rj45_w], center = true);

        // --- Floor ventilation slots ---
        vent_count = 5;
        vent_len = 20;
        vent_w = 2;
        vent_spacing = 5;
        vent_start_y = outer_h/2 - 20;

        for (i = [0 : vent_count - 1]) {
            translate([-inner_w/4, vent_start_y - i * vent_spacing, -0.5])
                vent_slot(vent_len, vent_w, wall);
            translate([inner_w/4, vent_start_y - i * vent_spacing, -0.5])
                vent_slot(vent_len, vent_w, wall);
        }

        // No wall pockets needed — inner cavity (60.6mm) gives 0.3mm clearance
        // from PCB edge to wall on each side. Posts at X=±26 are well clear
        // of walls at X=±30.3.

        // --- Kickstand hinge slot on floor ---
        translate([0, -outer_h/2 + ks_h * 0.4 + 10, -0.5])
            linear_extrude(height = wall + 1)
                hull() {
                    translate([-ks_hinge_slot_w/2, 0]) circle(d = ks_hinge_d + 1);
                    translate([ ks_hinge_slot_w/2, 0]) circle(d = ks_hinge_d + 1);
                }
    }
}

// ============================================================
// Part 2: Kickstand
// ============================================================

module kickstand() {
    hinge_r = ks_hinge_d / 2;

    difference() {
        union() {
            translate([-ks_w/2, 0, 0])
                cube([ks_w, ks_h, ks_thick]);

            translate([-ks_w/2 + hinge_r, 0, ks_thick/2])
                rotate([0, 90, 0])
                    cylinder(r = hinge_r, h = ks_w - ks_hinge_d);
        }

        translate([-ks_w/2 - 0.5, -hinge_r, ks_thick/2 - 0.4])
            cube([ks_w + 1, 1.5, 0.8]);
    }

    translate([-ks_w/2, ks_h - 1, 0])
        cube([ks_w, 1, ks_thick + 1.5]);
}

// ============================================================
// Part 3: Test Ring (top slice — standoffs + PCB pocket for fitment)
// ============================================================

module test_ring() {
    ring_h = 15;  // Slim ring: ~4mm standoff + PCB pocket + screen lip
    translate([0, 0, -(box_d - ring_h)])
        intersection() {
            inset_box();
            translate([-outer_w, -outer_h, box_d - ring_h])
                cube([outer_w * 2, outer_h * 2, ring_h]);
        }
}

// ============================================================
// Part Selector
// ============================================================

if (part == "box") {
    inset_box();
}
else if (part == "kickstand") {
    kickstand();
}
else if (part == "test_ring") {
    test_ring();
}
else {
    // "all" — arranged for visualization
    inset_box();

    translate([outer_w + 15, -outer_h/2 + 10, 0])
        kickstand();
}
