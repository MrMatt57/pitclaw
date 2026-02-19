#!/usr/bin/env python3
"""Generate KiCad 8 schematic for the Pit Claw BBQ Controller carrier board.

Run: python hardware/generate_schematic.py
Output: hardware/bbq-controller.kicad_sch

Every wire endpoint is computed from symbol pin definitions so connections
are guaranteed to match. Re-run after editing positions or topology.
"""

import uuid
import os

def uid():
    return str(uuid.uuid4())


# ---------------------------------------------------------------------------
# KiCad coordinate helpers
# ---------------------------------------------------------------------------
# Sheet coords: Y increases downward.
# Symbol local coords (in lib_symbols): Y increases upward.
# Pin (at lx ly angle) gives the CONNECTION POINT in local coords.
# Placed at (sx, sy) with rotation 0: sheet_pin = (sx + lx, sy - ly)

def pin_sheet(sx, sy, lx, ly):
    """Convert local pin position to sheet position (no rotation)."""
    return (round(sx + lx, 4), round(sy - ly, 4))


# ---------------------------------------------------------------------------
# Symbol definitions (lib_symbols)
# ---------------------------------------------------------------------------

LIB_SYMBOLS = r"""
  (lib_symbols
    (symbol "Device:R"
      (pin_numbers hide)
      (pin_names (offset 0) hide)
      (exclude_from_sim no)
      (in_bom yes)
      (on_board yes)
      (property "Reference" "R" (at 2.032 0 90) (effects (font (size 1.27 1.27))))
      (property "Value" "R" (at -2.032 0 90) (effects (font (size 1.27 1.27))))
      (property "Footprint" "" (at -1.778 0 90) (effects (font (size 1.27 1.27)) hide))
      (property "Datasheet" "~" (at 0 0 0) (effects (font (size 1.27 1.27)) hide))
      (property "Description" "Resistor" (at 0 0 0) (effects (font (size 1.27 1.27)) hide))
      (symbol "R_0_1"
        (rectangle (start -1.016 -2.54) (end 1.016 2.54)
          (stroke (width 0) (type default))
          (fill (type none))
        )
      )
      (symbol "R_1_1"
        (pin passive line (at 0 3.81 270) (length 1.27)
          (name "~" (effects (font (size 1.27 1.27))))
          (number "1" (effects (font (size 1.27 1.27))))
        )
        (pin passive line (at 0 -3.81 90) (length 1.27)
          (name "~" (effects (font (size 1.27 1.27))))
          (number "2" (effects (font (size 1.27 1.27))))
        )
      )
    )
    (symbol "Device:C"
      (pin_numbers hide)
      (pin_names (offset 0.254) hide)
      (exclude_from_sim no)
      (in_bom yes)
      (on_board yes)
      (property "Reference" "C" (at 2.54 0 90) (effects (font (size 1.27 1.27)) (justify left)))
      (property "Value" "C" (at -2.54 0 90) (effects (font (size 1.27 1.27)) (justify left)))
      (property "Footprint" "" (at 0.9652 -3.81 0) (effects (font (size 1.27 1.27)) hide))
      (property "Datasheet" "~" (at 0 0 0) (effects (font (size 1.27 1.27)) hide))
      (property "Description" "Capacitor" (at 0 0 0) (effects (font (size 1.27 1.27)) hide))
      (symbol "C_0_1"
        (polyline
          (pts (xy -2.032 -0.762) (xy 2.032 -0.762))
          (stroke (width 0.508) (type default))
          (fill (type none))
        )
        (polyline
          (pts (xy -2.032 0.762) (xy 2.032 0.762))
          (stroke (width 0.508) (type default))
          (fill (type none))
        )
      )
      (symbol "C_1_1"
        (pin passive line (at 0 3.81 270) (length 2.794)
          (name "~" (effects (font (size 1.27 1.27))))
          (number "1" (effects (font (size 1.27 1.27))))
        )
        (pin passive line (at 0 -3.81 90) (length 2.794)
          (name "~" (effects (font (size 1.27 1.27))))
          (number "2" (effects (font (size 1.27 1.27))))
        )
      )
    )
    (symbol "Device:Buzzer"
      (pin_names (offset 1.016) hide)
      (exclude_from_sim no)
      (in_bom yes)
      (on_board yes)
      (property "Reference" "BZ" (at 5.08 1.27 0) (effects (font (size 1.27 1.27)) (justify left)))
      (property "Value" "Buzzer" (at 5.08 -1.27 0) (effects (font (size 1.27 1.27)) (justify left)))
      (property "Footprint" "" (at -1.016 -3.556 90) (effects (font (size 1.27 1.27)) hide))
      (property "Datasheet" "~" (at -1.016 -3.556 90) (effects (font (size 1.27 1.27)) hide))
      (property "Description" "Buzzer, polarized" (at 0 0 0) (effects (font (size 1.27 1.27)) hide))
      (symbol "Buzzer_0_1"
        (arc (start -3.302 -2.54) (mid 2.159 0) (end -3.302 2.54)
          (stroke (width 0.254) (type default))
          (fill (type none))
        )
        (polyline
          (pts (xy -3.302 2.54) (xy -3.302 -2.54))
          (stroke (width 0.254) (type default))
          (fill (type none))
        )
        (text "+" (at -3.81 3.175 0) (effects (font (size 1.27 1.27))))
      )
      (symbol "Buzzer_1_1"
        (pin passive line (at 0 5.08 270) (length 2.54)
          (name "+" (effects (font (size 1.27 1.27))))
          (number "1" (effects (font (size 1.27 1.27))))
        )
        (pin passive line (at 0 -5.08 90) (length 2.54)
          (name "-" (effects (font (size 1.27 1.27))))
          (number "2" (effects (font (size 1.27 1.27))))
        )
      )
    )
    (symbol "bbq:IRLZ44N"
      (pin_names (offset 1.016))
      (exclude_from_sim no)
      (in_bom yes)
      (on_board yes)
      (property "Reference" "Q" (at 7.62 1.27 0) (effects (font (size 1.27 1.27)) (justify left)))
      (property "Value" "IRLZ44N" (at 7.62 -1.27 0) (effects (font (size 1.27 1.27)) (justify left)))
      (property "Footprint" "Package_TO_SOT_THT:TO-220-3_Vertical" (at 0 0 0) (effects (font (size 1.27 1.27)) hide))
      (property "Datasheet" "" (at 0 0 0) (effects (font (size 1.27 1.27)) hide))
      (property "Description" "N-channel logic-level MOSFET, TO-220" (at 0 0 0) (effects (font (size 1.27 1.27)) hide))
      (symbol "IRLZ44N_0_1"
        (polyline
          (pts (xy -2.54 -2.54) (xy -2.54 2.54))
          (stroke (width 0.254) (type default))
          (fill (type none))
        )
        (polyline
          (pts (xy -1.27 2.54) (xy -1.27 -2.54))
          (stroke (width 0.508) (type default))
          (fill (type none))
        )
        (polyline
          (pts (xy -1.27 -1.905) (xy 2.54 -1.905) (xy 2.54 -2.54))
          (stroke (width 0.254) (type default))
          (fill (type none))
        )
        (polyline
          (pts (xy -1.27 1.905) (xy 2.54 1.905) (xy 2.54 2.54))
          (stroke (width 0.254) (type default))
          (fill (type none))
        )
        (polyline
          (pts (xy -1.27 0) (xy 2.54 0))
          (stroke (width 0.254) (type default))
          (fill (type none))
        )
        (polyline
          (pts (xy 2.54 0) (xy 0.762 0.762) (xy 0.762 -0.762) (xy 2.54 0))
          (stroke (width 0.254) (type default))
          (fill (type outline))
        )
        (text "D" (at 3.81 -2.54 0) (effects (font (size 0.762 0.762))))
        (text "G" (at -3.81 0 0) (effects (font (size 0.762 0.762))))
        (text "S" (at 3.81 2.54 0) (effects (font (size 0.762 0.762))))
      )
      (symbol "IRLZ44N_1_1"
        (pin input line (at -5.08 0 0) (length 2.54)
          (name "G" (effects (font (size 1.27 1.27))))
          (number "1" (effects (font (size 1.27 1.27))))
        )
        (pin passive line (at 2.54 -5.08 90) (length 2.54)
          (name "D" (effects (font (size 1.27 1.27))))
          (number "2" (effects (font (size 1.27 1.27))))
        )
        (pin passive line (at 2.54 5.08 270) (length 2.54)
          (name "S" (effects (font (size 1.27 1.27))))
          (number "3" (effects (font (size 1.27 1.27))))
        )
      )
    )
    (symbol "bbq:MP1584EN_Module"
      (pin_names (offset 1.016))
      (exclude_from_sim no)
      (in_bom yes)
      (on_board yes)
      (property "Reference" "U" (at 0 6.35 0) (effects (font (size 1.27 1.27))))
      (property "Value" "MP1584EN" (at 0 -6.35 0) (effects (font (size 1.27 1.27))))
      (property "Footprint" "" (at 0 0 0) (effects (font (size 1.27 1.27)) hide))
      (property "Datasheet" "" (at 0 0 0) (effects (font (size 1.27 1.27)) hide))
      (property "Description" "MP1584EN adjustable buck converter module" (at 0 0 0) (effects (font (size 1.27 1.27)) hide))
      (symbol "MP1584EN_Module_0_1"
        (rectangle (start -7.62 5.08) (end 7.62 -5.08)
          (stroke (width 0.254) (type default))
          (fill (type background))
        )
      )
      (symbol "MP1584EN_Module_1_1"
        (pin power_in line (at -10.16 2.54 0) (length 2.54)
          (name "IN+" (effects (font (size 1.27 1.27))))
          (number "1" (effects (font (size 1.27 1.27))))
        )
        (pin power_in line (at -10.16 -2.54 0) (length 2.54)
          (name "IN-" (effects (font (size 1.27 1.27))))
          (number "2" (effects (font (size 1.27 1.27))))
        )
        (pin power_out line (at 10.16 2.54 180) (length 2.54)
          (name "OUT+" (effects (font (size 1.27 1.27))))
          (number "3" (effects (font (size 1.27 1.27))))
        )
        (pin power_out line (at 10.16 -2.54 180) (length 2.54)
          (name "OUT-" (effects (font (size 1.27 1.27))))
          (number "4" (effects (font (size 1.27 1.27))))
        )
      )
    )
    (symbol "bbq:ADS1115_Module"
      (pin_names (offset 1.016))
      (exclude_from_sim no)
      (in_bom yes)
      (on_board yes)
      (property "Reference" "U" (at 0 13.97 0) (effects (font (size 1.27 1.27))))
      (property "Value" "ADS1115" (at 0 -13.97 0) (effects (font (size 1.27 1.27))))
      (property "Footprint" "" (at 0 0 0) (effects (font (size 1.27 1.27)) hide))
      (property "Datasheet" "" (at 0 0 0) (effects (font (size 1.27 1.27)) hide))
      (property "Description" "ADS1115 16-bit I2C ADC breakout module" (at 0 0 0) (effects (font (size 1.27 1.27)) hide))
      (symbol "ADS1115_Module_0_1"
        (rectangle (start -7.62 12.7) (end 7.62 -12.7)
          (stroke (width 0.254) (type default))
          (fill (type background))
        )
      )
      (symbol "ADS1115_Module_1_1"
        (pin input line (at -10.16 7.62 0) (length 2.54)
          (name "A0" (effects (font (size 1.27 1.27))))
          (number "1" (effects (font (size 1.27 1.27))))
        )
        (pin input line (at -10.16 5.08 0) (length 2.54)
          (name "A1" (effects (font (size 1.27 1.27))))
          (number "2" (effects (font (size 1.27 1.27))))
        )
        (pin input line (at -10.16 2.54 0) (length 2.54)
          (name "A2" (effects (font (size 1.27 1.27))))
          (number "3" (effects (font (size 1.27 1.27))))
        )
        (pin input line (at -10.16 0 0) (length 2.54)
          (name "A3" (effects (font (size 1.27 1.27))))
          (number "4" (effects (font (size 1.27 1.27))))
        )
        (pin power_in line (at 0 15.24 270) (length 2.54)
          (name "VDD" (effects (font (size 1.27 1.27))))
          (number "5" (effects (font (size 1.27 1.27))))
        )
        (pin power_in line (at 0 -15.24 90) (length 2.54)
          (name "GND" (effects (font (size 1.27 1.27))))
          (number "6" (effects (font (size 1.27 1.27))))
        )
        (pin bidirectional line (at 10.16 5.08 180) (length 2.54)
          (name "SDA" (effects (font (size 1.27 1.27))))
          (number "7" (effects (font (size 1.27 1.27))))
        )
        (pin output line (at 10.16 2.54 180) (length 2.54)
          (name "SCL" (effects (font (size 1.27 1.27))))
          (number "8" (effects (font (size 1.27 1.27))))
        )
        (pin input line (at 10.16 -5.08 180) (length 2.54)
          (name "ADDR" (effects (font (size 1.27 1.27))))
          (number "9" (effects (font (size 1.27 1.27))))
        )
      )
    )
    (symbol "Connector_Generic:Conn_01x02"
      (pin_names (offset 1.016) hide)
      (exclude_from_sim no)
      (in_bom yes)
      (on_board yes)
      (property "Reference" "J" (at 0 2.54 0) (effects (font (size 1.27 1.27))))
      (property "Value" "Conn_01x02" (at 0 -5.08 0) (effects (font (size 1.27 1.27))))
      (property "Footprint" "" (at 0 0 0) (effects (font (size 1.27 1.27)) hide))
      (property "Datasheet" "~" (at 0 0 0) (effects (font (size 1.27 1.27)) hide))
      (property "Description" "Generic 2-pin connector" (at 0 0 0) (effects (font (size 1.27 1.27)) hide))
      (symbol "Conn_01x02_1_1"
        (rectangle (start -1.27 -2.413) (end 0 -2.667))
        (rectangle (start -1.27 0.127) (end 0 -0.127))
        (pin passive line (at -2.54 0 0) (length 1.27)
          (name "Pin_1" (effects (font (size 1.27 1.27))))
          (number "1" (effects (font (size 1.27 1.27))))
        )
        (pin passive line (at -2.54 -2.54 0) (length 1.27)
          (name "Pin_2" (effects (font (size 1.27 1.27))))
          (number "2" (effects (font (size 1.27 1.27))))
        )
      )
    )
    (symbol "Connector_Generic:Conn_01x03"
      (pin_names (offset 1.016) hide)
      (exclude_from_sim no)
      (in_bom yes)
      (on_board yes)
      (property "Reference" "J" (at 0 5.08 0) (effects (font (size 1.27 1.27))))
      (property "Value" "Conn_01x03" (at 0 -5.08 0) (effects (font (size 1.27 1.27))))
      (property "Footprint" "" (at 0 0 0) (effects (font (size 1.27 1.27)) hide))
      (property "Datasheet" "~" (at 0 0 0) (effects (font (size 1.27 1.27)) hide))
      (property "Description" "Generic 3-pin connector" (at 0 0 0) (effects (font (size 1.27 1.27)) hide))
      (symbol "Conn_01x03_1_1"
        (rectangle (start -1.27 -4.953) (end 0 -5.207))
        (rectangle (start -1.27 -2.413) (end 0 -2.667))
        (rectangle (start -1.27 0.127) (end 0 -0.127))
        (pin passive line (at -2.54 0 0) (length 1.27)
          (name "Pin_1" (effects (font (size 1.27 1.27))))
          (number "1" (effects (font (size 1.27 1.27))))
        )
        (pin passive line (at -2.54 -2.54 0) (length 1.27)
          (name "Pin_2" (effects (font (size 1.27 1.27))))
          (number "2" (effects (font (size 1.27 1.27))))
        )
        (pin passive line (at -2.54 -5.08 0) (length 1.27)
          (name "Pin_3" (effects (font (size 1.27 1.27))))
          (number "3" (effects (font (size 1.27 1.27))))
        )
      )
    )
    (symbol "Connector_Generic:Conn_01x08"
      (pin_names (offset 1.016) hide)
      (exclude_from_sim no)
      (in_bom yes)
      (on_board yes)
      (property "Reference" "J" (at 0 10.16 0) (effects (font (size 1.27 1.27))))
      (property "Value" "Conn_01x08" (at 0 -20.32 0) (effects (font (size 1.27 1.27))))
      (property "Footprint" "" (at 0 0 0) (effects (font (size 1.27 1.27)) hide))
      (property "Datasheet" "~" (at 0 0 0) (effects (font (size 1.27 1.27)) hide))
      (property "Description" "Generic 8-pin connector" (at 0 0 0) (effects (font (size 1.27 1.27)) hide))
      (symbol "Conn_01x08_1_1"
        (rectangle (start -1.27 -17.653) (end 0 -17.907))
        (rectangle (start -1.27 -15.113) (end 0 -15.367))
        (rectangle (start -1.27 -12.573) (end 0 -12.827))
        (rectangle (start -1.27 -10.033) (end 0 -10.287))
        (rectangle (start -1.27 -7.493) (end 0 -7.747))
        (rectangle (start -1.27 -4.953) (end 0 -5.207))
        (rectangle (start -1.27 -2.413) (end 0 -2.667))
        (rectangle (start -1.27 0.127) (end 0 -0.127))
        (pin passive line (at -2.54 0 0) (length 1.27)
          (name "Pin_1" (effects (font (size 1.27 1.27))))
          (number "1" (effects (font (size 1.27 1.27))))
        )
        (pin passive line (at -2.54 -2.54 0) (length 1.27)
          (name "Pin_2" (effects (font (size 1.27 1.27))))
          (number "2" (effects (font (size 1.27 1.27))))
        )
        (pin passive line (at -2.54 -5.08 0) (length 1.27)
          (name "Pin_3" (effects (font (size 1.27 1.27))))
          (number "3" (effects (font (size 1.27 1.27))))
        )
        (pin passive line (at -2.54 -7.62 0) (length 1.27)
          (name "Pin_4" (effects (font (size 1.27 1.27))))
          (number "4" (effects (font (size 1.27 1.27))))
        )
        (pin passive line (at -2.54 -10.16 0) (length 1.27)
          (name "Pin_5" (effects (font (size 1.27 1.27))))
          (number "5" (effects (font (size 1.27 1.27))))
        )
        (pin passive line (at -2.54 -12.7 0) (length 1.27)
          (name "Pin_6" (effects (font (size 1.27 1.27))))
          (number "6" (effects (font (size 1.27 1.27))))
        )
        (pin passive line (at -2.54 -15.24 0) (length 1.27)
          (name "Pin_7" (effects (font (size 1.27 1.27))))
          (number "7" (effects (font (size 1.27 1.27))))
        )
        (pin passive line (at -2.54 -17.78 0) (length 1.27)
          (name "Pin_8" (effects (font (size 1.27 1.27))))
          (number "8" (effects (font (size 1.27 1.27))))
        )
      )
    )
    (symbol "power:+12V"
      (power)
      (pin_names (offset 0))
      (exclude_from_sim no)
      (in_bom yes)
      (on_board yes)
      (property "Reference" "#PWR" (at 0 -3.81 0) (effects (font (size 1.27 1.27)) hide))
      (property "Value" "+12V" (at 0 3.556 0) (effects (font (size 1.27 1.27))))
      (property "Footprint" "" (at 0 0 0) (effects (font (size 1.27 1.27)) hide))
      (property "Datasheet" "" (at 0 0 0) (effects (font (size 1.27 1.27)) hide))
      (property "Description" "+12V power rail" (at 0 0 0) (effects (font (size 1.27 1.27)) hide))
      (symbol "+12V_0_1"
        (polyline
          (pts (xy -0.762 1.27) (xy 0 2.54))
          (stroke (width 0) (type default))
          (fill (type none))
        )
        (polyline
          (pts (xy 0 0) (xy 0 2.54))
          (stroke (width 0) (type default))
          (fill (type none))
        )
        (polyline
          (pts (xy 0 2.54) (xy 0.762 1.27))
          (stroke (width 0) (type default))
          (fill (type none))
        )
      )
      (symbol "+12V_1_1"
        (pin power_in line (at 0 0 90) (length 0)
          (name "+12V" (effects (font (size 1.27 1.27))))
          (number "1" (effects (font (size 1.27 1.27))))
        )
      )
    )
    (symbol "power:+5V"
      (power)
      (pin_names (offset 0))
      (exclude_from_sim no)
      (in_bom yes)
      (on_board yes)
      (property "Reference" "#PWR" (at 0 -3.81 0) (effects (font (size 1.27 1.27)) hide))
      (property "Value" "+5V" (at 0 3.556 0) (effects (font (size 1.27 1.27))))
      (property "Footprint" "" (at 0 0 0) (effects (font (size 1.27 1.27)) hide))
      (property "Datasheet" "" (at 0 0 0) (effects (font (size 1.27 1.27)) hide))
      (property "Description" "+5V power rail" (at 0 0 0) (effects (font (size 1.27 1.27)) hide))
      (symbol "+5V_0_1"
        (polyline
          (pts (xy -0.762 1.27) (xy 0 2.54))
          (stroke (width 0) (type default))
          (fill (type none))
        )
        (polyline
          (pts (xy 0 0) (xy 0 2.54))
          (stroke (width 0) (type default))
          (fill (type none))
        )
        (polyline
          (pts (xy 0 2.54) (xy 0.762 1.27))
          (stroke (width 0) (type default))
          (fill (type none))
        )
      )
      (symbol "+5V_1_1"
        (pin power_in line (at 0 0 90) (length 0)
          (name "+5V" (effects (font (size 1.27 1.27))))
          (number "1" (effects (font (size 1.27 1.27))))
        )
      )
    )
    (symbol "power:+3V3"
      (power)
      (pin_names (offset 0))
      (exclude_from_sim no)
      (in_bom yes)
      (on_board yes)
      (property "Reference" "#PWR" (at 0 -3.81 0) (effects (font (size 1.27 1.27)) hide))
      (property "Value" "+3V3" (at 0 3.556 0) (effects (font (size 1.27 1.27))))
      (property "Footprint" "" (at 0 0 0) (effects (font (size 1.27 1.27)) hide))
      (property "Datasheet" "" (at 0 0 0) (effects (font (size 1.27 1.27)) hide))
      (property "Description" "+3.3V power rail" (at 0 0 0) (effects (font (size 1.27 1.27)) hide))
      (symbol "+3V3_0_1"
        (polyline
          (pts (xy -0.762 1.27) (xy 0 2.54))
          (stroke (width 0) (type default))
          (fill (type none))
        )
        (polyline
          (pts (xy 0 0) (xy 0 2.54))
          (stroke (width 0) (type default))
          (fill (type none))
        )
        (polyline
          (pts (xy 0 2.54) (xy 0.762 1.27))
          (stroke (width 0) (type default))
          (fill (type none))
        )
      )
      (symbol "+3V3_1_1"
        (pin power_in line (at 0 0 90) (length 0)
          (name "+3V3" (effects (font (size 1.27 1.27))))
          (number "1" (effects (font (size 1.27 1.27))))
        )
      )
    )
    (symbol "power:GND"
      (power)
      (pin_names (offset 0))
      (exclude_from_sim no)
      (in_bom yes)
      (on_board yes)
      (property "Reference" "#PWR" (at 0 -6.35 0) (effects (font (size 1.27 1.27)) hide))
      (property "Value" "GND" (at 0 -3.81 0) (effects (font (size 1.27 1.27))))
      (property "Footprint" "" (at 0 0 0) (effects (font (size 1.27 1.27)) hide))
      (property "Datasheet" "" (at 0 0 0) (effects (font (size 1.27 1.27)) hide))
      (property "Description" "Ground" (at 0 0 0) (effects (font (size 1.27 1.27)) hide))
      (symbol "GND_0_1"
        (polyline
          (pts (xy 0 0) (xy 0 -1.27) (xy 1.27 -1.27) (xy 0 -2.54) (xy -1.27 -1.27) (xy 0 -1.27))
          (stroke (width 0) (type default))
          (fill (type none))
        )
      )
      (symbol "GND_1_1"
        (pin power_in line (at 0 0 270) (length 0)
          (name "GND" (effects (font (size 1.27 1.27))))
          (number "1" (effects (font (size 1.27 1.27))))
        )
      )
    )
  )
"""


# ---------------------------------------------------------------------------
# Pin offset definitions (local coords -> pin connection point offset)
# For symbol at (sx, sy), sheet pin = (sx + dx, sy + dy) where dy = -local_y
# ---------------------------------------------------------------------------

# Device:R — pin 1 at local(0, 3.81), pin 2 at local(0, -3.81)
R_PIN1 = (0, -3.81)   # top (sheet y goes up from center)
R_PIN2 = (0,  3.81)   # bottom

# Device:C — same as R
C_PIN1 = (0, -3.81)
C_PIN2 = (0,  3.81)

# Device:Buzzer — pin 1(+) at local(0, 5.08), pin 2(-) at local(0, -5.08)
BZ_PIN1 = (0, -5.08)  # + top
BZ_PIN2 = (0,  5.08)  # - bottom

# bbq:IRLZ44N — Gate local(-5.08, 0), Drain local(2.54, -5.08), Source local(2.54, 5.08)
Q_GATE   = (-5.08,  0)
Q_DRAIN  = ( 2.54, 5.08)   # local y=-5.08 -> sheet dy=+5.08... wait
# Actually: local (2.54, -5.08) -> sheet offset (2.54, -(-5.08)) = (2.54, 5.08)
# That means Drain is BELOW center on sheet. That's wrong for a MOSFET.
# Let me re-check: Drain should be at TOP (lower sheet Y).
# Pin def: (at 2.54 -5.08 90) -> local pos (2.54, -5.08), sheet offset = (2.54, +5.08)
# Sheet +5.08 means BELOW center. That's Source position.
#
# Wait, I defined Drain as local(2.54, -5.08, 90) which means connection at bottom of symbol.
# And Source as local(2.54, 5.08, 270) which means connection at top of symbol.
# But in a MOSFET, Drain should be at top and Source at bottom!
# I have them swapped in the symbol definition. Let me fix:
# Drain pin: local (2.54, 5.08, 270) -> sheet (sx+2.54, sy-5.08) = above center = TOP ✓
# Source pin: local (2.54, -5.08, 90) -> sheet (sx+2.54, sy+5.08) = below center = BOTTOM ✓
# But I wrote them the other way in the lib_symbols above! Let me fix that in the output.

# After fixing (Drain at local 5.08, Source at local -5.08):
Q_GATE   = (-5.08,  0)
Q_DRAIN  = ( 2.54, -5.08)  # sheet: ABOVE center (top)
Q_SOURCE = ( 2.54,  5.08)  # sheet: BELOW center (bottom)

# bbq:MP1584EN — IN+ local(-10.16, 2.54), IN- local(-10.16, -2.54),
#                OUT+ local(10.16, 2.54), OUT- local(10.16, -2.54)
MP_INP  = (-10.16, -2.54)  # sheet: sy - 2.54
MP_INN  = (-10.16,  2.54)  # sheet: sy + 2.54
MP_OUTP = ( 10.16, -2.54)
MP_OUTN = ( 10.16,  2.54)

# bbq:ADS1115 — see lib_symbols above
ADS_A0   = (-10.16, -7.62)
ADS_A1   = (-10.16, -5.08)
ADS_A2   = (-10.16, -2.54)
ADS_A3   = (-10.16,  0)
ADS_VDD  = (  0,    -15.24)
ADS_GND  = (  0,     15.24)
ADS_SDA  = ( 10.16,  -5.08)
ADS_SCL  = ( 10.16,  -2.54)
ADS_ADDR = ( 10.16,   5.08)

# Conn_01x02 — pin 1 at local(-2.54, 0), pin 2 at local(-2.54, -2.54)
C2_P1 = (-2.54,  0)
C2_P2 = (-2.54,  2.54)

# Conn_01x03 — pin 1 at local(-2.54, 0), pin 2 at local(-2.54, -2.54), pin 3 at local(-2.54, -5.08)
C3_P1 = (-2.54,  0)
C3_P2 = (-2.54,  2.54)
C3_P3 = (-2.54,  5.08)

# Conn_01x08 — pin n at local(-2.54, -(n-1)*2.54) for n=1..8
C8_PINS = [(-2.54, (n-1)*2.54) for n in range(1, 9)]  # dy offsets on sheet

# Power symbols: single pin at local (0, 0). Direction varies:
# +12V, +5V, +3V3: pin at (0,0,90) -> goes up -> place ABOVE wire point
# GND: pin at (0,0,270) -> goes down -> place BELOW wire point
# For power symbols, the connection point is at the symbol's sheet position.


# ---------------------------------------------------------------------------
# Component placement (sheet coordinates)
# ---------------------------------------------------------------------------

# Section 1: Power Supply (upper-left)
J1_POS  = (38.1,   55.88)    # Barrel jack
U1_POS  = (76.2,   55.88)    # MP1584EN

# Section 2: Probe Voltage Dividers (upper-right, 3 columns)
R1_POS  = (175.26, 48.26)    # Pit probe resistor
R2_POS  = (213.36, 48.26)    # Meat1 probe resistor
R3_POS  = (251.46, 48.26)    # Meat2 probe resistor
C1_POS  = (175.26, 63.5)     # Pit filter cap
C2_POS  = (213.36, 63.5)     # Meat1 filter cap
C3_POS  = (251.46, 63.5)     # Meat2 filter cap
J2_POS  = (187.96, 55.88)    # Pit probe jack
J3_POS  = (226.06, 55.88)    # Meat1 probe jack
J4_POS  = (264.16, 55.88)    # Meat2 probe jack

# Section 3: ADS1115 (middle)
U2_POS  = (213.36, 109.22)   # ADS1115 center

# Section 4: Fan Driver (lower-left)
Q1_POS  = (63.5,  119.38)    # IRLZ44N
R4_POS  = (48.26, 119.38)    # Gate pulldown
J7_POS  = (78.74, 104.14)    # Fan connector

# Section 5: Outputs (lower-right)
J5_POS  = (330.2, 114.3)     # WT32 8-pin connector
J6_POS  = (330.2, 157.48)    # Servo 3-pin connector
BZ1_POS = (294.64, 165.1)    # Buzzer


def p(sx, sy, offset):
    """Get sheet position of a pin: (sx + dx, sy + dy)."""
    return (round(sx + offset[0], 4), round(sy + offset[1], 4))


def fmt(v):
    """Format a coordinate value, dropping unnecessary trailing zeros."""
    if v == int(v):
        return str(int(v))
    return f"{v:.4f}".rstrip('0').rstrip('.')


def wire(x1, y1, x2, y2):
    return f'    (wire (pts (xy {fmt(x1)} {fmt(y1)}) (xy {fmt(x2)} {fmt(y2)})) (stroke (width 0) (type default)) (uuid "{uid()}"))'


def junction(x, y):
    return f'    (junction (at {fmt(x)} {fmt(y)}) (diameter 0) (color 0 0 0 0) (uuid "{uid()}"))'


def label(x, y, text, angle=0):
    return f'    (label "{text}" (at {fmt(x)} {fmt(y)} {angle}) (effects (font (size 1.27 1.27))) (uuid "{uid()}"))'


def global_label(x, y, text, angle=0, shape="bidirectional"):
    return f'    (global_label "{text}" (at {fmt(x)} {fmt(y)} {angle}) (shape {shape}) (effects (font (size 1.27 1.27)) (justify left)) (uuid "{uid()}"))'


def no_connect(x, y):
    return f'    (no_connect (at {fmt(x)} {fmt(y)}) (uuid "{uid()}"))'


def text_note(x, y, text, size=2.54):
    return f'    (text "{text}" (at {fmt(x)} {fmt(y)} 0) (effects (font (size {size} {size}) bold)))'


def symbol_instance(lib_id, ref, value, sx, sy, unit=1, props=None):
    """Generate a symbol instance on the sheet."""
    lines = []
    lines.append(f'    (symbol (lib_id "{lib_id}") (at {fmt(sx)} {fmt(sy)} 0) (unit {unit})')
    lines.append(f'      (in_bom yes) (on_board yes) (dnp no)')
    lines.append(f'      (uuid "{uid()}")')
    lines.append(f'      (property "Reference" "{ref}" (at {fmt(sx)} {fmt(sy - 8)} 0) (effects (font (size 1.27 1.27))))')
    lines.append(f'      (property "Value" "{value}" (at {fmt(sx)} {fmt(sy + 8)} 0) (effects (font (size 1.27 1.27))))')
    lines.append(f'      (property "Footprint" "" (at {fmt(sx)} {fmt(sy)} 0) (effects (font (size 1.27 1.27)) hide))')
    lines.append(f'      (property "Datasheet" "" (at {fmt(sx)} {fmt(sy)} 0) (effects (font (size 1.27 1.27)) hide))')
    if props:
        for k, v in props.items():
            lines.append(f'      (property "{k}" "{v}" (at {fmt(sx)} {fmt(sy)} 0) (effects (font (size 1.27 1.27)) hide))')
    # Pin instances (use default)
    lines.append(f'    )')
    return '\n'.join(lines)


def power_symbol(lib_id, ref, value, sx, sy):
    """Generate a power symbol (simplified properties)."""
    lines = []
    lines.append(f'    (symbol (lib_id "{lib_id}") (at {fmt(sx)} {fmt(sy)} 0) (unit 1)')
    lines.append(f'      (in_bom yes) (on_board yes) (dnp no)')
    lines.append(f'      (uuid "{uid()}")')
    lines.append(f'      (property "Reference" "{ref}" (at {fmt(sx)} {fmt(sy - 3)} 0) (effects (font (size 1.27 1.27)) hide))')
    lines.append(f'      (property "Value" "{value}" (at {fmt(sx)} {fmt(sy - 2)} 0) (effects (font (size 1.27 1.27))))')
    lines.append(f'      (property "Footprint" "" (at {fmt(sx)} {fmt(sy)} 0) (effects (font (size 1.27 1.27)) hide))')
    lines.append(f'      (property "Datasheet" "" (at {fmt(sx)} {fmt(sy)} 0) (effects (font (size 1.27 1.27)) hide))')
    lines.append(f'    )')
    return '\n'.join(lines)


pwr_idx = [0]
def pwr_ref():
    pwr_idx[0] += 1
    return f"#PWR{pwr_idx[0]:02d}"


def generate():
    parts = []

    # --- Header ---
    parts.append(f"""(kicad_sch
  (version 20231120)
  (generator "eeschema")
  (generator_version "8.0")
  (uuid "{uid()}")
  (paper "A3")
  (title_block
    (title "Pit Claw BBQ Controller")
    (date "2026-02-18")
    (rev "A")
    (company "Pit Claw Open Source")
    (comment 1 "Carrier Board (50x70mm perfboard)")
    (comment 2 "WT32-SC01 Plus + ADS1115 + IRLZ44N fan driver")
  )""")

    # --- Lib Symbols ---
    # Fix the MOSFET symbol: swap Drain and Source pin positions
    fixed_lib = LIB_SYMBOLS.replace(
        '(pin passive line (at 2.54 -5.08 90) (length 2.54)\n'
        '          (name "D" (effects (font (size 1.27 1.27))))\n'
        '          (number "2" (effects (font (size 1.27 1.27))))\n'
        '        )\n'
        '        (pin passive line (at 2.54 5.08 270) (length 2.54)\n'
        '          (name "S" (effects (font (size 1.27 1.27))))\n'
        '          (number "3" (effects (font (size 1.27 1.27))))',
        '(pin passive line (at 2.54 5.08 270) (length 2.54)\n'
        '          (name "D" (effects (font (size 1.27 1.27))))\n'
        '          (number "2" (effects (font (size 1.27 1.27))))\n'
        '        )\n'
        '        (pin passive line (at 2.54 -5.08 90) (length 2.54)\n'
        '          (name "S" (effects (font (size 1.27 1.27))))\n'
        '          (number "3" (effects (font (size 1.27 1.27))))'
    )
    parts.append(fixed_lib)

    elements = []

    # =====================================================================
    # SECTION TITLE ANNOTATIONS
    # =====================================================================
    elements.append(text_note(38.1, 30.48, "POWER SUPPLY", 3.81))
    elements.append(text_note(175.26, 30.48, "PROBE VOLTAGE DIVIDERS", 3.81))
    elements.append(text_note(175.26, 88.9, "ADS1115 ADC", 3.81))
    elements.append(text_note(38.1, 91.44, "FAN DRIVER", 3.81))
    elements.append(text_note(294.64, 96.52, "WT32-SC01 PLUS + OUTPUTS", 3.81))

    # =====================================================================
    # SECTION 1: POWER SUPPLY
    # =====================================================================

    # J1 - Barrel Jack (12V input)
    elements.append(symbol_instance("Connector_Generic:Conn_01x02", "J1", "DC_12V", *J1_POS))
    j1p1 = p(*J1_POS, C2_P1)  # 12V (tip)
    j1p2 = p(*J1_POS, C2_P2)  # GND (sleeve)

    # U1 - MP1584EN buck converter
    elements.append(symbol_instance("bbq:MP1584EN_Module", "U1", "MP1584EN", *U1_POS))
    u1_inp = p(*U1_POS, MP_INP)
    u1_inn = p(*U1_POS, MP_INN)
    u1_outp = p(*U1_POS, MP_OUTP)
    u1_outn = p(*U1_POS, MP_OUTN)

    # Wire J1 pin 1 -> U1 IN+ (horizontal)
    elements.append(wire(j1p1[0], j1p1[1], u1_inp[0], j1p1[1]))
    elements.append(wire(u1_inp[0], j1p1[1], u1_inp[0], u1_inp[1]))

    # Wire J1 pin 2 -> U1 IN-
    elements.append(wire(j1p2[0], j1p2[1], u1_inn[0], j1p2[1]))
    elements.append(wire(u1_inn[0], j1p2[1], u1_inn[0], u1_inn[1]))

    # +12V power symbol above J1 pin 1 wire
    pwr12v_j1 = (j1p1[0], j1p1[1] - 5.08)
    elements.append(power_symbol("power:+12V", pwr_ref(), "+12V", *pwr12v_j1))
    elements.append(wire(j1p1[0], j1p1[1], pwr12v_j1[0], pwr12v_j1[1]))

    # GND below J1 pin 2
    gnd_j1 = (j1p2[0], j1p2[1] + 5.08)
    elements.append(power_symbol("power:GND", pwr_ref(), "GND", *gnd_j1))
    elements.append(wire(j1p2[0], j1p2[1], gnd_j1[0], gnd_j1[1]))

    # +5V on U1 OUT+
    pwr5v_u1 = (u1_outp[0], u1_outp[1] - 5.08)
    elements.append(power_symbol("power:+5V", pwr_ref(), "+5V", *pwr5v_u1))
    elements.append(wire(u1_outp[0], u1_outp[1], pwr5v_u1[0], pwr5v_u1[1]))

    # GND on U1 OUT-
    gnd_u1 = (u1_outn[0], u1_outn[1] + 5.08)
    elements.append(power_symbol("power:GND", pwr_ref(), "GND", *gnd_u1))
    elements.append(wire(u1_outn[0], u1_outn[1], gnd_u1[0], gnd_u1[1]))

    # +12V on U1 IN+ (also labeled)
    pwr12v_u1 = (u1_inp[0], u1_inp[1] - 5.08)
    elements.append(power_symbol("power:+12V", pwr_ref(), "+12V", *pwr12v_u1))
    elements.append(wire(u1_inp[0], u1_inp[1], pwr12v_u1[0], pwr12v_u1[1]))

    # =====================================================================
    # SECTION 2: PROBE VOLTAGE DIVIDERS (3 identical columns)
    # =====================================================================

    probe_names = ["PROBE_PIT", "PROBE_MEAT1", "PROBE_MEAT2"]
    r_positions = [R1_POS, R2_POS, R3_POS]
    c_positions = [C1_POS, C2_POS, C3_POS]
    j_positions = [J2_POS, J3_POS, J4_POS]
    r_refs = ["R1", "R2", "R3"]
    c_refs = ["C1", "C2", "C3"]
    j_refs = ["J2", "J3", "J4"]
    j_labels = ["Pit Probe", "Meat1 Probe", "Meat2 Probe"]

    for i in range(3):
        rx, ry = r_positions[i]
        cx, cy = c_positions[i]
        jx, jy = j_positions[i]

        # Resistor
        elements.append(symbol_instance("Device:R", r_refs[i], "10K", rx, ry))
        r_top = p(rx, ry, R_PIN1)   # pin 1 top (to 3.3V)
        r_bot = p(rx, ry, R_PIN2)   # pin 2 bottom (junction)

        # Capacitor
        elements.append(symbol_instance("Device:C", c_refs[i], "100nF", cx, cy))
        c_top = p(cx, cy, C_PIN1)   # pin 1 top (junction)
        c_bot = p(cx, cy, C_PIN2)   # pin 2 bottom (GND)

        # Probe jack connector (2-pin)
        elements.append(symbol_instance("Connector_Generic:Conn_01x02", j_refs[i], j_labels[i], jx, jy))
        j_p1 = p(jx, jy, C2_P1)    # tip (to junction)
        j_p2 = p(jx, jy, C2_P2)    # sleeve (to GND)

        # +3.3V above resistor top
        pwr33 = (r_top[0], r_top[1] - 5.08)
        elements.append(power_symbol("power:+3V3", pwr_ref(), "+3V3", *pwr33))
        elements.append(wire(r_top[0], r_top[1], pwr33[0], pwr33[1]))

        # Junction node: R bottom, C top, and probe jack
        jn_y = r_bot[1]  # junction Y = R bottom pin Y

        # Wire R bottom -> down to junction line
        # Wire C top to junction (horizontal then vertical)
        # Since R and C are at the same X, just wire R_bot to C_top vertically
        elements.append(wire(r_bot[0], r_bot[1], c_top[0], c_top[1]))

        # Junction at R bottom where horizontal goes to probe
        elements.append(junction(r_bot[0], r_bot[1]))

        # Wire from junction to probe jack pin 1 (horizontal)
        elements.append(wire(r_bot[0], r_bot[1], j_p1[0], r_bot[1]))
        # Wire probe jack pin 1 (which is at j_p1[1]) to the junction Y level
        if abs(j_p1[1] - r_bot[1]) > 0.01:
            elements.append(wire(j_p1[0], j_p1[1], j_p1[0], r_bot[1]))

        # GND below capacitor
        gnd_c = (c_bot[0], c_bot[1] + 5.08)
        elements.append(power_symbol("power:GND", pwr_ref(), "GND", *gnd_c))
        elements.append(wire(c_bot[0], c_bot[1], gnd_c[0], gnd_c[1]))

        # GND on probe jack sleeve
        gnd_j = (j_p2[0], j_p2[1] + 5.08)
        elements.append(power_symbol("power:GND", pwr_ref(), "GND", *gnd_j))
        elements.append(wire(j_p2[0], j_p2[1], gnd_j[0], gnd_j[1]))

        # Net label at junction for ADC connection
        elements.append(label(r_bot[0] - 2.54, r_bot[1], probe_names[i], 180))
        elements.append(wire(r_bot[0], r_bot[1], r_bot[0] - 2.54, r_bot[1]))

    # =====================================================================
    # SECTION 3: ADS1115 ADC
    # =====================================================================

    u2x, u2y = U2_POS
    elements.append(symbol_instance("bbq:ADS1115_Module", "U2", "ADS1115", u2x, u2y))

    # Pin positions
    a0 = p(u2x, u2y, ADS_A0)
    a1 = p(u2x, u2y, ADS_A1)
    a2 = p(u2x, u2y, ADS_A2)
    a3 = p(u2x, u2y, ADS_A3)
    vdd = p(u2x, u2y, ADS_VDD)
    gnd = p(u2x, u2y, ADS_GND)
    sda = p(u2x, u2y, ADS_SDA)
    scl = p(u2x, u2y, ADS_SCL)
    addr = p(u2x, u2y, ADS_ADDR)

    # Probe net labels on A0-A2
    elements.append(label(a0[0] - 2.54, a0[1], "PROBE_PIT", 180))
    elements.append(wire(a0[0], a0[1], a0[0] - 2.54, a0[1]))
    elements.append(label(a1[0] - 2.54, a1[1], "PROBE_MEAT1", 180))
    elements.append(wire(a1[0], a1[1], a1[0] - 2.54, a1[1]))
    elements.append(label(a2[0] - 2.54, a2[1], "PROBE_MEAT2", 180))
    elements.append(wire(a2[0], a2[1], a2[0] - 2.54, a2[1]))

    # A3 - no connect
    elements.append(no_connect(a3[0], a3[1]))

    # VDD -> +3V3
    pwr33_ads = (vdd[0], vdd[1] - 5.08)
    elements.append(power_symbol("power:+3V3", pwr_ref(), "+3V3", *pwr33_ads))
    elements.append(wire(vdd[0], vdd[1], pwr33_ads[0], pwr33_ads[1]))

    # GND
    gnd_ads = (gnd[0], gnd[1] + 5.08)
    elements.append(power_symbol("power:GND", pwr_ref(), "GND", *gnd_ads))
    elements.append(wire(gnd[0], gnd[1], gnd_ads[0], gnd_ads[1]))

    # SDA and SCL net labels
    elements.append(label(sda[0] + 2.54, sda[1], "SDA", 0))
    elements.append(wire(sda[0], sda[1], sda[0] + 2.54, sda[1]))
    elements.append(label(scl[0] + 2.54, scl[1], "SCL", 0))
    elements.append(wire(scl[0], scl[1], scl[0] + 2.54, scl[1]))

    # ADDR -> GND
    gnd_addr = (addr[0] + 5.08, addr[1])
    elements.append(wire(addr[0], addr[1], gnd_addr[0], addr[1]))
    gnd_addr_sym = (gnd_addr[0], addr[1] + 5.08)
    elements.append(power_symbol("power:GND", pwr_ref(), "GND", *gnd_addr_sym))
    elements.append(wire(gnd_addr[0], addr[1], gnd_addr_sym[0], gnd_addr_sym[1]))

    # =====================================================================
    # SECTION 4: FAN DRIVER
    # =====================================================================

    q1x, q1y = Q1_POS
    elements.append(symbol_instance("bbq:IRLZ44N", "Q1", "IRLZ44N", q1x, q1y))
    gate = p(q1x, q1y, Q_GATE)
    drain = p(q1x, q1y, Q_DRAIN)
    source = p(q1x, q1y, Q_SOURCE)

    # R4 - Gate pulldown (vertical, beside gate)
    r4x, r4y = R4_POS
    elements.append(symbol_instance("Device:R", "R4", "10K", r4x, r4y))
    r4_top = p(r4x, r4y, R_PIN1)
    r4_bot = p(r4x, r4y, R_PIN2)

    # Wire gate to R4 top (horizontal)
    elements.append(wire(gate[0], gate[1], r4_top[0], gate[1]))
    elements.append(wire(r4_top[0], gate[1], r4_top[0], r4_top[1]))
    elements.append(junction(r4_top[0], gate[1]))

    # GND below R4
    gnd_r4 = (r4_bot[0], r4_bot[1] + 5.08)
    elements.append(power_symbol("power:GND", pwr_ref(), "GND", *gnd_r4))
    elements.append(wire(r4_bot[0], r4_bot[1], gnd_r4[0], gnd_r4[1]))

    # FAN_PWM label on gate
    elements.append(label(gate[0] - 10.16, gate[1], "FAN_PWM", 180))
    elements.append(wire(gate[0], gate[1], gate[0] - 10.16, gate[1]))

    # GND on Source
    gnd_src = (source[0], source[1] + 5.08)
    elements.append(power_symbol("power:GND", pwr_ref(), "GND", *gnd_src))
    elements.append(wire(source[0], source[1], gnd_src[0], gnd_src[1]))

    # J7 - Fan connector (2-pin)
    j7x, j7y = J7_POS
    elements.append(symbol_instance("Connector_Generic:Conn_01x02", "J7", "Fan", j7x, j7y))
    j7p1 = p(j7x, j7y, C2_P1)   # Fan+ (12V)
    j7p2 = p(j7x, j7y, C2_P2)   # Fan- (drain)

    # +12V on Fan+ pin
    pwr12v_fan = (j7p1[0] - 5.08, j7p1[1])
    elements.append(wire(j7p1[0], j7p1[1], pwr12v_fan[0], j7p1[1]))
    pwr12v_fan_sym = (pwr12v_fan[0], j7p1[1] - 5.08)
    elements.append(power_symbol("power:+12V", pwr_ref(), "+12V", *pwr12v_fan_sym))
    elements.append(wire(pwr12v_fan[0], j7p1[1], pwr12v_fan_sym[0], pwr12v_fan_sym[1]))

    # Wire Fan- to Drain
    elements.append(wire(j7p2[0], j7p2[1], drain[0], j7p2[1]))
    elements.append(wire(drain[0], j7p2[1], drain[0], drain[1]))

    # =====================================================================
    # SECTION 5: WT32 CONNECTOR + SERVO + BUZZER
    # =====================================================================

    # J5 - WT32-SC01 Plus 8-pin extension connector
    j5x, j5y = J5_POS
    elements.append(symbol_instance("Connector_Generic:Conn_01x08", "J5", "WT32-SC01", j5x, j5y))

    j5_pins = [p(j5x, j5y, off) for off in C8_PINS]
    # Pin 1: GPIO10 (SDA)
    # Pin 2: GPIO11 (SCL)
    # Pin 3: GPIO12 (FAN_PWM)
    # Pin 4: GPIO13 (SERVO_SIG)
    # Pin 5: GPIO14 (BUZZER_SIG)
    # Pin 6: 3V3
    # Pin 7: 5V
    # Pin 8: GND

    pin_labels = ["SDA", "SCL", "FAN_PWM", "SERVO_SIG", "BUZZER_SIG"]
    pin_gpios = ["GPIO10", "GPIO11", "GPIO12", "GPIO13", "GPIO14"]

    for idx in range(5):
        px, py = j5_pins[idx]
        # Net label to the left of pin
        lbl_x = px - 7.62
        elements.append(label(lbl_x, py, pin_labels[idx], 180))
        elements.append(wire(px, py, lbl_x, py))

    # Pin 6: +3V3
    px6, py6 = j5_pins[5]
    pwr33_j5 = (px6 - 7.62, py6)
    elements.append(wire(px6, py6, pwr33_j5[0], py6))
    pwr33_j5_sym = (pwr33_j5[0], py6 - 5.08)
    elements.append(power_symbol("power:+3V3", pwr_ref(), "+3V3", *pwr33_j5_sym))
    elements.append(wire(pwr33_j5[0], py6, pwr33_j5_sym[0], pwr33_j5_sym[1]))

    # Pin 7: +5V
    px7, py7 = j5_pins[6]
    pwr5v_j5 = (px7 - 7.62, py7)
    elements.append(wire(px7, py7, pwr5v_j5[0], py7))
    pwr5v_j5_sym = (pwr5v_j5[0], py7 - 5.08)
    elements.append(power_symbol("power:+5V", pwr_ref(), "+5V", *pwr5v_j5_sym))
    elements.append(wire(pwr5v_j5[0], py7, pwr5v_j5_sym[0], pwr5v_j5_sym[1]))

    # Pin 8: GND
    px8, py8 = j5_pins[7]
    gnd_j5 = (px8 - 7.62, py8)
    elements.append(wire(px8, py8, gnd_j5[0], py8))
    gnd_j5_sym = (gnd_j5[0], py8 + 5.08)
    elements.append(power_symbol("power:GND", pwr_ref(), "GND", *gnd_j5_sym))
    elements.append(wire(gnd_j5[0], py8, gnd_j5_sym[0], gnd_j5_sym[1]))

    # J6 - Servo connector (3-pin: Signal, VCC, GND)
    j6x, j6y = J6_POS
    elements.append(symbol_instance("Connector_Generic:Conn_01x03", "J6", "Servo", j6x, j6y))
    j6p1 = p(j6x, j6y, C3_P1)  # Signal
    j6p2 = p(j6x, j6y, C3_P2)  # VCC
    j6p3 = p(j6x, j6y, C3_P3)  # GND

    # SERVO_SIG label on pin 1
    elements.append(label(j6p1[0] - 7.62, j6p1[1], "SERVO_SIG", 180))
    elements.append(wire(j6p1[0], j6p1[1], j6p1[0] - 7.62, j6p1[1]))

    # +5V on pin 2
    pwr5v_servo = (j6p2[0] - 7.62, j6p2[1])
    elements.append(wire(j6p2[0], j6p2[1], pwr5v_servo[0], j6p2[1]))
    pwr5v_servo_sym = (pwr5v_servo[0], j6p2[1] - 5.08)
    elements.append(power_symbol("power:+5V", pwr_ref(), "+5V", *pwr5v_servo_sym))
    elements.append(wire(pwr5v_servo[0], j6p2[1], pwr5v_servo_sym[0], pwr5v_servo_sym[1]))

    # GND on pin 3
    gnd_servo = (j6p3[0] - 7.62, j6p3[1])
    elements.append(wire(j6p3[0], j6p3[1], gnd_servo[0], j6p3[1]))
    gnd_servo_sym = (gnd_servo[0], j6p3[1] + 5.08)
    elements.append(power_symbol("power:GND", pwr_ref(), "GND", *gnd_servo_sym))
    elements.append(wire(gnd_servo[0], j6p3[1], gnd_servo_sym[0], gnd_servo_sym[1]))

    # BZ1 - Buzzer
    bz1x, bz1y = BZ1_POS
    elements.append(symbol_instance("Device:Buzzer", "BZ1", "Buzzer", bz1x, bz1y))
    bz_plus = p(bz1x, bz1y, BZ_PIN1)   # + top
    bz_minus = p(bz1x, bz1y, BZ_PIN2)  # - bottom

    # BUZZER_SIG label on + pin
    elements.append(label(bz_plus[0], bz_plus[1] - 5.08, "BUZZER_SIG", 90))
    elements.append(wire(bz_plus[0], bz_plus[1], bz_plus[0], bz_plus[1] - 5.08))

    # GND on - pin
    gnd_bz = (bz_minus[0], bz_minus[1] + 5.08)
    elements.append(power_symbol("power:GND", pwr_ref(), "GND", *gnd_bz))
    elements.append(wire(bz_minus[0], bz_minus[1], gnd_bz[0], gnd_bz[1]))

    # =====================================================================
    # TEXT ANNOTATIONS for pin descriptions on J5
    # =====================================================================
    for idx, gpio in enumerate(pin_gpios):
        px, py = j5_pins[idx]
        elements.append(text_note(px + 2.54, py, gpio, 1.27))

    elements.append(text_note(j5_pins[5][0] + 2.54, j5_pins[5][1], "3V3", 1.27))
    elements.append(text_note(j5_pins[6][0] + 2.54, j5_pins[6][1], "5V", 1.27))
    elements.append(text_note(j5_pins[7][0] + 2.54, j5_pins[7][1], "GND", 1.27))

    # =====================================================================
    # Assemble output
    # =====================================================================
    parts.append('\n'.join(elements))
    parts.append(')')

    return '\n'.join(parts)


def main():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    output_path = os.path.join(script_dir, "bbq-controller.kicad_sch")

    content = generate()

    with open(output_path, 'w', newline='\n') as f:
        f.write(content)

    print(f"Generated: {output_path}")

    # Quick validation: check wire count and label pairs
    wire_count = content.count('(wire ')
    label_count = content.count('(label ')
    symbol_count = content.count('(symbol (lib_id')
    power_count = content.count('power:')

    print(f"  Wires: {wire_count}")
    print(f"  Labels: {label_count}")
    print(f"  Symbols: {symbol_count}")
    print(f"  Power symbols referenced: {power_count}")

    # Verify net label pairs
    import re
    labels = re.findall(r'\(label "([^"]+)"', content)
    from collections import Counter
    label_counts = Counter(labels)
    print("  Net label connections:")
    for name, count in sorted(label_counts.items()):
        status = "OK (paired)" if count == 2 else f"WARNING: {count} instances"
        print(f"    {name}: {status}")


if __name__ == "__main__":
    main()
