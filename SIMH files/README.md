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
```
### BSPLIN01
```
**************************************************************
*                                                            *
* NAME: BSPLIN01                                             *
*                                                            *
* TYPE: ASSEMBLER SOURCE                                     *
*                                                            *
* DESC: VTAM LOGON INTERPRET TABLE                           *
*                                                            *
**************************************************************
BSPLIN01 INTAB
         LOGCHAR APPLID=(APPLICID,TSO),SEQNCE='Tso'
         LOGCHAR APPLID=(APPLICID,TSO),SEQNCE='tso'
         LOGCHAR APPLID=(APPLICID,TSO),SEQNCE='TSO'
         LOGCHAR APPLID=(APPLICID,TSO),SEQNCE='Logon'
         LOGCHAR APPLID=(APPLICID,TSO),SEQNCE='logon'
         LOGCHAR APPLID=(APPLICID,TSO),SEQNCE='LOGON'
         LOGCHAR APPLID=(APPLICID,TSO),SEQNCE='IBMUSER'
         LOGCHAR APPLID=(APPLICID,TSO),SEQNCE='ibmuser'
         LOGCHAR APPLID=(APPLICID,TSO),SEQNCE='Ibmuser'
         LOGCHAR APPLID=(APPLICID,TSO),SEQNCE='Herc01'
         LOGCHAR APPLID=(APPLICID,TSO),SEQNCE='herc01'
         LOGCHAR APPLID=(APPLICID,TSO),SEQNCE='HERC01'
         LOGCHAR APPLID=(APPLICID,TSO),SEQNCE='Herc02'
         LOGCHAR APPLID=(APPLICID,TSO),SEQNCE='herc02'
         LOGCHAR APPLID=(APPLICID,TSO),SEQNCE='HERC02'
         LOGCHAR APPLID=(APPLICID,TSO),SEQNCE='Herc03'
         LOGCHAR APPLID=(APPLICID,TSO),SEQNCE='herc03'
         LOGCHAR APPLID=(APPLICID,TSO),SEQNCE='HERC03'
         LOGCHAR APPLID=(APPLICID,TSO),SEQNCE='Herc04'
         LOGCHAR APPLID=(APPLICID,TSO),SEQNCE='herc04'
         LOGCHAR APPLID=(APPLICID,TSO),SEQNCE='HERC04'
         ENDINTAB
         END
```
### BSPUDT01
```
//BSPUDT01 JOB CLASS=A,MSGCLASS=X,MSGLEVEL=(1,1)
//ASM EXEC ASMFC,PARM.ASM='DECK',REGION.ASM=512K
//ASM.SYSUT1 DD UNIT=SYSDA
//ASM.SYSUT2 DD UNIT=SYSDA
//ASM.SYSUT3 DD UNIT=SYSDA
//ASM.SYSPUNCH  DD  DSN=&&TEMP,DISP=(MOD,PASS),SPACE=(CYL,(1,1)),
//             UNIT=SYSDA,DCB=(DSORG=PS,RECFM=FB,LRECL=80,BLKSIZE=800)
//ASM.SYSIN DD *
**********************************************************************
* BSPUDT01 - USS TABLE FOR TUR(N)KEY MVS - MAY, 2009
* TK MODIFIED VERSION OF ORIGINAL FILE MVSSRC.SYM401.F09(ISTINCDT)
**********************************************************************
* /* START OF SPECIFICATIONS ****                                       00050000
*                                                                       00100000
*01*  MODULE-NAME = ISTINCDT                                            00150000
*                                                                       00200000
*01*  DESCRIPTIVE-NAME = DEFAULT USS DEFINITION TABLE                   00250000
*                                                                       00300000
*01*  COPYRIGHT = NONE                                                  00350000
*                                                                       00400000
*01*  STATUS = RELEASE 2                                                00450000
*                                                                       00500000
*01*  FUNCTION = THIS TABLE IS SUPPLIED BY IBM FOR THE USE OF ANYONE    00550000
*     WHO DESIRES THE SUPPORT CONTAINED.  THE TABLE CONSISTS OF THE     00600000
*     USSTAB CALL, A LOGON AND LOGOFF COMMAND FORMAT AND A STANDARD     00650000
*     CHARACTER  TRANSLATION TABLE. THIS ALSO PROVIDES AN INSTALLATION  00700000
*     WITH THE OPPORTUNITY TO REPLACE THE LOAD MODULE  (OR PHASE)       00750000
*     ISTINCDT AND THEREBY TAILOR THE DEFINITION OF THE USS COMMANDS    00800000
*     AND MESSAGES ON AN INSTALLATION- WIDE BASIS WITHOUT THE NECESSITY 00850000
*     FOR CODING THE USSTAB= PARAMETER FOR EACH LU.                     00900000
*                                                                       00950000
*01*  NOTES = A REPLACEMENT MODULE AS DESCRIBED ABOVE COULD REQUIRE     01000000
*     FREQUENT MODIFICATIONS SINCE NEW USS COMMANDS  AND MESSAGES MAY   01050000
*     BE ADDED IN FUTURE RELEASES.                                      01100000
*                                                                       01150000
*02*    CHARACTER-CODE-DEPENDENCIES = NONE                              01200000
*                                                                       01250000
*02*    DEPENDENCIES = NONE                                             01300000
*                                                                       01350000
*02*    RESTRICTIONS = NONE                                             01400000
*                                                                       01450000
*02*    REGISTER-CONVENTIONS = NOT APPLICABLE                           01500000
*                                                                       01550000
*02*    PATCH-LABEL = NONE                                              01600000
*                                                                       01650000
*01*  MODULE-TYPE = MODULE                                              01700000
*                                                                       01750000
*02*    PROCESSOR = ASSEM-370R                                          01800000
*                                                                       01850000
*02*    MODULE-SIZE = 610 BYTES                                         01900000
*                     COMMENTS-DEPENDENT UPON NUMBER OF MACRO           01950000
*                     INVOCATIONS USED IN THE BUILDING OF ISTINCDT      02000000
*                                                                       02050000
*02*    ATTRIBUTES = NON-EXECUTABLE                                     02100000
*                                                                       02150000
*03*      RELOCATE = PAGEABLE                                           02200000
*                                                                       02250000
*03*      MODE = NOT APPLICABLE                                         02300000
*                                                                       02350000
*03*      PROTECTION = USER-KEY                                         02400000
*                                                                       02450000
*03*      SPECIAL-PSW-SETTING = NONE                                    02500000
*                                                                       02550000
*01*  ENTRY = NOT APPLICABLE                                            02600000
*                                                                       02650000
*02*    PURPOSE = SEE FUNCTION                                          02700000
*                                                                       02750000
*02*    LINKAGE = NOT APPLICABLE                                        02800000
*                                                                       02850000
*02*    INPUT = NONE                                                    02900000
*                                                                       02950000
*03*      REGISTERS-SAVED-AND-RESTORED = NONE                           03000000
*                                                                       03050000
*03*      REGISTERS-INPUT = NONE                                        03100000
*                                                                       03150000
*02*    OUTPUT = NONE                                                   03200000
*                                                                       03250000
*03*      REGISTERS-OUTPUT = NONE                                       03300000
*                                                                       03350000
*03*      REGISTERS-NOT-CORRUPTED = ALL                                 03400000
*                                                                       03450000
*01*  EXIT-NORMAL = NONE                                                03500000
*                                                                       03550000
*01*  EXIT-ERROR = NONE                                                 03600000
*                                                                       03650000
*01*  EXTERNAL-REFERENCES = NONE                                        03700000
*                                                                       03750000
*02*    ROUTINES = NONE                                                 03800000
*                                                                       03850000
*03*      LINKAGE = NOT APPLICABLE                                      03900000
*                                                                       03950000
*03*      REGISTERS-PASSED = NONE                                       04000000
*                                                                       04050000
*03*      REGISTERS-RETURNED = NONE                                     04100000
*                                                                       04150000
*02*    DATA-SETS = NONE                                                04200000
*                                                                       04250000
*02*    DATA-AREA = NONE                                                04300000
*                                                                       04350000
*02*    CONTROL-BLOCKS-SYSTEM = NONE                                    04400000
*                                                                       04450000
*02*    CONTROL-BLOCKS-VTAM = NONE                                      04500000
*                                                                       04550000
*01*  TABLES = CONTAINS-UDT (USS DEFINITION TABLE), VPB (COMMAND        04600000
*     PROCESSING BLOCK), PPB (PARAMETER PROCESSING BLOCK),  MPB         04650000
*     (MESSAGE PROCESSING BLOCK).                                       04700000
*                                                                       04750000
*01*  MACROS = USSTAB, USSCMD, USSPARM, USSMSG, USSEND                  04800000
*                                                                       04850000
*01*  CHANGE-ACTIVITY = DCR 3872.2                                      04900000
*                                                                       04950000
**** END OF SPECIFICATIONS ***/                                         05000000
         EJECT                                                          05010000
BSPUDT01 USSTAB   TABLE=STDTRANS                                        05050000
         SPACE 4                                                        05060000
LOGON    USSCMD   CMD=LOGON,FORMAT=PL1                                  05100000
         USSPARM  PARM=APPLID                                           05150000
         USSPARM  PARM=LOGMODE                                          05200000
         USSPARM  PARM=DATA                                             05250000
         EJECT                                                          05260000
LOGOFF   USSCMD   CMD=LOGOFF,FORMAT=PL1                                 05300000
         USSPARM  PARM=APPLID                                           05350000
         USSPARM  PARM=TYPE,DEFAULT=UNCOND                              05400000
         USSPARM  PARM=HOLD,DEFAULT=NO                         @D32CKDS 05404000
         EJECT                                                          05410000
TSO      USSCMD   CMD=TSO,REP=LOGON,FORMAT=BAL
         USSPARM  PARM=APPLID,DEFAULT=TSO
         USSPARM  PARM=P1,REP=DATA
         USSPARM  PARM=LOGMODE
TSO2     USSCMD   CMD=TSO2,REP=LOGON,FORMAT=BAL
         USSPARM  PARM=APPLID,DEFAULT=TSO
         USSPARM  PARM=P1,REP=DATA
         USSPARM  PARM=LOGMODE,DEFAULT=MHP3278E
HERC01   USSCMD   CMD=HERC01,REP=LOGON,FORMAT=BAL
         USSPARM  PARM=APPLID,DEFAULT=TSO
         USSPARM  PARM=DATA,DEFAULT=HERC01
         USSPARM  PARM=LOGMODE
HERC02   USSCMD   CMD=HERC02,REP=LOGON,FORMAT=BAL
         USSPARM  PARM=APPLID,DEFAULT=TSO
         USSPARM  PARM=DATA,DEFAULT=HERC02
         USSPARM  PARM=LOGMODE
HERC03   USSCMD   CMD=HERC03,REP=LOGON,FORMAT=BAL
         USSPARM  PARM=APPLID,DEFAULT=TSO
         USSPARM  PARM=DATA,DEFAULT=HERC03
         USSPARM  PARM=LOGMODE
HERC04   USSCMD   CMD=HERC04,REP=LOGON,FORMAT=BAL
         USSPARM  PARM=APPLID,DEFAULT=TSO
         USSPARM  PARM=DATA,DEFAULT=HERC04
         USSPARM  PARM=LOGMODE
JRP      USSCMD   CMD=JRP,REP=LOGON,FORMAT=BAL
         USSPARM  PARM=APPLID,DEFAULT=JRP
         USSPARM  PARM=P1,REP=DATA
         USSPARM  PARM=LOGMODE
MESSAGES USSMSG   MSG=1,TEXT='INVALID COMMAND SYNTAX'                   05420000
         USSMSG   MSG=2,TEXT='% COMMAND UNRECOGNIZED'                   05430000
         USSMSG   MSG=3,TEXT='% PARAMETER UNRECOGNIZED'                 05440000
         USSMSG   MSG=4,TEXT='% PARAMETER INVALID'                      05442000
         USSMSG   MSG=5,TEXT='UNSUPPORTED FUNCTION'                     05444000
         USSMSG   MSG=6,TEXT='SEQUENCE ERROR'                           05446000
         USSMSG   MSG=7,TEXT='SESSION NOT BOUND'                        05448000
         USSMSG   MSG=8,TEXT='INSUFFICIENT STORAGE'                     05448100
         USSMSG   MSG=9,TEXT='MAGNETIC CARD DATA ERROR'                 05448200
         USSMSG   MSG=10,TEXT='TK USS10: PLEASE ENTER LOGON'            05448300
         EJECT                                                          05448400
STDTRANS DC       X'000102030440060708090A0B0C0D0E0F'                   05450000
         DC       X'101112131415161718191A1B1C1D1E1F'                   05500000
         DC       X'202122232425262728292A2B2C2D2E2F'                   05550000
         DC       X'303132333435363738393A3B3C3D3E3F'                   05600000
         DC       X'404142434445464748494A4B4C4D4E4F'                   05650000
         DC       X'505152535455565758595A5B5C5D5E5F'                   05700000
         DC       X'606162636465666768696A6B6C6D6E6F'                   05750000
         DC       X'707172737475767778797A7B7C7D7E7F'                   05800000
         DC       X'80C1C2C3C4C5C6C7C8C98A8B8C8D8E8F'                   05850000
         DC       X'90D1D2D3D4D5D6D7D8D99A9B9C9D9E9F'                   05900000
         DC       X'A0A1E2E3E4E5E6E7E8E9AAABACADAEAF'                   05950000
         DC       X'B0B1B2B3B4B5B6B7B8B9BABBBCBDBEBF'                   06000000
         DC       X'C0C1C2C3C4C5C6C7C8C9CACBCCCDCECF'                   06050000
         DC       X'D0D1D2D3D4D5D6D7D8D9DADBDCDDDEDF'                   06100000
         DC       X'E0E1E2E3E4E5E6E7E8E9EAEBECEDEEEF'                   06150000
         DC       X'F0F1F2F3F4F5F6F7F8F9FAFBFCFDFEFF'                   06200000
END      USSEND                                                         06250000
         END     ,             END OF ASSEMBLY                          06300000
/*
//LKED   EXEC  PGM=IEWL,PARM='XREF,LIST,LET,CALL,AC=0',REGION=512K
//*            COND=(4,LT,ASM2)
//SYSLIN DD    DSNAME=&&TEMP,DISP=(OLD,DELETE)
//SYSLMOD DD DISP=SHR,DSN=SYS1.VTAMLIB(BSPUDT01)
//SYSUT1 DD    DSNAME=&SYSUT1,UNIT=(SYSDA),
//             SPACE=(1024,(50,20))
//SYSPRINT DD  SYSOUT=A
//SYSLIB DD DUMMY
//*
//
```

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
