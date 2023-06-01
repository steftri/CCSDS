/**
 * @file      tmtc_control_cwrapper.cpp
 *
 * @brief     Source file of the TMTC control class c-wrapper
 *
 * @author    Stefan Trippler
 *
 * @copyright Copyright (C) 2021-2022 Stefan Trippler.  All rights reserved.
 */


#ifdef AVR
#include <Arduino.h>
#else
#include <cstdint>
#include <cstring>
#endif


#include "tmtc_control.h"
#include "tmtc_control_cwrapper.h"


using namespace CCSDS;




static TmTcControl g_TmTcControl;



extern "C" void tmtc_control_init(const uint16_t *pu16_SCIDs, const uint8_t u8_NumberOfSCIDs,
                       void *p_TfTcContext, TTfTcCallback *p_TfTcCallback,
                       void *p_VC0SpContext, TSpCallback *p_VC0SpCallback)
{
   g_TmTcControl.setSCIDs(pu16_SCIDs, u8_NumberOfSCIDs);
   g_TmTcControl.setTcCallback(p_TfTcContext, (TTfTcCallback*)p_TfTcCallback);
   g_TmTcControl.setTmCallback(0, p_VC0SpContext, (SpacePacket::TSpCallback *)p_VC0SpCallback);

   return;
}


extern "C" void tmtc_control_set_tm_ocf_callback(const uint8_t u8_VirtualChannelID, void *p_TfTmOcfContext, TTfTmOcfCallback *p_TfTmOcfCallback)
{
  g_TmTcControl.setTmOcfCallback(u8_VirtualChannelID, p_TfTmOcfContext, p_TfTmOcfCallback);
  
  return;
}


extern "C" void tmtc_control_set_tm_callback(const uint8_t u8_VirtualChannelID,
                                  void *p_SpContext, TSpCallback *p_SpCallback)
{
  g_TmTcControl.setTmCallback(u8_VirtualChannelID, p_SpContext, (SpacePacket::TSpCallback *)p_SpCallback);

  return;
}



extern "C" uint16_t tmtc_control_get_tm_scid_error_count(void)
{
  return g_TmTcControl.getTmScIdErrorCount();
}


extern "C" uint16_t tmtc_control_get_tm_vcfc_error_count(void)
{
  return g_TmTcControl.getTmVCFCErrorCount();
}


extern "C" uint16_t tmtc_control_get_tm_mcfc_error_count(void)
{
  return g_TmTcControl.getTmMCFCErrorCount();
}



extern "C" uint16_t tmtc_control_get_tm_sync_error_count(void)
{
  return g_TmTcControl.getTmSyncErrorCount();
}



extern "C" uint16_t tmtc_control_get_tm_checksum_error_count(void)
{
  return g_TmTcControl.getTmChecksumErrorCount();
}



extern "C" uint16_t tmtc_control_get_tm_overflow_error_count(void)
{
  return g_TmTcControl.getTmOverflowErrorCount();
}



extern "C" void tmtc_control_clear_error_counters(void)
{
  g_TmTcControl.clearErrorCounters();
  return;
}





extern "C" void tmtc_control_process_tf_tm(const uint8_t *pu8_Data, const uint16_t u16_DataSize)
{
  g_TmTcControl.processTfTm(pu8_Data, u16_DataSize);
  return;
}


/*
extern "C" void tmtc_control_set_tf_tm_callback(void *p_Context, const uint16_t u16_SpacecraftID, const uint8_t u8_VirtualChannelID,
                 const uint8_t u8_MasterChannelFrameCount, const uint8_t u8_VirtualChannelFrameCount,
                 const bool b_TFSecHdrFlag, const uint16_t u16_FirstHdrPtr,
                 const uint8_t *pu8_Data, const uint16_t u16_DataSize,
                 const uint32_t u32_OCF)
{
  g_TmTcControl.TfTmCallback(p_Context,u16_SpacecraftID, u8_VirtualChannelID, u8_MasterChannelFrameCount, u8_VirtualChannelFrameCount,
                 b_TFSecHdrFlag, u16_FirstHdrPtr, pu8_Data, u16_DataSize, u32_OCF);
  return;
}
*/




extern "C" int32_t tmtc_control_send_tc(const uint8_t u8_VirtualChannelID, const bool b_BypassFlag, const uint16_t u16_APID, const uint16_t u16_ApidSeqNr,
                            const uint8_t *pu8_Data, const uint16_t u16_DataSize)
{
  return g_TmTcControl.sendTc(u8_VirtualChannelID, b_BypassFlag, u16_APID, u16_ApidSeqNr,
                            pu8_Data, u16_DataSize);
}




extern "C" int32_t tmtc_control_send_init_ad(const uint8_t u8_VirtualChannelID)
{
  return g_TmTcControl.sendInitAD(u8_VirtualChannelID);
}



