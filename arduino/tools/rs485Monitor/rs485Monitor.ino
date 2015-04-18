
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
// 4 bytes go here
#define IDX_CHECKSUM       8
#define MAX_PACKET_LENGTH  9
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
  Serial.println("_SerialRemote_"); // Can be ignored
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
  Serial.println(c);


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

    /*if (byte_receive == 00) {
      state           = 1;
      checksum_in_msg = 0;
      checksum        = 0;
      dataIndex       = 0;
      trace_OK        = 0;
      address         = 0;
      data_received   = 0;
      data[dataIndex] = byte_receive;
      dataIndex++;

      // Serial.println("Start packet");
    } else if ((state == 1) && (dataIndex < MAX_PACKET_LENGTH)) {

      if (dataIndex < IDX_CHECKSUM) {
        checksum = checksum + byte_receive;
      }

      //    Serial.write('c');
      //    Serial.write(checksum);
      data[dataIndex] = byte_receive;
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
    }*/
  }
} // --(end main loop )---

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
