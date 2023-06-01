/**
 * @file      tmtc_control.h
 *
 * @brief     Include file of the TMTC control class
 *
 * @author    Stefan Trippler
 *
 * @copyright Copyright (C) 2021-2022 Stefan Trippler.  All rights reserved.
 */


#ifndef _TMTC_CONTROL_H_
#define _TMTC_CONTROL_H_

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


class TmTcControl
{
  // Callback for TC for sending to reveicer
  typedef void (TTfTcCallback)(void *p_Context, const uint8_t *pu8_Data, const uint16_t u16_DataSize);
  typedef void (TTfTmOcfCallback)(const uint8_t u8_VirtualChannelID, void *p_Context, const uint32_t u32_Ocf);
  
public:
  static const uint8_t MaxTcChannels = TMTC_MAX_TC_CHANNELS;
  static const uint8_t MaxTmChannels = TMTC_MAX_TM_CHANNELS;
  
  
private:
  uint8_t mu8_NumberOfSCIDs;
  uint16_t mau16_SCIDs[TMTC_MAX_SCIDS];
  
  TransferframeTm m_TfTm;
  SpacePacket ma_Sp[MaxTmChannels]; 
  
  uint8_t mau8_TcSpBuffer[SP_MAX_DATA_SIZE];
  uint8_t mau8_TfTcBuffer[TC_TF_MAX_SIZE];
  uint8_t mau8_FrameSeqNumber[MaxTcChannels];  // AD mode: N(s) - the TC FrameNumber + 1
  
  
#if USE_CLTU_SUPPORT == 1
  uint8_t mau8_CltuBuffer[CLTU_MAX_SIZE];
#endif
  
  struct
  {
    void *p_TfTmOcfContext;
    TTfTmOcfCallback *p_TfTmOcfCallback;
  } ma_TmCOP[MaxTmChannels];
  
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
  
  // Callback for TC for sending to satellite
  void *mp_TfTcContext;
  TTfTcCallback *mp_TfTcCallback;
  
  
  
  uint16_t mu16_ScIdErrorCount;
  uint16_t mu16_VCFCErrorCount;  
  uint16_t mu16_MCFCErrorCount;  
  
  
  
public:
  TmTcControl(const uint16_t *pu16_SCIDs = nullptr, const uint8_t u8_NumberOfSCIDs = 0,
              void *p_TfTcContext = nullptr, TTfTcCallback *p_TfTcCallback = nullptr,
              void *p_VC0SpContext = nullptr, SpacePacket::TSpCallback *p_VC0SpCallback = nullptr);
  
  int32_t setSCIDs(const uint16_t *pu16_SCIDs, const uint8_t u8_NumberOfSCIDs);
  
  int32_t setTcCallback(void *p_TfTcContext, TTfTcCallback *p_TfTcCallback);
  
  int32_t setTmOcfCallback(const uint8_t u8_VirtualChannelID, void *p_TfTmContext, TTfTmOcfCallback *p_TfTmOcfCallback);
  
  int32_t setTmCallback(const uint8_t u8_VirtualChannelID,
                        void *p_SpContext, SpacePacket::TSpCallback *p_SpCallback);
  
  uint16_t getTmScIdErrorCount(void);
  uint16_t getTmMCFCErrorCount(void);
  uint16_t getTmVCFCErrorCount(void);
  uint16_t getTmSyncErrorCount(void);
  uint16_t getTmChecksumErrorCount(void);
  uint16_t getTmOverflowErrorCount(void);
  void clearErrorCounters(void);
  
  
  // Telemetry
  
public:
  void processTfTm(const uint8_t *pu8_Data, const uint16_t u16_DataSize);
  
  static void TfTmCallback(void *p_Context, const uint16_t u16_SpacecraftID, const uint8_t u8_VirtualChannelID,
                           const uint8_t u8_MasterChannelFrameCount, const uint8_t u8_VirtualChannelFrameCount,
                           const bool b_TFSecHdrFlag, const uint16_t u16_FirstHdrPtr,
                           const uint8_t *pu8_Data, const uint16_t u16_DataSize,
                           const uint32_t u32_OCF);
  
private:
  void _TfTmCallback(const uint16_t u16_SpacecraftID, const uint8_t u8_VirtualChannelID,
                     const uint8_t u8_MasterChannelFrameCount, const uint8_t u8_VirtualChannelFrameCount,
                     const bool b_TFSecHdrFlag, const uint16_t u16_FirstHdrPtr,
                     const uint8_t *pu8_Data, const uint16_t u16_DataSize,
                     const uint32_t u32_OCF);
  
  
  
  // Telecommand
  
public:  
  int32_t sendTc(const uint8_t u8_VirtualChannelID, const bool b_BypassFlag, const uint16_t u16_APID, const uint16_t u16_ApidSeqNr,
                 const uint8_t *pu8_Data, const uint16_t u16_DataSize);
  
  int32_t sendInitAD(const uint8_t u8_VirtualChannelID);
  
private:
  void _TfTmCallback(const bool b_BypassFlag, const bool b_CtrlCmdFlag,
                     const uint16_t u16_SpacecraftID, const uint8_t u8_VirtualChannelID,
                     const uint8_t u8_FrameSeqNumber,
                     const uint8_t *pu8_Data, const uint16_t u16_DataSize);
  
  void _createAndSendTf(const uint8_t u8_VirtualChannelID, const bool b_BypassFlag, const bool b_CtrlCmFlag, const uint8_t *pu8_Data, const uint16_t u16_DataSize);
  
};





#endif /* _TMTC_CONTROL_H_ */

