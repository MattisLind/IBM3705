/* 3705_cpu.c: IBM 3705 CCU simulator (18bit EA)

  Copyright (c) 2021, Henk Stegeman and Edwin Freekenhorst

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

  cpu          IBM 3705 central processor (with 20bit Extended Addressin)

  The IBM 3705 Communications Controller is a transmission control unit
  designed to assume many of the line-control and processing functions
  for the teleprocessing subsystem. The IBM 3705 performs all the usual
  functions of a transmission control unit and, in addition, takes over
  many of the capabilities of an access method. In this way, the 3705
  removes much of the control of the teleprocessing subsystem from the CPU.

  The 3705 communications controller consists cf 4 major components:
  - The central control unit (CCU) contains most of the arithmetic and logic
    circuitry necessary for the operation of the 3705.
  - The core memory serves its usual function of providing a storage area for
    both machine instructions and data.
  - The channel adapter (CA) controls the interface between the 3705 and the
    host computer.
  - The communication scanner (CS) serves as the interface between the 3705
    and the communications network.

  Programs in the 3705 can execute at any of 5 priority levels which
  are controlled by hardware.  Levels 1 through 4 are interrupt driven;
  that is, they are entered only on the occurrence of specific hardware
  interrupt conditions.
  Level 1, the highest priority level, is used mainly for handling error
           conditions. It is entered when either a hardware failure or
           programming error occurs.
  Level 2, deals with the communication network, and is entered whenever a
           communication line must be serviced by the software.
  Level 3, is used to handle processing of a less critical nature,
           including communication with the host computer, timer maintenance,
           and operator intevention.
  Level 4, is the lowest level of the supervisor, and is entered only upon
           request of one of the other program levels.
  Level 5, the lowest priority level, is unique in several respects. It is
           not interrupt driven, and is executed only when there are no
           outstanding requests for any of-the other program levels.
           It is intended for non-critical background processing, and
           hence is not allowed to execute the privileged instructions
           available to the other 4 levels.

  The 3705 core storage is organized in bytes of 8 bits each; these may
  be grouped into halfwords (2 bytes) and fullwords (4 bytes). Storage
  addressing is by byte; the 1st byte of memory is designated Byte 0,
  and successive bytes are numbered sequentially.

  At each program level, the first general register (register 0) of the
  associated group serves as the instruction address register (IAR).
  It always contains the address of the next machine instruction to be
  executed. It is incremented sequentially as processing proceeds, unless
  it is modified by the executing program.
  When an interrupt occurs, the IAR of the appropriate level is loaded
  with the starting address for that level.

  Each program level has a pair of condition latches known as the C and
  Z latches. These are used to record the results of certain arithmetic,
  comparative, and logical operations.

  The 3705 recognizes 51 machine instructions.  All arithmetic and
  logical operations operate on registers only; the only instructions
  which directly reference storage are of the load and store variety.
  There are a number of branching instructions available, although
  branching may be accomplished by any instruction which modifies regis-
  ter 0 (the-IAR).

       ********      Instruction Set       ********
       Mnem   Description                    Format
       B      Branch                           RT
       BCL    Branch on C Latch                RT
       BZL    Branch on Z Latch                RT
       BB     Branch on Bit                    RT
       BCT    Branch on Count                  RT
       BAL    Branch and Link                  RA
       BALR   Branch and Link Register         RR
       AR     Add Register                     RR
       AHR    Add Halfword Register            RR
       ACR    Add Character Register           RR
       ARI    Add Register Immediate           RI
       SR     Subtract Register                RR
       SHR    Subtract Halfword Register       RR
       SCR    Subtract Character Register      RR
       SRI    Subtract Register Immediate      RI
       IC     Insert Character                 RS
       ICT    Insert Character and Count       RSA
       L      Load                             RS
       LH     Load Halfword                    RS
       LR     Load Register                    RR
       LHR    Load Halfword Register           RR
       LCR    Load Character Register          RR
       LRI    Load Register Immediate          RI
       LA     Load Address                     RA
       LOR    Load with Offset Register        RS
       LHOR   Load Halfword with Offset Reg.   RR
       LCOR   Load Char with Offset Reg.       RR
       ST     Store                            RS
       STH    Store Halfword                   RS
       STC    Store Character                  RS
       STCT   Store Character and Count        RSA
       CR     Compare Register                 RR
       CHR    Compare Halfword Register        RR
       CCR    Compare Character Register       RR
       CRI    Compare Register Immediate       RI
       NR     AND Register                     RR
       NHR    AND Halfword Register            RR
       NCR    AND Character Register           RR
       NRI    AND Register Immediate           RI
       OR     OR Register                      RR
       OHR    OR Halfword Register             RR
       OCR    OR Character Register            RR
       ORI    OR Register Immediate            RI
       XR     Exclusive OR Register            RR
       XHR    Exclusive OR Halfword Reg.       RR
       XRI    Exclusive OR Reg. Immediate      RI
       XCR    Exclusive OR Character Reg.      RR
       TRM    Test Register Under Mask         RI
       IN     Input                            RE
       OUT    Output                           RE
       EXIT   Exit                             EXIT

Notes:
Instruction formats:
       RA     Reg Addr
       RE     Reg External I/O reg
       RI     Reg Immediate
       RR     Reg Register
       RS     Reg Storage
       RSA    Reg Storage Address
       RT     Branch
       EXIT   Exit
*/

#include <sched.h>
#include "i3705_defs.h"
#include "i3705_Eregs.h"                                /* Exernal regs defs */
#include <pthread.h>

#define UNIT_V_MSIZE (UNIT_V_UF+3)                      /* dummy mask */
#define UNIT_MSIZE   (1 << UNIT_V_MSIZE)

extern void Get_ICW(int abar);                          /* CS2: ICW ===> Inp_Eregs 44, 45, 46, 47 rtn */
extern int abar;                                        /* CS2: scanner interface addr 0x0840 */
extern int8 icw_scf[];                                  /* CS2: sdf */
extern int8 icw_pdf[];                                  /* CS2: pdf */
extern int8 icw_lcd[];                                  /* CS2: lcd */
extern int8 icw_pcf[];                                  /* CS2: pcf */
extern int8 icw_sdf[];                                  /* CS2: sdf */
extern int8 icw_Rflags[];                               /* CS2: Rflags */
extern int8 icw_pdf_reg;                                /* CS2: pdf is filled or empty state */
extern int8 icw_pcf_new;                                /* CS2: new pdf */
extern int8 icw_pcf_mod;                                /* CS2: modified pdf flag */
extern pthread_mutex_t icw_lock;                        /* CS2: ICW update lock */
pthread_mutex_t r77_lock;                               /* CA2/CS2: Reg77 update lock */
pthread_mutex_t r7f_lock;                               /* CCU: Reg7F update lock */

uint8 M[MAXMEMSIZE] = { 0 };                            /* Memory 3705 */
int32 GR[8][4] = { 0x00 };                              /* General Registers Group 0-3 */
int32 opcode;                                           /* Operation Code 16 bits */
int32 opcode0, opcode1;                                 /* OpCode byte0(H) & Byte1(L) */
int8  CL_C[4] = { OFF };                                /* Condition Latches 'C' */
int8  CL_Z[4] = { OFF };                                /* Condition Latches 'Z' */
int32 Eregs_Inp[128] = { 0xEFEF };                      /* External regs X'00 -> X'7F' inp */
int32 Eregs_Out[128] = { 0x0000 };                      /* External regs X'00 -> X'7F' out */

int8  int_lvl_req[1+5]  = {0, OFF, OFF, OFF, OFF, OFF}; /* Requested Program Levels */
int8  int_lvl_ent[1+5]  = {0, OFF, OFF, OFF, OFF, OFF}; /* Entered Program Levels */
int8  int_lvl_mask[1+5] = {0, ON,  ON,  ON,  ON,  ON }; /* Masked Program Levels */
int8  ipl_req_L1 = OFF;                                 /* IPL L1 request flag */
int8  diag_req_L2 = OFF;                                /* Diagnostic L2 (in test mode only) */
int8  svc_req_L2 = OFF;                                 /* SVC L2 request flag */
int8  pci_req_L3 = OFF;                                 /* PCI L3 request flag */
int8  timer_req_L3 = OFF;                               /* Interval timer L3 request flag */
int8  inter_req_L3 = OFF;                               /* Panel interrupt L3 request flag */
int8  pci_req_L4 = OFF;                                 /* PCI L4 request flag */
int8  svc_req_L4 = OFF;                                 /* SVC L4 request flag */
// These flags below belong in chan.c
int8  CA1_DS_req_L3 = OFF;                              /* Chan Adap Data/Status request flag */
int8  CA1_IS_req_L3 = OFF;                              /* Chan Adap Initial Sel request flag */
int8  CA1_NSC_end_seq = OFF;                            /* NSC channel end xfer seq flag */
int8  CA1_NSC_final_seq = OFF;                          /* NSC channel final xfer seq flag */
int8  CA1_NSC_SB_clred = OFF;                           /* NSC status byte cleared flag */

int8 last_P_Ns;                                         /* Last frame Ns received from prim station */
int8 last_S_Ns;                                         /* Last frame Ns send to prim station */
int8 last_lu;

int8  load_state = OFF;                                 /* Load state flag (IPL loadTest mode flag */
int8  test_mode  = OFF;                                 /* Test mode flag */
int8  wait_state = OFF;                                 /* Wait state flag */
int8  pgm_stop   = OFF;                                 /* Program STOP flag */
int8  OP_reg_chk = OFF;                                 /* Invalid instruction */
int8  adr_ex_chk = OFF;                                 /* Address exception check */
int8  IO_L5_chk  = OFF;                                 /* I/O instr in L5 */
int32 lvl;                                              /* Active Program Level (1...5) */
int32 Grp;                                              /* Active Register Group (0...3) */
int32 PC;                                               /* Program Counter */
int32 LAR;                                              /* Lagging Address Register */
int32 saved_PC;                                         /* Previous (saved) PC */
int32 debug_reg = 0x40;                                 /* Bit flags for debug/trace */
int32 debug_flag = OFF;                                 /* 1 when trace.log open */
FILE  *trace;
int   tbar;                                             /* ICW table pointer */
int32 cc = 1;
int32 val[4] = { 0x00, 0x00, 0x00, 0x00 };              /* Used for printing mnem */

t_stat cpu_ex (t_value *vptr, t_addr addr, UNIT *uptr, int32 sw);
t_stat cpu_dep (t_value val, t_addr addr, UNIT *uptr, int32 sw);
t_stat cpu_reset (DEVICE *dptr);
t_stat cpu_set_size (UNIT *uptr, int32 val, char *cptr, void *desc);
t_stat cpu_boot (int32 unitno, DEVICE *dptr);

int32 RegGrp(int32 level);
int32 GetMem(int32 addr);
int32 PutMem(int32 addr, int32 data);

int16 reason;

/* CPU data structures
   cpu_unit     CPU unit descriptor
   cpu_reg      CPU register list
   cpu_dev      CPU device descriptor
   cpu_mod      CPU modifiers list
*/

UNIT cpu_unit = { UDATA (NULL, UNIT_FIX + UNIT_BINK, MAXMEMSIZE) };

REG cpu_reg[] = {
    { HRDATA (IAR, PC, 20), REG_RO },
    { HRDATA (LAR, LAR,20), REG_RO },
    { HRDATA (LVL, lvl, 8), REG_RO },
    { HRDATA (GRP, Grp, 8), REG_RO },

    /* Interrupt requests flags */
    { FLDATA (IREQ1, int_lvl_req[0], 8) },  // No idea why
    { FLDATA (IREQ2, int_lvl_req[1], 8) },  // 1 => 0
    { FLDATA (IREQ3, int_lvl_req[2], 8) },
    { FLDATA (IREQ4, int_lvl_req[3], 8) },
    { FLDATA (IREQ5, int_lvl_req[4], 8) },

    /* Group 0 registers */
    { HRDATA (GR0G0, GR[0][0], 18) },
    { HRDATA (GR1G0, GR[1][0], 18) },
    { HRDATA (GR2G0, GR[2][0], 18) },
    { HRDATA (GR3G0, GR[3][0], 18) },
    { HRDATA (GR4G0, GR[4][0], 18) },
    { HRDATA (GR5G0, GR[5][0], 18) },
    { HRDATA (GR6G0, GR[6][0], 18) },
    { HRDATA (GR7G0, GR[7][0], 18) },
    { FLDATA (CLCG0, CL_C[0],   8) },
    { FLDATA (CLZG0, CL_Z[0],   8) },

    /* Group 1 registers */
    { HRDATA (GR0G1, GR[0][1], 18) },
    { HRDATA (GR1G1, GR[1][1], 18) },
    { HRDATA (GR2G1, GR[2][1], 18) },
    { HRDATA (GR3G1, GR[3][1], 18) },
    { HRDATA (GR4G1, GR[4][1], 18) },
    { HRDATA (GR5G1, GR[5][1], 18) },
    { HRDATA (GR6G1, GR[6][1], 18) },
    { HRDATA (GR7G1, GR[7][1], 18) },
    { FLDATA (CLCG1, CL_C[1],   8) },
    { FLDATA (CLZG1, CL_Z[1],   8) },

    /* Group 2 registers */
    { HRDATA (GR0G2, GR[0][2], 18) },
    { HRDATA (GR1G2, GR[1][2], 18) },
    { HRDATA (GR2G2, GR[2][2], 18) },
    { HRDATA (GR3G2, GR[3][2], 18) },
    { HRDATA (GR4G2, GR[4][2], 18) },
    { HRDATA (GR5G2, GR[5][2], 18) },
    { HRDATA (GR6G2, GR[6][2], 18) },
    { HRDATA (GR7G2, GR[7][2], 18) },
    { FLDATA (CLCG2, CL_C[2],   8) },
    { FLDATA (CLZG2, CL_Z[2],   8) },

    /* Group 3 registers */
    { HRDATA (GR0G3, GR[0][3], 18) },
    { HRDATA (GR1G3, GR[1][3], 18) },
    { HRDATA (GR2G3, GR[2][3], 18) },
    { HRDATA (GR3G3, GR[3][3], 18) },
    { HRDATA (GR4G3, GR[4][3], 18) },
    { HRDATA (GR5G3, GR[5][3], 18) },
    { HRDATA (GR6G3, GR[6][3], 18) },
    { HRDATA (GR7G3, GR[7][3], 18) },
    { FLDATA (CLCG3, CL_C[3],   8) },
    { FLDATA (CLZG3, CL_Z[3],   8) },

    /* External registers CS2 */
    { HRDATA (CS40I, Eregs_Inp[0x40], 16) },
    { HRDATA (CS41I, Eregs_Inp[0x41], 16) },
    { HRDATA (CS42I, Eregs_Inp[0x42], 16) },
    { HRDATA (CS43I, Eregs_Inp[0x43], 16) },
    { HRDATA (CS44I, Eregs_Inp[0x44], 16) },
    { HRDATA (CS45I, Eregs_Inp[0x45], 16) },
    { HRDATA (CS46I, Eregs_Inp[0x46], 16) },
    { HRDATA (CS47I, Eregs_Inp[0x47], 16) },

    /* External registers CA1 */
    { HRDATA (CA60I, Eregs_Inp[0x60], 16) },
    { HRDATA (CA61I, Eregs_Inp[0x61], 16) },
    { HRDATA (CA62I, Eregs_Inp[0x62], 16) },
    { HRDATA (CA63I, Eregs_Inp[0x63], 16) },
    { HRDATA (CA64I, Eregs_Inp[0x64], 16) },
    { HRDATA (CA65I, Eregs_Inp[0x65], 16) },
    { HRDATA (CA66I, Eregs_Inp[0x66], 16) },
    { HRDATA (CA67I, Eregs_Inp[0x67], 16) },

    /* External registers CCU */
    { HRDATA (CU70I, Eregs_Inp[0x70], 16) },
    { HRDATA (CU71I, Eregs_Inp[0x71], 16) },
    { HRDATA (CU72I, Eregs_Inp[0x72], 16) },
    { HRDATA (CU73I, Eregs_Inp[0x73], 16) },
    { HRDATA (CU74I, Eregs_Inp[0x74], 16) },
    { HRDATA (CU75I, Eregs_Inp[0x75], 16) },
    { HRDATA (CU76I, Eregs_Inp[0x76], 16) },
    { HRDATA (CU77I, Eregs_Inp[0x77], 16) },

    { HRDATA (CU78I, Eregs_Inp[0x78], 16) },
    { HRDATA (CU79I, Eregs_Inp[0x79], 16) },
    { HRDATA (CU7AI, Eregs_Inp[0x7A], 16) },
    { HRDATA (CU7BI, Eregs_Inp[0x7B], 16) },
    { HRDATA (CU7CI, Eregs_Inp[0x7C], 16) },
    { HRDATA (CU7DI, Eregs_Inp[0x7D], 16) },
    { HRDATA (CU7EI, Eregs_Inp[0x7E], 16) },
    { HRDATA (CU7FI, Eregs_Inp[0x7F], 16) },

    /* External registers CS2 */
    { HRDATA (CS40O, Eregs_Out[0x40], 16) },
    { HRDATA (CS41O, Eregs_Out[0x41], 16) },
    { HRDATA (CS42O, Eregs_Out[0x42], 16) },
    { HRDATA (CS43O, Eregs_Out[0x43], 16) },
    { HRDATA (CS44O, Eregs_Out[0x44], 16) },
    { HRDATA (CS45O, Eregs_Out[0x45], 16) },
    { HRDATA (CS46O, Eregs_Out[0x46], 16) },
    { HRDATA (CS47O, Eregs_Out[0x47], 16) },

    /* External registers CA1 */
    { HRDATA (CA60O, Eregs_Out[0x60], 16) },
    { HRDATA (CA61O, Eregs_Out[0x61], 16) },
    { HRDATA (CA62O, Eregs_Out[0x62], 16) },
    { HRDATA (CA63O, Eregs_Out[0x63], 16) },
    { HRDATA (CA64O, Eregs_Out[0x64], 16) },
    { HRDATA (CA65O, Eregs_Out[0x65], 16) },
    { HRDATA (CA66O, Eregs_Out[0x66], 16) },
    { HRDATA (CA67O, Eregs_Out[0x67], 16) },

    /* External registers CCU */
    { HRDATA (CU70O, Eregs_Out[0x70], 16) },
    { HRDATA (CU71O, Eregs_Out[0x71], 16) },
    { HRDATA (CU72O, Eregs_Out[0x72], 16) },
    { HRDATA (CU73O, Eregs_Out[0x73], 16) },
    { HRDATA (CU74O, Eregs_Out[0x74], 16) },
    { HRDATA (CU75O, Eregs_Out[0x75], 16) },
    { HRDATA (CU76O, Eregs_Out[0x76], 16) },
    { HRDATA (CU77O, Eregs_Out[0x77], 16) },

    { HRDATA (CU78O, Eregs_Out[0x78], 16) },
    { HRDATA (CU79O, Eregs_Out[0x79], 16) },
    { HRDATA (CU7AO, Eregs_Out[0x7A], 16) },
    { HRDATA (CU7BO, Eregs_Out[0x7B], 16) },
    { HRDATA (CU7CO, Eregs_Out[0x7C], 16) },
    { HRDATA (CU7DO, Eregs_Out[0x7D], 16) },
    { HRDATA (CU7EO, Eregs_Out[0x7E], 16) },
    { HRDATA (CU7FO, Eregs_Out[0x7F], 16) },

    /* Misc */
    { HRDATA (WRU, sim_int_char, 8) },
    { HRDATA (DEBUG, debug_reg, 16) },
    { NULL }
};

MTAB cpu_mod[] = {    // <=== for future 3705 model selection
    { UNIT_MSIZE, 16384, NULL, "16K", &cpu_set_size },
    { UNIT_MSIZE, 32768, NULL, "32K", &cpu_set_size },
    { UNIT_MSIZE, 49152, NULL, "48K", &cpu_set_size },
    { UNIT_MSIZE, 65536, NULL, "64K", &cpu_set_size },
    { UNIT_MSIZE, 131072, NULL, "128", &cpu_set_size },
    { UNIT_MSIZE, 262144, NULL, "256K", &cpu_set_size },
    { 0 }
};

DEVICE cpu_dev = {
    "CPU", &cpu_unit, cpu_reg, cpu_mod,
    1, 16, 16, 1, 16, 8,
    &cpu_ex, &cpu_dep, &cpu_reset, &cpu_boot,
    NULL, NULL
};

//********************************************************
// Instruction simulator
//********************************************************

t_stat sim_instr (void) {

// Select which core's may be used
// We can set one or more bits here, each one representing a single CPU
   cpu_set_t cpuset;

   // Select the CPU core we want to use
   int cpu = 0;

   CPU_ZERO(&cpuset);       // Clears the cpuset
   CPU_SET( cpu , &cpuset); // Set CPU on cpuset

   /*
    * cpu affinity for the calling thread
    * first parameter is the pid, 0 = calling thread
    * second parameter is the size of your cpuset
    * third param is the cpuset in which your thread
    * will be placed. Each bit represents a CPU.
    */
   sched_setaffinity(0, sizeof(cpuset), &cpuset);


int32 i, j, w_byte, addr;
int32 R1fld, R2fld, Rfld;
int32 N1fld, N2fld, Nfld;
int32 Afld, Bfld, Dfld, Efld, Ifld, Mfld, Tfld;

Grp = RegGrp(lvl);
saved_PC = PC;
PC = GR[0][Grp];
reason = 0;

//********************************************************
// Main instruction fetch/decode loop
//********************************************************

//##################### START OF SIMULATOR WHILE LOOP ######################
// Scroll down 1800 lines to find the end of this while loop
while (reason == 0) {                          /* Loop until halted */
   if (sim_interval <= 0) {                    /* Check clock queue */
      if (reason = sim_process_event())
          break;                               /* Stop simulation */
   }
   sim_interval = sim_interval - 1;            /* Tick the clock */

   if (sim_brk_summ && sim_brk_test(PC, SWMASK('E')) ) {  /* Any breakpoint ? */
      reason = STOP_IBKPT;                     /* Stop simulation */
      break;
   }

//********************************************************
// Uncomment this code to start CCU execution trace
// after NCP load completion.
//********************************************************
//   Update ----- vvvv
//   if (ipl_req_L1 == OFF) debug_reg = 0x60;
//   if (svc_req_L2 || lvl == 2) {
//      debug_reg = 0x43;
//   } else { debug_reg = 0x00; }

//   if ((debug_reg == 0x00) && (debug_flag == ON)) {  /* Close log file ? */
//      fclose(trace);
//      debug_flag = OFF;
//   }

//********************************************************
//  Debug trace facility
//********************************************************
   if (debug_flag == OFF) {
      trace = fopen("trace.log", "w");
      fprintf(trace, "     ****** 3705 Executed instructions log file ****** \n\n"
                     "     sim> d debug 01 - trace IAR, mnem, C & Z & lvl \n"
                     "                  02 - trace all enter/leave/wait interrupts \n"
                     "                  04 - trace scanner ext input regs \n"
                     "                  08 - trace channel adap ext input regs \n"
                     "                  10 - trace CCU ext input regs \n"
                     "                  20 - trace PIU's \n"
                     "                  40 - trace ICW PCF \n"
                     "                  80 - trace channel activity \n"
                     "          AA55 = Unused external register \n\n");
      debug_flag = ON;
   }

//********************************************************
//  IBM 3705 CCU trace print statements
//********************************************************
   if (wait_state != ON) {
     if (debug_reg & 0x01) {  /* Trace instruction + mnem. */
         fprintf(trace, "\n[%06d] exec IAR=%05X - %04X        ", cc++,
            saved_PC, opcode );
         fprint_sym(trace, PC, (uint32 *) val, &cpu_unit, SWMASK('M') );
         fprintf(trace, "\n");
	 }
      if (debug_reg & 0x08) {  /* Trace external scanner registers */
         fprintf(trace, "         CS2: %05X %05X %05X %05X  %05X %05X %05X %05X (X'40-47') ",
            Eregs_Inp[CMBARIN], NOTUSED, NOTUSED, Eregs_Inp[CMERREG],
            Eregs_Inp[CMICWB0F], Eregs_Inp[CMICWLPS], Eregs_Inp[CMICWDPS], Eregs_Inp[CMICWB32]);
         fprintf(trace, "\n");
	 }
            if (debug_reg & 0x04) {  /* Trace external chan adaptor registers */
         fprintf(trace, "         CA1: %05X %05X %05X %05X  %05X %05X %05X %05X (X'60-67') ",
            Eregs_Inp[CAISC], Eregs_Inp[CAISD], Eregs_Inp[CASSC], Eregs_Inp[CASSA],
            Eregs_Inp[CASD12], Eregs_Inp[CASD34], Eregs_Inp[CARNSTAT], Eregs_Inp[CAECR]);
         fprintf(trace, "\n");
	 }
      if (debug_reg & 0x10) {  /* Trace CCU external registers */
         fprintf(trace, "         CCU: %05X %05X %05X %05X  %05X %05X %05X %05X (X'70-77') ",
            Eregs_Inp[SYSSTSZ], Eregs_Inp[SYSADRDT], Eregs_Inp[SYSFNINS], Eregs_Inp[SYSINKEY],
            Eregs_Inp[0x74], Eregs_Inp[0x75], Eregs_Inp[SYSADPG1], Eregs_Inp[SYSADPG2]);
         fprintf(trace, "\n");
         fprintf(trace, "         CCU: %05X %05X %05X %05X  %05X %05X %05X %05X (X'78-7F') ",
            NOTUSED, Eregs_Inp[SYSUTILI], Eregs_Inp[SYSCUCI],  Eregs_Inp[SYSBSCRC],
            NOTUSED, Eregs_Inp[SYSMCHK],  Eregs_Inp[SYSCCUG1], Eregs_Inp[SYSCCUG2]);
         fprintf(trace, "\n");
	 }
   }

//********************************************************
//  Check for any program level requests ?
//********************************************************

   /* Check for any L1 requests ? */
   if (ipl_req_L1 || OP_reg_chk || IO_L5_chk || adr_ex_chk)
      int_lvl_req[1] = ON;                     // Set L1 interrupt request
   else int_lvl_req[1] = OFF;

   /* Check for any L2 requests ? */
   if (diag_req_L2 || svc_req_L2)
      int_lvl_req[2] = ON;                     // Set L2 interrupt request
   else int_lvl_req[2] = OFF;

   /* Check for any L3 requests ? */
   if (inter_req_L3 || timer_req_L3 || pci_req_L3 || CA1_DS_req_L3 || CA1_IS_req_L3)
      int_lvl_req[3] = ON;                     // Set L3 interrupt request
   else int_lvl_req[3] = OFF;

   /* Check for any L4 requests ? */
   if (pci_req_L4 || svc_req_L4)
      int_lvl_req[4] = ON;                     // Set L4 interrupt request
   else int_lvl_req[4] = OFF;

      if (debug_reg & 0x02) {                     /* Trace interrupt flags */
      if (wait_state != ON) {
         fprintf(trace, "\n>>  REQ[1-5] = %d %d %d %d %d   ENT[1-5] = %d %d %d %d %d   MSK[1-5] = %d %d %d %d %d\n" ,
               int_lvl_req[1],  int_lvl_req[2],  int_lvl_req[3],  int_lvl_req[4],  int_lvl_req[5],
               int_lvl_ent[1],  int_lvl_ent[2],  int_lvl_ent[3],  int_lvl_ent[4],  int_lvl_ent[5],
               int_lvl_mask[1], int_lvl_mask[2], int_lvl_mask[3], int_lvl_mask[4], int_lvl_mask[5]);
   }  }

//********************************************************
// Check all 5 program levels for any work...
//********************************************************
   for (i = 1; i < 6; i++) {                   // 1, 2, 3, 4...5                          d
      if (int_lvl_ent[i] == OFF) {             // Lvl already running ? => continue
//         printf("ENTERED = OFF  lvl=%d \n\r", i );
         if ((int_lvl_req[i] == ON) || (i == 5)) {           // Lvl request pending ? => enter if not masked
//            printf("REQUESTED = ON lvl=%d \n\r", i );
            if (int_lvl_mask[i] == OFF) {      // Lvl mask on ? => skip this level
//               printf("MASK = OFF     lvl=%d\n\r", i );
               /* Start higher prio pgm level ! */
               int_lvl_ent[i] = ON;
               lvl = i;                        // Set new pgm level
               Grp = RegGrp(lvl);              // Set new reg group
               if (debug_reg & 0x02) {         // Trace CCU interrupt levels
                  if (lvl == 1)
                     fprintf(trace, "\n>>> Entering lvl=1 -- IPL=%d; OPchk=%d; IOchk=%d; AEchk=%d \n",
                             ipl_req_L1, OP_reg_chk, IO_L5_chk, adr_ex_chk);
                  if (lvl == 2)
                     fprintf(trace, "\n>>> Entering lvl=2 -- Diag=%d; SVCL2=%d \n",
                             diag_req_L2, svc_req_L2);
                  if (lvl == 3)
                     fprintf(trace, "\n>>> Entering lvl=3 -- Int=%d; Timer=%d; PCIL3=%d; CA1_IS=%d; CA1_D/S=%d \n",
                             inter_req_L3, timer_req_L3, pci_req_L3, CA1_IS_req_L3, CA1_DS_req_L3);
                  if (lvl == 4)
                     fprintf(trace, "\n>>> Entering lvl=4 -- PCIL4=%d; SVCL4=%d \n",
                             pci_req_L4, svc_req_L4);
                  if (lvl == 5)
                     fprintf(trace, "\n>>> Entering lvl=5 -- MSKL5=0 \n");
               }
               if (debug_reg & 0x02) {
               if (lvl == 1)                   // Display CCU interrupt levels
                  printf(">>> Entering lvl 1 -- IPL=%d; OPchk=%d; IOchk=%d; AEchk=%d \n\r",
                          ipl_req_L1, OP_reg_chk, IO_L5_chk, adr_ex_chk);
               if (lvl == 2)
                  printf(">>> Entering lvl 2 -- Diag=%d; SVCL2=%d \n\r",
                          diag_req_L2, svc_req_L2);
               if (lvl == 3)
                  printf(">>> Entering lvl 3 -- Int=%d; Timer=%d; PCIL3=%d; CA1_IS=%d; CA1_D/S=%d \n\r",
                          inter_req_L3, timer_req_L3, pci_req_L3, CA1_IS_req_L3, CA1_DS_req_L3);
               if (lvl == 4)
                  printf(">>> Entering lvl 4 -- PCIL4=%d; SVCL4=%d \n\r",
                          pci_req_L4, svc_req_L4);
               if (lvl == 5)
                  printf(">>> Entering lvl 5 -- MSKL5=0 \n\r");
               }
               wait_state = OFF;               // Exiting wait state. Work to do...

               switch (lvl) {
                  case 1:
                     GR[0][0] = 0x0010;        // Start addr level 1
                     break;
                  case 2:
                     GR[0][Grp] = 0x0080;      // Start addr level 2
                     break;
                  case 3:
                     GR[0][Grp] = 0x0100;      // Start addr level 3
                     break;
                  case 4:
                     GR[0][Grp] = 0x0180;      // Start addr level 4
                     break;
                  case 5:                      // Continue with GR0G3
                     break;
                  default:
                     reason = SCPE_IERR;       // We got a problem !
                     break;
               }
               break;                          // Go for it...
            }
            /* If level 5 and mask is ON go into WAIT state */
            if (i == 5) {
               if (int_lvl_mask[5] == ON) {
                  /* Looks like we have nothing to do, so let's wait...  */
                  if ((debug_reg & 0x02) && (wait_state == OFF)) {
                     fprintf(trace, "\n>>> Entering wait state in lvl=5, GR0G3=%05X \n",
                                    GR[0][RegGrp(i)]);
                     fprintf(trace, "\n>>> Waiting... \n");
                  }
                  wait_state = ON;             // Enter wait state
               }
               lvl = i;                        // Set pgm level 5
               Grp = RegGrp(lvl);              // Set reg group 3
               break;                          // Out of inner 'for' loop
            }
            continue;                          // Check next lower pgm lvl
         }
         continue;                             // Check next lower pgm lvl
      }

      lvl = i;                                 // Set current pgm level
      Grp = RegGrp(lvl);                       // Set reg group
      break;                                   // Continue with current pgm lvl
   }

   if (wait_state == ON) {
      usleep(1000);                            // Get some rest...
      continue;
   }

//=======================================================================================
// Read instruction and execute it starts here...
//=======================================================================================

   if (lvl != 1) LAR = saved_PC;               /* Update LAR if lvl 2, 3, 4 or 5 */
   PC = GR[0][Grp];
   saved_PC = PC;

   val[0] = opcode0 = GetMem(PC);              /* Instruction byte 0(H) */
   PC = (PC + 1) & AMASK;
   val[1] = opcode1 = GetMem(PC);              /* Instruction byte 1(L) */
   PC = (PC + 1) & AMASK;
   opcode = (opcode0 << 8) | (opcode1);        /* Instr to be executed. */
   val[2] = GetMem(PC);                        /* Needed for possible LA */
   val[3] = GetMem(PC + 1);                    /* and BAL instructions. */

   if (((opcode0 & 0x88) == 0x00) &&           /* Invalid instruction ? */
       (test_mode == OFF) &&
      ((opcode1 == 0x00) ||
       (opcode1 == 0x20) ||
       (opcode1 == 0x50) ||
       (opcode1 == 0x60) ||
       (opcode1 == 0x70))) {
      OP_reg_chk = ON;
      if (lvl == 1)
         reason = STOP_INVOP;                  /* SIMH stop */
      continue;
   }
   GR[0][Grp] = PC;                            /* Update IAR before execution */

   switch (opcode & 0xF800) {
      case (0xA800):
         /* B    T              [RT]  */
         /* 01234567 89012345
            10101T<- ------>#         */
         Grp = RegGrp(lvl);
         Tfld = opcode & 0x07FE;

         if (opcode & 0x0001)                  /* Check displacement sign */
            GR[0][Grp] = GR[0][Grp] - Tfld;
         else
            GR[0][Grp] = GR[0][Grp] + Tfld;
         PC = GR[0][Grp];                      /* Update PC with new IAR */
         break;

      case (0x9800):
         /* BCL  T              [RT]  */
         /* 01234567 89012345
            10011T<- ------>#         */
         Grp = RegGrp(lvl);
         Tfld = (opcode & 0x07FE);

         if (CL_C[Grp] == ON) {
            if (opcode & 0x0001)
               GR[0][Grp] = GR[0][Grp] - Tfld;
            else
               GR[0][Grp] = GR[0][Grp] + Tfld;
            PC = GR[0][Grp];                   /* Update PC with new IAR */
         }
         break;

      case (0x8800):
         /* BZL  T              [RT]  */
         /* 01234567 89012345
            10001T<- ------>#         */
         Grp = RegGrp(lvl);
         Tfld = (opcode & 0x07FE);

         if (CL_Z[Grp] == ON) {
            if (opcode & 0x0001)
               GR[0][Grp] = GR[0][Grp] - Tfld;
            else
               GR[0][Grp] = GR[0][Grp] + Tfld;
            PC = GR[0][Grp];                   /* Update PC with new IAR */
         }
         break;

      case (0xB800):
         /* BCT  R(N),T         [RT]  */
         /* 01234567 89012345
            10111RRN 1T<-->T#         */
         if (opcode1 & 0x80) {                 /* Must be a 1, else it is BAL or LA instr */
            Grp = RegGrp(lvl);
            Rfld = (opcode0 & 0x06) + 1;       /* Extract odd register nr */
            Nfld = (opcode0 & 0x01);
            Tfld =  opcode1 & 0x7E;

            if (Nfld == 0) {                   /* Count is contained in byte 0 only */
               w_byte = (GR[Rfld][Grp] - 0x00100) & 0x0FF00;
               GR[Rfld][Grp] = (GR[Rfld][Grp] & 0xF00FF) | w_byte;
            } else {                           /* Count is contained in byte 0 & 1 */
               w_byte = (GR[Rfld][Grp] - 0x00001) & 0x0FFFF;
               GR[Rfld][Grp] = (GR[Rfld][Grp] & 0xF0000) | w_byte;
            }
            if ((w_byte & 0xFFFF) == 0x0000)   /* Next instr if result = 0 */
               break;
            if (opcode1 & 0x01)                /* Check displacement sign */
               GR[0][Grp] = GR[0][Grp] - Tfld;
            else
               GR[0][Grp] = GR[0][Grp] + Tfld;
            PC = GR[0][Grp];                   /* Update PC with new IAR */
         }
         break;

      case (0xC800):
      case (0xD800):
      case (0xE800):
      case (0xF800):
         /* BB   R(N),T         [RT]  */
         /* 01234567 89012345
            11MM1RRN MT<-->T#         */
         Mfld = ((opcode0 & 0x30) >> 3) + ((opcode1 & 0x80) >> 7);
         Rfld = (opcode0 & 0x06) + 1;          /* Extract odd register nr */
         Nfld = opcode0 & 0x01;
         Tfld = opcode1 & 0x7E;

         if (Nfld == 0)                        /* Test for byte 0 or 1 */
            Mfld = 0x8000 >> Mfld;             /* Shift test mask to byte 0(H) */
         else
            Mfld = 0x0080 >> Mfld;             /* Create bit test mask */

         if ((GR[Rfld][Grp] & Mfld) != 0x0000) {  /* Test with mask */
            /* Selected bit is ON, continue at branch addr. */
            if (opcode1 & 0x01)                /* Check displacement sign */
               GR[0][Grp] = GR[0][Grp] - Tfld;
            else
               GR[0][Grp] = GR[0][Grp] + Tfld;
            PC = GR[0][Grp];                   /* Update PC with new IAR */
         }
         break;

      case (0x8000):
         /* LRI  R(N),I         [RI]  */
         /* 01234567 89012345
            10000RRN I<---->I         */
         Grp = RegGrp(lvl);
         Rfld = (opcode0 & 0x06) + 1;          /* Extract odd register nr */
         Nfld = (opcode0 & 0x01);
         /* Reset C&Z latches */
         CL_Z[Grp] = OFF;
         CL_C[Grp] = OFF;

         if (Nfld == 0) {                      /* Byte 0(H) */
            GR[Rfld][Grp] = (GR[Rfld][Grp] & 0x300FF) | (opcode1 << 8);
         } else {                              /* Byte 1(L) */
            GR[Rfld][Grp] = (GR[Rfld][Grp] & 0x3FF00) | opcode1;
         }
         /* Test selected byte for zero */
         if (opcode1 == 0x00) {
            CL_Z[Grp] = ON;
         } else {
            CL_C[Grp] = ON;
         }
         break;

      case (0x9000):
         /* ARI  R(N),I         [RI]  */
         /* 01234567 89012345
            10010RRN I<---->I         */
         Grp = RegGrp(lvl);
         Rfld = (opcode0 & 0x06) + 1;          /* Extract odd register nr */
         Nfld = (opcode0 & 0x01);
         Ifld =  opcode1;
         /* Reset C&Z latches */
         CL_Z[Grp] = OFF;
         CL_C[Grp] = OFF;

         if (Nfld == 0) {                      /* Byte 0(H) */
            w_byte = GR[Rfld][Grp] + (Ifld << 8);
            if (((GR[Rfld][Grp] & 0xFFFF) +    /* Overflow from byte 0(H) ? */
                 (Ifld << 8)) > 0xFFFF)
               CL_C[Grp] = ON;
            if ((w_byte & 0xFF00) == 0x0000)   /* Result zero ? */
               CL_Z[Grp] = ON;
            w_byte &= 0x3FFFF;                 /* Remove possible overflow bit */
            /* Store result back in register */
            GR[Rfld][Grp] = w_byte;
         } else {                              /* Byte X, 0(H) & 1(L) */
            w_byte = GR[Rfld][Grp] + Ifld;
            if (((GR[Rfld][Grp] & 0x0FFFF) +   /* Overflow from byte 1(L) ? */
                 (Ifld)) > 0xFFFF)
               CL_C[Grp] = ON;
            if ((w_byte & 0xFFFF) == 0x0000)   /* Result zero ? (X-byte not include) */
               CL_Z[Grp] = ON;
            w_byte &= 0x3FFFF;                 /* Remove possible overflow bit */
            /* Store result back in register */
            GR[Rfld][Grp] = w_byte;
         }
         break;

      case (0xA000):
         /* SRI  R(N),I         [RI]  */
         /* 01234567 89012345
            10100RRN I<---->I         */
         Grp = RegGrp(lvl);
         Rfld = (opcode0 & 0x06) + 1;          /* Extract odd register nr */
         Nfld = (opcode0 & 0x01);
         Ifld =  opcode1;
         /* Reset C&Z latches */
         CL_Z[Grp] = OFF;
         CL_C[Grp] = OFF;

         w_byte = Ifld;                        /* Get second operand */

         /* Perform SUB with operand 1 */
         if (Nfld == 0) {                      /* Byte 0(H) result */
            w_byte = GR[Rfld][Grp] + (~(w_byte << 8)) + 1;
            if (((GR[Rfld][Grp] & 0x0FF00) +   /* Overflow from byte 0(H) ? */
                 (~(Ifld << 8) & 0x3FF00) + 0x0100) & 0x10000)
               CL_C[Grp] = ON;
            if ((w_byte & 0x0FF00) == 0x0000)  /* Result zero ? (X-byte not include) */
               CL_Z[Grp] = ON;
            w_byte &= 0x3FFFF;                 /* Remove possible overflow bit */
         } else {                              /* Byte 0 & 1 result */
            w_byte = (GR[Rfld][Grp] + (~w_byte) + 1);
            if (((GR[Rfld][Grp] & 0x0FFFF) +   /* Overflow from byte 0 & 1 ? */
                 (~Ifld) + 1) & 0x10000)
               CL_C[Grp] = ON;
            if ((w_byte & 0xFFFF) == 0x0000)   /* Result zero ? (X-byte not include) */
               CL_Z[Grp] = ON;
            w_byte &= 0x3FFFF;                 /* Remove possible overflow bit */
         }
         /* Store result back in register */
         GR[Rfld][Grp] = w_byte;
         break;

      case (0xB000):
         /* CRI  R(N),I         [RI]  */
         /* 01234567 89012345
            10110RRN I<---->I         */
         Grp  = RegGrp(lvl);
         Rfld = (opcode0 & 0x06) + 1;          /* Extract odd register nr */
         Nfld = (opcode0 & 0x01);
         Ifld = opcode1;
         /* Reset C&Z latches */
         CL_C[Grp] = OFF;
         CL_Z[Grp] = OFF;

         if (Nfld == 0)                        /* Byte 0(H) */
            w_byte = (GR[Rfld][Grp] >> 8) & 0xFF;
         else                                  /* Byte 1(L) */
            w_byte = GR[Rfld][Grp] & 0x000FF;
         /* Update C&Z latches */
         if (w_byte < Ifld)                    /* R < Ifld ? */
            CL_C[Grp] = ON;
         if (w_byte == Ifld)                   /* Equal ? */
            CL_Z[Grp] = ON;
         break;

      case (0xC000):
         /* XRI  R(N),I         [RI]  */
         /* 01234567 89012345
            11000RRN I<---->I         */
         Grp  = RegGrp(lvl);
         Rfld = (opcode0 & 0x06) + 1;          /* Extract odd register nr */
         Nfld = (opcode0 & 0x01);
         Ifld =  opcode1;
         /* Reset C&Z latches */
         CL_C[Grp] = OFF;
         CL_Z[Grp] = OFF;

         if (Nfld == 0) {                      /* Byte 0(H) */
            GR[Rfld][Grp] = GR[Rfld][Grp] ^ (Ifld << 8);  /* XR */
            /* Update C&Z latches */
            if ((GR[Rfld][Grp] & 0x0FF00) == 0x00000)
               CL_Z[Grp] = ON;
            else
               CL_C[Grp] = ON;
         } else {                              /* Byte 1(L) */
            GR[Rfld][Grp] = GR[Rfld][Grp] ^ (Ifld);   /* XR */
            /* Update C&Z latches */
            if ((GR[Rfld][Grp] & 0x000FF) == 0x00000)
               CL_Z[Grp] = ON;
            else
               CL_C[Grp] = ON;
         }
         break;

      case (0xD000):
         /* ORI  R(N),I         [RI]  */
         /* 01234567 89012345
            11010RRN I<---->I         */
         Grp = RegGrp(lvl);
         Rfld = (opcode0 & 0x06) + 1;          /* Extract odd register nr */
         Nfld = (opcode0 & 0x01);
         Ifld =  opcode1;
         /* Reset C&Z latches */
         CL_C[Grp] = OFF;
         CL_Z[Grp] = OFF;

         if (Nfld == 0) {                      /* Byte 0(H) */
            GR[Rfld][Grp] = GR[Rfld][Grp] | (Ifld << 8);  /* OR */
            /* Update C&Z latches */
            if ((GR[Rfld][Grp] & 0x0FF00) == 0x00000)
               CL_Z[Grp] = ON;
            else
               CL_C[Grp] = ON;
         } else {                              /* Byte 1(L) */
            GR[Rfld][Grp] = GR[Rfld][Grp] | (Ifld);   /* OR */
            /* Update C&Z latches */
            if ((GR[Rfld][Grp] & 0x000FF) == 0x00000)
               CL_Z[Grp] = ON;
            else
               CL_C[Grp] = ON;
         }
         break;

      case (0xE000):
         /* NRI  R(N),I         [RI]  */
         /* 01234567 89012345
            11100RRN I<---->I         */
         Grp = RegGrp(lvl);
         Rfld = (opcode0 & 0x06) + 1;          /* Extract odd register nr */
         Nfld = (opcode0 & 0x01);
         Ifld =  opcode1;
         /* Reset C&Z latches */
         CL_C[Grp] = OFF;
         CL_Z[Grp] = OFF;

         if (Nfld == 0) {                      /* Byte 0(H) */
            Ifld = (Ifld << 8) | 0xF00FF;
            GR[Rfld][Grp] = GR[Rfld][Grp] & Ifld;     /* AND */
            /* Update C&Z latches */
            if ((GR[Rfld][Grp] & 0x0FF00) == 0x00000)
               CL_Z[Grp] = ON;
            else
               CL_C[Grp] = ON;
         } else {                              /* Byte 1(L) */
            Ifld = Ifld | 0x3FF00;
            GR[Rfld][Grp] = GR[Rfld][Grp] & Ifld;    /* AND */
            /* Update C&Z latches */
            if ((GR[Rfld][Grp] & 0x000FF) == 0x00000)
               CL_Z[Grp] = ON;
            else
               CL_C[Grp] = ON;
         }
         break;

      case (0xF000):
         /* TRM  R(N),I         [RI]  */
         /* 01234567 89012345
            11110RRN I<---->I         */
         Grp = RegGrp(lvl);
         Rfld = (opcode0 & 0x06) + 1;          /* Extract odd register nr */
         Nfld = (opcode0 & 0x01);
         Ifld =  opcode1;
         /* Reset C&Z latches */
         CL_C[Grp] = OFF;
         CL_Z[Grp] = OFF;

         if (Nfld == 0)                        /* Byte 0(H) */
            w_byte = GR[Rfld][Grp] >> 8;
         else                                  /* Byte 1(L) */
            w_byte = GR[Rfld][Grp] & 0x00FF;
         /* Update C&Z latches */
         if ((w_byte & Ifld) == 0x00)
            CL_Z[Grp] = ON;
         else
            CL_C[Grp] = ON;
         break;
   }

   switch (opcode & 0x88FF) {
      case (0x0008):
         /* LCR  R1(N1),R2(N2)  [RR]  */
         /* 01234567 89012345
            0R2N0R1N 00001000         */
         Grp = RegGrp(lvl);
         R1fld = ( opcode0 & 0x06) + 1;        /* Extract reg 1 nr */
         N1fld = ( opcode0 & 0x01);
         R2fld = ((opcode0 & 0x60) >> 4) + 1;  /* Extract reg 2 nr */
         N2fld = ((opcode0 & 0x10) >> 4);
         /* Reset Z&C latches */
         CL_C[Grp] = OFF;
         CL_Z[Grp] = OFF;

         /* Fetch the selected byte from R2 */
         if (N2fld == 0)                       /* Byte 0(H) */
            w_byte = (GR[R2fld][Grp] >> 8) & 0x000FF;
         else
            w_byte = GR[R2fld][Grp] & 0x000FF; /* Byte 1(L) */

         /* Store it the selected byte of R1 */
         if (N1fld == 0)                       /* Byte 0(H) */
            GR[R1fld][Grp] = (GR[R1fld][Grp] & 0x000FF) | (w_byte << 8);
         else                                  /* Byte 1(L) */
            GR[R1fld][Grp] = (GR[R1fld][Grp] & 0x0FF00) | w_byte;
         /* Set Z Latch if selected byte == 0x00 */
         if (w_byte == 0x00)
            CL_Z[Grp] = ON;
         /* Determine if nr of bits is odd or even and set C latch accordingly */
         j = 0;
         for (i = 8; i != 0; i--) {
            j += w_byte & 0x01;                /* Count the one bits */
            w_byte >>= 1;
         }
         if (j == 0 | j == 2 | j == 4 | j == 6 | j == 8)
            CL_C[Grp] = ON;                    /* Update C latch */
         else
            CL_C[Grp] = OFF;
         break;

      case (0x0018):
         /* ACR  R1(N1),R2(N2)  [RR]  */
         /* 01234567 89012345
            0R2N0R1N 00011000         */
         Grp = RegGrp(lvl);
         R1fld = ( opcode0 & 0x06) + 1;        /* Extract reg 1 nr */
         R2fld = ((opcode0 & 0x60) >> 4) + 1;  /* Extract reg 2 nr */
         N1fld = ( opcode0 & 0x01);
         N2fld = ((opcode0 & 0x10) >> 4);
         /* Reset Z&C latches */
         CL_Z[Grp] = OFF;
         CL_C[Grp] = OFF;

         /* Fetch the selected byte from R2 */
         if (N2fld == 0)                       /* Byte 0(H) */
            w_byte = (GR[R2fld][Grp] >> 8) & 0x000FF;
         else
            w_byte = GR[R2fld][Grp] & 0x000FF; /* Byte 1(L) */

         /* Perform ADD with the selected byte from R1 */
         if (N1fld == 0) {                     /* Byte 0(H) result */
            w_byte = GR[R1fld][Grp] + (w_byte << 8);
            if ((w_byte & 0x0FF00) == 0x00)    /* Zero ? */
               CL_Z[Grp] = ON;
         } else {                              /* Byte 0 & 1 result */
            w_byte = GR[R1fld][Grp] + w_byte;
            if ((w_byte & 0x0FFFF) == 0x00000) /* Zero ? */
               CL_Z[Grp] = ON;
         }
         if (w_byte > 0xFFFF)                  /* Overflow ? */
            CL_C[Grp] = ON;
         /* Remove possible overflow bit and save the result */
         GR[R1fld][Grp] = w_byte & 0x3FFFF;
         break;

      case (0x0028):
         /* SCR  R1(N1),R2(N2)  [RR]  */
         /* 01234567 89012345
            0R2N0R1N 00101000         */
         Grp = RegGrp(lvl);
         R1fld = ( opcode0 & 0x06) + 1;        /* Extract reg 1 nr */
         N1fld = ( opcode0 & 0x01);
         R2fld = ((opcode0 & 0x60) >> 4)  + 1; /* Extract reg 2 nr */
         N2fld = ((opcode0 & 0x10) >> 4);
         /* Reset Z&C latches */
         CL_Z[Grp] = OFF;
         CL_C[Grp] = OFF;

         int32 R2H, R2L;

         if (N1fld == 0) {                     /* R1 = Byte 0(H) only */
            if (N2fld == 0) {                  /* R2 = Byte 0(H)  */
                                     /* SCR: R1(H.) = R1(H.) - R2(H.) */
               w_byte = (GR[R2fld][Grp]) & 0x0FF00;
            } else {                           /* R2 = Byte 1(L) */
                                     /* SCR: R1(H.) = R1(H.) - R2(.L) */
               w_byte = (GR[R2fld][Grp] << 8) & 0x0FF00;
            }
            if (w_byte > (GR[R1fld][Grp] & 0x0FF00))  /* Result < 0 ? */
               CL_C[Grp] = ON;
            R2H = ~(w_byte);
            R2H = (R2H + 0x00100) & 0x3FF00;   /* 2-complement */
            GR[R1fld][Grp] = (GR[R1fld][Grp] + R2H) & 0x3FFFF;
            if ((GR[R1fld][Grp] & 0x0FF00) == 0x00)   /* Result zero ?*/
               CL_Z[Grp] = ON;

         } else {     /* N1fld == 1 */         /* R1 = Byte H & L */

            if (N2fld == 0) {                  /* R2 = Byte 0(H)  */
                                     /* SCR: R1(HL) = R1(HL) - R2(H.) */
               w_byte = (GR[R2fld][Grp] >> 8) & 0x000FF;
            } else {                           /* R2 = Byte 1(L)  */
                                     /* SCR: R1(HL) = R1(HL) - R2(.L) */
               w_byte = (GR[R2fld][Grp]) & 0x000FF;
            }
            if (w_byte > (GR[R1fld][Grp] & 0x0FFFF))  /* Result < 0 ? */
               CL_C[Grp] = ON;
            R2L = ~(w_byte);
            R2L = (R2L + 1) & 0x3FFFF;         /* 2-complement */
            GR[R1fld][Grp] = (GR[R1fld][Grp] + R2L) & 0x3FFFF;
            if ((GR[R1fld][Grp] & 0x0FFFF) == 0x0000) /* Result zero ?*/
               CL_Z[Grp] = ON;
         }
         break;

      case (0x0038):
         /* CCR  R1(N1),R2(N2)  [RR]  */
         /* 01234567 89012345
            0R2N0R1N 00111000         */
         Grp = RegGrp(lvl);
         R1fld = ( opcode0 & 0x06) + 1;        /* Extract reg 1 nr */
         R2fld = ((opcode0 & 0x60) >> 4) + 1;  /* Extract reg 2 nr */
         N1fld = ( opcode0 & 0x01);
         N2fld = ((opcode0 & 0x10) >> 4);
         /* Reset Z&C latches */
         CL_Z[Grp] = OFF;
         CL_C[Grp] = OFF;

         /* Fetch the required byte from R2 */
         if (N2fld == 0)
            w_byte = GR[R2fld][Grp] >> 8;      /* Byte 0(H) */
         else
            w_byte = GR[R2fld][Grp] & 0x000FF; /* Byte 1(L) */

         /* Perform a compare between the selected regs */
         if (N1fld == 0) {                     /* Byte 0(H) */
            if (( GR[R1fld][Grp] >> 8) < w_byte)
               CL_C[Grp] = ON;
            if (( GR[R1fld][Grp] >> 8) == w_byte)
               CL_Z[Grp] = ON;
         } else {                              /* Byte 1(L) */
            if (( GR[R1fld][Grp] & 0x00FF) < w_byte)
               CL_C[Grp] = ON;
            if (( GR[R1fld][Grp] & 0x00FF) == w_byte)
               CL_Z[Grp] = ON;
         }
         break;

      case (0x0048):
         /* XCR  R1(N1),R2(N2)  [RR]  */
         /* 01234567 89012345
            0R2N0R1N 01001000         */
         Grp = RegGrp(lvl);
         R1fld = ( opcode0 & 0x06) + 1;        /* Extract odd reg 1 nr */
         R2fld = ((opcode0 & 0x60) >> 4) + 1;  /* Extract odd reg 2 nr */
         N1fld = ( opcode0 & 0x01);
         N2fld = ((opcode0 & 0x10) >> 4);
         /* Reset Z&C latches */
         CL_Z[Grp] = OFF;
         CL_C[Grp] = OFF;

         /* Fetch the selected byte from R2 */
         if (N2fld == 0)                       /* Byte 0(H) */
            w_byte = (GR[R2fld][Grp] >> 8) & 0x000FF;
         else
            w_byte = GR[R2fld][Grp] & 0x000FF; /* Byte 1(L) */

         /* Perform XOR with the selected byte from R1 */
         if (N1fld == 0) {                     /* Byte 0(H) */
            GR[R1fld][Grp] ^= (w_byte << 8);
            if ((GR[R1fld][Grp] & 0xFF00) == 0x00)
               CL_Z[Grp] = ON;
            else
               CL_C[Grp] = ON;
         } else {                              /* Byte 1(L) */
            GR[R1fld][Grp] ^= w_byte;
            if ((GR[R1fld][Grp] & 0x00FF) == 0x00)
               CL_Z[Grp] = ON;
            else
               CL_C[Grp] = ON;
         }
         break;

      case (0x0058):
         /* OCR  R1(N1),R2(N2)  [RR]  */
         /* 01234567 89012345
            0R2N0R1N 01011000         */
         Grp = RegGrp(lvl);
         R1fld = ( opcode0 & 0x06) + 1;        /* Extract odd reg 1 nr */
         N1fld = ( opcode0 & 0x01);
         R2fld = ((opcode0 & 0x60) >> 4) + 1;  /* Extract odd reg 2 nr */
         N2fld = ((opcode0 & 0x10) >> 4);
         /* Reset Z&C latches */
         CL_Z[Grp] = OFF;
         CL_C[Grp] = OFF;

         /* Fetch the selected byte from R2 */
         if (N2fld == 0)                       /* Byte 0(H) */
            w_byte = (GR[R2fld][Grp] >> 8) & 0x000FF;
         else
            w_byte = GR[R2fld][Grp] & 0x000FF; /* Byte 1(L) */

         /* Perform OR with the selected byte from R1 */
         if (N1fld == 0) {                     /* Byte 0(H) */
            GR[R1fld][Grp] |= (w_byte << 8);
            if ((GR[R1fld][Grp] & 0x0FF00) == 0x00)
               CL_Z[Grp] = ON;
            else
               CL_C[Grp] = ON;
         } else {                              /* Byte 1(L) */
            GR[R1fld][Grp] |= w_byte;
            if ((GR[R1fld][Grp] & 0x000FF) == 0x00)
               CL_Z[Grp] = ON;
            else
               CL_C[Grp] = ON;
         }
         break;

      case (0x0068):
         /* NCR  R1(N1),R2(N2)  [RR]  */
         /* 01234567 89012345
            0R2N0R1N 01101000         */
         Grp = RegGrp(lvl);
         R1fld = ( opcode0 & 0x06) + 1;        /* Extract odd reg 1 nr */
         R2fld = ((opcode0 & 0x60) >> 4) + 1;  /* Extract odd reg 2 nr */
         N1fld = ( opcode0 & 0x01);
         N2fld = ((opcode0 & 0x10) >> 4);
         /* Reset Z&C latches */
         CL_Z[Grp] = OFF;
         CL_C[Grp] = OFF;

         /* Fetch the selected byte from R2 */
         if (N2fld == 0)                       /* Byte 0(H) */
            w_byte = (GR[R2fld][Grp] >> 8) & 0x000FF;
         else
            w_byte = GR[R2fld][Grp] & 0x000FF; /* Byte 1(L) */

         /* Perform AND with the selected byte from R1 */
         if (N1fld == 0) {
            GR[R1fld][Grp] &= ((w_byte << 8) | 0xF00FF);
            if ((GR[R1fld][Grp] & 0x0FF00) == 0x00)
               CL_Z[Grp] = ON;
            else
               CL_C[Grp] = ON;
         } else {
            GR[R1fld][Grp] &= (w_byte | 0x3FF00);
            if ((GR[R1fld][Grp] & 0x000FF) == 0x00)
               CL_Z[Grp] = ON;
            else
               CL_C[Grp] = ON;
         }
         break;

      case (0x0078):
         /* LCOR R1(N1),R2(N2)  [RR]  */
         /* 01234567 89012345
            0R2N0R1N 01111000         */
         Grp = RegGrp(lvl);
         R1fld = ( opcode0 & 0x06) + 1;        /* Extract odd reg 1 nr */
         R2fld = ((opcode0 & 0x60) >> 4) + 1;  /* Extract odd reg 2 nr */
         N1fld = ( opcode0 & 0x01);
         N2fld = ((opcode0 & 0x10) >> 4);
         /* Reset Z&C latches */
         CL_Z[Grp] = OFF;
         CL_C[Grp] = OFF;

         /* Fetch the selected byte from R2 */
         if (N2fld == 0)                       /* Byte 0(H) */
            w_byte = (GR[R2fld][Grp] >> 8) & 0x000FF;
         else
            w_byte = GR[R2fld][Grp] & 0x000FF; /* Byte 1(L) */

         /* Determine C latch and Shift one byte to the right */
         if ((w_byte & 0x00001) == 0x0001)     /* Will we loose a one bit? */
            CL_C[Grp] = ON;
         w_byte = w_byte >> 1;                 /* Shift 1 bit right */

         /* Store it the selected byte of R1 */
         if (N1fld == 0)                       /* Byte 0(H) */
            GR[R1fld][Grp] = (GR[R1fld][Grp] & 0x000FF) | (w_byte << 8);
         else                                  /* Byte 1(L) */
            GR[R1fld][Grp] = (GR[R1fld][Grp] & 0x0FF00) | w_byte;
         /* Set Z Latch */
         if (w_byte == 0x00)
            CL_Z[Grp] = ON;
         break;

      case (0x0010):
         /* ICT  R(N),B         [RSA] */
         /* 01234567 89012345
            0BBB0RRN 00010000         */
         Grp = RegGrp(lvl);
         Bfld = (opcode0 >> 4) & 0x007;
         Rfld = (opcode0 & 0x06) + 1;          /* Extract odd register nr */
         Nfld = (opcode0 & 0x01);

         addr = GR[Bfld][Grp];                 /* See PoO 4-9 */
         w_byte = GetMem(addr);
         GR[Bfld][Grp] = GR[Bfld][Grp] + 1;
         if (Nfld == 0) {                      /* Byte 0(H) */
            GR[Rfld][Grp] = (GR[Rfld][Grp] & 0xF00FF) | (w_byte << 8);
         } else {                              /* Byte 1(L) */
            GR[Rfld][Grp] = (GR[Rfld][Grp] & 0x3FF00) | w_byte;
         }
         break;

      case (0x0030):
         /* STCT R(N),B         [RSA] */
         /* 01234567 89012345
            0BBB0RRN 00110000         */
         Grp = RegGrp(lvl);
         Bfld = (opcode0 >> 4) & 0x007;
         Rfld = (opcode0 & 0x06) + 1;          /* Extract odd register nr */
         Nfld = (opcode0 & 0x01);

         addr = GR[Bfld][Grp];                 /* See PoO 4-13 */
         GR[Bfld][Grp] = GR[Bfld][Grp] + 1;
         if (Nfld == 0)                        /* Byte 0(H) */
            w_byte = (GR[Rfld][Grp] >> 8) & 0x000FF;
         else
            w_byte = GR[Rfld][Grp] & 0x000FF;  /* Byte 1(L) */
         PutMem(addr, w_byte);
         break;
   }

   switch (opcode & 0x8880) {
      case (0x0800):
         /* IC   R(N),D(B)      [RS]  */
         /* 01234567 89012345
            0BBB1RRN 0D<--->D         */
         Grp = RegGrp(lvl);
         Bfld = (opcode0 >> 4) & 0x007;
         Rfld = (opcode0 & 0x06) + 1;          /* Extract odd register nr */
         Nfld = (opcode0 & 0x01);
         Dfld = (opcode1) & 0x7F;

         if (Bfld == 0)
            addr = 0x00680 + Dfld;             /* See PoO 4-9 */
         else
            addr = GR[Bfld][Grp] + Dfld;
         w_byte = GetMem(addr);

         if (Nfld == 0)                        /* Byte 0(H) */
            GR[Rfld][Grp] = (GR[Rfld][Grp] & 0xF00FF) | (w_byte << 8);
         else                                  /* Byte 1(L) */
            GR[Rfld][Grp] = (GR[Rfld][Grp] & 0x3FF00) | w_byte;

         /* Test the selected byte (w_byte) */
         if (w_byte == 0x00)
            CL_Z[Grp] = ON;
         else
            CL_Z[Grp] = OFF;
         /* Determine if nr of bits is odd or even and set C latch accordingly */
         j = 0;
         for (i = 8; i != 0; i--) {
            j += w_byte & 0x01;                /* Count the one bits */
            w_byte >>= 1;
         }
         if (j == 0 | j == 2 | j == 4 | j == 6 | j == 8)
            CL_C[Grp] = ON;
         else
            CL_C[Grp] = OFF;
         break;

      case (0x0880):
         /* STC  R(N),D(B)      [RS]  */
         /* 01234567 89012345
            0BBB1RRN 1D<--->D         */
         Grp = RegGrp(lvl);
         Bfld = (opcode0 >> 4) & 0x07;
         Dfld = (opcode1) & 0x7F;
         Rfld = (opcode0 & 0x06) + 1;          /* Extract odd register nr */
         Nfld = (opcode0 & 0x01);

         if (Bfld == 0)
            addr = 0x00680 + Dfld;             /* See PoO 4-13 */
         else
            addr = GR[Bfld][Grp] + Dfld;

         if (Nfld == 0)
            w_byte = (GR[Rfld][Grp] >> 8) & 0x000FF;
         else
            w_byte = (GR[Rfld][Grp] & 0x000FF);
         PutMem(addr, w_byte);
         break;
   }

   switch (opcode & 0x8881) {
      case (0x0001):
         /* LH   R,D(B)         [RS]  */
         /* 01234567 89012345
            0BBB0RRR 0D<-->D1         */
         Grp = RegGrp(lvl);
         Bfld = (opcode0 >> 4) & 0x007;
         Dfld = (opcode1) & 0x7E;
         Rfld = (opcode0) & 0x007;             /* Extract register nr */

         if (Bfld == 0)
            addr = 0x00700 + Dfld;             /* See PoO 4-10 */
         else
            addr = (GR[Bfld][Grp] + Dfld);
         addr &= 0x3FFFE;                      /* Force HW boundary */

         w_byte = GetMem(addr) << 8;
         addr++;
         w_byte = w_byte | GetMem(addr);
         GR[Rfld][Grp] = w_byte;               /* X-byte = 0 */
         if (Rfld == 0) break;                 /* New IAR ! */

         /* Update C&Z latches */
         if (w_byte == 0x0000) {
            CL_Z[Grp] = ON;
            CL_C[Grp] = OFF;
         } else {
            CL_Z[Grp] = OFF;
            CL_C[Grp] = ON;
         }
         break;

      case (0x0081):
         /* STH  R,D(B)         [RS]  */
         /* 01234567 89012345
            0BBB0RRR 1D<-->D1         */
         Grp = RegGrp(lvl);
         Bfld = (opcode0 >> 4) & 0x007;
         Rfld = (opcode0) & 0x007;             /* Extract register nr */
         Dfld = (opcode1) & 0x7E;

         if (Bfld == 0)
            addr = 0x00700 + Dfld;             /* See PoO 4-4 */
         else
            addr = (GR[Bfld][Grp] + Dfld);
         addr &= 0x3FFFE;                      /* Force HW boundary */

         if (Rfld > 0) {
            PutMem(addr, (GR[Rfld][Grp] >> 8) & 0x000FF);
            addr++;
            PutMem(addr, GR[Rfld][Grp] & 0x000FF);
         } else {
            PutMem(addr,   0x00);
            PutMem(addr+1, 0x00);
         }
         break;
   }

   switch (opcode & 0x8883) {
      case (0x0002):
         /* L    R,D(B)         [RS]  */
         /* 01234567 89012345
            0BBB0RRR 0D<->D10         */
         Grp = RegGrp(lvl);
         Bfld = (opcode0 >> 4) & 0x007;
         Dfld = (opcode1) & 0x7C;              /* Dfld at fullword boundary */
         Rfld = (opcode0) & 0x007;             /* Extract register nr */

         if (Bfld == 0)
            addr = 0x00780 + Dfld;             /* See PoO 4-10 */
         else
            addr = (GR[Bfld][Grp] + Dfld);
         addr &= 0x3FFFE;                      /* Force HW boundary */

         w_byte = (GetMem(addr+1) & 0x03) << 16; /* Load X-byte */
         w_byte |= GetMem(addr+2) << 8;        /* Byte 0(H) */
         w_byte |= GetMem(addr+3);             /* Byte 1(L) */
         GR[Rfld][Grp] = w_byte;
         if (Rfld == 0) break;                 /* New IAR ! */

         /* Update C&Z latches */
         if (w_byte == 0x00000) {              /* Test includes X-byte */
            CL_Z[Grp] = ON;
            CL_C[Grp] = OFF;
         } else {
            CL_Z[Grp] = OFF;
            CL_C[Grp] = ON;
         }
         break;

      case (0x0082):
         /* ST   R,D(B)         [RS]  */
         /* 01234567 89012345
            0BBB0RRR 1D<->D10         */
         Grp = RegGrp(lvl);
         Bfld = (opcode0 >> 4) & 0x007;
         Dfld = (opcode1) & 0x7C;              /* Dfld at fullword boundary */
         Rfld = (opcode0) & 0x07;              /* Extract register nr */

         if (Bfld == 0)
            addr = 0x00780 + Dfld;             /* See PoO 4-12 */
         else
            addr = (GR[Bfld][Grp] + Dfld);
         addr &= 0x3FFFE;                      /* Force HW boundary */

         if (Rfld > 0) {
            PutMem(addr+3,  GR[Rfld][Grp] & 0xFF);
            PutMem(addr+2, (GR[Rfld][Grp] >> 8) & 0xFF);
            w_byte = GetMem(addr+1) & 0xFC;    /* Keep the high 6 bits */
            PutMem(addr+1, w_byte | ((GR[Rfld][Grp] >> 16) & 0x03));
         } else {
            PutMem(addr+3, 0x00);              /* Clear mem locations */
            PutMem(addr+2, 0x00);
            w_byte = GetMem(addr+1) & 0xFC;    /* Keep the high 6 bits */
            PutMem(addr+1, w_byte);            /* Clear X-byte bits */
         }
         // NOTE: special condition ST inst at loc 0x0010 to be implemented !!
         break;
   }

   switch (opcode & 0x88FF) {
      case (0x0080):
         /* LHR  R1,R2          [RR]  */
         /* 01234567 89012345
            0R2R0R1R 10000000         */
         Grp = RegGrp(lvl);
         R2fld = ((opcode0 & 0x70) >> 4);      /* Extract register 2 */
         R1fld = ( opcode0 & 0x07);            /* Extract register 1 */

         w_byte = GR[R2fld][Grp] & 0x0FFFF;    /* Load R1 with contents of R2 */
         GR[R1fld][Grp] = w_byte;
         /* If R1 = Register 0, a branch to newly formed address occurs */
         if (R1fld == 0) break;

         /* Update C&Z latches */
         if (GR[R1fld][Grp] == 0x0000) {
            CL_Z[Grp] = ON;
            CL_C[Grp] = OFF;
         } else {
            CL_Z[Grp] = OFF;
            CL_C[Grp] = ON;
         }
         break;

      case (0x0090):
         /* AHR  R1,R2          [RR]  */
         /* 01234567 89012345
            0R2R0R1R 10001000         */
         Grp = RegGrp(lvl);
         R2fld = ((opcode0 & 0x70) >> 4);      /* Extract register 2 */
         R1fld = ( opcode0 & 0x007);           /* Extract register 1 */

         w_byte = (GR[R1fld][Grp] & 0xFFFF) + (GR[R2fld][Grp] & 0xFFFF);
         GR[R1fld][Grp] = (GR[R1fld][Grp] & 0xF0000) | (w_byte & 0xFFFF);
         /* If R1 = Register 0, a branch to newly formed address occurs */
         if (R1fld == 0) break;

         /* Reset C&Z latches */
         CL_C[Grp] = OFF;
         CL_Z[Grp] = OFF;
         /* Update C&Z latches */
         if (w_byte & 0x10000)                 /* Overflow ? */
            CL_C[Grp] = ON;
         if ((w_byte & 0xFFFF) == 0x0000)      /* Result 0 ? */
            CL_Z[Grp] = ON;
         break;

      case (0x00A0):
         /* SHR  R1,R2          [RR]  */
         /* 01234567 89012345
            0R2R0R1R 10011000         */
         Grp = RegGrp(lvl);
         R2fld = ((opcode0 & 0x70) >> 4);      /* Extract register 2 */
         R1fld = ( opcode0 & 0x007);           /* Extract register 1 */

         w_byte = GR[R1fld][Grp] + ~(GR[R2fld][Grp]) + 1;
         GR[R1fld][Grp] = w_byte & 0xFFFF;     /* Remove possible overflow bit */
         /* If R1 = Register 0, a branch to newly formed address occurs */
         if (R1fld == 0) break;

         /* Reset C&Z latches */
         CL_C[Grp] = OFF;
         CL_Z[Grp] = OFF;
         /* Update C&Z latches */
         if (w_byte & 0x10000)                 /* Result < 0 ? */
            CL_C[Grp] = ON;
         if (GR[R1fld][Grp] == 0x0000)         /* Result == 0 ? */
            CL_Z[Grp] = ON;
         break;

      case (0x00B0):
         /* CHR  R1,R2          [RR]  */
         /* 01234567 89012345
            0R2R0R1R 10110000         */
         Grp = RegGrp(lvl);
         R2fld = ((opcode0 & 0x70) >> 4);      /* Extract register 2 */
         R1fld = ( opcode0 & 0x007);           /* Extract register 1 */
         /* Reset C&Z latches */
         CL_C[Grp] = OFF;
         CL_Z[Grp] = OFF;

         /* Test if R1 is < R2 */
         if ((GR[R1fld][Grp] & 0xFFFF) ==      /* Compare for equal */
             (GR[R2fld][Grp] & 0xFFFF))
            CL_Z[Grp] = ON;
         if ((GR[R1fld][Grp] & 0xFFFF) <       /* Compare for less */
             (GR[R2fld][Grp] & 0xFFFF))
            CL_C[Grp] = ON;
         break;

      case (0x00C0):
         /* XHR  R1,R2          [RR]  */
         /* 01234567 89012345
            0R2R0R1R 11000000         */
         Grp = RegGrp(lvl);
         R2fld = ((opcode0 & 0x70) >> 4);      /* Extract register 2 */
         R1fld = ( opcode0 & 0x007);           /* Extract register 1 */

         w_byte = (GR[R1fld][Grp] & 0x0FFFF) ^ (GR[R2fld][Grp] & 0x0FFFF);
         GR[R1fld][Grp] = (GR[R1fld][Grp] & 0xF0000) | w_byte;  /* XHR */
         /* If R1 = Register 0, a branch to newly formed address occurs */
         if (R1fld == 0) break;

         /* Update C&Z latches */
         if (w_byte == 0x0000) {
            CL_Z[Grp] = ON;
            CL_C[Grp] = OFF;
         } else {
            CL_Z[Grp] = OFF;
            CL_C[Grp] = ON;
         }
         break;

      case (0x00D0):
         /* OHR  R1,R2          [RR]  */
         /* 01234567 89012345
            0R2R0R1R 11010000         */
         Grp = RegGrp(lvl);
         R2fld = ((opcode0 & 0x70) >> 4);      /* Extract register 2 */
         R1fld = ( opcode0 & 0x007);           /* Extract register 1 */

         w_byte = (GR[R1fld][Grp] & 0x0FFFF) | (GR[R2fld][Grp] & 0x0FFFF);
         GR[R1fld][Grp] = (GR[R1fld][Grp] & 0xF0000) | w_byte;  /* OHR */
         /* If R1 = Register 0, a branch to newly formed address occurs */
         if (R1fld == 0) break;

         /* Update C&Z latches */
         if ((GR[R1fld][Grp] & 0xFFFF) == 0x0000) {
            CL_Z[Grp] = ON;
            CL_C[Grp] = OFF;
         } else {
            CL_Z[Grp] = OFF;
            CL_C[Grp] = ON;
         }
         break;

      case (0x00E0):
         /* NHR  R1,R2          [RR]  */
         /* 01234567 89012345
            0R2R0R1R 11100000         */
         Grp = RegGrp(lvl);
         R2fld = ((opcode0 & 0x70) >> 4);      /* Extract register 2 */
         R1fld = ( opcode0 & 0x007);           /* Extract register 1 */

         w_byte = (GR[R1fld][Grp] & 0x0FFFF) & (GR[R2fld][Grp] & 0x0FFFF);
         GR[R1fld][Grp] = (GR[R1fld][Grp] & 0xF0000) | w_byte;  /* OHR */
         /* If R1 = Register 0, a branch to newly formed address occurs */
         if (R1fld == 0) break;

         /* Update C&Z latches */
         if (w_byte == 0x0000) {
            CL_Z[Grp] = ON;
            CL_C[Grp] = OFF;
         } else {
            CL_Z[Grp] = OFF;
            CL_C[Grp] = ON;
         }
         break;

      case (0x00F0):
         /* LHOR R1,R2          [RR]  */
         /* 01234567 89012345
            0R2R0R1R 11110000         */
         Grp = RegGrp(lvl);
         R2fld = ((opcode0 & 0x70) >> 4);      /* Extract register 2 */
         R1fld = ( opcode0 & 0x007);           /* Extract register 1 */

         w_byte = GR[R2fld][Grp];
         GR[R1fld][Grp] = (GR[R2fld][Grp] & 0x0FFFF) >> 1; /* Shift 1 bit to the right */
         if (Rfld == 0) break;                 /* New IAR ! */

         /* Reset C&Z latches */
         CL_C[Grp] = OFF;
         CL_Z[Grp] = OFF;
         /* Update C&Z latches */
         /* If a 1 bit will be shifted out, set C latch */
         if (w_byte & 0x00001)
            CL_C[Grp] = ON;
         if (GR[R1fld][Grp] == 0x00000)
            CL_Z[Grp] = ON;
         break;

      case (0x0088):
         /* LR   R1,R2          [RR]  */
         /* 01234567 89012345
            0R2R0R1R 10001000         */
         Grp = RegGrp(lvl);
         R2fld = ((opcode0 & 0x70) >> 4);      /* Extract register 2 */
         R1fld = ( opcode0 & 0x007);           /* Extract register 1 */

         GR[R1fld][Grp] = GR[R2fld][Grp];      /* Load R1 with contents of R2 */
         /* If R1 = Register 0, a branch to newly formed address occurs */
         if (R1fld == 0) break;

         /* Update C&Z latches */
         if (GR[R1fld][Grp] == 0x00000) {
            CL_Z[Grp] = ON;
            CL_C[Grp] = OFF;
         } else {
            CL_Z[Grp] = OFF;
            CL_C[Grp] = ON;
         }
         break;

      case (0x0098):
         /* AR   R1,R2          [RR]  */
         /* 01234567 89012345
            0R2R0R1R 10011000         */
         Grp = RegGrp(lvl);
         R2fld = ((opcode0 & 0x70) >> 4);      /* Extract register 2 */
         R1fld = ( opcode0 & 0x007);           /* Extract register 1 */

         w_byte = GR[R1fld][Grp] + GR[R2fld][Grp];
         GR[R1fld][Grp] = w_byte & 0x3FFFF;    /* Remove possible overflow bit */
         /* If R1 = Register 0, a branch to newly formed address occurs */
         if (R1fld == 0) break;

         /* Reset C&Z latches */
         CL_C[Grp] = OFF;
         CL_Z[Grp] = OFF;
         /* Update C&Z latches */
         if (w_byte & 0x40000)                /* Bit 21 overflow ? */
            CL_C[Grp] = ON;
         if (GR[R1fld][Grp] == 0x00000)
            CL_Z[Grp] = ON;
         break;

      case (0x00A8):
         /* SR   R1,R2          [RR]  */
         /* 01234567 89012345
            0R2R0R1R 10101000         */
         Grp = RegGrp(lvl);
         R2fld = ((opcode0 & 0x70) >> 4);      /* Extract register 2 */
         R1fld = ( opcode0 & 0x007);           /* Extract register 1 */

         w_byte = GR[R1fld][Grp] + ~(GR[R2fld][Grp]) + 1;   /* SR */
         GR[R1fld][Grp] = w_byte & 0x3FFFF;    /* Remove possible overflow bit */
         /* If R1 = Register 0, a branch to newly formed address occurs */
         if (R1fld == 0) break;

         /* Reset C&Z latches */
         CL_C[Grp] = OFF;
         CL_Z[Grp] = OFF;
         /* Update C&Z latches */
         if (w_byte & 0x40000)                /* X-byte included */
            CL_C[Grp] = ON;
         if (GR[R1fld][Grp] == 0x00000)
            CL_Z[Grp] = ON;
         break;

      case (0x00B8):
         /* CR   R1,R2          [RR]  */
         /* 01234567 89012345
            0R2R0R1R 10110000         */
         Grp = RegGrp(lvl);
         R2fld = ((opcode0 & 0x70) >> 4);      /* Extract register 2 */
         R1fld = ( opcode0 & 0x007);           /* Extract register 1 */
         /* Reset C&Z latches */
         CL_C[Grp] = OFF;
         CL_Z[Grp] = OFF;

         /* Test if R1 is < R2 */                           /* CR */
         if (GR[R1fld][Grp] == GR[R2fld][Grp]) /* Compare for equal */
            CL_Z[Grp] = ON;
         if (GR[R1fld][Grp] < GR[R2fld][Grp])  /* Compare for less */
            CL_C[Grp] = ON;
         break;

         if (GR[R1fld][Grp] < GR[R2fld][Grp])  /* Compare for less */
            CL_C[Grp] = ON;
         break;

      case (0x00C8):
         /* XR   R1,R2          [RR]  */
         /* 01234567 89012345
            0R2R0R1R 11001000         */
         Grp = RegGrp(lvl);
         R2fld = ((opcode0 & 0x70) >> 4);      /* Extract register 2 */
         R1fld = ( opcode0 & 0x007);           /* Extract register 1 */

         GR[R1fld][Grp] = GR[R1fld][Grp] ^ GR[R2fld][Grp];  /* XR */
         /* If R1 = Register 0, a branch to newly formed address occurs */
         if (R1fld == 0) break;

         /* Update C&Z latches */
         if (GR[R1fld][Grp] == 0x00000) {      /* Result zero ? */
            CL_Z[Grp] = ON;
            CL_C[Grp] = OFF;
         } else {
            CL_Z[Grp] = OFF;
            CL_C[Grp] = ON;
         }
         break;

      case (0x00D8):
         /* OR   R1,R2          [RR]  */
         /* 01234567 89012345
            0R2R0R1R 11011000         */
         Grp = RegGrp(lvl);
         R2fld = ((opcode0 & 0x70) >> 4);      /* Extract register 2 */
         R1fld = ( opcode0 & 0x007);           /* Extract register 1 */

         GR[R1fld][Grp] = GR[R1fld][Grp] | GR[R2fld][Grp];  /* OR */
         /* If R1 = Register 0, a branch to newly formed address occurs */
         if (R1fld == 0) break;

         /* Update C&Z latches */
         if (GR[R1fld][Grp] == 0x00000) {      /* Result zero ? */
            CL_Z[Grp] = ON;
            CL_C[Grp] = OFF;
         } else {
            CL_Z[Grp] = OFF;
            CL_C[Grp] = ON;
         }
         break;

      case (0x00E8):
         /* NR   R1,R2          [RR]  */
         /* 01234567 89012345
            0R2R0R1R 11101000         */
         Grp = RegGrp(lvl);
         R2fld = ((opcode0 & 0x70) >> 4);      /* Extract register 2 */
         R1fld = ( opcode0 & 0x007);           /* Extract register 1 */

         GR[R1fld][Grp] = GR[R1fld][Grp] & GR[R2fld][Grp];  /* NR */
         /* If R1 = Register 0, a branch to newly formed address occurs */
         if (R1fld == 0) break;

         /* Update C&Z latches */
         if (GR[R1fld][Grp] == 0x00000) {      /* Result zero ? */
            CL_Z[Grp] = ON;
            CL_C[Grp] = OFF;
         } else {
            CL_Z[Grp] = OFF;
            CL_C[Grp] = ON;
         }
         break;

      case (0x00F8):
         /* LOR  R1,R2          [RR]  */
         /* 01234567 89012345
            0R2R0R1R 11111000         */
         Grp = RegGrp(lvl);
         R2fld = ((opcode0 & 0x70) >> 4);      /* Extract register 2 */
         R1fld = ( opcode0 & 0x007);           /* Extract register 1 */

         w_byte = GR[R2fld][Grp];
         GR[R1fld][Grp] = GR[R2fld][Grp] >> 1; /* Shift 1 bit to the right */
         GR[R1fld][Grp] &= 0x007FFFF;          /* Make sure a 0 is inserted */
         if (Rfld == 0) break;                 /* New IAR ! */

         /* Reset C&Z latches */
         CL_C[Grp] = OFF;
         CL_Z[Grp] = OFF;
         /* Update C&Z latches */
         /* If a 1 bit will be shifted out, set C latch */
         if (w_byte & 0x00001)
            CL_C[Grp] = ON;
         if (GR[R1fld][Grp] == 0x00000)        /* Result zero ? */
            CL_Z[Grp] = ON;
         break;

      case (0x0040):
         /* BALR R1,R2          [RR]  */
         /* 01234567 89012345
            0R2R0R1R 01000000         */
         Grp = RegGrp(lvl);
         R2fld = ((opcode0 & 0x70) >> 4);      /* Extract register 2 */
         R1fld = ( opcode0 & 0x007);           /* Extract register 1 */

         w_byte = GR[R2fld][Grp];              /* See PoO 4-7 */
         if (R1fld > 0)
            GR[R1fld][Grp] = GR[0][Grp];       /* Save addr next seq instr. */
         if (R2fld > 0)
            GR[0][Grp] = w_byte;               /* New IAR */
         break;
   }

   switch (opcode & 0x880F) {
      case (0x000C):
         /* IN   R,E            [RE]  */
         /* 01234567 89012345
            0EEE0RRR EEEE1100         */
         Grp = RegGrp(lvl);
         Efld = (opcode0  & 0x70) | (opcode1 >> 4);
         Rfld = (opcode0) & 0x007;             /* Extract register nr */

         if (lvl == 5) {   // && (test_mode == OFF)) {
            IO_L5_chk = ON;                    /* Check: I/O instr in level 5 ! */
            break;
         }
         if (Efld < 0x20) {                    /* Input from GR's ? */
            GR[Rfld][Grp] = GR[Efld & 0x007][Efld >> 3];
         } else {
            // An Input x'40' will reset L2 req
            if ((Efld == 0x40) && (lvl == 2)) {
               Eregs_Inp[0x40] = abar;         /* Moved - Echo abar */
               Eregs_Inp[0x77] &= ~0x4000;     /* Reset L2 flag */
               svc_req_L2 = OFF;               /* Reset L2 request flag */
            }
            if ((Efld >= 0x40) && Efld <= 0x47) {   // Addressing CS2 ICW regs ?
               // ICW Input register ===> Eregs_Out 44, 45, 46, 47
               Get_ICW(abar);                       // update ICW inpur regs
            }

            if (Efld == 0x44) {                     // NCP has read received byte
               if (icw_pcf[0] == 0x07)              // TEMP PDF is now empty for next rx
                  icw_pdf_reg = EMPTY;
            }
            if (Efld == 0x50) {                     // Get INCWAR ?
               Eregs_Inp[0x50] = Eregs_Out[0x50];   // Load INCWAR as used by CA
            }
            if (Efld == 0x51) {                     // Get OUTCWAR ?
               Eregs_Inp[0x51] = Eregs_Out[0x51];   // Load OUTCWAR as used by CA
            }

// ***      Eregs_Inp[0x70]  = 0x4000;         // 64kbyte storage size (3704)
            Eregs_Inp[0x70]  = 0x0400;   // 64kbyte storage size (3705)
            Eregs_Inp[0x70] |= 0x0001;   // 20 bits extended addr installed

// ***      Eregs_Inp[0x71] is updated by panel.c
// ***      Eregs_Inp[0x72] is updated by panel.c

            Eregs_Inp[0x74]  = LAR;      // Update LAR

// ***      Eregs_Inp[0x77] is updated by chan.T1.c

            Eregs_Inp[0x79]  = 0x0000;   // Reset all bits in reg 0x79
            Eregs_Inp[0x79] |= 0x0008;   // Fet storage installed
// ***      Eregs_Inp[0x79] |= 0x0004;   // 0 = 3705, 1 = 3704
            Eregs_Inp[0x79] |= 0x0001;   // CE IPL escape jumper NOT installed
            if (CL_C[3] == ON) Eregs_Inp[0x79] |= 0x0200;  // L5 C & Z flags
            if (CL_Z[3] == ON) Eregs_Inp[0x79] |= 0x0100;

            Eregs_Inp[0x7B] = 0x0000;    // Good BSC CRC.
            Eregs_Inp[0x7C] = 0xF0B8;    // Good SDLC CRC.

            Eregs_Inp[0x7E] = 0x0000;    // Reset all bits in reg 0x7E
            if (adr_ex_chk)  Eregs_Inp[0x7E]  |= 0x0040;   // Address exception check
            if (IO_L5_chk)   Eregs_Inp[0x7E]  |= 0x0020;   // I/O instr in L5
            if (OP_reg_chk)  Eregs_Inp[0x7E]  |= 0x0008;   // OPC check
            if (ipl_req_L1)  Eregs_Inp[0x7E]  |= 0x0002;   // IPL L1 request

            Eregs_Inp[0x7F] &= 0x0204;    // Reset bits in reg 0x7F
            if (diag_req_L2) Eregs_Inp[0x7F]  |= 0x8000;   // Diagnostic L2 request
            //if (inter_req_L3) Eregs_Inp[0x7F] |= 0x0200;   // Panel Interrupt L3
            if (pci_req_L4) Eregs_Inp[0x7F]   |= 0x0100;   // PCI L4 request
            //if (timer_req_L3) Eregs_Inp[0x7F] |= 0x0004;   // Interval timer L3 request
            if (pci_req_L3) Eregs_Inp[0x7F]   |= 0x0002;   // PCI L3 request
            if (svc_req_L4) Eregs_Inp[0x7F]   |= 0x0001;   // SVC L4 request

            GR[Rfld][Grp] = Eregs_Inp[Efld];      // <<==== !!!!
         }
         break;

      case (0x0004):
         /* OUT  R,E            [RE]  */
         /* 01234567 89012345
            0EEE0RRR EEEE0100         */
         Grp = RegGrp(lvl);
         Efld = (opcode0  & 0x70) | (opcode1 >> 4);
         Rfld = (opcode0) & 0x007;             /* Extract register nr */

         if (lvl == 5) {
            IO_L5_chk = ON;            // I/O instr in L5
            break;
         }
         if (Efld < 0x20) {            // Output to GR's ?
            if (Rfld == 0) break;      // Only regen of regs parity
            GR[Efld & 0x007][Efld >> 3] = GR[Rfld][Grp];
         } else {
            Eregs_Out[Efld] = GR[Rfld][Grp];   // <<==== !!! Finally update I/O reg.

#if 0  // <=== !!!

            if ((Efld >= 0x40) && (Efld <= 0x47)) {
               // Eregs_Out 44, 45, 46, 47 ===> ICW Local Store
               Get_ICW(0x20);          // abar must be temp 0x20
               Eregs_Out[Efld] = GR[Rfld][Grp];  // Update any ICW byte
               Put_ICW(0x20);          // abar must be temp 0x20
            }
#endif
            //********************************************************
            //          Communication Scanner Type 2 ICW updates
            //********************************************************
            if ((Efld >= 0x40) && Efld <= 0x47) {  // Addressing CS2 ICW regs ?
               // Obtain ICW update lock
               pthread_mutex_lock(&icw_lock);

               if ((Efld == 0x40) && ((lvl == 3) || (lvl == 4))) {
                  // Update ABAR CS2 and update ICW[ABAR] (only when in L3 or L4).
                  abar = Eregs_Out[0x40];
                  tbar = (abar - 0x0840) >> 1; // Get ICW table ptr from abar
                  //debug_reg = 0x63;                 // Very very very temp HJS
               }
               if (Efld == 0x44) {             // ICW SCF & PDF
                  icw_scf[tbar] = (Eregs_Out[0x44] >> 8) & 0x4E;   // Only Serv Req, DCD & Pgm Flag
                  icw_pdf[tbar] =  Eregs_Out[0x44] & 0x00FF;
                  if (icw_pcf[0] != 0x07)               // TEMP
                     icw_pdf_reg = FILLED;     // PDF is filled for tx
               }
               if (Efld == 0x45) {             // ICW LCD & PCF
                  icw_lcd[tbar] = (Eregs_Out[0x45] >> 4) & 0x0F;
                  icw_pcf_new =  Eregs_Out[0x45] & 0x0F;
                  icw_pcf_mod = 0x01;          // indicate pcf updated
               }
                                               // ICW SDF
               if (Efld == 0x46) icw_sdf[tbar]    = (Eregs_Out[0x46] >> 2) & 0xFF;
                                               // ICW 34 - 45
               if (Efld == 0x47) icw_Rflags[tbar] = (Eregs_Out[0x47] << 4) & 0x0070;
               // Release ICW update lock.
               pthread_mutex_unlock(&icw_lock);
            }

            //********************************************************
            //          Channel Adaptor Type 2 updates
            //********************************************************
            if (Efld == 0x53) {                // Channel Adapter Sense
               if (Eregs_Out[0x53] & 0xFFFF)   // If any bit set...
                  Eregs_Out[0x54] |= 0x0500;   // ...set Unit Check and Device End
            }
            if (Efld == 0x56) {                // Channel Adapter Mode
               if (Eregs_Out[0x56] & 0x2000) {
                  Eregs_Inp[0x55] &= ~0x2000;  // Reset INCWAR valid
                  Eregs_Out[0x55] &= ~0x2000;  // Reset INCWAR valid
                  }
               if (Eregs_Out[0x56] & 0x1000) {
                  Eregs_Inp[0x55] &= ~0x1000;  // Reset OUTCWAR valid
                  Eregs_Out[0x55] &= ~0x1000;  // Reset OUTCWAR valid
                  }
               }

            if (Efld == 0x57) {                // Channel Adapter Mode
               if (Eregs_Out[0x57] & 0x0010) { // Reset CA L3 interrupt
                  pthread_mutex_lock(&r77_lock);
                  Eregs_Inp[0x77] &= ~0x0028;  // Reset CA L3  interrupt
                  pthread_mutex_unlock(&r77_lock);
                  CA1_IS_req_L3 = OFF;
                  CA1_DS_req_L3 = OFF;
               }
               if (Eregs_Out[0x57] & 0x0008) { // Test for CA select
                  Eregs_Inp[0x55] |= 0x0001;   // Select CA1
                  Eregs_Inp[0x55] &= ~0x0002;  // deselect CA2
               }  else {
                  Eregs_Inp[0x55] |= 0x0002;   // Select CA2
                  Eregs_Inp[0x55] &= ~0x0001;  // deselect CA1
               }
               if (Eregs_Out[0x57] & 0x0100) { // Test for IPL required
                  Eregs_Out[0x53] |= 0x0200;   // Set not initialized sense
               }
               if (Eregs_Out[0x57] & 0x0004) {
                  Eregs_Inp[0x55] &= ~0x0010;  // Reset reset flag
               }
               if (Eregs_Out[0x57] & 0x0002) {
                  Eregs_Inp[0x55] &= ~0x0020;  // Reset channel stop
               }
            }

            //********************************************************
            //          Channel Adaptor Type 1 updates
            //********************************************************
            if (Efld == 0x62) {
               Eregs_Inp[0x62] &= ~0x0100;     // Reset PCI interrupt

               if (Eregs_Out[0x62] & 0x0400) { // Reset CA1 L3 interrupts
                 pthread_mutex_lock(&r77_lock);
                  Eregs_Inp[0x77] &= ~0x0008;  // Reset L3 initial selection
                  pthread_mutex_unlock(&r77_lock);
                  CA1_IS_req_L3 = OFF;
                  Eregs_Inp[0x60] &= ~0x8200;  // Reset NSC status bits
               }
               if (Eregs_Out[0x62] & 0x0200) { // Reset CA1 L3 data service
                  pthread_mutex_lock(&r77_lock);
                  Eregs_Inp[0x77] &= ~0x0010;  // Reset L3 data service
                  pthread_mutex_unlock(&r77_lock);
                  CA1_DS_req_L3 = OFF;
               }
               if (Eregs_Out[0x62] & 0x1000)
                  Eregs_Inp[0x62] |= 0x1000;   // Set NSC Channel end
               else
                  Eregs_Inp[0x62] &= ~0x1000;  // Reset NSC Channel end

               if (Eregs_Out[0x62] & 0x0800)
                  Eregs_Inp[0x62] |= 0x0800;   // Set NSC Final status
               else
                  Eregs_Inp[0x62] &= ~0x0800;  // Reset NSC Final status
            }

            //********************************************************
            //          CCU updates
            //********************************************************
            if (Efld == 0x70) {                // HARD STOP
               printf("\nDisplay Reg 1: %05X\n\r", Eregs_Out[0x71]);
               printf(  "Display Reg 2: %05X\n\r", Eregs_Out[0x72]);
               pgm_stop = ON;
               reason = SCPE_STOP;
               continue;
            }
            if (Efld == 0x77) {                // Miscellaneous Control
               w_byte = Eregs_Out[Efld];
               if (w_byte & 0x8000)  {         // Reset IPL L1 ?
                  Eregs_Inp[0x53] &= ~0x0200;  // Reset not-initialized flag
                  Eregs_Out[0x53] &= ~0x0200;  // Reset not-initialized flag
                  ipl_req_L1 = OFF;
               }
               if (w_byte & 0x0004)      // Reset all L1 prgm checks
                  IO_L5_chk = OP_reg_chk = adr_ex_chk = OFF;
               if (w_byte & 0x2000)  {    // Reset Panel Interrupt L3 ?
                     pthread_mutex_lock(&r7f_lock);
                     Eregs_Inp[0x7F] &= ~0x0200;  // Reset L3 Panel Interrupt
                     pthread_mutex_unlock(&r7f_lock);
                     inter_req_L3 = OFF;
                     Eregs_Out[0x77] &= ~0x2000;       //Reset L3 Panel Reset
                  }
                  inter_req_L3 = OFF;
               if ((w_byte &0x0200) && (test_mode))  // Set Diagnostic mode L2 ?
                  diag_req_L2 = ON;
               if ((w_byte &0x0100) && (test_mode))  // Reset Diagnostic mode L2 ?
                  diag_req_L2 = OFF;
               if (w_byte & 0x0040)  {    // Reset Interval Timer L3 ?
                     pthread_mutex_lock(&r7f_lock);
                     Eregs_Inp[0x7F] &= ~0x0004;  // Reset L3 Interval Timer
                     pthread_mutex_unlock(&r7f_lock);
                     timer_req_L3 = OFF;
                     Eregs_Out[0x77] &= ~0x0040;       //Reset L3 Interval Reset
                  }
               if (w_byte & 0x0020)      // Reset PCI L3 ?
                  pci_req_L3 = OFF;
               if (w_byte & 0x0002)      // Reset PCI L4 ?
                  pci_req_L4 = OFF;
               if (w_byte & 0x0001)      // Reset SVC L4 ?
                  svc_req_L4 = OFF;
            }
            if (Efld == 0x79) {          // Utility Control
               if (!(Eregs_Out[Efld] & 0x0400)) { // Inhibit bit PL5 C&Z flag off ?
                  if (Eregs_Out[Efld] & 0x0200)   // Prog L5 C flag
                     CL_C[3] = ON;
                  else
                     CL_C[3] = OFF;
                  if (Eregs_Out[Efld] & 0x0100)   // Prog L5 Z flag
                     CL_Z[3] = ON;
                  else
                     CL_Z[3] = OFF;
               }
               if (Eregs_Out[Efld] & 0x0040)   // Reset load state
                  load_state = OFF;
               if (Eregs_Out[Efld] & 0x0020)   // Set test mode
                  test_mode = ON;
               if (Eregs_Out[Efld] & 0x0002)   // Set test mode
                  test_mode = ON;
               if (Eregs_Out[Efld] & 0x0010)   // Reset test mode
                  test_mode = OFF;
            }
            if (Efld == 0x7C) {                // Program Call Interrupt L3
               pci_req_L3     = ON;
            }
            if (Efld == 0x7D) {                // Program Call Interrupt L4
               pci_req_L4     = ON;
            }
            if (Efld == 0x7E) {                // Set interrupt mask bits
               w_byte = Eregs_Out[Efld];
               if (w_byte & 0x0020)            // Level 2 ?
                  int_lvl_mask[2] = ON;
               if (w_byte & 0x0010)            // Level 3 ?
                  int_lvl_mask[3] = ON;
               if (w_byte & 0x0008)            // Level 4 ?
                  int_lvl_mask[4] = ON;
               if (w_byte & 0x0004)            // Level 5 ?
                  int_lvl_mask[5] = ON;
            }
            if (Efld == 0x7F) {                // Reset interrupt mask bits
               w_byte = Eregs_Out[Efld];
               if (w_byte & 0x0020)            // Level 2 ?
                  int_lvl_mask[2] = OFF;
               if (w_byte & 0x0010)            // Level 3 ?
                  int_lvl_mask[3] = OFF;
               if (w_byte & 0x0008)            // Level 4 ?
                  int_lvl_mask[4] = OFF;
               if (w_byte & 0x0004)            // Level 5 ?
                  int_lvl_mask[5] = OFF;
            }
         }
         break;
   }

   switch (opcode & 0xF8F0) {
      case (0xB800):
         /* BAL  R,A            [RA]  */
         /* 01234567 89012345 ... 901
            10111RRR 0000A<-- // -->A */
         Grp = RegGrp(lvl);
         Rfld = (opcode0) & 0x07;              /* Extract register nr */
                                               /* Get branch addr from memory */
         Afld = (opcode1 & 0x03) << 16;        /* Xbyte EA18 */
         Afld = Afld | (GetMem(PC) << 8);      /* Read 3rd & 4th byte */
         PC = (PC + 1) & AMASK;
         Afld = Afld |  GetMem(PC);
         PC = (PC + 1) & AMASK;

         if (Rfld > 0)                         /* No link addr if R=0 */
            GR[Rfld][Grp] = PC;                /* Store link address */
         GR[0][Grp] = Afld;                    /* Unconditional branch */
         break;

      case (0xB820):
         /* LA   R,A            [RA]  */
         /* 01234567 89012345 ... 901
            10111RRR 0010A<-- // -->A */
         Grp = RegGrp(lvl);
         Rfld = (opcode0) & 0x007;             /* Extract register nr */
                                               /* Get load address from memory */
         Afld = (opcode1 & 0x03) << 16;        /* Xbyte EA18 */
         Afld = Afld | (GetMem(PC) << 8);      /* Read 3rd & 4th byte */
         PC = (PC + 1) & AMASK;
         Afld = Afld |  GetMem(PC);
         PC = (PC + 1) & AMASK;
         GR[0][Grp] = PC;                      /* Update IAR */
         GR[Rfld][Grp] = Afld;                 /* Load R with 16 bit address */
         break;
   }

   if (opcode == 0xB840) {
      /* EXIT                EXIT  */
      /* 01234567 89012345
         10111000 01000000         */

      int_lvl_ent[lvl] = OFF;                  /* Reset current active PGM level */
      if (lvl == 5) {                          /* An EXIT while in L5 triggers SVC L4 */
         svc_req_L4 = ON;
      }
      if (debug_reg & 0x02)
         fprintf(trace, "\n>>> Leaving lvl=%d \n", lvl);
   }
   //if (debug_reg == 0x80) {                    /* Extra delay ? */
     // usleep(250);
  // }
}  /* end while (reason == 0) */

//###################### END OF SIMULATOR WHILE LOOP ######################

PC = saved_PC;
/* Simulation halted */
return (reason);
}

//********************************************************
// Sub-routines used by the simulator
//********************************************************

/*** Select register group ***/

int32 RegGrp(int32 level)
{                            // Lvl 1 => Reg Grp 0
   if (level == 1)           // Lvl 2 => Reg Grp 0
      return(0);             // Lvl 3 => Reg Grp 1
   else                      // Lvl 4 => Reg Grp 2
      return(level - 2);     // Lvl 5 => Reg Grp 3
}

/*** Fetch a byte from memory ***/

int32 GetMem(int32 addr)
{
   if (addr > MEMSIZE) {
       adr_ex_chk = ON;       // Addressing Exception ?
        printf("Addr %d  MEMSIZE %d ... \n\r",addr, MEMSIZE);
    }
   else
      return(M[addr] & 0xFF);
}

/*** Place a byte in memory ***/

int32 PutMem(int32 addr, int32 data)
{
   if (addr > MEMSIZE) {
      adr_ex_chk = ON;       // Addressing Exception ?
         printf("Addr %d  MEMSIZE %d ... \n\r",addr, MEMSIZE);
      }
   else
      M[addr] = data & 0xFF;
   return 0;
}

/*** Memory examine ***/

t_stat cpu_ex (t_value *vptr, t_addr addr, UNIT *uptr, int32 sw) {
   if (addr >= MEMSIZE) return SCPE_NXM;
   if (vptr != NULL) *vptr = M[addr] & 0xff;
   return SCPE_OK;
}

/*** Memory deposit ***/

t_stat cpu_dep (t_value val, t_addr addr, UNIT *uptr, int32 sw) {
   if (addr >= MEMSIZE) return SCPE_NXM;
   M[addr] = val & 0xFF;
   return SCPE_OK;
}

t_stat cpu_set_size (UNIT *uptr, int32 val, char *cptr, void *desc) {
   int32 mc = 0;
   uint32 i;

   if ((val <= 0) || (val > MAXMEMSIZE) || ((val & 0x0FFF) != 0))
      return SCPE_ARG;
   for (i = val; i < MEMSIZE; i++) mc = mc | M[i];
   if ((mc != 0) && (!get_yn ("Really truncate memory [N]?", FALSE)))
      return SCPE_OK;
   MEMSIZE = val;
   for (i = MEMSIZE; i < MAXMEMSIZE; i++) M[i] = 0x00;
   return SCPE_OK;
}

/*** BOOT/LOAD procedure ***/

t_stat cpu_boot (int32 unitno, DEVICE *dptr) {    /* LOAD pressed */
   //******************************************************************
   // IPL phase 2 - Transfer miniROS to core storage
   //******************************************************************
   int32 addr, temp;

   load_state = ON;

   char miniROS[] = {
   /*000   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F */
   "\x70\x04\xF6\xFF\x98\xB8\x80\xCE\x81\x0B\x86\xFF\x87\xFF\xA8\xA2"
   "\x00\x82\x00\x14\x01\x86\x00\x24\x02\x8A\x00\x34\x03\x8E\x00\x44"
   "\x04\x92\x00\x54\x05\x96\x00\x64\x06\x9A\x00\x74\x07\x9E\xA8\x04"
   "\x70\x04\x00\x00\x80\x00\x98\x09\x81\x00\x98\x0D\xD1\x00\x91\x00"
   "\xF1\xFF\x98\x15\xD1\xFF\x91\xFF\xF1\x01\x98\x1D\xF0\x01\x88\x21"
   "\xD0\xFF\x90\xFF\x98\x02\xFF\xFF\x80\x00\xD0\x00\x90\x00\xF0\xFF"
   "\x98\x33\x77\xC8\x71\x57\x05\x1C\x01\x34\x53\xC8\x98\x3F\x73\xC8"
   "\x98\x43\x17\xC8\x03\x54\x01\x3C\x15\xC8\x98\x4D\x80\x07\x17\x85"
   "\x13\x05\x73\xC8\x98\x57\x15\x85\x13\x05\x53\xC8\x98\x5F\x17\x86"
   "\x13\x05\x53\xC8\x98\x67\x15\x07\x75\xC8\x98\x6D\x15\x86\x07\x07"
   "\x75\xC8\x98\x75\x80\xCE\x81\x06\x83\xB4\xA8\x06\x91\x80\x98\xAF"
   "\xD0\x0E\x31\x81\xCE\x06\xF6\xFF\x98\x02\x88\x11\x70\x04\x81\x74"
   "\x83\xC8\x91\x10\xF8\x86\x31\x81\x00\x84\xA8\x0B\xD8\x84\x80\x10"
   "\xA8\x0D\x71\x9C\xF9\x82\xAE\x24\x71\xDC\x01\x85\x71\x6C\x01\x83"
   "\x71\xEC\x01\x87\xF9\x02\x70\x04\x33\xC8\x81\x10\x71\x94\x81\x08"
   "\x61\x74\x61\x7C\xE9\x02\xA8\x11\x86\x02\xE9\xB4\x87\x06\x81\x60"
   /*100   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F */
   "\x61\x74\x82\x0F\x71\x6C\xE8\x88\x71\x7C\xD9\x90\xE9\x36\xA8\x0D"
   "\x86\x12\x61\x7C\xE9\x9E\x81\x20\x61\x74\xA8\x08\x67\x64\x61\x7C"
   "\x61\x34\x63\x24\x71\x6C\xE8\x99\x71\x7C\xE9\x18\xD9\xFC\xA8\x0D"
   "\x71\x7C\xE9\x04\x87\x0E\xA8\x3B\x61\x1C\x65\x7C\x81\x00\x85\x00"
   "\x15\xC8\x98\x11\x82\x06\x61\x0C\xF8\xD8\xC8\xCC\xF8\x31\xE8\xB6"
   "\xD8\xAA\xC8\x08\x86\x12\x87\x0E\xD2\x08\xA8\x41\x61\x1C\x80\xFF"
   "\x91\xF6\x98\x14\x91\x01\x98\x0C\x91\x03\x98\x0C\x91\x01\x98\x58"
   "\x91\x01\x98\x1C\x86\x02\xA8\x23\x86\x82\xA8\x27\x86\x22\xA8\x63"
   "\x86\x22\x87\x02\xA8\x2F\xD8\x89\x61\x1C\xF1\xFF\x88\x2E\xA8\x28"
   "\x67\x44\xD2\x80\x83\x01\xA8\x7B\x61\x1C\x91\xFB\xF1\xFF\x88\x44"
   "\xA8\x2F\x86\x02\x81\x10\x61\x74\xA8\x87\x82\x02\x61\x2C\xD9\x8C"
   "\xE8\x92\xC9\x0C\xE8\x21\xC8\x9C\x87\x0C\xA8\x65\xD2\x08\xA8\xA3"
   "\x86\x22\xA8\x6F\xC8\x9E\xA8\x55\x84\x04\x85\x00\x55\x83\xD2\x40"
   "\x83\x02\xA8\xB7\xEC\x08\x61\x4C\x51\x81\x95\x02\xA8\x11\x86\x02"
   "\x87\x0F\xA8\x8D\x80\x04\x81\x02\x11\x01\x90\x04\x51\xC8\x98\x13"
   "\x80\xC0\x71\x74\x80\x12\x61\x24\x80\x01\x81\xFE\x10\x01\x04\x04"
   };

 char maxiROS[] = {
   /*000   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F */
   "\x70\x04\xF6\xFF\x98\xB8\x80\xCE\x81\x0B\x86\xFF\x87\xFF\xA8\xA2"
   "\x00\x82\x00\x14\x01\x86\x00\x24\x02\x8A\x00\x34\x03\x8E\x00\x44"
   "\x04\x92\x00\x54\x05\x96\x00\x64\x06\x9A\x00\x74\x07\x9E\xA8\x04"
   "\x70\x04\x00\x00\x80\x00\x98\x09\x81\x00\x98\x0D\xD1\x00\x91\x00"
   "\xF1\xFF\x98\x15\xD1\xFF\x91\xFF\xF1\x01\x98\x1D\xF0\x01\x88\x21"
   "\xD0\xFF\x90\xFF\x98\x02\xFF\xFF\x80\x00\xD0\x00\x90\x00\xF0\xFF"
   "\x98\x33\x77\xC8\x71\x57\x05\x1C\x01\x34\x53\xC8\x98\x3F\x73\xC8"
   "\x98\x43\x17\xC8\x03\x54\x01\x3C\x15\xC8\x98\x4D\x80\x07\x17\x85"
   "\x13\x05\x73\xC8\x98\x57\x15\x85\x13\x05\x53\xC8\x98\x5F\x17\x86"
   "\x13\x05\x53\xC8\x98\x67\x15\x07\x75\xC8\x98\x6D\x15\x86\x07\x07"
   "\x75\xC8\x98\x75\x80\xCE\x81\x06\x83\xB4\xA8\x06\x91\x80\x98\xAF"
   "\xD0\x0E\x31\x81\xCE\x06\xF6\xFF\x98\x02\x88\x11\x70\x04\x81\x74"
   "\x83\xC8\x91\x10\xF8\x86\x31\x81\x00\x84\xA8\x0B\xD8\x84\x80\x10"
   "\xA8\x0D\x71\x9C\xF9\x82\xAE\x24\x71\xDC\x01\x85\x71\x6C\x01\x83"
   "\x71\xEC\x01\x87\xF9\x02\x70\x04\x11\xC8\x80\x40\x71\x74\x75\xC8"
   "\x35\xC8\x33\xC8\x73\x14\x73\x24\x83\x10\x73\x94\x77\xC8\x87\x88"
   /*000   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F */
   "\x57\x74\x77\xC8\x71\x6C\xE8\xA0\x71\x7C\xE9\x06\x83\x04\x73\x24"
   "\xA8\x17\x51\x5C\xF9\x86\x83\x01\x73\x24\xA8\x21\x51\x8C\xE9\x0E"
   "\xE9\x8C\x83\x02\x73\x24\xA8\x0D\x87\x28\x57\x74\xA8\x33\x51\x5C"
   "\xF8\x84\x86\x19\xA8\x1E\x51\xCC\xF9\x84\x86\x59\xA8\x16\x01\x01"
   "\xF8\x89\x51\x5C\xD9\x84\xD9\x0F\xA8\x10\xD7\x3E\xD6\x01\xA8\x0A"
   "\xD7\x3A\xA8\x06\x51\x5C\xD9\x8F\xD9\x0B\x11\xC8\x01\x81\xD7\x38"
   "\xF6\xFF\x88\x04\x57\x74\x77\xC8\x83\x08\x73\x24\x71\x6C\xE8\xA5"
   "\x71\x7C\xE9\x02\xA8\x0F\x51\x5C\xD9\xB1\xD9\x4C\x51\xCC\xF9\x82"
   "\xA8\x37\x80\x01\x81\xFA\x01\x81\x51\x04\x82\x04\x83\x02\x33\x81"
   "\x87\x18\x57\x74\x73\x24\x71\x6C\xE8\xCF\x71\x7C\xE9\x02\xA8\x0D"
   "\x51\x5C\xD9\xDB\xD9\x02\xA8\x48\x11\xC8\x71\x24\x51\x2C\x82\xFF"
   "\x83\xFF\x90\xFC\x31\xC8\x82\x04\x83\x02\x33\x01\x93\x01\x31\xC8"
   "\x88\x0A\x11\xC8\x80\x01\x51\x44\x86\x59\xA8\x7D\x51\x9C\x91\x01"
   "\x92\x04\x31\xC8\x98\x15\x01\x81\x75\xC8\x15\xC8\x80\x01\x81\xF8"
   "\x13\x01\x35\xC8\x88\x04\x70\x04\x00\x00\x84\x80\x75\x74\x10\x07"
   "\xD6\x58\xA8\xA9\xA8\x00\xA8\x00\xFF\x2F\x8F\xF8\x04\x00\x04\x04"
   };

   if (Eregs_Inp[0x79] & 0x0002) {
      /* If CA Type 1 or 4 Load miniROS at location 0x0000 */
      printf("CPU: Loading MiniROS...\n\r");
      for (addr = 0x0000; addr < 0x0200; addr++) {
         temp = miniROS[addr];
         PutMem(addr, temp);
      }
   } else {
      /* If CA Type 2 or 3 Load maxiROS at location 0x0000 */
      printf("CPU: Loading MaxiROS...\n\r");
      for (addr = 0x0000; addr < 0x0200; addr++) {
         temp = maxiROS[addr];
         PutMem(addr, temp);
      }
   }

   /* Load IFL3705E at location 0x0700 */

   //******************************************************************
   // IPL phase 3 - Bootstrap load
   //******************************************************************
   printf("CPU: Booting... \n\r");
   int_lvl_mask[1] = OFF;                      /* Allow pgm level 1 */
   ipl_req_L1 = ON;                            /* Request L1 for IPL */
   return SCPE_OK;
}

/*** RESET pressed procedure ***/

t_stat cpu_reset (DEVICE *dptr) {              /* RESET pressed */
   //******************************************************************
   // IPL phase 1 - CCU reset procedure
   //******************************************************************
   int32 i;
   sim_brk_types = sim_brk_dflt = SWMASK ('E');  /* Clear all BP's */

   /* Clear all level GP registers */
   GR[0][0] = 0x00000;  GR[1][0] = 0x00000;  GR[2][0] = 0x00000;  GR[3][0] = 0x00000;
   GR[4][0] = 0x00000;  GR[5][0] = 0x00000;  GR[6][0] = 0x00000;  GR[7][0] = 0x00000;

   /* Set/reset HARD STOP, PGM STOP, IPL LATCHES 1 & 2, TEST MODE */
   test_mode = ON;                             /* See PoO 5-11 */
   pgm_stop  = OFF;
   load_state = OFF;
   wait_state = OFF;
   OP_reg_chk = OFF;
   IO_L5_chk = OFF;

   /* Reset all interrupt level flags */
   for (i = 0; i < 6; i++) {
      int_lvl_req[i]  = OFF;                   /* Reset all Pgm Level request */
      int_lvl_ent[i]  = OFF;                   /* Reset all "Interrupt Entered" */
      int_lvl_mask[i] = ON;                    /* Set all Pgm Level masks */
   }
   lvl = 5;

   printf("CPU: Reset... \n\r");
   printf("CPU: MEMORYSIZE %d bytes... \n\r", MEMSIZE);

   return SCPE_OK;
}
