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

#define tNAK             15
#define tENQ             5
#define tACK             6

#define ledB             10
#define ledG             11
#define ledR             12

#define numPixels        1
#define wait             80

#define mybaud           9600

byte myID;
byte toID;

long lastDebounceTime = 0;   // the last time the output pin was toggled
long debounceDelay    = 200; // the debounce time; increase if the output
                             // flickers

long lastMessageTime = 0;    // last time an incoming message was received
long displayDelay    = 300;

volatile boolean stateChange;

// ************ display variables
enum stateDisplayType {
  notHR,
  HR
};

stateDisplayType stateDisplay;

void setup() {
  // setup Input pin and Interrupt
  pinMode(PinINPUT0, INPUT_PULLUP);
  PCintPort::attachInterrupt(PinINPUT0, pulseISR, CHANGE); // attach a PinChange
                                                           // Interrupt to our
                                                           // pin CHANGE means
                                                           // any level change
                                                           // triggers an
                                                           // interrupt

  stateChange  = false;
  stateDisplay = notHR;

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
  if (stateChange) {
    debugCommsTx(true);
    stateChange = false;
    sendMSG(myID, toID, 'P');
    debugCommsTx(false);
  }
}

void pulseISR() {
  stateChange = true;
}

// --------------- Visual Methods
void changeVisuals() {
  // Check if HR received in last 4 beats
  if ((millis() - lastMessageTime) > 4 * debounceDelay) {
    stateDisplay = notHR;
  } else {
    debugVisuals(true);
    stateDisplay = HR;

    /* mg
       //display state machine
       //in hr
       if (stateDisplay == HR)
       if (stateChange){
        //digitalWrite(PinLED,HIGH);
        setPixelColor(255,255,255);
         = millis();
       } else {
        if ((millis() - ) > displayDelay) {
          //digitalWrite(PinLED,LOW);
          setPixelColor(0,0,0);
        }
       }
       else
       //digitalWrite(PinLED,LOW);
       setPixelColor(0,0,0);
       //in not hr
     */
    debugVisuals(false);
  }
}

// -------------- Debug methods
void debugPulse(boolean on) {
  //  digitalWrite(PinLED,on?HIGH:LOW);
}

void debugCommsTx(boolean on) {
  // digitalWrite(PinLED, on ? HIGH : LOW);
}

void debugCommsRx(boolean on) {
  digitalWrite(PinLED, on ? HIGH : LOW);
}

void debugVisuals(boolean on) {
  //    digitalWrite(PinLED,on?HIGH:LOW);
}
