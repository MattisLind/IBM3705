# IBM3705
IBM 3705 FEP Emulator based on SIMH V3 and interface for Hercules 390 

Below is an Informer 213 terminal connected to the Hercules IBM mainframe emulator. The Informer 213 is IBM 3178 / 3274 compatible. It has a V.24 serial port for connecting to the host. Originally this is made to an IBM 3704 / 3705 communication controller. The 3705 handles the SDLC protocol and interfaces with the mainframe. This pieace of code here emulates the IBM 3705. To allow for interfacing with real world hardware a small piece of hardware need to be added called the [SyncDongle](https://github.com/MattisLind/alfaskop_emu/tree/master/hardware/SyncDongle). The SyncDongle runs a [SDLC firmware](https://github.com/MattisLind/alfaskop_emu/tree/master/Utils/SDLCBridge) that interfaces to the host computer that runs the IBM 3705 emulation using a USB port.

The software in this repo is in early alpha stage and is just working and not very stable. It is mostly a fork of the work that Henk & Edwin has done with additions to interface to the SyncDongle.

One major culprit to get this working was to remove the "NOTIFY" message sent by the terminal to the host. The host running MVS 3.8 was not at all happy receiving the NOTIFY, responding with an "UNSUPPORTED FUNCTION" error message. The solution was to buffer the incoming messages and remove the NOTIFY message. This also meant that the N(S) and N(R) counter used for SDLC delivery of information frames needed rewriting as well as the CRC of each frame.

A remaining problem is that sometimes the IBM 3705 emulation is not setting the "Disable Zero-insert" control flag when sending the frame flag byte. This causes the communication to break down. Since this happens after quite some time it is likely to be a synchronisation problem between the NCP program running in the IBM 3705 emulator and the Scanner software that interfaces with the SDLC line. A more detailed understanding of how interrupts in the 3705 interacts with the Scanner is needed.

![Informer 213](https://i.imgur.com/pRhDuGn.jpg)


