/**
 * @file      ccsds_clcw.cpp
 *
 * @brief     Source file of the Communications Link Control Word (CLCW) class
 *
 * @author    Stefan Trippler
 * @date      2022-04-03
 *
 * @copyright Copyright (C) 2021-2022 Stefan Trippler.  All rights reserved.
 */


#ifdef AVR
#include <Arduino.h>
#else
#include <cstdint>
#include <cstring>
#endif

#include "ccsds_clcw.h"


namespace CCSDS
{
  
  /**
   * @brief Creates a Communications Link Control Word (CLCW) as described in CCSDS 232.0-B-3.
   *
   * @param u8_StatusField      The 3-bit Status Field is mission specific.
   * @param u8_VirtualChannelID The Virtual Channel Identifier shall contain the virtual channel of which the CLCW is associated with (6 bit).
   * @param b_NoRfAvail         The No RF Available Flag shall indicate if the radio frequency (RF) is available (value 0) or not (value 1).
   * @param b_NoBitLock         The No Bit Lock shall indicate if the physical layer finds a signal and is working normally (value 0)
   * @param b_LockOut           The LockOut Flag shall indicate that a Type-A Transfer Frame violated the acceptance criteria (value 1).
   * @param b_Wait              The Wait Flag shall indicate that the remote system is able to receive and process transfer frames (value 1).
   * @param b_Retransmit        The Retransmit Flag shall indicate that one or more Type-A Transfer Frames are rejected and need to be retransmitted (value 1).
   * @param u8_FarmBCounter     The FARM-B counter shall contain the 2 least segnificant bits of the counter maintained by the FARM everytime a Type-B Transfer Frame is accepted in Bypass Mode.
   * @param u8_ReportValue      The 8-bit FARM-A counter (the Report Value) shall contain the value of the Next Expected Frame Sequence Number.
   *
   * @return                    The Communications Link Control Word (CLCW) as an uint32_t.
   */
  uint32_t Clcw::create(const uint8_t u8_StatusField, const uint8_t u8_VirtualChannelID,
                        const bool b_NoRfAvail, const bool b_NoBitLock, const bool b_LockOut, const bool b_Wait, const bool b_Retransmit,
                        const uint8_t u8_FarmBCounter, const uint8_t u8_ReportValue)
  {
    uint32_t u32_CLCW = 0;
    
    u32_CLCW = ((uint32_t)(ClcwVersionNumber&0x3)<<29)
    | ((uint32_t)(u8_StatusField&0x7)<<26)
    | ((uint32_t)(COPinEffect&0x3)<<24)
    | ((uint32_t)(u8_VirtualChannelID&0x3f)<<18)
    | ((b_NoRfAvail?1:0)<<15) | ((b_NoBitLock?1:0)<<14) | ((b_LockOut?1:0)<<13) | ((b_Wait?1:0)<<12) | ((b_Retransmit?1:0)<<11)
    | ((uint32_t)(u8_FarmBCounter&0x3)<<9) | (uint32_t)(u8_ReportValue);
    
    return u32_CLCW;
  }
  
  
  /**
   * @brief Extractes a Communications Link Control Word (CLCW) as described in CCSDS 232.0-B-3.
   *
   * @param pu8_StatusField       Pointer to an uint8_t where the mission-specific Status Field shall be stored.
   * @param pu8_VirtualChannelID  Pointer to an uint8_t where the Virtual Channel Identifier shall be stored.
   * @param pb_NoRfAvail          Pointer to a bool where the No RF Available flag shall be stored.
   * @param pb_NoBitLock          Pointer to a bool where the No Bit Lock flag shall be stored.
   * @param pb_LockOut            Pointer to a bool where the LockOut Flag shall be stored.
   * @param pb_Wait               Pointer to a bool where the Wait Flag shall be stored.
   * @param pb_Retransmit         Pointer to a bool where the Retransmit flag shall be stored.
   * @param pu8_FarmBCounter      Pointer to an uint8_t where the FARM-B counter shall be stored.
   * @param pu8_ReportValue       Pointer to an uint8_t where the 8-bit FARM-A counter shall be stored.
   * @param u32_CLCW              The Communications Link Control Word (CLCW) which is to be extracted.
   *
   * @retval  0                   Exctraction was sucessfull.
   * @retval -1                   The CLCW version number does not match the expected one.
   */
  int32_t Clcw::extract(uint8_t *pu8_StatusField, uint8_t *pu8_VirtualChannelID,
                        bool *pb_NoRfAvail, bool *pb_NoBitLock, bool *pb_LockOut, bool *pb_Wait, bool *pb_Retransmit,
                        uint8_t *pu8_FarmBCounter, uint8_t *pu8_ReportValue, const uint32_t u32_CLCW)
  {
    uint8_t u8_ClcwVersionNumber;
    
    if(pu8_StatusField)
      *pu8_StatusField=(uint8_t)((u32_CLCW>>26)&0x7);
    // COPinEffect not used
    if(pu8_VirtualChannelID)
      *pu8_VirtualChannelID=(uint8_t)((u32_CLCW>>18)&0x3f);
    
    if(pb_NoRfAvail)
      *pb_NoRfAvail=((u32_CLCW>>15)&0x1)?true:false;
    if(pb_NoBitLock)
      *pb_NoBitLock=((u32_CLCW>>14)&0x1)?true:false;
    if(pb_LockOut)
      *pb_LockOut=((u32_CLCW>>13)&0x1)?true:false;
    if(pb_Wait)
      *pb_Wait=((u32_CLCW>>12)&0x1)?true:false;
    if(pb_Retransmit)
      *pb_Retransmit=((u32_CLCW>>11)&0x1)?true:false;
    
    if(pu8_FarmBCounter)
      *pu8_FarmBCounter=(uint8_t)((u32_CLCW>>9)&0x3);
    if(pu8_ReportValue)
      *pu8_ReportValue=(uint8_t)(u32_CLCW&0xff);
    
    // version number check (last - to
    u8_ClcwVersionNumber=(uint8_t)((u32_CLCW>>29)&0x3);
    if(u8_ClcwVersionNumber!=ClcwVersionNumber)
      return -1;
    
    return 0;
  }


}
