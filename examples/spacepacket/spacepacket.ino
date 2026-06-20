#include <ccsds_spacepacket.h>

using namespace CCSDS;

const uint16_t ApplicationID = 0x48;
uint16_t g_u16SequenceCount = 0;
uint8_t g_au8Payload[8] = {0};


class SpActionPrinter : public SpacePacketActionInterface
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
    Serial.println("SP RECEIVED:");
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
    Serial.print("  Data: ");
    for(uint16_t i = 0; i < u16_PacketDataLength; i++)
    {
      Serial.print(static_cast<uint32_t>(pu8_PacketData[i]), HEX);
      Serial.print(' ');
    }
    Serial.println();
  }
};


SpActionPrinter g_ActionPrinter;
SpacePacket g_Sp(&g_ActionPrinter);


void setup()
{
  Serial.begin(9600);
  Serial.println("CCSDS SpacePacket example");
  Serial.print("SpacePacket::MaxSize: ");
  Serial.println(SpacePacket::MaxSize);
  Serial.print("SpacePacket::MaxDataSize: ");
  Serial.println(SpacePacket::MaxDataSize);
}


void loop()
{
  uint8_t au8SpBuffer[SP_MAX_TOTAL_SIZE];

  const unsigned long u32Millis = millis();
  g_au8Payload[0] = static_cast<uint8_t>(u32Millis >> 24);
  g_au8Payload[1] = static_cast<uint8_t>(u32Millis >> 16);
  g_au8Payload[2] = static_cast<uint8_t>(u32Millis >> 8);
  g_au8Payload[3] = static_cast<uint8_t>(u32Millis);
  g_au8Payload[4] = 0x11;
  g_au8Payload[5] = 0x22;
  g_au8Payload[6] = 0x33;
  g_au8Payload[7] = 0x44;

  const uint32_t u32SpSize = SpacePacket::create(
      au8SpBuffer,
      sizeof(au8SpBuffer),
      ESpacePacketType::TM,
      ESpacePacketSequenceFlags::Unsegmented,
      ApplicationID,
      g_u16SequenceCount++,
      g_au8Payload,
      sizeof(g_au8Payload));

  Serial.print("Created SP size: ");
  Serial.println(u32SpSize);
  if(u32SpSize == 0)
  {
    Serial.println("SpacePacket::create failed");
    delay(2000);
    return;
  }

  Serial.print("Raw SP: ");
  for(uint32_t i = 0; i < u32SpSize; i++)
  {
    Serial.print(static_cast<uint32_t>(au8SpBuffer[i]), HEX);
    Serial.print(' ');
  }
  Serial.println();

  g_Sp.process(au8SpBuffer, u32SpSize);
  Serial.println();

  delay(2000);
}

