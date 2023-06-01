/**
 * @file      ccsds_transferframe.cpp
 *
 * @brief     Source file of the Transfer Frame (TF) base class
 *
 * @author    Stefan Trippler
 *
 * @copyright Copyright (C) 2021-2022 Stefan Trippler.  All rights reserved.
 */

#include "ccsds_transferframe.h"


// for debugging
//using namespace std;
//#include <iostream>

/*#include <iomanip>
 void displaybuffer(const uint8_t *pu8_Buffer, const uint16_t u16_BufferSize)
 {
 cout << "["; // << hex << setw(8) << setfill('0') << (uint32_t)pu8_Buffer << ", ";
 cout << dec << (uint32_t)u16_BufferSize << ": ";
 for(uint16_t i=0; i<u16_BufferSize; i++)
 {
 if(i%16==0 && i>0)
 cout << endl;
 cout << hex << setw(2) << setfill('0') << (uint32_t)pu8_Buffer[i] << ' ';
 }
 cout << ']' << endl;
 }*/



namespace CCSDS 
{
  
  
  /**
   * @brief Construct a new Transferframe object
   */
  Transferframe::Transferframe(void)
  {
    mu16_Index = 0;
    mu16_FrameLength = 0;
    mb_Sync = true;
    mu16_SyncErrorCount = 0;
    mu16_ChecksumErrorCount = 0;
    mu16_OverflowErrorCount = 0;
  }
  
  
  /**
   * @brief Sets the sync flag for the transfer frame processing
   *
   * This method enables the object to parse the data stram for a complete transfer frame.
   * In case of uplink data, this method must be called if the start of a new transfer frame
   * is detected by receiving a CLTU start sequence (Telecommand Transfer Frames do not
   * have the sync code). In case of downlink data, the synchronization
   * can be done automatically by detectiong the start code.
   */
  void Transferframe::setSync(void)
  {
    mu16_Index=SyncSize;
    mb_Sync=true;
  }
  
  
  
  /**
   * @brief The given data stream is parsed for Transfer Frames.
   *
   * The method can handle continuously incoming data as well as complete data blocks.
   * If a complete Transfer Frame is processed, the corresponding uplink or downlink
   * callback function is called.
   *
   * Attention: In contrast to Telemetry Transfer Frames, the Telecommand Transfer Frames
   * do not come with a sync code. For enabling the processing, the method setSync() must
   * be called if a start sequence was received by the wrapping protocol (usually a CLTU
   * start sequence).
   *
   * @param pu8_Data      The data buffer which is to parse
   * @param u16_DataSize  The size of the data buffer
   *
   * @retval  0   If the buffer was parsed
   * @retval -1   If fhe u16_DataSize is 0 or the pu8_Data is NULL
   */
  int32_t Transferframe::process(const uint8_t *pu8_Data, const uint16_t u16_DataSize)
  {
    uint16_t u16_MaxTfSize = _getMaxTfSize();
    uint8_t *pu8_Buffer = _getTfBufferAddr();
    uint16_t u16_PrimaryHdrSize = _getPrimaryHeaderSize();
    bool b_Valid = true;
    
    if((u16_DataSize==0) || !pu8_Data)
      return -1;
    
    //   cout << "Tf::process(); max size: " << u16_MaxTfSize << endl;
    uint8_t au8_Sync[SyncSize]={0x1a, 0xcf, 0xfc, 0x1d};
    
    for(uint16_t i=0; i<u16_DataSize; i++)
    {
      if(mu16_Index<SyncSize)
      {
        if(pu8_Data[i] == au8_Sync[mu16_Index])
        {
          mu16_Index++;
        }
        else
        {
          mb_Sync=false;
          if(pu8_Data[i]==au8_Sync[0])
            mu16_Index=1;
          else
            mu16_Index=0;
        }
        //      cout << mu16_Index;
      }
      else if((mu16_Index<SyncSize+u16_PrimaryHdrSize) || (mu16_Index<SyncSize+mu16_FrameLength+1))
      {
        if(mu16_Index-SyncSize<u16_MaxTfSize)
          pu8_Buffer[mu16_Index-SyncSize]=pu8_Data[i];
        mu16_Index++;
        //    cout << '.';
      }
      
      if(mu16_Index==SyncSize)
      {
        if(!mb_Sync && mu16_SyncErrorCount<0xffff)
          mu16_SyncErrorCount++;
        mb_Sync=true;
        //   cout << '>';
      }
      
      if(mu16_Index==SyncSize+(uint32_t)u16_PrimaryHdrSize)
      {
        //  cout << 'H';
        //displaybuffer(pu8_Buffer, u16_PrimaryHdrSize);
        _getFrameLength();
        if(mu16_FrameLength+1>_getMaxTfSize())
        {
          mb_Sync=false;
          mu16_Index=0;
          mu16_FrameLength=0;
          if(mu16_OverflowErrorCount<0xffff)
            mu16_OverflowErrorCount++;
        }
      }
      
      if((mu16_Index>SyncSize+(uint32_t)u16_PrimaryHdrSize) && (mu16_Index>=SyncSize+mu16_FrameLength+1))
      {
        //  cout << "C: ";
        //  displaybuffer(pu8_Buffer, mu16_FrameLength+1);
        //  cout << endl << "  ";
        
#if TF_USE_FECF == 1
        b_Valid = _checkCRC();
        if(!b_Valid && (mu16_ChecksumErrorCount<0xffff))
          mu16_ChecksumErrorCount++;
#endif
        
        if(b_Valid)
          _processFrame();
        mu16_Index=0;
        mu16_FrameLength=0;
      }
    }
    
    return 0;
  }
  
  
  /**
   * @brief Returns the number of sync errors
   *
   * Sync Errors occur if the parsing engine does not find an expected sync code.
   *
   * If the number of sync errors exceeds 65535, the method returns 65535.
   *
   * @return Number of sync errors as uint16_t
   */
  uint16_t Transferframe::getSyncErrorCount(void)
  {
    return mu16_SyncErrorCount + ((mu16_SyncErrorCount<0xffff && !mb_Sync)?1:0);
  }
  
  
  /**
   * @brief Returns the number of checksum errors
   *
   * Checksum errors occur if the FECF feature of the protocol is used and the
   * calculated checksum of the transfer frame does not match the received one.
   *
   * @return Number of checksum errors as uint16_t
   */
  uint16_t Transferframe::getChecksumErrorCount(void)
  {
    return mu16_ChecksumErrorCount;
  }
  
  
  /**
   * @brief Returns the number of overflow errors
   *
   * Overflow errors occur if the frame length from given in the header information is
   * bigger than the maximum remaining size of the transfer frame data section;
   * A wrong package alignment caused by a synchronization error can also lead to an
   * overflow error.
   *
   * If the number of overflow errors exceeds 65535, the method returns 65535.
   *
   * @return Number of overflow errors as uint16_t
   */
  uint16_t Transferframe::getOverflowErrorCount(void)
  {
    return mu16_OverflowErrorCount;
  }
  
  
  /**
   * @brief Clears all error counters (Sync Error, Overflow Error and Checksum Error)
   */
  void Transferframe::clearErrorCounters(void)
  {
    mu16_ChecksumErrorCount=0;
    mu16_SyncErrorCount=0;
    mu16_OverflowErrorCount=0;
    return;
  }
  
  
  
  bool Transferframe::_checkCRC(void)
  {
    uint8_t *pu8_Buffer = _getTfBufferAddr();
    //   uint16_t u16_PrimaryHdrSize = _getPrimaryHeaderSize();
    uint16_t u16_FrameCRC;
    uint16_t u16_CalcCRC;
    
    u16_FrameCRC = (uint16_t)(pu8_Buffer[mu16_FrameLength+1-2]<<8) | (uint16_t)pu8_Buffer[mu16_FrameLength+1-1];
    u16_CalcCRC = Transferframe::calcCRC(pu8_Buffer, mu16_FrameLength+1-2);
    
    //   printf("[0x%04x/0x%04x]", u16_FrameCRC, u16_CalcCRC);
    
    return (u16_FrameCRC == u16_CalcCRC)?true:false;
  }
  
  
  
  uint16_t Transferframe::calcCRC(const uint8_t *pu8_Buffer, const uint16_t u16_BufferSize)
  {
    uint16_t u16_CRC=0xffff;
    uint16_t u16_DataStreamXorBit15;
    
    // This implementation is slow compared to other implementations, but it
    // does consume much less space in memory (relevant for Arduino).
    
    // polynom: G(X) = X^16 + X^12 + X^5 + 1
    for(uint16_t u16_BytePos = 0; u16_BytePos<u16_BufferSize; u16_BytePos++)
    {
      for(uint32_t u8_BitPos = 0; u8_BitPos<8; u8_BitPos++)
      {
        u16_DataStreamXorBit15 = ((pu8_Buffer[u16_BytePos]>>(7-u8_BitPos))&0x1) ^ ((u16_CRC>>15)&0x1);
        u16_CRC = (uint16_t)((u16_CRC<<1) ^ ((u16_DataStreamXorBit15<<12) | (u16_DataStreamXorBit15<<5) | (u16_DataStreamXorBit15)));
      }
    }
    
    return u16_CRC;
  }
  
}
