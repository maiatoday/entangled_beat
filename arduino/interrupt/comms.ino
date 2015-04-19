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

/*#define IDX_CHECKSUM       4*/
/*#define MAX_PACKET_LENGTH  5*/
byte data[MAX_PACKET_LENGTH];
byte address;
byte byte_receive;
bool statePacketStarted = false;
byte dataIndex          = 0;
unsigned int checksum;

#define ADDR_BROADCAST 10

#define tNAK             15
#define tENQ             5
#define tACK             6

byte hex2addr(byte x) {
  byte result = x;

  if (x >= 48) {
    result = x - 48;
  }
  return result;
}

byte addr2hex(byte x) {
  byte result;

  result = 48 + x;
  return result;
}

// --------------- Comms TX methods
void sendMSG(byte address1, byte address2, byte data, unsigned long data2) {
  sendData(tENQ, addr2hex(address1), addr2hex(address2), data, data2);
}

void sendACK(byte address1, byte address2, byte data) {
  sendData(tACK, address1, address2, data, 0L);
}

void sendNAK(byte address1, byte address2, byte data) {
  sendData(tNAK, address1, address2, data, 0L);
}

void sendData(byte type, byte address1, byte address2, byte data, unsigned long data2) {
  unsigned int checksum_ACK;
  char b[5];

  longToChar(data2, b);

  // char test = '1';

  // checksum_ACK = address1 + address2 + data + test + test + test + test;
  checksum_ACK = address1 + address2 + data + b[0] + b[1] + b[2] + b[3];

  /*checksum_ACK = address1 + address2 + data ;*/

  /*for (int i = 0; i<4; i++) {
     checksum_ACK += b[i];
     }*/

  digitalWrite(RS485Control, RS485Transmit); // Enable RS485 Transmit

  delay(1);

  Serial.write(0);        // IDX_START_BYTE
  Serial.write(address1); // IDX_ADDR_SENDER
  Serial.write(address2); // IDX_ADDR_RECEIVER
  Serial.write(data);     // IDX_PAYLOAD

  /*Serial.write(b, 4);                       // IDX_INTERVAL0*/

  Serial.write(b[0]); // IDX_INTERVAL0
  Serial.write(b[1]); // IDX_INTERVAL1
  Serial.write(b[2]); // IDX_INTERVAL2
  Serial.write(b[3]); // IDX_INTERVAL3
  // Serial.write(test);                       // IDX_INTERVAL0
  // Serial.write(test);                       // IDX_INTERVAL1
  // Serial.write(test);                       // IDX_INTERVAL2
  // Serial.write(test);                       // IDX_INTERVAL3
  Serial.write(checksum_ACK & 0xFF);           // IDX_CHECKSUM
  Serial.flush();

  digitalWrite(RS485Control, RS485Receive); // Disable RS485 Transmit
}

// ---------------- Comms Rx methods
void readLoop() {
  // fixed length packets as defined at the top of the file.
  while (Serial.available() > 0) {
    byte_receive = Serial.read();

    if (byte_receive == 00) {
      // 00 is a start of message so start collecting
          debugCommsRx(true);
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
          debugCommsRx(false);

          // for the new pin setup we only check if the sender is not me
          address = hex2addr(data[IDX_ADDR_SENDER]);

          if ((address != myID) || (address == ADDR_BROADCAST)) {
            /*long data2 = bytesToLong(data[IDX_INTERVAL0], data[IDX_INTERVAL1],
               data[IDX_INTERVAL2], data[IDX_INTERVAL3]);
               dealWithPayload(data[IDX_PAYLOAD], data2);*/
            char b[5] =
            { data[IDX_INTERVAL0], data[IDX_INTERVAL1], data[IDX_INTERVAL2],
              data[IDX_INTERVAL3], 0 };

            unsigned long data2 = charsToLong(b);
            dealWithPayload(data[IDX_PAYLOAD], data2);

            // if necessary send an ack here
          } // packet is for me
        }   // checksum() ok
      }     // end of packet
    }       // not start byte
  } // while
}

void dealWithPayload(byte payload, long data2) {
  if (payload == 'P') {
    rememberInterval(data2);
  }
}

//** very very fragile !! wants a buffer of 5 bytes long!
void longToChar(unsigned long l, char *b) {
  if (l <= 9) {
    b[0] = ' ';
    b[1] = ' ';
    b[2] = ' ';
    ltoa(l,                      &b[3], 10);
  } else if (l <= 99) {
    b[0] = ' ';
    b[1] = ' ';
    ltoa(l,                      &b[2], 10);
  } else if (l <= 999) {
    b[0] = ' ';
    ltoa(l,                      &b[1], 10);
  } else if (l <= 9999) {
    ltoa(l,                      b, 10);
  } else {
    ltoa(MAX_INTERVAL_LENGTH_MS, b, 10);
  }
}

//** very very fragile !! wants a buffer of 5 bytes long!
unsigned long charsToLong(char *c) {
  char s[5] = { c[0], c[1], c[2], c[3], 0 };

  return atol(s);
  // return atol("2140");
}
