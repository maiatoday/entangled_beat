
#include <SoftwareSerial.h>

#define SSerialRX        10  //Serial Receive pin
#define SSerialTX        11  //Serial Transmit pin

#define SSerialTxControl 3   //RS485 Direction control
#define RS485Transmit    HIGH
#define RS485Receive     LOW

#define Pin13LED         13

#define mybaud           9600

byte  data[3];
byte  address;
byte  myID;
unsigned int data_received;
byte  byte_receive;
byte  state=0;
byte  cont1=1;
byte  trace_OK=0;
unsigned int checksum;
unsigned int checksum_trace;




SoftwareSerial RS485Serial(SSerialRX, SSerialTX); // RX, TX

long lastSendTime = 0;  
long hrDelay = 900;    



int byteReceived;
int byteSend;
int i;

void setup()   /****** SETUP: RUNS ONCE ******/
{
  // Start the built-in serial port, probably to Serial Monitor
  Serial.begin(mybaud);
  Serial.println("SerialRemote");  // Can be ignored
  
  pinMode(Pin13LED, OUTPUT);   
  pinMode(SSerialTxControl, OUTPUT);  
  
  digitalWrite(SSerialTxControl, RS485Receive);  // Init Transceiver
  
  // Start the software serial port, to another device
  RS485Serial.begin(mybaud);   // set the data rate
  i = 0;
}//--(end setup )---



void loop()   /****** LOOP: RUNS CONSTANTLY ******/
{
  long now = millis();


  
  while (RS485Serial.available() > 0){
    byte_receive=RS485Serial.read();
    Serial.write(byte_receive);
    if (byte_receive==00){
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
    } else if (state==1 && cont1==5){
      checksum_trace=checksum_trace+byte_receive;
      cont1=cont1+1;
      state=0;   
      if (checksum_trace==checksum){
        trace_OK=1;
        Serial.println("Decode");
      } else
        Serial.println("Junk");
    }
  }

  
}//--(end main loop )---

