# CCSDS

This is an implementation of a CCSDS compliant Transferframe and SpacePacket library as used in Satellites for transferring telecommands from ground to the satellite and telemetry form satellite to ground. 

## General

This is a beta release


## Usage


```TmTcClient g_TmTcClient(0x25C, NULL, &SendTelemetryCallback, NULL, &TelecommandReceiveCallback);
```

```// process simulated incoming data from ground station
g_TmTcClient.processCltu((uint8_t*)gau8_Tc, sizeof(gau8_Tc));
```

## Limitations

Limitations of Transferframes (Telemetry):                                                 
* The TM secondary header is not supported                 
* Randomization is not supported
  
Limitations of Transferframes (Telecommand):         
* The TC segment header is not supported    
  
Limitations of Space Packets:
* CCSDS secondary header format is not supportd 



## Known Anomalies

* The PUS CRC does not match the CRC of the example ones used for the BIROS satellite. 
* The AD mode acceptance window seems to be wrong


## Files & Configuration

* `tmtc_client.h`:  Contains the references to the protocol includes 






## Short Protocol Overview

TBD

### CLTUs

### Transferframes

### Space Packets

## License


## Website

Further information can be found on [Arduino section](http://www.trippler.de/stefan/arduino.php) of my web site.

