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

#ifndef _inc_dca_glue_h
#define _inc_dca_glue_h

// Globals need in other modules
extern DSMENTRYPROC lpDSM_Entry;       // function pointer to Source Mang. entry
extern TW_IDENTITY  appID, dsID;       // access to identity structs from glue
extern TW_STATUS gGlobalStatus;

/* Defines for the current_state table used in Explain_Error */
#define E_CLOSEDSM          (char *)"Error Closing DSM.  "
#define E_CLOSEDS           (char *)"Error Closing DS.  "
#define E_USERSELECT        (char *)"Error Accessing DS.  "
#define E_SETUPMEMXFER      (char *)"Error Setting up memory transfer.  "
#define E_DISABLEDS         (char *)"Error Disabling DS.  "
#define E_ENABLEDS          (char *)"Error Enabling DS.  "
#define E_GETFIRST          (char *)"Error Getting first Data Source.  "
#define E_GETNEXT           (char *)"Error Getting Next Data Source.  "
#define E_CAPPIXELGET       (char *)"Error Getting Cap Pixel Type.  "
#define E_CAPPIXELSET       (char *)"Error Setting Cap Pixel Type.  "
#define E_CAPABILITY		    (char *)"Error Setting Capability.  "


/***********************************************************************/
/* Function prototypes from module DCA_GLUE.C */
/***********************************************************************/
// Candy routines
VOID TWInitialize (HWND);

// Routines for DSM
BOOL TWOpenDSM (VOID);
BOOL TWCloseDSM (HANDLE bitmap);
BOOL TWIsDSMOpen (VOID);

// Routines for DS
BOOL TWOpenDS (VOID);
BOOL TWCloseDS (VOID);
BOOL TWIsDSOpen (VOID);

BOOL TWEnableDS (TW_BOOL);
BOOL TWEnableDSUIOnly();
BOOL TWDisableDS (VOID);
BOOL TWIsDSEnabled (VOID);

BOOL TWSelectDS (VOID);

VOID GreyMenu (int);
TW_UINT16 CallDSMEntry(pTW_IDENTITY, pTW_IDENTITY, TW_UINT32,
										TW_UINT16, TW_UINT16, TW_MEMREF);


#define VALID_HANDLE    32      // valid windows handle SB >= 32

#endif //_inc_dca_glue_h
