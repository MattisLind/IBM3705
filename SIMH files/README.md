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
