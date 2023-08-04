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

#include <inttypes.h>

<<<<<<< HEAD
#define PUS_TC_DEFAULT_SEC_HEADER_SIZE  5
=======
//#include "configCCSDS.h"
>>>>>>> 9b3ccdb66ece99823c9b846576e1ac99c915b688


namespace PUS 
{
<<<<<<< HEAD
  
  
=======
>>>>>>> 9b3ccdb66ece99823c9b846576e1ac99c915b688
  /**
   * @brief Interface class for handling PUS tc packet actions
   */
  class TcActionInterface
  {
  public:
    /**
     * @brief Declaration of the action which shall be called if a complete telecommand was received
     *
     * The implementation of this callback shall handle the telecommand. It shall implement the fowarding to the
     * corresponding application (or the further processing of the content).
     *
     * @param b_AckAcc              Flag if an Acceptence Report is requested
     * @param b_AckStart            Flag if an Execution Start Report is requested
     * @param b_AckProg             Flag if an Execution Progress Report is requested
     * @param b_AckComp             Flag if an Execution Complete Report is requested
     * @param u8_Service            The service ID of the command
     * @param u8_SubService         The Subservice ID of the command
     * @param u8_SourceID           The source ID of the command
     * @param pu8_Data              Command data (parameters)
     * @param u32_DataSize          The size of the command data
     */
    virtual void onTcReceived(const bool b_AckAcc, const bool b_AckStart, const bool b_AckProg, const bool b_AckComp,
                                  const uint8_t u8_Service, const uint8_t u8_SubService,
                                  const uint8_t u8_SourceID,
                                  const uint8_t *pu8_Data, const uint32_t u32_DataSize) = 0;
  };



  /**
   * @brief Class for handling the telecommands as described in the Packet Utilization Standard (PUS), ECSS-E-70-41A.
   *
   * Beside the description of the structure of space packets, the ECSS-E-70-41A also describes the inner
   * structure of telecommand space packets. This class implements this inner struture. It handles the service
   * and the subservice as well as the acknowledges for a command.
   */
  class Tc
  {
    const static uint8_t PacketVersion = 1;
    const static uint8_t MinSecHdrSize = 4;
    
  public:
    static const auto SecHeaderSize = 6;
    static const auto MaxDataSize = 20;

    enum CcsdsSecHeaderFlag
    {
      Custom = 0,
      CCSDS = 1
    };
    
    enum Service
    {
      TelecommandVerificationService                =  1,
      DeviceCommandDistributionService              =  2,
      HousekeepingAndDiagnosticDataReportingService =  3,
      ParameterStatisticsReportingService           =  4,
      EventReportingService                         =  5,
      MemoryManagementService                       =  6,
      NotUsed1                                      =  7,
      FunctionManagementService                     =  8,
      TimeManagementService                         =  9,
      NotUsed2                                      = 10,
      OnboardOperationsSchedulingService            = 11,
      OnboardMonitoringService                      = 12,
      LargeDataTransferService                      = 13,
      PacketForwardingControlService                = 14,
      OnboardStorageAndRetrievalService             = 15,
      NotUsed3                                      = 16,
      TestService                                   = 17,
      OnboardOperationsProcedureService             = 18,
      EventActionService                            = 19
    };
        
  private:  
    uint8_t mu8_SecHdrSize;
  
    TcActionInterface *mp_ActionInterface;
    
  public:
    Tc(const uint8_t u8_SecdrSize = PUS_TC_DEFAULT_SEC_HEADER_SIZE, TcActionInterface *p_ActionInterface = nullptr);
    Tc(TcActionInterface *p_ActionInterface = nullptr);
    
    void setActionInterface(TcActionInterface *p_ActionInterface);
  
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
<<<<<<< HEAD
=======
    
>>>>>>> 9b3ccdb66ece99823c9b846576e1ac99c915b688
  };
  
}

#endif // _PUS_TC_H_
