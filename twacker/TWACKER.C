// File:         twacker.c
// RCS:          $Header: $
// Description:  Exerciser application program for Twain
// Author:       TWAIN Working Group
// Created:      Jan 15,92
// Modified:     "
// Language:     C
// Package:      N/A
// Status:       Test tool
//
// (c) Copyright 1992, Hewlett-Packard Company, all rights reserved.
//
/*
*	Copyright © 1998 TWAIN Working Group: Adobe Systems Incorporated, 
*	Canon Information Systems, Eastman Kodak Company, 
*	Fujitsu Computer Products of America, Genoa Technology, 
*	Hewlett-Packard Company, Intel Corporation, Kofax Image Products, 
*	JFL Peripheral Solutions Inc., Ricoh Corporation, and Xerox Corporation.  
*	All rights reserved.
*/
//----------------------------------------------------------------------
//                            I n c l u d e s
//----------------------------------------------------------------------

//  This is a TWAIN sample application.  This simple application
//  will display .BMP files into a dynamically resized window.  The handle
//  to the .BMP file is passed to this application via TWAIN protocol
//  from the underlying sample data source.
//
//  The application was written assuming the use of MS Windows version 3.0.
//  It further assumes operation in Standard or Enhanced mode, no attempt
//  has been made to fit it into Real mode.

#include <windows.h>         // Req. for twain.h type defs and ...
#include <math.h>
#include <string.h>
#include <stdlib.h>

#include "..\\twcommon\\twain.h"
#include "..\\twcommon\\twndebug.h"

#ifdef WIN64
#include "res_64.h"
#elif defined(WIN32)
#include "res_32.h"
#else
#include "res_16.h"
#endif


#include "table.h"
#include "dca_glue.h"        // for function prototypes of glue code
#include "dca_type.h"        // contains function protos for this module
#include "dca_acq.h"         // contains buffered glue code function prototypes
#include "dca_app.h"
#include "special.h"
#include "twacker.h"
#include "captest.h"

//  SDH - 01/30/95: NULL under C with this compiler is void* not 0 which causes a
//  compiler warning.  Rather than change every line, we kludge this here.
#ifdef NULL
#undef NULL                      
#endif
#define NULL 0

//----------------------------------------------------------------------
//                            V a r i a b l e s
//----------------------------------------------------------------------

// Global variables
HWND hMainWnd = NULL;							// main app window handle
HGLOBAL _ghDIB = NULL; // handle to the current DIB
static HBITMAP hbm = NULL;				// holder handle to bit map passed from Source
static HPALETTE hDibPal = NULL;		// handle to palette
char MainTitle[40];
BOOL CapSupportedCaps = FALSE;
//BOOL done = FALSE;
int nPosition = 0;					/* Keep the scrollbar position */
BOOL	bSelection = TRUE;		/* To know the scrollbar in use TRUE for VSCROLL */
POINT	Point = {0,0};				/* Coordinate for the upper-left corner */
POINT PtV = {0,0}, PtH = {0,0};
RECT Rect, rc;
SCROLLINFO sbInfoV, sbInfoH;
BOOL bHideV = FALSE;
BOOL bHideH = FALSE;
BOOL g_bSpecialMenu  = FALSE;
#ifdef WIN64
BOOL g_bTWAIN20  = TRUE;
#else
BOOL g_bTWAIN20  = FALSE;
#endif
//----------------------------------------------------------------------
//                            F u n c t i o n s
//----------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////
// FUNCTION: WinMain
//
// ARGS:    hInstance       handle to current instance
//          hPrevInstance   handle to previous instance, if any
//          lpszCmdLine     ptr to the command line string
//          CmdShow         flag passed in from command line
//
// RETURNS: wParam
//
// NOTES:   This is the main windows procedure.      
//
int PASCAL WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, //HANDLE
            LPSTR  lpszCmdLine, int CmdShow)
{
	MSG msg;
	OFSTRUCT of;

	memset(&msg, 0, sizeof(MSG));
	memset(&of, 0, sizeof(OFSTRUCT));

	ASSERT(hInstance);
	ASSERT(lpszCmdLine);

	// erase the twacker.log file
	OpenFile("c:\\twacker.log",&of,OF_DELETE);
	OpenFile("c:\\twsrc.log",&of,OF_DELETE);

	// NOTE: this allows multiple instances at the same time

	if (!InitTWMain(hInstance))
	{
		return (FALSE);
	}

	if (!InitInstance(hInstance, CmdShow))
	{
		return (FALSE);
	}

	// Initialize 3D engine
	//Ctl3dRegister(hInstance);
	//Ctl3dAutoSubclass(hInstance);

	// The main loop of the program.  Get a Message; if it's not an
	// accelerator key, translate it and dispatch it to the application.
	#ifndef LOWOVERHEAD
	while (GetMessage((LPMSG)&msg, NULL, 0, 0))
	{
		if (!ProcessTWMessage ((LPMSG)&msg, hMainWnd))
		{
			TranslateMessage ((LPMSG)&msg);
			DispatchMessage ((LPMSG)&msg);
		}
	}                     
	#else
	while (GetMessage((LPMSG)&msg, NULL, 0, 0))
	{
		if ((!TWIsDSOpen()) || (!ProcessTWMessage ((LPMSG)&msg, hMainWnd)))
		{   
			TranslateMessage ((LPMSG)&msg);
			DispatchMessage ((LPMSG)&msg);
		}
	}
	#endif

	return (int)(msg.wParam);
} // WinMain

//////////////////////////////////////////////////////////////////////////
// FUNCTION: InitTWMain
//
// ARGS:    hInstance   handle to current instance
//
// RETURNS: TRUE/FALSE, status of class registration
//
// NOTES:   This function intializes and registers the class used by our
//          main window.
//
BOOL InitTWMain(HANDLE hInstance)
{
	WNDCLASS  wc;

	memset(&wc, 0, sizeof(WNDCLASS));

	ASSERT(hInstance);

	// Set up application's main window
	wc.style         = NULL;                            // no style bits
	wc.lpfnWndProc   = (WNDPROC)TW_MainWndProc;         // name of window proc
	wc.cbClsExtra    = 0;                               // no extra bits
	wc.cbWndExtra    = 0;
	wc.hInstance     = hInstance;                       // handle to cur instance
	wc.hIcon         = LoadIcon( hInstance, "TW_APP_ICO"); // load resources
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);     // load mouse icon
	wc.hbrBackground = GetStockObject(WHITE_BRUSH);     // use white backgnd
	wc.lpszMenuName  = NULL;                            // no menu
	wc.lpszClassName = "TW_App_MainWnd";                // class named

	return (RegisterClass(&wc));
} 

///////////////////////////////////////////////////////////////////////////////
// FUNCTION: InitInstance
//
// ARGS:    hInstance   handle to current instance
//          CmdShow     flag from command line used for window visiblity
//
// RETURNS: TRUE, successful initialization
//
// NOTES:   1). Create main window
//          2). initialize applications menus
//          3). register application particulars with the glue code module
//
BOOL InitInstance(HANDLE hInstance, int CmdShow)
{
	HWND        hWnd = NULL;
	HMENU       hMenu = NULL;
	TW_STR64    WindowTitle;
	TW_STR32    WindowHandle;

	memset(WindowTitle, 0, sizeof(TW_STR64));
	memset(WindowHandle, 0, sizeof(TW_STR32));

	ASSERT(hInstance);

	// Save the instance handle in static variable, which will be used in
	// many subsequence calls from this application to Windows.
	hInst = hInstance;
	hWait = LoadCursor(NULL, IDC_WAIT);
	hReady = LoadCursor(NULL, IDC_ARROW);

	// change the title if set through running parameters
	memset(WindowTitle,'\0',sizeof(WindowTitle));
	if (MainTitle[0] != '\0')
	{
		memcpy(WindowTitle,MainTitle,strlen(MainTitle));
	}
	else
	{
		LoadString(hInstance, IDS_WINDOWTITLE, WindowTitle,  sizeof(WindowTitle));
	}

	hWnd = CreateWindow("TW_App_MainWnd", WindowTitle, WS_VSCROLL | 
	WS_HSCROLL | WS_OVERLAPPEDWINDOW,
	0, 0, 500, 350, NULL, NULL, hInstance, NULL);

	// Save a copy of the main window handle globally
	hMainWnd = hWnd;
	GetWindowText(hWnd, WindowTitle, sizeof(TW_STR64));
	wsprintf(WindowHandle, "");
	lstrcat(WindowTitle, WindowHandle);
	SetWindowText(hWnd, WindowTitle);

	// all versions of twacker will share an ini file in the Windows directory
	// if fails, set to a reasonable default value
	if (GetWindowsDirectory(IniFile, sizeof(IniFile)) == 0)
	{                            
		ShowRC_CC(hWnd,0,0,0,"Unable to determine Windows directory",
				"Application Error");
		lstrcpy(IniFile,"c:\\windows");    
	}
	lstrcat(IniFile,"\\twacker.ini");

	// Add a menu to the window
	hMenu = LoadMenu (hInstance, "TW_APP_MENU");
	SetMenu (hWnd, hMenu);

	if (!hWnd)
	{
		return (FALSE);
	}

	// Set up the information about your application which you want to pass to
	// the SM in this call.
	//
	// Move all these text strings off to a resource fill for a real application.
	// They are here for easier readablilty for the student.


	// pass app particulars to glue code
	TWInitialize (hWnd);
	SetMessageLevel(ML_ERROR);
	SetWindowPos (hWnd, (HWND)NULL,0,0,400,300,SWP_NOMOVE|SWP_NOZORDER);
	ShowWindow (hWnd, CmdShow);
	UpdateWindow (hWnd);
	return (TRUE);
} 

/////////////////////////////////////////////////////////////////////////////////////
// FUNCTION: TW_MainWndProc
//
// ARGS:    hWnd        handle to main app window
//          iMessage    windows message
//          wParam      1st windows parm (message specific information)
//          lParam      2nd windows parm
//
// RETURNS: 0L, indicates success
//
// NOTES:   This routine services messages from windows event queue
//          1). messages serviced:
//              - WM_CLOSE  clean up after App, Source and Source Manager
//              - WM_ENDSESSION as above
//              - WM_PALETTECHANGING
//              - WM_PALETTECHANGED be aware of others changing palette
//                                  to redraw your bitmap
//              - WM_PAINT  support painting of select bitmap in a window
//              - WM_COMMAND decode App menu items
//                  - TW_APP_AQUIRE         get input from current Source
//                  - TW_APP_SELECT_SOURCE  select/view available Source's
//                  - TW_APP_QUIT           exit the App, and graceful close
//                  - TW_APP_ABOUT          display App about box
//
// Note: use of the keyword _export on TW_MainWndProc causes a Borland
// compiler warning message, 'Suspicious pointer conversion'
//
LRESULT FAR PASCAL TW_MainWndProc (HWND hWnd,
                                   UINT iMessage,
                                   WPARAM wParam,
                                   LPARAM lParam)
{
	PAINTSTRUCT     ps;
	HDC             hDC = NULL, hMemDC = NULL;
	BITMAP          bm;
	DLGPROC         lpProcAbout = NULL;        // pointer to the "About" function
	//FARPROC         lpProcAuto = NULL;
	int             ret = 0;
	static int      FirstDraw = 0;          // flag used to control window redraw
	TW_UINT16 wmId = 0;
	TW_UINT16 wmEvent = 0;
  #if defined(WIN32) || defined(WIN64)
		wmId = LOWORD(wParam);
		wmEvent = HIWORD (wParam);
	#else
		wmId = wParam;
		wmEvent = HIWORD (lParam);
	#endif

	memset(&ps, 0, sizeof(PAINTSTRUCT));
	memset(&bm, 0, sizeof(BITMAP));

	ASSERT(hWnd);

	switch (iMessage)
	{
//-----------------------------------------------------------------
		case WM_CLOSE:
		case WM_ENDSESSION:
			// Always try to take SM and Source down with you
			if(CloseApplication(hWnd))
			{
				CleanKillApp();
				DestroyWindow(hMainWnd);
			}
			break;
        
		case WM_DESTROY:
			TWIsDSEnabled();	
			break;        
//-----------------------------------------------------------------
		case WM_PALETTECHANGED:
			InvalidateRect (hWnd, NULL, FALSE);
			break;

		//case WM_SYSCOLORCHANGE:
		//	Ctl3dColorChange();
		//	break;
//-----------------------------------------------------------------
		case PM_XFERDONE:            
			LogMessage("PM_XFERDONE\r\n");

			// Get rid of old image first, because this App only deals with one
			CleanUpApp ();

			// get private message, hbm in wParam, from DCA_ACQ.C convert from
			// DCA_ACQ.C returns a DIB, so change to DDB for sample App
			if (wParam != NULL)
			{
				_ghDIB = CloneDib((HGLOBAL)wParam);
        DSM_Free((HGLOBAL)wParam);
				if (_ghDIB == NULL)
				{
					if (MessageLevel() >= ML_ERROR)
					{
						ShowRC_CC(hWnd, 0, 0, 0,"Handle to BitMap Invalid","PM_XFERDONE");
					}
          break;
				}
				hbm = FixUp (hWnd, _ghDIB);

				// Test for valid handle
				if (hbm == NULL)
				{
					if (MessageLevel() >= ML_ERROR)
					{
						ShowRC_CC(hWnd, 0, 0, 0,"Handle to BitMap Invalid","PM_XFERDONE");
					}
				}

				FirstDraw = TRUE;

				InvalidateRect(hWnd,NULL,TRUE);
			}
			break;
//-----------------------------------------------------------------
		case WM_PAINT:
			{
        #if defined(WIN32) || defined(WIN64)
					int AddHeight = 0, AddWidth = 0;
				#endif

				GetWindowRect(hWnd, &Rect);

				hDC = BeginPaint(hWnd, &ps);

				if (hbm)
				{
					hMemDC = CreateCompatibleDC(hDC);

					/*
					* setup correct palette
					*/
					if (hDibPal)
					{
						SelectPalette (hDC, hDibPal, FALSE);
						SelectPalette (hMemDC, hDibPal, FALSE);
						RealizePalette (hDC);
					}   
             
					GetObject(hbm, sizeof(BITMAP), (LPSTR) &bm);

					if (FirstDraw==TRUE)
					{
						LogMessage("Painting object first time\r\n");

						SetWindowPos (hWnd, (HWND)NULL, Rect.left, Rect.top,
												Rect.right,Rect.bottom,
												SWP_NOMOVE | SWP_NOZORDER);

						// flag to adjust size only when first drawn
						ValidateRect (hWnd, NULL);
						FirstDraw = FALSE;
					}

					SelectObject(hMemDC, hbm);

          #if defined(WIN32) || defined(WIN64)
						/*
						* True is the vertical scroll bar action
						*/
						if (bSelection)
						{
							BitBlt(hDC, -Point.x, -sbInfoV.nPos, bm.bmWidth, 
										bm.bmHeight, hMemDC, 0, 0, SRCCOPY);
						}
						/*
						* False is the horizontal scroll bar action
						*/
						else
						{
							BitBlt(hDC, -sbInfoH.nPos, -Point.y, bm.bmWidth, 
										bm.bmHeight, hMemDC, 0, 0, SRCCOPY);
						}

						DrawScrollBar(hWnd, &sbInfoV, SB_VERT, bm);
						DrawScrollBar(hWnd, &sbInfoH, SB_HORZ, bm);

					#else //_WIN32

						/*
						* True is the vertical scroll bar action
						*/
						if (bSelection)
						{
							BitBlt(hDC, -Point.x, -nPosition, bm.bmWidth, 
										bm.bmHeight, hMemDC, 0, 0, SRCCOPY);
						}
						/*
						* False is the horizontal scroll bar action
						*/
						else
						{
							BitBlt(hDC, -nPosition, -Point.y, bm.bmWidth, 
										bm.bmHeight, hMemDC, 0, 0, SRCCOPY);
						}
						SetScrollRange(hWnd, SB_VERT, 0, bm.bmHeight, FALSE);
						SetScrollRange(hWnd, SB_HORZ, 0, bm.bmWidth, FALSE);

					#endif //_WIN32

					DeleteDC(hMemDC);

				}
				else
				{
					ShowScrollBar(hWnd, SB_VERT, FALSE);
					ShowScrollBar(hWnd, SB_HORZ, FALSE);
				}
				EndPaint(hWnd, &ps);	
			}	 
			break;
//-----------------------------------------------------------------
		case WM_SIZING:
			{
				LPRECT lprc = (LPRECT)lParam;
				LONG Width = 0;
				LONG Height = 0;
				int AddHeight = 0, AddWidth = 0;

				if (hbm)
				{
					GetWindowRect(hWnd, &Rect);
					GetObject(hbm, sizeof(BITMAP), (LPSTR)&bm);
				}
				else
				{
					bm.bmHeight = 0;
					bm.bmWidth = 0;
				}

				AddWidth = GetSystemMetrics(SM_CXVSCROLL);
				AddWidth += (2 * (GetSystemMetrics(SM_CYFRAME)));
				AddHeight = GetSystemMetrics(SM_CYMENU);
				AddHeight += GetSystemMetrics(SM_CYHSCROLL);
				AddHeight += GetSystemMetrics(SM_CYCAPTION);
				AddHeight += (2 * (GetSystemMetrics(SM_CXFRAME)));

				Height = Rect.top + bm.bmHeight;
				Width = Rect.left + bm.bmWidth;

				/*
				* Vertical scroll bar
				*/
				if (bm.bmWidth > 400)
				{
					if (lprc->right > Width)
					{
						if (lprc->bottom > Height)
						{
							AddWidth -= GetSystemMetrics(SM_CXVSCROLL);
							AddHeight -= GetSystemMetrics(SM_CYHSCROLL);							
						}
						bHideV = TRUE;
						lprc->right = Width + AddWidth;
					}
					else if(lprc->right < Rect.left + 400)
					{
						bHideV = TRUE;
						lprc->right = Rect.left + 400;
					}
					else
					{
						bHideV = FALSE;
					}
				}
				else
				{
					bHideV = TRUE;
					lprc->right = Rect.left + 400;
				}

				if(bHideV == FALSE)
				{
					SetScrollInfo(hWnd, SB_VERT, &sbInfoV, TRUE);
					ShowScrollBar(hWnd, SB_VERT, TRUE);
				}
				else
				{
					ShowScrollBar(hWnd, SB_VERT, FALSE);
				}

				/*
				* Horizontal scroll bar
				*/

				if(bm.bmHeight > 300)
				{
					if(lprc->bottom > Height)
					{
						bHideH = TRUE;
						lprc->bottom = Height + AddHeight;
					}
					else if(lprc->bottom < Rect.top + 300)
					{
						bHideH = TRUE;
						lprc->bottom = Rect.top + 300;
					}
					else
					{
						bHideH = FALSE;
					}
				}					
				else
				{
					bHideH = TRUE;
					lprc->bottom = Rect.top + 300;
				}

				if(bHideH == FALSE)
				{
					SetScrollInfo(hWnd, SB_HORZ, &sbInfoH, TRUE);
					ShowScrollBar(hWnd, SB_HORZ, TRUE);
				}
				else
				{
					ShowScrollBar(hWnd, SB_HORZ, FALSE);
				}
			}
			break;
//-----------------------------------------------------------------
		/*
		* Generate the vertical scroll bar codes.
		*/
		case WM_VSCROLL:
			bSelection = TRUE;
			OnScrollBar(hWnd, SB_VERT, LOWORD(wParam), 
								HIWORD(wParam), &sbInfoV);
      #if defined(WIN32) || defined(WIN64)
				InvalidateRect(hWnd, FALSE, FALSE);
			#else //_WIN32
				InvalidateRect(hWnd, &rc, FALSE);
			#endif //_WIN32
			break;
//-----------------------------------------------------------------
		/*
		* Generate the Horizontal scroll bar codes.
		*/
		case WM_HSCROLL:
			bSelection = FALSE;
			OnScrollBar(hWnd, SB_HORZ, LOWORD(wParam),
								HIWORD(wParam), &sbInfoH);
      #if defined(WIN32) || defined(WIN64)
				InvalidateRect(hWnd, FALSE, FALSE);
			#else //_WIN32
				InvalidateRect(hWnd, &rc, FALSE);
			#endif //_WIN32
			break;
//-----------------------------------------------------------------
		/*
		* Generate the key down codes.
		*/
		case WM_KEYDOWN:
			OnKeyDown(hWnd, LOWORD(wParam));
      #if defined(WIN32) || defined(WIN64)
				InvalidateRect(hWnd, FALSE, FALSE);
			#else //_WIN32
				InvalidateRect(hWnd, &rc, FALSE);
			#endif //_WIN32
			break;
//-----------------------------------------------------------------
		case WM_COMMAND:
			switch (wmId)
			{
//-----------------------------------------------------------------
				case ID_TWAIN2:
				{
					HMENU   hmenu = GetMenu(hWnd);
          g_bTWAIN20 = (GetMenuState(hmenu, ID_TWAIN2,    MF_BYCOMMAND) & MF_CHECKED)==0;
          CheckMenuItem(hmenu, ID_TWAIN2,g_bTWAIN20?MF_CHECKED:MF_UNCHECKED);
          TWInitialize(hMainWnd);
          break;
        }
				case TW_APP_ACQUIRE:
				{   
					TW_INT16 Flag = 0;
					HMENU   hmenu = GetMenu(hWnd);

					//Setup dsID for default Source                             
					if (!TWIsDSOpen())
					{
						dsID.Id = 0;
						dsID.ProductName[0] = 0;
						appID.Id = 0;   
					}
					//  1 Disable/CloseDS/CloseDSM
					//  3 Do Not Disable - only if ShowUI=TRUE
					Flag =                  
							(!(GetMenuState(hmenu, TW_APP_SHOW,    MF_BYCOMMAND) & MF_CHECKED)
							||
							(GetMenuState(hmenu, TW_APP_DISABLE_AFTER, MF_BYCOMMAND) & MF_CHECKED))
							? 1 : 3;

					if (TWAcquire(hWnd,GetMenuState(hmenu, TW_APP_SHOW,    MF_BYCOMMAND) & MF_CHECKED,Flag))  
					{
						CheckSpecialMenu(hWnd, TW_APP_ENABLE);      
						LogMessage("Acquire OK\r\n");
					}
					else  
					{
						LogMessage("Acquire returned false\r\n");
						//InvalidateRect(NULL,NULL,FALSE);    
					}
					break;
				}               
//-----------------------------------------------------------------
				case TW_APP_SETUP:
				{   
					TW_INT16 Flag = 0;
					HMENU   hmenu = GetMenu(hWnd);

					/*
					* Setup dsID for default Source                             
					*/
					if (!TWIsDSOpen())
					{
						dsID.Id = 0;
						dsID.ProductName[0] = 0;
						appID.Id = 0;   
					}
					/*
					* 1 Disable/CloseDS/CloseDSM
					* 3 Do Not Disable - only if ShowUI=TRUE
					*/
					Flag = 
							(!(GetMenuState(hmenu, TW_APP_SHOW,    MF_BYCOMMAND) & MF_CHECKED)
							||
							(GetMenuState(hmenu, TW_APP_DISABLE_AFTER, MF_BYCOMMAND) & MF_CHECKED))
							? 1 : 3;

					if (TWSetup(hWnd, Flag))  
					{
						CheckSpecialMenu(hWnd, TW_APP_ENABLE);      
						LogMessage("Setup OK\r\n");
					}
					else  
					{
						LogMessage("Setup returned false\r\n");
					}
					break;
				}               
//-----------------------------------------------------------------
				case TW_APP_SELECT_SOURCE:
					// SM userselect, or standard SM based user interface to Source's
					// - open dsm
					// - show user the data sources found by the SM
					//   allow user to pick a particular Source
					// - close the SM
					if (TWOpenDSM () == TRUE)
					{
						TWSelectDS ();
						TWCloseDSM (NULL);
						if (!TWIsDSMOpen())
						{
							CheckSpecialMenu(hWnd, TW_APP_CLOSESM);
						}
					}
					break;
//-----------------------------------------------------------------
				/*
				* For Save the Bitmap image
				*/
				case TW_APP_SAVE:
					SaveBitmap(hWnd, _ghDIB);
					break;
//-----------------------------------------------------------------
				case TW_APP_QUIT:
					// The Source asks to be closed when parent is killed but message is never
					// processed by our message loop.  
					if(CloseApplication(hWnd))
					{
						CleanKillApp();
						DestroyWindow(hMainWnd);
					}
					break;
//-----------------------------------------------------------------
				case TW_APP_OPENSM:
					{
						HMENU hmenu = GetMenu(hWnd);

						SetCursor(hWait);
						if (!TWIsDSMOpen())
						{
							if (TWOpenDSM())
							{
								GreyMenu (TRUE);
								CheckSpecialMenu(hWnd, TW_APP_OPENSM);
							}
						}
						else
						{
							if (MessageLevel()  >= ML_ERROR)
							{
								ShowRC_CC(hWnd, 0, 0, 0,
											"Source manager already open\r\n", 
											"Sequence Error");
							}
						}
						SetCursor(hReady);
					}   
					break;
//-----------------------------------------------------------------
				case TW_APP_OPENDS:
					{
						SetCursor(hWait);
						if (!TWIsDSOpen())
						{
							if (TWOpenDS())             
							{
								GreyMenu (TRUE);
								CheckSpecialMenu(hWnd, TW_APP_OPENDS);
							}
						}
						else
						{
							if (MessageLevel()  >= ML_ERROR)
							{
								ShowRC_CC(hWnd, 0, 0, 0,
											"Source already open\r\n", 
											"Sequence Error");
							}
						}
						SetCursor(hReady);
					}   
					break;
//-----------------------------------------------------------------
				case TW_APP_SEND:
					{
						SendTwain(hWnd);
						if(TWIsDSMOpen() && TWIsDSOpen())
						{
							CheckSpecialMenu(hWnd, TW_APP_SEND);
						}
					}   
					break;
//-----------------------------------------------------------------
				case TW_APP_ENABLE:
 					{
						HMENU hmenu = GetMenu(hWnd);
						BOOL bMenuState = (BOOL)GetMenuState(hmenu,TW_APP_SHOW,MF_BYCOMMAND) & MF_CHECKED;

						if(TWEnableDS((TW_BOOL)bMenuState))
						{
							CheckSpecialMenu(hWnd, TW_APP_ENABLE);              

							//AcqFlag = 0 Do Not Accept MSG_XFERREADY
							//          1 Disable/CloseDS/CloseDSM
							//          2 Disable Only
							//          3 Do Not Disable - only if ShowUI=TRUE

							//  SDH - 01/30/95 - Correct 32-bit compiler warning.
							//  SetAcqFlag(
							//  (!(GetMenuState(hmenu, TW_APP_SHOW,    MF_BYCOMMAND) & MF_CHECKED)
							//  ||
							//  (GetMenuState(hmenu, TW_APP_DISABLE_AFTER, MF_BYCOMMAND) & MF_CHECKED))
							//  ? 2 : 3);
							SetAcqFlag((TW_INT16)
										((!(GetMenuState(hmenu, TW_APP_SHOW,    MF_BYCOMMAND) & MF_CHECKED)
										||
										(GetMenuState(hmenu, TW_APP_DISABLE_AFTER, MF_BYCOMMAND) & MF_CHECKED))
										? 2 : 3));
							g_bSpecialMenu = TRUE;
						}
					}
					break;
//-----------------------------------------------------------------
				case TW_APP_TRANSFER:
					{
						HMENU hmenu = GetMenu(hWnd);

						SetCursor(hWait);
						if (TWIsDSEnabled())
						{
							if (MessageLevel() >= ML_ERROR)
							{
								ShowRC_CC(hWnd, 0, 0, 0,
									"Cannot Setup for Transfer\nSource already enabled",
									"TWAIN Error");
							}
						}
						else
						{
							if (!TWIsDSMOpen())
							{
								if (MessageLevel() >= ML_ERROR)
								{
									ShowRC_CC(hWnd, 0, 0, 0,
									"Cannot Transfer\nSource Manager not Open",
									"TWAIN Error");
								}
							}
							else
							{
								if (!TWIsDSOpen())
								{
									if (MessageLevel() >= ML_ERROR)
									{
										ShowRC_CC(hWnd, 0, 0, 0,
										"Cannot Transfer\nSource not Open",
										"TWAIN Error");
									}
								}
								else
								{
									CheckSpecialMenu(hWnd, TW_APP_TRANSFER);              
									//Transfer mechnism is read from File Menu
									// Delete the following line if xfermech is to be set
									// on the Send Menu
									if (TWXferMech(hWnd) == TWRC_SUCCESS)
									{   
										//Do not check Autofeed on File menu
										// if settings are to be done through Send Menu
										if (TWAutofeedMenu(hWnd) == TWRC_SUCCESS)
										{
											if (TWEnableDS((TW_BOOL)(GetMenuState(hmenu, TW_APP_SHOW, MF_BYCOMMAND) & MF_CHECKED)))
											{   
												//Success

												//  2 Disable
												//  3 Do Not Disable - only if ShowUI=TRUE
												//  SDH - 01/30/95 - Correct 32-bit compiler warning.
												//  SetAcqFlag(
												//  (!(GetMenuState(hmenu, TW_APP_SHOW,MF_BYCOMMAND)&MF_CHECKED) 
												//  ||
												//  (GetMenuState(hmenu,TW_APP_DISABLE_AFTER,MF_BYCOMMAND)&MF_CHECKED)) 
												//  ? 2 : 3);
												SetAcqFlag((TW_INT16)
															((!(GetMenuState(hmenu, TW_APP_SHOW,MF_BYCOMMAND)&MF_CHECKED) 
															||
															(GetMenuState(hmenu,TW_APP_DISABLE_AFTER,MF_BYCOMMAND)&MF_CHECKED)) 
															? 2 : 3));
											}
										}
									}
								}
							}
						}
						g_bSpecialMenu = TRUE;
						SetCursor(hReady);
					}   
					break;
//-----------------------------------------------------------------
				case TW_APP_DISABLE:
					{
						SetCursor(hWait);
						if (TWDisableDS())
						{
							CheckSpecialMenu(hWnd, TW_APP_DISABLE);             
						}
						SetCursor(hReady);
						g_bSpecialMenu = FALSE;
					}
					break;
//-----------------------------------------------------------------
				case TW_APP_CLOSEDS:
					{
						HMENU hmenu = GetMenu(hWnd);
						SetCursor(hWait);               
						if (TWCloseDS())
						{
							dsID.Id = 0;
							CheckSpecialMenu(hWnd, TW_APP_CLOSEDS);
							GreyMenu (TRUE);
						}
						SetCursor(hReady);
						g_bSpecialMenu = FALSE;
					}  
					break;
//-----------------------------------------------------------------
				case TW_APP_CLOSESM:
					{
						HMENU hmenu = GetMenu(hWnd);

						SetCursor(hWait);
						if (TWCloseDSM(NULL))
						{
							CheckSpecialMenu(hWnd, TW_APP_CLOSESM);                 
							GreyMenu (FALSE);
						}
						SetCursor(hReady);
						g_bSpecialMenu = FALSE;
					}   
					break;
//-----------------------------------------------------------------
				case TW_APP_BUFFER:
				case TW_APP_FILE:
				case TW_APP_NATIVE:
					{
						HMENU hmenu = GetMenu(hWnd);

						CheckMenuItem(hmenu,TW_APP_NATIVE,MF_BYCOMMAND|MF_UNCHECKED);
						CheckMenuItem(hmenu,TW_APP_FILE,MF_BYCOMMAND|MF_UNCHECKED);
						CheckMenuItem(hmenu,TW_APP_BUFFER,MF_BYCOMMAND|MF_UNCHECKED);
						CheckMenuItem(hmenu,(UINT)wParam,MF_BYCOMMAND|MF_CHECKED);
					}   
					break;
//-----------------------------------------------------------------
				case TW_APP_SHOW:
					{
						HMENU hmenu = GetMenu(hWnd);
						TW_INT16 flag = 0;

						flag = GetMenuState(hmenu, TW_APP_SHOW, MF_BYCOMMAND)
										& MF_CHECKED;
						CheckMenuItem(hmenu, TW_APP_SHOW, MF_BYCOMMAND
										| (flag)?MF_UNCHECKED:MF_CHECKED);
					}   
					break;
//-----------------------------------------------------------------
				case TW_APP_MSG_NONE:
					{
						HMENU hmenu = GetMenu(hWnd);

						CheckMenuItem(hmenu,TW_APP_MSG_NONE,MF_BYCOMMAND|MF_CHECKED);
						CheckMenuItem(hmenu,TW_APP_MSG_ERROR,MF_BYCOMMAND|MF_UNCHECKED);
						CheckMenuItem(hmenu,TW_APP_MSG_INFO,MF_BYCOMMAND|MF_UNCHECKED);
						CheckMenuItem(hmenu,TW_APP_MSG_FULL,MF_BYCOMMAND|MF_UNCHECKED);

						SetMessageLevel(ML_NONE);
					}   
					break;
//-----------------------------------------------------------------
				case TW_APP_MSG_ERROR:
					{
						HMENU hmenu = GetMenu(hWnd);

						CheckMenuItem(hmenu,TW_APP_MSG_NONE,MF_BYCOMMAND|MF_UNCHECKED);
						CheckMenuItem(hmenu,TW_APP_MSG_ERROR,MF_BYCOMMAND|MF_CHECKED);
						CheckMenuItem(hmenu,TW_APP_MSG_INFO,MF_BYCOMMAND|MF_UNCHECKED);
						CheckMenuItem(hmenu,TW_APP_MSG_FULL,MF_BYCOMMAND|MF_UNCHECKED);

						SetMessageLevel(ML_ERROR);
					}   
					break;
//-----------------------------------------------------------------
				case TW_APP_MSG_INFO:
					{
						HMENU hmenu = GetMenu(hWnd);

						CheckMenuItem(hmenu,TW_APP_MSG_NONE,MF_BYCOMMAND|MF_UNCHECKED);
						CheckMenuItem(hmenu,TW_APP_MSG_ERROR,MF_BYCOMMAND|MF_UNCHECKED);
						CheckMenuItem(hmenu,TW_APP_MSG_INFO,MF_BYCOMMAND|MF_CHECKED);
						CheckMenuItem(hmenu,TW_APP_MSG_FULL,MF_BYCOMMAND|MF_UNCHECKED);

						SetMessageLevel(ML_INFO);
					}   
					break;
//-----------------------------------------------------------------
				case TW_APP_MSG_FULL:
					{
						HMENU hmenu = GetMenu(hWnd);

						CheckMenuItem(hmenu,TW_APP_MSG_NONE,MF_BYCOMMAND|MF_UNCHECKED);
						CheckMenuItem(hmenu,TW_APP_MSG_ERROR,MF_BYCOMMAND|MF_UNCHECKED);
						CheckMenuItem(hmenu,TW_APP_MSG_INFO,MF_BYCOMMAND|MF_UNCHECKED);
						CheckMenuItem(hmenu,TW_APP_MSG_FULL,MF_BYCOMMAND|MF_CHECKED);

						SetMessageLevel(ML_FULL);
					}   
					break;
//-----------------------------------------------------------------
				case TW_APP_AUTOFEED:
					{
						HMENU hmenu = GetMenu(hWnd);
						TW_INT16 flag = 0;

						flag = GetMenuState(hmenu, TW_APP_AUTOFEED, MF_BYCOMMAND)
										& MF_CHECKED;
						CheckMenuItem(hmenu, TW_APP_AUTOFEED, MF_BYCOMMAND
										| (flag)?MF_UNCHECKED:MF_CHECKED);
					}   
					break;
//-----------------------------------------------------------------
				case TW_APP_DISABLE_AFTER:
					{
						HMENU hmenu = GetMenu(hWnd);
						TW_INT16 flag = 0;

						flag = GetMenuState(hmenu, TW_APP_DISABLE_AFTER, MF_BYCOMMAND)
										& MF_CHECKED;
						CheckMenuItem(hmenu, TW_APP_DISABLE_AFTER, MF_BYCOMMAND
										| (flag)?MF_UNCHECKED:MF_CHECKED);
					}   
					break;
//-----------------------------------------------------------------
				case TW_APP_ABOUT:
					lpProcAbout = MakeProcInstance((DLGPROC)AboutDlgProc, hInst);
					DialogBox (hInst, "TW_APP_ABOUTBOX", hWnd, lpProcAbout);
					FreeProcInstance(lpProcAbout);
					break;
//-----------------------------------------------------------------
				default:
					break;
			}   
			break;

		default:
			return DefWindowProc (hWnd, iMessage, wParam, lParam);
	}   

	return 0L ;
}   

/*
* Function: DrawScrollBar
* Author: Nancy Létourneau / J.F.L. Peripheral Solutions Inc. / TWAIN Working Group
* Date: May 29/1998
* Input: 
*		hWnd - handle to main app window
*		si - pointer to a SCROLLINFO structure
*		nScroll_Id - Type of scroll bar
*		bm - Current bitmap
* Output: none
* Comment:
*		This function is call in the WM_PAINT message and it draw the
*		scroll bar.
*/
void DrawScrollBar(HWND hWnd, SCROLLINFO* si, int nScroll_Id, BITMAP bm)
{
	BOOL Hide = FALSE;
	int AddSize = 0;

	ASSERT(hWnd);

	if (nScroll_Id == SB_VERT)
	{
		si->nMax = bm.bmHeight;

		AddSize = GetSystemMetrics(SM_CYMENU);
		AddSize += GetSystemMetrics(SM_CYCAPTION);
		AddSize += GetSystemMetrics(SM_CXFRAME);
		AddSize += 1;
		if(bHideV == FALSE)
		{
			AddSize += GetSystemMetrics(SM_CYHSCROLL);
		}

		si->nPage = (Rect.bottom - Rect.top) - 
								(bm.bmHeight / Rect.bottom + AddSize);
	}
	else
	{
		si->nMax = bm.bmWidth;

		AddSize = GetSystemMetrics(SM_CYFRAME); 
		AddSize += 1;
		if (bHideH == FALSE)
		{
			AddSize += GetSystemMetrics(SM_CXVSCROLL); 				
		}

		si->nPage = (Rect.right - Rect.left) - 
								(bm.bmWidth / Rect.right + AddSize);
	}

	si->cbSize = sizeof(SCROLLINFO);
	si->fMask = SIF_ALL;
	SetScrollInfo(hWnd, nScroll_Id, si, TRUE);

	if (nScroll_Id == SB_VERT)
	{
		if((Rect.bottom - Rect.top) > bm.bmHeight)
		{
			Hide = TRUE;
		}
	}
	else
	{
		if((Rect.right - Rect.left) > bm.bmWidth)
		{
			Hide = TRUE;
		}
	}

	if(Hide == FALSE)
	{
		SetScrollInfo(hWnd, nScroll_Id, si, TRUE);
		ShowScrollBar(hWnd, nScroll_Id, TRUE);
	}
	else
	{
		ShowScrollBar(hWnd, nScroll_Id, FALSE);
	}
	return;
}

/*
* Function: SaveBitmap
* Author: Nancy Létourneau / J.F.L. Peripheral Solutions Inc. / TWAIN Working Group
* Date: June 1st/1998
* Input: 
*		hWnd - handle to main app window
* Output: none
* Comment:
*		This function save the bitmap in a file.
*/
void SaveBitmap(HWND hWnd, HGLOBAL _hDIB)
{
	/*
	*	Declared locals
	*/
	static OPENFILENAME ofn;
	char szDirName[256];
	char szFile[256];
	char szFileTitle[256];
	char chReplace = '|';
	char szFilter[256];
	BITMAPFILEHEADER bfh;
	BITMAPINFOHEADER *pBIH = NULL;

	HANDLE hf = NULL;
	UINT i = 0, cbString = 0;

	/*
	*	Initialize all structures
	*/
	memset(&ofn, 0, sizeof(OPENFILENAME));
	memset(szDirName, 0, sizeof(char[256]));
	memset(szFile, 0, sizeof(char[256]));
	memset(szFileTitle, 0, sizeof(char[256]));
	memset(szFilter, 0, sizeof(char[256]));
	memset(&bfh, 0, sizeof(BITMAPFILEHEADER));

	/*
	*	We are expecting a valid HWND
	*/
	ASSERT(hWnd);
	
	if(_hDIB)
	{
		/*
		*	Make sure only .bmp file extensions are possible
		*/
		GetCurrentDirectory(sizeof(szDirName), szDirName);
		szFile[0] = '\0';
		cbString = LoadString(hInst, IDS_FILTERSTRING, szFilter, 
									sizeof(szFilter));

		for(i = 0;szFilter[i] != '\0';i++)
		{
			if(szFilter[i] == chReplace)
			{
				szFilter[i] = '\0';
			}
		}

		/*
		*	Fill in the open file structure
		*/
		memset(&ofn, 0, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = hWnd;
		ofn.hInstance = NULL;
		ofn.lpstrFilter = szFilter;
		ofn.lpstrCustomFilter = (LPSTR)NULL;
		ofn.nFilterIndex = 1;
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFileTitle = szFileTitle;
		ofn.nMaxFileTitle = sizeof(szFileTitle);
		ofn.lpstrInitialDir = szDirName;
		ofn.lpstrTitle = (LPSTR)NULL;
		ofn.Flags = OFN_SHOWHELP|OFN_OVERWRITEPROMPT;
		ofn.nFileOffset = NULL;
		ofn.nFileExtension = NULL;
		ofn.lpstrDefExt = (LPSTR)"bmp";
		ofn.lCustData = 0L;
		ofn.lpfnHook = NULL;
		ofn.lpTemplateName = (LPSTR)NULL;

		/*
		* Get the user to provide the destination name
		*/
		GetSaveFileName(&ofn);
		hf = CreateFile(ofn.lpstrFile, 
										GENERIC_WRITE, 0, 
										(LPSECURITY_ATTRIBUTES)NULL,
										CREATE_ALWAYS,
										FILE_ATTRIBUTE_NORMAL,
										(HANDLE)NULL);

		if(hf)
		{
			/*
			*	Get a pointer to the beginning of the DIB
			*/
			pBIH = (BITMAPINFOHEADER*)DSM_LockMemory(_hDIB);
			if(pBIH)
			{
				DWORD dwBytesWritten = 0l;

				/*
				* Fill in the Bitmap File Header
				*/
				bfh.bfType = ( (WORD) ('M' << 8) | 'B');
				
				/*
				*	Calculate the size of the bitmap including the palette
				*/
				bfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) +
					((((pBIH->biWidth * pBIH->biBitCount + 31)/32) * 4) * pBIH->biHeight)
						+ pBIH->biClrUsed * sizeof(RGBQUAD);
				bfh.bfReserved1 = 0;
				bfh.bfReserved2 = 0;
				
				/*
				*	Offset to actual bits is after palette
				*/
				bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)
					+ pBIH->biClrUsed * sizeof(RGBQUAD);

				ASSERT(bfh.bfSize > sizeof(BITMAPFILEHEADER));

				/*
				*	Write the results
				*/
				WriteFile(hf, &bfh, sizeof(BITMAPFILEHEADER), &dwBytesWritten, NULL);
				WriteFile(hf, pBIH, bfh.bfSize-sizeof(BITMAPFILEHEADER), &dwBytesWritten, NULL);
				
				/*
				* Done now
				*/
				DSM_UnlockMemory(_hDIB);
			}
			CloseHandle(hf);
		}
	}
	return;
}

HGLOBAL CloneDib(HGLOBAL _hDIB)
{
	/*
	*	Declared locals
	*/
	BITMAPINFOHEADER *pBIH = NULL;
  void *pDIB = NULL;
  HGLOBAL hDIB = NULL;
  DWORD dwSize;
	if(!_hDIB)
  {
    return NULL;
  }

	/*
	*	Get a pointer to the beginning of the DIB
	*/
	pBIH = (BITMAPINFOHEADER*)DSM_LockMemory(_hDIB);
	if(!pBIH)
  {
    return NULL;
  }
  dwSize =  sizeof(BITMAPINFOHEADER) + ((((pBIH->biWidth * pBIH->biBitCount + 31)/32) * 4) * pBIH->biHeight) + pBIH->biClrUsed * sizeof(RGBQUAD);
	hDIB = GlobalAlloc(GHND, dwSize);
	if(!hDIB)
  {
	  DSM_UnlockMemory(_hDIB);
    return NULL;
  }	
	pDIB = (void*)GlobalLock(hDIB);
	if(!pDIB)
  {
    GlobalFree(hDIB);
	  DSM_UnlockMemory(_hDIB);
    return NULL;
  }	
  memcpy(pDIB,pBIH,dwSize);
  GlobalUnlock(hDIB);
	DSM_UnlockMemory(_hDIB);
	return hDIB;
}

/*
* Function: CloseApplication
* Author: Nancy Létourneau / J.F.L. Peripheral Solutions Inc. / TWAIN Working Group
* Date: May 25/1998
* Input: 
*		hWnd - handle to main app window
* Output:
*		TRUE  if successful closing
* Comment:
*		Close the application but make sure that the Source and the 
*		Source Manager already close.
*/
BOOL CloseApplication(HWND hWnd)
{
	int Message = 0;

	ASSERT(hWnd);

	/* 
	* if the source is enable 
	*/
	if (TWIsDSEnabled())
	{
		Message = MessageBox (NULL, "Disable and close the Source\n\rand unload and close the Source Manager?",
								"Close message", MB_OKCANCEL);
		if (Message == IDOK)
		{
			if (TWDisableDS())
			{
				if (TWCloseDS())
				{
					if (TWCloseDSM(NULL))
					{
						CheckSpecialMenu(hWnd, TW_APP_CLOSESM);
					}
				}
			}
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}

	/*
	* if the source is open
	*/
	if (TWIsDSOpen())
	{
		Message = MessageBox (NULL, "Close the Source and unload \n\rand close the Source Manager?",
								"Close message", MB_OKCANCEL);
		if (Message == IDOK)
		{
			if (TWCloseDS())
			{
				if (TWCloseDSM(NULL))
				{
					CheckSpecialMenu(hWnd, TW_APP_CLOSESM);
				}
			}
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}

	/*
	* if the source manager is open
	*/
	if (TWIsDSMOpen())
	{
		Message = MessageBox (NULL, "Unload and close the Source Manager?",
								"Close message", MB_OKCANCEL);
		if (Message == IDOK)
		{
			if (TWCloseDSM(NULL))
			{
				CheckSpecialMenu(hWnd, TW_APP_CLOSESM);
			}
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}

	/*
	* if nothing is open
	*/
	if (!TWIsDSEnabled() & !TWIsDSOpen() & !TWIsDSMOpen())
	{
		return TRUE;
	}

  return FALSE;
}

/*
* Function: InitTwainCombo
* Author:	TWAIN Working Group
* Input: 
*			hDlg - Handle for the Dialog 
*			Id - Id for Combo box reference.
*			pTable - Contain the new list of items for each combo box.
*			nSizeTable - Maximum of items in each list.
* Output:
*			Return 1 when all is set.
* Comments:
*			Sets the combo box values for the special menu dialog.
*/
TW_INT16 InitTwainCombo(HWND hDlg, TW_INT16 Id, 
												pTABLEENTRY pTable, int nSizeTable)
{
	TW_INT16 result = 0;
	int i = 0;

	ASSERT(hDlg);
	ASSERT(pTable);

	/*
	* Reset old content, prepare to replace with new list
	*/
	SendDlgItemMessage(hDlg, Id, CB_RESETCONTENT, 0, 0);
	for (i = 0; i < nSizeTable; i++)
	{
		int nIndex = (int)SendDlgItemMessage(hDlg, Id, CB_ADDSTRING , 0, 
											(LPARAM)(pTABLEENTRY)pTable[i].pszItemName);

		if(nIndex >= 0)
		{
			/*
			*	Store reference to the Table Entry associated with this string
			*/
			SendDlgItemMessage(hDlg, Id, CB_SETITEMDATA, (WPARAM)nIndex, 
												(LPARAM)&pTable[i]);
		}
		#ifdef _DEBUG
		else
		{
			TRACE("Serious problem adding item %s, to Combo box.\n", (pTABLEENTRY)pTable[i].pszItemName);
		}
		#endif //_DEBUG
	}
	SendDlgItemMessage(hDlg, Id, CB_SETCURSEL, 0, 0L);
	result = 1;

	return  result;
}


/*
* Function: InitTwainList
* Author:	TWAIN Working Group
* Input: 
*			hDlg - Handle for the Dialog 
*			Id - Id for List reference.
*			pTable - Contain the new list of items for each combo box.
*			nSizeTable - Maximum of items in each list.
* Output:
*			Return 1 when all is set.
* Comments:
*			Sets the listbox values for the special menu dialog.
*/
TW_INT16 InitTwainList(HWND hDlg, TW_INT16 Id, 
												pTABLEENTRY pTable, int nSizeTable)
{
	TW_INT16 result = 0;
	int i = 0;

	ASSERT(hDlg);
	ASSERT(pTable);

	/*
	* Reset old content, prepare to replace with new list
	*/
	SendDlgItemMessage(hDlg, Id, LB_RESETCONTENT, 0, 0);
	for (i = 0; i < nSizeTable; i++)
	{
		int nIndex = (int)SendDlgItemMessage(hDlg, Id, LB_ADDSTRING , 0, 
											(LPARAM)(pTABLEENTRY)pTable[i].pszItemName);

		if(nIndex >= 0)
		{
			/*
			*	Store reference to the Table Entry associated with this string
			*/
			SendDlgItemMessage(hDlg, Id, LB_SETITEMDATA, (WPARAM)nIndex, 
												(LPARAM)&pTable[i]);
		}
		#ifdef _DEBUG
		else
		{
			TRACE("Serious problem adding item %s, to List box.\n", (pTABLEENTRY)pTable[i].pszItemName);
		}
		#endif //_DEBUG
	}
	SendDlgItemMessage(hDlg, Id, LB_SETCURSEL, 0, 0L);
	result = 1;

	return  result;
}

/////////////////////////////////////////////////////////////////////////////
// FUNCTION: CleanUpApp
//
// ARGS:    none
//
// RETURNS: none
//
// NOTES:   1). delete any bit maps laying around
//          2). delete any palettes laying around
//
VOID CleanUpApp (VOID)
{   
	/*
	*	Free any previous DIB image
	*/
	if(_ghDIB)
	{
		GlobalFree(_ghDIB);
		_ghDIB = NULL;
	}

	if (hbm)
	{
		DeleteObject(hbm);
	}

	hbm = NULL;

	if (hDibPal)
	{
		DeleteObject(hDibPal);
	}

	hDibPal = NULL;
	return;
} 

/////////////////////////////////////////////////////////////////////////////
// FUNCTION: CleanKillApp
//
// ARGS:    none
//
// RETURNS: none
//
// NOTES:   1). delete any bit maps laying around
//          2). post window quit message
//
VOID CleanKillApp (VOID)
{
	CleanUpApp();

	PostQuitMessage(0);
	return;
} 

/////////////////////////////////////////////////////////////////////////////
// FUNCTION: DibNumColors
//
// ARGS:    pv  pointer to bitmap data
//
// RETURNS: number of colors, 0, 2, 16, 256, in the DIB
//
// NOTES:
//
WORD DibNumColors (VOID FAR *pv)
{
	int Bits = 0;
	LPBITMAPINFOHEADER lpbi = NULL;
	LPBITMAPCOREHEADER lpbc = NULL;

	ASSERT(pv);

	lpbi = ((LPBITMAPINFOHEADER)pv);
	lpbc = ((LPBITMAPCOREHEADER)pv);

	//    With the BITMAPINFO format headers, the size of the palette
	//    is in biClrUsed, whereas in the BITMAPCORE - style headers, it
	//    is dependent on the bits per pixel ( = 2 raised to the power of
	//    bits/pixel).
	if (lpbi->biSize != sizeof(BITMAPCOREHEADER))
	{
		if (lpbi->biClrUsed != 0)
		{
			return (WORD)lpbi->biClrUsed;
		}

		Bits = lpbi->biBitCount;
	}
	else
	{ 
		Bits = lpbc->bcBitCount;
	}

	switch (Bits)
	{
		case 1:
			return 2;

		case 4:
			return 16;

		case 8:
			return 256;

		default:
			// A 24 bitcount DIB has no color table
			return 0;
	}
}

/////////////////////////////////////////////////////////////////////////////
// FUNCTION: FixUp -- Creates a device independent bitmap and frees the bmp
// handle memory. 
//
// ARGS:    hDib    handle to DIB
//
// RETURNS: hBitMap handle to a DDB
//
HBITMAP FixUp(HWND hWnd, HANDLE hDib)
{
	LPBITMAPINFOHEADER  lpDib = NULL, lpBi = NULL;
	LPSTR               lpBits = NULL;
	DWORD               dwColorTableSize = 0;
	HBITMAP             hBitMap = NULL;
	HDC                 hDC = NULL;

	ASSERT(hWnd);
	ASSERT(hDib);

	if (hDib == NULL)
	{
		return(NULL);
	}

	if ((lpDib = (LPBITMAPINFOHEADER) DSM_LockMemory(hDib))==NULL)
	{
		if (MessageLevel()  >= ML_ERROR)
		{
			ShowRC_CC(hWnd, 0, 0, 0,
					"Could Not Lock Bitmap Memory", "FixUp in TWACKER.C");
		}
		return(NULL);
	}

	lpBi = lpDib;

	// Calculate the color table size, then point past the BITMAPINFOHEADER
	// and color table, to the byte array of bitmap bits.
	dwColorTableSize = (DWORD)(DibNumColors (lpDib) * sizeof(RGBQUAD));
	lpBits = (LPSTR)lpDib + lpBi->biSize + dwColorTableSize;

	// Create a logical palette based on the colors in the DIB header,
	// and realize it
	hDC = GetDC(hMainWnd);

	if (hDibPal = CreateBIPalette (lpBi))
	{
		SelectPalette (hDC, hDibPal, FALSE);
		RealizePalette (hDC);
	}

	//  Microsoft confirmed a defect in CreateDIBitmap whereas the function
	//  failed with handles to 1Meg or larger 1 color bitmaps.  The workaround
	//  is to create a device dependent bitmap whenever a 1 color bitmap
	//  is detected.  
	if (lpDib->biBitCount == 1)
	{
		// create a device dependent monochrome bitmap
		hBitMap = CreateBitmap((int)lpDib->biWidth,(int)lpDib->biHeight,1,1,lpBits);
		if(hBitMap)
		{
			SetDIBits(hDC,hBitMap,0,(UINT)lpDib->biHeight,lpBits,(LPBITMAPINFO)lpDib,DIB_RGB_COLORS);
		}
	}
	else
	{
		// Create a device-independent bitmap from the BMP
		hBitMap = CreateDIBitmap (hDC, lpDib, (LONG)CBM_INIT, (LPSTR)lpBits,
							(LPBITMAPINFO)lpDib, DIB_RGB_COLORS);
	}

	DSM_UnlockMemory(hDib);

	ReleaseDC (hMainWnd, hDC);

//	DSM_UnlockMemory(hDib);

	// Return handle to device-dependent bitmap
	return(hBitMap);
}

/////////////////////////////////////////////////////////////////////////////
// CreateBIPalette
//
// ARGS:    lpbi        pointer to a BITMAPINFO structure
//
// RETURNS: hPalette    handle to the created palette
//                                        
// NOTES:   Given a Pointer to a BITMAPINFO struct will create a
//          GDI palette object from the color table.
//
HPALETTE CreateBIPalette (LPBITMAPINFOHEADER lpbi)
{                           
	HGLOBAL             hPal = NULL;
	LOGPALETTE          *pPal = NULL;
	HPALETTE            hPalette = NULL;
	WORD                nNumColors = 0;
	BYTE                Red = 0;
	BYTE                Green = 0;
	BYTE                Blue = 0;
	WORD                i = 0;
	RGBQUAD             FAR *pRgb = NULL;

	ASSERT(lpbi);

	if (!lpbi)
	{
		return NULL;
	}

	if (lpbi->biSize != sizeof(BITMAPINFOHEADER))
	{
		return NULL;
	}

	// Get a pointer to the color table and the number of colors in it 
	pRgb = (RGBQUAD FAR *)((LPSTR)lpbi + (WORD)lpbi->biSize);
	nNumColors = DibNumColors(lpbi);

	if (nNumColors)
	{
		// Allocate for the logical palette structure
		//  SDH - 02/01/95 - For compatability with the Large model...
		//  pPal = (LOGPALETTE*)LocalAlloc(LPTR,sizeof(LOGPALETTE) + nNumColors *
		//                  sizeof(PALETTEENTRY));
		hPal = DSM_Alloc(sizeof(LOGPALETTE) + nNumColors *
							sizeof(PALETTEENTRY));
		pPal = (LOGPALETTE*)(DSM_LockMemory (hPal));
		if (!pPal)
		{
			return NULL;
		}

		pPal->palNumEntries = nNumColors;
		pPal->palVersion    = PALVERSION;

		// Fill in the palette entries from the DIB color table and
		// create a logical color palette.
		for (i = 0; i < nNumColors; i++)
		{
			pPal->palPalEntry[i].peRed   = pRgb[i].rgbRed;
			pPal->palPalEntry[i].peGreen = pRgb[i].rgbGreen;
			pPal->palPalEntry[i].peBlue  = pRgb[i].rgbBlue;
			pPal->palPalEntry[i].peFlags = (BYTE)0;
		}

		hPalette = CreatePalette(pPal);

		DSM_UnlockMemory(hPal);
		DSM_Free(hPal);
	}
	else if (lpbi->biBitCount == 24)
	{
		// A 24 bitcount DIB has no color table entries so, set
		// the number of to the maximum value (256).
		nNumColors = MAXPALETTE;

		hPal = DSM_Alloc(sizeof(LOGPALETTE) + nNumColors * sizeof(PALETTEENTRY));
		pPal = (LOGPALETTE*)DSM_LockMemory (hPal);

		if (!pPal)
		{
			return NULL;
		}

		pPal->palNumEntries = nNumColors;
		pPal->palVersion    = PALVERSION;

		Red = Green = Blue = 0;

		// Generate 256 (= 8*8*4) RGB combinations to fill
		// the palette entries.
		for (i = 0; i < pPal->palNumEntries; i++)
		{
			pPal->palPalEntry[i].peRed   = Red;
			pPal->palPalEntry[i].peGreen = Green;
			pPal->palPalEntry[i].peBlue  = Blue;
			pPal->palPalEntry[i].peFlags = (BYTE)0;

			if (!(Red += 32))
			{
				if (!(Green += 32))
				{
					Blue += 64;
				}
			}
		}

		hPalette = CreatePalette(pPal);

		DSM_UnlockMemory(hPal);
		DSM_Free(hPal);
	}
	return hPalette;
}

/////////////////////////////////////////////////////////////////////////////
// CheckSpecialMenu -- Checks the item in the specials main window menu corresponding
// to the current state.
//
// ARGS:    The Menu Item to check on the Special Menu
//          All others will be unchecked
//
// RETURNS: VOID
//                                       
VOID CheckSpecialMenu(HWND hWndofApp, TW_INT16 CkMenu)
{
	HMENU hmenu = GetMenu(hWndofApp);

	ASSERT(hWndofApp);

	CheckMenuItem(hmenu,TW_APP_OPENSM,MF_BYCOMMAND|MF_UNCHECKED);
	CheckMenuItem(hmenu,TW_APP_OPENDS,MF_BYCOMMAND|MF_UNCHECKED);
	CheckMenuItem(hmenu,TW_APP_SEND,MF_BYCOMMAND|MF_UNCHECKED);
	CheckMenuItem(hmenu,TW_APP_ENABLE,MF_BYCOMMAND|MF_UNCHECKED);               
	CheckMenuItem(hmenu,TW_APP_TRANSFER,MF_BYCOMMAND|MF_UNCHECKED);
	CheckMenuItem(hmenu,TW_APP_DISABLE,MF_BYCOMMAND|MF_UNCHECKED);
	CheckMenuItem(hmenu,TW_APP_CLOSEDS,MF_BYCOMMAND|MF_UNCHECKED);              
	CheckMenuItem(hmenu,TW_APP_CLOSESM,MF_BYCOMMAND|MF_UNCHECKED);

	CheckMenuItem(hmenu,CkMenu,MF_BYCOMMAND|MF_CHECKED);
	return;
}

///////////////////////////////////////////////////////////////////////////
// CheckIfAcquire -- Checks the main menu items for acquire types based on
// the attribute passed in.  This is done to keep the SET Capability call in
// sync with the RUN Acquire command.  SET Capability ICAP_XFERMECH can be 
// used in place of the SET ____Xfer call.
// 
void CheckIfAcquire(char attribute[])
{
	HMENU hmenu = NULL;
	UINT type = 99;

	hmenu = GetMenu(hMainWnd);

	if (memcmp(attribute,"TWSX_FILE",sizeof("TWSX_FILE"))==0)
	{
		type = TW_APP_FILE;
	}
	else if (memcmp(attribute,"TWSX_NATIVE",sizeof("TWSX_NATIVE"))==0)
	{
		type = TW_APP_NATIVE;
	}
	else if (memcmp(attribute,"TWSX_MEMORY",sizeof("TWSX_MEMORY"))==0)
	{
		type = TW_APP_BUFFER;
	}

	if (type != 99)
	{
		CheckMenuItem(hmenu,TW_APP_NATIVE,MF_BYCOMMAND|MF_UNCHECKED);
		CheckMenuItem(hmenu,TW_APP_FILE,MF_BYCOMMAND|MF_UNCHECKED);
		CheckMenuItem(hmenu,TW_APP_BUFFER,MF_BYCOMMAND|MF_UNCHECKED);
		CheckMenuItem(hmenu,type,MF_BYCOMMAND|MF_CHECKED);
	}
	return;
}

/*
* Function:	OnScrollBar
* Author:	Nancy Létourneau / J.F.L. Peripheral Solutions Inc. / TWAIN Working Group
* Date:		March 1998
* Input:	hWnd - Handle to window with scroll bar.
*			nScroll_Id - Type of scroll bar.
*			code - Scroll bar value.
*			Pos	- Scroll box position.
* Output:	
* Comments: Intercept the scrollbar code, depend what the user press.
*/
void OnScrollBar(HWND hWnd, int nScroll_Id, UINT code,
				 int Pos, SCROLLINFO *sbInfo)
{
	int  nTmp = 0;
	int nPageSize = 0;

	ASSERT(hWnd);

	/*
	* To keep the coordinate  of the upper-left corner.
	* Put the value of the coordinate in a tempory variable
	*/
	if (nScroll_Id == SB_VERT)
	{
		nTmp = Point.y;
	}
	else
	{
		nTmp = Point.x;
	}

  #if defined(WIN32) || defined(WIN64)
	{
		int AddSize = 0;

		GetScrollInfo(hWnd, nScroll_Id, sbInfo);
		nPageSize = sbInfo->nPage;

		switch(code)
		{
			/* 
			* When you press the "HOME" key.
			*/
			case SB_TOP:
				sbInfo->nPos = sbInfo->nMin;
				nTmp = sbInfo->nMin;
				break;

			/*
			* When you press the "END" key.
			*/
			case SB_BOTTOM:
				sbInfo->nPos = sbInfo->nMax;
				nTmp = sbInfo->nMax;
				break;						 

			case SB_LINEUP:
				if ((int)sbInfo->nPos > 0)
				{
					sbInfo->nPos -= 1;
					nTmp -= 1;
				}
				break;

			case SB_LINEDOWN:
				if ((int)sbInfo->nPos < sbInfo->nMax)
				{
					sbInfo->nPos += 1;
					nTmp += 1;
				}
				break;

			case SB_PAGEUP:
				if ((UINT)sbInfo->nPos < sbInfo->nPage)
				{
					nPageSize = sbInfo->nPos;
				}
				sbInfo->nPos -= nPageSize;
				nTmp -= nPageSize;
				break;

			case SB_PAGEDOWN:
				if ((UINT)sbInfo->nPos > sbInfo->nMax - sbInfo->nPage)
				{
					nPageSize = sbInfo->nMax - sbInfo->nPage;
				}
				sbInfo->nPos += nPageSize;
				nTmp += nPageSize;
				break;

			case SB_THUMBTRACK:
				sbInfo->nPos = sbInfo->nTrackPos;
				nTmp = sbInfo->nTrackPos;
				break;

			case SB_ENDSCROLL:
				return; 
		}

		/*
		* Put back the value in the good coordinate.
		*/
		if (nScroll_Id == SB_VERT)
		{
			if(code == SB_PAGEUP || code == SB_PAGEDOWN)
			{
				AddSize = GetSystemMetrics(SM_CYMENU);
				AddSize += GetSystemMetrics(SM_CYCAPTION);
				AddSize += GetSystemMetrics(SM_CXFRAME);
				if(bHideV == FALSE)
				{
					AddSize += GetSystemMetrics(SM_CYHSCROLL);
				}
				if(sbInfo->nPos > sbInfo->nMax - Rect.bottom)
				{
					Point.y = sbInfo->nMax - Rect.bottom + AddSize;
				}
				else
				{
					Point.y = nTmp;
				}
			}
			else
			{
				Point.y = nTmp;
			}
		}
		else
		{						
			if(code == SB_PAGEUP || code == SB_PAGEDOWN)
			{
				AddSize = GetSystemMetrics(SM_CYFRAME); 
				if (bHideH == FALSE)
				{
					AddSize += GetSystemMetrics(SM_CXVSCROLL); 				
				}
				if(sbInfo->nPos > sbInfo->nMax - Rect.right)
				{
					Point.x = sbInfo->nMax - Rect.right + AddSize;
				}
				else
				{
					Point.x = nTmp;
				}
			}
			else
			{
				Point.x = nTmp;
			}
		}
		SetScrollInfo(hWnd, nScroll_Id, sbInfo, TRUE);
	}

	#else //_WIN32
	{
		int nTemp = 0;
		int nScrPos = 0;
		int nDiff = 0;
		int nMax = 0;
		intnMin = 0;

		nPosition = GetScrollPos(hWnd, nScroll_Id);
		GetScrollRange(hWnd, nScroll_Id, &nMin, &nMax);
		switch(code)
		{
			/* 
			* When you press the "HOME" key.
			*/
			case SB_TOP:
				nPosition = nMin;
				Point.y = nMin;
				Point.x = nMin;
				nScrPos = nMax;
				nTmp = nMin;
				break;

			/*
			* When you press the "END" key.
			*/
			case SB_BOTTOM:
				nPosition = nMax;
				Point.y = nMax;
				Point.x = nMax;
				nScrPos = nMax;
				nTmp = nMax;
				break;						 

			case SB_LINEUP:
				if ((int)nPosition > 0)
				{
					nPosition -= 1;
					nTmp -= 1;
					nScrPos = 1;
					nDiff -= 1;		 
				}
				break;

			case SB_LINEDOWN:
				if ((int)nPosition < nMax)
				{
					nPosition += 1;
					nTmp += 1;
					nScrPos = -1;
					nDiff += 1;
				}
				break;

			case SB_PAGEUP:
				if ((int)nPosition > 0)
				{
					nPosition -= nPageSize;
					nTmp -= nPageSize;
					nScrPos = nPageSize;		 
					nDiff -= nPageSize;
				}
				break;

			case SB_PAGEDOWN:
				if ((int)nPosition < nMax)
				{
					nPosition += nPageSize;
					nTmp += nPageSize;
					nScrPos = -nPageSize;
					nDiff += nPageSize;
				}
				break;

			case SB_THUMBTRACK:
				nTemp = nPosition;
				nPosition = Pos;
				nScrPos = (nTemp - nPosition);
				nDiff = -(nTemp - nPosition);
				break;

			case SB_THUMBPOSITION:
				nPosition = Pos;
				nTmp = Pos;
				break;
		}

		/*
		* Put back the value in the good coordinate.
		*/
		if (nScroll_Id == SB_VERT)
		{
			Point.y = nTmp;
		}
		else
		{						
			Point.x = nTmp;
		}
		SetScrollPos(hWnd, nScroll_Id, nPosition, TRUE);

		/*
		* Condition for the vertical scroll
		*/
		if (nScroll_Id == SB_VERT)
		{
			if (nDiff > 0)
			{
				rc.left = Rect.left;
				rc.top = Rect.bottom - nDiff;
				rc.right = Rect.right;
				rc.bottom = Rect.bottom;
			}
			else
			{
				rc.left = Rect.left;
				rc.top = Rect.top;
				rc.right = Rect.right;
				rc.bottom = Rect.top + nDiff;
			}
			ScrollWindow(hWnd, 0, nScrPos, NULL, NULL);
		}
		/*
		* Condition for the horizontal scroll
		*/
		else
		{
			if (nDiff > 0)
			{
				rc.left = Rect.right - nDiff;
				rc.top = Rect.top;
				rc.right = Rect.right;
				rc.bottom = Rect.bottom;
			}
			else
			{
				rc.left = Rect.left;
				rc.top = Rect.top;
				rc.right = Rect.left + nDiff;
				rc.bottom = Rect.bottom;
			}
			ScrollWindow(hWnd, nScrPos, 0, NULL, NULL);		
		}
	}
	#endif //_WIN32 
	return;
}

/*
* Function:	OnKeyDown 
* Author:	Nancy Létourneau / J.F.L. Peripheral Solutions Inc. / TWAIN Working Group
* Date:		March 1998
* Input:	hWnd - Handle to window with scroll bar.
*			vk - Key data.
* Output:	
* Comments: Intercept the key down woh go with the scrollbar.
*/
void OnKeyDown(HWND hWnd, UINT vk)
{
	ASSERT(hWnd);

	switch(vk)
	{
		case VK_HOME:
			SendMessage(hWnd, WM_VSCROLL, SB_TOP, 0L);
			SendMessage(hWnd, WM_HSCROLL, SB_TOP, 0L);
			break;
		case VK_END:
			SendMessage(hWnd, WM_VSCROLL, SB_BOTTOM, 0L);
			SendMessage(hWnd, WM_HSCROLL, SB_BOTTOM, 0L);
			break;
		case VK_DOWN:
			SendMessage(hWnd, WM_VSCROLL, SB_LINEDOWN, 0L);
			break;
		case VK_UP:
			SendMessage(hWnd, WM_VSCROLL, SB_LINEUP, 0L);
			break;
		case VK_PRIOR:
			SendMessage(hWnd, WM_VSCROLL, SB_PAGEUP, 0L);
			break;
		case VK_NEXT:
			SendMessage(hWnd, WM_VSCROLL, SB_PAGEDOWN, 0L);
			break;
		case VK_LEFT:
			SendMessage(hWnd, WM_HSCROLL, SB_LINEUP, 0L);
			break;
		case VK_RIGHT:
			SendMessage(hWnd, WM_HSCROLL, SB_LINEDOWN, 0L);
			break;
	}
}
TW_ENTRYPOINT g_DSM_Entry;

void setEntryPoints(pTW_ENTRYPOINT _pEntryPoints)
{
  if(_pEntryPoints)
  {
    g_DSM_Entry = *_pEntryPoints;
  }
  else
  {
    memset(&g_DSM_Entry, 0, sizeof(TW_ENTRYPOINT));
  }
}

TW_HANDLE DSM_Alloc(TW_UINT32 _size)
{
  if(g_DSM_Entry.DSM_MemAllocate)
  {
    return g_DSM_Entry.DSM_MemAllocate(_size);
  }

  return GlobalAlloc(GHND, _size);
}

void DSM_Free(TW_HANDLE _pPtr)
{
  if(g_DSM_Entry.DSM_MemFree)
  {
    g_DSM_Entry.DSM_MemFree(_pPtr);
    return;
  }

  GlobalFree(_pPtr);
}

TW_MEMREF DSM_LockMemory(TW_HANDLE _pMemory)
{
  if(g_DSM_Entry.DSM_MemLock)
  {
    return g_DSM_Entry.DSM_MemLock(_pMemory);
  }

  return (TW_MEMREF)GlobalLock(_pMemory);
}

void DSM_UnlockMemory(TW_MEMREF _pMemory)
{
  if(g_DSM_Entry.DSM_MemUnlock)
  {
    g_DSM_Entry.DSM_MemUnlock(_pMemory);
    return;
  }

  GlobalUnlock(_pMemory);
}

