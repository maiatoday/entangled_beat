
#include <RS485_non_blocking.h>

#define RS485Control     17 // RS485 Direction control

#define RS485Transmit    HIGH
#define RS485Receive     LOW

// *********** Packet structure and receive variables
#define IDX_ADDR_SENDER    0
#define IDX_ADDR_RECEIVER  1
#define IDX_PAYLOAD        2
#define IDX_INTERVAL0      3
#define IDX_INTERVAL1      4
#define IDX_INTERVAL2      5
#define IDX_INTERVAL3      6

#define ADDR_BROADCAST 10

#define mybaud           9600

size_t fWrite(const byte what)
{
  return Serial.write(what);
}

int fAvailable()
{
  return Serial.available();
}

int fRead()
{
  return Serial.read();
}

RS485 myChannel(fRead,
                fAvailable,
                fWrite,
                20);

// ---------------- Comms Setup method
// == commsSetup is called from elsewhere
void commsSetup() {
  pinMode(RS485Control, OUTPUT);
  digitalWrite(RS485Control, RS485Receive);
  Serial.begin(mybaud);
  myChannel.begin();
}

// --------------- Comms TX methods
// === sendMSG is called from elsewhere
void sendMSG(byte address1, byte address2, byte data, long interval) {
  sendData(address1, address2, data, interval);
}

void sendData(byte address1, byte address2, byte data, long interval) {
  byte b0    = (byte)interval & 0xFF;
  byte b1    = (byte)((interval >> 8) & 0xFF);
  byte b2    = (byte)((interval >> 16) & 0xFF);
  byte b3    = (byte)((interval >> 24) & 0xFF);
  byte msg[] = { address1,                   // IDX_ADDR_SENDER
                 address2,                   // IDX_ADDR_RECEIVER
                 data,                       // IDX_PAYLOAD
                 b0,                         // IDX_INTERVAL0
                 b1,                         // IDX_INTERVAL1
                 b2,                         // IDX_INTERVAL2
                 b3                          // IDX_INTERVAL3
  };

  digitalWrite(RS485Control, RS485Transmit); // Enable RS485 Transmit
  delay(1);
  myChannel.sendMsg(msg, sizeof(msg));
  digitalWrite(RS485Control, RS485Receive);  // Disable RS485 Transmit
}

// ---------------- Comms Rx methods
// === readLoop is called from elsewhere and calls back with rememberInterval
void readLoop() {
  if (myChannel.update()) {
    debugCommsRx(false);

    // do we need to check the sender addr?
    const byte *data = myChannel.getData();
    byte address     = data[IDX_ADDR_RECEIVER];

    if ((address == myID) || (address == ADDR_BROADCAST)) {
      dealWithPayload(data[IDX_PAYLOAD],
                      bytesToLong(data[IDX_INTERVAL0],
                                  data[IDX_INTERVAL1],
                                  data[IDX_INTERVAL2],
                                  data[IDX_INTERVAL3]));

      // if necessary send an ack here
    } // packet is for me
  }
}

void dealWithPayload(byte payload, long interval) {
  if (payload == 'P') {
    // get interval from message
    rememberInterval(interval);
  }
}

long bytesToLong(byte b0, byte b1, byte b2, byte b3) {
  return (long)(b3 << 24) | (b2 << 16) | (b1 << 8) | b0;
}
