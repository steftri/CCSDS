#ifndef _CONFIG_CCSDS_H_
#define _CONFIG_CCSDS_H_

/* To be able to use this library on ARDUINO UNO, the default setting
 * is a very memory saving configuration
 */

/** Maximum size of space packets (can be up to 65535 according to the standard) */
#ifndef CCSDS_SP_MAX_DATA_SIZE
  #define CCSDS_SP_MAX_DATA_SIZE      32
#endif

/** Telecommand TF size (without SYNC); maximum as defined in CCSDS 232.0-B-3 is 1024 */
#ifndef CCSDS_TC_TF_MAX_SIZE
  #define CCSDS_TC_TF_MAX_SIZE        44
#endif

/** Telemetry TF size (without SYNC) */
#ifndef CCSDS_TM_TF_TOTAL_SIZE
  #define CCSDS_TM_TF_TOTAL_SIZE      44
#endif

/** The OCF field (which contains the CLCW needed for automatic re-transfer) is optional */
#ifndef CCSDS_TF_USE_OCF
  #define CCSDS_TF_USE_OCF             1
#endif

/** The Frame Error Control Field (FECF) contains the CRC of telemetry packets */
#ifndef CCSDS_TF_USE_FECF
  #define CCSDS_TF_USE_FECF            1
#endif

/** The Segment Header contains the Multiplexer Access Point (MAP) */
#ifndef CCSDS_TF_TC_USE_SEG_HDR
  #define CCSDS_TF_TC_USE_SEG_HDR      1
#endif


#endif // _CONFIG_CCSDS_H_
