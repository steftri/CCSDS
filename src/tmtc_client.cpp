/**
 * @file      tmtc_client.cpp
 *
 * @brief     Source file of the TMTC client class
 *
 * @author    Stefan Trippler
 *
 * @copyright Copyright (C) 2021-2022 Stefan Trippler.  All rights reserved.
 */

#include <string.h>


//
//  tmtc_client.cpp
//  CCSDS
//
//  Created by Stefan on 2021-03-10.
//

#include "tmtc_client.h"


using namespace CCSDS;

// for debug:
//#include <cstdio>
//#include <iostream>
//using namespace std;




TmTcClient::TmTcClient(const uint16_t *pu16_SCIDs, const uint8_t u8_NumberOfSCIDs,
                       TmActionInterface *p_TmActionInterface, 
                       SpacePacketActionInterface *p_VC0TcActionInterface)
{
  
  mu8_TmMCFC = 0;
  memset(mau8_TmVCFC, 0, sizeof(mau8_TmVCFC));
  
#if USE_CLTU_SUPPORT == 1
  m_Cltu.setActionInterface(this);
#endif
  
  m_TfTc.setActionInterface(this);
  ma_Sp[0].setActionInterface(p_VC0TcActionInterface);
  for(uint8_t i=1; i<MaxTcChannels; i++)
    ma_Sp[i].setActionInterface(nullptr);
  
  for(uint8_t i=0; i<MaxTcChannels; i++)
  {
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
  
  mu16_IdleSpSequenceCount=0;
  
  mp_TmActionInterface = p_TmActionInterface;
  
  mu16_ScIdErrorCount = 0;  
  mu16_VirtualChannelErrorCount = 0;  
  mu16_RetransmitErrorCount = 0;  
  mu16_LockoutErrorCount = 0;    
}



int32_t TmTcClient::setTmActionInterface(TmActionInterface *p_TmActionInterface)
{
  mp_TmActionInterface = p_TmActionInterface;
  return 0;
}



int32_t TmTcClient::setTcActionInterface(const uint8_t u8_VirtualChannelID,
                                  SpacePacketActionInterface *p_ActionInterface)
{
  if(u8_VirtualChannelID>=MaxTcChannels)
    return -1;
  
  ma_Sp[u8_VirtualChannelID].setActionInterface(p_ActionInterface);
  return 0;
}



uint16_t TmTcClient::getScIdErrorCount(void)
{
  return mu16_ScIdErrorCount;
}


uint16_t TmTcClient::getVirtualChannelErrorCount(void)
{
  return mu16_VirtualChannelErrorCount;
}


uint16_t TmTcClient::getRetransmitErrorCount(void)
{
  return mu16_RetransmitErrorCount;
}


uint16_t TmTcClient::getLockoutErrorCount(void)
{
  return mu16_LockoutErrorCount;
}


void TmTcClient::clearErrorCounters(void)
{
  mu16_ScIdErrorCount = 0;  
  mu16_VirtualChannelErrorCount = 0;  
  mu16_RetransmitErrorCount = 0;  
  mu16_LockoutErrorCount = 0;    
}








void TmTcClient::setSync(void)
{
  m_TfTc.setSync();
}



void TmTcClient::processTfTc(const uint8_t *pu8_Data, const uint16_t u16_DataSize)
{
  m_TfTc.process(pu8_Data, u16_DataSize);
}





void TmTcClient::onTransferframeTcReceived(const bool b_BypassFlag, const bool b_CtrlCmdFlag,
                               const uint16_t u16_SpacecraftID, const uint8_t u8_VirtualChannelID,
                               const uint8_t u8_FrameSeqNumber, const uint8_t u8_MAP,
                               const uint8_t *pu8_Data, const uint16_t u16_DataSize)
{
  bool b_SpacecraftIdValid=false;
  int8_t s8_FrameSeqDiff;
  
  // first, check Spacecraft ID
  for(uint8_t i=0; i<mu8_NumberOfSCIDs; i++)
  {
    //    cout << u16_SpacecraftID << "==" << mau16_SCIDs[i] << "?" << endl;
    if(u16_SpacecraftID == mau16_SCIDs[i])
      b_SpacecraftIdValid = true;
  }
  if(!b_SpacecraftIdValid)
  {
    if(mu16_ScIdErrorCount<0xffff)
      mu16_ScIdErrorCount++;
    return;
  }   
  
  // than check if CV is supported
  if(u8_VirtualChannelID > MaxTcChannels)
  {
    if(mu16_VirtualChannelErrorCount<0xffff)
      mu16_VirtualChannelErrorCount++;
    return;
  }
  
  // if the command is sent in AD mode, perform FARM 
  if(b_BypassFlag == false)
  {
    // don't accept any command if we are in lockout
    if(ma_COP[u8_VirtualChannelID].b_LockOut)
      return;
    
    s8_FrameSeqDiff = (int8_t)(ma_COP[u8_VirtualChannelID].u8_NextFrameSeqNumber-u8_FrameSeqNumber);
    if((u8_FrameSeqNumber!=0) && (s8_FrameSeqDiff!=0))
    {
      //     cout << "Frame diff: " << (int)(int8_t)(ma_COP[u8_VirtualChannelID].u8_NextFrameSeqNumber-u8_FrameSeqNumber) << endl;
      if((s8_FrameSeqDiff>0) && (s8_FrameSeqDiff<FARM_SLIDING_WINDOW_WIDTH/2))
      {
        // received FrameSeqNumber is within negative window area, only discard frame
        ;
      }
      else if((s8_FrameSeqDiff<0) && (s8_FrameSeqDiff>=(int8_t)(-FARM_SLIDING_WINDOW_WIDTH/2)))
      {
        
        // received FrameSeqNumber is within positive window area, discard frame and set retransmit flag 
        ma_COP[u8_VirtualChannelID].b_Retransmit = true;
        if(mu16_RetransmitErrorCount<0xffff)
          mu16_RetransmitErrorCount++;
      }
      else
      {
        // recieved FrameSeqNumer is way boyond the limits, go into lockout state
        ma_COP[u8_VirtualChannelID].b_LockOut = true;
        if(mu16_LockoutErrorCount<0xffff)
          mu16_LockoutErrorCount++;
      }
      
      return;
    }
    else
      ma_COP[u8_VirtualChannelID].b_Retransmit = false;
    
    ma_COP[u8_VirtualChannelID].u8_NextFrameSeqNumber = (u8_FrameSeqNumber+1)&0xff;
  }
  else
    ma_COP[u8_VirtualChannelID].u8_FarmBCounter++;
  
  
  if(b_CtrlCmdFlag)
  {
    if((u16_DataSize==1) && (pu8_Data[0]==0x00)) // unlock command
      _ctrlCmdUnlock(u8_VirtualChannelID);
    if((u16_DataSize==3) && (pu8_Data[0]==0x82) && (pu8_Data[1]==0x00))  // set V(R) command
      _ctrlCmdSetV(u8_VirtualChannelID, pu8_Data[2]);
  }
  else
    ma_Sp[u8_VirtualChannelID].process(pu8_Data, u16_DataSize);   // regular command found, handover to callback function
  
  
  return;  
}



void TmTcClient::_ctrlCmdUnlock(const uint8_t u8_VirtualChannelID)
{
  ma_COP[u8_VirtualChannelID].b_LockOut = false;
}



void TmTcClient::_ctrlCmdSetV(const uint8_t u8_VirtualChannelID, const uint8_t u8_R)
{
  ma_COP[u8_VirtualChannelID].u8_NextFrameSeqNumber = u8_R; 
  ma_COP[u8_VirtualChannelID].b_Retransmit = false;
}



#if USE_CLTU_SUPPORT == 1

void TmTcClient::processCltu(const uint8_t *pu8_Data, const uint16_t u16_DataSize)
{
  m_Cltu.process(pu8_Data, u16_DataSize);
}


void TmTcClient::onStartOfTransmission(void)
{
  m_TfTc.setSync();
}


void TmTcClient::onCltuDataReceived(const uint8_t *pu8_Data, const uint16_t u16_DataSize)
{
  m_TfTc.process(pu8_Data, u16_DataSize);
}

#endif // USE_CLTU_SUPPORT









int32_t TmTcClient::sendTm(const uint8_t u8_VirtualChannelID, const uint16_t u16_APID, const uint16_t u16_ApidSeqNr,
                           const uint8_t *pu8_Data, const uint16_t u16_DataSize)
{
  uint32_t u32_SpPacketSize = 0;
  uint32_t u32_CLCW;
  
  if(u8_VirtualChannelID>=MaxTmChannels)
    return -1;
  
  // a space packet must be created with the data to be transfered
  u32_SpPacketSize=SpacePacket::create(mau8_TmSpBuffer, SP_MAX_DATA_SIZE,
                                       SpacePacket::TM, SpacePacket::Unsegmented, u16_APID, u16_ApidSeqNr,
                                       pu8_Data, u16_DataSize);
  
  // a telemetry frame has allways a fixed size, and we have to fill
  // up the frame with idle space packets
  u32_SpPacketSize+=SpacePacket::createIdle(&mau8_TmSpBuffer[u32_SpPacketSize],
                                            SP_MAX_DATA_SIZE-u32_SpPacketSize, mu16_IdleSpSequenceCount++, SP_MAX_DATA_SIZE-u32_SpPacketSize);
  
  if(u8_VirtualChannelID<MaxTcChannels)
  {
    u32_CLCW = Clcw::create(0, u8_VirtualChannelID,
                            ma_COP[u8_VirtualChannelID].b_NoRfAvail, ma_COP[u8_VirtualChannelID].b_NoBitLock, ma_COP[u8_VirtualChannelID].b_LockOut,
                            ma_COP[u8_VirtualChannelID].b_Wait, ma_COP[u8_VirtualChannelID].b_Retransmit,
                            ma_COP[u8_VirtualChannelID].u8_FarmBCounter, ma_COP[u8_VirtualChannelID].u8_NextFrameSeqNumber);
  }
  else
    u32_CLCW = Clcw::create(0, u8_VirtualChannelID, true, true, false, false, false, 0, 0);
  
  // now, we can create the transfer frame
  TransferframeTm::create(mau8_TfTmBuffer, TM_TF_TOTAL_SIZE,
                          mau16_SCIDs[0], 0, mu8_TmMCFC++, mau8_TmVCFC[u8_VirtualChannelID]++, 0,
                          mau8_TmSpBuffer, u32_SpPacketSize, u32_CLCW);
  
  if(mp_TmActionInterface)
  {
    mp_TmActionInterface->onTmDataCreated((uint8_t*)"\x1a\xcf\xfc\x1d", TF_SYNC_SIZE);
    mp_TmActionInterface->onTmDataCreated(mau8_TfTmBuffer, TM_TF_TOTAL_SIZE);
  }
  
  return 0;
}


int32_t TmTcClient::sendIdle(void)
{
  uint32_t u32_CLCW;
  uint8_t u8_VirtualChannelID = 0x7;
  
  if(u8_VirtualChannelID>=MaxTmChannels)
    u8_VirtualChannelID = 0;
  
  if(u8_VirtualChannelID<MaxTcChannels)
  {
    u32_CLCW = Clcw::create(0, u8_VirtualChannelID,
                            ma_COP[u8_VirtualChannelID].b_NoRfAvail, ma_COP[u8_VirtualChannelID].b_NoBitLock, ma_COP[u8_VirtualChannelID].b_LockOut,
                            ma_COP[u8_VirtualChannelID].b_Wait, ma_COP[u8_VirtualChannelID].b_Retransmit,
                            ma_COP[u8_VirtualChannelID].u8_FarmBCounter, ma_COP[u8_VirtualChannelID].u8_NextFrameSeqNumber);
  }
  else
    u32_CLCW = Clcw::create(0, u8_VirtualChannelID, true, true, false, false, false, 0, 0);
  
  TransferframeTm::createIdle(mau8_TfTmBuffer, TM_TF_TOTAL_SIZE,
                              mau16_SCIDs[0], u8_VirtualChannelID, mu8_TmMCFC++, mau8_TmVCFC[u8_VirtualChannelID]++, u32_CLCW);
  
  if(mp_TmActionInterface)
  {
    mp_TmActionInterface->onTmDataCreated((uint8_t*)"\x1a\xcf\xfc\x1d", TF_SYNC_SIZE);
    mp_TmActionInterface->onTmDataCreated(mau8_TfTmBuffer, TM_TF_TOTAL_SIZE);
  }
  
  return 0;
}


