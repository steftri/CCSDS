/*
  Create Telemetry accoding to CCSDS standard

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
uint8_t Apid48SequenceCount = 0;
uint8_t IdleSpSequenceCount = 0;
unsigned char Data[4];


TransferframeTm tm(NULL);


// the setup routine runs once when you press reset:
void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  memset(Data, 0, sizeof(Data));
}



void loop() {
  uint8_t TfBuffer[TF_SYNC_SIZE+TM_TF_TOTAL_SIZE]="\x1a\xcf\xfc\x1d";
  uint8_t SpBuffer[SP_MAX_DATA_SIZE];
  
  unsigned long Millis;
  uint16_t SpPacketSize;
  
  // lets populate the telemetry data field
  Millis=millis();
  Data[0]=(unsigned char)(Millis>>24);
  Data[1]=(unsigned char)(Millis>>16);
  Data[2]=(unsigned char)(Millis>>8);
  Data[3]=(unsigned char)(Millis);
  
  // a space packet must be created with the data to be transfered
  SpPacketSize=SpacePacket::create(SpBuffer, SP_MAX_DATA_SIZE,
                  SpacePacket::TM, SpacePacket::Unsegmented, ApplicationID, Apid48SequenceCount++,
                  Data, sizeof(Data));
  
  // a telemetry frame has allways a fixed size, and we have to fill
  // up the frame with idle space packets
  SpPacketSize+=SpacePacket::createIdle(SpBuffer+SpPacketSize, SP_MAX_DATA_SIZE-SpPacketSize, IdleSpSequenceCount++, SP_MAX_DATA_SIZE-SpPacketSize);
  
  // now, we can create the transfer frame
  TransferframeTm::create(&TfBuffer[TF_SYNC_SIZE], TM_TF_TOTAL_SIZE,
     SpacecraftID, 0, MasterChannelFrameCount++, VirtualChannelFrameCount++, 0,
     SpBuffer, SpPacketSize,
     0);

  
  // we finally transfer the frame via the serial interface
  // ATTENTION: this is an example, so we send it a bit more human readable
  //            instead of the raw data using Serial.write(Buffer, sizeof(Buffer));
  for(unsigned int i=0; i<sizeof(TfBuffer); i++)
  {
    Serial.print(TfBuffer[i], HEX);
    Serial.print(' ');
  }
  Serial.println();
  
  delay(2000);
}

