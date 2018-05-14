// -*-C-*-
////////////////////////////////////////////////////////////////////////////////
//
//File:         twacker.h
//RCS:          $Header: $
//Description:  Exerciser application program for Twain
//Author:       TWAIN Working Group
//Created:      Jan 15,92
//Modified:     "
//Language:     C
//Package:      N/A
//Status:       Test tool
//
//(c) Copyright 1992, Hewlett-Packard Company, all rights reserved.
//
/*	Copyright © 1998 TWAIN Working Group: Adobe Systems Incorporated, 
*	Canon Information Systems, Eastman Kodak Company, 
*	Fujitsu Computer Products of America, Genoa Technology, 
*	Hewlett-Packard Company, Intel Corporation, Kofax Image Products, 
*	JFL Peripheral Solutions Inc., Ricoh Corporation, and Xerox Corporation.  
*	All rights reserved.
*/
//

#ifndef _inc_twacker_h
#define _inc_twacker_h

#if defined(WIN32) || defined(WIN64)
#define HUGE
#else
#define HUGE huge
#endif //_WIN32

//
//--------------------------------------------------------------------
//					V a r i a b l e s
//--------------------------------------------------------------------

extern BOOL g_bSpecialMenu;
extern BOOL g_bTWAIN20;

//
//--------------------------------------------------------------------
//					F u n c t i o n s
//--------------------------------------------------------------------
//

TW_INT16 InitTwainCombo(HWND hDlg, TW_INT16 Id, 
												pTABLEENTRY pTable, int nSizeTable);
TW_INT16 InitTwainList	(HWND hDlg, TW_INT16 Id, 
												pTABLEENTRY pTable, int nSizeTable);

void CheckSpecialMenu
	(HWND hWndofApp, TW_INT16 CkMenu);
void CheckIfAcquire(char attribute[]);
void OnScrollBar(HWND, int, UINT, int, SCROLLINFO*);
void OnKeyDown(HWND, UINT);
BOOL CloseApplication(HWND hWnd);
void DrawScrollBar(HWND, SCROLLINFO*, int, BITMAP);
void SaveBitmap(HWND, HGLOBAL _hDIB);
HGLOBAL CloneDib(HGLOBAL _hDIB);

extern TW_ENTRYPOINT g_DSM_Entry;
void setEntryPoints(pTW_ENTRYPOINT _pEntryPoints);
TW_HANDLE DSM_Alloc(TW_UINT32 _size);
void DSM_Free(TW_HANDLE _pPtr);
TW_MEMREF DSM_LockMemory(TW_HANDLE _pMemory);
void DSM_UnlockMemory(TW_MEMREF _pMemory);
SIZE_T DSM_Size(TW_HANDLE _pMemory);
TW_HANDLE DSM_ReAlloc(TW_HANDLE _pMemory, TW_UINT32 _size);
extern HGLOBAL _ghDIB; // handle to the current DIB

#endif //_inc_twacker_h

