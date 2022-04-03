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
#include <sys/time.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <termios.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>

#define MAX_TBAR   3                   // ICW table size (4 line sets)
#define BAUD 19200                     // 19200 bps is approx 500 us per bit
#define BITTIME 20
#define BAUD_CODE 0x0b                 // 19200 bps 

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
int8 Eflg_rvcd;                        /* Eflag received */

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
uint8_t BLU_buf[65536];                   /* DLC header + TH + RH + RU + DLC trailer */
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
  //fprintf(stderr, "devStr=%s\n", devStr);
  fd = open(devStr, O_RDWR | O_NOCTTY | O_NDELAY);
  //fprintf(stderr, "open fd=%d", fd);
  ret = fcntl(fd, F_SETFL, 0);
  if (ret == -1) {
    perror("Error:");
  }
  //fprintf(stderr, "fcntl ret=%d",ret);



/* get the current options */
  ret = tcgetattr(fd, &options);
  //fprintf(stderr, "tcgetattr ret=%d",ret);
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
  //fprintf(stderr, "tcsetattr ret=%d\n", ret);
  return fd;
}

int readSDLC(int fd, unsigned char * buf, int * sum) {
  int ret=0;
  (*sum)=0;
  int end_of_frame = 0;
  do {
    ret = read (fd, buf+(*sum), 1);
    (*sum) += ret;
    //printFrame("Receive", buf, (*sum));
    if ((*sum) >1) {
      //fprintf (stderr, "%02x %02x ret=%d\n", 0xff &buf[(*sum)-1], 0xff&buf[(*sum)-2], ret);
      //fprintf (stderr, "%d %d\n", ((0xff & buf[(*sum)-1]) == 0xef), ((0xff & buf[(*sum)-2]) == 0xff));
      end_of_frame = ((((0xff & buf[*(sum)-1]) == 0xef) || ((0xff & buf[*(sum)-1]) == 0xfe)) && ((0xff & buf[*(sum)-2]) == 0xff));
    } else {
      end_of_frame = 0;
    }
  } while (!end_of_frame && ((*sum) > 0));
  //printFrame("Receive", buf, *sum);
  return  ((0xff & buf[*(sum)-1]) == 0xef);
}

int IBM3705_Sender_diff = 0; 
int Terminal_Sender_diff = 0;

static unsigned short calculateSDLCCrcChar (unsigned short crc, unsigned char data_p) {
  unsigned char i;
  unsigned int data;
  for (i=0, data=(unsigned int)0xff & data_p;
       i < 8; 
       i++, data >>= 1)
    {
      if ((crc & 0x0001) ^ (data & 0x0001))
	crc = (crc >> 1) ^ 0x8408;
      else  crc >>= 1;
    }
  return crc;
}


// direction: 0 = coming from IBM3705. 1 = coming from Terminal

void rewriteSDLC (int direction, unsigned char * buffer, int * size) {
  unsigned short crc = 0xffff;
  int adjustedSize;
  int ns, nr;
  //fprintf (stderr, "buffer[11]=%02X buffer[12]=%02X buffer[13]=%02X direction=%d\n", 0xff& buffer[11], 0xff&buffer[12], 0xff&buffer[13]&0xff, direction);
  // Match the incoming message and see if it is going to be dropped.
  if (buffer[11] == 0x81 && buffer[12] == 0x06 && buffer[13] == 0x20 && direction == 1 && *size > 13) {
    //fprintf (stderr, "Detected a packet that is going to be dropped.\n");
    if (buffer[1] & 0x10) {
      //fprintf(stderr, "Final bit set - replace with a new RR packet.\n");
      buffer[1] = (buffer[1] & 0xE0) | 0x11;  // RR
      *size = 4; // including the CRC					       
    } else {
      //fprintf(stderr, "Final bit not set - drop packet.\n");
      *size = 0;
    }
    // act as if the sender actually sent the I frame.
    Terminal_Sender_diff++;
    if (Terminal_Sender_diff==8) Terminal_Sender_diff=0;
    //fprintf(stderr, "Terminal_sender_diff is now %d\n", Terminal_Sender_diff);
  }
  // Now we need to handle the diffs and recalculate N(S) and N(R)
  if (direction) {

    if ((buffer[1] & 0x1) == 0) {
      // info
      ns = (buffer[1]>>1) & 0x7;
      nr = (buffer[1]>>5) & 0x7;
      //fprintf(stderr, "Got an info frame from the terminal with N(S)=%d and N(R)=%d\n", ns, nr);
      nr += IBM3705_Sender_diff;
      nr = nr & 0x7;
      ns -= Terminal_Sender_diff;
      ns = ns & 0x7;
      buffer[1] &= 0x11;
      buffer[1] |= (ns << 1);
      buffer[1] |= (nr << 5);
      //fprintf(stderr, "After adjusting > N(S)=%d N(R)=%d SDLC Control byte=%02X \n",ns, nr, buffer[1]);      
    } else if ((buffer[1] & 0x3) == 0x1) {
      // Supervisory frames like RR
      nr = (buffer[1]>>5) & 0x7;
      //fprintf(stderr, "Got an supervisory frame from the terminal with N(R)=%d\n", nr);
      nr += IBM3705_Sender_diff;
      nr = nr & 0x7;
      buffer[1] &= 0x1F;
      buffer[1] |= (nr << 5);      
      //fprintf(stderr, "After adjusting >  N(R)=%d SDLC Control byte =%02X \n", nr, buffer[1]);      
    } else if (buffer[1] == 0x93) {
      IBM3705_Sender_diff = 0; 
      Terminal_Sender_diff = 0;
      //fprintf(stderr, "Got a SNRM - resetting diff counts\n");      
    }
  } else {
    if ((buffer[1] & 0x1) == 0) {
      // info
      ns = (buffer[1]>>1) & 0x7;
      nr = (buffer[1]>>5) & 0x7;
      //fprintf(stderr, "Got an info frame from the IBM3705 with N(S)=%d and N(R)=%d\n", ns, nr);
      nr += Terminal_Sender_diff;
      nr = nr & 0x7;
      ns -= IBM3705_Sender_diff;
      ns = ns & 0x7;
      buffer[1] &= 0x11;
      buffer[1] |= (ns << 1);
      buffer[1] |= (nr << 5);
      //fprintf(stderr, "After adjusting > N(S)=%d N(R)=%d SDLC Control byte=%02X \n",ns, nr, buffer[1]);      
    } else if ((buffer[1] & 0x3) == 0x1) {
      // Supervisory frames like RR
      nr = (buffer[1]>>5) & 0x7;
      //fprintf(stderr, "Got an supervisory frame from the IBM3705 with N(R)=%d\n", nr);
      nr += Terminal_Sender_diff;
      nr = nr & 0x7;
      buffer[1] &= 0x1F;
      buffer[1] |= (nr << 5);      
      //fprintf(stderr, "After adjusting >  N(R)=%d SDLC Control byte =%02X \n", nr, buffer[1]);      
    } else if (buffer[1] == 0x93) {
      IBM3705_Sender_diff = 0; 
      Terminal_Sender_diff = 0;
      //fprintf(stderr, "Got a SNRM - resetting diff counts\n");
    }
  }
  adjustedSize = (*size);
  if (direction) { // from terminal
    adjustedSize -= 2;
  }
  adjustedSize -=2;
  if (buffer[adjustedSize-1] == 0xff) {
    adjustedSize--;  // go backwards to find a FF in the CRC that we need to remove
  }
  adjustedSize--;
  if (buffer[adjustedSize-1] == 0xff) {
    adjustedSize--;
  }
  adjustedSize--;
  for (int i=0; i < adjustedSize; i++) { // skip the CRC bytes of course.
    if (buffer[i] == 0xff) i++;
    crc = calculateSDLCCrcChar(crc,buffer[i]);
  }
  //fprintf(stderr, "Old CRC = %02X%02X new CRC=%04X\n", 0xff&buffer[adjustedSize], 0xff&buffer[adjustedSize+1], (~crc) & 0xffff);
  buffer[adjustedSize] =(~crc) & 0xff;
  if (buffer[adjustedSize] == 0xff) {
    adjustedSize++;
    buffer[adjustedSize] = 0xff;
  }
  adjustedSize++;
  buffer[adjustedSize] = ((~crc) >> 8) & 0xff;
  if (buffer[adjustedSize] == 0xff) {
    adjustedSize ++;
    buffer[adjustedSize] = 0xff;
  }
  adjustedSize++;
  buffer[adjustedSize++] = 0xff; // Add the end flag again since it might have been destroyed.
  buffer[adjustedSize++] = 0xef;
  (*size) = adjustedSize;
}


void proc_BLU(unsigned char *BLU_buf, int j);
void Put_ICW(int i);
void Get_ICW(int i);

void printFrame(const char * str, char * buf,int cnt) {
  fprintf (stderr, "%s %d bytes : ", str, cnt);
  for (int i=0; i<cnt; i++) {
    fprintf(stderr, "%02X ", 0xff &buf[i]);
  }
  fprintf (stderr, "\n");  
}


/* Function to be run as a thread always must have the same signature:
   it has one void* parameter and returns void                        */

void *CS2_thread(void *arg) {
   int t;                              // ICW table index pointer
   int j = 0;                          // Tx/Rx buffer index pointer
   int i,c, size, fd;
   register char *s;
   int normal_frame;
   int write_buffer_index = 0;
   unsigned char * write_buffer_ptr [8];
   int write_buffer_size [8];
   int end_flag = 0;
   int lastj = 0;
   
   pcap_hdr_t pcap;
   pcaprec_hdr_t pcap_rec;
   struct timeval tv;
   char setBaudRateCommand[] = {0xff, 0xf0, BAUD_CODE}; 
   fprintf(stderr, "\nCS2: thread %ld started succesfully...\n",syscall(SYS_gettid));
   fd = openSerial();
   write (fd, setBaudRateCommand, 3);  
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

   //fwrite (&pcap, sizeof pcap, 1, pcap_file);
   //fflush(pcap_file);
   while(1) {
//    for (i = 0; i < MAX_TBAR; i++) {     // Pending multiple line support !!!
         t = 0;                            // Temp for ONE line set.
         icw_scf[t] |= 0x08;               // Turn DCD always on.

         // Obtain ICW lock to avoid sync issues with NCP coding
  
         pthread_mutex_lock(&icw_lock);
         if (icw_pcf[t] != icw_pcf_new) {  // pcf changed by NCP ?
	   //            if (debug_reg & 0x40)   // Trace PCF state ?
           //    fprintf(stderr, "\n>>> CS2[%1X]: NCP changed PCF to %1X \n\r",
           //            icw_pcf[t], icw_pcf_new);
            if (icw_pcf_new == 0x0)        // NCP changed PCF = 0 ?
               icw_lne_stat[t] = RESET;    // Line state = RESET
            icw_pcf_prev[t] = icw_pcf[t];  // Save current pcf and
            icw_pcf[t] = icw_pcf_new;      // set new current pcf
         }

         switch (icw_pcf[t]) {
            case 0x0:                  // NO-OP
               if (icw_pcf_prev[t] != icw_pcf[t]) {
		 //if (debug_reg & 0x40)    // Trace PCF state ?
                 //    fprintf(stderr, "\n>>> CS2[%1X]: PCF = 0 entered, next PCF will be set by NCP \n\r", icw_pcf[t]);
               }
         //      icw_lne_stat[t] = RESET;  // Line state = RESET
               icw_scf[t] &= 0x4B;     // Reset all check cond. bits.
               break;

            case 0x1:                  // Set mode
               if (icw_pcf_prev[t] != icw_pcf[t]) {  // First entry ?
		 //if (debug_reg & 0x40)  // Trace PCF state ?
                 //    fprintf(stderr, "\n>>> CS2[%1X]: PCF = 1 entered, next PCF will be 0 \n\r", icw_pcf[t]);
                  icw_scf[t] |= 0x40;  // Set norm char serv flag
                  icw_pcf_new = 0x0;   // Goto PCF = 0...
                  CS2_req_L2_int = ON; // ...and issue a L2 int
               }
               break;

            case 0x2:                  // Mon DSR on
               if (icw_pcf_prev[t] != icw_pcf[t]) {  // First entry ?
                 // if (debug_reg & 0x40)   // Trace PCF state ?
                 //    fprintf(stderr, "\n>>> CS2[%1X]: PCF = 2 entered, next PCF will be set by NCP \n\r", icw_pcf[t]);
                  icw_scf[t] |= 0x40;  // Set norm char serv flag
                  icw_pcf_new = 0x0;   // Goto PCF = 4... (Via PCF = 0)
                  CS2_req_L2_int = ON; // ...and issue a L2 int
               }
               break;

            case 0x3:                  // Mon RI or DSR on
               if (icw_pcf_prev[t] != icw_pcf[t]) {  // First entry ?
                 // if (debug_reg & 0x40)   // Trace PCF state ?
                 //    fprintf(stderr, "\n>>> CS2[%1X]: PCF = 3 entered, next PCF will be 0 \n\r", icw_pcf[t]);
                  icw_scf[t] |= 0x40;  // Set norm char serv flag
                  icw_pcf_new = 0x0;   // Goto PCF = 0...
                  CS2_req_L2_int = ON; // ...and issue a L2 int
               }
               break;

            case 0x4:                  // Mon 7E flag - block DSR error
            case 0x5:                  // Mon 7E flag - allow DSR error
               icw_scf[t] &= 0xFB;     // Reset 7E detected flag
               if (icw_pcf_prev[t] != icw_pcf[t]) {  // First entry ?
		 //if (debug_reg & 0x40) {  // Trace PCF state ?
		    //   fprintf(stderr, "\n>>> CS2[%1X]: PCF = 5 entered, next PCF will be 6 \n\r", icw_pcf[t]);
		 //   fprintf(stderr, "\n<<< CS2[%1X]: Receiving PDF = *** %02X ***, j = %d \n\r", icw_pcf[t], icw_pdf[t], j-1);
		 //  }
               }
               j = 0;                          // Reset buffer pointer
	       /*               if (icw_lne_stat[t] == RESET)   // Line is silent. Wait for NCP time out.
                  break;
               if (icw_lne_stat[t] == TX)      // Line is silent. Wait for NCP action.
	       break;*/
	       do {
		 normal_frame = readSDLC(fd, BLU_buf, &size);
		 if (normal_frame && size > 0) {
		   //printFrame("Before rewriteSDLC : ", BLU_buf, size);
		   rewriteSDLC (1, BLU_buf, &size);
		   //printFrame("After rewriteSDLC : ", BLU_buf, size);
		 }
	       } while (size>0 && !normal_frame);
	       //printFrame("Receive : ", BLU_buf, size);
	     // write pcap_buf to file
	       if (size>=8) {
	         gettimeofday(&tv,NULL);
	         pcap_rec.ts_sec = tv.tv_sec;
	         pcap_rec.ts_usec = tv.tv_usec;
	         pcap_rec.incl_len = size - 4;
	         pcap_rec.orig_len = size -4;
	         //fwrite (&pcap_rec, sizeof pcap_rec, 1, pcap_file);
	         //fwrite (BLU_buf, size -4, 1, pcap_file);
	         //fflush(pcap_file);
	       }
	       size -=2; // skip the CRC calc and EOR
               // Line state is receiving, wait for BFlag...
               if (size  > 0) {       // we got a normal frame (not abort) ?
                  icw_scf[t]  |= 0x04;         // Set flag detected. (NO Serv bit)
                  icw_lcd[t]   = 0x9;          // LCD = 9 (SDLC 8-bit)
                  icw_pcf_new  = 0x6;          // Goto PCF = 6...
                  CS2_req_L2_int = ON;         // ...and issue a L2 int
               }
               break;

            case 0x6:                  // Receive info-inhibit data interrupt
               if ((svc_req_L2 == ON) || (lvl == 2))  // If L2 interrupt active ?
                  break;                              // Loop till inactive...
               icw_pdf[t] = BLU_buf[j++];
               //if (debug_reg & 0x40) { // Trace PCF state ?
               //   fprintf(stderr, "\n>>> CS2[%1X]: PCF = 6 entered, next PCF will be 7 \n\r", icw_pcf[t]);
               //   fprintf(stderr, "\n<<< CS2[%1X]: Receiving PDF = *** %02X ***, j = %d \n\r", icw_pcf[t], icw_pdf[t], j-1);
               //}
               icw_scf[t] |= 0x40;     // Set norm char serv flag
               icw_scf[t] &= 0xFB;     // Reset 7E detected flag
               icw_pdf_reg = FILLED;
               icw_pcf_new = 0x7;      // Goto PCF = 7...
               CS2_req_L2_int = ON;    // ...and issue a L2 int
               break;

            case 0x7:                  // Receive info-allow data interrupt
               if ((svc_req_L2 == ON) || (lvl == 2))  // If L2 interrupt active ?
                  break;                              // Loop till inactive...

               if (icw_pdf_reg == EMPTY) {   // NCP has read pdf ?
                  // Check for Eflag (for transparency x'470F7E' CRC + EFlag)
		 if (size == j)  {
		   Eflg_rvcd = ON;
		   icw_pdf[t] = 0x7e; // Get received byte
		 } else {
		   Eflg_rvcd = OFF;   // No Eflag
		   icw_pdf[t] = BLU_buf[j++]; // Get received byte
		   if (icw_pdf[t] == 0xff) icw_pdf[t] = BLU_buf[j++];
		 }

                  
                 // if (debug_reg & 0x40) {    // Trace PCF state ?
                 //    fprintf(stderr, "\n<<< CS2[%1X]: PCF = 7 (re-)entered \n\r", icw_pcf[t]);
                 //    fprintf(stderr, "\n<<< CS2[%1X]: Receiving PDF = *** %02X ***, j = %d \n\r", icw_pcf[t], icw_pdf[t], j-1);
                 // }
                  if (Eflg_rvcd == ON) {     // EFlag received ?
                     icw_lne_stat[t] = TX;   // Line turnaround to transmitting...
                     icw_scf[t] |= 0x44;     // Set char serv and flag det bit
                     icw_pcf_new = 0x6;      // Go back to PCF = 6...
                     CS2_req_L2_int = ON;    // Issue a L2 interrupt
                  } else {
                     icw_pdf_reg = FILLED;   // Signal NCP to read pdf.
                     icw_scf[t] |= 0x40;     // Set norm char serv flag
                     icw_pcf_new = 0x7;      // Stay in PCF = 7...
                     CS2_req_L2_int = ON;    // Issue a L2 interrupt
                  }
               }
               break;

            case 0x8:                  // Transmit initial-turn RTS on
	      //if (debug_reg & 0x40)   // Trace PCF state ?
              //    fprintf(stderr, "\n>>> CS2[%1X]: PCF = 8 entered, next PCF will be 9 \n\r", icw_pcf[t]);
               icw_scf[t] &= 0xFB;     // Reset flag detected flag
               // CTS is now on.
               icw_pcf_new = 0x9;      // Goto PCF = 9
               j = 0;                  // Reset Tx buffer pointer.
	       lastj = 0;
	       write_buffer_ptr[0] = BLU_buf;
               // NO CS2_req_L2_int !
               break;

            case 0x9:                  // Transmit normal
               if ((svc_req_L2 == ON) || (lvl == 2))  // If L2 interrupt active ?
                  break;
               if (icw_pdf_reg == FILLED) {   // New char avail to xmit ?
		 //  if (debug_reg & 0x40) { // Trace PCF state ?
                 //    fprintf(stderr, "\n>>> CS2[%1X]: PCF = 9 (re-)entered \n\r", icw_pcf[t]);
                 //    fprintf(stderr, "\n>>> CS2[%1X]: Transmitting PDF = *** %02X ***, j = %d \n\r", icw_pcf[t], icw_pdf[t], j);
                 // }

		  if (icw_scf[t]&0x01) {   // send flag byte
		    // fprintf (stderr, "end_flag = %d.\n", end_flag);
		    if (end_flag) {
		      end_flag = 0;
		      // fprintf (stderr, "Flag bit set - End of frame. Send a EOR record. write_buffer_index=%d\n", write_buffer_index);
		      BLU_buf[j++] = 0xff;		    
		      BLU_buf[j++] = 0xef;
		      write_buffer_size[write_buffer_index++] = j-lastj;
		      lastj = j;
		      write_buffer_ptr[write_buffer_index] = BLU_buf + j;
		    } else {
		      // start of frame
		      // fprintf(stderr, "Flag bit set - Start of frame.\n");
		      end_flag = 1;
		    }
		  } else {
		    BLU_buf[j++] = icw_pdf[t];
		    if (icw_pdf[t] == 0xff) BLU_buf[j++] = icw_pdf[t];
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
		 //if (debug_reg & 0x40)   // Stderr PCF state ?
		    //fprintf(stderr, "\n>>> CS2[%1X]: PCF = C entered, next PCF will be set by NCP \n\r", icw_pcf[t]);
		 //fprintf (stderr, "BLU_buf=%p\n", BLU_buf);
		 //fprintf (stderr, "write_buffer_index = %d \n", write_buffer_index);
		 //fprintf (stderr, "end_flag = %d - now clearing it!.\n", end_flag);
		  for (int i=0; i<write_buffer_index; i++) {
		    //fprintf (stderr, "i=%d\n", i);
		    //fprintf(stderr, "write_buffer_ptr = %p write_buffer_size = %d\n", write_buffer_ptr[i], write_buffer_size[i]);
		    //printFrame("Send : ", write_buffer_ptr[i], write_buffer_size[i]);
		    rewriteSDLC(0, write_buffer_ptr[i], &write_buffer_size[i]);
		    //printFrame("Send : ", write_buffer_ptr[i], write_buffer_size[i]);		    
		    if (write_buffer_size[i] > 3) {
		      //printFrame("Send : ", write_buffer_ptr[i], write_buffer_size[i]);
		      gettimeofday(&tv,NULL);
		      pcap_rec.ts_sec = tv.tv_sec;
		      pcap_rec.ts_usec = tv.tv_usec;
		      pcap_rec.incl_len = write_buffer_size[i]-4;
		      pcap_rec.orig_len = write_buffer_size[i]-4;
		      //fwrite (&pcap_rec, sizeof pcap_rec, 1, pcap_file);
		      //fwrite (write_buffer_ptr[i], write_buffer_size[i]-4, 1, pcap_file);
		      //fflush(pcap_file);		   
 		      write (fd, write_buffer_ptr[i], write_buffer_size[i]);
		      usleep(BITTIME*write_buffer_size[i]);
		    } else {
		      //fprintf(stderr, "Packet dropped after rewriteSDLC.\n");
		    }

		  }
		  

                  //if (debug_reg & 0x20) {
		  //  fprintf(stderr, "\n>>> CS2[%1X]: BLU_buf = ", icw_pcf[t]);
		  // for (s = (char *) &BLU_buf[0], i = 0; i < 16; ++i, ++s)
                  //      fprintf(stderr, "%02X ", (int) *s & 0xFF);
                  //   fprintf(stderr, "\n");
                  //}
                  icw_lne_stat[t] = RX;   // Line turnaround to receiving...
                  j = 0;                  // Reset Rx buffer pointer.
                  icw_scf[t] |= 0x40;     // Set norm char serv flag
                  icw_pcf_new = 0x5;      // Goto PCF = 5...
                  CS2_req_L2_int = ON;    // ...and issue a L2 int
		  write_buffer_index = 0; // Reset the write buffers
               }
               break;

            case 0xD:                     // Transmit turnaround-keep RTS on
               if (icw_pcf_prev[t] != icw_pcf[t]) {  // First entry ?
		 //if (debug_reg & 0x40)   // Trace PCF state ?
                 //    fprintf(stderr, "\n>>> CS2[%1X]: PCF = D entered, next PCF will be set by NCP \n\r", icw_pcf[t]);
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
	   //if (debug_reg & 0x40)      // Trace PCF state ?
           //    fprintf(stderr, "\n>>> CS2[%1X]: SVCL2 interrupt issued for PCF = %1X \n\r",
	   //             icw_pcf_prev[t], icw_pcf_prev[t]);
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
         if (debug_reg & 0x40 & (icw_pcf[t] != 0))         // Trace Prev / Curr PCF state ?
	   fprintf(stderr, "\n>>> CS2[%1X]: Next PCF = %1X \n\r",
                    icw_pcf_prev[t], icw_pcf[t]);
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

