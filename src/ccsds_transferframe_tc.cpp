/**
 * @file      ccsds_transferframe_tc.cpp
 *
 * @brief     Source file of the Transfer Frame (TC) class
 *
 * @author    Stefan Trippler
 *
 * @copyright Copyright (C) 2021-2022 Stefan Trippler.  All rights reserved.
 */

#include <string.h>

#include "ccsds_transferframe_tc.h"


// for debug:
//#include <cstdio>
//#include <iostream>
//using namespace std; 


namespace CCSDS 
{
  
  /**
   * @brief Construct a new TransferframeTc object
   *
   * @param p_TcContext   A pointer to the context which has to be used when a telecommand transfer frame is received
   * @param p_TcCallback  This callback is called when a telecommand transfer frame is received
   */
  TransferframeTc::TransferframeTc(void *p_TcContext, TTcCallback *p_TcCallback) : Transferframe()
  {
    mp_TcContext = p_TcContext;
    mp_TcCallback = p_TcCallback;
  }
  
  
  
  /**
   * @brief Overwrites the context pointer and callback which were set using the constructor
   *
   * @param p_TcContext   A pointer to the context which has to be used when a telecommand transfer frame is received
   * @param p_TcCallback  This callback is called when a telecommand transfer frame is received
   */
  void TransferframeTc::setCallback(void *p_TcContext, TTcCallback *p_TcCallback)
  {
    mp_TcContext = p_TcContext;
    mp_TcCallback = p_TcCallback;
  }
  
  
  
  /**
   * @brief Creates a Telecommand Transfer Frame and writes it into the given buffer
   *
   * @param pu8_Buffer          A pointer to the buffer where the space packet shall be stored
   * @param u32_BufferSize      The available size of the buffer
   * @param b_BypassFlag        Indicates if the command shall be sent in AD mode (BypassFlag must be false) with
   *                            handling of the Frame Acceptance and Reporting Mechanism (FARM) or in
   *                            BD mode (BypassFlag must be true, each packet shall be accepted onboard)
   * @param b_CtrlCmdFlag       Indicates if the packet is intended for the AD mode mechanism
   * @param u16_SpacecraftID    The spacecraft ID which is used for this package (12 bit)
   * @param u8_VirtualChannelID The virtual channel which is used for this package (0 up to 63)
   * @param u8_FrameSeqNumber   The channel-specific frame sequence number, must be increased externally
   * @param pu8_Data            A pointer to the data block which shall be wrapped
   * @param u16_DataSize        The size of the data block in bytes
   *
   * @retval 0  No packet could be created
   * @return The size of the created packet in bytes as uint32_t
   */
  uint32_t TransferframeTc::create(uint8_t *pu8_Buffer, const uint32_t u32_BufferSize,
                                   const bool b_BypassFlag, const bool b_CtrlCmdFlag,
                                   const uint16_t u16_SpacecraftID, const uint8_t u8_VirtualChannelID,
                                   const uint8_t u8_FrameSeqNumber,
                                   const uint8_t *pu8_Data, const uint16_t u16_DataSize)
  {
    uint16_t u16_AvailableDataSize;
#if TF_USE_FECF == 1
    uint16_t u16_CRC;
#endif
    
    if(!pu8_Buffer || (u32_BufferSize<PrimaryHdrSize+1+(UseFECF?FecfSize:0)))
      return 0;
    if((u16_DataSize==0) || !pu8_Data)
      return 0;
    
    u16_AvailableDataSize=u32_BufferSize-PrimaryHdrSize-(UseFECF?FecfSize:0);
    // printf("[createTC: used %i/%i; PrimHdrSize: %i]", u16_DataSize, u16_AvailableDataSize, PrimaryHdrSize);
    
    if(u16_DataSize>u16_AvailableDataSize)
      return 0;
    
    // create primary header
    _createPrimaryHeader(pu8_Buffer,
                         b_BypassFlag, b_CtrlCmdFlag,
                         u16_SpacecraftID, u8_VirtualChannelID,
                         PrimaryHdrSize+u16_DataSize+(UseFECF?FecfSize:0)-1, u8_FrameSeqNumber);
    
    memcpy((char*)&pu8_Buffer[PrimaryHdrSize], pu8_Data, u16_DataSize);
    if(u16_AvailableDataSize>u16_DataSize)
      memset((char*)&pu8_Buffer[PrimaryHdrSize+u16_DataSize], 0xCA, u16_AvailableDataSize-u16_DataSize);
    
#if TF_USE_FECF == 1
    u16_CRC = Transferframe::calcCRC(pu8_Buffer, PrimaryHdrSize+u16_DataSize);
    pu8_Buffer[PrimaryHdrSize+u16_DataSize]   = (uint8_t)(u16_CRC>>8);
    pu8_Buffer[PrimaryHdrSize+u16_DataSize+1] = (uint8_t)(u16_CRC&0xff);
#endif
    
    return PrimaryHdrSize+u16_DataSize+(UseFECF?FecfSize:0);
  }
  
  
  
  int32_t TransferframeTc::_createPrimaryHeader(uint8_t *pu8_Buffer,
                                                const bool b_BypassFlag, const bool b_CtrlCmdFlag,
                                                const uint16_t u16_SpacecraftID, const uint8_t u8_VirtualChannelID,
                                                const uint16_t u16_FrameLength, const uint8_t u8_FrameSeqNumber)
  {
    pu8_Buffer[0] = (uint8_t)(((TcTfVersionNumber&0x3)<<6) | ((b_BypassFlag?1:0)<<5) | ((b_CtrlCmdFlag?1:0)<<4) | ((u16_SpacecraftID>>8)&0x03));
    pu8_Buffer[1] = (uint8_t)(u16_SpacecraftID&0xFF);
    pu8_Buffer[2] = (uint8_t)(((u8_VirtualChannelID&0x3F)<<2) | ((u16_FrameLength>>8)&0x03));
    pu8_Buffer[3] = (uint8_t)(u16_FrameLength&0xff);
    pu8_Buffer[4] = u8_FrameSeqNumber;
    
    return 0;
  }
  
  
  
  
  
  inline uint16_t TransferframeTc::_getMaxTfSize(void)
  {
    return MaxTfSize;
  }
  
  inline uint8_t *TransferframeTc::_getTfBufferAddr(void)
  {
    return mau8_Buffer;
  }
  
  inline uint16_t TransferframeTc::_getPrimaryHeaderSize(void)
  {
    return PrimaryHdrSize;
  }
  
  inline void TransferframeTc::_getFrameLength(void)
  {
    mu16_FrameLength=(((uint16_t)(mau8_Buffer[2]&0x3)<<8) | (uint16_t)mau8_Buffer[3]);
    // cout << "[" << mu16_FrameLength << "]";
  }
  
  
  
  int32_t TransferframeTc::_processFrame(void)
  {
    bool b_BypassFlag;
    bool b_CtrlCmdFlag;
    uint16_t u16_SpacecraftID;
    uint8_t u8_VirtualChannelID;
    uint8_t u8_FrameSeqNumber;
    
    b_BypassFlag = (mau8_Buffer[0]&0x20)?true:false;
    b_CtrlCmdFlag = (mau8_Buffer[0]&0x10)?true:false;
    u16_SpacecraftID = (uint16_t)((mau8_Buffer[0]&0x03)<<8) | (uint16_t)mau8_Buffer[1];
    u8_VirtualChannelID = (uint8_t)((mau8_Buffer[2]&0xFC)>>2);
    u8_FrameSeqNumber = mau8_Buffer[4];
    
    
    if(mp_TcCallback)
    {
      mp_TcCallback(mp_TcContext, b_BypassFlag, b_CtrlCmdFlag,
                    u16_SpacecraftID, u8_VirtualChannelID,
                    u8_FrameSeqNumber,
                    &mau8_Buffer[PrimaryHdrSize], (mu16_FrameLength+1)-PrimaryHdrSize-(UseFECF?FecfSize:0));
    }
    return 0;
  }
  
  
}
