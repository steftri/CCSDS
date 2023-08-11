/**
 * @file      tmtc_client.h
 *
 * @brief     Include file of the TMTC client class
 *
 * @author    Stefan Trippler
 *
 * @copyright Copyright (C) 2021-2022 Stefan Trippler.  All rights reserved.
 */


#ifndef _TMTC_CLIENT_H_
#define _TMTC_CLIENT_H_

#include <inttypes.h>

#include "configCCSDS.h"


#include "ccsds_cltu.h"
#include "ccsds_transferframe_tc.h"
#include "ccsds_transferframe_tm.h"
#include "ccsds_clcw.h"
#include "ccsds_spacepacket.h"



#ifdef configTMTC_MAX_TC_CHANNELS
#define TMTC_MAX_TC_CHANNELS configTMTC_MAX_TC_CHANNELS
#else
#define TMTC_MAX_TC_CHANNELS 1
#endif

#ifdef configTMTC_MAX_TM_CHANNELS
#define TMTC_MAX_TM_CHANNELS configTMTC_MAX_TM_CHANNELS
#else
#define TMTC_MAX_TM_CHANNELS 8
#endif

#ifdef configUSE_CLTU_SUPPORT
#define USE_CLTU_SUPPORT configUSE_CLTU_SUPPORT
#else
#define USE_CLTU_SUPPORT 0
#endif

#ifdef configTMTC_MAX_SCIDS
#define TMTC_MAX_SCIDS configTMTC_MAX_SCIDS
#else
#define TMTC_MAX_SCIDS 2
#endif

#ifdef configFARM_SLIDING_WINDOW_WIDTH
#define FARM_SLIDING_WINDOW_WIDTH configFARM_SLIDING_WINDOW_WIDTH
#else
#define FARM_SLIDING_WINDOW_WIDTH 16
#endif



using namespace CCSDS;


/**
 * @brief Interface class for handling telemetry actions
 */
class TmActionInterface
{
public:
  /**
   * @brief Declaration of the action which shall be called if a complete telemetry transferframe is ready to send
   *
   * The implementation of this callback shall send the telemetry packet.
   *
   * @param pu8_Data      A pointer to the data block which holds the data to be send
   * @param u16_DataSize  The size of the data in bytes
   */
  virtual void onTmDataCreated(const uint8_t *pu8_Data, const uint16_t u16_DataSize) = 0;
};




/**
 * @brief Class which implements a Frame Acceptance and Reporting Mechanism (FARM) on the spacecraft
 *
 * The purpose of this class is to integrate the CCSDS protocol classes and allow an easy usage of it.
 *
 * This class integrates the transferframe tc and tm classes, handles the frame counters on the downlink side
 * and implements the frame acceptance and reporting mechanism (FARM) to ensure a reliable uplink connection
 * to the spacecraft.
 *
 * For a simple example how to use this class, see the Arduino example tmtc_client_standalone.
 */
class TmTcClient 
  : private TransferframeTcActionInterface
#if USE_CLTU_SUPPORT == 1
  , private CltuActionInterface
#endif
{  
public:
  static const uint8_t MaxTcChannels = TMTC_MAX_TC_CHANNELS;
  static const uint8_t MaxTmChannels = TMTC_MAX_TM_CHANNELS;
   
private:
  uint8_t mu8_NumberOfSCIDs;
  uint16_t mau16_SCIDs[TMTC_MAX_SCIDS];
  
#if USE_CLTU_SUPPORT == 1
  Cltu m_Cltu;
#endif
  TransferframeTc m_TfTc;
  SpacePacket ma_Sp[MaxTcChannels];
  
  uint8_t mau8_TfTmBuffer[TM_TF_TOTAL_SIZE];
  uint8_t mau8_TmSpBuffer[SP_MAX_DATA_SIZE];
  
  struct
  {
    bool b_NoRfAvail;
    bool b_NoBitLock;
    bool b_LockOut;
    bool b_Wait;
    bool b_Retransmit;
    uint8_t u8_FarmBCounter; // BD mode counter
    uint8_t u8_NextFrameSeqNumber;  // AD mode: N(s) - the TC FrameNumber + 1
  } ma_COP[MaxTcChannels];
  
  uint8_t mu8_TmMCFC;
  uint8_t mau8_TmVCFC[MaxTmChannels];
  uint16_t mu16_IdleSpSequenceCount;
  
  uint16_t mu16_ScIdErrorCount;
  uint16_t mu16_VirtualChannelErrorCount;
  uint16_t mu16_RetransmitErrorCount;
  uint16_t mu16_LockoutErrorCount;
  
  TmActionInterface *mp_TmActionInterface;
  SpacePacketActionInterface *map_TcSpActionInterface[MaxTcChannels];
  
public:
  TmTcClient(const uint16_t *pu16_SCIDs, const uint8_t u8_NumberOfSCIDs,
             TmActionInterface *p_TmActionInterface = nullptr, 
             SpacePacketActionInterface *p_VC0TcActionInterface = nullptr);

  int32_t setTmActionInterface(TmActionInterface *p_ActionInterface);
  int32_t setTcActionInterface(const uint8_t u8_VirtualChannelID, SpacePacketActionInterface *p_ActionInterface);
  
  uint16_t getScIdErrorCount(void);
  uint16_t getVirtualChannelErrorCount(void);
  uint16_t getRetransmitErrorCount(void);
  uint16_t getLockoutErrorCount(void);
  void clearErrorCounters(void);
  
  // Telecommand
public:
  void setSync(void);
  void processTfTc(const uint8_t *pu8_Data, const uint16_t u16_DataSize);

  void onTransferframeTcReceived(const bool b_BypassFlag, const bool b_CtrlCmdFlag,
                                 const uint16_t u16_SpacecraftID, const uint8_t u8_VirtualChannelID,
                                 const uint8_t u8_FrameSeqNumber,
                                 const uint8_t *pu8_Data, const uint16_t u16_DataSize) override;

  
#if USE_CLTU_SUPPORT == 1
  void processCltu(const uint8_t *pu8_Data, const uint16_t u16_DataSize);

  void onStartOfTransmission(void) override;
  void onCltuDataReceived(const uint8_t *pu8_Data, const uint16_t u16_DataSize) override;
#endif
  
private:
  void _ctrlCmdUnlock(const uint8_t u8_VirtualChannelID);
  void _ctrlCmdSetV(const uint8_t u8_VirtualChannelID, const uint8_t u8_R);
  
  // Telemetry
public:  
  int32_t sendTm(const uint8_t u8_VirtualChannelID, const uint16_t u16_APID, const uint16_t u16_ApidSeqNr,
                 const uint8_t *pu8_Data, const uint16_t u16_DataSize);
  int32_t sendIdle(void);
  
private:
  void _setFrameSequenceNumber(const uint8_t mu8_TcFrameSeqNr);
};





#endif /* _TMTC_CLIENT_H_ */

