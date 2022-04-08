# Supporting BSC

BSC is the predecessor of SDLC for remote communication with terminals. This branch is for adapting the IBM 3705 emulator to BSC.

## Changes to the configuration to MVS running in Hercules

Som random notes for now.

Example for CLUSTER and TERMINAL macro
```

CL100   CLUSTER   CUTYPE=3271,GPOLL=40407F7F,                C
                  XMITLIM=1,FEATUR2=(MODEL2,EDATS),          C
                  MODETAB=MODE6000,ISTATUS=ACTIVE
*------------------------------------------------------*
* Display session example, model 2-5                   *
*------------------------------------------------------*
CL1001  TERMINAL  TERM=3277,ADDR=60604040,                   C
                  POLL=40404040,DLOGMODE=DS23279
                  
```

DLOGMODE, MODETAB, FEATUR2 and ISTATUS is VTAM configuration option and is not acted upon by the NCP Gen process.

USSTAB=ISTNSNA
DLOGMOD=S3270
USSTAB=VUSS3270
MODETAB=NOSPTAB
LOGTAB=TABLE01
LOGAPPL=NETSOL
PU=YES
```
* GROUP SPECIFICATION FOR REMOTE 3270'S *
BSC3270   GROUP   DIAL=NO,NON-SWITCHED LINES X
                  CRETRY=7,      TIME OUT WILL TAKE ABOUT 63 SECONDS X
                  LNCTL=BSC,BSC LINE CONTROL X
*** TRANSFR times the BFRS value in the BUILD macro should be a value
*** greater than 256 but not exceed ^00.
                  TRANSFR=5,     LIMIT NUMBER OF RECEIVE BUFFERS
*** CUT0FF=1 is required for 3270 BSC cluster control units.
                  CUT0FF=1,      LIMIT NUMBER OF SUBBLOCKS
                  TYPE=NCP,      FOR USE BY NCR ONLY
*** XMITLIM=1 is required for BSC 3270 Clusters.
                  XMITLIM=1,     TRANSMISSION LIMIT
                  WACKCNT=15,    WACK COUNT OF 15
                  WAKDLAY=2.2,   WACK DELAY OF 2.2 SEC
                  SYNDLAY=1,     SYNC INTERVAL RATE
                  REPLYT0=3      REPLY TIME OUT
```


```
*
* VTAM configuration statements for graphics devices attached
* to remote BSC control units
*
BSC79M3        TERMINAL TERM=3277,         A 3277 terminal X
            ADDR=6060C1C1, Installation specific to port # X
            POLL=4040C1C1,                                 X
            FEATUR2=(MODEL2,EDATS),      <== Color display X
            USSTAB=USSTAB00,            USS table for VTAM X
            MODETAB=MT3274,                 EDS mode table X
            DLOGMOD=L3279M3                 A model 3 3279

```


TK4- 
* VTAM LOGMODE TABLE SYS1.VTAMSRC(BSPLMT01) - ISTINCLM
* VTAM LOGON INTERPRET TABLE SYS1.VTAMSRC(BSPLIN01)
* VTAM USS TABLE FOR TK4- SYS1.VTAMSRC(BSPUDT01) ISTINDT

### BSPLMT01
```
**************************************************************
*                                                            *
* NAME: BSPLMT01                                             *
*                                                            *
* TYPE: ASSEMBLER SOURCE                                     *
*                                                            *
* DESC: VTAM LOGMODE TABLE                                   *
*                                                            *
**************************************************************
BSPLMT01 MODETAB
*****************************************************************
* NON-SNA 3270 LOCAL TERMINALS                                  *
*      PRIMARY SCREEN   : MODEL 2                               *
*      SECONDARY SCREEN : NON                                   *
*****************************************************************
S3270    MODEENT LOGMODE=S3270,                                        X
               FMPROF=X'02',                                           X
               TSPROF=X'02',                                           X
               PRIPROT=X'71',                                          X
               SECPROT=X'40',                                          X
               COMPROT=X'2000',                                        X
               PSERVIC=X'000000000000000000000200'
*****************************************************************
* NON-SNA 3270 LOCAL TERMINALS                                  *
*      PRIMARY SCREEN   : MODEL 5                               *
*      SECONDARY SCREEN : NON                                   *
*****************************************************************
S32785   MODEENT LOGMODE=S32785,                                       X
               FMPROF=X'02',                                           X
               TSPROF=X'02',                                           X
               PRIPROT=X'71',                                          X
               SECPROT=X'40',                                          X
               COMPROT=X'2000',                                        X
               PSERVIC=X'00000000000018501B847F00'
*****************************************************************
* 3274 MODEL 1C WITH MODEL 2 SCREEN (REMOTE SNA)                *
*      PRIMARY SCREEN   : MODEL 2                               *
*      SECONDARY SCREEN : NON                                   *
*****************************************************************
D4C32782 MODEENT LOGMODE=D4C32782,                                     X
               FMPROF=X'03',                                           X
               TSPROF=X'03',                                           X
               PRIPROT=X'B1',                                          X
               SECPROT=X'90',                                          X
               COMPROT=X'3080',                                        X
               RUSIZES=X'87F8',                                        X
               PSERVIC=X'020000000000185020507F00'
*****************************************************************
*      3276 SNA WITH MODEL 2 SCREEN (REMOTE SNA)                *
*      PRIMARY SCREEN   : MODEL 2                               *
*      SECONDARY SCREEN : NON                                   *
*****************************************************************
D6327802 MODEENT LOGMODE=D6327802,                                     X
               FMPROF=X'03',                                           X
               TSPROF=X'03',                                           X
               PRIPROT=X'B1',                                          X
               SECPROT=X'90',                                          X
               COMPROT=X'3080',                                        X
               RUSIZES=X'88F8',                                        X
               PSERVIC=X'020000000000185000007E00'
*****************************************************************
*      3274 1C SNA WITH MODEL 5 SCREEN (REMOTE SNA)             *
*      PRIMARY SCREEN   : MODEL 5                               *
*      SECONDARY SCREEN : NONE                                  *
*****************************************************************
D4C32785 MODEENT LOGMODE=D4C32785,                                     X
               FMPROF=X'03',                                           X
               TSPROF=X'03',                                           X
               PRIPROT=X'B1',                                          X
               SECPROT=X'90',                                          X
               COMPROT=X'3080',                                        X
               RUSIZES=X'87F8',                                        X
               PSERVIC=X'0200000000001B8400007E00'
*****************************************************************
*      3276 SNA WITH MODEL 2 SCREEN (REMOTE SNA) (T.S.O)        *
*      PRIMARY SCREEN   : MODEL 2                               *
*      SECONDARY SCREEN : NON                                   *
*****************************************************************
D63278TS MODEENT LOGMODE=D63278TS,                                     X
               FMPROF=X'03',                                           X
               TSPROF=X'03',                                           X
               PRIPROT=X'B1',                                          X
               SECPROT=X'90',                                          X
               COMPROT=X'3080',                                        X
               RUSIZES=X'8587',                                        X
               PSERVIC=X'020000000000000000000200'
*****************************************************************
*      3276 SNA WITH 3289 MODEL 2 PRINTER                       *
*****************************************************************
D6328902 MODEENT LOGMODE=D6328902,                                     X
               FMPROF=X'03',                                           X
               TSPROF=X'03',                                           X
               PRIPROT=X'B1',                                          X
               SECPROT=X'90',                                          X
               COMPROT=X'3080',                                        X
               RUSIZES=X'8787',                                        X
               PSERVIC=X'030000000000185018507F00'
*****************************************************************
*      3274 NON-SNA  MODEL 2 SCREEN (LOCAL)                     *
*      PRIMARY SCREEN   : MODEL 2                               *
*      SECONDARY SCREEN : NON                                   *
*****************************************************************
D4B32782 MODEENT LOGMODE=D4B32782,                                     X
               FMPROF=X'02',                                           X
               TSPROF=X'02',                                           X
               PRIPROT=X'71',                                          X
               SECPROT=X'40',                                          X
               COMPROT=X'2000',                                        X
               RUSIZES=X'0000',                                        X
               PSERVIC=X'000000000000185000007E00'
*****************************************************************
*     S C S   P R I N T E R                                     *
*****************************************************************
SCS      MODEENT LOGMODE=SCS,                                          X
               FMPROF=X'03',                                           X
               TSPROF=X'03',                                           X
               PRIPROT=X'B1',                                          X
               SECPROT=X'90',                                          X
               COMPROT=X'3080',                                        X
               RUSIZES=X'87C6',                                        X
               PSNDPAC=X'01',                                          X
               SRCVPAC=X'01',                                          X
               PSERVIC=X'01000000E100000000000000'
*****************************************************************
*        N C C F                                                *
*****************************************************************
DSILGMOD MODEENT LOGMODE=DSILGMOD,                                     X
               FMPROF=X'02',                                           X
               TSPROF=X'02',                                           X
               PRIPROT=X'71',                                          X
               SECPROT=X'40',                                          X
               COMPROT=X'2000',                                        X
               RUSIZES=X'0000',                                        X
               PSERVIC=X'000000000000000000000200'
*****************************************************************
*        N C C F                                                *
*****************************************************************
DSIXDMN  MODEENT LOGMODE=DSIXDMN,                                      X
               FMPROF=X'03',                                           X
               TSPROF=X'03',                                           X
               PRIPROT=X'20',                                          X
               SECPROT=X'20',                                          X
               COMPROT=X'4000',                                        X
               RUSIZES=X'0000',                                        X
               PSERVIC=X'000000000000000000000000'
*****************************************************************
*      3276 SNA WITH MODEL 2 SCREEN (MAGNETIC STRIPE READER)    *
*      PRIMARY SCREEN   : MODEL 2                               *
*      SECONDARY SCREEN : NON                                   *
*      TEST TEST TEST TEST TEST TEST                            *
*****************************************************************
SCSLRDR  MODEENT LOGMODE=SCSLRDR,                                      X
               FMPROF=X'03',                                           X
               TSPROF=X'03',                                           X
               PRIPROT=X'B1',                                          X
               SECPROT=X'90',                                          X
               COMPROT=X'3080',                                        X
               RUSIZES=X'87C6',                                        X
               PSNDPAC=X'01',                                          X
               SRCVPAC=X'01',                                          X
               PSERVIC=X'04000000E100000000000000'
         MODEEND
         END
``

### LOGMODE

Specifies the name of a logon mode table to be used to correlate each logon mode name with a set of session parameters for the logical unit. The name you code must be the name of a logon mode table created as described in Logon mode table. If you do not supply a logon mode table for the logical unit on the MODETAB operand, an IBM®-supplied default logon mode table (ISTINCLM) is used. If you specify a table, both the table you specify and the default table are used.

### DLOGMOD

Specifies the name of the logon mode table entry used by default if one is not otherwise provided. If you do not code this operand and the name of a logon mode table entry is not otherwise provided, VTAM® uses the first entry in the applicable logon mode table (specified on the MODETAB operand or used by default).

If you specify MODETAB, the entry must be in either the specified table or in ISTINCLM, an IBM®-supplied logon mode table. If you do not specify MODETAB, the entry must be in ISTINCLM.

A logon mode entry determines which entry in the applicable logon mode table is to provide a set of session parameters for the application program if the application program is a secondary logical unit (SLU). The name specified on the DLOGMOD operand must be the name of an entry in a logon mode table.

### USSTAB

Specifies the name of a USS table that VTAM® uses to process character-coded input that it receives from the logical unit.

A terminal user can issue a USS command by coding the LANGTAB operand. This causes a second USS table to be associated with the logical unit, which overrides the table specified with USSTAB. If you do not code USSTAB and a LANGTAB USS table is not in use, the IBM®-supplied USS table (ISTINCDT) is used. For more information on USS tables, see Unformatted system services tables.

### FEATUR2

#### FEATUR2=EDATS
#### FEATUR2=NOEDATS
Specifies whether this terminal has the extended data stream feature. You cannot use this operand for terminals attached by SDLC lines.
#### FEATUR2=DUALCSE
#### FEATUR2=LOWERCSE
Specifies how VTAM® sends alphabetical characters coded with the TEXT operand on a USSMSG macroinstruction to a non-SNA terminal over the SSCP-LU session. This value does not affect non-alphabetical characters, or any characters coded on the BUFFER operand of a USSMSG macroinstruction.
Code LOWERCSE to indicate that alphabetical characters are sent to the terminal over the SSCP-LU session in lowercase. Code DUALCSE to indicate that VTAM sends all characters as they are coded in the USSMSG macroinstruction.

#### FEATUR2=MODEL1
#### FEATUR2=MODEL2
Identifies the specific model number (Model 1 or 2) for this 3275, 3277, 3284, or 3286 component. Code MODEL1 for those devices that have a default screen or buffer size of 480 bytes. Code MODEL2 for those devices that have a default screen or buffer size of 1920 bytes.
This information is available to an application program as part of the device characteristics pertaining to this terminal. You can obtain those characteristics by using the INQUIRE macroinstruction. For more information on using the INQUIRE macroinstruction, see z/OS Communications Server: SNA Programming.

#### FEATUR2=NOPRINTR
#### FEATUR2=PRINTR
Specifies whether this terminal has an attached IBM® 3284 Model 3 printer. This operand is valid only if TERM=3275.
#### FEATUR2=NOSELPEN
#### FEATUR2=SELPEN
Specifies whether this terminal supports a selector pen.

### MODETAB

Specifies the name of a logon mode table to be used for the logical unit. The name you code must be the name of a logon mode table created as described in Logon mode table. If you do not supply a logon mode table for the logical unit on the MODETAB operand, an IBM®-supplied default logon mode table (ISTINCLM) is used. If you specify a table, both the table you specify and the default table are used.

SYS1.VTAMLST(HJS3705)

```
***********************************************************************
*                                                                     *
*      THIS GENERATION IS FOR AN IBM 3705                             *
*                                                                     *
***********************************************************************
         SPACE 2
***********************************************************************
*      PCCU SPECIFICATIONS - OS/VS (VTAM ONLY)                        *
***********************************************************************
NCPSTART PCCU  CUADDR=660,         3705 CONTROL UNIT ADDRESS           X
               AUTODMP=NO,         PROMPT BEFORE DUMPING NCP           X
               AUTOIPL=YES,        AUTOIPL AND RESTART                 X
               DUMPDS=NCPDUMP,     AUTODUMP REQUESTED                  X
               INITEST=YES         NCP INITIALIZATION TEST
         EJECT
***********************************************************************
*      BUILD MACRO SPECIFICATIONS FOR OS                              *
***********************************************************************
NCPBUILD BUILD MAXSUBA=31,          MUST BE SAME AS IN VTAM STR DEF    X
               LOADLIB=NCPLOAD,     LIBRARY FOR NCP LOAD MODULE        X
               OBJLIB=NCPOBJ1,      LIBRARY FOR ASSEMBLER OUTPUTS      X
               LESIZE=320,          REGION SIZE FOR LINK-EDIT          X
               TYPSYS=OS,           OS USED FOR STAGE 2                X
               QUALIFY=SYS1,        1ST LEVEL QUALIFIER                X
               UNIT=SYSDA,          DATA SET FOR ASSEMBLY              X
               MEMSIZE=64,          3705 STORAGE SIZE IS 64K BYTES     X
               TYPGEN=NCP,          NCP ONLY                           X
               ABEND=YES,           ABEND FACILITY INCLUDED            X
               ANS=YES,             AUTOMATIC NETWORK SHUTDOWN         X
               ASMXREF=NO,          NO ASSEMBLER CROSS-REFERENCE       X
               BFRS=64,             NCP BUFFER SIZE                    X
               CHANTYP=TYPE2,       PRIMARY CHANNEL ADAPTER            X
               ERASE=NO,            DO NOT ERASE BUFFERS (DEFAULT)     X
               ENABLTO=2.2,         LEASED LINE ONLY (DEFAULT)         X
               JOBCARD=MULTI,       JOBCARDS PROVIDED BY NCP GEN       X
               MODEL=3705-2,        .                                  X
               NEWNAME=HJS3705,     NAME OF THIS NCP LOAD MODULE       X
               OLT=YES,             ONLINE TEST AVAILABLE(DEFAULT)     X
               SLODOWN=12,          SLOWDOWN WHEN 12% OF BUFFERS AVAIL X
               SUBAREA=16,          SUBAREA ADDRESS = 3                X
               TRACE=(YES,10)       10 ADDRESS-TRACE ENTRIES
         SPACE 2
***********************************************************************
*      SYSCNTRL OPTIONS FOR VTAM OR TCAM                              *
*      NOTE THAT OPERATOR CONTROLS ARE NOT INCLUDED.                  *
***********************************************************************
NCPSYSC  SYSCNTRL OPTIONS=(MODE,                                       X
               RCNTRL,RCOND,RECMD,RIMM,ENDCALL,                        X
               BHSASSC)
         SPACE 2
***********************************************************************
*      HOST MACRO SPECIFICATIONS OS VTAM                              *
*      UNITSZ TIMES MAXBFRU MINUS BFRPAD EQUALS MAX MESSAGE SIZE      *
*      FOR INBOUND MESSAGES                                           *
***********************************************************************
NCPHOST  HOST  INBFRS=5,           INITIAL 3705 ALLOCATION             X
               MAXBFRU=4,          VTAM BUFFER UNIT ALLOCATION         X
               UNITSZ=4016,        *                                   X
               BFRPAD=28,          VTAM(OS=28, DOS=15, ACF=0), EXTM=2  X
               DELAY=.2,           .2 SECOND ATTENTION DELAY           X
               STATMOD=YES,        YES VTAM, NO FOR EXTM               X
               TIMEOUT=(120.0)     AUTO SHUTDOWN IF NO RESP IN 120SEC
        SPACE  2
***********************************************************************
*      CSB MACRO SPECIFICATIONS                                       *
***********************************************************************
NCPCSB  CSB    SPEED=(1200),       BUS MACH CLOCK                      X
               MOD=0,              SCANNER ADDRESS 020 TO 05F          X
               TYPE=TYPE2          TYPE 2 COMM SCANNER
        EJECT
***********************************************************************
*      SPECIFICATIONS FOR SDLC LEASED LINES                           *
*      GROUP MACRO SPECIFICATIONS                                     *
***********************************************************************
SDLCGPL GROUP  LNCTL=SDLC,         SYNCHRONOUS DATA LINK               X
               DIAL=NO,            REQUIRED FOR LEASED LINE            X
               REPLYTO=1.0,        USE DEFAULT                         X
               TYPE=NCP            NCP ONLY
        SPACE  2
***********************************************************************
*      LINE MACRO SPECIFICATION - FULL-DUPLEX, LEASED                 *
*      MAY BE USED FOR 3790, 3600, OR 3650                            *
*                                                                     *
*      NOTE: LINE SPEED MAY BE RAISED TO 2400 FOR                     *
*      ALL PHYSICAL UNITS AND TO 4800 FOR 3600 AND 3650               *
*      WITHOUT DOING A NEW GEN OF NCP.                                *
*      RETRIES VALUE FOR LINE SHOULD BE GREATER THAN 30               *
*      SECONDS AND LESS THAN ONE MINUTE FOR 3650.                     *
*                                                                     *
***********************************************************************
SDLC01   LINE  ADDRESS=020,        TRANSMIT AND RECEIVE ADDRESSES      X
               DUPLEX=HALF,        MODEM IS STRAPPED FOR HALF DUPLEX   X
               SPEED=9600,         SPEED MAY BE HIGHER (SEE NOTES)     X
               NRZI=NO,            SPECIFY YES ONLY IF REQUIRED        X
               NEWSYNC=NO,         CHECK MODEM REQUIREMENTS            X
               CLOCKNG=EXT,        MODEM PROVIDES CLOCKING             X
               POLLED=YES,                                             X
               RETRIES=(5,10,4),   5 RETRIES PER RECOVERY SEQUENCE     X
               ISTATUS=ACTIVE
         SPACE 2
***********************************************************************
*      SERVICE ORDER FOR SDLC LINK                                    *
***********************************************************************
         SERVICE ORDER=(SDLC3274)
         EJECT
***********************************************************************
*      PHYSICAL UNIT SPECIFICATIONS                                   *
***********************************************************************
SDLC3274 PU    ADDR=C1,           POLL ADDRESS                         X
               PUTYPE=2,                                               X
               ISTATUS=ACTIVE,                                         X
               MODETAB=BSPLMT02,                                       X
               SSCPFM=USS3270,                                         X
               MAXOUT=7,          MAX PATH INFO UNITS BEFORE RESPONSE  X
               MAXDATA=265,       MAXIMUM AMOUNT OF DATA               X
               PASSLIM=7,         .                                    X
               PACING=0,          FOR DISPLAYS AND DSC PRINTERS        X
               VPACING=0,         FOR DISPLAYS AND DSC PRINTERS        X
               DISCNT=(NO),       .                                    X
               RETRIES=(,1,4)     4 RETRIES, 1 SECOND BETWEEN
         SPACE 2
***********************************************************************
*      LOGICAL UNIT SPECIFICATIONS                                    *
***********************************************************************
SDLCPA01 LU LOCADDR=2,USSTAB=BSPUDT01,ISTATUS=INACTIVE
SDLCPA02 LU LOCADDR=3,USSTAB=BSPUDT01,ISTATUS=INACTIVE
SDLCPA03 LU LOCADDR=4,USSTAB=BSPUDT01,ISTATUS=INACTIVE
SDLCPA04 LU LOCADDR=5,USSTAB=BSPUDT01,ISTATUS=INACTIVE
         EJECT
***********************************************************************
*      GENEND DELIMITER                                               *
***********************************************************************
         GENEND
         END
```
