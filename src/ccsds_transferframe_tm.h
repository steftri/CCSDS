/**
 * @file      ccsds_transferframe_tm.h
 *
 * @brief     Include file of the Transfer Frame (TM) class
 *
 * @author    Stefan Trippler
 *
 * @copyright Copyright (C) 2021-2022 Stefan Trippler.  All rights reserved.
 */

#ifndef _CCSDS_TRANSFERFRAME_TM_H_
#define _CCSDS_TRANSFERFRAME_TM_H_

/****************************************************************/
/* TM Transferframes according to                               */
/*                                                              */
/*  - CCSDS 131.0-B-3, - TM Synchronization and Channel Coding  */
/*    https://public.ccsds.org/Pubs/131x0b3e1.pdf               */
/*  - CCSDS 132.0-B-2, - TM Space Data Link Protocol            */
/*    https://public.ccsds.org/Pubs/132x0b2.pdf                 */
/*                                                              */
/* Limitations:                                                 */ 
/*  - The TM secondary header is not supported                  */
/*  - Randomization is not supported                            */
/*                                                              */
/****************************************************************/

#include <inttypes.h>

#include "configCCSDS.h"

#ifdef configTM_TF_TOTAL_SIZE  
#define TM_TF_TOTAL_SIZE configTM_TF_TOTAL_SIZE
#else 
#define TM_TF_TOTAL_SIZE 508
#endif

#ifdef configTF_USE_OCF
#define TF_USE_OCF configTF_USE_OCF
#else
#define TF_USE_OCF 1
#endif


#include "ccsds_transferframe.h"


namespace CCSDS
{
  
  
  /**
   * @brief Class for handling the Transfer Frames for Telemetry as described in CCSDS 132.0-B-2.
   *
   * Transfer Frames in general are used to ensure a transfer from the ground to the spacecraft and
   * vice versa, corresponding to OSI layer 2.
   *
   * For downlink data, Transfer Frames with a fixed size are used. These Transfer Frames also include
   * the Operational Control Field (OCF), which is part of the flow control mechanism for uplink data.
   * So this field usually holds the Communications Link Control Word (CLCW) with the information about
   * the uplink signal status or the FARM counters.
   *
   * With respect to the specification, this class does *not* add a synchronization sequence to the head
   * of the transfer frame. These synchronization sequence (usually 0x1ACFFC1D) must be added before
   * sending the transfer frame to the ground is done.
   *
   * With the Transfer Frame protocol, virtual channels (0 to 7) are supported. The virtual channels can
   * be used for different sub systems within one spacecraft or different purposes. Channel 0 is usually
   * used for real-time data of the main system. Channel 1 could be used for historic data, channels 2-6
   * could be used for payload systems. Virtual Channel 7 is usually reserved for IDLE data to fill up
   * the data stream from the satellite to be continuous.
   *
   * This class inherits from the Transfer Frame base class and is responsible for handling the part
   * which is needed to create and process telemetry or downlink data.
   *
   * The size of the Transfer Frame is fix within a mission. Since source packets can be larger,
   * sequence flags are used for segmentation of the downlink data. In case the included source packet is
   * smaller than the available space in the transfer frame, the frame must be filled up with idle
   * source packets.
   */
  class TransferframeTm : public Transferframe
  {
  public:
    
    /**
     * @typedef TTmCallback
     * @brief Declaration of the callback which shall be called if a complete telemetry transfer frame was received
     *
     * The implementation of this callback shall handle the telemetry transfer frame. It shall implement the
     * Frame Acceptance and Reporting Mechanism (FARM) as well as further processing of the embedded
     * protocol (usually space packets).
     *
     * @param p_Context           The pointer to the context which has to be used
     * @param u16_SpacecraftID    The souce spacecraft ID
     * @param u8_VirtualChannelID The virtual channel ID
     * @param u8_MasterChannelFrameCount  The overall frame count over all virtual channels
     * @param u8_VirtualChannelFrameCount The channel-specific frame count
     * @param b_TFSecHdrFlag      A flag which indicates the presence of a secondary transfer frame header
     * @param u16_FirstHdrPtr     Offset of the first space packet within the data section
     * @param pu8_Data            A pointer to the data block which holds the content of the package
     * @param u16_DataSize        The size of the data block in bytes
     * @param u32_OCF             The Operational Control Field (OCF), which is part of the flow control
     *                            mechanism for uplink data (can hold the Communications Link Control Word (CLCW))
     */
    typedef void (TTmCallback)(void *p_Context, const uint16_t u16_SpacecraftID, const uint8_t u8_VirtualChannelID,
                               const uint8_t u8_MasterChannelFrameCount, const uint8_t u8_VirtualChannelFrameCount,
                               const bool b_TFSecHdrFlag, const uint16_t u16_FirstHdrPtr,
                               const uint8_t *pu8_Data, const uint16_t u16_DataSize,
                               const uint32_t u32_OCF);
    
    
  private:
    static const int TmTfVersionNumber = 0;
    const static uint8_t PrimaryHdrSize = 6;
    const static uint8_t OcfSize = 4;
    const static uint16_t TfSize = TM_TF_TOTAL_SIZE;
    uint8_t mau8_Buffer[TfSize];
    
    const static bool UseOCF = (TF_USE_OCF)?true:false;  // Operational Control Field (CLCW)
    
    void *mp_TmContext;
    TTmCallback *mp_TmCallback;
    
  public:
    TransferframeTm(void *p_TmContext = nullptr, TTmCallback *mp_TmCallback = nullptr);
    
    void setCallback(void *p_TmContext, TTmCallback *mp_TmCallback);
    
    
    // TM generation
    static uint32_t create(uint8_t *pu8_Buffer, const uint32_t u32_BufferSize,
                           const uint16_t u16_SpacecraftID, const uint8_t u8_VirtualChannelID,
                           const uint8_t u8_MasterChannelFrameCount, const uint8_t u8_VirtualChannelFrameCount,
                           const uint16_t u16_FirstHdrPtr,
                           const uint8_t *pu8_Data, const uint16_t u16_DataSize,
                           const uint32_t u32_OCF = 0);
    
    static uint32_t createIdle(uint8_t *pu8_Buffer, const uint32_t u32_BufferSize,
                               const uint16_t u16_SpacecraftID, const uint8_t u8_VirtualChannelID,
                               const uint8_t u8_MasterChannelFrameCount, const uint8_t u8_VirtualChannelFrameCount,
                               const uint32_t u32_OCF = 0);
    
  private:
    static int32_t _createPrimaryHeader(uint8_t *pu8_Buffer,
                                        const uint16_t u16_SpacecraftID, const uint8_t u8_VirtualChannelID, const bool b_OcfFlag,
                                        const uint8_t u8_MasterChannelFrameCount, const uint8_t u8_VirtualChannelFrameCount,
                                        const bool b_TFSecHdrFlag, const bool b_SyncFlag, const bool b_PacketOrderFlag,
                                        const uint8_t u8_SegLengthID, const uint16_t u16_FirstHdrPtr);
    
    int32_t _processFrame(void);
    
  private:
    inline uint16_t _getMaxTfSize(void);
    inline uint8_t *_getTfBufferAddr(void);
    inline uint16_t _getPrimaryHeaderSize(void);
    inline void _getFrameLength(void);
  };
  
}


#endif // _CCSDS_TRANSFERFRAME_TM_H_
