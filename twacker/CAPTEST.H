/* -*-C-*-
********************************************************************************
*
* File:         captest.h
* RCS:          $Header: $
* Description:  Capability negotiation for Twacker
* Author:       TWAIN Working Group
* Created:      Feb 5,92
* Modified:     "
* Language:     C
* Package:      N/A
* Status:       Test tool
*
* (c) Copyright 1992, Hewlett-Packard Company, all rights reserved.
*
*	Copyright © 1998 TWAIN Working Group: Adobe Systems Incorporated, 
*	Canon Information Systems, Eastman Kodak Company, 
*	Fujitsu Computer Products of America, Genoa Technology, 
*	Hewlett-Packard Company, Intel Corporation, Kofax Image Products, 
*	JFL Peripheral Solutions Inc., Ricoh Corporation, and Xerox Corporation.  
*	All rights reserved.
*
********************************************************************************
*/

#ifndef _inc_captest_h
#define _inc_captest_h

/*
----------------------------------------------------------------------
							F u n c t i o n s
----------------------------------------------------------------------
*/

TW_INT16 TWXferMech (HWND hWnd);
TW_INT16 TWAutofeedMenu (HWND hWnd);
BOOL AutoFeedOn (VOID);
float FIX32ToFloat (TW_FIX32 fix32);
TW_FIX32 FloatToFIX32 (float floater);
void ShowImageInfo (HWND hWndofApp);
void ShowImageLayout (HWND hWndofApp);
void ShowCapability (HWND hWndofApp, TW_UINT16 Cap);
void ShowPendingXfers (HWND hWndofApp);
void ShowRC_CC (HWND hWndofApp, TW_UINT16 DisplayRC, TW_UINT16 ReturnCode, 
		TW_UINT16 Dest, char * Text, char * Title);
void ShowTW_ID (HWND hWndofApp, TW_IDENTITY twID, char *text);
void SetMessageLevel (int Level);
int MessageLevel(VOID);
BOOL MsgQuerySupport(TW_UINT16 cap,TW_UINT16 type, pTW_CAPABILITY ptwCapability);
BOOL MsgGetCapability(TW_UINT16 cap,TW_UINT16 type, pTW_CAPABILITY ptwCapability);
BOOL MsgSetCapability(TW_UINT16 cap,TW_UINT16 type, pTW_CAPABILITY ptwCapability);

//Values for MessageLevel
//Note tests of message level are done >= ML_XXXX
#define ML_NONE 	0
#define ML_ERROR	1
#define ML_INFO		2
#define ML_FULL 	3

#endif //_cap_test_h
