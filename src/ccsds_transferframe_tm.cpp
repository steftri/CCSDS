/**
 * @file      ccsds_transferframe_tm.cpp
 *
 * @brief     Source file of the Transfer Frame (TM) class
 *
 * @author    Stefan Trippler
 *
 * @copyright Copyright (C) 2021-2022 Stefan Trippler.  All rights reserved.
 */

#include <string.h>

#include "ccsds_transferframe_tm.h"


// for debug:
/*#include <cstdio>
 #include <iostream>
 using namespace std;
 */

namespace CCSDS 
{
  
  
  /**
   * @brief Construct a new TransferframeTm object
   *
   * @param p_TmContext   A pointer to the context which has to be used when a telemetry transfer frame is received
   * @param p_TmCallback  This callback is called when a telemetry transfer frame is received
   */
  TransferframeTm::TransferframeTm(void *p_TmContext, TTmCallback *p_TmCallback) : Transferframe()
  {
    mp_TmContext = p_TmContext;
    mp_TmCallback = p_TmCallback;
  }
  
  
  /**
   * @brief Overwrites the context pointer and callback which were set using the constructor
   *
   * @param p_TmContext   A pointer to the context which has to be used when a telemetry transfer frame is received
   * @param p_TmCallback  This callback is called when a telemetry transfer frame is received
   */
  void TransferframeTm::setCallback(void *p_TmContext, TTmCallback *p_TmCallback)
  {
    mp_TmContext = p_TmContext;
    mp_TmCallback = p_TmCallback;
  }
  
  
  
  /**
   * @brief Creates a Telemetry Transfer Frame and writes it into the given buffer
   *
   * @param pu8_Buffer                  A pointer to the buffer where the space packet shall be stored
   * @param u32_BufferSize              The available size of the buffer
   * @param u16_SpacecraftID            The spacecraft ID which is used for this package (12 bit)
   * @param u8_VirtualChannelID         The virtual channel which is used for this package (0 to 7)
   * @param u8_MasterChannelFrameCount  The overall frame count over all virtual channels, must be increased externally
   * @param u8_VirtualChannelFrameCount The channel-specific frame count, must be increased externally
   * @param u16_FirstHdrPtr             Offset of the first space packet within the data section
   * @param pu8_Data                    A pointer to the data block which shall be wrapped
   * @param u16_DataSize                The size of the data block in bytes
   * @param u32_OCF                     The Operational Control Field (OCF), which is part of the flow control
   *                                    mechanism for uplink data (can hold the Communications Link Control Word (CLCW))
   *
   * @retval 0  No packet could be created
   * @return The size of the created packet in bytes as uint32_t
   */
  uint32_t TransferframeTm::create(uint8_t *pu8_Buffer, const uint32_t u32_BufferSize,
                                   const uint16_t u16_SpacecraftID, const uint8_t u8_VirtualChannelID,
                                   const uint8_t u8_MasterChannelFrameCount, const uint8_t u8_VirtualChannelFrameCount,
                                   const uint16_t u16_FirstHdrPtr,
                                   const uint8_t *pu8_Data, const uint16_t u16_DataSize,
                                   const uint32_t u32_OCF)
  {
    uint16_t u16_AvailableDataSize;
#if TF_USE_FECF == 1
    uint16_t u16_CRC;
#endif
    
    if(!pu8_Buffer || (u32_BufferSize<TfSize))
      return 0;
    if(u16_DataSize>0 && !pu8_Data)
      return 0;
    
    u16_AvailableDataSize=TfSize-PrimaryHdrSize-(UseOCF?OcfSize:0)-(UseFECF?FecfSize:0);
    if(u16_DataSize>u16_AvailableDataSize)
      return 0;
    
    // create primary header
    _createPrimaryHeader(pu8_Buffer, u16_SpacecraftID, u8_VirtualChannelID, UseOCF,
                         u8_MasterChannelFrameCount, u8_VirtualChannelFrameCount,
                         false, false, false,
                         0, u16_FirstHdrPtr);
    
    memcpy((char*)&pu8_Buffer[PrimaryHdrSize], pu8_Data, u16_DataSize);
    if(u16_AvailableDataSize>u16_DataSize)
      memset((char*)&pu8_Buffer[PrimaryHdrSize+u16_DataSize], 0xCA, u16_AvailableDataSize-u16_DataSize);
    
#if TF_USE_OCF == 1
    pu8_Buffer[TfSize-(UseFECF?FecfSize:0)-OcfSize] = (uint8_t)(u32_OCF>>24);
    pu8_Buffer[TfSize-(UseFECF?FecfSize:0)-OcfSize+1] = (uint8_t)(u32_OCF>>16);
    pu8_Buffer[TfSize-(UseFECF?FecfSize:0)-OcfSize+2] = (uint8_t)(u32_OCF>>8);
    pu8_Buffer[TfSize-(UseFECF?FecfSize:0)-OcfSize+3] = (uint8_t)(u32_OCF&0xff);
#endif
    
#if TF_USE_FECF == 1
    u16_CRC = Transferframe::calcCRC(pu8_Buffer, TfSize-2);
    pu8_Buffer[TfSize-2]   = (uint8_t)(u16_CRC>>8);
    pu8_Buffer[TfSize-1] = (uint8_t)(u16_CRC&0xff);
#endif
    
    return TfSize;
  }
  
  
  /**
   * @brief Creates an Idle Telemetry Transfer Frame and writes it into the given buffer
   *
   * Idle Frames are used to fill up the data stream to have a continuous data flow to the ground.
   * Usually, the virtual channel 7 is used for idle frames.
   *
   * @param pu8_Buffer                  A pointer to the buffer where the space packet shall be stored
   * @param u32_BufferSize              The available size of the buffer
   * @param u16_SpacecraftID            The spacecraft ID which is used for this package (12 bit)
   * @param u8_VirtualChannelID         The virtual channel which is used for this idle frame (usually 7)
   * @param u8_MasterChannelFrameCount  The overall frame count over all virtual channels, must be increased externally
   * @param u8_VirtualChannelFrameCount The channel-specific frame count, must be increased externally
   * @param u32_OCF                     The Operational Control Field (OCF), which is part of the flow control
   *                                    mechanism for uplink data (can hold the Communications Link Control Word (CLCW))
   *
   * @retval 0  No packet could be created
   * @return The size of the created packet in bytes as uint32_t
   */
  uint32_t TransferframeTm::createIdle(uint8_t *pu8_Buffer, const uint32_t u32_BufferSize,
                                       const uint16_t u16_SpacecraftID, const uint8_t u8_VirtualChannelID,
                                       const uint8_t u8_MasterChannelFrameCount, const uint8_t u8_VirtualChannelFrameCount,
                                       const uint32_t u32_OCF)
  {
    uint16_t u16_AvailableDataSize;
#if TF_USE_FECF == 1
    uint16_t u16_CRC;
#endif
    
    if(!pu8_Buffer || (u32_BufferSize<TfSize))
      return 0;
    
    u16_AvailableDataSize=TfSize-PrimaryHdrSize-(UseOCF?OcfSize:0)-(UseFECF?FecfSize:0);
    
    // create primary header
    _createPrimaryHeader(pu8_Buffer, u16_SpacecraftID, u8_VirtualChannelID, UseOCF,
                         u8_MasterChannelFrameCount, u8_VirtualChannelFrameCount,
                         false, false, false,
                         0, 0x7FE);
    
    
    memset((char*)&pu8_Buffer[PrimaryHdrSize], 0xCA, u16_AvailableDataSize);
    
#if TF_USE_OCF == 1
    pu8_Buffer[TfSize-(UseFECF?FecfSize:0)-OcfSize] = (uint8_t)(u32_OCF>>24);
    pu8_Buffer[TfSize-(UseFECF?FecfSize:0)-OcfSize+1] = (uint8_t)(u32_OCF>>16);
    pu8_Buffer[TfSize-(UseFECF?FecfSize:0)-OcfSize+2] = (uint8_t)(u32_OCF>>8);
    pu8_Buffer[TfSize-(UseFECF?FecfSize:0)-OcfSize+3] = (uint8_t)(u32_OCF&0xff);
#endif
    
#if TF_USE_FECF == 1
    u16_CRC = Transferframe::calcCRC(pu8_Buffer, TfSize-2);
    pu8_Buffer[TfSize-2]   = (uint8_t)(u16_CRC>>8);
    pu8_Buffer[TfSize-1] = (uint8_t)(u16_CRC&0xff);
#endif
    
    return TfSize;
  }
  
  
  
  int32_t TransferframeTm::_createPrimaryHeader(uint8_t *pu8_Buffer,
                                                const uint16_t u16_SpacecraftID, const uint8_t u8_VirtualChannelID, const bool b_OcfFlag,
                                                const uint8_t u8_MasterChannelFrameCount, const uint8_t u8_VirtualChannelFrameCount,
                                                const bool b_TFSecHdrFlag, const bool b_SyncFlag, const bool b_PacketOrderFlag,
                                                const uint8_t u8_SegLengthID, const uint16_t u16_FirstHdrPtr)
  {
    pu8_Buffer[0] = (uint8_t)(((TmTfVersionNumber&0x3)<<6) | ((u16_SpacecraftID>>4)&0x3F));
    pu8_Buffer[1] = (uint8_t)(((u16_SpacecraftID&0xF)<<4) | ((u8_VirtualChannelID&0x7)<<1) | (b_OcfFlag?1:0)) ;
    pu8_Buffer[2] = u8_MasterChannelFrameCount;
    pu8_Buffer[3] = u8_VirtualChannelFrameCount;
    pu8_Buffer[4] = (uint8_t)(((b_TFSecHdrFlag?1:0)<<7) | ((b_SyncFlag?1:0)<<6) | ((b_PacketOrderFlag?1:0)<<5)
                              | ((u8_SegLengthID&0x3)<<3) | ((u16_FirstHdrPtr>>8)&0x7));
    pu8_Buffer[5] = (uint8_t)(u16_FirstHdrPtr&0xff);
    
    return 0;
  }
  
  
  
  
  
  inline uint16_t TransferframeTm::_getMaxTfSize(void)
  {
    return TfSize;
  }
  
  inline uint8_t *TransferframeTm::_getTfBufferAddr(void)
  {
    return mau8_Buffer;
  }
  
  inline uint16_t TransferframeTm::_getPrimaryHeaderSize(void)
  {
    return PrimaryHdrSize;
  }
  
  inline void TransferframeTm::_getFrameLength(void)
  {
    mu16_FrameLength=TfSize-1;
    // cout << "[" << mu16_FrameLength << "]";
  }
  
  
  
  
  
  
  int32_t TransferframeTm::_processFrame(void)
  {
    uint16_t u16_SpacecraftID;
    uint8_t  u8_VirtualChannelID;
    bool     b_OcfFlag;
    uint8_t  u8_MasterChannelFrameCount;
    uint8_t  u8_VirtualChannelFrameCount;
    uint16_t u16_FirstHdrPtr;
    bool     b_TFSecHdrFlag;
    uint32_t u32_OCF=0x00;
    
    
    u16_SpacecraftID = (uint16_t)((mau8_Buffer[0]&0x3f)<<4) | (uint16_t)((mau8_Buffer[1]&0xf0)>>4);
    u8_VirtualChannelID = (uint8_t)((mau8_Buffer[1]&0x0e)>>1);
    b_OcfFlag = (mau8_Buffer[1]&0x01)?true:false;
    u8_MasterChannelFrameCount = mau8_Buffer[2];
    u8_VirtualChannelFrameCount = mau8_Buffer[3];
    b_TFSecHdrFlag = (mau8_Buffer[4]&0x80)?true:false;
    u16_FirstHdrPtr = (uint16_t)((mau8_Buffer[4]&0x03)<<8) | (uint16_t)mau8_Buffer[5];
    
#if TF_USE_OCF == 1
    if(UseOCF&&b_OcfFlag)
    {
      uint32_t u32_OCFPos=TM_TF_TOTAL_SIZE-OcfSize-(UseFECF?FecfSize:0);
      u32_OCF = ((uint32_t)mau8_Buffer[u32_OCFPos]<<24) | ((uint32_t)mau8_Buffer[u32_OCFPos+1]<<16) | ((uint32_t)mau8_Buffer[u32_OCFPos+2]<<8) | (uint32_t)mau8_Buffer[u32_OCFPos+3];
    }
#endif
    
    if(mp_TmCallback)
    {
      mp_TmCallback(mp_TmContext, u16_SpacecraftID, u8_VirtualChannelID,
                    u8_MasterChannelFrameCount, u8_VirtualChannelFrameCount,
                    b_TFSecHdrFlag, u16_FirstHdrPtr,
                    &mau8_Buffer[PrimaryHdrSize], TM_TF_TOTAL_SIZE-PrimaryHdrSize-((UseOCF&&b_OcfFlag)?OcfSize:0)-(UseFECF?FecfSize:0),
                    u32_OCF);
    }
    return 0;
  }
  
  
}
