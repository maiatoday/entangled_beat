#include <PinChangeInt.h>

#define RS485Control     17   //RS485 Direction control

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

long lastDebounceTime = 0;  // the last time the output pin was toggled
long debounceDelay = 200;    // the debounce time; increase if the output flickers

long displayUpdateTime = 0;
long displayDelay = 300;

boolean stateChange;

byte  data[3];
byte  address;
byte  myID;
byte  toID;
unsigned int data_received;
byte  byte_receive;
byte  state=0;
byte  cont1=1;
byte  trace_OK=0;
unsigned int checksum;
unsigned int checksum_trace;

enum stateDisplayType {
  notHR,
  HR
};

stateDisplayType stateDisplay;


void setPixelColor(byte r, byte g, byte b)
{
  analogWrite(ledR,r);
  analogWrite(ledG,g);
  analogWrite(ledB,b);
}

void setup() {
  //setup Input pin and Interrupt 
  pinMode(PinINPUT0,INPUT_PULLUP);
  PCintPort::attachInterrupt(PinINPUT0, change,CHANGE); // attach a PinChange Interrupt to our pin on the rising edge
  
  stateChange = false;
  stateDisplay = notHR;
  
  pinMode(PinLED,OUTPUT);
  pinMode(RS485Control,OUTPUT);
  digitalWrite(RS485Control,RS485Receive);

  Serial.begin(mybaud);
  
  pinMode(ledB,OUTPUT);
  pinMode(ledR,OUTPUT);
  pinMode(ledG,OUTPUT);
  
  pinMode(PinADDR0,INPUT_PULLUP);
  pinMode(PinADDR1,INPUT_PULLUP);
  pinMode(PinADDR2,INPUT_PULLUP);
  pinMode(PinADDR3,INPUT_PULLUP);
  
  //pinMode(PinINPUT0,INPUT_PULLUP);
  //pinMode(PinINPUT1,INPUT_PULLUP);
  //pinMode(PinINPUT2,INPUT_PULLUP);
  //pinMode(PinINPUT3,INPUT_PULLUP);

  myID = 0;
  myID = myID + !digitalRead(PinADDR0);
  myID = myID + 2 * !digitalRead(PinADDR1);
  
  toID = 0;
  toID = toID + !digitalRead(PinADDR2); 
  toID = toID + 2 * !digitalRead(PinADDR3);   
}

void display_loop() {
  
  //display state machine
  //in hr
  if (stateDisplay == HR) 
    if (stateChange){
      //digitalWrite(PinLED,HIGH);
      setPixelColor(255,255,255);
      displayUpdateTime = millis();
    } else {
      if ((millis() - displayUpdateTime) > displayDelay) {
        //digitalWrite(PinLED,LOW);     
        setPixelColor(0,0,0); 
      }
    }
  else
    //digitalWrite(PinLED,LOW);
    setPixelColor(0,0,0);
  //in not hr 
}

void readLoop() {
  
  while (Serial.available() > 0){
    byte_receive=Serial.read();
    
    if (byte_receive==00){
      digitalWrite(PinLED,!digitalRead(PinLED));
      state=1;
      checksum_trace=0;
      checksum=0;
      trace_OK=0;
      address=0;
      data_received=0;
      cont1=1;
    } else if (state==1 && cont1<=4){
      data[cont1-1]=byte_receive;
      checksum=checksum+byte_receive;
      cont1=cont1+1;
//   Needed if data was longer
//    } else if (state==1 && cont1==4){
//      checksum_trace=byte_receive<<8;
//      cont1=cont1+1;
    } else if (state==1 && cont1==5){
      checksum_trace=checksum_trace+byte_receive;
      cont1=cont1+1;
      state=0;   
      if (checksum_trace==checksum){
        trace_OK=1;
        if (data[0] == 49)
          address=10+hex2num(data[1]);
        else
          address=hex2num(data[1]);
        if (address==myID){
          if (data[2] == 'P'){
            //stateDisplay = HR;
            //sendMSG(49,toID+48,'D');
            
          } else
          //sendMSG(48,toID+48,'M');
          delay(1);
        } else
          //sendMSG(48,toID+48,'N');
          delay(1);
      }
    }
  }
}

void loop() {
  
  //Check if HR received in last 4 beats
  //if ((millis() - lastDebounceTime) > 4*debounceDelay) {
  //  stateDisplay = notHR;
  //}
  //change display
  //display_loop();
  
  //send and received data
  if (stateChange) {
    //sendBeat
    //Serial.println("Change");
    stateChange = false;
    sendMSG(48,toID+48,'P');
  } else {
    readLoop();
  }
}

void change(){
  //long now = millis();
  //if ((now - lastDebounceTime) > debounceDelay) {
    stateChange = true;
    //stateDisplay = HR;
  //  lastDebounceTime = now;
    
 // }
}

void sendMSG(byte address1,byte address2,byte data){
  sendData(tENQ,address1,address2,data); 
}

void sendACK(byte address1,byte address2,byte data){
  sendData(tACK,address1,address2,data);
}

void sendNAK(byte address1,byte address2,byte data){
  sendData(tNAK,address1,address2,data);
}

void sendData(byte type, byte address1,byte address2,byte data){

  unsigned int checksum_ACK;
  checksum_ACK=address1+address2+data+3;

  digitalWrite(RS485Control, RS485Transmit);  // Enable RS485 Transmit
  
  //digitalWrite(PinLED,HIGH);  // Disable RS485 Transmit   
  delay(1);

  Serial.write(0);
  Serial.write(address1);
  Serial.write(address2);
  Serial.write(data); 
  Serial.write(3);
//  Needed if data was longer
//  Serial.write(((checksum_ACK>>8)&255));
  Serial.write(((checksum_ACK)&255));
  Serial.flush();

  digitalWrite(RS485Control, RS485Receive);  // Disable RS485 Transmit
  //digitalWrite(PinLED,LOW);  // Disable RS485 Transmit    
}
