/*
  This example shows the usage of the telemetry/telecommand client library.
  The library utilizes Transferframes and Space Packets according to the CCSDS
  standard as it is widely used in satellites to communicate with the ground control.
  
  Every second, the command TcLedOn/TcLedOff is parsed by the CCSDS library and the
  function "TcCallback", which is then called by the library, switches the build-in LED
  on or off.
  
  A telemetry packed is created by the library. For sending the packet, the callback function
  "SendTelemetryCallback" within this example is called. This example writes the packet to the
  serial device in binary form.
  
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

PUS::tc g_Tc(NULL, &TcCallback);



/* This callback is called when a telecommand is extracted from the
   uplink data stream. In this case, the callback calls the further processing of the telecommand
   if the target application is 0x60 */
   
void TelecommandReceiveCallback(void *p_SpContext, const uint8_t u8_PacketType,
                             const uint8_t u8_SequenceFlags, const uint16_t u16_APID,
                             const uint16_t u16_SequenceCount, const bool b_SecHeader,
                             const uint8_t *pu8_PacketData, const uint16_t u16_PacketDataLength)
{
  if(u16_APID==0x60)
    g_Tc.process(pu8_PacketData, u16_PacketDataLength);
}



/* This callback is called when a telemetry package is finalized and can be sent. */

void SendTelemetryCallback(void *p_Context, const uint8_t *pu8_Data, const uint16_t u16_DataSize)
{
  Serial.write(pu8_Data, u16_DataSize);
}




/* We need an instance of the TMTC client class. This can be created as global. We also need an array which holds the spacecraft IDs which are accepted for telecommand reception. */

const uint16_t gu16_SpacecraftIDs[2] =
{
  0x20C, /* spacecraft ID 1 - e.g. flight model */
  0x20D  /* spacecraft ID 2 - e.g. engineering model */
};

TmTcClient g_TmTcClient(gu16_SpacecraftIDs, 2, NULL, &SendTelemetryCallback, NULL, &TelecommandReceiveCallback);






/* For this example, we need two test commands:
   Command LED_ON  for Spacecraft 0x20c, ApplicationID 0x60, Service 128, SubService 1
   and
   Command LED_OFF for Spacecraft 0x20c, ApplicationID 0x60, Service 128, SubService 2
*/

uint8_t gau8_TcLedOn[] = {
  0x1a, 0xcf, 0xfc, 0x1d,
  0x22, 0x0C, 0x00, 0x16, 0x00, 0x00, 0x10, 0x60, 0xC0, 0x00, 0x00, 0x08, 0x11, 0x80, 0x01, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x80, 0x29, 0xB5
};
uint8_t gau8_TcLedOff[] = {
  0x1a, 0xcf, 0xfc, 0x1d,
  0x22, 0x0C, 0x00, 0x16, 0x00, 0x00, 0x10, 0x60, 0xC0, 0x00, 0x00, 0x08, 0x11, 0x80, 0x02, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x80, 0xF1, 0x37
};



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
  
  static uint32_t u32_Loop=1;
  uint8_t u8_LedOn;
  uint8_t au8_TelemetryData[5];
  
  if(u32_Loop&1)
    u8_LedOn = 1;
  else
    u8_LedOn = 0;

  // process simulated incoming data from ground station WITHOUT CLTUs
  if(u8_LedOn==1)
    g_TmTcClient.processTfTc((uint8_t*)gau8_TcLedOn, sizeof(gau8_TcLedOn));
  else
    g_TmTcClient.processTfTc((uint8_t*)gau8_TcLedOff, sizeof(gau8_TcLedOff));

  // send simulated telemetry data; content: the loop counter on bytes 1-4 and the LED state on byte 5
  au8_TelemetryData[0] = (uint8_t)(u32_Loop>>24);
  au8_TelemetryData[1] = (uint8_t)(u32_Loop>>16);
  au8_TelemetryData[2] = (uint8_t)(u32_Loop>>8);
  au8_TelemetryData[3] = (uint8_t)(u32_Loop);
  au8_TelemetryData[4] = u8_LedOn;
  g_TmTcClient.sendTm(0, 0x48, 0, au8_TelemetryData, sizeof(au8_TelemetryData));
  
  delay(1000);
  u32_Loop++;
}

