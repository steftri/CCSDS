/**
 * @file      ccsds_transferframe_tc.h
 *
 * @brief     Include file of the Transfer Frame (TC) class
 *
 * @author    Stefan Trippler
 *
 * @copyright Copyright (C) 2021-2022 Stefan Trippler.  All rights reserved.
 */

#ifndef _CCSDS_TRANSFERFRAME_TC_H_
#define _CCSDS_TRANSFERFRAME_TC_H_

/****************************************************************/
/* TC Transferframes according to                               */
/*                                                              */
/*  - CCSDS 232.0-B-3, - TC Space Data Link Protocol            */
/*    https://public.ccsds.org/Pubs/232x0b3.pdf                 */
/*                                                              */
/* Limitations:                                                 */ 
/*  - The TC segment header is not supported                    */
/*                                                              */
/* Remarks:                                                     */
/*  - the sync code 0x1acffc1d is not generated                 */
/*    by the create() method.                                   */
/*                                                              */
/****************************************************************/

#include <inttypes.h>

#include "configCCSDS.h"

#ifdef configTC_TF_MAX_SIZE  
#define TC_TF_MAX_SIZE configTC_TF_MAX_SIZE
#else
#define TC_TF_MAX_SIZE 508
#endif


#include "ccsds_transferframe.h"


namespace CCSDS 
{

  /**
   * @brief Interface class for handling transferframe tc packet actions
   */
  class TransferframeTcActionInterface
  {
  public:
    /**
     * @brief Declaration of the action which shall be called if a complete telecommand transfer frame was received
     *
     * The implementation of this callback shall handle the telecommand transfer frame. It shall implement the
     * Frame Acceptance and Reporting Mechanism (FARM) as well as further processing of the embedded
     * protocol (usually space packets) or forwarding the packet to another target.
     *
     * @param b_BypassFlag        Indicates if the command was sent in AD mode (BypassFlag is false) with
     *                           handling of the Frame Acceptance and Reporting Mechanism (FARM) or in
     *                           BD mode (BypassFlag is true, each packet shall be accepted onboard)
     * @param b_CtrlCmdFlag       Indicates if the packet is intended for the AD mode mechanism
     * @param u16_SpacecraftID    The target spacecraft ID
     * @param u8_VirtualChannelID The virtual channel ID
     * @param u8_FrameSeqNumber   The virtual channel specific frame sequence number
     * @param u8_MAP              The multiplexer access point identifier
     * @param pu8_Data            A pointer to the data block which holds the content of the package
     * @param u16_DataSize        The size of the data block in bytes
     */
    virtual void onTransferframeTcReceived(const bool b_BypassFlag, const bool b_CtrlCmdFlag,
                                           const uint16_t u16_SpacecraftID, const uint8_t u8_VirtualChannelID,
                                           const uint8_t u8_FrameSeqNumber, const uint8_t u8_MAP,
                                           const uint8_t *pu8_Data, const uint16_t u16_DataSize) = 0;
  };



  /**
   * @brief Class for handling the Transfer Frames for Telecommand as described in CCSDS 232.0-B-3.
   *
   * Transfer Frames in general are used to ensure a transfer from the ground to the spacecraft and
   * vice versa, corresponding to OSI layer 2.
   *
   * For Uplink Data (such as telecommands or software update data blocks), a flow control mechanism
   * and an error detection mechanism is implemented in this protocol. This layer does not have a
   * synchronization mechanism, so it is common to embed transfer frames in Communications
   * Link Transmission Units (CLTUs, see CCSDS 231.0-B-3).
   *
   * With the Transfer Frame protocol, virtual channels are supported. Each frame has a virtual channel
   * ID (0 up to 63, depending on the configuration), which could for e.g. address different subsystems
   * within the same spacecraft. Each virtual channel comes with its own flow control mechanism.
   * Attention: Telemetry Transfer Frames only support virtual channels 0 to 7.
   *
   * This class inherits from the Transfer Frame base class and is responsible for handling the part
   * which is needed to create and process telecommands or uplink data (configuration files, software
   * update, ...).
   *
   * The size of the Transfer Frame may vary and depends on the information which are to be transfered.
   * The maximum size is limmited to 1024 bytes by the protocol including the header and the CRC. Since
   * source packets can be larger, sequence flags are used for segmentation of the uplink data.
   */
  class TransferframeTc : public Transferframe
  {
  private:
    const static int TcTfVersionNumber = 0;
    const static uint8_t PrimaryHdrSize = 5;
    const static uint8_t SegmentHdrSize = 1;
    const static uint16_t MaxTfSize = TC_TF_MAX_SIZE;
    uint8_t mau8_Buffer[MaxTfSize];
    
    TransferframeTcActionInterface *mp_ActionInterface;

    enum ESeqFlags 
    {
      FirstPortion = 0x1, 
      ContinuingPortion = 0x0,
      LastPortion = 0x2, 
      NoSegmentation = 0x3
    };
    
  public:
    TransferframeTc(TransferframeTcActionInterface *p_ActionInterface = nullptr);
    
    void setActionInterface(TransferframeTcActionInterface *p_ActionInterface);
    
    // TC generation
    static uint32_t create(uint8_t *pu8_Buffer, const uint32_t u32_BufferSize,
                           const bool b_BypassFlag, const bool b_CtrlCmdFlag,
                           const uint16_t u16_SpacecraftID, const uint8_t u8_VirtualChannelID,
                           const uint8_t u8_FrameSeqNumber, const uint8_t u8_MAP,
                           const uint8_t *pu8_Data, const uint16_t u16_DataSize);
    
  private:
    static int32_t _createPrimaryHeader(uint8_t *pu8_Buffer,
                                        const bool b_BypassFlag, const bool b_CtrlCmdFlag,
                                        const uint16_t u16_SpacecraftID, const uint8_t u8_VirtualChannelID,
                                        const uint16_t u16_FrameLength, const uint8_t u8_FrameSeqNumber);

    static int32_t _createSegmentHeader(uint8_t *pu8_Buffer, const enum ESeqFlags e_SeqFlags, const uint8_t u8_MAP);
    
    int32_t _processFrame(void);
    
  private:
    inline uint16_t _getMaxTfSize(void);
    inline uint8_t *_getTfBufferAddr(void);
    inline uint16_t _getPrimaryHeaderSize(void);
    inline void _getFrameLength(void);
  };
    
}


#endif // _CCSDS_TRANSFERFRAME_TC_H_


