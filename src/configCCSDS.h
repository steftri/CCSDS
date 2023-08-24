#ifndef _CONFIG_CCSDS_H_
#define _CONFIG_CCSDS_H_


/** Maximum size of space packets (can be up to 65535 according to the standard) */
#define configSP_MAX_DATA_SIZE     496  

/** Telecommand TF size (without SYNC); maximum as defined in CCSDS 232.0-B-3 is 1024 */
#define configTC_TF_MAX_SIZE       508  

/** Telemetry TF size (without SYNC) */
#define configTM_TF_TOTAL_SIZE     508  

/** The OCF field (which contains the CLCW needed for automatic re-transfer) is optional */
#define configTF_USE_OCF             1  

/** The Frame Error Control Field (FECF) contains the CRC of telemetry packets */
#define configTF_USE_FECF            1

/** The Segment Header contains the Multiplexer Access Point (MAP) */
#define configTF_TC_USE_SEG_HDR      1

/** The CLTU sequence must eb able to hold the whole Transferframe for uplink 
 *  The calculation  is (2+(((configTC_TF_MAX_SIZE+6)/7)*8+8)
 */
#define configCLTU_MAX_SIZE        594   


#endif // _CONFIG_CCSDS_H_
