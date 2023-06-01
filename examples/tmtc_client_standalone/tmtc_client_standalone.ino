/*
  This example shows the usage of the telemetry/telecommand client library.
  The library utilizes Transferframes and Space Packets according to the CCSDS
  standard as it is widely used in satellites to communicate with the ground control.
  
  A telecommand is included in this example program and is extracted by the library.
  
  A telemetry packed is created by the library. For sending the packet, a callback within
  this example is called. This example prints the packet on the serial device in a more
  human readable hexadecimal format. In a real application, the data would be send in
  binary form.
  
  Beside this client example, the underlying protocol classes also support the opposite
  direction, so this library can also be used for creating telecommands and extracting
  telemetry.

  This example code is in the public domain.

  http://www.trippler.de/stefan/arduino/ccsds/
*/


#include "tmtc_client.h"



/* This callback is called when a valid telecommand is extracted from the
   uplink data stream. The utilized protocols supports several virtual channels, and
   each virtual channel can have its own receive callback function. If you need more than one
   virtual channel, this can be configured in configCCSDS.h. */
   
void TelecommandReceiveCallback(void *p_SpContext, const uint8_t u8_PacketType,
                             const uint8_t u8_SequenceFlags, const uint16_t u16_APID,
                             const uint16_t u16_SequenceCount, const bool b_SecHeader,
                             const uint8_t *pu8_PacketData, const uint16_t u16_PacketDataLength)
{
  Serial.print("-> AppID 0x");
  Serial.print(u16_APID, HEX);
  Serial.print(": ");
  
  for(uint16_t i=0; i<u16_PacketDataLength; i++)
  {
    Serial.print(pu8_PacketData[i]>>4, HEX);
    Serial.print(pu8_PacketData[i]&0xf, HEX);
    Serial.print(' ');
  }
  Serial.println();
}


/* This callback is called when a telemetry package is finalized and can be sent.
   ATTENTION: this is an example, so we send it a bit more human readable
              instead of the raw data using Serial.write(pu8_Data, u16_DataSize); */

void SendTelemetryCallback(void *p_Context, const uint8_t *pu8_Data, const uint16_t u16_DataSize)
{
  Serial.print("<- ");
  for(uint16_t i=0; i<u16_DataSize; i++)
  {
    Serial.print(pu8_Data[i]>>4, HEX);
    Serial.print(pu8_Data[i]&0xf, HEX);
    Serial.print(' ');
  }
  Serial.println();
}



/* We need an instance of the TMTC client class. This can be created as global. We also need an array which holds the spacecraft IDs which are accepted for telecommand reception. */

const uint16_t gu16_SpacecraftIDs[3] =
{
  0x20C, /* spacecraft ID 1 - e.g. flight model */
  0x20D, /* spacecraft ID 2 - e.g. engineering model 1 */
  0x25C  /* spacecraft ID 3 - e.g. engineering model 2 */
};  

TmTcClient g_TmTcClient(gu16_SpacecraftIDs, 3, NULL, &SendTelemetryCallback, NULL, &TelecommandReceiveCallback);



/* For this example, we need test data: Command TIM_NOP for Spacecraft 0x25c, ApplicationID 0x060;
   The CCSDS library supports telecommands with and without the CLTU synchronization protocol.
   In real satellites CLTUs are used for the telecommand transfer and extracted by hardware.
   If CLTUs shall be extracted by the library, this feature can be enabled by setting
   configUSE_CLTU_SUPPORT to 1 in configCCSDS.h. The feature needs some additional memory. */
   
#if configUSE_CLTU_SUPPORT == 1

/* This is a TC using CLTUs. */
uint8_t gau8_CltuTfTc[] =
{
  0xeb, 0x90, // CLTU header
  0x22, 0x5c, 0x00, 0x1d, 0x00, 0xc1, 0x18,   0x30,  // 7 bytes transfer frame data, 1 byte checksum
  0x60, 0xc0, 0x02, 0x00, 0x0f, 0x11, 0x80,   0x32, 
  0x80, 0x00, 0x00, 0x00, 0x00, 0x7d, 0xca,   0x4e,
  0xca, 0x00, 0x00, 0x00, 0x01, 0x72, 0xf3,   0x3a, 
  0x7b, 0xea, 0x55, 0x55, 0x55, 0x55, 0x55,   0x44,
  0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55    // 8 bytes footer
};

#else

/* This is the same telecommand, but NOT using CLTUs. You have to take care that die synchronization is given by yourself. */
uint8_t gau8_TfTc[] =
{
  0x22, 0x5c, 0x00, 0x1d, 0x00, 0xc1, 0x18, 
  0x60, 0xc0, 0x02, 0x00, 0x0f, 0x11, 0x80, 
  0x80, 0x00, 0x00, 0x00, 0x00, 0x7d, 0xca, 
  0xca, 0x00, 0x00, 0x00, 0x01, 0x72, 0xf3,
  0x7b, 0xea
};

#endif


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  Serial.println(__FILE__ ": " __DATE__ ", " __TIME__); 

  // lets see, how much RAM we need for extracting TCs and creating TMs
  Serial.print("sizeof(TmTcClient): ");
  Serial.print(sizeof(TmTcClient));
  Serial.println(" bytes");


#if configUSE_CLTU_SUPPORT == 1 
  Serial.println("CLTU support: ON"); 
#else
  Serial.println("CLTU support: OFF"); 
#endif
}



void loop() {
  // put your main code here, to run repeatedly:
  static int i=1;
  Serial.print("Loop ");
  Serial.println(i++);

  // process simulated telecommand 
#if configUSE_CLTU_SUPPORT == 1 
  // process simulated incoming data from ground station as CLTUs
  g_TmTcClient.processCltu((uint8_t*)gau8_CltuTfTc, sizeof(gau8_CltuTfTc));
#else
  // process simulated incoming data from ground station WITHOUT CLTUs
  g_TmTcClient.setSync();
  g_TmTcClient.processTfTc((uint8_t*)gau8_TfTc, sizeof(gau8_TfTc));
#endif

  delay(1000);

  // send simulated telemetry data
  g_TmTcClient.sendTm(0, 0x48, 0, (uint8_t*)"\x01\x02\x03\x04", 4);
  
  delay(1000);
}
