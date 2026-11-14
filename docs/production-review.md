# Product readiness review — 13 November 2026

**Disposition: NOT READY TO SHIP.** Review covers the current firmware/control path, display and web integration, persistence, network/update endpoints, build/release workflows, hardware instructions and OpenSCAD source. Software compilation and tests cannot qualify the physical power path, fan, damper or enclosure. No physical hardware was available. This is a product engineering review, not certification or an exhaustive security audit.

## Changes in this PR

- Preserve the existing hardware and incorporate the existing working-tree probe conversion, bounded ADC access, pit-fault holdoff, independent PWM allocation, servo mapping and serial commissioning work.
- Validate incoming setpoints (100–500°F), alarm ranges and fan mode; limit command size; preserve omitted alarm targets; remove unnecessary writes past received WebSocket payloads.
- Stop fan kick-start on a zero/nonfinite speed request.
- Keep controller measurements, alarms, persisted samples and protocol in Fahrenheit. Convert touchscreen readouts/graphs at presentation; changing display units clears that display's graph. Editors remain explicitly Fahrenheit.
- Expose only named web assets from LittleFS. Configuration and session files are not static website assets.
- Pin platform and direct library dependencies to reviewed versions. CI builds real firmware, bench firmware, SDL simulator and LittleFS; exercises native regressions and HTTP/WebSocket integration; packages binaries with commit/version/checksums.
- Tags run the same validation workflow and create a **draft** release only after success. Branch pushes do not publish releases. Physical acceptance remains required before publishing.

## Open findings and required acceptance

| Priority | Evidence / failure | Required completion |
|---|---|---|
| Blocker | Existing direct GPIO→IRLZ44N fan drive has no specified on-resistance at 3.3V; 5015 names a size, not an electrical control specification. | Identify exact fan and MOSFET supplier; qualify drive voltage, PWM compatibility, startup, switching loss and heat across supply/load/temperature. Keeping hardware unchanged cannot establish a datasheet guarantee. Any added gate driver/protection requires an explicit hardware revision. |
| Blocker | `main.cpp` automatically enables PID after valid pit input; there is no explicit Start/Stop control or latched overtemperature shutdown. A hobby servo is not spring-return closed on power loss. | Define boot/recovery/stop/update states and temperature limits; implement independent output inhibition and test resets, power loss, sensor faults and watchdog recovery. Characterize residual natural airflow and mechanical power-loss position. |
| Blocker | `ota_manager.cpp` registers unauthenticated ElegantOTA and does not inhibit actuators during updates. Web control also lacks device authorization. | Require device provisioning/authentication, a local update authorization state, outputs off through update/failure/reboot, and validated firmware compatibility/recovery. Test unauthorized requests and interrupted updates on hardware. |
| Blocker | Async WebSocket callbacks call main/UI/config/session functions from the network task (`web_server.cpp`, `main.cpp`). | Queue bounded validated commands; mutate LVGL, controller and storage only from their owning task. Stress simultaneous clients, resets and downloads while controlling. |
| High | `cook_session.cpp::flush` writes the start-time header only if append-open fails; append normally creates a missing file. Recovery always consumes a header. Write results are ignored. CSV/JSON use the 600-sample RAM window only. `DataPoint` is not explicitly packed despite its 13-byte comment. | Version the on-disk format, explicitly write/check header and records, recover truncated writes, stream full flash history, and test full/failed flash and power cuts. Preserve/export legacy files before migration. |
| High | `config_manager.cpp` formats on mount failure and truncates config before saving. Most setting callbacks never call `save`; JSON fields are not range checked. | Atomic validated saves with recovery, schema migration, clear persistence errors, and no automatic destructive format. Power-cut-test each save stage. |
| High | Latest main integrates `ui_setup_wizard`; Wi-Fi boot attempts still block the loop for up to 45 seconds. Both captive portal and main HTTP server target port 80. | Validate the existing onboarding state machine and hardware commissioning gate; test portal/server ownership, factory reset and operation without Wi-Fi. |
| High | No production time-sync call found; `time()` drives web/session timestamps. Pushover configuration exists without notification delivery integration. Predictor is not wired into production main. | Establish clock validity/offline elapsed time, connect promised alarm delivery and prediction, or remove unsupported product claims. Confirm timestamps after reconnect/reboot. |
| High | Web shell loads uPlot from CDN; service-worker installation depends on remote assets. Plain LAN HTTP does not provide a general secure context for service workers. | Vendor licensed chart assets and test a fresh browser with no internet. Define supported install/offline behavior for the actual deployment and browsers. |
| High | `bbq-case.scad` in the working tree computes about 65.6×97.6×55.6mm; older instructions describe 40mm depth and M3×30 screws. The original local fan README describes rotary parts absent from `bbq-fan-assembly.scad`; current main retains the two-part fan design and a different snap-fit controller case. butterfly plate uses `%` preview-only geometry. | Freeze one measured CAD revision, provide a printable damper selector and complete source-derived STLs, verify fastener stack/clearance, print and fit every part. Existing uncommitted mechanical work remains separate from this firmware PR. |
| High | Generic regulator, fan, buzzer and display descriptions; no complete rated RJ45 harness contract, thermal test, environmental enclosure rating or production BOM. | Freeze exact part numbers/revisions, measured harness pinout and ratings, fuse/protection decision, strain relief, thermal envelope and intended indoor/outdoor use. Approve a golden assembly and fixture before manufacturing. |
| Medium | Native tests compile out timing/hardware branches; native PID compute returns zero. Simulator uses its own thermal control model. | Run target integration tests and real PID response/long-pulse/wraparound/soak tests; measure heap under multiple clients and long exports. Initial controller static RAM usage is ~77%, so runtime memory margin matters. |

## Simplified assembly with unchanged hardware

Use a single **assembly traveler**, one cable-label convention and three independently checked subassemblies: display harness, repeated sensor channels, power/output section. Keep the existing perfboard; no PCB redesign is assumed. Pre-cut and label leads using one verified pilot build, photograph both sides of the accepted board, continuity-check before inserting socketed modules, and install prewired panel connectors last. The short [build guide](build-guide.md) is the starting sequence. Existing historical drawings are not a production wiring authority.

Record for each unit: serial number, component revisions, firmware commit, rail measurements, resistor-fixture results on all channels, fan start/current/temperature, calibrated servo endpoints, combined-load behavior, fault shutdown, network/USB power arrangement and operator/date. Reject units with brownout, moving sensor readings under motor load, binding damper, failed fan restart or missing fault inhibition. Numerical electrical/thermal limits must be approved from the exact purchased parts before this traveler can be a production pass/fail specification.

## Release process

1. Merge only after the `Tests and all firmware images` job passes for the final PR commit. Configure repository branch protection to require it; workflow files alone do not enforce branch protection.
2. Complete the open software findings and hardware acceptance above. Retain the test record for the intended board/case/harness revision.
3. Push a reviewed `vMAJOR.MINOR.PATCH` tag. Release CI injects that version, repeats validation, and packages firmware, bench, filesystem, bootloader and partition images plus a manifest/checksums. These are individual images, not a merged factory flashing bundle.
4. Review the resulting draft and physical acceptance record before publishing. Use PlatformIO from the matching commit for initial USB provisioning. Filesystem flashing erases user data; firmware-only OTA preserves it. No production readiness is implied by draft generation.

## Sources checked

The [Infineon IRLZ44N datasheet](https://www.infineon.com/assets/row/public/documents/24/49/infineon-irlz44n-datasheet-en.pdf) does not specify maximum on-resistance at 3.3V gate drive. The [TI ADS1115 datasheet](https://www.ti.com/lit/ds/symlink/ads1115.pdf) defines the gain-one 125µV/count scaling and supply/input limits. [TowerPro MG90S specifications](https://towerpro.com.tw/product/mg90s-3/) do not establish the behavior of an unidentified clone. These sources support component checks; they do not validate this assembly.

## Verification record

Initial local checkout baseline (before integration with newer main): controller and bench firmware compiled; all 132 existing native tests passed. Final PR verification is recorded in the PR and CI run, including new protocol/fan regressions and shared simulator smoke tests. The integrated branch passed 137 native tests locally; clean GitHub builds passed controller, bench, simulator and LittleFS compilation. The initial simulator smoke fixture required correction for the newly integrated first-boot wizard. Physical tests, browser visual QA, flash power-cut tests, unattended runtime endurance, regulatory testing and secure update qualification are **not completed**. Product-ready status remains blocked by the findings above.
