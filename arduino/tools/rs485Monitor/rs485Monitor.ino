
#include <SoftwareSerial.h>

#define SSerialRX        10 // Serial Receive pin
#define SSerialTX        11 // Serial Transmit pin

#define SSerialTxControl 3  // RS485 Direction control
#define RS485Transmit    HIGH
#define RS485Receive     LOW

#define Pin13LED         13

#define mybaud           9600

byte address;
byte myID;
unsigned int data_received;
byte byte_receive;
byte state    = 0;
byte trace_OK = 0;
unsigned int checksum;
unsigned int checksum_in_msg;

byte dataIndex = 0;

// /
// *********** Packet structure and receive variables
#define IDX_START_BYTE     0
#define IDX_ADDR_SENDER    1
#define IDX_ADDR_RECEIVER  2
#define IDX_PAYLOAD        3
#define IDX_CHECKSUM       4
#define MAX_PACKET_LENGTH  5
byte data[MAX_PACKET_LENGTH];

// /


SoftwareSerial RS485Serial(SSerialRX,
                           SSerialTX); // RX, TX

long lastSendTime = 0;
long hrDelay      = 900;


int byteReceived;
int byteSend;
int i;

void setup() /****** SETUP: RUNS ONCE ******/
{
  // Start the built-in serial port, probably to Serial Monitor
  Serial.begin(mybaud);
  Serial.println("SerialRemote"); // Can be ignored

  pinMode(Pin13LED,         OUTPUT);
  pinMode(SSerialTxControl, OUTPUT);

  digitalWrite(SSerialTxControl, RS485Receive); // Init Transceiver

  // Start the software serial port, to another device
  RS485Serial.begin(mybaud);                    // set the data rate
  i = 0;
} // --(end setup )---

void loop() /****** LOOP: RUNS CONSTANTLY ******/
{
  long now = millis();


  while (RS485Serial.available() > 0) {
    byte_receive = RS485Serial.read();
    Serial.write(byte_receive);

    if (byte_receive == 00) {
      state           = 1;
      checksum_in_msg = 0;
      checksum        = 0;
      trace_OK        = 0;
      address         = 0;
      data_received   = 0;
      dataIndex++;

      // Serial.println("Start packet");
    } else if ((state == 1) && (dataIndex < MAX_PACKET_LENGTH)) {
      data[dataIndex] = byte_receive;

      if (dataIndex < IDX_CHECKSUM) {
        checksum = checksum + byte_receive;
      }

      //    Serial.write('c');
      //    Serial.write(checksum);
      dataIndex++;

      if (dataIndex == MAX_PACKET_LENGTH) {
        // Serial.println("end packet");
        checksum_in_msg = data[IDX_CHECKSUM];
        state           = 0;
        dataIndex       = 0;

        // end of packet
        if (checksum_in_msg == checksum) {
          trace_OK = 1;
          Serial.println("Decode");
        } else {
          Serial.write('?');
          Serial.write(checksum);
          Serial.write(checksum_in_msg);
        }
      }
    }
  }
} // --(end main loop )---
