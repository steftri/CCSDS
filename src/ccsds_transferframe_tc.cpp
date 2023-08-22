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
   * @param p_ActionInterface   A pointer to the implementation of the action interface
   */
  TransferframeTc::TransferframeTc(TransferframeTcActionInterface *p_ActionInterface) 
    : Transferframe()
    , mp_ActionInterface{p_ActionInterface}
  {
  }
  
  
  
  /**
   * @brief Overwrites the context pointer and callback which were set using the constructor
   *
   * @param p_ActionInterface   A pointer to the implementation of the action interface
   */
  void TransferframeTc::setActionInterface(TransferframeTcActionInterface *p_ActionInterface)
  {
    mp_ActionInterface = p_ActionInterface;
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
                                   const uint8_t u8_FrameSeqNumber, const uint8_t u8_MAP,
                                   const uint8_t *pu8_Data, const uint16_t u16_DataSize)
  {
    uint16_t u16_AvailableDataSize;
#if TF_USE_FECF == 1
    uint16_t u16_CRC;
#endif
    
    if(!pu8_Buffer || (u32_BufferSize<PrimaryHdrSize+SegmentHdrSize+1+(UseFECF?FecfSize:0)))
      return 0;
    if((u16_DataSize==0) || !pu8_Data)
      return 0;
    
    u16_AvailableDataSize=u32_BufferSize-PrimaryHdrSize-SegmentHdrSize-(UseFECF?FecfSize:0);
    // printf("[createTC: used %i/%i; HdrSize: %i]", u16_DataSize, u16_AvailableDataSize, PrimaryHdrSize+SegmentHdrSize);
    
    if(u16_DataSize>u16_AvailableDataSize)
      return 0;
    
    // create primary header
    _createPrimaryHeader(pu8_Buffer,
                         b_BypassFlag, b_CtrlCmdFlag,
                         u16_SpacecraftID, u8_VirtualChannelID,
                         PrimaryHdrSize+SegmentHdrSize+u16_DataSize+(UseFECF?FecfSize:0)-1, u8_FrameSeqNumber);

    // create segment header
    if(UseSegHdr)
      _createSegmentHeader(&pu8_Buffer[PrimaryHdrSize], NoSegmentation, u8_MAP);

    memcpy((char*)&pu8_Buffer[PrimaryHdrSize+SegmentHdrSize], pu8_Data, u16_DataSize);
    if(u16_AvailableDataSize>u16_DataSize)
      memset((char*)&pu8_Buffer[PrimaryHdrSize+SegmentHdrSize+u16_DataSize], 0xCA, u16_AvailableDataSize-u16_DataSize);
    
#if TF_USE_FECF == 1
    u16_CRC = Transferframe::calcCRC(pu8_Buffer, PrimaryHdrSize+SegmentHdrSize+u16_DataSize);
    pu8_Buffer[PrimaryHdrSize+SegmentHdrSize+u16_DataSize]   = (uint8_t)(u16_CRC>>8);
    pu8_Buffer[PrimaryHdrSize+SegmentHdrSize+u16_DataSize+1] = (uint8_t)(u16_CRC&0xff);
#endif
    
    return PrimaryHdrSize+SegmentHdrSize+u16_DataSize+(UseFECF?FecfSize:0);
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

    return 5;
  }
  
  
  int32_t TransferframeTc::_createSegmentHeader(uint8_t *pu8_Buffer, const enum ESeqFlags e_SeqFlags, const uint8_t u8_MAP)
  {
    pu8_Buffer[0] = (uint8_t)((e_SeqFlags)<<6) | (u8_MAP&0x3f);
    
    return 1;
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
  
  inline uint16_t TransferframeTc::_getFrameLength(void)
  {
    return (((uint16_t)(mau8_Buffer[2]&0x3)<<8) | (uint16_t)mau8_Buffer[3]);
  }
  
  
  
  void TransferframeTc::_processFrame(void)
  {
    bool b_BypassFlag;
    bool b_CtrlCmdFlag;
    uint16_t u16_SpacecraftID;
    uint8_t u8_VirtualChannelID;
    uint8_t u8_FrameSeqNumber;
    uint8_t u8_MAP;
    uint8_t *pu8_PrimaryHeader=mau8_Buffer;
    uint8_t *pu8_SegmentHeader=&mau8_Buffer[PrimaryHdrSize];
    
    b_BypassFlag = (pu8_PrimaryHeader[0]&0x20)?true:false;
    b_CtrlCmdFlag = (pu8_PrimaryHeader[0]&0x10)?true:false;
    u16_SpacecraftID = (uint16_t)((pu8_PrimaryHeader[0]&0x03)<<8) | (uint16_t)pu8_PrimaryHeader[1];
    u8_VirtualChannelID = (uint8_t)((pu8_PrimaryHeader[2]&0xFC)>>2);
    u8_FrameSeqNumber = pu8_PrimaryHeader[4];

    u8_MAP = UseSegHdr?(pu8_SegmentHeader[0]&0x3F):0x00;
    
    if(mp_ActionInterface)
    {
      mp_ActionInterface->onTransferframeTcReceived(b_BypassFlag, b_CtrlCmdFlag,
                                                    u16_SpacecraftID, u8_VirtualChannelID,
                                                    u8_FrameSeqNumber, u8_MAP,
                                                    &mau8_Buffer[PrimaryHdrSize+SegmentHdrSize], (mu16_FrameLength+1)-PrimaryHdrSize-SegmentHdrSize-(UseFECF?FecfSize:0));
    }
  }
  
  
}
