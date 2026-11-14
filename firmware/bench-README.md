# Serial hardware commissioning

The `bench` environment runs the real probe conversion code with manual, timed
output tests. It has no PID, networking, or touchscreen UI. The screen remains
dark. All three outputs boot off. Use it on a cold bench with the fan secured
and the servo horn/linkage removed.

From `firmware/`:

```powershell
pio run -e bench
pio run -e bench --target upload
pio device monitor -e bench --baud 115200 --filter send_on_enter
```

Select the board's serial port with `--upload-port COMx` or monitor `--port COMx`
if automatic selection is ambiguous. Type one command and press Enter.
Commands and reports are available on both built-in USB CDC and DEBUG UART0;
each port has its own input buffer. The UART is 115200 baud, 8 data bits, no
parity, 1 stop bit, no flow control. Select the power arrangement below before
opening either console.

## USB for flashing and probe checks

1. Turn off and disconnect the 12V supply. Unplug the fan and servo. Disconnect
   the H05 fan, H06 servo, and H07 buzzer output signal leads from their unpowered
   drivers/loads. This also prevents GPIO backfeeding an unpowered TC4420 gate
   driver through its input. Insulate the loose ends individually.
2. Disconnect and insulate the WT32 I/O connector's pin-1 external 5V lead
   (H01 in the new wiring packet). Keep sensor signal/ground wiring as shown
   in the build guide. This prevents USB from backfeeding the buck/servo rail.
3. Connect the WT32's built-in USB port. Flash `bench` and open its USB serial
   port. Use `status`, resistor fixtures, and probe checks.
4. Disconnect built-in USB before restoring the external 5V connection.

## External 12V for fan, servo, and load checks

Use a **3.3V logic USB-to-UART adapter**. Its VCC/power output must remain
disconnected. A power-output selector on an adapter does not necessarily set
its TX logic voltage; confirm its TX signal is 3.3V. Do not use an RS-232 port.

| WT32 7-pin DEBUG connector | USB-to-UART adapter |
|---|---|
| Pin 3, TX / GPIO43 | RX |
| Pin 4, RX / GPIO44 | TX (3.3V logic) |
| Pin 7, GND | GND |
| All other DEBUG pins | Unconnected to adapter |
| Adapter VCC, 5V, 3V3, DTR, RTS | Unconnected |

1. Disconnect all power. Remove the WT32's built-in USB cable.
2. Restore the WT32 I/O pin-1 5V lead and the H05/H06/H07 output connections.
   Connect only the three DEBUG wires
   listed above. Verify connector pin numbering against the board guide;
   cable color is not a pin number. Leave the adapter unplugged from the PC.
3. Apply external 12V through the checked power section, then plug the UART
   adapter into the PC. This avoids applying an idle-high TX signal to an
   unpowered WT32. Open the adapter's COM port:

   ```powershell
   pio device monitor -e bench --port COMx --baud 115200 --filter send_on_enter
   ```

4. Send `help` or `status`, then run the individual timed output tests below.
   The WT32's built-in USB port stays unplugged throughout these tests.
5. Send `off`, unplug the UART adapter from the PC, then disconnect 12V before
   changing wiring. Return to the USB arrangement above for another upload.

The bench reports ADC availability to both consoles at boot and probe status
every two seconds. Low-level library diagnostic messages may appear only on
USB; `OPEN` fault sentinels and the reboot requirement apply on both consoles.

| Command | Result |
|---|---|
| `help` | Prints commands and test limits |
| `status` | Raw ADC, node volts, probe status, resistance, Celsius and Fahrenheit |
| `off` | Immediately writes zero duty to fan, servo and buzzer |
| `fan 100` | Full supply voltage for 3 seconds, then off |
| `fan 50` | 50% duty at 25kHz for 3 seconds, then off |
| `fan 0` | Stops all outputs |
| `servo 1500` | 1500us pulses at 50Hz for 2 seconds, then pulse output off |
| `servo 1000` / `servo 2000` | Endpoint test, 2 seconds; remove horn/linkage first |
| `beep` | 2kHz buzzer drive for 500ms, then off |

Fan arguments are integers 0-100; servo arguments are integers 1000-2000us.
Every accepted output command first stops all other outputs. Invalid or
overlength commands also stop outputs. `status` and `help` do not extend a test.
Stopping servo pulses does not mechanically move or spring-close a damper.

Power and continuity checks in the [build guide](../docs/build-guide.md)
must pass before connecting any load. USB and external 5V must not be connected
together until the exact board's power arrangement has been verified. A generic
5015 fan is not automatically rated for supply PWM: start with `fan 100`, then
characterize permitted PWM settings using its datasheet and actual hardware.
The fixed 3-second test is a functional check, not a thermal endurance test.

## Probe fixtures

Connect a known resistor between jack tip and sleeve (power off while changing
wiring). With a 3.300V divider rail and 10k reference resistor:

| Fixture | Node voltage | Expected raw ADC | Expected temperature |
|---|---:|---:|---:|
| Jack open | ~3.300V | ~26400 | OPEN |
| 100k | 3.000V | 24000 | ~24.17C / 75.51F |
| 10k | 1.650V | 13200 | ~84.54C / 184.17F |
| 5k | 1.100V | 8800 | ~106.99C / 224.57F |
| Tip shorted to sleeve | ~0V | ~0 | SHORT |

The temperatures use this project's stored Thermoworks coefficients. Fixture
results prove wiring/conversion; they do not prove an unknown probe has these
coefficients. Verify each actual probe in an ice bath and against a reference
thermometer before controlling a fire.

Measure the 3V3 rail. Conversion assumes `PROBE_EXCITATION_VOLTAGE=3.3` in
`src/config.h`; set this to the measured stable excitation voltage when needed,
and rebuild both `bench` and normal firmware. The open threshold is 98% of that
voltage (about 25872 counts at 3.3V). Values below 100 counts indicate short.
The open guard band excludes very cold probes below roughly -8C. If the supply
changes appreciably with fan or servo activity, correct the power/wiring first.

A missing ADC at boot or a failed I2C transaction/conversion inhibits valid
probe readings until reboot. Raw values after an ADC fault are fault sentinels,
not physical measurements. Check the wiring and reboot; the firmware does not
silently resume after a bus failure.

## Restoring the controller

```powershell
pio run -e wt32_sc01_plus --target upload
```

Flashing the bench or controller environment does not intentionally erase
LittleFS. Upload web assets only when needed; an `uploadfs` operation replaces
the filesystem image and can overwrite saved configuration/session files.
Commission in Fahrenheit. UI/alarm unit conversion remains a separate review
item. The control calculation uses Fahrenheit internally.

Normal firmware now stops the fan and commands damper closed on an invalid pit
probe, waits 3 continuous valid seconds on reconnect, then restarts PID from
zero after the next 4-second compute interval. Check this with a resistor and
unplug/reconnect test on the cold bench. Electronic shutdown cannot guarantee
airflow stops if the damper is jammed, unpowered, or absent.

LEDC timers are explicitly separated: fan channel 0, servo channel 2, buzzer
channel 4, display backlight channel 7. Servo PWM uses 14-bit resolution, within
the ESP32-S3 LEDC limit. Failed PWM setup disables bench output tests.
Production defaults are now closed=1000us and open=1500us. Those are initial
bench positions, not a claim that an unknown linkage is calibrated.

## Transfer measured servo endpoints to production

1. With horn/linkage removed, start at `servo 1500` and check motion using small
   pulse changes within 1000-2000us. Record safe movement directions and limits.
2. Send the intended closed pulse, stop power, and fit the horn at the desired
   closed position. Adjust the linkage so the servo does not push against a
   mechanical stop. Power up and use brief pulses to verify movement without
   binding; keep the smoker cold throughout.
3. Record the exact pulse widths that produce a sealed closed position and the
   desired open position without sustained buzzing, binding, or supply sag.
4. Edit `DAMPER_CLOSED_US` and `DAMPER_OPEN_US` in `src/config.h` to those measured
   values. They may be reversed, such as closed=1800 and open=1200. Keep both
   within the validated 1000-2000us limits. The existing logical `DAMPER_CLOSED=0`
   and `DAMPER_OPEN=90` labels can stay unchanged; they are not shaft measurements.
5. Rebuild production with `pio run -e wt32_sc01_plus`. Remove horn/linkage for
   the first production boot and confirm the commanded closed position. Restore
   the linkage only after a cold powered test confirms it will not bind.
6. With the cold mechanism connected, verify both normal operation and a pit
   probe disconnect. Startup and pit faults use the same calibrated closed pulse.

The `bench` program continues to use explicit pulse commands; it does not load
these production endpoint constants. Firmware builds reject endpoint constants
outside `SERVO_MIN_US` and `SERVO_MAX_US`. Expanding those limits requires
separate validation of the actual servo and mechanism.
