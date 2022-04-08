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

### LOGMODE

Specifies the name of a logon mode table to be used to correlate each logon mode name with a set of session parameters for the logical unit. The name you code must be the name of a logon mode table created as described in Logon mode table. If you do not supply a logon mode table for the logical unit on the MODETAB operand, an IBM®-supplied default logon mode table (ISTINCLM) is used. If you specify a table, both the table you specify and the default table are used.

### DLOGMOD

Specifies the name of the logon mode table entry used by default if one is not otherwise provided. If you do not code this operand and the name of a logon mode table entry is not otherwise provided, VTAM® uses the first entry in the applicable logon mode table (specified on the MODETAB operand or used by default).

If you specify MODETAB, the entry must be in either the specified table or in ISTINCLM, an IBM®-supplied logon mode table. If you do not specify MODETAB, the entry must be in ISTINCLM.

A logon mode entry determines which entry in the applicable logon mode table is to provide a set of session parameters for the application program if the application program is a secondary logical unit (SLU). The name specified on the DLOGMOD operand must be the name of an entry in a logon mode table.

### USSTAB

Specifies the name of a USS table that VTAM® uses to process character-coded input that it receives from the logical unit.

A terminal user can issue a USS command by coding the LANGTAB operand. This causes a second USS table to be associated with the logical unit, which overrides the table specified with USSTAB. If you do not code USSTAB and a LANGTAB USS table is not in use, the IBM®-supplied USS table (ISTINCDT) is used. For more information on USS tables, see Unformatted system services tables.
