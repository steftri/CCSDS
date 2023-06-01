/**
 * @file      ccsds_cltu.h
 *
 * @brief     Include file of the Communications Link Transmission Unit (CLTU) class
 *
 * @author    Stefan Trippler
 *
 * @copyright Copyright (C) 2021-2022 Stefan Trippler.  All rights reserved.
 */


#ifndef _CCSDS_CLTU_H_
#define _CCSDS_CLTU_H_

#ifdef configCLTU_MAX_SIZE
#define CLTU_MAX_SIZE configCLTU_MAX_SIZE
#else
#define CLTU_MAX_SIZE (2+((TC_TF_MAX_SIZE+6)/7)*8+8)
#endif


namespace CCSDS
{
  
  typedef void (TStartOfTransmissionCallback)(void *p_Context);
  typedef void (TReceiveCallback)(void *p_Context, const uint8_t *pu8_Data, const uint16_t u16_DataSize);
  
  /**
   * @brief Class for handling Communications Link Transmission Unit (CLTU) as described in CCSDS 231.0-B-3.
   *
   * CLTUs are used to syncronize to the uplink data stream.
   *
   * On real satellites, CLTUs are usually processed by hardware, and the raw Transfer Frames are handed over to the
   * application which handles the communication between ground and the satellite. With this class, it is possible to
   * detect a start of transmission, to create and unpack CLTUs in software.
   */
  class Cltu
  {
  private:
    const static uint8_t StartSequenceSize = 2;
    const static uint8_t DataBlockSize = 7;
    const static uint8_t CRCSize = 1;
    const static uint8_t TailSquenceSize = 8;
    
    bool mb_Sync;
    uint8_t mau8_Buffer[DataBlockSize];
    uint8_t mu8_Index;
    
    void *mp_StartOfTransmissionContext;
    TStartOfTransmissionCallback *mp_StartOfTransmissionCallback;
    
    void *mp_ReceiveContext;
    TReceiveCallback *mp_ReceiveCallback;
    
  public:
    Cltu(void *p_StartOfTransmissionContext = NULL, TStartOfTransmissionCallback *p_StartOfTransmissionCallback = NULL,
         void *p_ReceiveContext = NULL, TReceiveCallback *p_ReceiveCallback = NULL);
    
    void setCallbacks(void *p_StartOfTransmissionContext, TStartOfTransmissionCallback *p_StartOfTransmissionCallback,
                      void *p_ReceiveContext, TReceiveCallback *p_ReceiveCallback);
    
    
  public:
    static uint32_t create(uint8_t *pu8_Buffer, const uint32_t u32_BufferSize,
                           const uint8_t *pu8_Data, const uint16_t u16_DataSize);
    
    void process(const uint8_t *pu8_Data, const uint16_t u16_DataSize);
    
  private:
    static uint8_t calcCRC(const uint8_t *pu8_Buffer, const uint8_t u8_BufferSize);
  };
  
  
}

#endif /* _CCSDS_CLTU_H_ */
