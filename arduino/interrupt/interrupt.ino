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

volatile long lastPulseMessageTime = 0; // last time an incoming pulse message
                                        // was
// received
volatile long pulseInterval = 0;

// -------------- Interval managment properties
long fadeDelay  = 30;
byte brightness = 0;
#define MAX_INTERVAL_LENGTH_MS 2140
#define MIN_INTERVAL_LENGTH_MS 200
#define  MAX_INTERVALS 5

/*long intervals[MAX_INTERVALS];
   int  indexInterval = 0;*/
long lastInterval = 0;
int  fadeAmount   = 5;
boolean showPulse = false;
#define MIN_DELAY 5
#define MAX_DELAY 300

boolean standalone      = false;
volatile boolean inSync = false;

// ---------------- Setup
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

  if (myID == 0) {
    standalone = true;
  }

  toID = 0;
  toID = toID + !digitalRead(PinADDR2);
  toID = toID + 2 * !digitalRead(PinADDR3);
  workOutFadeDelay(MAX_INTERVAL_LENGTH_MS);
  lastPulseMessageTime = millis();
}

// ---------- Loop
void loop() {
  checkSend();

  if (!standalone) readLoop(); changeVisuals();
}

void checkSend() {
  if (gotPulse) {
    gotPulse = false;
    detectSync();

    if (standalone) {
      rememberInterval(pulseInterval);
    } else {
      debugCommsTx(true);
      sendMSG(myID, toID, 'P', pulseInterval);
      debugCommsTx(false);
    }
  }
}

void pulseISR() {
  long now = millis();

  pulseInterval        = now - lastPulseMessageTime;
  lastPulseMessageTime = now;
  gotPulse             = true;
  detectSync();
}

// --------------- Visual Methods
void changeVisuals() {
  checkLiveCount();

  if (showPulse) {
    debugVisuals(true);

    if (inSync) {
      setPixelColor(brightness, brightness, brightness);
    }
    else {
      setPixelColor(0,          brightness, brightness);
    }

    adjustBrightness();
  } else {
    debugVisuals(false);
    setPixelColor(80, 0, 0);
  }
  delay(fadeDelay);
}

// -------------- Interval managment methods

// MAX_LIVE_COUNT n cycles ((255*2)/5)*n
#define MAX_LIVE_COUNT 310
int liveCount = MAX_LIVE_COUNT;

/** rememberInterval records the interval between beats.
 * It gets called when a packet with an interval  arrives
 */
void rememberInterval(long interval) {
  showPulse    = true;
  liveCount    = MAX_LIVE_COUNT;
  lastInterval = interval;

  if (intervalInRange(interval)) {
    workOutFadeDelay(interval);
  }
}

void checkLiveCount() {
  if (liveCount <= 0) {
    showPulse = false;
    workOutFadeDelay(MAX_INTERVAL_LENGTH_MS);
    inSync = false;
    debugSync(false);
  } else {
    liveCount--;
  }
}

void workOutFadeDelay(long interval) {
  // fade levels are from 0 to 255 and back so the whole interval should be
  // divided into 255*2
  long delay = (interval * fadeAmount) / (255 * 2);

  if ((delay > MIN_DELAY) && (delay < MAX_DELAY)) {
    fadeDelay = delay;
  }
}

void adjustBrightness() {
  brightness = brightness + fadeAmount;
  if ((brightness == 0) || (brightness == 255)) {
    fadeAmount = -fadeAmount;
  }
}

#define SYNC_THRESHOLD_MS 100

void detectSync() {
  if (intervalInRange(lastInterval) &&
      intervalInRange(pulseInterval)) {
    if (abs(lastInterval - pulseInterval) < SYNC_THRESHOLD_MS) {
      inSync = true;
    } else {
      inSync = false;
    }
  }
  debugSync(inSync);
}

boolean intervalInRange(unsigned long i) {
  if ((MIN_INTERVAL_LENGTH_MS <= i) &&
      (i <= MAX_INTERVAL_LENGTH_MS)) {
    return true;
  }
  return false;
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

  /*debugVisuals(debugToggle);
     debugToggle = !debugToggle;*/
}

void debugVisuals(boolean on) {
  // digitalWrite(PinLED, on ? HIGH : LOW);
}

void debugSync(boolean on) {
  digitalWrite(PinLED, on ? HIGH : LOW);
}
