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

#ifndef _inc_dca_type_h
#define _inc_dca_type_h

/***********************************************************************/
/* Function prototypes from module DCA_MAIN.C */
/***********************************************************************/

/* Required for proper function exports.  Since I want to use explicit
   function exports with Borland compiler the only way to then generate
   the proper prolog/epilog code is to use the _export keyword

   However, the use of the _export keyword upsets the compiler slightly
   and it starts to WHINE about incorrect type definitions when an exported
   function is used as a parameter. i.e. MakeProcInstance (type trouble, xxx)
*/
//  SDH - 02/06/95 - Make function declaration portable.
//  long FAR PASCAL TW_MainWndProc(HWND, WORD, WORD, LONG);
//  BOOL FAR PASCAL AboutDlgProc(HWND, WORD, WORD, LONG);
LRESULT FAR PASCAL TW_MainWndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR FAR PASCAL AboutDlgProc(HWND, UINT, WPARAM, LPARAM);

/* The routines in DLL must have a function prototype as well as information
   from the import.lib to statisfy the linker.

   Nice to catch parms passing errors with compiler
*/
BOOL InitInstance (HANDLE, int);
BOOL InitTWMain (HANDLE);
VOID CleanUpApp (VOID);
VOID CleanKillApp (VOID);
WORD DibNumColors (VOID FAR *);
HPALETTE CreateBIPalette (LPBITMAPINFOHEADER);
HBITMAP FixUp (HWND, HANDLE);


// Defines for DCA_MAIN.C
#define PALVERSION      0x300
#define MAXPALETTE      256      /* max. # supported palette entries */
#define LOWOVERHEAD     100     // different call in main event loop uses flags
                                // vs. glue code call, faster??
#endif //_inc_dca_type_h
