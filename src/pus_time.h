/**
 * @file      pus_tc.h
 *
 * @brief     Include file of the PUS Time class
 *
 * @author    Stefan Trippler
 *
 * @copyright Copyright (C) 2021-2022 Stefan Trippler.  All rights reserved.
 */


#ifndef _PUS_TIME_H_
#define _PUS_TIME_H_

#include <stdio.h>

/****************************************************************/
/* PUS TC Packet according to                                   */
/*                                                              */
/*  ECSS-E-70-41A - Space Packet Protocol                       */
/*                                                              */
/*  Time base is TAI.                                           */
/*                                                              */
/****************************************************************/


namespace PUS
{
  

  class Time
  {
  protected:
    uint32_t mu32_Sec;
    uint32_t mu32_SubSec;
    
  public:
    
    Time(const uint32_t u32_Sec = 0, const uint32_t u32_SubSec = 0);
    
    void set(const uint32_t u32_Sec, const uint32_t u32_SubSec = 0);
    uint32_t get(uint32_t *pu32_Sec = NULL, uint32_t *pu32_SubSec = NULL);
    
    bool operator==(const Time &T2);
    bool operator<=(const Time &T2);
    bool operator<(const Time &T2);
    bool operator>=(const Time &T2);
    bool operator>(const Time &T2);
  };
  
  
  
  const uint8_t CTimeCucSize = 8;
  
  typedef enum
  {
    TimeCUC10 = 0x20,
    TimeCUC11 = 0x21,
    TimeCUC12 = 0x22,
    TimeCUC13 = 0x23,
    TimeCUC20 = 0x24,
    TimeCUC21 = 0x25,
    TimeCUC22 = 0x26,
    TimeCUC23 = 0x27,
    TimeCUC30 = 0x28,
    TimeCUC31 = 0x29,
    TimeCUC32 = 0x2A,
    TimeCUC33 = 0x2B,
    TimeCUC40 = 0x2C,
    TimeCUC41 = 0x2D,
    TimeCUC42 = 0x2E,
    TimeCUC43 = 0x2F
  } ETimeCucFormat;
  
  
  
  class TimeCuc : public Time
  {
  public:
    void set(const uint8_t *pu8_TimeCucBuffer, const uint8_t u8_BufferSize);
    void get(uint8_t *pu8_TimeCucBuffer, const uint8_t u8_BufferSize, const ETimeCucFormat e_TimeCucFormat = TimeCUC42);
  };
  
  
  
  
  class TimeGregorian : public Time
  {
  private:
    //  static int8_t ms8_UtcLeapSec;
    static uint32_t mu32_EpochOffset;  // default epoch is 1970-01-01 00:00:00 UTC (Unixtime)
    int16_t ms16_TimeZoneInMin;
    
  public:
    TimeGregorian(const int16_t s16_TimeZoneInMin = 0);
    
    static void time2Gregorian(uint16_t *pu16_Year, uint8_t *pu8_Month, uint8_t *pu8_Day,
                               uint8_t *pu8_Hour, uint8_t *pu8_Min, uint8_t *pu8_Sec, const uint32_t u32_TimeInSec, const int16_t s16_TimeZoneMin = 0);
    
    static uint32_t gregorian2Time(uint32_t *pu32_TimeInSec,
                                   const uint16_t u16_Year, const uint8_t u8_Month, const uint8_t u8_Day,
                                   const uint8_t u8_Hour, const uint8_t u8_Min, const uint8_t u8_Sec,
                                   const int16_t s16_TimeZoneMin = 0);
    
    static int32_t setEpoch(const uint16_t u16_Year = 1970, const uint8_t u8_Month = 1, const uint8_t u8_Day = 1,
                            const uint8_t u8_Hour = 0, const uint8_t u8_Min = 0, const uint8_t u8_Sec = 0);
    
    static int32_t time2Iso8601(char *pc_Buffer, const uint8_t u8_BufferSize, const uint32_t u32_TimeInSec, const int16_t s16_TimeZoneMin = 0);
    
    void set(const uint16_t u16_Year, const uint8_t u8_Month, const uint8_t u8_Day,
             const uint8_t u8_Hour, const uint8_t u8_Min, const uint8_t u8_Sec = 0);
    
    void get(uint16_t *pu16_Year, uint8_t *pu8_Month, uint8_t *pu8_Day,
             uint8_t *pu8_Hour, uint8_t *pu8_Min, uint8_t *pu8_Sec = NULL);
    
  private:
    static uint32_t _gregorian2UnixTime(uint32_t *pu32_TimeInSec,
                                        const uint16_t u16_Year, const uint8_t u8_Month, const uint8_t u8_Day,
                                        const uint8_t u8_Hour, const uint8_t u8_Min, const uint8_t u8_Sec,
                                        const int16_t s16_TimeZoneMin = 0);
  };
  
  
}

#endif // _PUS_TIME_H_
