
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
#define IDX_INTERVAL0      4
#define IDX_INTERVAL1      5
#define IDX_INTERVAL2      6
#define IDX_INTERVAL3      7
#define IDX_CHECKSUM       8
#define MAX_PACKET_LENGTH  9
byte data[MAX_PACKET_LENGTH];

bool statePacketStarted = false;
#define DEBUG_PROTOCOL true

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
  Serial.println("_SerialRemote_"); // Can be ignored
  /* long encoding tests

  char c[5];
  longToChar(1L, c);
  Serial.println(c);
  if (charsToLong(c) == 1L) {
    Serial.println("1L ok");
  } else {
      Serial.println("1L error");
  }
  longToChar(22L, c);
  if (charsToLong(c) == 22L) {
    Serial.println("22L ok");
  } else {
      Serial.println("22L error");
  }
  Serial.println(c);
  longToChar(333L, c);
  if (charsToLong(c) == 333L) {
    Serial.println("333L ok");
  } else {
      Serial.println("333L error");
  }
  Serial.println(c);
  longToChar(5555L, c);
  if (charsToLong(c) == 5555L) {
    Serial.println("5555L ok");
  } else {
      Serial.println("5555L error");
  }
  Serial.println(c);
  longToChar(55555L, c);
  if (charsToLong(c) == 2140L) {
    Serial.println("2140L ok");
  } else {
      Serial.println("2140L error");
  }
  Serial.println(c);*/


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
    /*Serial.write(byte_receive);*/
    Serial.print(byte_receive, HEX);
    Serial.print(' ');
    if (DEBUG_PROTOCOL) {
    if (byte_receive == 00) {
        // 00 is a start of message so start collecting
          //  debugCommsRx(true);
          Serial.print("!");
        statePacketStarted = true;
        checksum           = 0;
        address            = 0;
        dataIndex          = 0;
        data[dataIndex]    = byte_receive; // we don't really need this
        dataIndex++;
      } else if (statePacketStarted && (dataIndex < MAX_PACKET_LENGTH)) {
        if (dataIndex < IDX_CHECKSUM) {
          // this assumes the checksum is last so only bytes before it
          // gets added to the checksum.
          checksum += byte_receive;
        }
        data[dataIndex] = byte_receive;
        dataIndex++;

        if (dataIndex == MAX_PACKET_LENGTH) {
          // we are at the last byte of the packet, get ready for the next one
          dataIndex          = 0;
          statePacketStarted = false;

          if (data[IDX_CHECKSUM] == (checksum&0xFF)) {
            /*debugCommsRx(false);*/

            // do we need to check the sender addr?
            //address = hex2addr(data[IDX_ADDR_RECEIVER]);

            //if ((address == myID) || (address == ADDR_BROADCAST)) {
              /*long data2 = bytesToLong(data[IDX_INTERVAL0], data[IDX_INTERVAL1],
                 data[IDX_INTERVAL2], data[IDX_INTERVAL3]);
                 dealWithPayload(data[IDX_PAYLOAD], data2);*/
              char b[5] =
              { data[IDX_INTERVAL0], data[IDX_INTERVAL1], data[IDX_INTERVAL2],
                data[IDX_INTERVAL3], 0 };

              unsigned long data2 = charsToLong(b);
              dealWithPayload(data[IDX_PAYLOAD], data2);

              /*dealWithPayload(data[IDX_PAYLOAD], 2140L);*/

              // if necessary send an ack here
          //  } // packet is for me
          }   else {
            Serial.println("!!bad checksum!! ");
            Serial.println(data[IDX_CHECKSUM], HEX);
            Serial.println(checksum&0xFF, HEX);
            }
        }     // end of packet
      }       // not start byte
    }
  }
} // --(end main loop )---

void dealWithPayload(byte payload, long data2) {
  if (payload == 'P') {
    char dd[5];
    longToChar(data2, dd);
    Serial.print(" payload ms: ");
    Serial.println(dd);
  }
}

void longToChar(unsigned long l, char *b) {
  if (l <= 9) {
    b[0] = ' ';
    b[1] = ' ';
    b[2] = ' ';
    ltoa(l,                      &b[3], 10);
    /*ltoa(1L,                      &b[3], 10);*/
  } else if (l <= 99) {
    b[0] = ' ';
    b[1] = ' ';
    ltoa(l,                      &b[2], 10);
    /*ltoa(22,                      &b[2], 10);*/
  } else if (l <= 999) {
    b[0] = ' ';
    ltoa(l,                      &b[1], 10);
    /*ltoa(333L,                      &b[1], 10);*/
  } else if (l <= 9999) {
     ltoa(l,                      b, 10);
      /*ltoa(4444L,                      b, 10);*/
  } else {
    ltoa(2140, b, 10);
  }
}

//** very very fragile !! wants a buffer of 5 bytes long!
unsigned long charsToLong(char *c) {
  char s[5] = { c[0], c[1], c[2], c[3], 0 };

  return atol(s);
  // return atol("2140");
}
