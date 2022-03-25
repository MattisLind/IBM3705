# IBM3705
IBM 3705 FEP Emulator based on SIMH V3 and interface for Hercules 390 

Below is an Informer 213 terminal connected to the Hercules IBM mainframe emulator. The Informer 213 is IBM 3178 / 3274 compatible. It has a V.24 serial port for connecting to the host. Originally this is made to a TB; 3704 / 3705 communication controller. The 3705 handles the SDLC protocol and interfaces with the mainframe. This pieace of code here emulates the IBM 3705. To allow for interfacing with real world hardware a small piece of hardware need to be added called the ([SyncDongle](https://github.com/MattisLind/alfaskop_emu/tree/master/hardware/SyncDongle)). The SyncDongle runs a [SDLC firmware](https://github.com/MattisLind/alfaskop_emu/tree/master/Utils/SDLCBridge) that interfaces to the host computer that runs the IBM 3705 emulation using a USB port.

The software in this repo is in early alpha stage and is just working and not very stable. IOt is mostly a fork of the work that Henk & Edwin has done with additions to interface to the SyncDongle.

![Informer 213](https://i.imgur.com/pRhDuGn.jpg)


