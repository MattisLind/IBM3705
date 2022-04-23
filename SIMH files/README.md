# Supporting BSC

BSC is the predecessor of SDLC for remote communication with terminals. This branch is for adapting the IBM 3705 emulator to BSC.

## Changes to the configuration to MVS running in Hercules

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
### BSPLMT02
```
//BSPLMT02 JOB CLASS=A,MSGCLASS=X,MSGLEVEL=(1,1)
//ASM EXEC ASMFC,PARM.ASM='DECK',REGION.ASM=512K
//ASM.SYSUT1 DD UNIT=SYSDA
//ASM.SYSUT2 DD UNIT=SYSDA
//ASM.SYSUT3 DD UNIT=SYSDA
//ASM.SYSPUNCH  DD  DSN=&&TEMP,DISP=(MOD,PASS),SPACE=(CYL,(1,1)),
//             UNIT=SYSDA,DCB=(DSORG=PS,RECFM=FB,LRECL=80,BLKSIZE=800)
//ASM.SYSIN DD *
*********************************************************************   00001000
* LOGMODE TABLE FOR TUR(N)KEY MVS, MAY 2009                             00002000
* CONSISTING OF                                                         00003000
* 1. ORIGINAL SOURCE FROM IBM MVSSRC.SYM401.F12(ISTINCLM)               00004000
* 2. TK3 JCL/SG0250 BSPLMT01 (S3270 RENAMED TO BSPS3270)                00005000
* 3. ENTRIES CONTRIBUTED BY GERHARD POSTPISCHIL                         00006000
* 4. ENTRIES CONTRIBUTED BY MAX PARKE                                   00007000
*********************************************************************   00008000
         MACRO ,                                                        00009000
&NM      CRT   &FM=,&TS=,&PRI=,&SEC=,&COM=,&LU=,&EDS=,&DEF=,&ALT=,     *00010000
               &SW=,&RU=                                                00011000
         GBLC  &GFM,&GTS,&GPRI,&GSEC,&GCOM,&GLU,&GEDS,&GDEF,&GALT,&GSW  00012000
         GBLC  &GRU                                                     00013000
         GBLB  &GONCE                                                   00014000
         AIF   (&GONCE).GONCE                                           00015000
&GFM     SETC  '02'                                                     00016000
&GTS     SETC  '02'                                                     00017000
&GPRI    SETC  '71'                                                     00018000
&GSEC    SETC  '40'                                                     00019000
&GCOM    SETC  '2000'                                                   00020000
&GLU     SETC  '00'                                                     00021000
&GEDS    SETC  '00'                                                     00022000
&GDEF    SETC  '1850'                                                   00023000
&GALT    SETC  '1850'                                                   00024000
&GSW     SETC  '7F'                                                     00025000
&GRU     SETC  '0000'                                                   00026000
&GONCE   SETB  1             SET FIRST TIME DONE                        00027000
.GONCE   AIF   ('&FM' EQ '').NOGFM                                      00028000
&GFM     SETC  '&FM'                                                    00029000
.NOGFM   AIF   ('&TS' EQ '').NOGTS                                      00030000
&GTS     SETC  '&TS'                                                    00031000
.NOGTS   AIF   ('&PRI' EQ '').NOGPRI                                    00032000
&GPRI    SETC  '&PRI'                                                   00033000
.NOGPRI  AIF   ('&SEC' EQ '').NOGSEC                                    00034000
&GSEC    SETC  '&SEC'                                                   00035000
.NOGSEC  AIF   ('&COM' EQ '').NOGCOM                                    00036000
&GCOM    SETC  '&COM'                                                   00037000
.NOGCOM  AIF   ('&LU' EQ '').NOGLU                                      00038000
&GLU     SETC  '&LU'                                                    00039000
.NOGLU   AIF   ('&EDS' EQ '').NOGEDS                                    00040000
&GEDS    SETC  '&EDS'                                                   00041000
.NOGEDS  AIF   ('&DEF' EQ '').NOGDEF                                    00042000
&GDEF    SETC  '&DEF'                                                   00043000
.NOGDEF  AIF   ('&ALT' EQ '').NOGALT                                    00044000
&GALT    SETC  '&ALT'                                                   00045000
.NOGALT  AIF   ('&SW' EQ '').NOGSW                                      00046000
&GSW     SETC  '&SW'                                                    00047000
.NOGSW   AIF   ('&RU' EQ '').NOGRU                                      00048000
&GRU     SETC  '&RU'                                                    00049000
.NOGRU   AIF   (T'&SYSLIST(0) NE 'O').OK                                00050000
         MNOTE 4,'LOGMODE (NAME FIELD) EXPECTED'                        00051000
.OK      ANOP  ,                                                        00052000
&NM      MODEENT LOGMODE=&NM,FMPROF=X'&GFM',TSPROF=X'&GTS',RUSIZES=X'&G*00053000
               RU',PRIPROT=X'&GPRI',SECPROT=X'&GSEC',COMPROT=X'&GCOM', *00054000
               PSERVIC=X'&GLU&GEDS.00000000&GDEF&GALT&GSW.00'           00055000
         MEND  ,                                                        00056000
         EJECT ,                                                        00057000
BSPLMT02 MODETAB                                                        00058000
         EJECT ,                                                        00059000
* /* START OF SPECIFICATIONS ****                                       00060000
*                                                                       00061000
*01*  MODULE-NAME = ISTINCLM                                            00062000
*                                                                       00063000
*01*  DESCRIPTIVE-NAME = DEFAULT LOGON MODE TABLE                       00064000
*                                                                       00065000
*01*  COPYRIGHT = NONE                                                  00066000
*                                                                       00067000
*01*  STATUS = RELEASE 2                                                00068000
*                                                                       00069000
*01*  FUNCTION = THE PURPOSE OF THIS TABLE IS TO PROVIDE THE USER WITH  00070000
*     A DEFAULT TABLE PROVIDING SUPPORT FOR THE DEVICES LISTED BELOW:   00071000
*     3767/3770 INTERACTIVE, 3770 BATCH, 3270 WITH SDLC SUPPORT, 3600,  00072000
*      3650 INTERACTIVE, 3650 INTERUSER, 3650 SDLC, 3650 PIPELINE       00073000
*      AND 3660 USBATCH.                                                00074000
*                                                                       00075000
*01*  NOTES = NONE                                                      00076000
*                                                                       00077000
*02*    CHARACTER-CODE-DEPENDENCIES = NONE                              00078000
*                                                                       00079000
*02*    DEPENDENCIES = NONE                                             00080000
*                                                                       00081000
*02*    RESTRICTIONS = NONE                                             00082000
*                                                                       00083000
*02*    REGISTER-CONVENTIONS = NONE                                     00084000
*                                                                       00085000
*02*    PATCH-LABEL = NONE                                              00086000
*                                                                       00087000
*01*  MODULE-TYPE = MODULE, NON EXECUTABLE                              00088000
*                                                                       00089000
*02*    PROCESSOR = ASSEMBLER                                           00090000
*                                                                       00091000
*02*    MODULE-SIZE = RES: CHOOSE: (9) BYTES,                           00092000
*                     COMMENTS: ENTER SIZE CONSTRAINTS IF KNOWN,        00093000
*                               OTHERWISE LEAVE;                        00094000
*                                                                       00095000
*02*    ATTRIBUTES = REFRESHABLE, NO EXECUTABLE CODE                    00096000
*                                                                       00097000
*03*      RELOCATE = PAGEABLE                                           00098000
*                                                                       00099000
*03*      MODE = PROBLEM-PROGRAM                                        00100000
*                                                                       00101000
*03*      PROTECTION = USER-KEY                                         00102000
*                                                                       00103000
*03*      SPECIAL-PSW-SETTING = NONE                                    00104000
*                                                                       00105000
*01*  ENTRY = ISTINCLM                                                  00106000
*                                                                       00107000
*02*    PURPOSE = SEE FUNCTION                                          00108000
*                                                                       00109000
*02*    LINKAGE = NOT APPLICABLE                                        00110000
*                                                                       00111000
*02*    INPUT = NONE                                                    00112000
*                                                                       00113000
*03*      REGISTERS-SAVED-AND-RESTORED = NOT APPLICABLE                 00114000
*                                                                       00115000
*03*      REGISTERS-INPUT = NOT APPLICABLE                              00116000
*                                                                       00117000
*02*    OUTPUT = NONE                                                   00118000
*                                                                       00119000
*03*      REGISTERS-OUTPUT = NOT APPLICABLE                             00120000
*                                                                       00121000
*03*      REGISTERS-NOT-CORRUPTED = ALL                                 00122000
*                                                                       00123000
*01*  EXIT-NORMAL = NOT APPLICABLE                                      00124000
*                                                                       00125000
*01*  EXIT-ERROR = NOT APPLICABLE                                       00126000
*                                                                       00127000
*01*  EXTERNAL-REFERENCES = NONE                                        00128000
*                                                                       00129000
*02*    ROUTINES = NONE                                                 00130000
*                                                                       00131000
*03*      LINKAGE = NOT APPLICABLE                                      00132000
*                                                                       00133000
*03*      REGISTERS-PASSED = NOT APPLICABLE                             00134000
*                                                                       00135000
*03*      REGISTERS-RETURNED = NOT APPLICABLE                           00136000
*                                                                       00137000
*02*    DATA-SETS = NONE                                                00138000
*                                                                       00139000
*02*    DATA-AREA = NONE                                                00140000
*                                                                       00141000
*02*    CONTROL-BLOCKS-SYSTEM = NONE                                    00142000
*                                                                       00143000
*02*    CONTROL-BLOCKS-VTAM = NONE                                      00144000
*                                                                       00145000
*01*  TABLES = NONE                                                     00146000
*                                                                       00147000
*01*  MACROS = MODETAB,MODEENT,MODEEND                                  00148000
*                                                                       00149000
*01*  CHANGE-ACTIVITY = NONE                                            00150000
*                                                                       00151000
**** END OF SPECIFICATIONS ***/                                         00152000
         EJECT                                                          00153000
IBM3767  MODEENT LOGMODE=INTERACT,FMPROF=X'03',TSPROF=X'03',PRIPROT=X'B*00154000
               1',SECPROT=X'A0',COMPROT=X'3040'                         00155000
         EJECT                                                          00156000
IBM3770  MODEENT LOGMODE=BATCH,FMPROF=X'03',TSPROF=X'03',PRIPROT=X'A3',*00157000
               SECPROT=X'A3',COMPROT=X'7080'                            00158000
         EJECT                                                          00159000
IBMS3270 MODEENT LOGMODE=S3270,FMPROF=X'02',TSPROF=X'02',PRIPROT=X'71',*00160000
               SECPROT=X'40',COMPROT=X'2000'                            00161000
         EJECT                                                          00162000
IBM3600  MODEENT LOGMODE=IBM3600,FMPROF=X'04',TSPROF=X'04',PRIPROT=X'F1*00163000
               ',SECPROT=X'F1',COMPROT=X'7000'                          00164000
         EJECT                                                          00165000
IBM3650I MODEENT LOGMODE=INTRACT,FMPROF=X'00',TSPROF=X'04',PRIPROT=X'B1*00166000
               ',SECPROT=X'90',COMPROT=X'6000'                          00167000
         EJECT                                                          00168000
IBM3650U MODEENT LOGMODE=INTRUSER,FMPROF=X'00',TSPROF=X'04',PRIPROT=X'3*00169000
               1',SECPROT=X'30',COMPROT=X'6000'                         00170000
         EJECT                                                          00171000
IBMS3650 MODEENT LOGMODE=IBMS3650,FMPROF=X'00',TSPROF=X'04',PRIPROT=X'B*00172000
               0',SECPROT=X'90',COMPROT=X'4000'                         00173000
         EJECT                                                          00174000
IBM3650P MODEENT LOGMODE=PIPELINE,FMPROF=X'00',TSPROF=X'03',PRIPROT=X'3*00175000
               0',SECPROT=X'10',COMPROT=X'0000'                         00176000
         EJECT                                                          00177000
IBM3660  MODEENT LOGMODE=SMAPPL,FMPROF=X'03',TSPROF=X'03',PRIPROT=X'A0'*00178000
               ,SECPROT=X'A0',COMPROT=X'0081'                           00179000
         EJECT                                                          00180000
IBM3660A MODEENT LOGMODE=SMSNA100,FMPROF=X'00',TSPROF=X'00',PRIPROT=X'0*00181000
               0',SECPROT=X'00',COMPROT=X'0000'                         00182000
         EJECT                                                          00183000
**********************************************************************  00484000
* SNA LU2 3278-2 TESTED FOR USE WITH MVS 3.8J TSO                       00485000
**********************************************************************  00486000
MHP32782 MODEENT LOGMODE=MHP32782,                                     X00487000
               FMPROF=X'03',                                           X00488000
               TSPROF=X'03',                                           X00489000
               PRIPROT=X'B1',                                          X00490000
               SECPROT=X'90',                                          X00491000
               COMPROT=X'3080',                                        X00492000
               RUSIZES=X'8585',                                        X00493000
               PSERVIC=X'020000000000185018500200'                      00494000
         SPACE 2                                                        00495000
**********************************************************************  00496000
* SNA LU2 3278-2 WITH EXTENDED 3270 DATASTREAM                          00497000
**********************************************************************  00498000
MHP3278E MODEENT LOGMODE=MHP3278E,                                     X00499000
               FMPROF=X'03',                                           X00500000
               TSPROF=X'03',                                           X00501000
               PRIPROT=X'B1',                                          X00502000
               SECPROT=X'90',                                          X00503000
               COMPROT=X'3080',                                        X00504000
               RUSIZES=X'8585',                                        X00505000
               PSERVIC=X'028000000000185018500200'                      00506000
         SPACE 2                                                        00507000
**********************************************************************  00508000
* 3790 IN DATA STREAM COMPATIBILITY MODE                                00509000
**********************************************************************  00510000
EMU3790  MODEENT LOGMODE=EMU3790,                                      X00511000
               FMPROF=X'03',                                           X00512000
               TSPROF=X'03',                                           X00513000
               PRIPROT=X'B1',                                          X00514000
               SECPROT=X'90',                                          X00515000
               COMPROT=X'3080',                                        X00516000
               PSERVIC=X'020000000000000000000200'                      00517000
         EJECT                                                          00518000
**************************************************************          00184000
*                                                            *          00185000
* NAME: BSPLMT01                                             *          00186000
*                                                            *          00187000
* TYPE: ASSEMBLER SOURCE                                     *          00188000
*                                                            *          00189000
* DESC: VTAM LOGMODE TABLE                                   *          00190000
*                                                            *          00191000
**************************************************************          00192000
*****************************************************************       00193000
* NON-SNA 3270 LOCAL TERMINALS                                  *       00194000
*      PRIMARY SCREEN   : MODEL 2                               *       00195000
*      SECONDARY SCREEN : NON                                   *       00196000
*****************************************************************       00197000
S3270    MODEENT LOGMODE=BSPS3270,                                     X00198000
               FMPROF=X'02',                                           X00199000
               TSPROF=X'02',                                           X00200000
               PRIPROT=X'71',                                          X00201000
               SECPROT=X'40',                                          X00202000
               COMPROT=X'2000',                                        X00203000
               PSERVIC=X'000000000000000000000200'                      00204000
*****************************************************************       00205000
* NON-SNA 3270 LOCAL TERMINALS                                  *       00206000
*      PRIMARY SCREEN   : MODEL 5                               *       00207000
*      SECONDARY SCREEN : NON                                   *       00208000
*****************************************************************       00209000
S32785   MODEENT LOGMODE=S32785,                                       X00210000
               FMPROF=X'02',                                           X00211000
               TSPROF=X'02',                                           X00212000
               PRIPROT=X'71',                                          X00213000
               SECPROT=X'40',                                          X00214000
               COMPROT=X'2000',                                        X00215000
               PSERVIC=X'00000000000018501B847F00'                      00216000
*****************************************************************       00217000
* 3274 MODEL 1C WITH MODEL 2 SCREEN (REMOTE SNA)                *       00218000
*      PRIMARY SCREEN   : MODEL 2                               *       00219000
*      SECONDARY SCREEN : NON                                   *       00220000
*****************************************************************       00221000
D4C32782 MODEENT LOGMODE=D4C32782,                                     X00222000
               FMPROF=X'03',                                           X00223000
               TSPROF=X'03',                                           X00224000
               PRIPROT=X'B1',                                          X00225000
               SECPROT=X'90',                                          X00226000
               COMPROT=X'3080',                                        X00227000
               RUSIZES=X'87F8',                                        X00228000
               PSERVIC=X'020000000000185020507F00'                      00229000
*****************************************************************       00230000
*      3276 SNA WITH MODEL 2 SCREEN (REMOTE SNA)                *       00231000
*      PRIMARY SCREEN   : MODEL 2                               *       00232000
*      SECONDARY SCREEN : NON                                   *       00233000
*****************************************************************       00234000
D6327802 MODEENT LOGMODE=D6327802,                                     X00235000
               FMPROF=X'03',                                           X00236000
               TSPROF=X'03',                                           X00237000
               PRIPROT=X'B1',                                          X00238000
               SECPROT=X'90',                                          X00239000
               COMPROT=X'3080',                                        X00240000
               RUSIZES=X'88F8',                                        X00241000
               PSERVIC=X'020000000000185000007E00'                      00242000
*****************************************************************       00243000
*      3274 1C SNA WITH MODEL 5 SCREEN (REMOTE SNA)             *       00244000
*      PRIMARY SCREEN   : MODEL 5                               *       00245000
*      SECONDARY SCREEN : NONE                                  *       00246000
*****************************************************************       00247000
D4C32785 MODEENT LOGMODE=D4C32785,                                     X00248000
               FMPROF=X'03',                                           X00249000
               TSPROF=X'03',                                           X00250000
               PRIPROT=X'B1',                                          X00251000
               SECPROT=X'90',                                          X00252000
               COMPROT=X'3080',                                        X00253000
               RUSIZES=X'87F8',                                        X00254000
               PSERVIC=X'0200000000001B8400007E00'                      00255000
*****************************************************************       00256000
*      3276 SNA WITH MODEL 2 SCREEN (REMOTE SNA) (T.S.O)        *       00257000
*      PRIMARY SCREEN   : MODEL 2                               *       00258000
*      SECONDARY SCREEN : NON                                   *       00259000
*****************************************************************       00260000
D63278TS MODEENT LOGMODE=D63278TS,                                     X00261000
               FMPROF=X'03',                                           X00262000
               TSPROF=X'03',                                           X00263000
               PRIPROT=X'B1',                                          X00264000
               SECPROT=X'90',                                          X00265000
               COMPROT=X'3080',                                        X00266000
               RUSIZES=X'8587',                                        X00267000
               PSERVIC=X'020000000000000000000200'                      00268000
*****************************************************************       00269000
*      3276 SNA WITH 3289 MODEL 2 PRINTER                       *       00270000
*****************************************************************       00271000
D6328902 MODEENT LOGMODE=D6328902,                                     X00272000
               FMPROF=X'03',                                           X00273000
               TSPROF=X'03',                                           X00274000
               PRIPROT=X'B1',                                          X00275000
               SECPROT=X'90',                                          X00276000
               COMPROT=X'3080',                                        X00277000
               RUSIZES=X'8787',                                        X00278000
               PSERVIC=X'030000000000185018507F00'                      00279000
*****************************************************************       00280000
*      3274 NON-SNA  MODEL 2 SCREEN (LOCAL)                     *       00281000
*      PRIMARY SCREEN   : MODEL 2                               *       00282000
*      SECONDARY SCREEN : NON                                   *       00283000
*****************************************************************       00284000
D4B32782 MODEENT LOGMODE=D4B32782,                                     X00285000
               FMPROF=X'02',                                           X00286000
               TSPROF=X'02',                                           X00287000
               PRIPROT=X'71',                                          X00288000
               SECPROT=X'40',                                          X00289000
               COMPROT=X'2000',                                        X00290000
               RUSIZES=X'0000',                                        X00291000
               PSERVIC=X'000000000000185000007E00'                      00292000
*****************************************************************       00293000
*     S C S   P R I N T E R                                     *       00294000
*****************************************************************       00295000
SCS      MODEENT LOGMODE=SCS,                                          X00296000
               FMPROF=X'03',                                           X00297000
               TSPROF=X'03',                                           X00298000
               PRIPROT=X'B1',                                          X00299000
               SECPROT=X'90',                                          X00300000
               COMPROT=X'3080',                                        X00301000
               RUSIZES=X'87C6',                                        X00302000
               PSNDPAC=X'01',                                          X00303000
               SRCVPAC=X'01',                                          X00304000
               PSERVIC=X'01000000E100000000000000'                      00305000
*****************************************************************       00306000
*        N C C F                                                *       00307000
*****************************************************************       00308000
DSILGMOD MODEENT LOGMODE=DSILGMOD,                                     X00309000
               FMPROF=X'02',                                           X00310000
               TSPROF=X'02',                                           X00311000
               PRIPROT=X'71',                                          X00312000
               SECPROT=X'40',                                          X00313000
               COMPROT=X'2000',                                        X00314000
               RUSIZES=X'0000',                                        X00315000
               PSERVIC=X'000000000000000000000200'                      00316000
*****************************************************************       00317000
*        N C C F                                                *       00318000
*****************************************************************       00319000
DSIXDMN  MODEENT LOGMODE=DSIXDMN,                                      X00320000
               FMPROF=X'03',                                           X00321000
               TSPROF=X'03',                                           X00322000
               PRIPROT=X'20',                                          X00323000
               SECPROT=X'20',                                          X00324000
               COMPROT=X'4000',                                        X00325000
               RUSIZES=X'0000',                                        X00326000
               PSERVIC=X'000000000000000000000000'                      00327000
*****************************************************************       00328000
*      3276 SNA WITH MODEL 2 SCREEN (MAGNETIC STRIPE READER)    *       00329000
*      PRIMARY SCREEN   : MODEL 2                               *       00330000
*      SECONDARY SCREEN : NON                                   *       00331000
*      TEST TEST TEST TEST TEST TEST                            *       00332000
*****************************************************************       00333000
SCSLRDR  MODEENT LOGMODE=SCSLRDR,                                      X00334000
               FMPROF=X'03',                                           X00335000
               TSPROF=X'03',                                           X00336000
               PRIPROT=X'B1',                                          X00337000
               SECPROT=X'90',                                          X00338000
               COMPROT=X'3080',                                        X00339000
               RUSIZES=X'87C6',                                        X00340000
               PSNDPAC=X'01',                                          X00341000
               SRCVPAC=X'01',                                          X00342000
               PSERVIC=X'04000000E100000000000000'                      00343000
         EJECT                                                          00344000
MODETABG TITLE 'M O D E T A B  ***  SAS/GRAPH AND WYLBUR MODE TABLE'    00345000
         PRINT GEN                                               86128  00346000
         SPACE 2                                                        00347000
*        LOCAL AND BISYNCH REMOTE CRTS                           85354  00348000
*                                                                       00349000
L3277M2  CRT   DEF=0000,ALT=0000,SW=02   3277-2                  85354  00350000
         SPACE 1                                                 85354  00351000
*    LOCAL VARIABLE SIZE DEVICES (3290, 3180)                    92290  00352000
*                                                                92290  00353000
L3180MX  CRT   DEF=0000,ALT=0000,SW=03,EDS=80 QUERY; USE LARGEST 92290  00354000
L3278GX  CRT   DEF=0000,ALT=0000,SW=03,EDS=80 QUERY; USE LARGEST 92290  00355000
L3278MX  CRT   DEF=0000,ALT=0000,SW=03,EDS=00 QUERY; USE LARGEST 92290  00356000
L3279GX  CRT   DEF=0000,ALT=0000,SW=03,EDS=80 QUERY; USE LARGEST 92290  00357000
L3279MX  CRT   DEF=0000,ALT=0000,SW=03,EDS=00 QUERY; USE LARGEST 92290  00358000
L3290    CRT   DEF=0000,ALT=0000,SW=03,EDS=80 QUERY; USE LARGEST 92290  00359000
         SPACE 1                                                 85354  00360000
L3278M2  CRT   DEF=1850,ALT=0000,SW=7E,EDS=00 LOCAL PLAIN 24*80  92290  00361000
L3278G2  CRT   EDS=80        EXTENDED FIELDS SUPPORT                    00362000
L3278M3  CRT   ALT=2050,SW=7F,EDS=00  PLAIN 32*80                85360  00363000
L3278G3  CRT   EDS=80        EXTENDED FIELD                             00364000
L3278M4  CRT   ALT=2B50,EDS=00  PLAIN 43*80                             00365000
L3278G4  CRT   EDS=80        EXTENDED FIELD                             00366000
L3278M5  CRT   ALT=1B84,EDS=00        PLAIN 27*132                      00367000
L3278G5  CRT   EDS=80        EXTENDED FIELD                             00368000
         SPACE 1                                                        00369000
L3279M2  CRT   ALT=1850,EDS=00   BASIC 24*80 COLOR                      00370000
L3279G2  CRT   EDS=80        EXTENDED 24*80                             00371000
L3279M3  CRT   ALT=2050,EDS=00   BASIC 32*80 COLOR                      00372000
L3279G3  CRT   EDS=80        EXTENDED 24*80                             00373000
         SPACE 1                                                 85354  00374000
*        (VANILLA) IBM 3180 CRTS IN LOCAL NON-SNA MODE           88209  00375000
L3180M2  CRT   DEF=1850,ALT=0000,SW=7E,EDS=80 LOCAL PLAIN 24*80  88209  00376000
L3180M3  CRT   ALT=2050,SW=7F  PLAIN 32*80                       88209  00377000
L3180M4  CRT   ALT=2B50      PLAIN 43*80                         88209  00378000
L3180M5  CRT   ALT=1B84      PLAIN 27*132                        88209  00379000
         SPACE 1                                                 88209  00380000
L3286    CRT   DEF=0000,ALT=0000,EDS=00,SW=02 3286-2             85354  00381000
L3287    CRT   DEF=,ALT=,SW=                  3287-2             85354  00382000
L3287G   CRT   EDS=80             EXTENDED DS 3287-2             85354  00383000
         SPACE 1                                                 89038  00384000
*        ENTRY FOR ZAPPING                                       89038  00385000
L6262    CRT   DEF=1850,ALT=0000,EDS=80,SW=FE  LOCAL NON-SNA 6262       00386000
         SPACE 1                                                 89038  00387000
*        NEXT ENTRY FOR FUN AND GAMES - 3278 WHEN ATTACHED TO    89038  00388000
*        4341 CONSOLE PORT, OR A 3278 IN/ON A 3082.              89038  00389000
*        SCREEN SIZE IS 20*80                                    89038  00390000
LCONS    CRT   DEF=1450,ALT=0000,EDS=80,SW=FE  3278 AS CONSOLE   89038  00391000
         SPACE 2                                                        00392000
*        SNA CRTS - LOCAL                                               00393000
*                                                                       00394000
S3278M2  CRT   FM=03,TS=03,PRI=B1,SEC=90,COM=3080,RU=87C7,LU=02,       *00395000
               EDS=00,DEF=1850,ALT=0000,SW=7E  NO SWITCHING      86128  00396000
S3278G2  CRT   EDS=80        EXTENDED FIELDS SUPPORT                    00397000
S3278M3  CRT   ALT=2050,EDS=00,SW=7F  PLAIN 32*80                86128  00398000
S3278G3  CRT   EDS=80        EXTENDED FIELD                             00399000
S3278M4  CRT   ALT=2B50,EDS=00  PLAIN 43*80                             00400000
S3278G4  CRT   EDS=80        EXTENDED FIELD                             00401000
S3278M5  CRT   ALT=1B84,EDS=00  PLAIN 27*132                     86128  00402000
S3278G5  CRT   EDS=80        EXTENDED FIELD                             00403000
         SPACE 1                                                 87103  00404000
*        ITT LARGE CRT 43*80 IN DEFAULT MODE AND 27*132 IN ALTERNATE    00405000
*                                                                87103  00406000
S3278M45 CRT EDS=80,DEF=2B50,ALT=1B84,SW=7F  43*80 AND 27*132    87103  00407000
S3278M35 CRT EDS=80,DEF=2050,ALT=1B84,SW=7F  32*80 AND 27*132    88209  00408000
         SPACE 1                                                        00409000
S3279M2  CRT   DEF=1850,ALT=0000,EDS=00,SW=7E  BASIC 24*80 COLOR 86167  00410000
S3279G2  CRT   EDS=80        EXTENDED 24*80                             00411000
S3279M3  CRT   ALT=2050,EDS=00,SW=7F   BASIC 32*80 COLOR         86128  00412000
S3279G3  CRT   EDS=80        EXTENDED 24*80                             00413000
         SPACE 1                                                 85354  00414000
S3287    CRT   FM=03,TS=03,PRI=B1,SEC=90,COM=3080,RU=8787,LU=03,       *00415000
               EDS=00,DEF=1850,ALT=2B50,SW=7F   SNA 3287 LU 3    86128  00416000
S3287G   CRT   FM=03,TS=03,PRI=B1,SEC=90,COM=3080,RU=8787,LU=03,       *00417000
               EDS=80,DEF=1850,ALT=2B50,SW=7F   3287 LU 3 EDS    86128  00418000
S3287LU1 MODEENT LOGMODE=S3287LU1,FMPROF=X'03',TSPROF=X'03',           *00419000
               PRIPROT=X'B1',SECPROT=X'90',COMPROT=X'7080',            *00420000
               RUSIZES=X'8787',PSERVIC=X'01800001E100000000000000',    *00421000
               PSNDPAC=X'01',SRCVPAC=X'01'                       85354  00422000
         SPACE 1                                                 91014  00423000
*   THE 3268 IS DEFINED SEPARATELY TO AVOID ERRORS IN HASP328X   91014  00424000
*        NOTE THAT PRI SIZE= SEC.SIZE IS SIGNAL TO 328X CODE.    91014  00425000
S3268    CRT   FM=03,TS=03,PRI=B1,SEC=90,COM=3080,RU=8787,LU=03,       *00426000
               EDS=00,DEF=1850,ALT=1850,SW=7F   SNA 3268 LU 3    91014  00427000
         SPACE 2                                                 86167  00428000
*        SNA CRTS - REMOTE   (DEFINED FROM WYLBUR INST. MANUAL)  86167  00429000
*                                                                86167  00430000
T3278M2  CRT   FM=03,TS=03,PRI=B1,SEC=90,COM=3080,RU=87F8,LU=02,       *00431000
               EDS=00,DEF=1850,ALT=1850,SW=7F                    86167  00432000
T3278G2  CRT   EDS=80        EXTENDED FIELDS SUPPORT             86167  00433000
T3278M3  CRT   ALT=2050,EDS=00        PLAIN 32*80                86167  00434000
T3278G3  CRT   EDS=80        EXTENDED FIELD                      86167  00435000
T3278M4  CRT   ALT=2B50,EDS=00  PLAIN 43*80                      86167  00436000
T3278G4  CRT   EDS=80        EXTENDED FIELD                      86167  00437000
T3278M5  CRT   ALT=1B84,EDS=00  PLAIN 27*132                     86167  00438000
T3278G5  CRT   EDS=80        EXTENDED FIELD                      86167  00439000
         SPACE 1                                                 86167  00440000
T3279M2  CRT   DEF=1850,ALT=1850,EDS=00  BASIC 24*80 COLOR       86167  00441000
T3279G2  CRT   EDS=80        EXTENDED 24*80                      86167  00442000
T3279M3  CRT   ALT=2050,EDS=00    BASIC 32*80 COLOR              86167  00443000
T3279G3  CRT   EDS=80        EXTENDED 24*80                      86167  00444000
         SPACE 1                                                 92290  00445000
T3278MX  CRT   EDS=00,DEF=0000,ALT=0000,SW=03 QUERY; USE LARGEST 92290  00446000
T3278GX  CRT   EDS=80,DEF=0000,ALT=0000,SW=03 QUERY; USE LARGEST 92290  00447000
T3279MX  CRT   EDS=00,DEF=0000,ALT=0000,SW=03 QUERY; USE LARGEST 92290  00448000
T3279GX  CRT   EDS=80,DEF=0000,ALT=0000,SW=03 QUERY; USE LARGEST 92290  00449000
T3180MX  CRT   EDS=80,DEF=0000,ALT=0000,SW=03 QUERY; USE LARGEST 92290  00450000
T3290    CRT   EDS=80,DEF=0000,ALT=0000,SW=03 QUERY; USE LARGEST 92290  00451000
         SPACE 1                                                 85354  00452000
T3277M2  CRT   DEF=0000,ALT=0000,SW=02,SEC=90  3277-2 SNA !      86167  00453000
T3277M1  CRT   SW=01                  3277-1 SNA !!              86167  00454000
         SPACE 2                                                        00455000
*        NTO2 SUPPORTED TTYS                                            00456000
*                                                                       00457000
TTY      MODEENT LOGMODE=TTY,FMPROF=X'03',TSPROF=X'03',PRIPROT=X'B1',  *00458000
               SECPROT=X'90',COMPROT=X'3040',                    86128 *00459000
               PSERVIC=X'010000000000000000000000'               92071  00460000
         SPACE 1                                                 92071  00461000
*        NTO ENTRY FOR SSI (SUPER)WYLBUR                         92071  00462000
SWYLNTO1 MODEENT LOGMODE=SWYLTTY,FMPROF=X'03',TSPROF=X'03',            *00463000
               PRIPROT=X'B1',SECPROT=X'B1',COMPROT=X'3080',            *00464000
               PSERVIC=X'010000000000000000000000',RUSIZES=X'8585'      00465000
         SPACE 1                                                 88248  00466000
*        NTO ENTRY FOR RELAY/3270 (RELAY GOLD)                   88248  00467000
B3767LU1 MODEENT LOGMODE=RLY3767,FMPROF=X'03',TSPROF=X'03',            *00468000
               PRIPROT=X'B1',SECPROT=X'A0',COMPROT=X'3040',            *00469000
               PSERVIC=X'010000000000000000000000'               88248  00470000
         SPACE 1                                                 90147  00471000
*        CRT ENTRY FOR LANDMARK'S THE MONITOR FOR CICS (TMON)    90147  00472000
T3279M21 MODEENT LOGMODE=T3279M21,FMPROF=X'03',TSPROF=X'03',           *00473000
               PRIPROT=X'B1',SECPROT=X'90',COMPROT=X'3080',            *00474000
               RUSIZES=X'87C7',PSERVIC=X'028000000000185018507F00'      00475000
         SPACE 2                                                 90123  00476000
*        LU 6.2 AND APPC SESSION DEFINITION                           * 00477000
*                                                                     * 00478000
SNASVCMG MODEENT LOGMODE=SNASVCMG,FMPROF=X'13',TSPROF=X'07',           *00479000
               PRIPROT=X'B0',SECPROT=X'B0',COMPROT=X'D0B1',            *00480000
               RUSIZES=X'8585', ENCR=B'0000',                          *00481000
               PSERVIC=X'060000000000000000000300'               93260  00482000
         EJECT                                                          00483000
         MODEEND ,                                                      00519000
         END   ,                                                        00520000
/*
//LKED   EXEC  PGM=IEWL,PARM='XREF,LIST,LET,CALL,AC=0',REGION=512K
//*            COND=(4,LT,ASM2)
//SYSLIN DD    DSNAME=&&TEMP,DISP=(OLD,DELETE)
//SYSLMOD DD DISP=SHR,DSN=SYS1.LPALIB(BSPLMT02)
//SYSUT1 DD    DSNAME=&SYSUT1,UNIT=(SYSDA),
//             SPACE=(1024,(50,20))
//SYSPRINT DD  SYSOUT=A
//SYSLIB DD DUMMY
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


### NCP GEN BSC

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
NCPHOST  HOST  INBFRS=10,          INITIAL 3705 ALLOCATION             X
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
*      SPECIFICATIONS FOR BSC LEASED LINES                            *
*      GROUP MACRO SPECIFICATIONS                                     *
***********************************************************************
BSC3270 GROUP  LNCTL=BSC,          SYNCHRONOUS DATA LINK               X
               DIAL=NO,            REQUIRED FOR LEASED LINE            X
               TYPE=NCP,           NCP ONLY                            X
               TRANSFR=5,          LIMIT NUMBER OF RECEIVE BUFFERS     X
               CUTOFF=1,           LIMIT NUMBER OF SUBBLOCKS           X
               CRETRY=7,           TIME OUT WILL TAKE 63 SECONDS       X
               XMITLIM=1,          TRANSMISSION LIMIT                  X
               REPLYTO=1           REPLY TIMEOUT
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
BSC01    LINE  ADDRESS=020,        TRANSMIT AND RECEIVE ADDRESSES      X
               DUPLEX=HALF,        MODEM IS STRAPPED FOR HALF DUPLEX   X
               SPEED=9600,         SPEED MAY BE HIGHER (SEE NOTES)     X
               NEWSYNC=NO,         CHECK MODEM REQUIREMENTS            X
               CLOCKNG=EXT,        MODEM PROVIDES CLOCKING             X
               NEGPOLP=.1,         NEGATIVE POLL PAUSE                 X
               POLLED=YES,         POLLED DEVICE                       X
               RETRIES=(5,10,4),   5 RETRIES PER RECOVERY SEQUENCE     X
               ISTATUS=ACTIVE,     ACF/VTAM USE ONLY                   X
               CODE=EBCDIC,        EBCDIC 3270'S ONLY                  X
               INTPRI=1,           INTERRUPT PRIORITY IS 1             X
               PAUSE=1,            DELAY BETWEEN SERVICE CYCLES        X
               SERVPRI=OLD,        PRIORITY TO OLD SESSIONS            X
               SESSION=1,          SPECIFY 1 FOR EACH CLUSTER          X
               POLIMIT=(1,QUEUE),                                      X
               SSCPFM=USS3270,                                         X
               USSTAB=BSPUDT01
*
         SPACE 2
***********************************************************************
*      SERVICE ORDER FOR BSC LINK                                     *
***********************************************************************
         SERVICE ORDER=(BSC3274,BSCTERM1)
         EJECT
***********************************************************************
*      CLUSTER SPECIFICATIONS                                         *
***********************************************************************
BSC3274 CLUSTER  CUTYPE=3271,                                          X
               FEATUR2=(MODEL2,ANKEY,PFK),                             X
               GPOLL=40407F7F,                                         X
               MODETAB=BSPLMT02
         SPACE 2
***********************************************************************
*      TERMINAL SPECIFICATIONS                                        *
***********************************************************************
BSCTERM1 TERMINAL TERM=3277,                                           X
               FEATUR2=(MODEL2,ANKEY,PFK),                             X
               ISTATUS=INACTIVE,                                       X
               ADDR=60604040,                                          X
               POLL=40404040,                                          X
               LGRAPHS=(ACCEPT,ACCEPT)
         EJECT
***********************************************************************
*      GENEND DELIMITER                                               *
***********************************************************************
         GENEND
         END

``` 
```
CMD V NET,ACT,ID=T3278S31,LOGON=SNASOL,LOGMODE=MHP3278E
```

### Links

* [VTAM System Programmers Guide](http://www.bitsavers.org/pdf/ibm/sna/acf/SC38-0258-1_ACF_VTAM_System_Programmers_Guide_197805.pdf)
