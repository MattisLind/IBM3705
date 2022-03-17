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
   3) This code is pre-positined for multiple lines, but correctly only
      suitable for only ONE line.

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
   +-- no DSR | no DCD-- [F] Disable

   L2 = Level 2 interrupt
   See IBM 3705 hardware documentation for more details.
*/

#include "sim_defs.h"
#include "i3705_defs.h"
#include "i3705_sdlc.h"
#include "i3705_Eregs.h"               /* External regs defs */
#include <signal.h>
#include <ctype.h>
#include <time.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <termios.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <assert.h>

#define MAX_TBAR   3                   // ICW table size (4 line sets)

extern int32 debug_reg;
extern int32 Eregs_Inp[];
extern int32 Eregs_Out[];
extern int8  svc_req_L2;               /* SVC L2 request flag */
extern FILE *trace;
extern int32 lvl;
extern int32 cc;

extern int Ireg_bit(int reg, int bit_mask);
extern void wait();

int8 icw_pdf_reg = EMPTY;              /* Status ICW PDF reg: ncp FILLED pdf for Tx */
                                       /*                     ncp EMPTY pdf during Rx */
int8 Rsp_buf     = EMPTY;              /* Status PIU response buf (FILLED/EMPTY) */
int  Plen;                             /* Length of PIU response */
int8 Eflg_rvcd = ON;                        /* Eflag received */

/* ICW Local Store Registers */
int  abar;
uint8_t  icw_scf[MAX_TBAR];               /* ICW[ 0- 7] SCF - Secondary Control Field */
uint8_t  icw_pdf[MAX_TBAR];               /* ICW[ 8-15] PDF - Parallel Data Field */
uint8_t icw_lcd[MAX_TBAR];               /* ICW[16-19] LCD - Line Code Definer */
uint8_t icw_pcf[MAX_TBAR];               /* ICW[20-23] PCF - Primary Control Field */
uint8_t  icw_sdf[MAX_TBAR];               /* ICW[24-31] SDF - Serial Data Field */
                                       /* ICW[32-33] Not implemented (OSC sel bits) */
uint16_t icw_Rflags[MAX_TBAR];           /* ICW[34-47] flags */
uint8_t icw_pcf_prev[MAX_TBAR];          /* Previous icw_pcf */
uint8_t icw_lne_stat[MAX_TBAR];          /* Line state: RESET, TX, RX */

uint8_t icw_pcf_new = 0x0;
uint8_t icw_pcf_mod = 0x00;
int8 CS2_req_L2_int = OFF;

pthread_mutex_t icw_lock;              /* ICW lock (0 - 45)  */
extern pthread_mutex_t r77_lock;       /* I/O reg x'77' lock */

unsigned char pcap_buf[1024];
int pcap_ptr = 0;
FILE * pcap_file;

typedef unsigned int guint32;
typedef unsigned short guint16; 
typedef int gint32;

typedef struct pcap_hdr_s {
  guint32 magic_number;   /* magic number */
  guint16 version_major;  /* major version number */
  guint16 version_minor;  /* minor version number */
  gint32  thiszone;       /* GMT to local correction */
  guint32 sigfigs;        /* accuracy of timestamps */
  guint32 snaplen;        /* max length of captured packets, in octets */
  guint32 network;        /* data link type */
} pcap_hdr_t;

typedef struct pcaprec_hdr_s {
  guint32 ts_sec;         /* timestamp seconds */
  guint32 ts_usec;        /* timestamp microseconds */
  guint32 incl_len;       /* number of octets of packet saved in file */
  guint32 orig_len;       /* actual length of packet */
} pcaprec_hdr_t;


int openSerial () {
  int ret;
  int fd;
  const char * devStr = "/dev/ttyACM0";
  struct termios options;
  unsigned char ch;
  /* open the port */
  fprintf(stderr, "devStr=%s\n", devStr);
  fd = open(devStr, O_RDWR | O_NOCTTY | O_NDELAY);
  fprintf(stderr, "open fd=%d", fd);
  ret = fcntl(fd, F_SETFL, 0);
  if (ret == -1) {
    perror("Error:");
  }
  fprintf(stderr, "fcntl ret=%d",ret);
  
  /* set raw input, 1 second timeout */
  
  
  options.c_iflag     = 0;
  options.c_oflag     = 0; 
  options.c_cflag     = 0;
  options.c_lflag     = 0;

  cfsetispeed(&options, B19200);
  cfsetospeed(&options, B19200);

  options.c_cflag  |=  (CS8 | CLOCAL | CREAD);

  for (int i=0; i<NCCS; i++) options.c_cc[i]  = 0;

  options.c_cc[VMIN]  = 0;
  options.c_cc[VTIME] = 1;
  
  /* set the options */
  ret=tcsetattr(fd, TCSANOW, &options);
  sleep(2); //required to make flush work, for some reason
  tcflush(fd,TCIOFLUSH);
  fprintf(stderr, "tcsetattr ret=%d\n", ret);
  return fd;
}

void Put_ICW(int i);
void Get_ICW(int i);

/* Function to be run as a thread always must have the same signature:
   it has one void* parameter and returns void                        */

void *CS2_thread(void *arg) {
   int t;                              // ICW table index pointer
   int j = 0;                          // Tx/Rx buffer index pointer
   int i,c, fd, first_char_received=0, escape = 0;
   register char *s;
   int start_of_frame = 1;
   char end_frame[2] = {0xff,0xef};
   pcap_hdr_t pcap;
   pcaprec_hdr_t pcap_rec;
   struct timeval tv;
   // open serial port 
   fd = openSerial();
   pcap_file = fopen ("i3705-sdlc.pcap", "w");
   if (pcap_file == NULL) {
     fprintf(stderr, "Failed to open pcap - file.\r\n");
   }
   pcap.magic_number = 0xa1b2c3d4;   /* magic number */
   pcap.version_major = 2;  /* major version number */
   pcap.version_minor = 4;  /* minor version number */
   pcap.thiszone = 0;       /* GMT to local correction */
   pcap.sigfigs = 0;        /* accuracy of timestamps */
   pcap.snaplen = 2048;        /* max length of captured packets, in octets */
   pcap.network = 268;        /* data link type */

   fwrite (&pcap, sizeof pcap, 1, pcap_file);
   fflush(pcap_file);
   fprintf(stderr, "\nCS2: thread %ld started succesfully...\n",syscall(SYS_gettid));
   int ch, ret;
   while(1) {
//    for (i = 0; i < MAX_TBAR; i++) {     // Pending multiple line support !!!
         t = 0;                            // Temp for ONE line set.
         icw_scf[t] |= 0x08;               // Turn DCD always on.

         // Obtain ICW lock to avoid sync issues with NCP coding
  
         pthread_mutex_lock(&icw_lock);
         if (icw_pcf[t] != icw_pcf_new) {  // pcf changed by NCP ?
            if (debug_reg & 0x40)   // Trace PCF state ?
	      //fprintf(stderr, "\n>>> CS2[%1X]: NCP changed PCF to %1X \n\r",icw_pcf[t], icw_pcf_new);
            if (icw_pcf_new == 0x0)        // NCP changed PCF = 0 ?
               icw_lne_stat[t] = RESET;    // Line state = RESET
            icw_pcf_prev[t] = icw_pcf[t];  // Save current pcf and
            icw_pcf[t] = icw_pcf_new;      // set new current pcf
         }

         switch (icw_pcf[t]) {
            case 0x0:                  // NO-OP
               if (icw_pcf_prev[t] != icw_pcf[t]) {
		 //if (debug_reg & 0x40)    // Trace PCF state ?
		    //fprintf(stderr, "\n>>> CS2[%1X]: PCF = 0 entered, next PCF will be set by NCP \n\r", icw_pcf[t]);
               }
         //      icw_lne_stat[t] = RESET;  // Line state = RESET
               icw_scf[t] &= 0x4A;     // Reset all check cond. bits.
               break;

            case 0x1:                  // Set mode
               if (icw_pcf_prev[t] != icw_pcf[t]) {  // First entry ?
                  if (debug_reg & 0x40)  // Trace PCF state ?
		    //fprintf(stderr, "\n>>> CS2[%1X]: PCF = 1 entered, next PCF will be 0 \n\r", icw_pcf[t]);
                  icw_scf[t] |= 0x40;  // Set norm char serv flag
                  icw_pcf_new = 0x0;   // Goto PCF = 0...
                  CS2_req_L2_int = ON; // ...and issue a L2 int
               }
               break;

            case 0x2:                  // Mon DSR on
               if (icw_pcf_prev[t] != icw_pcf[t]) {  // First entry ?
                  if (debug_reg & 0x40)   // Trace PCF state ?
		    //fprintf(stderr, "\n>>> CS2[%1X]: PCF = 2 entered, next PCF will be set by NCP \n\r", icw_pcf[t]);
                  icw_scf[t] |= 0x40;  // Set norm char serv flag
                  icw_pcf_new = 0x0;   // Goto PCF = 4... (Via PCF = 0)
                  CS2_req_L2_int = ON; // ...and issue a L2 int
               }
               break;

            case 0x3:                  // Mon RI or DSR on
               if (icw_pcf_prev[t] != icw_pcf[t]) {  // First entry ?
                  if (debug_reg & 0x40)   // Trace PCF state ?
                     fprintf(stderr, "\n>>> CS2[%1X]: PCF = 3 entered, next PCF will be 0 \n\r", icw_pcf[t]);
                  icw_scf[t] |= 0x40;  // Set norm char serv flag
                  icw_pcf_new = 0x0;   // Goto PCF = 0...
                  CS2_req_L2_int = ON; // ...and issue a L2 int
               }
               break;

            case 0x4:                  // Mon 7E flag - block DSR error
            case 0x5:                  // Mon 7E flag - allow DSR error
	      icw_scf[t] &= 0xFB;     // Reset 7E detected flag
               if (icw_pcf_prev[t] != icw_pcf[t]) {  // First entry ?
                  if (debug_reg & 0x40) {  // Trace PCF state ?
                     fprintf(stderr, "\n>>> CS2[%1X]: PCF = 5 entered, next PCF will be 6 \n\r", icw_pcf[t]);
                     fprintf(stderr, "\n<<< CS2[%1X]: Receiving PDF = *** %02X ***, j = %d \n\r", icw_pcf[t], icw_pdf[t], j-1);
                  }
               }
               j = 0;                          // Reset buffer pointer
               if (icw_lne_stat[t] == RESET)   // Line is silent. Wait for NCP time out.
                  break;
               if (icw_lne_stat[t] == TX)      // Line is silent. Wait for NCP action.
                  break;
	       //	       if (Eflg_rvcd == ON) {
	       //readmore:		 
	       ret = read (fd, &ch, 1);
	       //fprintf(stderr, "Read ret=%d ch=%02X\r\n");
               // Line state is receiving, wait for BFlag...
               if (ret == 1) {                 // we got a character? That is the start of the frame.
		 fprintf(stderr, "Read %02X\r\n",ch);
		  if (ch == 0xff) {
		    fprintf (stderr, "Got escape while waiting for leading frame marker.\r\n");
		    escape = 1;
		    break;
		  }
		  if (escape) {
		    if (ch == 0xef) { // End of frame marker
		      ch = 0x7e;
		      Eflg_rvcd = ON;
		      pcap_ptr=0;
		      fprintf (stderr, "Got End Flag - while waiting for leading frame marker\r\n");
		      break;
		    }
		    escape = 0;		    
		  }
		  //if (ch != 0xC1) goto readmore;
		  if (Eflg_rvcd == ON) {
		    fprintf(stderr, "Eflg_rvcd = ON processing a start of next frame.\r\n");
		    Eflg_rvcd = OFF;
	      
		  icw_scf[t]  |= 0x04;         // Set flag detected. (NO Serv bit)
		  icw_pdf[t] = 0x7E;
		  icw_scf[t]  &= 0xBF;         // turn off service flag
                  icw_lcd[t]   = 0x9;          // LCD = 9 (SDLC 8-bit)
                  icw_pcf_new  = 0x6;          // Goto PCF = 6...
                  CS2_req_L2_int = ON;         // ...and issue a L2 int
		  }
               } //}
               break;

            case 0x6:                  // Receive info-inhibit data interrupt
               if ((svc_req_L2 == ON) || (lvl == 2))  // If L2 interrupt active ?
                  break;                              // Loop till inactive...
	       icw_pdf[t] = ch;
	       pcap_buf[pcap_ptr++] = ch;
               if (debug_reg & 0x40) { // Trace PCF state ?
                  fprintf(stderr, "\n>>> CS2[%1X]: PCF = 6 entered, next PCF will be 7 \n\r", icw_pcf[t]);
                  fprintf(stderr, "\n<<< CS2[%1X]: Receiving PDF = *** %02X ***, j = %d \n\r", icw_pcf[t], icw_pdf[t], j-1);
               }
               icw_scf[t] |= 0x40;     // Set norm char serv flag
               icw_scf[t] &= 0xFB;     // Reset 7E detected flag
               icw_pdf_reg = FILLED;
	       fprintf(stderr, "\n<<< CS2[%1X]: Rx Char interrupt = *** %02X ***, j = %d \n\r", icw_pcf[t], icw_pdf[t], j-1);
	       icw_pcf_new = 0x7;      // Goto PCF = 7...
               CS2_req_L2_int = ON;    // ...and issue a L2 int
               break;

            case 0x7:                  // Receive info-allow data interrupt
               if ((svc_req_L2 == ON) || (lvl == 2))  // If L2 interrupt active ?
                  break;                              // Loop till inactive...

               if (icw_pdf_reg == EMPTY) {   // NCP has read pdf ?
                  Eflg_rvcd = OFF;   // No Eflag
		  ret = read (fd, &ch, 1);
		  if (ret == 0) break;
		  fprintf(stderr, "Read ch=%02X\r\n",ch);
		  if (ch == 0xff) {
		    fprintf (stderr, "Got escape\r\n");
		    escape = 1;
		    break;
		  }
		  if (escape) {
		    if (ch == 0xef) { // End of frame marker
		      ch = 0x7e;
		      Eflg_rvcd = ON;
		      fprintf (stderr, "Got End Flag\r\n");
		    }
		    escape = 0;		    
		  }
                  icw_pdf[t] = ch; // Get received byte
		  pcap_buf[pcap_ptr++] = ch;
                  if (debug_reg & 0x40) {    // Trace PCF state ?
                     fprintf(stderr, "\n<<< CS2[%1X]: PCF = 7 (re-)entered \n\r", icw_pcf[t]);
                     fprintf(stderr, "\n<<< CS2[%1X]: Receiving PDF = *** %02X ***, j = %d \n\r", icw_pcf[t], icw_pdf[t], j-1);
                  }
                  if (Eflg_rvcd == ON) {     // EFlag received ?
		    fprintf(stderr, "End flag processing\r\n");
                    icw_lne_stat[t] = TX;   // Line turnaround to transmitting...
                    icw_scf[t] |= 0x44;     // Set char serv and flag det bit
                    icw_pcf_new = 0x6;      // Go back to PCF = 6...
                    CS2_req_L2_int = ON;    // Issue a L2 interrupt
	     // write pcap_buf to file
		    gettimeofday(&tv,NULL);
		    pcap_rec.ts_sec = tv.tv_sec;
		    pcap_rec.ts_usec = tv.tv_usec;
		    pcap_rec.incl_len = (pcap_ptr-3)>0?pcap_ptr-3:0;
		    pcap_rec.orig_len = (pcap_ptr-3)>0?pcap_ptr-3:0;
		    fwrite (&pcap_rec, sizeof pcap_rec, 1, pcap_file);
		    fwrite (pcap_buf, (pcap_ptr-3)>0?pcap_ptr-3:0, 1, pcap_file);
		    fflush(pcap_file);
		    pcap_ptr = 0;
                  } else {
		    fprintf(stderr, "Normal processing\r\n");
                    icw_pdf_reg = FILLED;   // Signal NCP to read pdf.
		    fprintf(stderr, "\n<<< CS2[%1X]: Rx Char interrupt = *** %02X ***, j = %d \n\r", icw_pcf[t], icw_pdf[t], j-1);     
                    icw_scf[t] |= 0x40;     // Set norm char serv flag
                    icw_pcf_new = 0x7;      // Stay in PCF = 7...
                    CS2_req_L2_int = ON;    // Issue a L2 interrupt
                  }
               }
               break;

            case 0x8:                  // Transmit initial-turn RTS on
	      if (debug_reg & 0x40) {   // Trace PCF state ?
                fprintf(stderr, "\n>>> CS2[%1X]: PCF = 8 entered, next PCF will be 9 \n\r", icw_pcf[t]);
	        fprintf(stderr, "\n>>> CS2[%1X]: SCF=%02X \n\r", icw_pcf[t], icw_scf[t]);
	      }
	      icw_scf[t] &= 0xFB;     // Reset flag detected flag
	      // CTS is now on.
	      icw_pcf_new = 0x9;      // Goto PCF = 9
	      j = 0;                  // Reset Tx buffer pointer.
	      // NO CS2_req_L2_int !
	      break;

            case 0x9:                  // Transmit normal
               if ((svc_req_L2 == ON) || (lvl == 2))  // If L2 interrupt active ?
                  break;
               if (icw_pdf_reg == FILLED) {   // New char avail to xmit ?
                  if (debug_reg & 0x40) { // Trace PCF state ?
                     fprintf(stderr, "\n>>> CS2[%1X]: PCF = 9 (re-)entered \n\r", icw_pcf[t]);
                     fprintf(stderr, "\n>>> CS2[%1X]: Transmitting PDF = *** %02X ***, j = %d \n\r", icw_pcf[t], icw_pdf[t], j);
		     fprintf(stderr, "\n>>> CS2[%1X]: SCF=%02X \n\r", icw_pcf[t], icw_scf[t]);
                  }
		  if (icw_scf[t]&0x01) {   // send flag byte
		    if (start_of_frame==1) {
		      start_of_frame=0;
		      pcap_ptr = 0;
		      fprintf (stderr, "Start of frame!\r\n");
		    } else {
		      start_of_frame=1;
		      write(fd, end_frame, 2);
		      fprintf(stderr, "End of frame! Send 0xff 0xef. \r\n");
		      gettimeofday(&tv,NULL);
		      pcap_rec.ts_sec = tv.tv_sec;
		      pcap_rec.ts_usec = tv.tv_usec;
		      pcap_rec.incl_len = (pcap_ptr-2)>0?pcap_ptr-2:0;
		      pcap_rec.orig_len = (pcap_ptr-2)>0?pcap_ptr-2:0;
		      fwrite (&pcap_rec, sizeof pcap_rec, 1, pcap_file);
		      fwrite (pcap_buf, (pcap_ptr-2)>0?pcap_ptr-2:0, 1, pcap_file);
		      fflush(pcap_file);
		      pcap_ptr = 0;
		    }
		  } else {
		    fprintf (stderr, "Wrote byte=%02X \r\n", icw_pdf[t]);
		    pcap_buf[pcap_ptr++] = icw_pdf[t];
		    write(fd, &icw_pdf[t], 1);
		  }
                  // Next byte please...
                  icw_pdf_reg = EMPTY; // Ask NCP for next byte
                  icw_scf[t] |= 0x40;  // Set norm char serv flag
                  icw_pcf_new = 0x9;   // Stay in PCF = 9...
                  CS2_req_L2_int = ON; // Issue a L2 interrupt
               }
               break;

            case 0xA:                  // Transmit normal with new sync
               break;
            case 0xB:                  // Not used
               break;

            case 0xC:                  // Transmit turnaround-turn RTS off
               if (icw_pcf_prev[t] != icw_pcf[t]) {  // First entry ?
                  if (debug_reg & 0x40)   // Trace PCF state ?
                     fprintf(stderr, "\n>>> CS2[%1X]: PCF = C entered, next PCF will be set by NCP \n\r", icw_pcf[t]);
		  // Send here end of frame here!
		  
                  icw_lne_stat[t] = RX;   // Line turnaround to receiving...
                  j = 0;                  // Reset Rx buffer pointer.
                  icw_scf[t] |= 0x40;     // Set norm char serv flag
                  icw_pcf_new = 0x5;      // Goto PCF = 5...
                  CS2_req_L2_int = ON;    // ...and issue a L2 int
               }
               break;

            case 0xD:                     // Transmit turnaround-keep RTS on
               if (icw_pcf_prev[t] != icw_pcf[t]) {  // First entry ?
                  if (debug_reg & 0x40)   // Trace PCF state ?
                     fprintf(stderr, "\n>>> CS2[%1X]: PCF = D entered, next PCF will be set by NCP \n\r", icw_pcf[t]);
               }
               // NO CS2_req_L2_int !
               break;

            case 0xE:                  // Not used
               break;

            case 0xF:                  // Disable
               if (icw_pcf_prev[t] != icw_pcf[t]) {
                  if (debug_reg & 0x40)  // Trace PCF state ?
                     fprintf(stderr, "\n>>> CS2[%1X]: PCF = F entered, next PCF will be set by NCP \n\r", icw_pcf[t]);
               }
               icw_scf[t] |= 0x40;     // Set norm char serv flag
               icw_pcf_new = 0x0;      // Goto PCF = 0...
               CS2_req_L2_int = ON;    // ...and issue a L2 int
               break;

         }     // End of switch (icw_pcf[t])

         // =========  POST-PROCESSING SCAN CYCLE  =========

         if (CS2_req_L2_int) {         // CS2 L2 interrupt request ?
            if (debug_reg & 0x40)      // Trace PCF state ?
               fprintf(stderr, "\n>>> CS2[%1X]: SVCL2 interrupt issued for PCF = %1X \n\r",
                       icw_pcf_prev[t], icw_pcf_prev[t]);
            pthread_mutex_lock(&r77_lock);
 //         Eregs_Inp[0x77] |= 0x4000; // Indicate L2 scanner interrupt
            pthread_mutex_unlock(&r77_lock);
            svc_req_L2 = ON;           // Issue a level 2 interrrupt
            CS2_req_L2_int = OFF;      // Reset int req flag
         }
         icw_pcf_prev[t] = icw_pcf[t]; // Save current pcf
         if (icw_pcf[t] != icw_pcf_new) {   // pcf state changed ?
            icw_pcf[t] = icw_pcf_new;       // set new current pcf
         }
         //if (debug_reg & 0x40)         // Trace Prev / Curr PCF state ?
         //   fprintf(stderr, "\n>>> CS2[%1X]: Next PCF = %1X \n\r",
         //           icw_pcf_prev[t], icw_pcf[t]);
         // Release the ICW lock
         pthread_mutex_unlock(&icw_lock);
         usleep(1000);                 // Time window for NCP to set PCF.
//    }                                // Pending multiple line support !!!
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
   icw_Rflags[tbar] = (Eregs_Out[0x47] << 4) & 0x00DF;

   return;
}
#endif

/* Copy ICW[ABAR] to input regs */
void Get_ICW(int abar) {               // See 3705 CE manauls for details.
   int tbar = (abar - 0x0840) >> 1;    // Get ICW table ptr from abar
   Eregs_Inp[0x44]  = (icw_scf[tbar] << 8)  | icw_pdf[tbar];
   Eregs_Inp[0x45]  = (icw_lcd[tbar] << 12) | (icw_pcf[tbar] << 8) | icw_sdf[tbar];
   Eregs_Inp[0x46]  =  0xF0A5;             // Display reg (tbd)
   Eregs_Inp[0x47]  =  icw_Rflags[tbar];   // ICW 32 - 47

   return;
}

