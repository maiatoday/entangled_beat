// *********** Packet structure and receive variables
#define IDX_START_BYTE     0
#define IDX_ADDR_SENDER    1
#define IDX_ADDR_RECEIVER  2
#define IDX_PAYLOAD        3
#define IDX_CHECKSUM       4
#define MAX_PACKET_LENGTH  5
byte data[MAX_PACKET_LENGTH];
byte address;
byte byte_receive;
bool statePacketStarted = false;
byte dataIndex          = 0;
unsigned int checksum;

#define ADDR_BROADCAST 10

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
void sendMSG(byte address1, byte address2, byte data) {
  sendData(tENQ, addr2hex(address1), addr2hex(address2), data);
}

void sendACK(byte address1, byte address2, byte data) {
  sendData(tACK, address1, address2, data);
}

void sendNAK(byte address1, byte address2, byte data) {
  sendData(tNAK, address1, address2, data);
}

void sendData(byte type, byte address1, byte address2, byte data) {
  unsigned int checksum_ACK;

  checksum_ACK = address1 + address2 + data;

  digitalWrite(RS485Control, RS485Transmit); // Enable RS485 Transmit

  delay(1);

  Serial.write(0);                          // IDX_START_BYTE
  Serial.write(address1);                   // IDX_ADDR_SENDER
  Serial.write(address2);                   // IDX_ADDR_RECEIVER
  Serial.write(data);                       // IDX_PAYLOAD
  Serial.write(((checksum_ACK) & 255));     // IDX_CHECKSUM
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

        if (data[IDX_CHECKSUM] == checksum) {
          debugCommsRx(false);

          // do we need to check the sender addr?
          address = hex2addr(data[IDX_ADDR_RECEIVER]);

          if ((address == myID) || (address == ADDR_BROADCAST)) {
            dealWithPayload(data[IDX_PAYLOAD]);

            // if necessary send an ack here
          } // packet is for me
        }   // checksum ok
      }     // end of packet
    }       // not start byte
  }                            // while
}

long lastPulseMessageTime = 0; // last time an incoming pulse message was
                               // received
void dealWithPayload(byte payload) {
  if (payload == 'P') {
    long now = millis();
    rememberInterval(lastPulseMessageTime - now);
    lastPulseMessageTime = now;
  }
}
