/*
  Leonardo HID: Throttle/Mixture/CarbHeat + Flaps + Trim Encoder (Production)
  Requires: Joystick library by MHeironimus (Library Manager)

  Axes:
    X = Throttle (A1)
    Y = Mixture  (A2)
    Z = CarbHeat (A0)

  Buttons:
    0 = Trim DOWN (pulsed)
    1 = Trim UP   (pulsed)
    2 = Trim push (held)
    3 = Flaps UP   (held)
    4 = Flaps DOWN (held)
*/

// ---------------- User options ----------------
const uint16_t TRIM_PULSE_MS   = 85; // length of each trim button pulse
const int8_t   ENC_FULL_STEP   = 2;  // fragments per detent (try 2 or 4 as needed)

// Set to 1 to invert an axis if your mechanics are reversed
const uint8_t INVERT_THR  = 0;
const uint8_t INVERT_MIX  = 0;
const uint8_t INVERT_CARB = 0;

// ---------------- Pin map ----------------
const uint8_t PIN_CARB_A = A0;
const uint8_t PIN_THR_A  = A1;
const uint8_t PIN_MIX_A  = A2;

const uint8_t PIN_FLAP_UP   = 6;  // SPDT momentary to GND when active
const uint8_t PIN_FLAP_DOWN = 7;

const uint8_t PIN_ENC_CLK = 2;    // encoder A
const uint8_t PIN_ENC_DT  = 3;    // encoder B
const uint8_t PIN_ENC_SW  = 4;    // encoder push (to GND when pressed)

#include <Joystick.h>
// Joystick descriptor (15-arg constructor)
Joystick_ Joystick(
  JOYSTICK_DEFAULT_REPORT_ID,
  JOYSTICK_TYPE_JOYSTICK,
  5, 0,               // 5 buttons, 0 hats
  true, true, true,   // X,Y,Z axes
  false,false,false,  // Rx,Ry,Rz
  false,false,false,  // Rudder,Throttle,Accelerator
  false,false         // Brake,Steering
);

// Trim buttons
const uint8_t BTN_TRIM_DOWN = 0;
const uint8_t BTN_TRIM_UP   = 1;
const uint8_t BTN_TRIM_PUSH = 2;

// Flaps buttons
const uint8_t BTN_FLAPS_UP   = 3;
const uint8_t BTN_FLAPS_DOWN = 4;

// Robust quadrature decoder
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

void setup() {
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
  } else if (encAcc <= -ENC_FULL_STEP) {
    encAcc = 0;
    Joystick.setButton(BTN_TRIM_DOWN, 1);
    trimPulseEndDown = now + TRIM_PULSE_MS;
  }

  // Release pulses
  if (trimPulseEndUp && now >= trimPulseEndUp)   { Joystick.setButton(BTN_TRIM_UP, 0);   trimPulseEndUp=0; }
  if (trimPulseEndDown && now >= trimPulseEndDown){ Joystick.setButton(BTN_TRIM_DOWN, 0); trimPulseEndDown=0; }

  // Encoder push (held)
  Joystick.setButton(BTN_TRIM_PUSH, digitalRead(PIN_ENC_SW)==LOW);

  // -------- Flaps (held) --------
  Joystick.setButton(BTN_FLAPS_UP,   digitalRead(PIN_FLAP_UP)==LOW);
  Joystick.setButton(BTN_FLAPS_DOWN, digitalRead(PIN_FLAP_DOWN)==LOW);

  // Keep loop responsive & USB happy
  delay(2);
}
