/**
 * @file      pus_tc.cpp
 *
 * @brief     Source file of the PUS TC class
 *
 * @author    Stefan Trippler
 *
 * @copyright Copyright (C) 2021-2022 Stefan Trippler.  All rights reserved.
 */

#include <string.h>

#include "pus_tc.h"


#define DATA_FIELD_HDR_FLAGS_POS      0
#define  DFH_SEC_HDR_FLAG_POS     7
#define  DFH_PUS_VERSION_POS      4
#define  DFH_FLAG_ACK_COMP_POS    3
#define  DFH_FLAG_ACK_PROG_POS    2
#define  DFH_FLAG_ACK_START_POS   1
#define  DFH_FLAG_ACK_ACC_POS     0
#define DATA_FIELD_HDR_SERVICE_POS    1
#define DATA_FIELD_HDR_SUBSERVICE_POS 2
#define DATA_FIELD_HDR_SOURCEID_POS   3
#define DATA_FIELD_HDR_SPARE_POS      4



namespace PUS 
{

  /**
   * @brief Construct a new PUS TC object
   *
   * @param u8_SecHdrSize       The size of the secondary header (default is 5)
   * @param p_ActionInterface   A pointer to the implementation of the action interface
   */
  Tc::Tc(const uint8_t u8_SecHdrSize, TcActionInterface *p_ActionInterface)
    : mp_ActionInterface{p_ActionInterface}
  {
    if(u8_SecHdrSize>=MinSecHdrSize)
      mu8_SecHdrSize = u8_SecHdrSize;
    else
      mu8_SecHdrSize = MinSecHdrSize;
  }



  /**
   * @brief Construct a new PUS TC object
   *
   * @param p_ActionInterface   A pointer to the implementation of the action interface
   */
  Tc::Tc(TcActionInterface *p_ActionInterface)
    : mu8_SecHdrSize{PUS_TC_DEFAULT_SEC_HEADER_SIZE}
    , mp_ActionInterface{p_ActionInterface}
  {
  }



  /**
   * @brief Sets the action class which is used to call the methods when an action is to be called
   *
   * @param p_ActionInterface A pointer to the implementation of the action interface
   */
  void Tc::setActionInterface(TcActionInterface *p_ActionInterface)
  {
    mp_ActionInterface = p_ActionInterface;
  }


  
  /**
   * @brief Creates a Telecommand and writes it into the given buffer
   *
   * @param pu8_SecHdrBuffer      A pointer to the buffer where the header (service, subservice, acknowledge requests) shall be stored
   * @param u32_SecHdrSize        The size of the header buffer (must be at least 6)
   * @param pu8_PacketDataBuffer  A pointer to the buffer where the data (command parameters) shall be stored
   * @param u32_PacketDataSize    The size of the data buffer
   * @param b_AckAcc              Flag if an Acceptence Report is requested
   * @param b_AckStart            Flag if an Execution Start Report is requested
   * @param b_AckProg             Flag if an Execution Progress Report is requested
   * @param b_AckComp             Flag if an Execution Complete Report is requested
   * @param u8_Service            The service ID of the command
   * @param u8_SubService         The Subservice ID of the command
   * @param u8_SourceID           The source ID of the command
   * @param pu8_Data              Command data (parameters)
   * @param u32_DataSize          The size of the command data
   * @param e_ChecksumType        The checksum algorithm
   *
   * @retval 0  No packet could be created
   * @return The size of the created data (without header)
   */
  uint32_t Tc::create(uint8_t *pu8_SecHdrBuffer, const uint32_t u32_SecHdrSize,
                      uint8_t *pu8_PacketDataBuffer, const uint32_t u32_PacketDataSize,
                      const bool b_AckAcc, const bool b_AckStart, const bool b_AckProg, const bool b_AckComp,
                      const uint8_t u8_Service, const uint8_t u8_SubService,
                      const uint8_t u8_SourceID,
                      const uint8_t *pu8_Data, const uint32_t u32_DataSize, 
                      const enum ChecksumType e_ChecksumType)
  {
    if(!pu8_SecHdrBuffer || (u32_SecHdrSize<MinSecHdrSize))
      return 0;
    if(!pu8_PacketDataBuffer || (u32_PacketDataSize<u32_DataSize))
      return 0;
    if(!pu8_Data)
      return 0;
    
    pu8_SecHdrBuffer[DATA_FIELD_HDR_FLAGS_POS] = (uint8_t)((((uint8_t)(CcsdsSecHeaderFlag::Custom)&0x1)<<7) 
                                    | (uint8_t)((PacketVersion&0x7)<<DFH_PUS_VERSION_POS)
                                    | ((b_AckAcc?1:0)<<DFH_FLAG_ACK_ACC_POS) 
                                    | ((b_AckStart?1:0)<<DFH_FLAG_ACK_START_POS) 
                                    | ((b_AckProg?1:0)<<DFH_FLAG_ACK_PROG_POS) 
                                    | ((b_AckComp?1:0)<<DFH_FLAG_ACK_COMP_POS));
    pu8_SecHdrBuffer[DATA_FIELD_HDR_SERVICE_POS] = u8_Service;
    pu8_SecHdrBuffer[DATA_FIELD_HDR_SUBSERVICE_POS] = u8_SubService;
    if(u32_SecHdrSize>DATA_FIELD_HDR_SOURCEID_POS)
      pu8_SecHdrBuffer[DATA_FIELD_HDR_SOURCEID_POS] = u8_SourceID;
    for(uint32_t i=DATA_FIELD_HDR_SPARE_POS; i<u32_SecHdrSize; i++)
      pu8_SecHdrBuffer[i] = 0;
    
    memcpy(pu8_PacketDataBuffer, pu8_Data, u32_DataSize);
    
    return u32_DataSize;
  }
  
  
  
  /**
   * @brief The given data is processed.
   *
   * The method can only handle complete telecommands.
   * With the extracted information from the data buffer, the function p_PusTcCallback is called.
   *
   * @param pu8_Buffer      The data buffer which is to extract
   * @param u32_BufferSize  The size of the data buffer
   *
   * @retval  0   If the buffer was extracted successfully
   * @retval -1   If fhe u32_BufferSize is 0 or the pu8_Buffer is nullptr
   */
  int32_t Tc::process(const uint8_t *pu8_Buffer, const uint32_t u32_BufferSize)
  {
    bool b_AckAcc;
    bool b_AckStart;
    bool b_AckProg;
    bool b_AckComp;
    uint8_t u8_Service;
    uint8_t u8_SubService;
    uint8_t u8_SourceID = 0;
    
    if((u32_BufferSize<MinSecHdrSize) || (u32_BufferSize<mu8_SecHdrSize))
      return -1;
    
    b_AckAcc   = (pu8_Buffer[DATA_FIELD_HDR_FLAGS_POS]&(1<<DFH_FLAG_ACK_ACC_POS))?true:false;
    b_AckStart = (pu8_Buffer[DATA_FIELD_HDR_FLAGS_POS]&(1<<DFH_FLAG_ACK_START_POS))?true:false;
    b_AckProg  = (pu8_Buffer[DATA_FIELD_HDR_FLAGS_POS]&(1<<DFH_FLAG_ACK_PROG_POS))?true:false;
    b_AckComp  = (pu8_Buffer[DATA_FIELD_HDR_FLAGS_POS]&(1<<DFH_FLAG_ACK_COMP_POS))?true:false;
    u8_Service = pu8_Buffer[DATA_FIELD_HDR_SERVICE_POS];
    u8_SubService = pu8_Buffer[DATA_FIELD_HDR_SUBSERVICE_POS];
    if(mu8_SecHdrSize>=DATA_FIELD_HDR_SOURCEID_POS)
      u8_SourceID = pu8_Buffer[DATA_FIELD_HDR_SOURCEID_POS];
    
    if(nullptr!=mp_ActionInterface)
    {
      mp_ActionInterface->onTcReceived(b_AckAcc, b_AckStart, b_AckProg, b_AckComp,
                                       u8_Service, u8_SubService, u8_SourceID,
                                       &pu8_Buffer[mu8_SecHdrSize], u32_BufferSize-mu8_SecHdrSize);
    }
    return 0;
  }
  
  
  
  uint16_t Tc::calcCRC(const uint8_t *pu8_Buffer, const uint16_t u16_BufferSize)
  {
    uint16_t u16_Syndrome=0xffff;
    uint8_t u8_Data;
    
    for(uint16_t i=0; i<u16_BufferSize; i++)
    {
      u8_Data = pu8_Buffer[i];
      for(uint8_t j=0; j<8; j++)
      {
        if((u8_Data&0x80)^((u16_Syndrome&0x8000)>>8))
        {
          u16_Syndrome = (uint16_t)((u16_Syndrome<<1)^0x1021);
        }
        else
        {
          u16_Syndrome <<= 1;
        }
        u8_Data <<= 1;
      }
    }
    return (u16_Syndrome);
  }

}












