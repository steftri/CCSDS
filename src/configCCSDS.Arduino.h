#ifndef _CONFIG_CCSDS_H_
#define _CONFIG_CCSDS_H_

/* To be able to use this library on ARDUINO UNO, the default setting
 * is a very memory saving configuration
 */

/** Maximum size of space packets (can be up to 65535 according to the standard) */
#define configSP_MAX_DATA_SIZE      32  

/** Telecommand TF size (without SYNC); maximum as defined in CCSDS 232.0-B-3 is 1024 */
#define configTC_TF_MAX_SIZE        44  

/** Telemetry TF size (without SYNC) */
#define configTM_TF_TOTAL_SIZE      44  

/** The OCF field (which contains the CLCW needed for automatic re-transfer) is optional */
#define configTF_USE_OCF             1  

/** The Frame Error Control Field (FECF) contains the CRC of telemetry packets */
#define configTF_USE_FECF            1 


#endif // _CONFIG_CCSDS_H_
