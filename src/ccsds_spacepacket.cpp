/**
 * @file      ccsds_spacepacket.cpp
 *
 * @brief     Source file of the Space Packet (SP) class
 *
 * @author    Stefan Trippler
 *
 * @copyright Copyright (C) 2021-2022 Stefan Trippler.  All rights reserved.
 */

#include <string.h>

#include "ccsds_spacepacket.h"


namespace CCSDS 
{
  
  /**
   * @brief Construct a new SpacePacket object
   *
   * @param p_SpContext   A pointer to the context which has to be used when a space packet is received
   * @param p_SpCallback  This callback is called when a space packet is received
   */
  SpacePacket::SpacePacket(void *p_SpContext, TSpCallback *p_SpCallback)
  {
    mu32_Index = 0;
    mu8_PacketVersionNumber = 0;
    me_PacketType = SpacePacket::TM;
    mb_SecHdrFlag = false;
    mu16_APID = 0x000;
    me_SequenceFlags = SpacePacket::Unsegmented;
    mu16_PacketSequenceCount = 0;
    mu16_PacketDataLength = 0;
    mu16_SyncErrorCount = 0;
    mb_Overflow = 0;
    
    mp_SpContext = p_SpContext;
    mp_SpCallback = p_SpCallback;
  }
  
  
  
  /**
   * @brief Overwrites the context pointer and callback which were set using the constructor
   *
   * @param p_SpContext   A pointer to the context which has to be used when a space packet is received
   * @param p_SpCallback  This callback is called when a space packet is received
   */
  void SpacePacket::setCallback(void *p_SpContext, TSpCallback *p_SpCallback)
  {
    mp_SpContext = p_SpContext;
    mp_SpCallback = p_SpCallback;
  }
  
  
  
  /**
   * @brief Creates a Space Packet and writes it into the given buffer
   *
   * @param pu8_Buffer           A pointer to the buffer where the space packet shall be stored
   * @param u32_BufferSize       The available size of the buffer
   * @param e_PacketType         Identifies the type of Space Packet (SpacePacket::TM or SpacePacket::TC)
   * @param e_SequenceFlags      Identifies that a packet belongs to a sequence of packets
   *                             (one of SpacePacket::Unsegmented, SpacePacket::FirstSegment, SpacePacket::ContinuationSegment, SpacePacket::LastSegment)
   * @param u16_APID             The Application Identifier where this packet belongs to
   * @param u16_SequenceCount    The 14-bit sequence counter is handeled by the calling context and
   *                             must be specific for each APID
   * @param pu8_PacketData       A pointer to the data block which shall be wrapped
   * @param u16_PacketDataLength The size of the data block in bytes
   *
   * @retval 0  No packet could be created
   * @return The size of the created packet in bytes as uint32_t
   */
  uint32_t SpacePacket::create(uint8_t *pu8_Buffer, const uint32_t u32_BufferSize,
                               const PacketType e_PacketType, const SequenceFlags e_SequenceFlags, const uint16_t u16_APID, const uint16_t u16_SequenceCount,
                               const uint8_t *pu8_PacketData, const uint16_t u16_PacketDataLength)
  {
    return create(pu8_Buffer, u32_BufferSize, e_PacketType, e_SequenceFlags, u16_APID, u16_SequenceCount,
                  nullptr, 0, pu8_PacketData, u16_PacketDataLength);
  }
  
  
  
  /**
   * @brief Creates a Space Packet with Secondary Header and writes it into the given buffer
   *
   * The secondary header is not standardized. It can for example hold a time stamp in a telemetry packet.
   * It also can hold additional information about the sender/receiver if SpacecraftID and the ApplicationID
   * is not enough.
   *
   * @param pu8_Buffer           A pointer to the buffer where the space packet shall be stored
   * @param u32_BufferSize       The available size of the buffer
   * @param e_PacketType         Identifies the type of Space Packet (SpacePacket::TM or SpacePacket::TC)
   * @param e_SequenceFlags      Identifies that a packet belongs to a sequence of packets
   *                             (one of SpacePacket::Unsegmented, SpacePacket::FirstSegment, SpacePacket::ContinuationSegment, SpacePacket::LastSegment)
   * @param u16_APID             The application identifier (APID) defines where this packet belongs to
   * @param u16_SequenceCount    The 14-bit sequence counter is handeled by the calling context and
   *                             must be specific for each APID.
   * @param pu8_SecondaryHeaderData A pointer to the secondary header
   * @param u16_SecondaryHeaderLength The size of the secondary header
   * @param pu8_PacketData       A pointer to the data block which shall be wrapped
   * @param u16_PacketDataLength The size of the data block in bytes; the secondary header and
   *                             the packet data must both fit into 65535 bytes.
   *
   * @retval 0  No packet could be created
   * @return The size of the created packet in bytes as uint32_t
   */
  uint32_t SpacePacket::create(uint8_t *pu8_Buffer, const uint32_t u32_BufferSize,
                               const PacketType e_PacketType, const SequenceFlags e_SequenceFlags, const uint16_t u16_APID, const uint16_t u16_SequenceCount,
                               const uint8_t *pu8_SecondaryHeaderData, const uint16_t u16_SecondaryHeaderLength,
                               const uint8_t *pu8_PacketData, const uint16_t u16_PacketDataLength)
  {
    if(!pu8_Buffer || (u32_BufferSize<7)
       || (u32_BufferSize<6UL+u16_SecondaryHeaderLength+u16_PacketDataLength)
       || (u16_PacketDataLength==0) || !pu8_PacketData)
      return 0;
    
    if(u16_SecondaryHeaderLength>0 && !pu8_SecondaryHeaderData)
      return 0;
    
    if((uint32_t)u16_SecondaryHeaderLength+(uint32_t)u16_PacketDataLength-1>0xffff)
      return 0;
    
    // create primary header
    _create_primary_header(pu8_Buffer, e_PacketType, e_SequenceFlags, u16_APID, u16_SequenceCount,
                           u16_SecondaryHeaderLength?true:false, (uint32_t)u16_SecondaryHeaderLength+(uint32_t)u16_PacketDataLength);
    
    if(u16_SecondaryHeaderLength>0)
      memcpy((char*)&pu8_Buffer[PrimaryHdrSize], (const char*)pu8_SecondaryHeaderData, u16_SecondaryHeaderLength);
    memcpy((char*)&pu8_Buffer[PrimaryHdrSize+u16_SecondaryHeaderLength], pu8_PacketData, u16_PacketDataLength);
    
    return PrimaryHdrSize+u16_SecondaryHeaderLength+u16_PacketDataLength;
  }
  
  
  
  /**
   * @brief Creates an Idle Space Packet and writes into the given buffer
   *
   * An Idle Frame is a Space Packet with APID 0x7ff; The content of the packet is all 0xff.
   * Idle frames are used to fill up telemetry transfer frames.
   *
   * @param pu8_Buffer           A pointer to the buffer where the idle space packet shall be stored
   * @param u32_BufferSize       The available size of the buffer
   * @param u16_SequenceCount    The 14-bit sequence counter is handeled by the calling context and
   *                             must be specific also for idle frames.
   * @param u16_TargetPacketSize The requested size of the packet (usually the remaining size within
   *                             the transfer frame)
   *
   * @retval 0  No packet could be created
   * @return The size of the created packet in bytes as uint32_t
   */
  uint32_t SpacePacket::createIdle(uint8_t *pu8_Buffer, const uint32_t u32_BufferSize,
                                   const uint16_t u16_SequenceCount, const uint16_t u16_TargetPacketSize)
  {
    if(!pu8_Buffer || (u32_BufferSize<u16_TargetPacketSize)
       || (u16_TargetPacketSize<PrimaryHdrSize+1))
      return 0;
    
    // create primary header
    _create_primary_header(pu8_Buffer, SpacePacket::TM, SpacePacket::Unsegmented, 0x7ff, u16_SequenceCount,
                           false, (uint32_t)u16_TargetPacketSize-PrimaryHdrSize);
    
    memset((char*)&pu8_Buffer[PrimaryHdrSize], 0xff, u16_TargetPacketSize-PrimaryHdrSize);
    
    return u16_TargetPacketSize;
  }
  
  
  
  int32_t SpacePacket::_create_primary_header(uint8_t *pu8_Buffer,
                                              const PacketType e_PacketType, const SequenceFlags e_SequenceFlags, const uint16_t u16_APID, const uint16_t u16_SequenceCount, const bool b_SecHeader,
                                              const uint32_t u32_PacketDataLength)
  {
    pu8_Buffer[0] = (uint8_t)(((SpPacketVersion&0x7)<<5) | ((e_PacketType&0x1)<<4) | ((b_SecHeader?1:0)<<3) | ((u16_APID>>8)&0x7));
    pu8_Buffer[1] = (uint8_t)(u16_APID&0xff);
    pu8_Buffer[2] = (uint8_t)(((e_SequenceFlags&0x3)<<6) | ((u16_SequenceCount>>8)&0x3f));
    pu8_Buffer[3] = (uint8_t)(u16_SequenceCount&0xff);
    pu8_Buffer[4] = (uint8_t)((u32_PacketDataLength-1)>>8);
    pu8_Buffer[5] = (uint8_t)((u32_PacketDataLength-1)&0xff);
    
    return 0;
  }
  
  
  
  /**
   * @brief Resets the scanning for a space packet.
   *
   * A partly received space packet is discarded; In this case, the SyncErrorCounter is increased.
   */
  int32_t SpacePacket::reset(void)
  {
    if((mu32_Index>0) && (mu16_SyncErrorCount<0xffff))
      mu16_SyncErrorCount++;
    mu32_Index = 0;
    mb_Overflow = false;
    return 0;
  }
  
  
  
  /**
   * @brief The given upstream data is parsed for SpacePackets.
   *
   * The method can handle continuously incoming data as well as complete data blocks.
   * If a complete SpacePacket is processed, the callback function p_SpCallback is called.
   *
   * @param pu8_Buffer      The data buffer which is to parse
   * @param u32_BufferSize  The size of the data buffer
   *
   * @retval  0   If the buffer was parsed
   * @retval -1   If fhe u32_BufferSize is 0 or the pu8_Buffer is nullptr
   */
  int32_t SpacePacket::process(const uint8_t *pu8_Buffer, const uint32_t u32_BufferSize)
  {
    if((u32_BufferSize==0) || !pu8_Buffer)
      return -1;
    
    for(uint32_t i=0; i<u32_BufferSize; i++)
    {
      switch(mu32_Index)
      {
        case 0:
          mu8_PacketVersionNumber = (uint8_t)((pu8_Buffer[i]&0xe0)>>5);
          me_PacketType = (PacketType)((pu8_Buffer[i]&0x10)>>4);
          mb_SecHdrFlag = (((pu8_Buffer[i]&0x08)>>3)==1)?true:false;
          mu16_APID = (uint16_t)((pu8_Buffer[i]&0x07)<<8);
          break;
        case 1:
          mu16_APID |= (uint16_t)pu8_Buffer[i];
          break;
        case 2:
          me_SequenceFlags = (SequenceFlags)((pu8_Buffer[i]&0xc0)>>6);;
          mu16_PacketSequenceCount = (uint16_t)((pu8_Buffer[i]&0x003F)<<8);
          break;
        case 3:
          mu16_PacketSequenceCount |= (uint16_t)(pu8_Buffer[i]);
          break;
        case 4:
          mu16_PacketDataLength = (uint16_t)(pu8_Buffer[i]<<8);
          break;
        case 5:
          mu16_PacketDataLength |= (uint16_t)(pu8_Buffer[i]);
          break;
        default:
          if(mu32_Index-SP_HEADER_SIZE<SP_MAX_DATA_SIZE)
          {
            au8_PacketData[mu32_Index-PrimaryHdrSize]=pu8_Buffer[i];
          }
          else
          {
            if(!mb_Overflow && mu16_OverflowErrorCount<0xffff)
              mu16_OverflowErrorCount++;
            mb_Overflow = true;
          }
          break;
      }
      mu32_Index++;
      if((mu32_Index>=SP_HEADER_SIZE) && (mu32_Index>=(SP_HEADER_SIZE+mu16_PacketDataLength+1UL)))
      {
        if(nullptr!=mp_SpCallback)
          mp_SpCallback(mp_SpContext, me_PacketType, me_SequenceFlags, mu16_APID, mu16_PacketSequenceCount, mb_SecHdrFlag, au8_PacketData, mu16_PacketDataLength+1);
        mu32_Index = 0;
        mb_Overflow = false;
      }
    }
    return 0;
  }
  
  
  
  /**
   * @brief Returns the number of sync errors
   *
   * Sync Errors occur if the parsing engine is resettet while a space packet is
   * currently processed. The reset can only be triggered from outside (for example
   * by the transferframe class after detecting a wrong checksum or an uncontinuous
   * TF frame counter which cannot be resolved)
   *
   * If the number of sync errors exceeds 65535, the method returns 65535.
   *
   * @return Number of sync errors as uint16_t
   */
  uint16_t SpacePacket::getSyncErrorCount(void)
  {
    return mu16_SyncErrorCount;
  }
  
  
  
  /**
   * @brief Returns the number of overflow errors
   *
   * Overflow errors occur if the space packet is bigger than the internal
   * memory of this class used for storing the space packet;
   * A wrong package alignment or a incorrect size given within the space packet
   * can also lead to an overflow error.
   *
   * If the number of overflow errors exceeds 65535, the method returns 65535.
   *
   * @return Number of overflow errors as uint16_t
   */
  uint16_t SpacePacket::getOverflowErrorCount(void)
  {
    return mu16_OverflowErrorCount;
  }
  
  
  
  /**
   * @brief Clears all error counters (Sync Error and Overflow Error)
   */
  void SpacePacket::clearErrorCounters(void)
  {
    mu16_SyncErrorCount=0;
    mu16_OverflowErrorCount=0;
    return;
  }
  
  
}

