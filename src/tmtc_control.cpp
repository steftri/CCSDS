/**
 * @file      tmtc_control.cpp
 *
 * @brief     Source file of the TMTC control class
 *
 * @author    Stefan Trippler
 *
 * @copyright Copyright (C) 2021-2022 Stefan Trippler.  All rights reserved.
 */

#include <string.h>

#include "tmtc_control.h"


using namespace CCSDS;

// for debug:
//#include <cstdio>
//#include <iostream>
//using namespace std; 




TmTcControl::TmTcControl(const uint16_t *pu16_SCIDs, const uint8_t u8_NumberOfSCIDs,
                         void *p_TfTcContext, TTfTcCallback *p_TfTcCallback,
                         void *p_VC0SpContext, SpacePacket::TSpCallback *p_VC0SpCallback)
{
  
  mu8_TmMCFC = 0;
  memset(mau8_TmVCFC, 0, sizeof(mau8_TmVCFC));
  
  m_TfTm.setCallback((void*)this, TmTcControl::TfTmCallback);
  
  if(MaxTmChannels>0)
  {
    ma_TmCOP[0].p_TfTmOcfContext = nullptr;
    ma_TmCOP[0].p_TfTmOcfCallback = nullptr;
    ma_Sp[0].setCallback(p_VC0SpContext, p_VC0SpCallback);
  }
  for(uint8_t i=1; i<MaxTmChannels; i++)
  {
    ma_TmCOP[i].p_TfTmOcfContext = nullptr;
    ma_TmCOP[i].p_TfTmOcfCallback = nullptr;
    ma_Sp[i].setCallback(nullptr, nullptr);
  }
  
  for(uint8_t i=0; i<MaxTcChannels; i++)
  {
    mau8_FrameSeqNumber[i] = 0;  
    ma_COP[i].u8_FarmBCounter = 0;
    ma_COP[i].u8_NextFrameSeqNumber = 0;
    ma_COP[i].b_NoRfAvail = false;
    ma_COP[i].b_NoBitLock = true;
    ma_COP[i].b_LockOut = false;
    ma_COP[i].b_Wait = false;
    ma_COP[i].b_Retransmit = false;
  }
  
  mu8_NumberOfSCIDs = (u8_NumberOfSCIDs>TMTC_MAX_SCIDS)?TMTC_MAX_SCIDS:u8_NumberOfSCIDs;
  for(uint8_t i=0; i<mu8_NumberOfSCIDs; i++)
    mau16_SCIDs[i] = pu16_SCIDs[i];  
  
  mp_TfTcContext = p_TfTcContext;
  mp_TfTcCallback = p_TfTcCallback;
  
  mu16_ScIdErrorCount = 0;  
  mu16_VCFCErrorCount = 0;  
  mu16_MCFCErrorCount = 0;  
}




int32_t TmTcControl::setSCIDs(const uint16_t *pu16_SCIDs, const uint8_t u8_NumberOfSCIDs)
{
  mu8_NumberOfSCIDs = (u8_NumberOfSCIDs>TMTC_MAX_SCIDS)?TMTC_MAX_SCIDS:u8_NumberOfSCIDs;
  for(uint8_t i=0; i<mu8_NumberOfSCIDs; i++)
    mau16_SCIDs[i] = pu16_SCIDs[i];
  
  return 0; 
}




int32_t TmTcControl::setTcCallback(void *p_TfTcContext, TTfTcCallback *p_TfTcCallback)
{
  mp_TfTcContext = p_TfTcContext;
  mp_TfTcCallback = p_TfTcCallback;
  
  return 0;
}


int32_t TmTcControl::setTmOcfCallback(const uint8_t u8_VirtualChannelID, void *p_TfTmContext, TTfTmOcfCallback *p_TfTmOcfCallback)
{
  if(u8_VirtualChannelID>=MaxTmChannels)
    return -1;
  
  ma_TmCOP[u8_VirtualChannelID].p_TfTmOcfContext = p_TfTmContext;
  ma_TmCOP[u8_VirtualChannelID].p_TfTmOcfCallback = p_TfTmOcfCallback;
  
  return 0;
}



int32_t TmTcControl::setTmCallback(const uint8_t u8_VirtualChannelID,
                                   void *p_SpContext, SpacePacket::TSpCallback *p_SpCallback)
{
  if(u8_VirtualChannelID>=MaxTmChannels)
    return -1;
  
  ma_Sp[u8_VirtualChannelID].setCallback(p_SpContext, p_SpCallback);
  return 0;
}



uint16_t TmTcControl::getTmScIdErrorCount(void)
{
  return mu16_ScIdErrorCount;
}


uint16_t TmTcControl::getTmVCFCErrorCount(void)
{
  return mu16_VCFCErrorCount;
}


uint16_t TmTcControl::getTmMCFCErrorCount(void)
{
  return mu16_MCFCErrorCount;
}


uint16_t TmTcControl::getTmSyncErrorCount(void)
{
  return m_TfTm.getSyncErrorCount();
}


uint16_t TmTcControl::getTmChecksumErrorCount(void)
{
  return m_TfTm.getChecksumErrorCount();
}


uint16_t TmTcControl::getTmOverflowErrorCount(void)
{
  return m_TfTm.getOverflowErrorCount();
}




void TmTcControl::clearErrorCounters(void)
{
  mu16_ScIdErrorCount = 0;  
  mu16_VCFCErrorCount = 0;  
  mu16_MCFCErrorCount = 0;
  m_TfTm.clearErrorCounters();
}





void TmTcControl::processTfTm(const uint8_t *pu8_Data, const uint16_t u16_DataSize)
{
  m_TfTm.process(pu8_Data, u16_DataSize);
}



void TmTcControl::TfTmCallback(void *p_Context, const uint16_t u16_SpacecraftID, const uint8_t u8_VirtualChannelID,
                               const uint8_t u8_MasterChannelFrameCount, const uint8_t u8_VirtualChannelFrameCount,
                               const bool b_TFSecHdrFlag, const uint16_t u16_FirstHdrPtr,
                               const uint8_t *pu8_Data, const uint16_t u16_DataSize,
                               const uint32_t u32_OCF)
{
  return reinterpret_cast<TmTcControl*>(p_Context)->_TfTmCallback(u16_SpacecraftID, u8_VirtualChannelID,
                                                                  u8_MasterChannelFrameCount, u8_VirtualChannelFrameCount,
                                                                  b_TFSecHdrFlag, u16_FirstHdrPtr,
                                                                  pu8_Data, u16_DataSize,
                                                                  u32_OCF);
}



void TmTcControl::_TfTmCallback(const uint16_t u16_SpacecraftID, const uint8_t u8_VirtualChannelID,
                                const uint8_t u8_MasterChannelFrameCount, const uint8_t u8_VirtualChannelFrameCount,
                                const bool b_TFSecHdrFlag, const uint16_t u16_FirstHdrPtr,
                                const uint8_t *pu8_Data, const uint16_t u16_DataSize,
                                const uint32_t u32_OCF)
{
  bool b_SpacecraftIdValid=false;
  
  // first, check Spacecraft ID
  for(uint8_t i=0; i<mu8_NumberOfSCIDs; i++)
  {
    //  cout << u16_SpacecraftID << "==" << mau16_SCIDs[i] << "?" << endl;
    if(u16_SpacecraftID == mau16_SCIDs[i])
      b_SpacecraftIdValid = true;
  }
  if(!b_SpacecraftIdValid)
  {
    if(mu16_ScIdErrorCount<0xffff)
      mu16_ScIdErrorCount++;
    return;
  }   
  
  if(u8_VirtualChannelID<MaxTcChannels)
  {
    Clcw::extract(nullptr, nullptr, &ma_COP[u8_VirtualChannelID].b_NoRfAvail, &ma_COP[u8_VirtualChannelID].b_NoBitLock, 
                  &ma_COP[u8_VirtualChannelID].b_LockOut, &ma_COP[u8_VirtualChannelID].b_Wait, &ma_COP[u8_VirtualChannelID].b_Retransmit,
                  &ma_COP[u8_VirtualChannelID].u8_FarmBCounter, &ma_COP[u8_VirtualChannelID].u8_NextFrameSeqNumber, u32_OCF);
  }
  
  if(mu8_TmMCFC!=u8_MasterChannelFrameCount)
  {
    if(mu16_MCFCErrorCount<0xffff)
      mu16_MCFCErrorCount++;
    mu8_TmMCFC=(u8_MasterChannelFrameCount+1)&0xff;   
  }
  if(u8_VirtualChannelID<MaxTcChannels)
  {
    if(mau8_TmVCFC[u8_VirtualChannelID]!=u8_VirtualChannelFrameCount)
    {
      if(mu16_VCFCErrorCount<0xffff)
        mu16_VCFCErrorCount++;
      mau8_TmVCFC[u8_VirtualChannelID]=(u8_VirtualChannelFrameCount+1)&0xff;   
    }
  }
  
  if(u8_VirtualChannelID<MaxTmChannels)
  {
    if(ma_TmCOP[u8_VirtualChannelID].p_TfTmOcfCallback!=nullptr)
      ma_TmCOP[u8_VirtualChannelID].p_TfTmOcfCallback(u8_VirtualChannelID, ma_TmCOP[u8_VirtualChannelID].p_TfTmOcfContext, u32_OCF);
    ma_Sp[u8_VirtualChannelID].process(pu8_Data, u16_DataSize);
  }
  return;
}










int32_t TmTcControl::sendTc(const uint8_t u8_VirtualChannelID, const bool b_BypassFlag, const uint16_t u16_APID, const uint16_t u16_ApidSeqNr,
                            const uint8_t *pu8_Data, const uint16_t u16_DataSize)
{
  uint32_t u32_SpPacketSize = 0;
  
  if(u8_VirtualChannelID>=MaxTcChannels)
    return -1;
  
  // a space packet must be created with the data to be transfered
  u32_SpPacketSize=SpacePacket::create(mau8_TcSpBuffer, SP_MAX_DATA_SIZE,
                                       SpacePacket::TC, SpacePacket::Unsegmented, u16_APID, u16_ApidSeqNr,
                                       pu8_Data, u16_DataSize);
  
  _createAndSendTf(u8_VirtualChannelID, b_BypassFlag, false, mau8_TcSpBuffer, u32_SpPacketSize);
  
  return 0;
}




int32_t TmTcControl::sendInitAD(const uint8_t u8_VirtualChannelID)
{
  static const uint8_t au8_SetV[] = {0x82, 0x00, 0x00};
  static const uint8_t au8_Unlock[] = {0x00};
  
  if(u8_VirtualChannelID>=MaxTcChannels)
    return -1;
  
  mau8_FrameSeqNumber[u8_VirtualChannelID]=0;
  
  _createAndSendTf(u8_VirtualChannelID, true, true, au8_SetV, sizeof(au8_SetV));
  _createAndSendTf(u8_VirtualChannelID, true, true, au8_Unlock, sizeof(au8_Unlock));
  
  return 0;
}




void TmTcControl::_createAndSendTf(const uint8_t u8_VirtualChannelID, const bool b_BypassFlag, const bool b_CtrlCmdFlag, const uint8_t *pu8_Data, const uint16_t u16_DataSize)
{
  uint32_t u32_TfTcSize = 0;
#if USE_CLTU_SUPPORT == 1
  uint32_t u32_CltuSize = 0;
#endif    
  // now, we can create the transfer frame
  u32_TfTcSize = TransferframeTc::create(mau8_TfTcBuffer, sizeof(mau8_TfTcBuffer), 
                                         b_BypassFlag, b_CtrlCmdFlag,
                                         mau16_SCIDs[0], u8_VirtualChannelID, 
                                         mau8_FrameSeqNumber[u8_VirtualChannelID],
                                         pu8_Data, u16_DataSize);
  
  if(!b_BypassFlag && !b_CtrlCmdFlag)
    mau8_FrameSeqNumber[u8_VirtualChannelID]++;
  
#if USE_CLTU_SUPPORT == 1
  u32_CltuSize = Cltu::create(mau8_CltuBuffer, sizeof(mau8_CltuBuffer), mau8_TfTcBuffer, u32_TfTcSize);
#endif    
  
  if(mp_TfTcCallback)
  {
#if USE_CLTU_SUPPORT == 1
    mp_TfTcCallback(mp_TfTcContext, mau8_CltuBuffer, u32_CltuSize);
#else
    mp_TfTcCallback(mp_TfTcContext, (uint8_t*)"\x1a\xcf\xfc\x1d", TF_SYNC_SIZE);
    mp_TfTcCallback(mp_TfTcContext, mau8_TfTcBuffer, u32_TfTcSize);
#endif    
  }
  
  return;
}

