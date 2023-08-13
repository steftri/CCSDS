/*
  Create Telemetry accoding to CCSDS standard

  This example code is in the public domain.

  http://www.trippler.de/stefan/arduino/ccsds/
*/


#include <ccsds_cltu.h>
#include <ccsds_transferframe_tc.h>
#include <ccsds_spacepacket.h>


using namespace CCSDS;


void StartOfTransmissionCallback(void *p_Context);
void CltuCallback(void *p_Context, const uint8_t *pu8_Data, const uint16_t u16_DataSize);
void TcCallback(void *p_Context, const bool b_BypassFlag, const bool b_CtrlCmdFlag,
                const uint16_t u16_SpacecraftID, const uint8_t u8_VirtualChannelID,
                const uint8_t u8_FrameSeqNumber,
                const uint8_t *pu8_Data, const uint16_t u16_DataSize);
void SpCallback(void *p_Context, const SpacePacket::PacketType e_PacketType, const SpacePacket::SequenceFlags e_SequenceFlags, const uint16_t u16_APID, const uint16_t u16_SequenceCount, const bool b_SecHeader,
                const uint8_t *pu8_Data, const uint16_t u16_DataSize);


Cltu g_Cltu(NULL, &StartOfTransmissionCallback, NULL, &CltuCallback);
TransferframeTc g_Tc(NULL, &TcCallback);
SpacePacket g_Sp(NULL, &SpCallback);



void StartOfTransmissionCallback(void *p_Context)
{
  Serial.println("SOT");
  g_Tc.setSync();
}


void CltuCallback(void *p_Context, const uint8_t *pu8_Data, const uint16_t u16_DataSize)
{
  Serial.print("CLTU DATA: ");
  for(uint16_t i=0; i<u16_DataSize; i++)
  {
    Serial.print((uint32_t)pu8_Data[i], HEX);
    Serial.print(' ');
  }
  Serial.println();
  g_Tc.process((uint8_t*)pu8_Data, u16_DataSize);
}


void TcCallback(void *p_Context, const bool b_BypassFlag, const bool b_CtrlCmdFlag,
                const uint16_t u16_SpacecraftID, const uint8_t u8_VirtualChannelID,
                const uint8_t u8_FrameSeqNumber,
                const uint8_t *pu8_Data, const uint16_t u16_DataSize)
{
  Serial.print("TC DATA: ");
  for(uint16_t i=0; i<u16_DataSize; i++)
  {
    Serial.print((uint32_t)pu8_Data[i], HEX);
    Serial.print(' ');
  }
  Serial.println();
  g_Sp.process(pu8_Data, u16_DataSize);
}



void SpCallback(void *p_Context, const SpacePacket::PacketType e_PacketType, const SpacePacket::SequenceFlags e_SequenceFlags, const uint16_t u16_APID, const uint16_t u16_SequenceCount, const bool b_SecHeader,
                const uint8_t *pu8_Data, const uint16_t u16_DataSize)
{
  Serial.print("SP DATA: ");
  for(uint16_t i=0; i<u16_DataSize; i++)
  {
    Serial.print((uint32_t)pu8_Data[i], HEX);
    Serial.print(' ');
  }
  Serial.println();
}


// the setup routine runs once when you press reset:
void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  Serial.print("sizeof(g_Cltu): "); Serial.println(sizeof(g_Cltu));
  Serial.print("sizeof(g_Tc): "); Serial.println(sizeof(g_Tc));
  Serial.print("sizeof(g_Sp): "); Serial.println(sizeof(g_Sp));
}





void loop() {
  
  uint8_t au8_Cltu[] = {
    0xeb, 0x90,
    0x22, 0x5c, 0x00, 0x1d, 0x00, 0xc1, 0x18,    0x30,
    0x60, 0xc0, 0x02, 0x00, 0x0f, 0x11, 0x80,    0x32,
    0x80, 0x00, 0x00, 0x00, 0x00, 0x7d, 0xca,    0x4e,
    0xca, 0x00, 0x00, 0x00, 0x01, 0x72, 0xf3,    0x3a,
    0x7b, 0xea, 0x55, 0x55, 0x55, 0x55, 0x55,    0x44,
    
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,    0x79
  };

   
  g_Cltu.process(au8_Cltu, sizeof(au8_Cltu));

  
  delay(2000);
}
