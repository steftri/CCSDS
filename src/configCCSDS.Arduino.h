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

/** the maximum number of spacecraft IDs which can be checked by tmtc client */
#define configTMTC_MAX_SCIDS         2  

/** FrameSeqNumber window for AD mode, see 232.1-b-2 */
#define configFARM_SLIDING_WINDOW_WIDTH 16  

#ifndef configUSE_CLTU_SUPPORT             // this is needed for test cases
/** CLTUs are used to syncronize to the uplink data stream. On TET1, this is done by hardware. */
# define configUSE_CLTU_SUPPORT       0
#endif

/** The CLTU sequence must eb able to hold the whole Transferframe for uplink 
 *  The calculation  is (2+(((configTC_TF_MAX_SIZE+6)/7)*8+8)
 */
#define configCLTU_MAX_SIZE         66   


#endif // _CONFIG_CCSDS_H_
