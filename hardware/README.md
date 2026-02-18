# Hardware - KiCad Schematic

KiCad 8 project for the Pit Claw BBQ Controller carrier board circuit.

## Opening

1. Install [KiCad](https://www.kicad.org/) (free, open source)
2. Open `bbq-controller.kicad_pro`
3. The schematic editor opens `bbq-controller.kicad_sch`

## What's in the schematic

All components on the 50x70mm carrier board and their connections to the WT32-SC01 Plus:

| Ref | Component | Function |
|-----|-----------|----------|
| U1 | MP1584EN module | 12V to 5V buck converter |
| U2 | ADS1115 module | 16-bit I2C ADC (3 probe channels) |
| Q1 | IRLZ44N | N-channel MOSFET fan driver |
| R1-R3 | 10K 1% | Probe voltage divider reference resistors |
| R4 | 10K | MOSFET gate pulldown |
| C1-C3 | 0.1uF | ADC input filter capacitors |
| BZ1 | Piezo buzzer | Audio alarm |
| J1 | Barrel jack | 12V DC power input |
| J2-J4 | 2.5mm mono jacks | Temperature probe connectors |
| J5 | 8-pin header | WT32-SC01 Plus extension connector |
| J6 | 3-pin header | MG90S servo (damper) |
| J7 | 2-pin header | 5015 blower fan |

## Net labels

Inter-section connections use net labels (matching labels are electrically connected):

| Net Label | Signal | From | To |
|-----------|--------|------|-----|
| SDA | I2C data | U2 pin 3 | J5 pin 1 (GPIO10) |
| SCL | I2C clock | U2 pin 4 | J5 pin 2 (GPIO11) |
| FAN_PWM | Fan speed | Q1 gate / R4 | J5 pin 3 (GPIO12) |
| SERVO_SIG | Servo position | J6 pin 1 | J5 pin 4 (GPIO13) |
| BUZZER_SIG | Buzzer drive | BZ1 pin 1 | J5 pin 5 (GPIO14) |
| PROBE_PIT | Pit temp | R1/C1/J2 junction | U2 A0 (pin 7) |
| PROBE_MEAT1 | Meat 1 temp | R2/C2/J3 junction | U2 A1 (pin 8) |
| PROBE_MEAT2 | Meat 2 temp | R3/C3/J4 junction | U2 A2 (pin 9) |

## Exporting a viewable schematic

From KiCad's schematic editor:
- **PDF**: File > Export > PDF
- **SVG**: File > Export > SVG
- Place the exported file in `docs/` for easy reference

## Notes

- Custom symbols (prefixed `bbq:`) are embedded in the schematic file -- no external symbol libraries needed
- The schematic uses standard KiCad power symbols (+12V, +5V, +3V3, GND)
- This is a carrier board schematic, not a PCB layout -- the board is hand-wired on perfboard
