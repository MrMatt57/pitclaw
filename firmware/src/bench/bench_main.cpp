#include <Arduino.h>
#include <initializer_list>
#include <soc/soc_caps.h>
#include "config.h"
#include "temp_manager.h"
#include "servo_pwm.h"

static_assert(SERVO_PWM_RESOLUTION <= SOC_LEDC_TIMER_BIT_WIDE_NUM,
              "Servo PWM resolution exceeds this chip's LEDC capability");

// A serial-only bench program. Never use with a lit smoker.
// Outputs start off; each accepted output command expires automatically.
static TempManager probes;
// Separate parsers prevent partial commands from different ports combining.
struct ConsoleInput {
    char command[48] = {};
    size_t length = 0;
    bool overflow = false;
};
static ConsoleInput usbInput;
static ConsoleInput uartInput;

class BenchOutput : public Print {
public:
    using Print::write;
    size_t write(uint8_t value) override {
        Serial0.write(value);
        if (Serial) Serial.write(value);
        return 1;
    }
};
static BenchOutput console;
static uint32_t outputStarted = 0;
static uint32_t outputDuration = 0;
static uint32_t lastReport = 0;
static bool outputsReady = false;

static void off() {
    ledcWrite(FAN_PWM_CHANNEL, 0);
    ledcWrite(SERVO_PWM_CHANNEL, 0);
    ledcWrite(BUZZER_PWM_CHANNEL, 0);
    outputDuration = 0;
}
static const char* statusName(ProbeStatus status) {
    switch (status) {
        case ProbeStatus::OK: return "OK";
        case ProbeStatus::OPEN_CIRCUIT: return "OPEN";
        case ProbeStatus::SHORT_CIRCUIT: return "SHORT";
    }
    return "INVALID";
}
static void report() {
    for (uint8_t i = 0; i < NUM_PROBES; ++i) {
        int16_t raw = probes.getRawADC(i);
        console.printf("A%u raw=%d node=%.4fV status=%s", i, raw,
                      TempManager::adcToVoltage(raw), statusName(probes.getStatus(i)));
        if (probes.isConnected(i))
            console.printf(" R=%.0fohm C=%.2f F=%.2f", TempManager::adcToResistance(raw),
                          probes.getTempC(i), probes.getTemp(i));
        console.println();
    }
}
static void help() {
    console.println("BENCH ONLY: fan secured, servo horn/linkage removed; no live fire.");
    console.println("help | status | off | fan 0..100 | servo 1000..2000 | beep");
    console.println("fan: 3 seconds at 25kHz duty; 100 means constant full voltage.");
    console.println("servo: pulse width in us, 2 seconds, then pulses stop. Start with servo 1500.");
    console.println("beep: 2kHz for 500ms. Each output command first stops all outputs.");
    console.println("Status repeats every 2 seconds. Commands require Enter (newline).");
    console.println("Both USB CDC and DEBUG UART0 accept commands at 115200 baud.");
}
static void execute(const char* line) {
    if (!strcmp(line, "off")) { off(); console.println("All outputs OFF."); return; }
    if (!strcmp(line, "help")) { help(); return; }
    if (!strcmp(line, "status")) { report(); return; }
    if (!outputsReady) {
        off();
        console.println("PWM setup failed; output tests disabled.");
        return;
    }
    int value = 0;
    char extra = 0;
    if (sscanf(line, "fan %d %c", &value, &extra) == 1 && value >= 0 && value <= 100) {
        off();
        ledcWrite(FAN_PWM_CHANNEL, (value * 255 + 50) / 100);
        outputDuration = value ? 3000 : 0;
    } else if (sscanf(line, "servo %d %c", &value, &extra) == 1 && value >= 1000 && value <= 2000) {
        off();
        ledcWrite(SERVO_PWM_CHANNEL, servoPulseToDuty((uint16_t)value));
        outputDuration = 2000;
    } else if (!strcmp(line, "beep")) {
        off();
        ledcWriteTone(BUZZER_PWM_CHANNEL, ALARM_BUZZER_FREQ);
        outputDuration = 500;
    } else {
        off();
        console.println("Invalid command; all outputs OFF. Type help.");
        return;
    }
    outputStarted = millis();
    console.printf("Accepted: %s; automatic off in %lums.\n", line, (unsigned long)outputDuration);
}
void setup() {
    // Set low before peripherals initialize. External gate pulldown is still required.
    for (uint8_t pin : {PIN_FAN_PWM, PIN_SERVO, PIN_BUZZER}) {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, LOW);
    }
    pinMode(45, OUTPUT); // WT32 backlight off: this firmware has no display UI.
    digitalWrite(45, LOW);
    bool fanReady = ledcSetup(FAN_PWM_CHANNEL, FAN_PWM_FREQ, FAN_PWM_RESOLUTION) > 0;
    bool servoReady = ledcSetup(SERVO_PWM_CHANNEL, 50, SERVO_PWM_RESOLUTION) > 0;
    bool buzzerReady = ledcSetup(BUZZER_PWM_CHANNEL, ALARM_BUZZER_FREQ, 8) > 0;
    outputsReady = fanReady && servoReady && buzzerReady;
    if (outputsReady) {
        ledcAttachPin(PIN_FAN_PWM, FAN_PWM_CHANNEL);
        ledcAttachPin(PIN_SERVO, SERVO_PWM_CHANNEL);
        ledcAttachPin(PIN_BUZZER, BUZZER_PWM_CHANNEL);
    }
    off();
    Serial.begin(115200);
    Serial.setTxTimeoutMs(0); // A disconnected/stalled USB host must not delay tests.
    Serial0.begin(115200, SERIAL_8N1, 44, 43); // DEBUG RX=GPIO44, TX=GPIO43
    delay(500);
    if (!outputsReady) console.println("PWM setup failed; all output tests disabled.");
    bool adcReady = probes.begin();
    console.println(adcReady ? "ADC initialized at 0x48." :
        "ADC missing: inspect wiring and reboot. No valid probe measurements.");
    probes.setEMAAlpha(1.0f);
    help();
}
static void readConsole(Stream& port, ConsoleInput& input) {
    // Bound parsing work per loop so a stream of bytes cannot starve timed-off.
    unsigned budget = 64;
    while (budget-- && port.available()) {
        char c = (char)port.read();
        if (c == '\r') continue;
        if (c == '\n') {
            input.command[input.length] = '\0';
            if (input.overflow) { off(); console.println("Invalid/long input; outputs OFF."); }
            else if (input.length) execute(input.command);
            input.length = 0;
            input.overflow = false;
        } else if (c >= 32 && c <= 126 && input.length < sizeof(input.command) - 1 && !input.overflow) {
            input.command[input.length++] = c;
        } else { input.overflow = true; off(); }
    }
}
void loop() {
    if (outputDuration && (uint32_t)(millis() - outputStarted) >= outputDuration) {
        off();
        console.println("Timed test complete; all outputs OFF.");
    }
    probes.update();
    readConsole(Serial, usbInput);
    readConsole(Serial0, uartInput);
    if ((uint32_t)(millis() - lastReport) >= 2000) { lastReport = millis(); report(); }
    delay(2);
}
