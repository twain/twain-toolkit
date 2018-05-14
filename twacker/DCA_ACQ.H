//********************************************************************************
//*
//* File:         dca_acq.h
//* RCS:          $Header: $
//* Description:  Acquire code
//* Author:       TWAIN Working Group
//* Created:      Feb 5,92
//* Modified:     "
//* Language:     C
//* Package:      N/A
//* Status:       Test tool
//*
//* (c) Copyright 1992, Hewlett-Packard Company, all rights reserved.
//*
//*	Copyright © 1998 TWAIN Working Group: Adobe Systems Incorporated, 
//*	Canon Information Systems, Eastman Kodak Company, 
//*	Fujitsu Computer Products of America, Genoa Technology, 
//*	Hewlett-Packard Company, Intel Corporation, Kofax Image Products, 
//*	JFL Peripheral Solutions Inc., Ricoh Corporation, and Xerox Corporation.  
//*	All rights reserved.
//*
//********************************************************************************

#ifndef _inc_dca_acq_h
#define _inc_dca_acq_h

//----------------------------------------------------------------------
//							V a r i a b l e s
//----------------------------------------------------------------------

extern char				Filename[256];

//----------------------------------------------------------------------
//							F u n c t i o n s
//----------------------------------------------------------------------

BOOL TWAcquire (HWND hWnd, BOOL Show, TW_INT16 Flag);
BOOL TWSetup (HWND hWnd, TW_INT16 Flag);
BOOL ProcessTWMessage (LPMSG, HWND);
void TWTransferImage (HWND hWnd);
void LogMessage(char msg[]);
void CloseConnection(HANDLE bitmap);
void SetAcqFlag (TW_INT16 Flag); 
BOOL IsSampleSourceActive();

#endif //_inc_dca_acq_h

