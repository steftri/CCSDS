/**
 * @file      ccsds_clcw.h
 *
 * @brief     Include file of the Communications Link Control Word (CLCW) class
 *
 * @author    Stefan Trippler
 *
 * @copyright Copyright (C) 2021-2023 Stefan Trippler.  All rights reserved.
 */

#ifndef _CCSDS_CLCW_H_
#define _CCSDS_CLCW_H_

#include <inttypes.h>


namespace CCSDS
{
  
  /**
   * @brief Class for handling the Communications Link Control Word (CLCW) as described in CCSDS 232.0-B-3.
   *
   * The CLCW is carried by the Operational Control Field (OCF) of the Telemetry Transfer Frame. It is used
   * to report the state of the remote data communication system. Except the NoRFAvail and the NoBitLock flags,
   * which indicates the general physical layer of the telecommand channel, all values are specific to the
   * respective virtual channel.
   */
  class Clcw
  {
  private:
    const static uint8_t ClcwVersionNumber = 0;
    const static uint8_t COPinEffect = 0x1;
    
  public:
    static uint32_t create(const uint8_t u8_StatusField, const uint8_t u8_VirtualChannelID,
                           const bool b_NoRfAvail, const bool b_NoBitLock, const bool b_LockOut, const bool b_Wait, const bool b_Retransmit,
                           const uint8_t u8_FarmBCounter, const uint8_t u8_ReportValue);
    
    static int32_t extract(uint8_t *pu8_StatusField, uint8_t *pu8_VirtualChannelID,
                           bool *pb_NoRfAvail, bool *pb_NoBitLock, bool *pb_LockOut, bool *pb_Wait, bool *pb_Retransmit,
                           uint8_t *pu8_FarmBCounter, uint8_t *pu8_ReportValue, const uint32_t u32_CLCW);
  };
  
}

#endif /* _CCSDS_CLCW_H_ */
