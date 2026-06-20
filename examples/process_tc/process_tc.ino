/*
  Process telecommand uplink data according to CCSDS standard

  This example code is in the public domain.

  http://www.trippler.de/stefan/arduino/ccsds/
*/


#include <ccsds_cltu.h>
#include <ccsds_transferframe_tc.h>
#include <ccsds_spacepacket.h>


using namespace CCSDS;

class TcPipeline;


class SpacePacketPrinter : public SpacePacketActionInterface
{
public:
  void onSpacePacketReceived(ESpacePacketType e_PacketType,
                             ESpacePacketSequenceFlags e_SequenceFlags,
                             uint16_t u16_APID,
                             uint16_t u16_SequenceCount,
                             bool b_SecHeader,
                             const uint8_t *pu8_PacketData,
                             uint16_t u16_PacketDataLength) override
  {
    Serial.println("SP DATA:");
    Serial.print("  Type: ");
    Serial.println(e_PacketType == ESpacePacketType::TM ? "TM" : "TC");
    Serial.print("  SeqFlags: ");
    Serial.println(static_cast<uint8_t>(e_SequenceFlags), HEX);
    Serial.print("  APID: 0x");
    Serial.println(u16_APID, HEX);
    Serial.print("  SequenceCount: ");
    Serial.println(u16_SequenceCount);
    Serial.print("  SecondaryHeader: ");
    Serial.println(b_SecHeader ? "yes" : "no");
    Serial.print("  Payload: ");
    for(uint16_t i = 0; i < u16_PacketDataLength; i++)
    {
      Serial.print(static_cast<uint32_t>(pu8_PacketData[i]), HEX);
      Serial.print(' ');
    }
    Serial.println();
  }
};


class TcFramePrinter : public TransferframeTcActionInterface
{
private:
  SpacePacket *mp_Sp;

public:
  explicit TcFramePrinter(SpacePacket *p_Sp)
    : mp_Sp{p_Sp}
  {
  }

  void onTransferframeTcReceived(bool b_BypassFlag,
                                 bool b_CtrlCmdFlag,
                                 uint16_t u16_SpacecraftID,
                                 uint8_t u8_VirtualChannelID,
                                 uint8_t u8_FrameSeqNumber,
                                 uint8_t u8_MAP,
                                 const uint8_t *pu8_Data,
                                 const uint16_t u16_DataSize) override
  {
    Serial.println("TC DATA:");
    Serial.print("  BypassFlag: ");
    Serial.println(b_BypassFlag ? "true" : "false");
    Serial.print("  CtrlCmdFlag: ");
    Serial.println(b_CtrlCmdFlag ? "true" : "false");
    Serial.print("  SpacecraftID: 0x");
    Serial.println(u16_SpacecraftID, HEX);
    Serial.print("  VCID: ");
    Serial.println(u8_VirtualChannelID);
    Serial.print("  FrameSeq: ");
    Serial.println(u8_FrameSeqNumber);
    Serial.print("  MAP: ");
    Serial.println(u8_MAP);
    Serial.print("  Payload: ");
    for(uint16_t i = 0; i < u16_DataSize; i++)
    {
      Serial.print(static_cast<uint32_t>(pu8_Data[i]), HEX);
      Serial.print(' ');
    }
    Serial.println();

    if(mp_Sp)
      mp_Sp->process(pu8_Data, u16_DataSize);
  }
};


class CltuPrinter : public CltuActionInterface
{
private:
  TransferframeTc *mp_Tc;

public:
  explicit CltuPrinter(TransferframeTc *p_Tc)
    : mp_Tc{p_Tc}
  {
  }

  void onStartOfTransmission(void) override
  {
    Serial.println("SOT");
    if(mp_Tc)
      mp_Tc->setSync();
  }

  void onCltuDataReceived(const uint8_t *pu8_Data, const uint16_t u16_DataSize) override
  {
    Serial.print("CLTU DATA: ");
    for(uint16_t i = 0; i < u16_DataSize; i++)
    {
      Serial.print(static_cast<uint32_t>(pu8_Data[i]), HEX);
      Serial.print(' ');
    }
    Serial.println();

    if(mp_Tc)
      mp_Tc->process(pu8_Data, u16_DataSize);
  }
};


SpacePacketPrinter g_SpPrinter;
SpacePacket g_Sp(&g_SpPrinter);

TcFramePrinter g_TcPrinter(&g_Sp);
TransferframeTc g_Tc(&g_TcPrinter);

CltuPrinter g_CltuPrinter(&g_Tc);
Cltu g_Cltu(&g_CltuPrinter);


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
