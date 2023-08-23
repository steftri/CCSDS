#ifndef _CCSDS_H_
#define _CCSDS_H_

/*! \mainpage CCSDS
 *
 * \section intro Introduction
 *
 * This is an implementation of a CCSDS compliant Transferframe and SpacePacket library as used in 
 * satellites for transferring telecommands and data from ground to the satellite and telemetry from satellite to ground.
 *
 * \section install Installation
 *
 * See <a href="../../readme.md">readme.md</a> for usage.
 */

#include "ccsds_cltu.h"

#include "ccsds_clcw.h"
#include "ccsds_transferframe_tc.h"
#include "ccsds_transferframe_tm.h"

#include "ccsds_spacepacket.h"

#include "pus_tc.h"

#endif