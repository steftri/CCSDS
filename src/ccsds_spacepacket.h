/**
 * @file      ccsds_spacepacket.h
 *
 * @brief     Include file of the Space Packet (SP) class
 *
 * @author    Stefan Trippler
 *
 * @copyright Copyright (C) 2021-2022 Stefan Trippler.  All rights reserved.
 */


#ifndef _CCSDS_SPACEPACKET_H_
#define _CCSDS_SPACEPACKET_H_

/****************************************************************/
/* SpacePackets according to                                    */
/*                                                              */
/*  - CCSDS 133.0-B-2, - Space Packet Protocol                  */
/*    https://public.ccsds.org/Pubs/133x0b2e1.pdf               */
/*                                                              */
/* Limitations:                                                 */ 
/*  - CCSDS secondary header format is not supportd             */
/*                                                              */
/****************************************************************/

#include <inttypes.h>

#include "configCCSDS.h"

#ifdef configSP_MAX_DATA_SIZE
#define SP_MAX_DATA_SIZE configSP_MAX_DATA_SIZE
#else 
#define SP_MAX_DATA_SIZE 496
#endif



#define SP_HEADER_SIZE     6
#define SP_MAX_TOTAL_SIZE  (SP_HEADER_SIZE+SP_MAX_DATA_SIZE)



namespace CCSDS 
{
  
  /**
   * @brief Class for handling the Space Packets as described in CCSDS 133.0-B-2.
   *
   * Space Packets are used to transfer data from one specific application to another which
   * are identified by the Application ID (APID), corresponding to OSI layer 4. Even thouth
   * Space Packets have a sequence counter, there is no mechanism for resending packets.
   * This means, if a connection is expected to be disturbed and unreliable, Space Packes
   * should be wrapped in Transfer Frames.
   *
   * The size of the Space Package may vary and depends on the information which are to be transfered.
   * The maximum size is limmited to 65536 bytes by the protocol, but typically the maximum size
   * is configured to be much smaller, depending on the needs and the available memory for processing
   * the packages on the controller board.
   */
  class SpacePacket
  {
  public:
    static const int MaxSize = SP_MAX_TOTAL_SIZE;     /**< Maximum size of a space packet including header(s) */
    static const int MaxDataSize = SP_MAX_DATA_SIZE;  /**< Maximum size of the data section of a space packet */

    
    /** The package type defines if a space packet contains
     *  a telecommand (TC) or telemetry (TM).
     */
    enum PacketType
    {
      TM = 0, /**< Telemetry (downlink) */
      TC = 1  /**< Telecommand (uplink) */
    };
    
    
    /** If the data to be sent is more than it can be handeled in one single
     *  space packet, than these flags are used to identify the first and the last package.
     */
    enum SequenceFlags
    {
      ContinuationSegment = 0x0,  /**< If the package belongs to a sequence of packages, this flag identifies that this package is somewhere in the middle. */
      FirstSegment = 0x1,         /**< The first package of a sequence of packages mus be identified with this flag. */
      LastSegment = 0x2,          /**< This flag identifies the last package of a sequence. */
      Unsegmented = 0x3           /**< If the package is completly within one source packet, this flag is used. */
    };
    
    
    /**
     * @typedef TSpCallback
     * @brief Declaration of the callback which shall be called if a complete space packet was received
     *
     * The implementation of this callback shall handle the space packet. It shall implement the fowarding to the
     * corresponding application (or the further processing of the content.
     *
     * @param p_SpContext         The pointer to the context which has to be used
     * @param u8_PacketType       The packet type (TM or TC)
     * @param u8_SequenceFlags    The sequence flags (Continuation, First, Last Segment or Unsegmented)
     * @param u16_APID            The target application ID
     * @param u16_SequenceCount   The channel-specific frame count
     * @param b_SecHeader         A flag which indicates the presence of a secondary space packet header
     * @param pu8_PacketData      A pointer to the data block which holds the content of the package
     * @param u16_PacketDataLength The size of the data block in bytes
     */
    typedef void (TSpCallback)(void *p_SpContext, const uint8_t u8_PacketType,
                               const uint8_t u8_SequenceFlags, const uint16_t u16_APID,
                               const uint16_t u16_SequenceCount, const bool b_SecHeader,
                               const uint8_t *pu8_PacketData, const uint16_t u16_PacketDataLength);
    
  private:
    const static uint8_t SpPacketVersion = 0;
    const static uint8_t PrimaryHdrSize = SP_HEADER_SIZE;
    
    uint32_t mu32_Index;
    uint8_t mu8_PacketVersionNumber;
    enum PacketType me_PacketType;
    bool mb_SecHdrFlag;
    uint16_t mu16_APID;
    enum SequenceFlags me_SequenceFlags;
    uint16_t mu16_PacketSequenceCount;
    uint16_t mu16_PacketDataLength;
    uint8_t au8_PacketData[SP_MAX_DATA_SIZE];
    
    bool mb_Overflow;
    uint16_t mu16_SyncErrorCount;
    uint16_t mu16_OverflowErrorCount;
    
    void *mp_SpContext;
    TSpCallback *mp_SpCallback;
    
    
  public:
    SpacePacket(void *p_SpContext = nullptr, TSpCallback *mp_SpCallback = nullptr);
    
    void setCallback(void *p_SpContext, TSpCallback *mp_SpCallback);
    
    
    // SP generation
    static uint32_t create(uint8_t *pu8_Buffer, const uint32_t u32_BufferSize,
                           const PacketType e_PacketType, const SequenceFlags e_SequenceFlags, const uint16_t u16_APID, const uint16_t u16_SequenceCount,
                           const uint8_t *pu8_PacketData, const uint16_t u16_PacketDataLength);   // without secondary header
    
    static uint32_t create(uint8_t *pu8_Buffer, const uint32_t u32_BufferSize,
                           const PacketType e_PacketType, const SequenceFlags e_SequenceFlags, const uint16_t u16_APID, const uint16_t u16_SequenceCount,
                           const uint8_t *pu8_SecondaryHeaderData, const uint16_t u16_SecondaryHeaderLength,
                           const uint8_t *pu8_PacketData, const uint16_t u16_PacketDataLength);
    
    static uint32_t createIdle(uint8_t *pu8_Buffer, const uint32_t u32_BufferSize,
                               const uint16_t u16_SequenceCount, const uint16_t u16_TargetPacketSize);
    
    // sp processing
    int32_t process(const uint8_t *pu8_Buffer, const uint32_t u32_BufferSize);
    int32_t reset(void);
    
    uint16_t getSyncErrorCount(void);
    uint16_t getOverflowErrorCount(void);
    void clearErrorCounters(void);
    
  private:
    static int32_t _create_primary_header(uint8_t *pu8_Buffer,
                                          const PacketType e_PacketType, const SequenceFlags e_SequenceFlags, const uint16_t u16_APID, const uint16_t u16_SequenceCount, const bool b_SecHeader,
                                          const uint32_t u32_PacketDataLength);
  };
  
}

#endif // _CCSDS_SPACEPACKET_H_
