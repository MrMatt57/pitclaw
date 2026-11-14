# Pit Claw: shortest build guide

**Status: bench prototype; release qualification is incomplete. Keep the existing WT32-SC01 Plus, ADS1115, MP1584EN, IRLZ44N, 5015 fan, MG90S, probes and jacks.** The [review](production-review.md) lists shipping blockers. No replacement board or new controller is required by this guide.

1. **Prepare three subassemblies.** Label the display harness by pin number; build three identical probe channels on the existing 50×70mm perfboard; assemble the existing power/fan section. Pre-wire panel jacks on the bench. Mark both ends of every lead. Keep motor returns separate from probe returns until the power ground. Secure and insulate all solder joints.
2. **Check power first.** Disconnect display, ADC, fan and servo. Verify barrel polarity and absence of shorts. Power the buck from 12V, adjust to **5.00V**, switch off, then connect the display and servo to the 5V distribution point. ADC and probe pull-ups use **3.3V**, with common ground. Verify rails again under combined load.
3. **Wire the display.** Confirm pin 1 on the actual board and continuity-map the cable; colors are not pin numbers. Use the table below. Source ADC 3.3V from the verified DEBUG pin 2/3V3 point; it is absent from the eight-pin connector.
4. **Wire probes.** For each channel: **3.3V → 10kΩ → node → jack tip; jack sleeve → GND**. Connect node to A0/A1/A2 and a 100nF capacitor from node to GND. ADC: VDD=3.3V, ADDR=GND, SDA=GPIO10, SCL=GPIO11; verify breakout pull-ups/bypass. Identify switched jack contacts with a plug and meter; leave switched contacts unused.
5. **Qualify outputs before connecting the smoker.** Existing fan circuit: +12V to fan+, fan− to IRLZ44N drain, source to GND, GPIO12 to gate with the existing 10kΩ gate/source pull-down. **This direct 3.3V drive is unqualified; do not approve production from a successful spin test.** Verify the exact fan's switching specification, startup, current and MOSFET temperature. Servo: 5V/GND/GPIO13; remove linkage and calibrate closed/open pulses before assembly. GPIO14 drives only a verified suitable passive piezo. See the review for electrical blockers.
6. **Flash and bench-test.** Use the commands below. During USB flashing isolate external display 5V and unpowered output signal paths; disconnect USB before restoring external power. For powered motor tests use a 3.3V-logic UART adapter, **RX←DEBUG3, TX→DEBUG4, GND→DEBUG7; adapter VCC disconnected**. `status`, `fan 100`, `servo 1500`, `beep`, `off` exercise the bench firmware. Check all probe channels with a 10kΩ resistor across tip/sleeve: approximately **1.650V / 13200 counts / 184.2°F** at a measured 3.300V rail. Open must report OPEN; short must report SHORT.
7. **Fit and accept.** Dry-fit the case/test ring against the actual board and jacks. Determine screw length from the printed stack; older M3×30 instructions do not match every case revision. Fasten the carrier, route cables clear of standoffs, strain-relieve external leads, then close the display. Test simultaneous loads, pit unplug/short, ADC loss, reboot, Wi-Fi loss and a supervised cook. Record temperatures, rail voltages, fault response and firmware commit. **Do not ship until the review's release gates pass.**

| WT32 extension pin | Connect to |
|---|---|
| 1 | Buck 5V |
| 2 | Common GND |
| 3 / GPIO10 | ADS1115 SDA |
| 4 / GPIO11 | ADS1115 SCL |
| 5 / GPIO12 | Existing fan control input |
| 6 / GPIO13 | Servo signal |
| 7 / GPIO14 | Qualified piezo drive |
| 8 / GPIO21 | Unconnected |

Run from `firmware/` with PlatformIO installed:

```powershell
pio test -e native
pio run -e bench --target upload
pio device monitor -e bench --baud 115200 --filter send_on_enter
# After bench acceptance, disconnect power and restore the USB flashing arrangement:
pio run -e wt32_sc01_plus --target upload
pio run -e wt32_sc01_plus --target uploadfs
```

`uploadfs` is for initial provisioning: it overwrites stored settings and cook data. Normal firmware-only updates preserve that partition. The web UI is at `http://bbq.local` when connected. Touchscreen temperature editors and the web protocol use °F; the touchscreen readout can show °C. Do not wire an RJ45 output from an Ethernet color convention: its end-to-end powered pinout and cable current rating still require definition.
