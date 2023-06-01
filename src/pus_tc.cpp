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


namespace PUS 
{
  
#define TC_SEC_HEADER_SIZE  6  
  
  
  /**
   * @brief Construct a new PUS TC object
   *
   * @param p_Context        A pointer to the context which has to be used when a command is received
   * @param p_PusTcCallback  This callback is called when a command is received
   */
  tc::tc(void *p_Context, TPusTcCallback *p_PusTcCallback)
  {
    mp_Context = p_Context;
    mp_PusTcCallback = p_PusTcCallback;
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
   *
   * @retval 0  No packet could be created
   * @return The size of the created data (without header)
   */
  uint32_t tc::create(uint8_t *pu8_SecHdrBuffer, const uint32_t u32_SecHdrSize,
                      uint8_t *pu8_PacketDataBuffer, const uint32_t u32_PacketDataSize,
                      const bool b_AckAcc, const bool b_AckStart, const bool b_AckProg, const bool b_AckComp,
                      const uint8_t u8_Service, const uint8_t u8_SubService,
                      const uint8_t u8_SourceID,
                      const uint8_t *pu8_Data, const uint32_t u32_DataSize)
  {
    if(!pu8_SecHdrBuffer || (u32_SecHdrSize<TC_SEC_HEADER_SIZE))
      return 0;
    if(!pu8_PacketDataBuffer || (u32_PacketDataSize<u32_DataSize))
      return 0;
    if(!pu8_Data)
      return 0;
    
    pu8_SecHdrBuffer[0] = (uint8_t)((((uint8_t)(Custom)&0x1)<<7) |  (uint8_t)((PusTcPacketVersion&0x7)<<4)
                                    | ((b_AckAcc?1:0)<<3) | ((b_AckStart?1:0)<<2) | ((b_AckProg?1:0)<<1) | (b_AckComp?1:0));
    pu8_SecHdrBuffer[1] = u8_Service;
    pu8_SecHdrBuffer[2] = u8_SubService;
    pu8_SecHdrBuffer[3] = u8_SourceID;
    pu8_SecHdrBuffer[4] = 0; //(uint8_t)(u16_Spare>>8);
    pu8_SecHdrBuffer[5] = 0; //(uint8_t)(u16_Spare&0xff);
    
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
  int32_t tc::process(const uint8_t *pu8_Buffer, const uint32_t u32_BufferSize)
  {
    bool b_AckAcc;
    bool b_AckStart;
    bool b_AckProg;
    bool b_AckComp;
    uint8_t u8_Service;
    uint8_t u8_SubService;
    uint8_t u8_SourceID;
    //uint16_t u16_Spare;
    
    if(u32_BufferSize<TC_SEC_HEADER_SIZE)
      return -1;
    
    b_AckAcc = (pu8_Buffer[0]&0x08)?true:false;
    b_AckStart = (pu8_Buffer[0]&0x04)?true:false;
    b_AckProg = (pu8_Buffer[0]&0x02)?true:false;
    b_AckComp = (pu8_Buffer[0]&0x01)?true:false;
    u8_Service = pu8_Buffer[1];
    u8_SubService = pu8_Buffer[2];
    u8_SourceID = pu8_Buffer[3];
    //u16_Spare = ((uint16_t)pu8_Buffer[4]<<8) | (uint16_t)pu8_Buffer[5];
    
    if(nullptr!=mp_PusTcCallback)
      mp_PusTcCallback(mp_Context, b_AckAcc, b_AckStart, b_AckProg, b_AckComp,
                       u8_Service, u8_SubService, u8_SourceID,
                       &pu8_Buffer[TC_SEC_HEADER_SIZE], u32_BufferSize-TC_SEC_HEADER_SIZE);
    
    return 0;
  }
  
  
  
  
  uint16_t tc::_calcCRC(const uint8_t *pu8_Buffer, const uint16_t u16_BufferSize)
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
