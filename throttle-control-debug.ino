/*
  Leonardo HID: Throttle/Mixture/CarbHeat + Flaps + Trim Encoder (DEBUG)
  - Adds Serial prints for: axes, trim pulses (UP/DOWN), encoder direction,
    encoder press, and flaps UP/DOWN.
*/

#include <Joystick.h>

// ---------------- User options ----------------
const uint16_t TRIM_PULSE_MS   = 85; // length of each trim button pulse
const int8_t   ENC_FULL_STEP   = 2;  // fragments per detent (try 2 or 4 as needed)

// Set to 1 to invert an axis if your mechanics are reversed
const uint8_t INVERT_THR  = 0;
const uint8_t INVERT_MIX  = 0;
const uint8_t INVERT_CARB = 0;

// Debug print rate (ms)
const uint16_t DEBUG_PRINT_MS = 250;

// ---------------- Pin map ----------------
const uint8_t PIN_CARB_A = A0;
const uint8_t PIN_THR_A  = A1;
const uint8_t PIN_MIX_A  = A2;

const uint8_t PIN_FLAP_UP   = 6;  // SPDT momentary to GND when active
const uint8_t PIN_FLAP_DOWN = 7;

const uint8_t PIN_ENC_CLK = 2;    // encoder A
const uint8_t PIN_ENC_DT  = 3;    // encoder B
const uint8_t PIN_ENC_SW  = 4;    // encoder push (to GND when pressed)

// ---------------- HID descriptor ----------------
Joystick_ Joystick(
  JOYSTICK_DEFAULT_REPORT_ID,
  JOYSTICK_TYPE_JOYSTICK,
  5, 0,               // 5 buttons, 0 hats
  true, true, true,   // X,Y,Z axes
  false,false,false,  // Rx,Ry,Rz
  false,false,false,  // Rudder,Throttle,Accelerator
  false,false         // Brake,Steering
);

// Button indices
const uint8_t BTN_TRIM_DOWN = 0;
const uint8_t BTN_TRIM_UP   = 1;
const uint8_t BTN_TRIM_PUSH = 2;
const uint8_t BTN_FLAPS_UP  = 3;
const uint8_t BTN_FLAPS_DOWN= 4;

// ---------------- Robust quadrature decoder ----------------
static uint8_t encPrev = 0;    // previous 2-bit state
static int8_t  encAcc  = 0;    // accumulate +/- fragments
const int8_t encTrans[16] = {
  0, -1, +1,  0,
 +1,  0,  0, -1,
 -1,  0,  0, +1,
  0, +1, -1,  0
};

uint32_t trimPulseEndUp   = 0;
uint32_t trimPulseEndDown = 0;

// Debug counters/state
long dbgTrimUpCount = 0, dbgTrimDownCount = 0;
uint8_t lastSW = HIGH;
uint8_t lastFlapsUp = HIGH, lastFlapsDown = HIGH;
uint32_t lastPrint = 0;

void setup() {
  Serial.begin(115200);
  delay(50);
  Serial.println(F("=== C150 Throttle+Trim (DEBUG) ==="));
  Serial.println(F("Pins: THR=A1  MIX=A2  CARB=A0  ENC: CLK=D2 DT=D3 SW=D4  FLAPS: UP=D6 DOWN=D7"));
  Serial.print(F("ENC_FULL_STEP=")); Serial.print(ENC_FULL_STEP);
  Serial.print(F("  TRIM_PULSE_MS=")); Serial.println(TRIM_PULSE_MS);

  // Digital inputs
  pinMode(PIN_FLAP_UP,   INPUT_PULLUP);
  pinMode(PIN_FLAP_DOWN, INPUT_PULLUP);

  pinMode(PIN_ENC_CLK, INPUT_PULLUP);
  pinMode(PIN_ENC_DT,  INPUT_PULLUP);
  pinMode(PIN_ENC_SW,  INPUT_PULLUP);

  // Analog inputs
  pinMode(PIN_CARB_A, INPUT);
  pinMode(PIN_THR_A,  INPUT);
  pinMode(PIN_MIX_A,  INPUT);

  // Seed encoder state
  encPrev = ((digitalRead(PIN_ENC_CLK) ? 1 : 0) << 1) |
            (digitalRead(PIN_ENC_DT)  ? 1 : 0);

  // HID ranges & start
  Joystick.setXAxisRange(0,1023);
  Joystick.setYAxisRange(0,1023);
  Joystick.setZAxisRange(0,1023);
  Joystick.begin();
}

void loop() {
  const uint32_t now = millis();

  // -------- Axes --------
  int thr  = analogRead(PIN_THR_A);
  int mix  = analogRead(PIN_MIX_A);
  int carb = analogRead(PIN_CARB_A);

  if (INVERT_THR)  thr  = 1023 - thr;
  if (INVERT_MIX)  mix  = 1023 - mix;
  if (INVERT_CARB) carb = 1023 - carb;

  Joystick.setXAxis(thr);
  Joystick.setYAxis(mix);
  Joystick.setZAxis(carb);

  // -------- Encoder (robust quadrature) --------
  uint8_t a = digitalRead(PIN_ENC_CLK) ? 1 : 0;
  uint8_t b = digitalRead(PIN_ENC_DT)  ? 1 : 0;
  uint8_t curr = (a << 1) | b;

  encAcc += encTrans[(encPrev << 2) | curr];
  encPrev = curr;

  if (encAcc >= ENC_FULL_STEP) {
    encAcc = 0;
    Joystick.setButton(BTN_TRIM_UP, 1);
    trimPulseEndUp = now + TRIM_PULSE_MS;
    dbgTrimUpCount++;
    Serial.println(F("TRIM: UP pulse"));
  } else if (encAcc <= -ENC_FULL_STEP) {
    encAcc = 0;
    Joystick.setButton(BTN_TRIM_DOWN, 1);
    trimPulseEndDown = now + TRIM_PULSE_MS;
    dbgTrimDownCount++;
    Serial.println(F("TRIM: DOWN pulse"));
  }

  // Release pulses
  if (trimPulseEndUp && now >= trimPulseEndUp)   { Joystick.setButton(BTN_TRIM_UP, 0);   trimPulseEndUp=0; }
  if (trimPulseEndDown && now >= trimPulseEndDown){ Joystick.setButton(BTN_TRIM_DOWN, 0); trimPulseEndDown=0; }

  // Encoder push (held)
  uint8_t sw = digitalRead(PIN_ENC_SW);
  Joystick.setButton(BTN_TRIM_PUSH, sw == LOW);
  if (sw != lastSW) {
    Serial.print(F("ENC SW: ")); Serial.println(sw==LOW ? F("PRESSED") : F("RELEASED"));
    lastSW = sw;
  }

  // -------- Flaps (held) --------
  uint8_t flUp = digitalRead(PIN_FLAP_UP);
  uint8_t flDn = digitalRead(PIN_FLAP_DOWN);
  Joystick.setButton(BTN_FLAPS_UP,   flUp==LOW);
  Joystick.setButton(BTN_FLAPS_DOWN, flDn==LOW);

  if (flUp != lastFlapsUp) {
    Serial.print(F("FLAPS: ")); Serial.println(flUp==LOW ? F("UP (active)") : F("UP (released)"));
    lastFlapsUp = flUp;
  }
  if (flDn != lastFlapsDown) {
    Serial.print(F("FLAPS: ")); Serial.println(flDn==LOW ? F("DOWN (active)") : F("DOWN (released)"));
    lastFlapsDown = flDn;
  }

  // -------- Periodic status --------
  if (now - lastPrint >= DEBUG_PRINT_MS) {
    Serial.print(F("AXES  Thr:"));  Serial.print(thr);
    Serial.print(F("  Mix:"));      Serial.print(mix);
    Serial.print(F("  Carb:"));     Serial.print(carb);

    Serial.print(F("  | Flaps: "));
    if (flUp==LOW)      Serial.print(F("UP"));
    else if (flDn==LOW) Serial.print(F("DOWN"));
    else                Serial.print(F("Neutral"));

    Serial.print(F("  | TrimCnt U:")); Serial.print(dbgTrimUpCount);
    Serial.print(F(" D:"));             Serial.print(dbgTrimDownCount);
    Serial.println();

    lastPrint = now;
  }

  // Keep loop responsive & USB happy
  delay(2);
}