#include <PinChangeInt.h>

#define RS485Control     17 // RS485 Direction control

#define RS485Transmit    HIGH
#define RS485Receive     LOW

#define PinLED           13
#define mybaud           9600

// new pins
#define PinISR               7
#define ledB                 3
#define ledG                 5
#define ledR                 9
#define PinToADDR0           2
#define PinToADDR1           4

#define PinPowerSensor       8

byte myID;
byte toID;

volatile boolean gotPulse = false;

volatile long lastPulseTime = 0; // last time an incoming pulse message
// was
// received
volatile long pulseInterval = 0;
volatile long prevPulseInterval = 0;

// -------------- Interval managment properties

/*#define MAX_BRIGHTNESS 255
 #define FADE_AMOUNT (-5)*/
#define NONE_BRIGHTNESS 40
#define MAX_BRIGHTNESS 153
#define FADE_AMOUNT (-3)
long fadeDelay = 30;
#define MAX_INTERVAL_LENGTH_MS 2140
#define MIN_INTERVAL_LENGTH_MS 300

// #define  MAX_INTERVALS 5

/*long intervals[MAX_INTERVALS];
   int  indexInterval = 0;*/
long lastInterval = 0;
byte brightness   = MAX_BRIGHTNESS;
int  fadeAmount   = FADE_AMOUNT;
boolean showPulse = false;
#define MIN_DELAY 5
#define MAX_DELAY 300

boolean standalone      = false;
volatile boolean inSync = false;

//3 minutes =  seconds or 3000 / 30 to get the number of loops in 3 minutes about
#define MAX_SENSOR_WATCHDOG 6000
long sensorWatchDogPats = MAX_SENSOR_WATCHDOG;

// ---------------- Setup
void setup() {
  // setup Input pin and Interrupt
  pinMode(PinISR, INPUT_PULLUP);
  PCintPort::attachInterrupt(PinISR, pulseISR, CHANGE); // attach a PinChange
                                                        // Interrupt to our
                                                        // pin CHANGE means
                                                        // any level change
                                                        // triggers an
                                                        // interrupt

  gotPulse = false;

  // Switch on the heart rate sensor
  pinMode(PinPowerSensor, OUTPUT);
  digitalWrite(PinPowerSensor, HIGH);

  pinMode(PinLED,       OUTPUT);
  pinMode(RS485Control, OUTPUT);
  digitalWrite(RS485Control, RS485Receive);

  Serial.begin(mybaud);

  pinMode(ledB,       OUTPUT);
  pinMode(ledR,       OUTPUT);
  pinMode(ledG,       OUTPUT);

  pinMode(PinToADDR0, INPUT_PULLUP);
  pinMode(PinToADDR1, INPUT_PULLUP);

  toID = 0;
  toID = toID + !digitalRead(PinToADDR0);
  toID = toID + 2 * !digitalRead(PinToADDR1);

  myID = 0;
  myID = myID + !digitalRead(PinToADDR0);
  myID = myID + 2 * !digitalRead(PinToADDR1);

  if (myID == 0) {
    standalone = true;
  }

  workOutFadeDelay(MAX_INTERVAL_LENGTH_MS);
  lastPulseTime = millis();
}

/*
   void setupOldPins() {
   // setup Input pin and Interrupt
   pinMode(PinINPUT0, INPUT_PULLUP);
   PCintPort::attachInterrupt(PinINPUT0, pulseISR, CHANGE); // attach a
      PinChange
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
   lastPulseTime = millis();
   }
 */

// ---------- Loop
void loop() {
  checkSend();

  if (!standalone) readLoop();
  changeVisuals();
}

void checkSend() {
  if (gotPulse) {
    sensorWatchDogPats = MAX_SENSOR_WATCHDOG;
    gotPulse = false;
    detectSync();

    noFeedback();

    if (standalone) {
      rememberInterval(pulseInterval);
    } else {
      debugCommsTx(true);
      sendMSG(myID, toID, 'P', pulseInterval);
      debugCommsTx(false);
    }

  } else {
    sensorWatchDogPats--;
    if (sensorWatchDogPats <= 0) {
      resetSensor();
      sensorWatchDogPats = MAX_SENSOR_WATCHDOG;
    }
  }
}

void resetSensor() {
  debugWatchdog(true);
  digitalWrite(PinPowerSensor, LOW);
  delay(500);
  digitalWrite(PinPowerSensor, HIGH);
  debugWatchdog(false);
}

void pulseISR() {
  long now = millis();
  prevPulseInterval = pulseInterval;
  pulseInterval = now - lastPulseTime;
  lastPulseTime = now;
  gotPulse      = true;
  detectSync();
}

boolean goodData(long prevIn, long newIn) {
  if (abs(prevIn - newIn) < 50) {
    return true;
  }
  return false;
}

// --------------- Visual Methods
void changeVisuals() {
  checkLiveCount();

  if (showPulse) {
    debugVisuals(true);

    if (inSync) {
      setPixelColor(0, brightness, brightness);
    }
    else {
      setPixelColor(0, brightness, 0);
    }

    adjustBrightness();
  } else {
    debugVisuals(false);
    setPixelColor(NONE_BRIGHTNESS, 0, 0);
  }

  // debugFeedbackTime(fadeDelay);
  delay(fadeDelay);
}

// -------------- Interval managment methods

// MAX_LIVE_COUNT n cycles ((255*2)/5)*n
#define MAX_LIVE_COUNT 310
int liveCount = MAX_LIVE_COUNT;

/** rememberInterval records the interval between beats.
 * It gets called when a packet with an interval  arrives
 * or in standalone mode from the detected interval.
 */
void rememberInterval(long interval) {
  showPulse    = true;
  if (goodData(lastInterval, interval)) {
    liveCount = MAX_LIVE_COUNT;
  }
  lastInterval = interval;

  if (intervalInRange(interval)) {
    liveCount    = MAX_LIVE_COUNT;
  // this pulls the brightness to full when a pulls comes but makes the beat jarring
  // so comment out for now  brightness = MAX_BRIGHTNESS; //TODO maybe take this out
    workOutFadeDelay(interval);
  }
}

void checkLiveCount() {
  if (liveCount <= 0) {
    switchOff();
  } else {
    liveCount--;
  }
}

void switchOff() {
  showPulse = false;
  workOutFadeDelay(MAX_INTERVAL_LENGTH_MS);
  inSync     = false;
  brightness = MAX_BRIGHTNESS;
  fadeAmount = FADE_AMOUNT;
  debugSync(false);
}

void workOutFadeDelay(long interval) {
  // fade levels are from 0 to 255 and back so the whole interval should be
  // divided into 255*2
  long delay = (interval * fadeAmount) / (MAX_BRIGHTNESS * 2);

  if ((delay > MIN_DELAY) && (delay < MAX_DELAY)) {
    fadeDelay = delay;
  }
}

long brightTime = 0;
long lastBrightTime = 0;
long brightInterval = 0;
void adjustBrightness() {
  if (brightness == MAX_BRIGHTNESS) {
    lastBrightTime = brightTime;
    brightTime = millis();
    brightInterval = brightTime - lastBrightTime;
  }
  brightness = brightness + fadeAmount;

  if ((brightness == 0) || (brightness == MAX_BRIGHTNESS)) {
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

// -------------- feedback methods
#define PEAK_DIFF 20
long peakDiffTime         = 0;
long previousPeakDiffTime = 0;
boolean noFeedback() {
  boolean feedback = false;

  peakDiffTime = abs(lastPulseTime - brightTime);

  if (abs(previousPeakDiffTime - peakDiffTime) < PEAK_DIFF) {
    feedback = true;
  }
  previousPeakDiffTime = peakDiffTime;
  debugFeedback(feedback);
  debugFeedbackTime('P', pulseInterval);
  debugFeedbackTime('B', brightInterval);
  return feedback;
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
  // digitalWrite(PinLED, on ? HIGH : LOW);
}

void debugFeedback(boolean on) {
  //digitalWrite(PinLED, on ? HIGH : LOW);
}

void debugFeedbackTime(char c, unsigned long l) {
  /*if (standalone) {
    sendMSG(myID, toID, c, l);
  }*/
}

void debugWatchdog(boolean on) {
  digitalWrite(PinLED, on ? HIGH : LOW);
}
