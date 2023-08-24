/**
 * @file      ccsds_cltu.h
 *
 * @brief     Include file of the Communications Link Transmission Unit (CLTU) class
 *
 * @author    Stefan Trippler
 *
 * @copyright Copyright (C) 2021-2023 Stefan Trippler.  All rights reserved.
 */

#ifndef _CCSDS_CLTU_H_
#define _CCSDS_CLTU_H_

/****************************************************************/
/* CLTUs according to                                           */
/*                                                              */
/*  - CCSDS 231.0-B-3 - TC Synchronization and Channel Coding   */
/*    https://public.ccsds.org/Pubs/231x0b3s.pdf                */
/*                                                              */
/* Limitations:                                                 */ 
/*  - Low-Density Parity-Check (LDPC) is not supported          */
/*                                                              */
/****************************************************************/

#include <inttypes.h>

#include "ccsds_transferframe_tc.h"

#define CLTU_DATA_BLOCK_SIZE  7
#define CLTU_MAX_SIZE (2+((TC_TF_MAX_SIZE+6)/7)*8+8)


namespace CCSDS
{

  /**
   * @brief Interface class for handling cltu actions
   */
  class CltuActionInterface
  {
  public:
    /**
     * @brief Declaration of the action which shall be called if a start of transmission was detected.
     *
     * The implementation of this callback shall handle a start of transmission. It shall implement the fowarding to the
     * corresponding application (or the further processing of the content).
     */
    virtual void onStartOfTransmission(void) = 0;

    /**
     * @brief Declaration of the action which shall be called if complete cltus were received
     *
     * The implementation of this callback shall handle the cltu content. It shall implement the fowarding to the
     * corresponding application (or the further processing of the content).
     *
     * @param pu8_Data      A pointer to the data block which holds the content of the package
     * @param u16_DataSize  The size of the data block in bytes
     */
    virtual void onCltuDataReceived(const uint8_t *pu8_Data, const uint16_t u16_DataSize) = 0;
  };



  /**
   * @brief Class for handling Communications Link Transmission Unit (CLTU) as described in CCSDS 231.0-B-3.
   *
   * CLTUs are used to syncronize to the uplink data strea
   *
   * On real satellites, CLTUs are usually processed by hardware, and the raw Transfer Frames are handed over to the
   * application which handles the communication between ground and the satellite. With this class, it is possible to
   * detect a start of transmission, to create and unpack CLTUs in software.
   */
  class Cltu
  {
  public:
    const static uint16_t MaxSize = CLTU_MAX_SIZE;

  private:
    const static uint8_t StartSequenceSize = 2;
    const static uint8_t DataBlockSize = CLTU_DATA_BLOCK_SIZE;
    const static uint8_t CRCSize = 1;
    const static uint8_t TailSquenceSize = 8;
    
    bool mb_Sync;
    uint8_t mau8_Buffer[CLTU_DATA_BLOCK_SIZE];
    uint8_t mu8_Index;
    
    CltuActionInterface *mp_ActionInterface;
    
  public:
    Cltu(CltuActionInterface *p_ActionInterface = nullptr);
    
    void setActionInterface(CltuActionInterface *p_ActionInterface);
    
  public:
   static uint32_t create(uint8_t *pu8_Buffer, const uint32_t u32_BufferSize,
                           const uint8_t *pu8_Data, const uint16_t u16_DataSize);
    
    void process(const uint8_t *pu8_Data, const uint16_t u16_DataSize);
    
  private:
    static uint8_t calcCRC(const uint8_t *pu8_Buffer, const uint8_t u8_BufferSize);
  };
    
}

#endif /* _CCSDS_CLTU_H_ */
