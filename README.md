# IBM3705
IBM 3705 FEP Emulator based on SIMH V3 and interface for Hercules 390 by Henk & Edwin.
Additions to support real hardware by me.

Below is an Informer 213 terminal connected to the Hercules IBM mainframe emulator. The Informer 213 is IBM 3178 / 3274 compatible. It has a V.24 serial port for connecting to the host. Originally this is made to an IBM 3704 / 3705 communication controller. The 3705 handles the SDLC protocol and interfaces with the mainframe. This pieace of code here emulates the IBM 3705. To allow for interfacing with real world hardware a small piece of hardware need to be added called the [SyncDongle](https://github.com/MattisLind/alfaskop_emu/tree/master/hardware/SyncDongle). The SyncDongle runs a [SDLC firmware](https://github.com/MattisLind/alfaskop_emu/tree/master/Utils/SDLCBridge) that interfaces to the host computer that runs the IBM 3705 emulation using a USB port.

The software in this repo is in early beta stage and seems to work quite stable. Cleanup of the code is much needed. The serial port is currently hardcoded as /dev/ttyACM0. It has been noted that after long periods of idle time the communication between the Hercules emulator and the IBM 3705 emulator has stopped. The reason for this is unknown and remain to be investigated. It is mostly a fork of the work that Henk & Edwin has done with additions to interface to the SyncDongle.

One major culprit to get this working was to remove the "NOTIFY" message sent by the terminal to the host. The host running MVS 3.8 was not at all happy receiving the NOTIFY, responding with an "UNSUPPORTED FUNCTION" error message. The solution was to buffer the incoming messages and remove the NOTIFY message. This also meant that the N(S) and N(R) counter used for SDLC delivery of information frames needed rewriting as well as the CRC of each frame.


![Informer 213](https://i.imgur.com/pRhDuGn.jpg)


