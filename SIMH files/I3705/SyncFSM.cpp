/*

SyncFSM.cpp 

Handles finding the sync position. When in sync it will dispatch the receivedSyncData for each character received. Typically
used by the message FSM.
 
The Sync FSM has two states, Hunt state and Sync state. When in Hunt state it will search for the BSC SYNC characters, SYN SYN, 0x32 0x32.

*/

#include "SyncFSM.h"

// Construtcor taks as argument a callback to call when sync is established and valid data is received

SyncFSM::SyncFSM(void (*recivedSyncData)(unsigned char)) {
  cb = recivedSyncData;
  dataWord = 0xffffffff;
  syncState = HUNT; 
}

// Is called by the data receiver 

void SyncFSM::receivedData(unsigned char read) {
  int i;
  fprintf (stderr, "Received %02X\n", read);
  dataWord = dataWord << 8;  
  dataWord = (0xffffff00 & dataWord) | (0xff & read);
  if (syncState==HUNT) { // Hunting for SYNC
    // Try to find sync
    if ((dataWord  & 0xffff) == 0x3232) {
      fprintf(stderr, "Found SYNC!\n");
      syncState = SYNC; // We have found sync
      cb(SYN);
      cb(SYN);
    }
  }
  else {
    cb(read);
  } 
} 

// Routine to tell the Sync FSM to enter Hunt state

void SyncFSM::enterHuntState() {
  syncState = HUNT;
}