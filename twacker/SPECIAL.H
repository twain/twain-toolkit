/* -*-C-*-
********************************************************************************
*
* File:         special.h
* RCS:          $Header: $
* Description:  Exerciser application program for Twain
* Author:       TWAIN Working Group
* Created:      Jan 15,92
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
********************************************************************************
*/

#ifndef _inc_special_h
#define _inc_special_h

#define IDLISTSELBOX	1075
#define IDEDITBOX		  1076


/*
----------------------------------------------------------------------
							V a r i a b l e s
----------------------------------------------------------------------
*/

extern TW_STR255	IniFile;
extern HANDLE		hStruct;
extern LPSTR		pStruct;
extern HANDLE   	hInst;             // current instance
extern HCURSOR		hWait;
extern HCURSOR		hReady;
extern TW_UINT32 uiNumItems;
extern HWND g_hMainDlg;

/*
----------------------------------------------------------------------
							F u n c t i o n s
----------------------------------------------------------------------
*/
//  SDH - 02/06/95 - Make function declaration portable.
//  int far pascal 	SendDlgProc
//  	(HWND hDlg,WORD wMsg,WORD wParam, long lParam);
//  BOOL FAR PASCAL 	AboutDlgProc
//  	(HWND hDlg, WORD message, WORD wParam, LONG lParam);
INT_PTR FAR PASCAL SendDlgProc (HWND hDlg, 
                             UINT wMsg,
                             WPARAM wParam,
                             LPARAM lParam);
INT_PTR FAR PASCAL AboutDlgProc (HWND hDlg,
                              UINT message,
                              WPARAM wParam,
                              LPARAM lParam);
INT_PTR FAR PASCAL FrameDlgProc (HWND hDlgFrame,
                              UINT message,
                              WPARAM wParam,
                              LPARAM lParam);

TW_BOOL MatchTwainInt(pTABLEENTRY pTable, TW_UINT32 uiTableSize,
												TW_INT32 uiCapId, LPSTR pString);

void SendTwain (HWND hWnd);
void RemoveEquals(char string[], LONG count);
BOOL ControlMsg(TW_UINT16 dat,TW_UINT16 msg,HWND hDlg);
BOOL ImageMsg(TW_UINT16 dat,TW_UINT16 msg,HWND hDlg);
BOOL AudioMsg(TW_UINT16 dat,TW_UINT16 msg,HWND hDlg);
BOOL GetResponse(HWND hDlg, pTW_GRAYRESPONSE resp,TW_UINT16 bits);
BOOL DisplayResponse(HWND hDlg,pTW_GRAYRESPONSE resp,TW_UINT16 bits);

#endif //_inc_special_h
