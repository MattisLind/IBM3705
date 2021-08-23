/* 3705_scan_T2.c: IBM 3705 Communication Scanner Type 2 simulator

   Copyright (c) 2020, Henk Stegeman and Edwin Freekenhorst

   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
   ROBERT M SUPNIK BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
   IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

   Except as contained in this notice, the name of Charles E. Owen shall not be
   used in advertising or otherwise to promote the sale, use or other dealings
   in this Software without prior written authorization from Charles E. Owen.

   ------------------------------------------------------------------------------

   Some notes:
   1) The scanner has:
      - a ICW work register and is implemented in Ereg_Inp[0x44->0x47]
        Function PutICW will transfer ICW work reg to ICW storage[ABAR].
      - a ICW local storage register (See below)
        Function GetICW will transfer ICW storage[ABAR] to ICW input reg.
      - a ICW input register and is implemented in Ereg_Out[0x44/45/47]
   2) The 2 bits in SDF for Business Clock Osc selection bits are
      not implemented.  Reason: for programming simplicity.
   3)

   *** Input to CS2 (CCU output) Eregs ***
   Label      Ereg         Function
   --------------------------------------------------------------
   CMBAROUT   0x40         // ABAR Interface address
   CMADRSUB   0x41         // Scanner addr substitution.
   CMSCANLT   0x42         // Upper scan limit modification.
   CMCTL      0x43         // CA Address and ESC status.
   CMICWB0F   0x44         // ICW  0 THRU 15
   CMICWLP    0x45         // ICW 16 THRU 23
   CMICWS     0x46         // ICW 24 THRU 33
   CMICWB34   0x47         // ICW 34 THRU 43

   *** Output from CS2 (CCU input) Eregs ***
   Label      Ereg         Function
   --------------------------------------------------------------
   CMBARIN    0x40         // ABAR Interface address
              0x41         // Unused
              0x42         // Unused
   CMERREG    0x43         // Scan Error register
   CMICWB0F   0x44         // ICW  0 THRU 15
   CMICWLPS   0x45         // ICW 16 THRU 31
   CMICWDSP   0x46         // Display register
   CMICWB32   0x47         // ICW 32 THRU 45

                        PCF state
   +------------> +---->  [0] NO-OP
   |              |
   |              |
   |              L2 <--  [1] Set Mode - DTR on
   |              ^
   |              |
   |         +----<-----  [2] Monitor DSR
   |         |    |
   |         |    |
   |         L2   +-----  [3] Monitor DSR or RI on
   |         |
   |         |
   |         +--------->  [4] Monitor flag - Block DSR error
   |         +-----flag--/
   |         |
   |    +----|--------->  [5] Monitor flag - Allow DSR error
   |    |    +-----flag--/
   |    |    v
   |    |    L2 ------->  [6] Receive Info - Block Data Interrupts
   |    |    ^    +------/
   |    |    |    L2
   |    |    |    +---->  [7] Receive Info - Allow Data Interrupts
   L2   |    +-----flag--/
   |    |
   |    L2   +-----CTS--  [8] Transmit Initial - RTS on
   |    |    |
   |    |    |
   |    |    +--------->  [9] Transmit Normal
   |    |
   |    |
   |    +-SDF is empty--  [C] Tx -> Rx turnaround RTS off
   |
   |
   +--- no DSR or no DCD- [F] Disable

   L2 = Level 2 interrupt
   See IBM 3705 hardware documentation for more details.
*/

#include "sim_defs.h"
#include "i3705_defs.h"
#include "i3705_sdlc.h"

#include "i3705_Eregs.h"               /* External regs defs */
#include <signal.h>
#include <ctype.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>

#define MAX_TBAR   3                   // ICW table size (4 line sets)

extern int32 debug_reg;
extern int32 Eregs_Inp[];
extern int32 Eregs_Out[];
extern int8  svc_req_L2;               /* SVC L2 request flag */
extern FILE *trace;
extern int32 lvl;
extern int32 cc;

int8 icw_pdf_reg = EMPTY;              /* Status ICW PDF reg (FILLED/EMPTY) */
int8 Rsp_buf     = EMPTY;              /* Status PIU response buf (FILLED/EMPTY) */
int8 Plen;                             /* Length of PIU response */
int8 Eflg_rvcd;                        /* Eflag received */

/* ICW Local Store Registers */
int  abar;
char icw_scf[MAX_TBAR];                /* ICW[ 0- 7] SCF - Secondary Control Field */
char icw_pdf[MAX_TBAR];                /* ICW[ 8-15] PDF - Parallel Data Field */
int8 icw_lcd[MAX_TBAR];                /* ICW[16-19] LCD - Line Code Definer */
int8 icw_pcf[MAX_TBAR];                /* ICW[20-23] PCF - Primary Control Field */
char icw_sdf[MAX_TBAR];                /* ICW[24-31] SDF - Serial Data Field */
                                       /* ICW[32-33] Not implemented (OSC sel bits) */
int16 icw_Rflags[MAX_TBAR];            /* ICW[34-45] flags */
int8 icw_pcf_prev[MAX_TBAR];           /* Previous icw_pcf */
int8 icw_lne_stat[MAX_TBAR];           /* Line state: RESET, TX, RX */

int8 icw_pcf_new = 0x0;
int8 CS2_req_L2_int = OFF;
char BLU_buf[65536];                     // DLC header + TH + RH + RU + DLC trailer
pthread_mutex_t icw_lock;              // ICW lock

void proc_BLU(char *BLU_buf, int j);
void Put_ICW(int i);
void Get_ICW(int i);

/* Function to be run as a thread always must have the same signature:
   it has one void* parameter and returns void                        */

void *CS2_thread(void *arg) {
   int t;                              /* ICW table index pointer */
   int j = 0;                          /* Tx/Rx buffer index pointer */
   int i;
   register char *s;

   fprintf(stderr, "\nCS2: thread started succesfully... \n");

   while(1) {
//      for (i = 0; i < MAX_TBAR; i++) {
         t = 0;                            // Temp for ONE line set.
         // Obtain the lock to avoid sync issues with NCP coding
         pthread_mutex_lock(&icw_lock);
         if (icw_pcf[t] != icw_pcf_new) {   // pcf changed by NCP ?
            icw_pcf_prev[t] = icw_pcf[t];   // Save current pcf and
            icw_pcf[t] = icw_pcf_new;       // set new current pcf
         }
//          if (debug_reg & 0x40)      // Trace Prev / Curr PCF state ?
//             fprintf(trace, "\n[%06d] >>> CS2- Current PCF = %1X (set by NCP, was %1X) \n\r",
//                    cc++, icw_pcf[t], icw_pcf_prev[t]);

         icw_scf[t] |= 0x08;  // Turn DCD always on.
         switch (icw_pcf[t]) {
            case 0x0:                  // NO-OP
               if (icw_pcf_prev[t] != icw_pcf[t]) {
                  if (debug_reg & 0x40)    /* Trace PCF state */
                     fprintf(trace, "\n>>> CS2: PCF = 0 entered\n\r");
               }
               icw_lne_stat[t] = RESET;  // Line state = RESET
               icw_scf[t] &= 0x4A;     // Reset all check cond. bits.
               break;

            case 0x1:                  // Set mode
               if (icw_pcf_prev[t] != icw_pcf[t]) {  // First entry ?
                  if (debug_reg & 0x40)   /* Trace PCF state */
                     fprintf(trace, "\n>>> CS2: PCF = 1 entered, next will be 0 \n\r");
                  icw_scf[t] |= 0x40;  // Set norm char serv flag
                  icw_pcf_new = 0x0;   // Goto PCF = 0...
                  CS2_req_L2_int = ON; // ...and issue a L2 int
               }
               break;

            case 0x2:                  // Mon DSR on
               if (icw_pcf_prev[t] != icw_pcf[t]) {  // First entry ?
                  if (debug_reg & 0x40)   /* Trace PCF state */
                     fprintf(trace, "\n>>> CS2: PCF = 2 entered, next will be 4 \n\r");
                  icw_scf[t] |= 0x40;  // Set norm char serv flag
                  icw_pcf_new = 0x4;   // Goto PCF = 4...
                  CS2_req_L2_int = ON; // ...and issue a L2 int
               }
               break;

            case 0x3:                  // Mon RI or DSR on
               if (icw_pcf_prev[t] != icw_pcf[t]) {  // First entry ?
                  if (debug_reg & 0x40)   /* Trace PCF state */
                     fprintf(trace, "\n>>> CS2: PCF = 3 entered, next will be 0 \n\r");
                  icw_scf[t] |= 0x40;  // Set norm char serv flag
                  icw_pcf_new = 0x0;   // Goto PCF = 0...
                  CS2_req_L2_int = ON; // ...and issue a L2 int
               }
               break;

            case 0x4:                  // Mon 7E flag - block DSR error
            case 0x5:                  // Mon 7E flag - allow DSR error
               icw_scf[t] &= 0xFB;     // Reset 7E detected flag
               if (icw_pcf_prev[t] != icw_pcf[t]) {  // First entry ?
                  if (debug_reg & 0x40) {  /* Trace PCF state */
                     fprintf(trace, "\n[%06d] >>> CS2: PCF = %1X entered, next will be 6 \n\r",
                             cc++, icw_pcf[t]);
                     fprintf(trace, "\n<<< CS2: Receiving PDF = ** %02X **, j = %d \n\r", icw_pdf[t], j-1);
                  }
               }
               j = 0;                          // Reset buffer pointer
               if (icw_lne_stat[t] == RESET)   // Line is silent. Wait for NCP time out.
                  break;
               if (icw_lne_stat[t] == TX) {    // Line is silent. Wait for NCP action.
                  break;
               }
               // Line state is receiving, wait for BFlag...
               if (BLU_buf[j] == 0x7E) {       // x'7E' Bflag received ?
                  icw_scf[t]  |= 0x04;         // Set flag detected. (NO Serv bit)
                  icw_lcd[t]   = 0x9;          // LCD = 9 (SDLC 8-bit)
                  icw_pcf_new  = 0x6;          // Goto PCF = 6...
                  CS2_req_L2_int = ON;         // ...and issue a L2 int
               }
               break;

            case 0x6:                  // Receive info-inhibit data interrupt
               icw_pdf[t] = BLU_buf[j++];
               if (debug_reg & 0x40) { // Trace PCF state
                  fprintf(trace, "\n<<< CS2: PCF = 6 entered, next will be 7\n\r");
                  fprintf(trace, "\n<<< CS2: Receiving PDF = ** %02X **, j = %d \n\r", icw_pdf[t], j-1);
               }
               if (icw_pdf[t] == 0x7E) // Flag ?  Skip it.
                  break ;
               icw_scf[t] |= 0x40;     // Set norm char serv flag
               icw_scf[t] &= 0xFB;     // Reset 7E detected flag
               icw_pdf_reg = FILLED;
               icw_pcf_new = 0x7;      // Goto PCF = 7...
               CS2_req_L2_int = ON;    // ...and issue a L2 int
               break;

            case 0x7:                  // Receive info-allow data interrupt
               if (icw_pdf_reg == EMPTY) {   // Next char ?
                  // Check for Eflag (for transparency x'470F7E' CRC + EFlag)
                  if ((BLU_buf[j - 2] == 0x47) &&    // CRC high
                      (BLU_buf[j - 1] == 0x0F) &&    // CRC low
                      (BLU_buf[j - 0] == 0x7E))  Eflg_rvcd = ON;
                    else Eflg_rvcd = OFF;    // No Eflag

                  icw_pdf[t] = BLU_buf[j++];
                  if (debug_reg & 0x40) {    /* Trace PCF state */
                     fprintf(trace, "\n[%06d] <<< CS2: PCF = 7 entered \n\r", cc++);
                     fprintf(trace, "\n<<< CS2: Receiving PDF = ** %02X **, j = %d \n\r", icw_pdf[t], j-1);
                  }
                  if (Eflg_rvcd == ON) {     // EFlag received ?
                     icw_lne_stat[t] = TX;   // Line turnaround to transmitting...
                     icw_scf[t] |= 0x44;     // Set char serv and flag det bit
                     icw_pcf_new = 0x6;      // Go back to PCF = 6...
                     CS2_req_L2_int = ON;    // Issue a L2 interrupt
                  } else {
                     icw_pdf_reg == FILLED;  // Signal NCP to read pdf.
                     icw_scf[t] |= 0x40;     // Set norm char serv flag
                     icw_pcf_new = 0x7;      // Stay in PCF = 7...
                     CS2_req_L2_int = ON;    // Issue a L2 interrupt
                  }
               }
               break;

            case 0x8:                  // Transmit initial-turn RTS on
               if (debug_reg & 0x40) { /* Trace PCF state */
                  fprintf(trace, "\n>>> CS2: PCF = 8 entered, next will be 9 \n\r");
               }
               icw_scf[t] &= 0xFB;     // Reset flag detected flag
               // CTS is now on.
               icw_pcf_new = 0x9;      // Goto PCF = 9
               j = 0;                  // Reset Tx buffer pointer.
               // NO CS2_req_L2_int !
               break;

            case 0x9:                  // Transmit normal
               if (icw_pdf_reg == FILLED) {   // New char avail to xmit ?
                  if (debug_reg & 0x40) { /* Trace PCF state */
                     fprintf(trace, "\n[%06d] >>> CS2: PCF = 9 entered \n\r", cc++);
                     fprintf(trace, "\n       >>> CS2: Transmitting PDF = ** %02X **, j = %d \n\r", icw_pdf[t], j);
                  }
                  BLU_buf[j++] = icw_pdf[t];
                  // Next byte please...
                  icw_pdf_reg = EMPTY; // Signal NCP for next byte
                  icw_scf[t] |= 0x40;  // Set norm char serv flag
                  CS2_req_L2_int = ON; // Issue a L2 interrupt
               }
               break;

            case 0xA:                  // Transmit normal with new sync
               break;
            case 0xB:                  // Not used
               break;

            case 0xC:                  // Transmit turnaround-turn RTS off
               if (icw_pcf_prev[t] != icw_pcf[t]) {  // First entry ?
                  if (debug_reg & 0x40) { /* Trace PCF state */
                     fprintf(trace, "\n>>> CS2: PCF = C entered, next will be set by NCP \n\r");
                  }
                  // ******************************************************************
                  proc_BLU(BLU_buf, j);   // Process received BLU and wait for response
                  // ******************************************************************
                  if (debug_reg & 0x20) {
                     fprintf(trace, "\n>>> CS2: BLU_buf = ");
                     for (s = (char *) &BLU_buf[0], i = 0; i < 16; ++i, ++s)
                        fprintf(trace, "%02X ", (int) *s & 0xFF);
                     fprintf(trace, "\n");
                  }
                  icw_lne_stat[t] = RX;   // Line turnaround to receiving...
                  j = 0;                  // Reset Rx buffer pointer.
                  icw_scf[t] |= 0x40;     // Set norm char serv flag
                  icw_pcf_new = 0x5;      // Goto PCF = 5...
                  CS2_req_L2_int = ON;    // ...and issue a L2 int
               }
               break;

            case 0xD:                  // Transmit turnaround-keep RTS on
               if (icw_pcf_prev[t] != icw_pcf[t]) {  // First entry ?
                  if (debug_reg & 0x40) { /* Trace PCF state */
                     fprintf(trace, "\n>>> CS2: PCF = D entered, next will be set by NCP \n\r");
                  }
               }
               // NO CS2_req_L2_int !
               break;

            case 0xE:                  // Not used
               break;

            case 0xF:                  // Disable
               if (icw_pcf_prev[t] != icw_pcf[t]) {
                  if (debug_reg & 0x40)   /* Trace PCF state */
                     fprintf(trace, "\n>>> CS2: PCF = F entered, next will be set by NCP \n\r");
               }
               icw_scf[t] |= 0x40;     // Set norm char serv flag
               icw_pcf_new = 0x0;      // Goto PCF = 0...
               CS2_req_L2_int = ON;    // ...and issue a L2 int
               break;

         }     // End of switch (icw_pcf[t])

         if (debug_reg)
            fprintf(trace, "\n>>> CS2b PCF - P C N = %1X %1X %1X  SCF = %02X \n\r",
                  icw_pcf_prev[t], icw_pcf[t], icw_pcf_new, icw_scf[t] );
         icw_pcf_prev[t] = icw_pcf[t];   // Save current pcf and
         if (icw_pcf[t] != icw_pcf_new) {   // pcf state changed ?
            icw_pcf[t] = icw_pcf_new;       // set new current pcf
            if (debug_reg & 0x40)      // Trace Prev / Curr PCF state ?
               fprintf(trace, "\n[%06d] >>> CS2: Current PCF = %1X  (was %1X) \n\r",
                      cc++, icw_pcf[t], icw_pcf_prev[t]);
         }
         if (CS2_req_L2_int) {         // ...with CS2 L2 interrupt ?
            if (debug_reg & 0x40)      /* Trace PCF state */
               fprintf(trace, "\n[%06d] >>> CS2: SVCL2 interrupt issued for PCF = %1X \n\r",
                      cc++, icw_pcf[t]);
            while (lvl == 2) {         // CCU still in lvl 2 processing ?
               pthread_mutex_unlock(&icw_lock);
               usleep(2500);           // Wait for CCU to exit prev L2 int.
               pthread_mutex_lock(&icw_lock);
            }
            icw_Rflags[t] |= 0x20;     // Lvl 2 SVC req
            svc_req_L2 = ON;           // Issue a level 2 interrrupt
            CS2_req_L2_int = OFF;      // Reset int req flag
         }
         // Release the lock
         pthread_mutex_unlock(&icw_lock);
         usleep(10000);                // Time window for NCP to set PCF.
//    }
   }     // End of while(1)...
return (0);
}

/* Copy output regs to ICW[ABAR] */
#if 0
void Put_ICW(int abar) {               // See 3705 CE manauls for details.
   int tbar = (abar - 0x0840) >> 1;    // get ICW table ptr from abar
   icw_scf[tbar]    = (Eregs_Out[0x44] >> 8) & 0x4A;   // Only Serv Req, DCD & Pgm Flag
   icw_pdf[tbar]    =  Eregs_Out[0x44] & 0x00FF;
   icw_lcd[tbar]    = (Eregs_Out[0x45] >> 4) & 0x0F;
   icw_pcf[tbar]    =  Eregs_Out[0x45] & 0x0F;
   icw_sdf[tbar]    = (Eregs_Out[0x46] >> 2) & 0xFF;
   icw_Rflags[tbar] = (Eregs_Out[0x47] << 6) & 0x3DF0;

   return;
}
#endif

/* Copy ICW[ABAR] to input regs */
void Get_ICW(int abar) {               // See 3705 CE manauls for details.
   int tbar = (abar - 0x0840) >> 1;    // get ICW table ptr from abar
   Eregs_Inp[0x44]  = (icw_scf[tbar] << 8)  | icw_pdf[tbar];
   Eregs_Inp[0x45]  = (icw_lcd[tbar] << 12) | (icw_pcf[tbar] << 8) | icw_sdf[tbar];
   Eregs_Inp[0x46]  =  0xF0A5;         // Display reg (tbd)
   Eregs_Inp[0x47]  =  icw_Rflags[tbar];

   return;
}

