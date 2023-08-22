/**
 * @file      ccsds_transferframe.h
 *
 * @brief     Include file of the Transferframe (TF) class
 *
 * @author    Stefan Trippler
 *
 * @copyright Copyright (C) 2021-2023 Stefan Trippler.  All rights reserved.
 */

#ifndef _CCSDS_TRANSFERFRAME_H_
#define _CCSDS_TRANSFERFRAME_H_

/****************************************************************/
/* Transferframes according to                                  */
/*                                                              */
/*  - CCSDS 131.0-B-3, - TM Synchronization and Channel Coding  */
/*    https://public.ccsds.org/Pubs/131x0b3e1.pdf               */
/*  - CCSDS 132.0-B-2, - TM Space Data Link Protocol            */
/*    https://public.ccsds.org/Pubs/132x0b2.pdf                 */
/*  - CCSDS 231.0-B-3, - TC Synchronization and Channel Coding  */
/*    https://public.ccsds.org/Pubs/231x0b3.pdf                 */
/*  - CCSDS 232.0-B-3, - TC Space Data Link Protocol            */
/*    https://public.ccsds.org/Pubs/232x0b3.pdf                 */
/*                                                              */
/****************************************************************/

#include <inttypes.h>

#include "configCCSDS.h"


#ifdef configTF_USE_OCF
#define TF_USE_FECF configTF_USE_FECF
#else
#define TF_USE_FECF 1
#endif




#define TF_SYNC_SIZE 4

namespace CCSDS 
{
  
  /**
   * @brief Base Class for handling Transfer Frames as described in CCSDS 131.0-B-3, CCSDS 132.0-B-2, CCSDS 231.0-B-3 and CCSDS 232.0-B-3.
   *
   * Transfer Frames are used to ensure a transfer from the ground to the spacecraft and
   * vice versa, corresponding to OSI layer 2.
   *
   * The transfer frames for uplink and downlink differ, therefore this class is the base for the
   * specific classes TransferframeTc and TransferframeTm.
   */
  class Transferframe
  {
  protected:
    const static uint8_t SyncSize = TF_SYNC_SIZE;
    const static uint8_t FecfSize = 2;
    const static bool UseFECF = (TF_USE_FECF)?true:false;  // Frame error control field (CRC)
    
    uint16_t mu16_Index;
    uint16_t mu16_FrameLength;
    bool mb_Sync;
    uint16_t mu16_SyncErrorCount;
    uint16_t mu16_ChecksumErrorCount;
    uint16_t mu16_OverflowErrorCount;
    
  public:
    void setSync(void);
    int32_t process(const uint8_t *pu8_Data, const uint16_t u16_DataSize);
    uint16_t getSyncErrorCount(void);
    uint16_t getChecksumErrorCount(void);
    uint16_t getOverflowErrorCount(void);
    void clearErrorCounters(void);
    
  protected:
    Transferframe(void);
    bool _checkCRC(void);
    static uint16_t calcCRC(const uint8_t *pu8_Buffer, const uint16_t u16_BufferSize);
    
  private:
    virtual void _processFrame(void) = 0;
    virtual uint16_t _getMaxTfSize(void) = 0;
    virtual uint8_t *_getTfBufferAddr(void) = 0;
    virtual uint16_t _getPrimaryHeaderSize(void) = 0;
    virtual uint16_t _getFrameLength(void) = 0;
  };
  
  
}

#endif // _CCSDS_TRANSFERFRAME_H_


