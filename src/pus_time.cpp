/**
 * @file      pus_tc.cpp
 *
 * @brief     Source file of the PUS Time class
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

#include "pus_time.h"


namespace PUS
{
  
  
  Time::Time(const uint32_t u32_Sec, const uint32_t u32_SubSec)
  {
    mu32_Sec = u32_Sec;
    mu32_SubSec = u32_SubSec;
  }
  
  
  
  void Time::set(const uint32_t u32_Sec, const uint32_t u32_SubSec)
  {
    mu32_Sec = u32_Sec;
    mu32_SubSec = u32_SubSec;
  }
  
  
  
  uint32_t Time::get(uint32_t *pu32_Sec, uint32_t *pu32_SubSec)
  {
    uint32_t u32_Sec;
    uint32_t u32_SubSec;
    
    u32_Sec=mu32_Sec;
    u32_SubSec=mu32_SubSec;
    
    if(pu32_Sec)
      *pu32_Sec = mu32_Sec;
    if(pu32_SubSec)
      *pu32_SubSec = mu32_SubSec;
    
    return mu32_Sec;
  }
  
  
  
  bool Time::operator==(const Time &T2)
  {
    if(  (this->mu32_Sec==T2.mu32_Sec)
       &&(this->mu32_SubSec==T2.mu32_SubSec))
      return true;
    return false;
  }
  
  
  
  bool Time::operator<=(const Time &T2)
  {
    if(this->mu32_Sec<T2.mu32_Sec)
      return true;
    if(this->mu32_Sec==T2.mu32_Sec && this->mu32_SubSec<=T2.mu32_SubSec)
      return true;
    return false;
  }
  
  
  
  bool Time::operator<(const Time &T2)
  {
    if(this->mu32_Sec<T2.mu32_Sec)
      return true;
    if(this->mu32_Sec==T2.mu32_Sec && this->mu32_SubSec<T2.mu32_SubSec)
      return true;
    return false;
  }
  
  
  
  bool Time::operator>=(const Time &T2)
  {
    if(this->mu32_Sec>T2.mu32_Sec)
      return true;
    if(this->mu32_Sec==T2.mu32_Sec && this->mu32_SubSec>=T2.mu32_SubSec)
      return true;
    return false;
  }
  
  
  
  bool Time::operator>(const Time &T2)
  {
    if(this->mu32_Sec>T2.mu32_Sec)
      return true;
    if(this->mu32_Sec==T2.mu32_Sec && this->mu32_SubSec>T2.mu32_SubSec)
      return true;
    return false;
  }
  
  
  
  
  
  
  void TimeCuc::set(const uint8_t *pu8_TimeCucBuffer, const uint8_t u8_BufferSize)
  {
    uint8_t u8_CoarseTime;
    uint8_t u8_FineTime;
    uint32_t u32_Sec;
    uint32_t u32_SubSec;
    
    if((NULL==pu8_TimeCucBuffer) || (u8_BufferSize<2))
      return;
    
    if(0x20!=(pu8_TimeCucBuffer[0]&0xF0))  // first nibble must be 0x20
      return;  // format not supported
    
    u8_CoarseTime = ((pu8_TimeCucBuffer[0]>>2)&0x3)+1;
    u8_FineTime = pu8_TimeCucBuffer[0]&0x3;
    
    if(u8_BufferSize < 1+u8_CoarseTime+u8_FineTime)
      return; // buffer size is too small to hold valid time
    
    u32_Sec = 0;
    u32_SubSec = 0;
    for(uint8_t i=0; i<u8_CoarseTime; i++)
      u32_Sec = (u32_Sec<<8) | pu8_TimeCucBuffer[1+i];
    for(uint8_t i=0; i<u8_FineTime; i++)
      u32_SubSec |= ((uint32_t)(pu8_TimeCucBuffer[1+u8_CoarseTime+i])<<(8*(3-i)));
    
    Time::set(u32_Sec, u32_SubSec);
    
    return;
  }
  
  
  
  void TimeCuc::get(uint8_t *pu8_TimeCucBuffer, const uint8_t u8_BufferSize, const ETimeCucFormat e_TimeCucFormat)
  {
    uint8_t u8_CoarseTime;
    uint8_t u8_FineTime;
    uint32_t u32_Sec;
    uint32_t u32_SubSec;
    
    if((NULL==pu8_TimeCucBuffer) || (u8_BufferSize<2))
      return;
    
    if(0x20!=((uint8_t)e_TimeCucFormat&0xF0))  // first nibble must be 0x20
      return;  // format not supported
    
    u8_CoarseTime = ((((uint8_t)e_TimeCucFormat)>>2)&0x3)+1;
    u8_FineTime = ((uint8_t)e_TimeCucFormat)&0x3;
    
    if(u8_BufferSize < 1+u8_CoarseTime+u8_FineTime)
      return; // buffer size is too small to hold valid time
    
    // first, get time in Sec and SubSec
    Time::get(&u32_Sec, &u32_SubSec);
    
    pu8_TimeCucBuffer[0]=(uint8_t)e_TimeCucFormat;
    for(uint8_t i=0; i<u8_CoarseTime; i++)
      pu8_TimeCucBuffer[1+i] = (uint8_t)(u32_Sec>>(8*(u8_CoarseTime-i-1)));
    for(uint8_t i=0; i<u8_FineTime; i++)
      pu8_TimeCucBuffer[1+u8_CoarseTime+i] = (uint8_t)(u32_SubSec>>(8*(3-i)));
    
    return;
  }
  
  
  
  
  
  
  
  TimeGregorian::TimeGregorian(const int16_t s16_TimeZoneInMin)
  {
    ms16_TimeZoneInMin = s16_TimeZoneInMin;
  }
  
  
  
  void TimeGregorian::time2Gregorian(uint16_t *pu16_Year, uint8_t *pu8_Month, uint8_t *pu8_Day,
                                     uint8_t *pu8_Hour, uint8_t *pu8_Min, uint8_t *pu8_Sec, const uint32_t u32_TimeInSec, const int16_t s16_TimeZoneMin)
  {
    
    const uint32_t SEC_PER_DAY        =  86400ul; /*  24* 60 * 60 */
    const uint32_t DAYS_PER_YEAR      =    365ul; /* kein Schaltjahr */
    const uint32_t DAYS_PER_4_YEARS   =   1461ul; /*   4*365 +   1 */
    const uint32_t DAYS_PER_100_YEARS =  36524ul; /* 100*365 +  25 - 1 */
    const uint32_t DAYS_PER_400_YEARS = 146097ul; /* 400*365 + 100 - 4 + 1 */
    const uint32_t DAY_AD_1970_01_01  = 719468ul; /* Tagnummer bezogen auf den 1. Maerz des Jahres "Null" */
    
    uint32_t u32_DaysAD;
    uint32_t u32_SecOfDay;
    uint32_t u32_Temp;
    
    uint16_t u16_Year;
    uint8_t u8_Month;
    uint8_t u8_Day;
    
    u32_DaysAD = DAY_AD_1970_01_01 + (u32_TimeInSec+mu32_EpochOffset) / SEC_PER_DAY;
    u32_SecOfDay = (u32_TimeInSec+mu32_EpochOffset) % SEC_PER_DAY;
    
    /* Schaltjahrregel des Gregorianischen Kalenders: jedes durch 100 teilbare Jahr ist kein Schaltjahr, es sei denn, es ist durch 400 teilbar. */
    u32_Temp = 4 * (u32_DaysAD + DAYS_PER_100_YEARS + 1) / DAYS_PER_400_YEARS - 1;
    u16_Year = 100 * u32_Temp;
    u32_DaysAD -= DAYS_PER_100_YEARS * u32_Temp + u32_Temp / 4;
    
    /* Schaltjahrregel des Julianischen Kalenders:
     Jedes durch 4 teilbare Jahr ist ein Schaltjahr. */
    u32_Temp = 4 * (u32_DaysAD + DAYS_PER_YEAR + 1) / DAYS_PER_4_YEARS - 1;
    u16_Year += u32_Temp;
    u32_DaysAD -= DAYS_PER_YEAR * u32_Temp + u32_Temp / 4;
    
    /* u32_DaysAD enthaelt jetzt nur noch die Tage des errechneten Jahres bezogen auf den 1. Maerz. */
    u8_Month = (5 * u32_DaysAD + 2) / 153;
    u8_Day = u32_DaysAD - (u8_Month * 153 + 2) / 5 + 1;
    /*  153 = 31+30+31+30+31 Tage fuer die 5 Monate von Maerz bis Juli
     153 = 31+30+31+30+31 Tage fuer die 5 Monate von August bis Dezember
     31+28          Tage fuer Januar und Februar (siehe unten)
     +2: Justierung der Rundung
     +1: Der erste Tag im Monat ist 1 (und nicht 0).
     */
    
    u8_Month += 3; /* vom Jahr, das am 1. Maerz beginnt auf unser normales Jahr umrechnen: */
    if(u8_Month > 12)
    {  /* Monate 13 und 14 entsprechen 1 (Januar) und 2 (Februar) des naechsten Jahres */
      u8_Month -= 12;
      u16_Year++;
    }
    
    if(pu16_Year)
      *pu16_Year = u16_Year;
    
    if(pu8_Month)
      *pu8_Month = u8_Month;
    
    if(pu8_Day)
      *pu8_Day = u8_Day;
    
    if(pu8_Hour)
      *pu8_Hour = u32_SecOfDay / 3600;
    
    if(pu8_Min)
      *pu8_Min = u32_SecOfDay % 3600 / 60;
    
    if(pu8_Sec)
      *pu8_Sec = u32_SecOfDay % 60;
  }
  
  
  
  uint32_t TimeGregorian::_gregorian2UnixTime(uint32_t *pu32_TimeInSec,
                                              const uint16_t u16_Year, const uint8_t u8_Month, const uint8_t u8_Day,
                                              const uint8_t u8_Hour, const uint8_t u8_Min, const uint8_t u8_Sec,
                                              const int16_t s16_TimeZoneMin)
  {
    uint32_t u32_TimeInSec;
    uint16_t u16_DayOfYear[12] = /* Anzahl der Tage seit Jahresanfang ohne Tage des aktuellen Monats und ohne Schalttag */
    {0,31,59,90,120,151,181,212,243,273,304,334};
    uint16_t u16_LeapYears;
    uint32_t u32_DaysSince1970;
    
    
    /* for algorithm, see https://de.wikipedia.org/wiki/Unixzeit */
    
    u16_LeapYears = ((u16_Year-1)-1968)/4    /* Anzahl der Schaltjahre seit 1970 (ohne das evtl. laufende Schaltjahr) */
    - ((u16_Year-1)-1900)/100
    + ((u16_Year-1)-1600)/400;
    
    u32_DaysSince1970 = (u16_Year-1970)*365 + u16_LeapYears
    + u16_DayOfYear[u8_Month-1] + u8_Day-1;
    
    if( (u8_Month>2) && (u16_Year%4==0 && (u16_Year%100!=0 || u16_Year%400==0)) )
      u32_DaysSince1970 += 1; /* +Schalttag, wenn jahr Schaltjahr ist */
    
    u32_TimeInSec = (u8_Sec + 60 * ( u8_Min + 60 * (u8_Hour + 24*u32_DaysSince1970) + s16_TimeZoneMin ) );
    
    if(pu32_TimeInSec)
      *pu32_TimeInSec = u32_TimeInSec;
    
    return u32_TimeInSec;
  }
  
  
  uint32_t TimeGregorian::gregorian2Time(uint32_t *pu32_TimeInSec,
                                         const uint16_t u16_Year, const uint8_t u8_Month, const uint8_t u8_Day,
                                         const uint8_t u8_Hour, const uint8_t u8_Min, const uint8_t u8_Sec,
                                         const int16_t s16_TimeZoneMin)
  {
    uint32_t u32_TimeInSec;
    
    _gregorian2UnixTime(&u32_TimeInSec, u16_Year, u8_Month, u8_Day, u8_Hour, u8_Min, u8_Sec, s16_TimeZoneMin);
    
    u32_TimeInSec -= mu32_EpochOffset;
    
    if(pu32_TimeInSec)
      *pu32_TimeInSec = u32_TimeInSec;
    
    return u32_TimeInSec;
  }
  
  
  
  
  uint32_t TimeGregorian::mu32_EpochOffset = 0;
  
  int32_t TimeGregorian::setEpoch(const uint16_t u16_Year, const uint8_t u8_Month, const uint8_t u8_Day,
                                  const uint8_t u8_Hour, const uint8_t u8_Min, const uint8_t u8_Sec)
  {
    if(u16_Year<1970)
      return -1;
    
    _gregorian2UnixTime(&mu32_EpochOffset, u16_Year, u8_Month, u8_Day, u8_Hour, u8_Min, u8_Sec);
    
    return 0;
  }
  
  
  int32_t TimeGregorian::time2Iso8601(char *pc_Buffer, const uint8_t u8_BufferSize, const uint32_t u32_TimeInSec, const int16_t s16_TimeZoneMin)
  {
    uint16_t u16_Year;
    uint8_t u8_Month, u8_Day;
    uint8_t u8_Hour, u8_Min, u8_Sec;
    
    if((NULL==pc_Buffer) || (0==u8_BufferSize))
      return -1;
    
    time2Gregorian(&u16_Year, &u8_Month, &u8_Day, &u8_Hour, &u8_Min, &u8_Sec, u32_TimeInSec, s16_TimeZoneMin);
    
    if(u8_BufferSize>=20)
      sprintf(pc_Buffer, "%04d-%02d-%02d %02d:%02d:%02d", u16_Year, u8_Month, u8_Day, u8_Hour, u8_Min, u8_Sec);
    else if(u8_BufferSize>=11)
      sprintf(pc_Buffer, "%04d-%02d-%02d", u16_Year, u8_Month, u8_Day);
    else
      return -1;
    
    return 0;
  }
  
  
  
  void TimeGregorian::set(const uint16_t u16_Year, const uint8_t u8_Month, const uint8_t u8_Day,
                          const uint8_t u8_Hour, const uint8_t u8_Min, const uint8_t u8_Sec)
  {
    uint32_t u32_TimeInSec = 0;
    
    gregorian2Time(&u32_TimeInSec, u16_Year, u8_Month, u8_Day, u8_Hour, u8_Min, u8_Sec, ms16_TimeZoneInMin);
    Time::set(u32_TimeInSec);
  }
  
  
  
  void TimeGregorian::get(uint16_t *pu16_Year, uint8_t *pu8_Month, uint8_t *pu8_Day,
                          uint8_t *pu8_Hour, uint8_t *pu8_Min, uint8_t *pu8_Sec)
  {
    uint32_t u32_TimeInSec = 0;
    
    Time::get(&u32_TimeInSec);
    time2Gregorian(pu16_Year, pu8_Month, pu8_Day, pu8_Hour, pu8_Min, pu8_Sec, u32_TimeInSec, ms16_TimeZoneInMin);
  }
  
  
  
  
  /*
   int16_t TimeUtc::ms16_UtcLeapSec = 32;
   
   
   void TimeUtc::setUtcLeapSec(const int16_t s16_LeapSec)
   {
   ms16_UtcLeapSec=s16_LeapSec;
   }
   
   
   
   int16_t TimeUtc::getUtcLeapSec(void)
   {
   return ms16_UtcLeapSec;
   }*/
  

}
