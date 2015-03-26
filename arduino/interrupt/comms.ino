// *********** Packet structure and receive variables
#define IDX_START_BYTE     0
#define IDX_ADDR_SENDER    1
#define IDX_ADDR_RECEIVER  2
#define IDX_PAYLOAD        3
#define IDX_CHECKSUM       4
#define MAX_PACKET_LENGTH  5
byte data[MAX_PACKET_LENGTH];
byte address;
unsigned int data_received;
byte byte_receive;
byte state     = 0;
byte dataIndex = 0;
unsigned int checksum;

byte hex2addr(byte x) {
  byte result;

  if ((x >= 48) && (x <= 57)) {
    result = x - 48;
  }
  else if ((x >= 65) && (x <= 70)) {
    result = x - 55;
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

  checksum_ACK = address1 + address2 + data; // why the +3? and why send the 3
                                             // in the message

  digitalWrite(RS485Control, RS485Transmit); // Enable RS485 Transmit

  delay(1);

  Serial.write(0);        // IDX_START_BYTE
  Serial.write(address1); // IDX_ADDR_SENDER
  Serial.write(address2); // IDX_ADDR_RECEIVER
  Serial.write(data);     // IDX_PAYLOAD
  //  Serial.write(3); // why is this here?
  //  Needed if data was longer
  //  Serial.write(((checksum_ACK>>8)&255));
  Serial.write(((checksum_ACK) & 255));     // IDX_CHECKSUM
  Serial.flush();

  digitalWrite(RS485Control, RS485Receive); // Disable RS485 Transmit
}

// ---------------- Comms Rx methods
void readLoop() {
  // The message consists of 5 bytes
  // 00 senderAddr receiverAddr payload checksum
  while (Serial.available() > 0) {
    byte_receive = Serial.read();

    if (byte_receive == 00) {
      // 00 is a start of message so start collecting the rest
          debugCommsRx(true);
      state           = 1;
      checksum        = 0;
      address         = 0;
      data_received   = 0;
      dataIndex       = 0;
      data[dataIndex] = byte_receive; // we don't really need this
    } else if ((state == 1) && (dataIndex < MAX_PACKET_LENGTH)) {
      data[dataIndex] = byte_receive;
      checksum        = checksum + byte_receive;
      dataIndex++;

      if (dataIndex == MAX_PACKET_LENGTH) {
        // we are at the last byte of the packet
        dataIndex = 0;
        state     = 0;

        if (data[IDX_CHECKSUM] == checksum) {
          debugCommsRx(false);

          // do we need to check the sender addr?
          address = hex2addr(data[IDX_ADDR_RECEIVER]);

          if (address == myID) {
            if (data[IDX_PAYLOAD] == 'P') {
              // TODO measure the interval here
              stateDisplay    = HR;
              lastMessageTime = millis();
            } // only respond to 'P' messages
              // if necessary send an ack here
          }   // packet is for me
        }     // checksum ok
      }       // end of packet
    }         // not start byte
  }           // while
}
