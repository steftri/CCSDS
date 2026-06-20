/*
  Create telemetry according to CCSDS standard

  This example code is in the public domain.

  http://www.trippler.de/stefan/arduino/ccsds/
*/

#include <ccsds_transferframe_tm.h>
#include <ccsds_spacepacket.h>


using namespace CCSDS;

const uint16_t SpacecraftID = 0x20B;
const uint16_t ApplicationID = 0x48;
uint8_t MasterChannelFrameCount = 0;
uint8_t VirtualChannelFrameCount = 0;
uint16_t Apid48SequenceCount = 0;
uint16_t IdleSpSequenceCount = 0;
uint8_t Data[4];




// the setup routine runs once when you press reset:
void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  memset(Data, 0, sizeof(Data));
}



void loop() {
  uint8_t TfBuffer[TF_SYNC_SIZE + CCSDS_TM_TF_TOTAL_SIZE] = {0};
  uint8_t SpBuffer[SpacePacket::MaxSize] = {0};
  
  unsigned long Millis;
  uint32_t SpPacketSize;

  TfBuffer[0] = 0x1a;
  TfBuffer[1] = 0xcf;
  TfBuffer[2] = 0xfc;
  TfBuffer[3] = 0x1d;
  
  // lets populate the telemetry data field
  Millis = millis();
  Data[0] = static_cast<uint8_t>(Millis >> 24);
  Data[1] = static_cast<uint8_t>(Millis >> 16);
  Data[2] = static_cast<uint8_t>(Millis >> 8);
  Data[3] = static_cast<uint8_t>(Millis);
  
  // a space packet must be created with the data to be transfered
  SpPacketSize = SpacePacket::create(SpBuffer, sizeof(SpBuffer),
                  ESpacePacketType::TM, ESpacePacketSequenceFlags::Unsegmented,
                  ApplicationID, Apid48SequenceCount++, Data, sizeof(Data));

  if(SpPacketSize == 0)
  {
    Serial.println("SpacePacket::create failed");
    delay(2000);
    return;
  }
  
  // a telemetry frame has always a fixed size, and we have to fill
  // up the frame with idle space packets
  SpPacketSize += SpacePacket::createIdle(SpBuffer + SpPacketSize,
                  sizeof(SpBuffer) - SpPacketSize,
                  IdleSpSequenceCount++,
                  sizeof(SpBuffer) - SpPacketSize);
  
  // now, we can create the transfer frame
  const uint32_t TfSize = TransferframeTm::create(&TfBuffer[TF_SYNC_SIZE], CCSDS_TM_TF_TOTAL_SIZE,
     SpacecraftID, 0, MasterChannelFrameCount++, VirtualChannelFrameCount++, 0,
     SpBuffer, SpPacketSize,
     0);

  if(TfSize == 0)
  {
    Serial.println("TransferframeTm::create failed");
    delay(2000);
    return;
  }

  
  // we finally transfer the frame via the serial interface
  // ATTENTION: this is an example, so we send it a bit more human readable
  //            instead of the raw data using Serial.write(Buffer, sizeof(Buffer));
  for(uint16_t i = 0; i < sizeof(TfBuffer); i++)
  {
    Serial.print(TfBuffer[i], HEX);
    Serial.print(' ');
  }
  Serial.println();
  
  delay(2000);
}

