/**
 * @file      ccsds_cltu.cpp
 *
 * @brief     Source file of the Communications Link Transmission Unit (CLTU) class
 *
 * @author    Stefan Trippler
 *
 * @copyright Copyright (C) 2021-2022 Stefan Trippler.  All rights reserved.
 */

#include <inttypes.h>
#include <string.h>

#include "ccsds_cltu.h"


namespace CCSDS
{
  /**
   * @brief Construct a new Cltu object
   *
   * @param p_StartOfTransmissionContext   A pointer to the context which has to be used when a start of transmission is detected
   * @param p_StartOfTransmissionCallback  This callback is called when a start of transmission is detected
   * @param p_ReceiveContext               A pointer to the context which has to be used when a data block is received
   * @param p_ReceiveCallback              This callback is called when a data block is received
   */
  Cltu::Cltu(void *p_StartOfTransmissionContext, TStartOfTransmissionCallback *p_StartOfTransmissionCallback,
             void *p_ReceiveContext, TReceiveCallback *p_ReceiveCallback)
  {
    mb_Sync = false;
    mp_StartOfTransmissionContext = p_StartOfTransmissionContext;
    mp_StartOfTransmissionCallback  = p_StartOfTransmissionCallback;
    mp_ReceiveContext = p_ReceiveContext;
    mp_ReceiveCallback = p_ReceiveCallback;
  }
  
  
  /**
   * @brief Overwrites the context pointers and callbacks which were set using the constructor
   *
   * @param p_StartOfTransmissionContext   A pointer to the context which has to be used when a start of transmission is detected
   * @param p_StartOfTransmissionCallback  This callback is called when a start of transmission is detected
   * @param p_ReceiveContext               A pointer to the context which has to be used when a data block is received
   * @param p_ReceiveCallback              This callback is called when a data block is received
   */
  void Cltu::setCallbacks(void *p_StartOfTransmissionContext, TStartOfTransmissionCallback *p_StartOfTransmissionCallback,
                          void *p_ReceiveContext, TReceiveCallback *p_ReceiveCallback)
  {
    mp_StartOfTransmissionContext = p_StartOfTransmissionContext;
    mp_StartOfTransmissionCallback  = p_StartOfTransmissionCallback;
    mp_ReceiveContext = p_ReceiveContext;
    mp_ReceiveCallback = p_ReceiveCallback;
  }
  
  
  /**
   * @brief Creates a sequence of CLTUs, embeds the data to be sent and writes it into the given buffer.
   *
   * @param pu8_Buffer      A pointer to the buffer where the CLTUs shall be stored
   * @param u32_BufferSize  The available size of the buffer
   * @param pu8_Data        A pointer to the data block which shall be wrapped
   * @param u16_DataSize    The size of the data block
   *
   * @return     The size of the CLTU sequence if sucessfull
   * @retval  0  If the buffer size is not sufficient or an other error occured
   */
  uint32_t Cltu::create(uint8_t *pu8_Buffer, const uint32_t u32_BufferSize,
                        const uint8_t *pu8_Data, const uint16_t u16_DataSize)
  {
    uint16_t u16_RequiredBufferSize = StartSequenceSize
    +((u16_DataSize+(DataBlockSize-1))/DataBlockSize)*(DataBlockSize+CRCSize)
    +TailSquenceSize;
    uint16_t u16_BlockNr;
    uint16_t u16_RemainingDataSize = 0;
    uint16_t u16_WritePos = 0;
    
    if(!pu8_Buffer || (u32_BufferSize<u16_RequiredBufferSize))
      return 0;
    if(u16_DataSize>0 && !pu8_Data)
      return 0;
    
    pu8_Buffer[u16_WritePos++]=0xEB;
    pu8_Buffer[u16_WritePos++]=0x90;
    for(u16_BlockNr=0; ((u16_BlockNr+1)*DataBlockSize-1)<u16_DataSize; u16_BlockNr++)
    {
      memcpy(&pu8_Buffer[u16_WritePos], &pu8_Data[u16_BlockNr*DataBlockSize], DataBlockSize);
      pu8_Buffer[u16_WritePos+DataBlockSize]=calcCRC(&pu8_Buffer[u16_WritePos], DataBlockSize);
      u16_WritePos+=(DataBlockSize+CRCSize);
    }
    u16_RemainingDataSize=u16_DataSize-(u16_BlockNr*DataBlockSize);
    if(u16_RemainingDataSize)
    {
      memcpy(&pu8_Buffer[u16_WritePos], &pu8_Data[u16_BlockNr*DataBlockSize], u16_RemainingDataSize);
      memset(&pu8_Buffer[u16_WritePos+u16_RemainingDataSize], 0x55, DataBlockSize-u16_RemainingDataSize);
      pu8_Buffer[u16_WritePos+DataBlockSize]=calcCRC(&pu8_Buffer[u16_WritePos], DataBlockSize);
      u16_WritePos+=(DataBlockSize+CRCSize);
    }
    memset(&pu8_Buffer[u16_WritePos], 0x55, DataBlockSize);
    pu8_Buffer[u16_WritePos+DataBlockSize]=0x79;
    
    return u16_WritePos+DataBlockSize+CRCSize;
  }
  
  
  
  /**
   * @brief The given upstream data is parsed for a CLTU sequence.
   *
   * The method can handle continuously incoming data as well as complete data blocks.
   * If a CLTU start sequence is found, the callback function p_StartOfTransmissionCallback is called.
   * If a complete and valid CLTU sequence is processed, the callback function p_ReceiveCallback is called.
   *
   * @param pu8_Data     The data buffer which is to parse
   * @param u16_DataSize The size of the data buffer
   */
  void Cltu::process(const uint8_t *pu8_Data, const uint16_t u16_DataSize)
  {
    //bool mb_Sync;
    //uint8_t mau8_Buffer[DataBlockSize];
    //uint8_t mu8_Index;
    uint8_t au8_Sync[StartSequenceSize]={0xeb, 0x90};
    
    for(uint16_t i=0; i<u16_DataSize; i++)
    {
      if(!mb_Sync)
      {
        if(mu8_Index<StartSequenceSize)
        {
          if(pu8_Data[i] == au8_Sync[mu8_Index])
          {
            mu8_Index++;
          }
          else
          {
            mb_Sync=false;
            if(pu8_Data[i]==au8_Sync[0])
              mu8_Index=1;
            else
              mu8_Index=0;
          }
        }
        if(mu8_Index == StartSequenceSize)
        {
          // sync complete
          //cout << "Sync found" << endl;
          mb_Sync=true;
          mu8_Index=0;
          if(mp_StartOfTransmissionCallback)
            mp_StartOfTransmissionCallback(mp_StartOfTransmissionContext);
        }
      }
      else
      {
        if(mu8_Index<DataBlockSize)
        {
          mau8_Buffer[mu8_Index]=pu8_Data[i];
          mu8_Index++;
        }
        else
        {
          if(calcCRC(mau8_Buffer, DataBlockSize) == pu8_Data[i])
          {
            if(mp_ReceiveCallback)
              mp_ReceiveCallback(mp_ReceiveContext, mau8_Buffer, DataBlockSize);
          }
          else
          {
            mb_Sync=false;
          }
          mu8_Index=0;
        }
      }
    }
    
    return;
  }
  
  
  
  
  uint8_t Cltu::calcCRC(const uint8_t *pu8_Buffer, const uint8_t u8_BufferSize)
  {
    uint8_t u8_CRC=0x00;
    uint8_t u8_DataStreamXorBit6;
    
    // polynom: G(X) = X^7 + X^6 + X^2 + 1
    
    for(uint8_t u8_BytePos = 0; u8_BytePos<u8_BufferSize; u8_BytePos++)
    {
      for(uint32_t u8_BitPos = 0; u8_BitPos<8; u8_BitPos++)
      {
        u8_DataStreamXorBit6 = ((pu8_Buffer[u8_BytePos]>>(7-u8_BitPos))&0x1) ^ ((u8_CRC>>6)&0x1);
        u8_CRC = (uint8_t)(((u8_CRC<<1)&0x7f) ^ ((u8_DataStreamXorBit6<<6) | (u8_DataStreamXorBit6<<2) | (u8_DataStreamXorBit6)));
      }
    }
    
    return (~u8_CRC)<<1;
  }
  

}
