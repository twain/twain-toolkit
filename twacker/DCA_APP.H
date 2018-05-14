/***********************************************************************
 TWAIN source code:
 Copyright (C) '91-'92 TWAIN Working Group:
 Aldus, Caere, Eastman-Kodak, Logitech,
 Hewlett-Packard Corporations.
 All rights reserved.

*	Copyright © 1998 TWAIN Working Group: Adobe Systems Incorporated, 
*	Canon Information Systems, Eastman Kodak Company, 
*	Fujitsu Computer Products of America, Genoa Technology, 
*	Hewlett-Packard Company, Intel Corporation, Kofax Image Products, 
*	JFL Peripheral Solutions Inc., Ricoh Corporation, and Xerox Corporation.  
*	All rights reserved.
*************************************************************************/

#ifndef _inc_dca_app_h
#define _inc_dca_app_h

#define MAX_TWPATH 260

// Generic access to TWAIN DSM
#define  IDS_DSMNAME        0
#define  IDS_WINDOWTITLE    1

// Resource based standard App error messages
#define IDS_NODSM           16
#define IDS_NODS            17
#define IDS_LOWMEMORY       18
#define IDS_MAXCONNECTIONS  19
#define IDS_BUMMER          20
#define IDS_BADCAP          21
#define IDS_BADPROTOCOL     22
#define IDS_BADVALUE        23
#define IDS_OPERATIONERROR  24
#define IDS_SEQERROR        25
#define IDS_SUCCESS         26
#define IDS_NODETAIL        27
#define IDS_DEFAULT         28   
#define IDS_BADDEST         29

#define MM_FILE             0
#define MM_SPECIAL          1
#define MM_DG               2
#define MM_DAT              3
#define MM_MSG              4
#define MM_SEND             5
#define MM_HELP             6

#define TW_APP_ACQUIRE          100
#define TW_APP_SELECT_SOURCE    101
#define TW_APP_NATIVE           110
#define TW_APP_FILE             111
#define TW_APP_BUFFER           112
#define TW_APP_SHOW             120
#define TW_APP_DISABLE_AFTER    121
#define TW_APP_QUIT             130
#define TW_APP_SETSOURCE        131

#define TW_APP_MSG_NONE         150
#define TW_APP_MSG_ERROR        151
#define TW_APP_MSG_INFO         152
#define TW_APP_MSG_FULL         153


#define TW_APP_AUTOFEED         160

#define TW_APP_RUN_TEST         180

#define TW_APP_OPENSM           201
#define TW_APP_OPENDS           202
#define TW_APP_SEND             203
#define TW_APP_ENABLE           204
#define TW_APP_TRANSFER         205
#define TW_APP_DISABLE          206
#define TW_APP_CLOSEDS          207
#define TW_APP_CLOSESM          208

#define TW_DLG_AUTOTEST         209
#define TW_AUTOTESTS_RUN        210
#define TW_AUTOTESTS_CREATE     211        
#define TW_AUTOTESTS_STEP       212        
#define IDC_COMMENTS            213

#define IDC_ACQUIRE             1014 
#define IDC_SELECT_SOURCE       1012

//#ifndef WIN32  // temp fix until re-calibrate the .rc files
    #define IDC_RUNSCRIPTS          214 
    #define IDC_BREAKSCHECK         215  
    #define IDC_TESTLIST            216
    #define IDADD                   217
    #define IDDELETE                218
    #define IDOPEN                  219
    #define IDC_PATH                220
//#endif

#define TW_AUTOTESTS_LOG        221
#define TW_AUTOTESTS_STOP       222

#define TW_DLG_CREATE           300

#define TW_DLG_CHECK            400
#define IDC_ACTION              401
#define IDC_CAPABILITY          402
#define IDC_L_CONTAINER         403
#define IDC_L_CAP1              404
#define IDC_L_CAP2              405
#define IDC_L_CAP3              406
#define IDC_L_CAP4              407
#define IDC_L_CAP5              408
#define IDC_L_CAP6              409
#define IDC_R_CONTAINER         410
#define IDC_R_CAP1              411
#define IDC_R_CAP2              412
#define IDC_R_CAP3              413
#define IDC_R_CAP4              414
#define IDC_R_CAP5              415
#define IDC_R_CAP6              416
#define IDC_R_CONTAINER2        417

#define TW_DLG_SETSOURCE        500
#define IDC_EDIT                501

#define TW_APP_ABOUT            700

#define TW_APP_ABOUT_CO         120
#define TW_APP_ABOUT_VER        121
#define TW_APP_ABOUT_COPYRIGHT  122
#define TW_APP_ABOUT_OKAY       123

#define DLG_SEND                100
#define IDDG                    101
#define IDDAT                   102
#define IDMSG                   103
#define IDCAPTEXT               104
#define IDCAP                   105
#define IDTYPETEXT              106
#define IDTYPE                  107
#define IDEDITTEXT              108
#define IDEDIT                  109
#define IDSTATUS                110
#define IDDEST                  111

#define  PM_XFERDONE            WM_USER + 0

#define OFF     0
#define ON      1

#define ATTENDED    0
#define UNATTENDED  1

#endif //_inc_dca_app_h
