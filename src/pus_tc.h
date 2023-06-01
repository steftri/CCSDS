/**
 * @file      pus_tc.h
 *
 * @brief     Include file of the PUS TC class
 *
 * @author    Stefan Trippler
 *
 * @copyright Copyright (C) 2021-2022 Stefan Trippler.  All rights reserved.
 */


#ifndef _PUS_TC_H_
#define _PUS_TC_H_

/****************************************************************/
/* PUS TC Packet according to                                   */
/*                                                              */
/*  ECSS-E-70-41A - Space Packet Protocol                       */
/*                                                              */
/****************************************************************/


namespace PUS 
{
  
#define PUS_TC_HEADER_SIZE  6   // can act as spacepacket secondary header
#define PUS_TC_MAX_DATA_SIZE 20
  
  /**
   * @brief Class for handling the telecommands as described in the Packet Utilization Standard (PUS), ECSS-E-70-41A.
   *
   * Beside the description of the structure of space packets, the ECSS-E-70-41A also describes the inner
   * structure of telecommand space packets. This class implements this inner struture. It handles the service
   * and the subservice as well as the acknowledges for a command.
   */
  class tc
  {
  public:
    enum CcsdsSecHeaderFlag
    {
      Custom = 0,
      CCSDS = 1
    };
    
    enum Service
    {
      TelecommandVerificationService = 1,
      DeviceCommandDistributionService = 2,
      HousekeepingAndDiagnosticDataReportingService = 3,
      ParameterStatisticsReportingService = 4,
      EventReportingService = 5,
      MemoryManagementService = 6,
      NotUsed1 = 7,
      FunctionManagementService = 8,
      TimeManagementService = 9,
      NotUsed2 = 10,
      OnboardOperationsSchedulingService = 11,
      OnboardMonitoringService = 12,
      LargeDataTransferService = 13,
      PacketForwardingControlService = 14,
      OnboardStorageAndRetrievalService = 15,
      NotUsed3 = 16,
      TestService = 17,
      OnboardOperationsProcedureService = 18,
      EventActionService = 19
    };
    
    typedef void (TPusTcCallback)(void *p_Context,
                                  const bool b_AckAcc, const bool b_AckStart, const bool b_AckProg, const bool b_AckComp,
                                  const uint8_t u8_Service, const uint8_t u8_SubService,
                                  const uint8_t u8_SourceID,
                                  const uint8_t *pu8_Data, const uint32_t u32_DataSize);
    
  private:
    static const int PusTcPacketVersion = 1;
    
    void *mp_Context;
    TPusTcCallback *mp_PusTcCallback;
    
    
  public:
    tc(void *p_Context = NULL, TPusTcCallback *p_PusTcCallback = NULL);
    
    static uint32_t create(uint8_t *pu8_SecHdrBuffer, const uint32_t u32_SecHdrSize,
                           uint8_t *pu8_PacketDataBuffer, const uint32_t u32_PacketDataSize,
                           const bool b_AckAcc, const bool b_AckStart, const bool b_AckProg, const bool b_AckComp,
                           const uint8_t u8_Service, const uint8_t u8_SubService,
                           const uint8_t u8_SourceID,
                           const uint8_t *pu8_Data, const uint32_t u32_DataSize);
    
    // sp processing
    int32_t process(const uint8_t *pu8_Buffer, const uint32_t u32_BufferSize);
    
    
    
  private:
    static int32_t _create_secondary_header(uint8_t *pu8_Buffer,
                                            const bool b_AckAcc, const bool b_AckStart, const bool b_AckProg, const bool b_AckComp,
                                            const uint8_t u8_Service, const uint8_t u8_SubService,
                                            const uint8_t u8_SourceID, const uint16_t u16_Spare);
    
  public:
    
    static uint16_t _calcCRC(const uint8_t *pu8_Buffer, const uint16_t u16_BufferSize);
    
    
  };
  
}

#endif // _PUS_TC_H_
