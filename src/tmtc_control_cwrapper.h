/**
 * @file      tmtc_control_cwrapper.h
 *
 * @brief     Include file of the TMTC control class c-wrapper
 *
 * @author    Stefan Trippler
 *
 * @copyright Copyright (C) 2021-2022 Stefan Trippler.  All rights reserved.
 */


#ifndef _TMTC_CONTROL_CWRAPPER_H_
#define _TMTC_CONTROL_CWRAPPER_H_

#include <inttypes.h>

#include "configCCSDS.h"


typedef enum { TM = 0, TC = 1 } ESpPacketType;

typedef enum { ContinuationSegment = 0x0, FirstSegment = 0x1, LastSegment = 0x2, Unsegmented = 0x3 } ESpSequenceFlags;


typedef void (TTfTcCallback)(void *p_Context, const uint8_t *pu8_Data, const uint16_t u16_DataSize);

typedef void (TTfTmOcfCallback)(const uint8_t u8_VirtualChannelID, void *p_Context, const uint32_t u32_Ocf);

typedef void (TSpCallback)(void *p_SpContext, const uint8_t u8_PacketType,
                             const uint8_t u8_SequenceFlags, const uint16_t u16_APID,
                             const uint16_t u16_SequenceCount, const bool b_SecHeader,
                             const uint8_t *pu8_PacketData, const uint16_t u16_PacketDataLength);


#ifdef __cplusplus
extern "C"
{
#endif


void tmtc_control_init(const uint16_t *pu16_SCIDs, const uint8_t u8_NumberOfSCIDs,
                       void *p_TfTcContext, TTfTcCallback *p_TfTcCallback,
                       void *p_VC0SpContext, TSpCallback *p_VC0SpCallback);


void tmtc_control_set_tm_callback(const uint8_t u8_VirtualChannelID,
                                  void *p_SpContext, TSpCallback *p_SpCallback);

void tmtc_control_set_tm_ocf_callback(const uint8_t u8_VirtualChannelID, void *p_TfTmOcfContext, TTfTmOcfCallback *p_TfTmOcfCallback);

uint16_t tmtc_control_get_tm_scid_error_count(void);

uint16_t tmtc_control_get_tm_vcfc_error_count(void);

uint16_t tmtc_control_get_tm_mcfc_error_count(void);

uint16_t tmtc_control_get_tm_sync_error_count(void);

uint16_t tmtc_control_get_tm_checksum_error_count(void);

uint16_t tmtc_control_get_tm_overflow_error_count(void);

void tmtc_control_clear_error_counters(void);




void tmtc_control_process_tf_tm(const uint8_t *pu8_Data, const uint16_t u16_DataSize);


void tmtc_control_set_tf_tm_callback(void *p_Context, const uint16_t u16_SpacecraftID, const uint8_t u8_VirtualChannelID,
                 const uint8_t u8_MasterChannelFrameCount, const uint8_t u8_VirtualChannelFrameCount,
                 const bool b_TFSecHdrFlag, const uint16_t u16_FirstHdrPtr,
                 const uint8_t *pu8_Data, const uint16_t u16_DataSize,
                 const uint32_t u32_OCF);





int32_t tmtc_control_send_tc(const uint8_t u8_VirtualChannelID, const bool b_BypassFlag, const uint16_t u16_APID, const uint16_t u16_ApidSeqNr,
                            const uint8_t *pu8_Data, const uint16_t u16_DataSize);




int32_t tmtc_control_send_init_ad(const uint8_t u8_VirtualChannelID);


#ifdef __cplusplus
} // extern "C"
#endif


#endif /* _TMTC_CONTROL_CWRAPPER_H_ */
