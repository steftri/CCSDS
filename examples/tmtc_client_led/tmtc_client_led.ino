/*
  This example shows the usage of the telemetry/telecommand client library.
  The library utilizes Transferframes and Space Packets according to the CCSDS
  standard as it is widely used in satellites to communicate with the ground control.
  
  Data, which comes from the serial device, is parsed by the CCSDS library. If a valid
  telecommand packet for spacecraft ID 0x20c is received, the callback function
  "TelecommandReceiveCallback" is called.
  If the target application ID is 0xs60, the packet is further processed by the PUS TC class.
  If the command is valid, the function "TcCallback", which is then called by the library,
  switches the build-in LED on or off.
  
  Beside this client example, the underlying protocol classes also support the opposite
  direction, so this library can also be used for creating telecommands and extracting
  telemetry.

  This example code is in the public domain.

  http://www.trippler.de/stefan/arduino/ccsds/
*/


#include "tmtc_client.h"
#include "pus_tc.h"




/* This callback is called when a telecommand for Application 0x60 is processed.
  If Service 128 / SubService 1 is requested, the LED is swiched ON.
  If Service 128 / SubService 2 is requested, the LED is swiched OFF.
*/

void TcCallback(const void *p_Context, 
  const bool b_AckAcc, const bool b_AckStart, const bool b_AckProg, const bool b_AckComp,
  const uint8_t u8_Service, const uint8_t u8_SubService,
  const uint8_t u8_SourceID, 
  const uint8_t *pu8_DataBuffer, const uint32_t u32_DataSize)
{
  if(u8_Service==128)
  {
    switch(u8_SubService)
    {
    case 1:
      digitalWrite(LED_BUILTIN, HIGH);
      break;
    case 2:
      digitalWrite(LED_BUILTIN, LOW);
      break;
    default:
      break;
    }
  }
}

/* We need an instance of the Tc class which is able to process telecommands. */

PUS::tc g_Tc(&TcCallback);



/* This callback is called when a telecommand is extracted from the
   uplink data stream. In this case, the callback calls the further processing of the telecommand
   if the target application is 0x60 */
   
void TelecommandReceiveCallback(void *p_SpContext, const uint8_t u8_PacketType,
                             const uint8_t u8_SequenceFlags, const uint16_t u16_APID,
                             const uint16_t u16_SequenceCount, const bool b_SecHeader,
                             const uint8_t *pu8_PacketData, const uint16_t u16_PacketDataLength)
{
  Serial.println("TC received");

  if(u16_APID==0x60)
  {
    g_Tc.process(pu8_PacketData, u16_PacketDataLength);
  }
}




/* We need an instance of the TMTC client class. This can be created as global. We also need an array which holds the spacecraft IDs which are accepted for telecommand reception. */

const uint16_t gu16_SpacecraftIDs[2] =
{
  0x20C, /* spacecraft ID 1 - e.g. flight model */
  0x20D  /* spacecraft ID 2 - e.g. engineering model */
};

TmTcClient g_TmTcClient(gu16_SpacecraftIDs, 2, NULL, NULL, NULL, &TelecommandReceiveCallback);




void setup() {
  // put your setup code here, to run once:
  
  Serial.begin(9600);
  Serial.println(__FILE__ ": " __DATE__ ", " __TIME__);

  pinMode(LED_BUILTIN, OUTPUT);

  // lets see, how much RAM we need for extracting TCs and creating TMs
  Serial.print("sizeof(g_TmTcClient): ");
  Serial.print(sizeof(g_TmTcClient));
  Serial.println(" bytes");
  Serial.print("sizeof(g_Tc): ");
  Serial.print(sizeof(g_Tc));
  Serial.println(" bytes");

#if configUSE_CLTU_SUPPORT == 1
  Serial.println("CLTU support: ON");
#else
  Serial.println("CLTU support: OFF");
#endif
}



void loop() {
  // put your main code here, to run repeatedly:
  
  while(Serial.available())
  {
    char c_Char = Serial.read();
    g_TmTcClient.processTfTc((uint8_t*)&c_Char, 1);
  }
}

