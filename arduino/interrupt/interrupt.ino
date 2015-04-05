#include <PinChangeInt.h>

#define RS485Control     17 // RS485 Direction control

#define RS485Transmit    HIGH
#define RS485Receive     LOW

#define PinLED           13

#define PinADDR0         2
#define PinADDR1         3
#define PinADDR2         4
#define PinADDR3         5

#define PinINPUT3        6
#define PinINPUT2        7
#define PinINPUT1        8
#define PinINPUT0        9

#define ledB             10
#define ledG             11
#define ledR             12

#define mybaud           9600

byte myID;
byte toID;

volatile boolean gotPulse = false;

// -------------- Interval managment properties
long fadeDelay  = 30;
byte brightness = 0;

void setup() {
  // setup Input pin and Interrupt
  pinMode(PinINPUT0, INPUT_PULLUP);
  PCintPort::attachInterrupt(PinINPUT0, pulseISR, CHANGE); // attach a PinChange
                                                           // Interrupt to our
                                                           // pin CHANGE means
                                                           // any level change
                                                           // triggers an
                                                           // interrupt

  gotPulse = false;

  pinMode(PinLED,       OUTPUT);
  pinMode(RS485Control, OUTPUT);
  digitalWrite(RS485Control, RS485Receive);

  Serial.begin(mybaud);

  pinMode(ledB,     OUTPUT);
  pinMode(ledR,     OUTPUT);
  pinMode(ledG,     OUTPUT);

  pinMode(PinADDR0, INPUT_PULLUP);
  pinMode(PinADDR1, INPUT_PULLUP);
  pinMode(PinADDR2, INPUT_PULLUP);
  pinMode(PinADDR3, INPUT_PULLUP);

  myID = 0;
  myID = myID + !digitalRead(PinADDR0);
  myID = myID + 2 * !digitalRead(PinADDR1);

  toID = 0;
  toID = toID + !digitalRead(PinADDR2);
  toID = toID + 2 * !digitalRead(PinADDR3);
}

// ---------- main loop
void loop() {
  checkSend();
  readLoop();
  changeVisuals();
}

void checkSend() {
  if (gotPulse) {
    debugCommsTx(true);
    gotPulse = false;
    sendMSG(myID, toID, 'P');
    debugCommsTx(false);
  }
}

void pulseISR() {
  gotPulse = true;
}

// --------------- Visual Methods
void changeVisuals() {
  debugVisuals(true);
  setPixelColor(0, 0, brightness);
  adjustBrightness();
  delay(fadeDelay);
  debugVisuals(false);
}

// -------------- Interval managment methods
#define MAX_INTERVAL_LENGTH_MS 2140
#define MIN_INTERVAL_LENGTH_MS 200
#define  MAX_INTERVALS 5
long intervals[MAX_INTERVALS];
int  indexInterval = 0;
long lastInterval  = 0;
int  fadeAmount    = 5;

void rememberInterval(long interval) {
  debugToggleVisuals();

  // remember the last MAX_INTERVALS intervals between pulses
  lastInterval             = interval;
  intervals[indexInterval] = interval;
  indexInterval++;

  if (indexInterval == MAX_INTERVALS) {
    indexInterval = 0;
  }
}

void workOutFadeDelay(long interval) {
  // fade levels are from 0 to 255 and back so the whole interval should be
  // divided into 255*2

  fadeDelay = (interval * fadeAmount) / (255 * 2);
}

void adjustBrightness() {
  brightness = brightness + fadeAmount;

  if ((brightness == 0) || (brightness == 255)) {
    fadeAmount = -fadeAmount;
  }
}

// -------------- Debug methods

bool debugToggle = false;
void debugPulse(boolean on) {
  //  digitalWrite(PinLED,on?HIGH:LOW);
}

void debugCommsTx(boolean on) {
  // digitalWrite(PinLED, on ? HIGH : LOW);
}

void debugCommsRx(boolean on) {
  // digitalWrite(PinLED, on ? HIGH : LOW);
}

void debugToggleVisuals() {
  // if (debugToggle) {
  //   setPixelColor(255, 0,   0);
  // } else {
  //   setPixelColor(0,   255, 255);
  // }
  debugVisuals(debugToggle);
  debugToggle = !debugToggle;
}

void debugVisuals(boolean on) {
  digitalWrite(PinLED, on ? HIGH : LOW);
}
