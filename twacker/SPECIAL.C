/*
* File:         special.c
* RCS:          $Header: $
* Description:  Exerciser application program for Twain
* Owner:        TWAIN Working Group
* Author:       TWAIN Working Group
* Created:      Jan 15,92
* Modified:     June 1993
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
* This code now uses triplets.c for all communication to the data source.
*/

//----------------------------------------------------------------------
//                          I n c l u d e s
//----------------------------------------------------------------------

#include <windows.h>         // Req. for twain.h type defs and ...
#include <commdlg.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

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
#include "twacker.h"
#include "special.h"
#include "captest.h"
//#include "ctl3d.h"
#include "triplets.h"

TW_UINT16 AltTWItemSize(TW_INT16 ItemType);

//---------------------------------------------------------------------------
//                              V a r i a b l e s
//---------------------------------------------------------------------------

TW_STR255   IniFile;
HANDLE      hStruct = NULL;
LPSTR       pStruct = NULL;
HANDLE      hInst = NULL;             // current instance
HCURSOR     hWait = NULL;
HCURSOR     hReady = NULL;

HWND IdItem = NULL;
TW_UINT32 uiNumItems = 0;
TW_FRAME g_Frame;
pTABLEENTRY g_pCurrentConType = NULL;
pTABLEENTRY g_pCurrentItemType = NULL;

HWND g_hMainDlg = NULL;
HWND g_hMainFrameDlg = NULL;
HWND g_hDynamicListBox = NULL;
HWND g_hEditBox = NULL;
WNDPROC g_fnOldControl;

BOOL g_bAllowFrac = FALSE;
BOOL g_bString = FALSE;
BOOL g_bNegative = FALSE;
BOOL g_bNewEditBox = FALSE;
BOOL g_bFrameDlg = FALSE;
BOOL g_bNewDisplay = FALSE;

TW_UINT32 g_uiOldIdControl = 0;
TW_INT16 g_ListBoxSel = 0;

static LPCSTR g_szSubClassEditBoxProc = "g_szSubClassEditBoxProc";

extern HWND hMainWnd;
extern BOOL CapSupportedCaps;

extern TABLEENTRY Destination[MAX_DEST];
extern TABLEENTRY DataGroup[MAX_DG];
extern TABLEENTRY DataType[MAX_DAT];
extern TABLEENTRY Message[MAX_MSG];
extern TABLEENTRY Capability[MAX_CAP];
extern TABLEENTRY ConType[MAX_CONTYPE];
extern TABLEENTRY DG_SourceManager[MAX_DGSM];
extern TABLEENTRY DG_Source[MAX_DGS];
extern TABLEENTRY DSM_DG_Control[MAX_DSMDAT];
extern TABLEENTRY DS_DG_Control[MAX_DSCONTROL];
extern TABLEENTRY DS_DG_Image[MAX_DSIMAGE];
extern TABLEENTRY DS_DG_Audio[MAX_DSAUDIO];
extern TABLEMSG MsgTable[MAX_DAT];
extern TABLEDATSTRUCT DataStructure[MAX_DATASTRUCTURE];
extern TABLEENTRY ReturnCode[MAX_RETURNCODE];
extern char *gszFrameStructure[FRAMEFIELDS];
extern TABLEENTRY ConditionCode[MAX_CONDITIONCODE];
extern TABLECAP Caps[MAX_CAP];
extern TABLEENTRY TypesRange[MAX_TYPESRANGE];
extern TABLEENTRY FileTypes[MAX_FILETYPE];

//---------------------------------------------------------------------------
//                         L o c a l  F u n c t i o n s
//---------------------------------------------------------------------------

void ClearTheControl(HWND hDlg);
void UnSubClassEditBox(TW_UINT32 uiOldIdControl, HWND hDlg);
void GetCurrentItemType(HWND hDlg, pTABLEENTRY);
LRESULT CALLBACK SubClassEditProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);

void SetItemListBox(HWND hDlg, TW_UINT32 uiItems, pTABLECAP pTableCap, TW_UINT32 ControlID);
void SetControlListBox(HWND hDlg, TW_UINT32 IdListBox, TW_UINT32 wmEvent);
static void ShowDialogItem(HWND hDlg, HWND IdItem, int iShowItem);
static void SetConType(pTABLEENTRY pEntryConType, HWND hDlg, HWND IdItem);

static TW_INT16 GetMenuItemChecked(HMENU hMenu);
static TW_INT16 InitTwainEdit(HWND hDlg, TW_INT16 Id, TW_INT16 MaxDataStruct,	char** DataStruct);
void UpdateTripletMenus (HWND hDlg, TW_UINT16 Updated);

/*
* Function: InitTwainEdit 
* Author: TWAIN Working Group
* Input:
*		hDlg - Handle of Dialog Box
*		Id -	Id of the Data Structure 
*		MaxDataStruct - Maximum fo item in the table entry
*		DataStruct - Table contain all the Data structure to intialize the
*								edit control.
* Output:
*		TW_INT16 - Return 1 if is successful.
* Comments:
*		Initializes the edit control in the special dialog
*/
static TW_INT16 InitTwainEdit(HWND hDlg, TW_INT16 Id, 
															TW_INT16 MaxDataStruct,
															char** DataStruct)
{
	TW_STR255   text;
	TW_INT16    status = 0;
	TW_INT16		i = 0;

	memset(text, 0, sizeof(TW_STR255));
	ASSERT(hDlg);

	for(i = 0; i < MaxDataStruct; i++)
	{
		wsprintf(text,"%s", DataStruct[i]);
		SendDlgItemMessage(hDlg,Id,EM_REPLACESEL,0,(LPARAM)(LPSTR)text);
		SendDlgItemMessage(hDlg,Id,EM_REPLACESEL,0,(LPARAM)(LPSTR)"\r\n");
	}   

	SendDlgItemMessage(hDlg, Id, EM_SETSEL, 0, MAKELONG(0,1));
	status = 1;
	return status;
}   

/*
* Function: MatchTwainInt
* Author: TWAIN Working Group
* Input:
*		pTable - Pointer to a Table entry that contain the value for the initialization
*		uiTableSize - Maximum of item in table 
*		uiCap -	ID for the current capability 
*		pString - 
* Output:
*		TW_BOOL -	TRUE is successful
* Comments:
*/
TW_BOOL MatchTwainInt(pTABLEENTRY pTable, TW_UINT32 uiTableSize,
												TW_INT32 uiCapId, LPSTR pString)

{
	TW_BOOL result = FALSE;
	TW_UINT16 i = 0;

	ASSERT(pTable);
	ASSERT(pString);

	for(i = 0; i < uiTableSize; i++)
	{
		if (pTable[i].ItemId == uiCapId)
		{
			lstrcpy(pString, pTable[i].pszItemName);
			result = TRUE;
			break;
		}   
	}   

	return  result;
}   

/*
* Function: SendTwain --
* Author: TWAIN Working Group
* Input:
*		hWnd - handle to main app window
* Output: none
* Comments:
*/
void SendTwain(HWND hWnd)
{
	DLGPROC proc = NULL;

	ASSERT(hWnd);

	if (!TWIsDSMOpen())
	{
		if (MessageLevel() >= ML_ERROR)
		{
			ShowRC_CC(hWnd, 0, 0, 0,
						"Source Manager Must Be Open",
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
							"Source Must Be Open",
							"TWAIN Error");
			}
		}
		else
		{
			/*
			* allocate room for the data struture 
			*/
			if (hStruct = DSM_Alloc( 2048))
			{
				pStruct = DSM_LockMemory(hStruct);

				/*
				* send structure command and show results 
				*/
				if (proc = MakeProcInstance(SendDlgProc, hInst))
				{
					if (DialogBoxParam(hInst, MAKEINTRESOURCE(DLG_SEND), hWnd, proc, 0L) == -1)
					{
						if (MessageLevel()  >= ML_ERROR)
						{
							ShowRC_CC(hWnd, 0, 0, 0,
										"DialogBoxParam Failed", "SendTwain in SPECIAL.C");
						}
					}

					FreeProcInstance(proc);
				}
				else
				{
					if (MessageLevel()  >= ML_ERROR)
					{
						ShowRC_CC(hWnd, 0, 0, 0,
									"Could Not Make Proc Instance", "SendTwain in SPECIAL.C");
					}
				}

				/*
				* unallocate the structure memory 
				*/
				if (pStruct) 
				{ 
					DSM_UnlockMemory(hStruct); pStruct = NULL;
				}

				DSM_Free(hStruct);
				hStruct = NULL;
			}
			else
			{
				if (MessageLevel()  >= ML_ERROR)
				{
					ShowRC_CC(hWnd, 0, 0, 0,
								"Could Not Allocate Memory", "SendTwain in SPECIAL.C");
				}
			}
		}
	}
	return;
}   

/*
* Function: SendDlgProc -- Special menu dialog procedure
* Author: TWAIN Working Group
* Input:
*		hDlg - handle to the dialog window
*		wMsg - variable containing the message to process
*		wParam - variable containing virtual keycode
*		lParam - variable containing additional information about the message
*					to process.
* Output:
*		status - 1 if dialog command processed
*					 - 0 if dialog command received is invalid
* Comments:
*		Special menu dialog procedure
*		This procedure processes commands for any generic dialog box.
*		Caption the window caption.     (Read Access)
*/
INT_PTR FAR PASCAL SendDlgProc(HWND hDlg,
														UINT wMsg,
														WPARAM wParam,
														LPARAM lParam)
{
	TW_UINT16 wmId = 0;
	TW_UINT16 wmEvent = 0;
	TABLEENTRY ConTypeSet[MAX_CONTYPE-1] =
					{
						{"TWON_ONEVALUE", TWON_ONEVALUE},
						{"TWON_ENUMERATION", TWON_ENUMERATION},
						{"TWON_ARRAY", TWON_ARRAY},
						{"TWON_RANGE", TWON_RANGE}
					};

#if defined(WIN32) || defined(WIN64)
		wmId = LOWORD(wParam);
		wmEvent = HIWORD (wParam);
	#else
		wmId = wParam;
		wmEvent = HIWORD (lParam);
	#endif

	ASSERT(hDlg);

	g_hMainDlg = hDlg;
	switch (wMsg)
	{
		case WM_INITDIALOG:
			if (InitTwainCombo(hDlg, IDDEST, Destination, MAX_DEST)
					&& InitTwainCombo(hDlg, IDDG, DataGroup, MAX_DG)
					&& InitTwainCombo(hDlg, IDDAT, DataType, MAX_DAT)
					&& InitTwainCombo(hDlg, IDMSG, Message, MAX_MSG)
					&& InitTwainCombo(hDlg, IDCAP, Capability, MAX_CAP)
					&& InitTwainCombo(hDlg, IDTYPE, ConType, MAX_CONTYPE))
			{
				UpdateTripletMenus(hDlg, IDDEST);               
				ShowDialogItem(hDlg, IdItem, IDEDIT);
			} 
			else 
			{
				if (MessageLevel()  >= ML_ERROR)
				{
					ShowRC_CC(hDlg, 0, 0, 0,
								"InitTwainCombo Failed: Verify that Twacker.ini is in the Windows directory.", 
								"SendDlgProc in SPECIAL.C");
				}
				EndDialog(hDlg, 0);
			}
			break;

		case WM_COMMAND:
			switch (wmId)
			{
				/*
				* OnClicked Message handler --
				*/
				case IDDEST:
					if (wmEvent == CBN_SELCHANGE)
					{
						UpdateTripletMenus(hDlg, IDDEST);
						ShowDialogItem(hDlg, IdItem, IDEDIT);
					}
					break;

				/* 
				* OnClicked Message handler --
				*/
				case IDDG:
					if (wmEvent == CBN_SELCHANGE)
					{
						UpdateTripletMenus(hDlg, IDDG);
						ShowDialogItem(hDlg, IdItem, IDEDIT);
					}   
					break;

				/*
				* OnClicked Message handler --
				*/
				case IDDAT:
					if (wmEvent == CBN_SELCHANGE)
					{
						UpdateTripletMenus(hDlg, IDDAT);
						ShowDialogItem(hDlg, IdItem, IDEDIT);
					}   
					break;

				case IDMSG:
					if (wmEvent == CBN_SELCHANGE)
					{
						int iCurrSel = 0;
						int iCurrType = 0;
						pTABLEENTRY pEntryMsg = NULL;
						pTABLEENTRY pEntryConType = NULL;
						int i = 0;
						TW_STR64 text;

						memset(text, 0, sizeof(TW_STR64));

						/*
						*	Get the current Message that has been selected
						*/
						iCurrSel = (int)SendDlgItemMessage(hDlg, IDMSG, CB_GETCURSEL, 0, 0);
						if(iCurrSel >= 0)
						{
							pEntryMsg = (pTABLEENTRY)SendDlgItemMessage(hDlg, IDMSG, CB_GETITEMDATA, iCurrSel, 0);
							
							/*
							*	This entry should be valid
							*/
							ASSERT(pEntryMsg);								
						}
						/*
						* fill in the list box of ConType 
						*/
						if (pEntryMsg->ItemId == MSG_SET)
						{
							InitTwainCombo(hDlg, IDTYPE, ConTypeSet, MAX_CONTYPE-1);
							if(g_pCurrentConType == NULL)
							{
								iCurrSel = (int)SendDlgItemMessage(hDlg, IDTYPE, CB_GETCURSEL, 0, 0);
								if(iCurrSel >= 0)
								{
									g_pCurrentConType = (pTABLEENTRY)SendDlgItemMessage(hDlg, IDTYPE, CB_GETITEMDATA, iCurrSel, 0);
									
									/*
									*	This entry should be valid
									*/
									ASSERT(g_pCurrentConType);								
								}
							}
							else
							{
								TW_UINT16 i = 0;

								/* 
								* Find in set the Current Type
								*/
								for(i = 0; i < MAX_CONTYPE - 1; i++)
								{
									if(ConTypeSet[i].ItemId == g_pCurrentConType->ItemId)
									{
 										SendDlgItemMessage(hDlg, IDTYPE, CB_SELECTSTRING, -1, (LPARAM)ConTypeSet[i].pszItemName);
										break;
									}
								}
							}
							SetConType(g_pCurrentConType, hDlg, IdItem);
							g_bNewEditBox = TRUE;
						}
						else if(pEntryMsg->ItemId == MSG_GET) 
						{
							TW_UINT16 i = 0;
							pTABLECAP pCapEntry  = NULL;

							ShowDialogItem(hDlg, IdItem, IDEDIT);

							SendDlgItemMessage(hDlg, IDEDIT, WM_SETREDRAW, 0, 0L);
							SendDlgItemMessage(hDlg,IDEDIT, WM_SETTEXT, 0, (LPARAM)(LPSTR)"");
							
							/*
							* Fill in edit Box
							*/

							for (i = 0; i < MAX_DATASTRUCTURE; i++)
							{
							/*
							*	Get the current Cap that has been selected
							*/
								iCurrType = (int)SendDlgItemMessage(hDlg, IDCAP, CB_GETCURSEL, 0, 0);
								if(iCurrType >= 0)
								{
									pCapEntry = (pTABLECAP)SendDlgItemMessage(hDlg, IDCAP, CB_GETITEMDATA, iCurrType, 0);
									
									/*
									*	This entry should be valid
									*/
									ASSERT(pCapEntry);								
								}

								InitTwainCombo(hDlg, IDTYPE, ConType, MAX_CONTYPE);
								if(g_pCurrentConType == NULL)
								{
									iCurrSel = (int)SendDlgItemMessage(hDlg, IDTYPE, CB_GETCURSEL, 0, 0);
									if(iCurrSel >= 0)
									{
										g_pCurrentConType = (pTABLEENTRY)SendDlgItemMessage(hDlg, IDTYPE, CB_GETITEMDATA, iCurrSel, 0);
										
										/*
										*	This entry should be valid
										*/
										ASSERT(g_pCurrentConType);								
									}
								}
								else
								{
									TW_UINT16 i = 0;

									/* 
									* Find and set the Current Type 
									*/
									for(i = 0; i < MAX_CONTYPE; i++)
									{
										if(ConType[i].ItemId == g_pCurrentConType->ItemId)
										{
											SendDlgItemMessage(hDlg, IDTYPE, CB_SELECTSTRING, -1, (LPARAM)ConType[i].pszItemName);
											iCurrSel = i;
											break;
										}
									}
								}
								if(lstrcmp(g_pCurrentConType->pszItemName, DataStructure[i].pszDataType) == 0)
								{
									/*
									* special case for the ICAP_FRAMES capability
									*/
									if (pCapEntry->EntryHeader.ItemId == ICAP_FRAMES)
									{
										if(lstrcmp(g_pCurrentConType->pszItemName, "TWON_ONEVALUE") == 0)
										{
											/* 
											* Give the table entry with the special fields for 
											* the ICAP_FRAMES.
											*/
											InitTwainEdit(hDlg, IDEDIT, FRAMEFIELDS, 
																		gszFrameStructure);
											break;
										}
										else
										{
											InitTwainEdit(hDlg, IDEDIT, DataStructure[i].Max, 
																		DataStructure[i].gszDataStructure);
											break;
										}
									}
									else
									{
										InitTwainEdit(hDlg, IDEDIT, DataStructure[i].Max, 
																	DataStructure[i].gszDataStructure);
										break;
									}
								}
							}
							SendDlgItemMessage(hDlg, IDEDIT, WM_SETREDRAW, 1, 0L);
							InvalidateRect(GetDlgItem(hDlg, IDEDIT), NULL, 1);
						}
						/*
						* Other message that MSG_SET and MSG_GET
						*/
						else
						{
							TW_UINT16 i = 0;

							ShowDialogItem(hDlg, IdItem, IDEDIT);
							for(i = 0; i < MAX_DATASTRUCTURE; i++)
							{
								if(lstrcmp(pEntryMsg->pszItemName, DataStructure[i].pszDataType) == 0)
								{
									SetWindowText(GetDlgItem(hDlg,IDEDIT)," ");
									InitTwainEdit(hDlg, IDEDIT, DataStructure[i].Max, 
																DataStructure[i].gszDataStructure);
									break;
								}
								else if(lstrcmp(DataStructure[i].pszDataType, "DAT_FILESYSTEM") == 0)
								{
									TW_STR64 TempText;
									GetDlgItemText(hDlg, IDMSG, TempText, sizeof(TempText));
									if(lstrcmp(TempText, "MSG_DELETE") == 0)
									{}
									else if(lstrcmp(TempText, "MSG_GETINFO") == 0)
									{
										i++;
									}
									else
									{
										i = i+2;
									}
									SetWindowText(GetDlgItem(hDlg,IDEDIT)," ");
									InitTwainEdit(hDlg, IDEDIT, DataStructure[i].Max, 
																DataStructure[i].gszDataStructure);
									break;
								}
							}
						}
					}										
					break;

				/*
				* OnClicked Message handler --
				*/
				case IDCAP:
					if (wmEvent == CBN_SELCHANGE)
					{
						TW_STR64 text;
						GetDlgItemText(hDlg, IDMSG, text, sizeof(text));
						if(lstrcmp(text, "MSG_SET") == 0)
						{
							TW_INT16 i = 0;
							int iCurrSel = 0;
							pTABLECAP pCapEntry = NULL;
							pTABLEENTRY pEntry = NULL;
							/*
							*	Get the current Cap that has been selected
							*/
							iCurrSel = (int)SendDlgItemMessage(hDlg, IDCAP, CB_GETCURSEL, 0, 0);
							if(iCurrSel >= 0)
							{
								pCapEntry = (pTABLECAP)SendDlgItemMessage(hDlg, IDCAP, CB_GETITEMDATA, iCurrSel, 0);
								
								/*
								*	This entry should be valid
								*/
								ASSERT(pCapEntry);								
							}

							GetDlgItemText(hDlg, IDTYPE, text, sizeof(text));
							for(i = 0; i < MAX_CONTYPE; i++)
							{
								if(lstrcmp(text, ConType[i].pszItemName) == 0)
								{
									pEntry = &ConType[i];
									break;
								}
							}
							SetConType(pEntry, hDlg, IdItem);
							g_bNewEditBox = TRUE;
						}
						/*
						* Other message that MSG_SET and MSG_GET
						*/
						else
						{
							TW_UINT16 i = 0;
							BOOL bFound = FALSE;

							GetDlgItemText(hDlg, IDTYPE, text, sizeof(text));
							ShowDialogItem(hDlg, IdItem, IDEDIT);
							for(i = 0; i < MAX_DATASTRUCTURE; i++)
							{
								if(lstrcmp(text, DataStructure[i].pszDataType) == 0)
								{
									SetWindowText(GetDlgItem(hDlg,IDEDIT)," ");
									InitTwainEdit(hDlg, IDEDIT, DataStructure[i].Max, 
																DataStructure[i].gszDataStructure);
									bFound  = TRUE;
									break;
								}
							}
							if(!bFound)
							{
								if(lstrcmp(text, "TWCC_BADPROTOCOL") == 0)
								{
									SetWindowText(GetDlgItem(hDlg,IDEDIT)," ");
									bFound = TRUE;
								}
							}
						}
					}   
					break;

				/*
				* OnClicked Message handler --
				*/
				case IDTYPE:
					if (wmEvent == CBN_SELCHANGE)
					{
						TW_INT16 i = 0;
						int iCurrSel = 0;
						int iCurrType = 0;
						pTABLECAP pCapEntry = NULL;
						TW_STR64 text;
						pTABLEENTRY pEntryConType = NULL;

						GetDlgItemText(hDlg, IDMSG, text, sizeof(text));

						/*
						*	Get the current Type that has been selected
						*/
						iCurrSel = (int)SendDlgItemMessage(hDlg, IDTYPE, CB_GETCURSEL, 0, 0);
						if(iCurrSel >= 0)
						{
						 g_pCurrentConType = (pTABLEENTRY)SendDlgItemMessage(hDlg, IDTYPE, CB_GETITEMDATA, iCurrSel, 0);
							
							/*
							*	This entry should be valid
							*/
							ASSERT(g_pCurrentConType);								
						}

						if (lstrcmp(text, "MSG_GET") == 0)
						{
							SendDlgItemMessage(hDlg, IDEDIT, WM_SETREDRAW, 0, 0L);
							SendDlgItemMessage(hDlg,IDEDIT, WM_SETTEXT, 0, (LPARAM)(LPSTR)"");
							
							/*
							* Fill in edit Box
							*/

							for (i = 0; i < MAX_DATASTRUCTURE; i++)
							{
							/*
							*	Get the current Cap that has been selected
							*/
								iCurrType = (int)SendDlgItemMessage(hDlg, IDCAP, CB_GETCURSEL, 0, 0);
								if(iCurrType >= 0)
								{
									pCapEntry = (pTABLECAP)SendDlgItemMessage(hDlg, IDCAP, CB_GETITEMDATA, iCurrType, 0);
									
									/*
									*	This entry should be valid
									*/
									ASSERT(pCapEntry);								
								}
								if(lstrcmp(g_pCurrentConType->pszItemName, DataStructure[i].pszDataType) == 0)
								{
									/*
									* special case for the ICAP_FRAMES capability
									*/
									if (pCapEntry->EntryHeader.ItemId == ICAP_FRAMES)
									{
										if(lstrcmp(g_pCurrentConType->pszItemName, "TWON_ONEVALUE") == 0)
										{
											/* 
											* Give the table entry with the special fields for 
											* the ICAP_FRAMES.
											*/
											InitTwainEdit(hDlg, IDEDIT, FRAMEFIELDS, 
																		gszFrameStructure);
											break;
										}
										else
										{
											InitTwainEdit(hDlg, IDEDIT, DataStructure[i].Max, 
																		DataStructure[i].gszDataStructure);
											break;
										}
									}
									else
									{
										InitTwainEdit(hDlg, IDEDIT, DataStructure[i].Max, 
																	DataStructure[i].gszDataStructure);
										break;
									}
								}
							}
							SendDlgItemMessage(hDlg, IDEDIT, WM_SETREDRAW, 1, 0L);
							InvalidateRect(GetDlgItem(hDlg, IDEDIT), NULL, 1);
						}
						else if (lstrcmp(text, "MSG_SET") == 0)
						{
							uiNumItems = 0;
							SetConType(g_pCurrentConType, hDlg, IdItem);
							g_bNewEditBox = TRUE;
						}
					}   
					break;

				case IDITEMTYPE:
					if (wmEvent == CBN_SELCHANGE)
					{
						int i = 0;
						int iCurrSel = 0;
						pTABLEENTRY pEntry = NULL;
						pTABLEENTRY pEntryConType = NULL;
						pTABLECAP pTableCap = NULL;
						TW_STR64 text;

						memset(text, 0, sizeof(TW_STR64));

						uiNumItems = 0;
						/*
						*	Get the current Type that has been selected
						*/
						iCurrSel = (int)SendDlgItemMessage(hDlg, IDITEMTYPE, CB_GETCURSEL, 0, 0);
						if(iCurrSel >= 0)
						{
						 pEntry = (pTABLEENTRY)SendDlgItemMessage(hDlg, IDITEMTYPE, CB_GETITEMDATA, iCurrSel, 0);
							
							/*
							*	This entry should be valid
							*/
							ASSERT(pEntry);
						}
						GetCurrentItemType(hDlg, pEntry);
						ClearTheControl(hDlg);

						/*
						* Get the current ConType
						*/
						GetDlgItemText(hDlg, IDTYPE, text, sizeof(text));
						for(i = 0; i < MAX_CONTYPE; i++)
						{
							if(lstrcmp(text, ConType[i].pszItemName) == 0)
							{
								pEntryConType = &ConType[i];
								break;
							}
						}

						/*
						* Get the current Capability
						*/
						GetDlgItemText(hDlg, IDCAP, text, sizeof(text));

						for(i = 0; i < MAX_CAP; i++)
						{
							if(lstrcmp(text, Caps[i].EntryHeader.pszItemName) == 0)
							{
								pTableCap = &Caps[i];
								break;
							}
						}
						ASSERT(pTableCap);

						/*
						* Condition to Display the good control 
						* An Edit Box or a Combo Box in the case
						*/
						if(pEntryConType->ItemId == TWON_ONEVALUE)
						{
							if(pTableCap->pItemTable != NULL)
							{
								if((pEntry->ItemId != TWTY_UINT16) &&
									(pEntry->ItemId != TWTY_FRAME) &&
									(pEntry->ItemId != TWTY_BOOL))
								{
									ShowDialogItem(hDlg, IdItem, IDVALUE);
									g_bNewEditBox = TRUE;
								}
								else
								{
									if(pTableCap->ExpectedType != TWTY_BOOL)
									{
										if(pEntry->ItemId == TWTY_UINT16)
										{
											ShowDialogItem(hDlg, IdItem, IDVALUECB);
											g_bNewEditBox = TRUE;
										}
										else if(pEntry->ItemId == TWTY_BOOL)
										{
											ShowDialogItem(hDlg, IdItem, IDVALUE);
											g_bNewEditBox = TRUE;
										}
										else if(pEntry->ItemId == TWTY_FRAME)
										{
											ShowDialogItem(hDlg, IdItem, IDLEFT);
											g_bNewEditBox = TRUE;
										}
									}
									else
									{
										if(pEntry->ItemId == TWTY_BOOL)
										{
											ShowDialogItem(hDlg, IdItem, IDVALUECB);
											g_bNewEditBox = TRUE;
										}
										else if(pEntry->ItemId == TWTY_FRAME)
										{
											ShowDialogItem(hDlg, IdItem, IDLEFT);
											g_bNewEditBox = TRUE;
										}
										else if(pEntry->ItemId == TWTY_UINT16)
										{
											ShowDialogItem(hDlg, IdItem, IDVALUE);
											g_bNewEditBox = TRUE;
										}
									}
								}
							}
							else
							{
								if(pEntry->ItemId !=TWTY_FRAME)
								{
									ShowDialogItem(hDlg, IdItem, IDVALUE);
									g_bNewEditBox = TRUE;
								}
								else
								{
									ShowDialogItem(hDlg, IdItem, IDLEFT);
									g_bNewEditBox = TRUE;
								}
							}
						}
					}
					break;

				/* 
				* In case of TWON_ENUMERATION or TWON_ARRAY
				*/
				case IDNUMITEMS:
					if(wmEvent == EN_KILLFOCUS)
					{
						TW_UINT32 uiItems = 0;
						TW_STR64 text;
						pTABLECAP pTableCap = NULL;
						TW_UINT32 i = 0;

						memset(text, 0, sizeof(TW_STR64));

						/*
						* Get the current capability
						*/
						GetDlgItemText(hDlg, IDCAP, text, sizeof(text));

						for(i = 0; i < MAX_CAP; i++)
						{
							if(lstrcmp(text, Caps[i].EntryHeader.pszItemName) == 0)
							{
								pTableCap = &Caps[i];
								break;
							}
						}
						ASSERT(pTableCap);

						/*
						* get the number in NumItems EditBox
						*/
						lstrcpy(text, "");
						GetWindowText(GetDlgItem(hDlg, IDNUMITEMS), text, 64);
						if(lstrcmp(text, "") != 0)
						{
							TW_UINT32 temp = 0;
							TW_UINT32 i = 0;
							int Val = 0;

							Val = (int)atoi(text);

							/*
							* Look if the number is bigger that 0
							*/
							if(Val > 0)
							{
								uiItems = Val;	
								/*
								* Get the current ConType
								*/
								GetDlgItemText(hDlg, IDTYPE, text, sizeof(text));

								if(lstrcmp(text, "TWON_ENUMERATION") == 0)
								{
									SetItemListBox(hDlg, uiItems, pTableCap, IDITEMLISTENUM);
								}

								if(lstrcmp(text, "TWON_ARRAY") == 0)
								{
									SetItemListBox(hDlg, uiItems, pTableCap, IDITEMLISTARRAY);
								}
							}
							/*
							* For value <= 0
							*/
							else
							{
								ShowRC_CC(hDlg, 0, 0, 0,
											"The item value should be bigger that 0\r\n", "Error");
							}
						}
					}
					else if(wmEvent == EN_SETFOCUS)
					{
						int i = 0;

						for(i = 0; i < MAX_TYPES; i++)
						{
							if(Types[i].ItemId == TWTY_UINT32)
							{
								GetCurrentItemType(hDlg, &Types[i]);
							}
						}
					}
					break;

				/*
				* In case of TWON_ENUMERATION when it's MSG_SET
				*/
				case IDDEFAULTINDEX:
				case IDCURRENTINDEX:
					if(wmEvent == EN_SETFOCUS)
					{
						int i = 0;

						for(i = 0; i < MAX_TYPES; i++)
						{
							if(Types[i].ItemId == TWTY_UINT32)
							{
								GetCurrentItemType(hDlg, &Types[i]);
							}
						}
					}
					break;

				/*
				* In case of TWON_ENUMERATION
				*/
				case IDITEMLISTENUM:
					SetControlListBox(hDlg, IDITEMLISTENUM, wmEvent);
					break;

				/*
				* In case of TWON_ARRAY
				*/
				case IDITEMLISTARRAY:
					SetControlListBox(hDlg, IDITEMLISTARRAY, wmEvent);
					break;

				/*
				* When a Combo Box is create in the ListBox
				*/
				case IDLISTSELBOX:
					{
						if(g_hDynamicListBox != NULL)
						{						
						  TW_UINT32 IdListBox = 0;
						  TW_STR64 text;

						  memset(text, 0, sizeof(TW_STR64));

						  GetDlgItemText(hDlg, IDTYPE, text, sizeof(text));
						  if(lstrcmp(text, "TWON_ENUMERATION") == 0)
						  {
							  IdListBox = IDITEMLISTENUM;		
						  }
						  else if(lstrcmp(text, "TWON_ARRAY") == 0)
						  {
							  IdListBox = IDITEMLISTARRAY;								
						  }
              TRACE("List box event %d\n", wmEvent);
						 /* if(wmEvent == LBN_SELCHANGE)
						  {
							  int iCurrSel = SendDlgItemMessage(hDlg, IDLISTSELBOX, LB_GETCURSEL, 0, 0);
                if(iCurrSel >= 0)
                {
							    TW_STR64 text;
							    memset(text, 0, sizeof(TW_STR64));
                  SendDlgItemMessage(hDlg, IDLISTSELBOX, LB_GETTEXT, iCurrSel, (LPARAM)(LPSTR)text);
                  
							    SendDlgItemMessage(hDlg, IdListBox, LB_DELETESTRING, g_ListBoxSel, 0);
							    SendDlgItemMessage(hDlg, IdListBox, LB_INSERTSTRING, g_ListBoxSel, (LPARAM)(LPSTR)text);
                }
						  }
              */
						  if( wmEvent == LBN_KILLFOCUS
               || wmEvent == LBN_DBLCLK )
						  {
							  int iCurrSel = (int)SendDlgItemMessage(hDlg, IDLISTSELBOX, LB_GETCURSEL, 0, 0);
                if(iCurrSel >= 0)
                {
							    TW_STR64 text;
							    memset(text, 0, sizeof(TW_STR64));
                  SendDlgItemMessage(hDlg, IDLISTSELBOX, LB_GETTEXT, iCurrSel, (LPARAM)(LPSTR)text);
                  
							    SendDlgItemMessage(hDlg, IdListBox, LB_DELETESTRING, g_ListBoxSel, 0);
							    SendDlgItemMessage(hDlg, IdListBox, LB_INSERTSTRING, g_ListBoxSel, (LPARAM)(LPSTR)text);
                }

							  EnableWindow(GetDlgItem(hDlg, IdListBox), TRUE);

							  //DestroyWindow(g_hDynamicListBox);
                PostMessage(g_hDynamicListBox, WM_CLOSE, 0, 0);
                g_hDynamicListBox = NULL;

                TRACE("Destroying the combo box.\n");
						  }
            }
					}
					break;

				/*
				* When an Edit Box is create in the List Box
				*/
				case IDEDITBOX:
					{
						TW_UINT32 IdListBox = 0;
						TW_STR64 text;

						memset(text, 0, sizeof(TW_STR64));

						GetDlgItemText(hDlg, IDTYPE, text, sizeof(text));
						if(lstrcmp(text, "TWON_ENUMERATION") == 0)
						{
							IdListBox = IDITEMLISTENUM;		
						}
						else if(lstrcmp(text, "TWON_ARRAY") == 0)
						{
							IdListBox = IDITEMLISTARRAY;								
						}
						if(wmEvent == EN_KILLFOCUS)
							{
							if(g_hEditBox != NULL)
							{
								TW_STR64 text;
								float Value = 0;
								TW_STR64 string;
								int i = 0;
					
								memset(text, 0, sizeof(TW_STR64));

								GetWindowText(g_hEditBox, text, 64);


								if((g_pCurrentItemType->ItemId == TWTY_FIX32) ||
									 (g_pCurrentItemType->ItemId == TWTY_FRAME))
								{
									Value = (float)atof(text);
									sprintf_s(string, sizeof(string), "%.3f", Value);

									SendDlgItemMessage(hDlg, IdListBox, LB_DELETESTRING, g_ListBoxSel, 0);
									SendDlgItemMessage(hDlg, IdListBox, LB_INSERTSTRING, g_ListBoxSel, (LPARAM)(LPSTR)string);
								}
								else
								{
									SendDlgItemMessage(hDlg, IdListBox, LB_DELETESTRING, g_ListBoxSel, 0);
									SendDlgItemMessage(hDlg, IdListBox, LB_INSERTSTRING, g_ListBoxSel, (LPARAM)(LPSTR)text);
								}
								if((g_pCurrentItemType->ItemId != TWTY_STR32) &&
									 (g_pCurrentItemType->ItemId != TWTY_STR64) &&
									 (g_pCurrentItemType->ItemId != TWTY_STR128) &&
									 (g_pCurrentItemType->ItemId != TWTY_STR255))
								{						
									UnSubClassEditBox(g_uiOldIdControl, hDlg);
								}

								TRACE("UnSubClass Edit Box.\n");
								DestroyWindow(g_hEditBox);
								g_hEditBox = NULL;
								TRACE("Destroying the edit box.\n");
								EnableWindow(GetDlgItem(hDlg, IdListBox), TRUE);

								for(i = 0; i < MAX_TYPES; i++)
								{
									if(Types[i].ItemId == TWTY_UINT32)
									{
										GetCurrentItemType(hDlg, &Types[i]);
										break;
									}
								}
							}
						}
					}
					break;

				/*
				* Send button message handler
				*/
				case IDOK:
					{
						TW_INT16 dg = 0;
						TW_INT16 dat = 0;
						TW_INT16 msg = 0;
						TW_STR64 error;
						TW_UINT16 status = TWRC_FAILURE;
						pTABLEENTRY pEntry = NULL;
 						int iCurrSel = 0;

						memset(error, 0, sizeof(TW_STR64));

						/*
						*	Get the current Data Group that has been selected
						*/
						iCurrSel = (int)SendDlgItemMessage(hDlg, IDDG, CB_GETCURSEL, 0, 0);
						if(iCurrSel >= 0)
						{
							/*
							*	Get the ID from the table entry
							*/
							pEntry = (pTABLEENTRY)SendDlgItemMessage(hDlg, IDDG, CB_GETITEMDATA, iCurrSel, 0);

							/*
							*	This entry should be valid
							*/
							ASSERT(pEntry);

							dg = pEntry->ItemId;
						}

						/*
						*	Get the current message that has been selected
						*/
						iCurrSel = (int)SendDlgItemMessage(hDlg, IDMSG, CB_GETCURSEL, 0, 0);
						if(iCurrSel >= 0)
						{
							/*
							*	Get the ID from the table entry
							*/
							pEntry = (pTABLEENTRY)SendDlgItemMessage(hDlg, IDMSG, CB_GETITEMDATA, iCurrSel, 0);
							
							/*
							*	This entry should be valid
							*/
							ASSERT(pEntry);

							msg = pEntry->ItemId;
						}

						/*
						*	Get the current data type that has been selected
						*/
						iCurrSel = (int)SendDlgItemMessage(hDlg, IDDAT, CB_GETCURSEL, 0, 0);
						if(iCurrSel >= 0)
						{
							/*
							*	Get the ID from the table entry
							*/
							pEntry = (pTABLEENTRY)SendDlgItemMessage(hDlg, IDDAT, CB_GETITEMDATA, iCurrSel, 0);
							
							/*
							*	This entry should be valid
							*/
							ASSERT(pEntry);

							dat = pEntry->ItemId;
						}

						switch(dg)
						{
							case DG_CONTROL:
								status = ControlMsg(dat,msg,hDlg);
								break;

							case DG_IMAGE:
								status = ImageMsg(dat,msg,hDlg);
								break;

							case DG_AUDIO:
								status = AudioMsg(dat, msg, hDlg);
								break;

							default:
								break;
						}

						/*
						* default to success
						*/
						lstrcpyn(error,"TWRC_SUCCESS",sizeof(error));

						if (status != TWRC_SUCCESS)
						{
							TW_STR64 temp;

							memset(temp, 0, sizeof(TW_STR64));

							MatchTwainInt(ReturnCode, MAX_RETURNCODE, (TW_INT32)status, 
														error);

							MatchTwainInt(ConditionCode, MAX_CONDITIONCODE, 
														(TW_INT32)gGlobalStatus.ConditionCode,
														temp);
							lstrcat(error," -- ");
							lstrcat(error,temp);
						}
						SetWindowText(GetDlgItem(hDlg,IDC_STATUS),error);
					}   
  					break;

				/* 
				* status button message handler
				*/
				case IDSTATUS:
					{
						TW_STATUS   result;
						TW_STR32    error;

						memset(&result, 0, sizeof(TW_STATUS));
						memset(error, 0, sizeof(TW_STR32));

						MatchTwainInt(ConditionCode, MAX_CONDITIONCODE, 
													(TW_INT32)gGlobalStatus.ConditionCode,
													error);

						MessageBox(hDlg,error,"Condition Code",MB_OK);
					}   
					break;

				case IDCANCEL:
					uiNumItems = 0;
					g_pCurrentConType = NULL;
					EndDialog(hDlg, 0);
					UnSubClassEditBox(g_uiOldIdControl, hDlg);
					break;
				
				default:
					break;
			}   
			break;

		default:
			return 0;
	}   
	return 1;
}   

/*
* Function: SetControlListBox
* Author: Nancy Létourneau / J.F.L. Peripheral Solutions Inc. / TWAIN Working Group
* Date: September 14, 1998
* Input:
*		hDlg: Handle tho the dialog window
*		IdListbox: The current ListBox.
*		wmEvent: The event of the control
* Output: none
* Comment:
*		Display the good Control when the user double click on a List box item.
*/
void SetControlListBox(HWND hDlg, TW_UINT32 IdListBox, TW_UINT32 wmEvent)
{
	int iCurrSel = 0;
	TW_STR64 text;
	pTABLECAP pTableCap = NULL;
	TW_UINT32 i = 0;
	pTABLEENTRY pItemTypeEntry = NULL;

	memset(text, 0, sizeof(TW_STR64));

	/*
	* Get the current Capabitity
	*/
	GetDlgItemText(hDlg, IDCAP, text, sizeof(text));

	for(i = 0; i < MAX_CAP; i++)
	{
		if(lstrcmp(text, Caps[i].EntryHeader.pszItemName) == 0)
		{
			pTableCap = &Caps[i];
			break;
		}
	}

	/*
	* Get the urrent ItemType
	*/
	GetDlgItemText(hDlg, IDITEMTYPE, text, sizeof(text));

	for(i = 0; i < MAX_TYPES; i++)
	{
		if(lstrcmp(text, Types[i].pszItemName) == 0)
		{
			pItemTypeEntry = &Types[i];
      break;
		}
	}

	/*
	* When you double click on an item in the list box
	*/
	if(wmEvent == LBN_DBLCLK)
	{
		RECT Rect;
		POINT PtLeftTop;
		POINT PtRightBottom;

		memset(&Rect, 0, sizeof(RECT));

		iCurrSel = (int)SendDlgItemMessage(hDlg, IdListBox, LB_GETCURSEL, 0, 0);

		if(iCurrSel >= 0)
		{
			g_ListBoxSel = iCurrSel;
			SendDlgItemMessage(hDlg, IdListBox, LB_GETITEMRECT, iCurrSel, (LPARAM)&Rect);
			
			PtLeftTop.x = Rect.left;
			PtLeftTop.y = Rect.top;

			ClientToScreen(GetDlgItem(hDlg, IdListBox), &PtLeftTop);
			ScreenToClient(hDlg, &PtLeftTop);

			PtRightBottom.x = Rect.right;
			PtRightBottom.y = Rect.bottom;

			ClientToScreen(GetDlgItem(hDlg, IdListBox), &PtRightBottom);
			ScreenToClient(hDlg, &PtRightBottom);

			/*
			* Create a Combo Box to SET the value with a choice
			*/
			if((pTableCap->pItemTable != NULL) && 
				 (pItemTypeEntry->ItemId == pTableCap->ExpectedType))
			{
/*				g_hComboBox = CreateWindowEx(0, "Combobox", "", 
										WS_BORDER | WS_CHILD | WS_TABSTOP | WS_VISIBLE |	CBS_DROPDOWNLIST | 
										CBS_AUTOHSCROLL | CBS_SORT | WS_VSCROLL, 
										PtLeftTop.x, PtLeftTop.y, 
										PtRightBottom.x - PtLeftTop.x, 120, 
										hDlg, (HMENU)IDLISTSELBOX, hInst, NULL); 
                    */
				g_hDynamicListBox = CreateWindowEx(0, "Listbox", "", 
										WS_CHILD | WS_TABSTOP | WS_VISIBLE | WS_VSCROLL |
										LBS_NOTIFY | LBS_SORT | LBS_HASSTRINGS, 
										PtLeftTop.x, PtRightBottom.y , 
										PtRightBottom.x - PtLeftTop.x, 120, 
										hDlg, (HMENU)IDLISTSELBOX, hInst, NULL); 

				TRACE("Create combo box.\n");
        if(g_hDynamicListBox)
        {
          WPARAM wParam = (WPARAM) SendMessage(hDlg, WM_GETFONT, 0, 0);
          SendMessage(g_hDynamicListBox, WM_SETFONT, wParam, MAKELPARAM(FALSE,0));
        }

				//InitTwainCombo(hDlg, IDLISTSELBOX, pTableCap->pItemTable, 
				//							 pTableCap->ItemTableSize);
				InitTwainList(hDlg, IDLISTSELBOX, pTableCap->pItemTable, 
											 pTableCap->ItemTableSize);

				SendDlgItemMessage(hDlg, IdListBox, LB_GETTEXT, iCurrSel, (LPARAM)(LPSTR)text);
				SendDlgItemMessage(hDlg, IDLISTSELBOX, CB_SELECTSTRING, -1, (LPARAM)text);
				
				SetFocus(g_hDynamicListBox);
				EnableWindow(GetDlgItem(hDlg, IdListBox), FALSE);
			}
			else
			{
				/*
				* Create a Edit Box to SET the value
				*/
				if(pTableCap->ExpectedType != TWTY_FRAME)
				{
					g_hEditBox = CreateWindow("edit", "", 
											WS_CHILD | WS_VISIBLE, 
											PtLeftTop.x, PtLeftTop.y, 
											PtRightBottom.x - PtLeftTop.x, 
											PtRightBottom.y - PtLeftTop.y, 
											GetDlgItem(hDlg, IdListBox),	(HMENU)IDEDITBOX, hInst, NULL);

					TRACE("Create edit box.\n");

					GetCurrentItemType(hDlg, pItemTypeEntry);

					if((pItemTypeEntry->ItemId != TWTY_STR32) &&
						 (pItemTypeEntry->ItemId != TWTY_STR64) &&
						 (pItemTypeEntry->ItemId != TWTY_STR128) &&
						 (pItemTypeEntry->ItemId != TWTY_STR255))
					{						
#ifdef WIN64
						g_fnOldControl = (WNDPROC)SetWindowLongPtr(g_hEditBox, GWLP_WNDPROC, 
													 (LPARAM)SubClassEditProc);
#else
						g_fnOldControl = (WNDPROC)SetWindowLong(g_hEditBox, GWL_WNDPROC, 
													 (LPARAM)SubClassEditProc);
#endif
						SetProp(g_hEditBox, g_szSubClassEditBoxProc, g_fnOldControl);
						TRACE("SubClass Edit Box.\n");

						g_uiOldIdControl = IDEDITBOX;
					}
					SendDlgItemMessage(hDlg,IdListBox,LB_GETTEXT,iCurrSel,(LPARAM)(LPSTR)text);
					SendDlgItemMessage(hDlg,IDEDITBOX,EM_REPLACESEL,iCurrSel,(LPARAM)(LPSTR)text);

					SetFocus(g_hEditBox);
					EnableWindow(GetDlgItem(hDlg, IdListBox), FALSE);
				}
				/*
				* Create a Dialog Box to SET the value of TW_FRAME
				*/
				else
				{
					BOOL Change = FALSE;
					char *token = NULL;
          char *next_token = NULL;
					TW_STR64 string;

					memset(&g_Frame, 0, sizeof(TW_FRAME));
					memset(string, 0, sizeof(TW_STR64));

					/*
					* Take the current value of Frame in the ListBox and put it in 
					* a Frame Structure.
					*/
					SendDlgItemMessage(hDlg, IdListBox, LB_GETTEXT, iCurrSel, (LPARAM)(LPSTR)string);
						
					token = strtok_s(string, ",",&next_token);

					while(token != NULL)
					{
						if(token[0] == 'L')
						{
							RemoveEquals(token, lstrlen(token));
							g_Frame.Left = FloatToFIX32((float)atof(token));
						}
						else if(token[0] == 'T')
						{
							RemoveEquals(token, lstrlen(token));
							g_Frame.Top = FloatToFIX32((float)atof(token));
						}
						else if(token[0] == 'R')
						{
							RemoveEquals(token, lstrlen(token));
							g_Frame.Right = FloatToFIX32((float)atof(token));
						}
						else if(token[0] == 'B')
						{
							RemoveEquals(token, lstrlen(token));
							g_Frame.Bottom = FloatToFIX32((float)atof(token));
						}
						token = strtok_s(NULL,",",&next_token);
					}

					/*
					* Create a Dialog Box to SET the Frame values
					*/
					Change = (BOOL)DialogBox(hInst, MAKEINTRESOURCE(DLG_FRAME), hDlg, FrameDlgProc);
					TRACE("Create dialog box.\n");
	
					if(Change == TRUE)
					{
						TW_STR128 buffer;
						float fl = 0;

						memset(buffer, 0, sizeof(TW_STR128));

						lstrcpy(buffer, "L=");
						fl = FIX32ToFloat(g_Frame.Left);
						sprintf_s(string, sizeof(string), "%.3f", fl);
						lstrcat(buffer, string);

						lstrcat(buffer, ",T=");
						fl = FIX32ToFloat(g_Frame.Top);
						sprintf_s(string, sizeof(string), "%.3f", fl);
						lstrcat(buffer, string);

						lstrcat(buffer, ",R=");
						fl = FIX32ToFloat(g_Frame.Right);
						sprintf_s(string, sizeof(string), "%.3f", fl);
						lstrcat(buffer, string);

						lstrcat(buffer, ",B=");
						fl = FIX32ToFloat(g_Frame.Bottom);
						sprintf_s(string, sizeof(string), "%.3f", fl);
						lstrcat(buffer, string);

						SendDlgItemMessage(hDlg, IdListBox, LB_DELETESTRING, iCurrSel, 0);
						SendDlgItemMessage(hDlg, IdListBox, LB_INSERTSTRING, iCurrSel, (LPARAM)(LPSTR)buffer);
					}
				}
			}
		}
	}
	return;
}

/*
* Function: FrameDlgProc
* Author: Nancy Létourneau / J.F.L. Peripheral Solutions Inc. / TWAIN Working Group
* Date: August 18, 1998
* Input:
*		hDlgFrame - handle to the dialog window for Frame
*		wMsg - variable containing the message to process
*		wParam - variable containing virtual keycode
*		lParam - variable containing additional information about the message
*			to process.
* Output:
*		status - 1 if dialog command processed
*					 - 0 if dialog command received is invalid
* Comment:
*		Frame dialog procedure
*		This procedure processes commands for any generic dialog box.
*		Caption the window caption.
*/
INT_PTR FAR PASCAL FrameDlgProc (HWND hDlgFrame,
                              UINT message,
                              WPARAM wParam,
                              LPARAM lParam)
{
	TW_UINT16 wmId = 0;
	TW_UINT16 wmEvent = 0;
	TW_STR64 string;
	TW_STR128 buffer;
	pTABLECAP pTableCap = NULL;
	float fl = 0;
	int count = 0;
	int i = 0;
	HWND hItem;

	memset(string, 0, sizeof(TW_STR64));
	memset(buffer, 0, sizeof(TW_STR128));

#if defined(WIN32) || defined(WIN64)
		wmId = LOWORD(wParam);
		wmEvent = HIWORD (wParam);
	#else
		wmId = wParam;
		wmEvent = HIWORD (lParam);
	#endif

	ASSERT(hDlgFrame);

	for(i = 0; i < MAX_TYPES; i++)
	{
		if(Types[i].ItemId == TWTY_FRAME)
		{
			GetCurrentItemType(hDlgFrame, &Types[i]);
			break;
		}
	}

	g_hMainFrameDlg = hDlgFrame;

	GetDlgItemText(g_hMainDlg, IDCAP, string, sizeof(string));

	for(i = 0; i < MAX_CAP; i++)
	{
		if(lstrcmp(string, Caps[i].EntryHeader.pszItemName) == 0)
		{
			pTableCap = &Caps[i];
			break;
		}
	}
	if(pTableCap->EntryHeader.ItemId == ICAP_FRAMES)
	{
		g_bFrameDlg = TRUE;
	}

	switch (message)
	{
		case WM_INITDIALOG:
			fl = FIX32ToFloat(g_Frame.Left);
			sprintf_s(string, sizeof(string), "%.3f", fl);
			SendDlgItemMessage(hDlgFrame, IDLEFT, EM_REPLACESEL, 0, (LPARAM)(LPSTR)string);
			hItem = GetDlgItem(hDlgFrame, IDLEFT);
#ifdef WIN64
			g_fnOldControl = (WNDPROC)SetWindowLongPtr(GetDlgItem(hDlgFrame, IDLEFT), GWLP_WNDPROC, 
										 (LPARAM)SubClassEditProc);
#else
			g_fnOldControl = (WNDPROC)SetWindowLong(GetDlgItem(hDlgFrame, IDLEFT), GWL_WNDPROC, 
										 (LPARAM)SubClassEditProc);
#endif

			SetProp(GetDlgItem(hDlgFrame, IDLEFT), g_szSubClassEditBoxProc, g_fnOldControl);
			g_uiOldIdControl = IDLEFT;

			fl = FIX32ToFloat(g_Frame.Top);
			sprintf_s(string, sizeof(string), "%.3f", fl);
			SendDlgItemMessage(hDlgFrame, IDTOP, EM_REPLACESEL, 0, (LPARAM)(LPSTR)string);
#ifdef WIN64
			g_fnOldControl = (WNDPROC)SetWindowLongPtr(GetDlgItem(hDlgFrame, IDTOP), GWLP_WNDPROC, 
										 (LPARAM)SubClassEditProc);
#else
			g_fnOldControl = (WNDPROC)SetWindowLong(GetDlgItem(hDlgFrame, IDTOP), GWL_WNDPROC, 
										 (LPARAM)SubClassEditProc);
#endif
			SetProp(GetDlgItem(hDlgFrame, IDTOP), g_szSubClassEditBoxProc, g_fnOldControl);
			g_uiOldIdControl = IDTOP;

			fl = FIX32ToFloat(g_Frame.Right);
			sprintf_s(string, sizeof(string), "%.3f", fl);
			SendDlgItemMessage(hDlgFrame, IDRIGHT, EM_REPLACESEL, 0, (LPARAM)(LPSTR)string);
#ifdef WIN64
			g_fnOldControl = (WNDPROC)SetWindowLongPtr(GetDlgItem(hDlgFrame, IDRIGHT), GWLP_WNDPROC, 
										 (LPARAM)SubClassEditProc);
#else
			g_fnOldControl = (WNDPROC)SetWindowLong(GetDlgItem(hDlgFrame, IDRIGHT), GWL_WNDPROC, 
										 (LPARAM)SubClassEditProc);
#endif

			SetProp(GetDlgItem(hDlgFrame, IDRIGHT), g_szSubClassEditBoxProc, g_fnOldControl);
			g_uiOldIdControl = IDRIGHT;

			fl = FIX32ToFloat(g_Frame.Bottom);
			sprintf_s(string, sizeof(string), "%.3f", fl);
			SendDlgItemMessage(hDlgFrame, IDBOTTOM, EM_REPLACESEL, 0, (LPARAM)(LPSTR)string);
#ifdef WIN64
			g_fnOldControl = (WNDPROC)SetWindowLongPtr(GetDlgItem(hDlgFrame, IDBOTTOM), GWLP_WNDPROC, 
										 (LPARAM)SubClassEditProc);
#else
			g_fnOldControl = (WNDPROC)SetWindowLong(GetDlgItem(hDlgFrame, IDBOTTOM), GWL_WNDPROC, 
										 (LPARAM)SubClassEditProc);
#endif

      SetProp(GetDlgItem(hDlgFrame, IDBOTTOM), g_szSubClassEditBoxProc, g_fnOldControl);
			g_uiOldIdControl = IDBOTTOM;
			break;

		case WM_COMMAND:
			switch(wmId)
			{
				/*
				* OK button message handler
				*/
				case IDOK:
					count = GetWindowText(GetDlgItem(hDlgFrame, IDLEFT), string, 64);
					g_Frame.Left = FloatToFIX32((float)atof(string));

					count = GetWindowText(GetDlgItem(hDlgFrame, IDTOP), string, 64);
					g_Frame.Top = FloatToFIX32((float)atof(string));

					count = GetWindowText(GetDlgItem(hDlgFrame, IDRIGHT), string, 64);
					g_Frame.Right = FloatToFIX32((float)atof(string));

					count = GetWindowText(GetDlgItem(hDlgFrame, IDBOTTOM), string, 64);
					g_Frame.Bottom = FloatToFIX32((float)atof(string));

					if((g_Frame.Left.Whole >= g_Frame.Right.Whole) && (g_Frame.Top.Whole >= g_Frame.Bottom.Whole))
					{
						ShowRC_CC(hDlgFrame, 0, 0, 0,
									"The Left value should be smaller that the Right value and\r\n"
									"the Top value should be smaller that the Bottom value\r\n", 
									"Error");
					}
					else if(g_Frame.Left.Whole >= g_Frame.Right.Whole)
					{
						ShowRC_CC(hDlgFrame, 0, 0, 0,
									"The Left value should be smaller that the Right value\r\n", "Error");
					}
					else if(g_Frame.Top.Whole >= g_Frame.Bottom.Whole)
					{
						ShowRC_CC(hDlgFrame, 0, 0, 0,
									"The Top value should be smaller that the Bottom value\r\n", "Error");
					}

					 
					UnSubClassEditBox(g_uiOldIdControl, hDlgFrame);
					g_uiOldIdControl = IDCURRENTINDEX;
					g_bFrameDlg = FALSE;
					EndDialog(hDlgFrame, TRUE);
					break;

				/*
				* Cancel button message handler
				*/
				case IDCANCEL:
					UnSubClassEditBox(g_uiOldIdControl, hDlgFrame);
					g_bFrameDlg = FALSE;
					EndDialog(hDlgFrame, TRUE);
					break;

				default:
					break;
			}
			break;

		default:
			break;
	}
	return (FALSE);            
} 

/*
* Function: ShowDialogItem
* Author: Nancy Létourneau / J.F.L. Peripheral Solutions Inc. / TWAIN Working Group
* Input:
*		hDlg - handle to the dialog window
*		IdItem - handle to the dialog item 
*		iShowItem - item to be show in the Dialog box for MSG_SET
* Output: none
* Comments:
*		Recieve in input variable the dialog item to be show if it's MSG_SET
*/
static void ShowDialogItem(HWND hDlg, HWND IdItem, int iShowItem)
{
	int i = 0;
	TW_STR32 string;
	TW_STR32 szItemType;
	memset(string, 0, sizeof(TW_STR32));
	memset(szItemType, 0, sizeof(TW_STR32));
	

	UnSubClassEditBox(g_uiOldIdControl, hDlg);
	uiNumItems = 0;
	ClearTheControl(hDlg);
	GetDlgItemText(hDlg, IDITEMTYPE, szItemType, sizeof(szItemType));

	/*
	* Hide all the Dialog Control before show the current one.
	*/
	IdItem = GetDlgItem(hDlg, IDC_STATIC_ITEMTYPE);
	ShowWindow(IdItem, SW_HIDE);
	IdItem = GetDlgItem(hDlg, IDITEMTYPE);
	ShowWindow(IdItem, SW_HIDE);

	IdItem = GetDlgItem(hDlg, IDC_STATIC_VALUE);
	ShowWindow(IdItem, SW_HIDE);
	IdItem = GetDlgItem(hDlg, IDVALUECB);
	ShowWindow(IdItem, SW_HIDE);
	IdItem = GetDlgItem(hDlg, IDVALUE);
	ShowWindow(IdItem, SW_HIDE);

	IdItem = GetDlgItem(hDlg, IDC_STATIC_LEFT);
	ShowWindow(IdItem, SW_HIDE);
	IdItem = GetDlgItem(hDlg, IDLEFT);
	ShowWindow(IdItem, SW_HIDE);

	IdItem = GetDlgItem(hDlg, IDC_STATIC_TOP);
	ShowWindow(IdItem, SW_HIDE);
	IdItem = GetDlgItem(hDlg, IDTOP);
	ShowWindow(IdItem, SW_HIDE);

	IdItem = GetDlgItem(hDlg, IDC_STATIC_RIGHT);
	ShowWindow(IdItem, SW_HIDE);
	IdItem = GetDlgItem(hDlg, IDRIGHT);
	ShowWindow(IdItem, SW_HIDE);

	IdItem = GetDlgItem(hDlg, IDC_STATIC_BOTTOM);
	ShowWindow(IdItem, SW_HIDE);
	IdItem = GetDlgItem(hDlg, IDBOTTOM);
	ShowWindow(IdItem, SW_HIDE);

	IdItem = GetDlgItem(hDlg, IDC_STATIC_MINVALUE);
	ShowWindow(IdItem, SW_HIDE);
	IdItem = GetDlgItem(hDlg, IDMINVALUE);
	ShowWindow(IdItem, SW_HIDE);

	IdItem = GetDlgItem(hDlg, IDC_STATIC_MAXVALUE);
	ShowWindow(IdItem, SW_HIDE);
	IdItem = GetDlgItem(hDlg, IDMAXVALUE);
	ShowWindow(IdItem, SW_HIDE);

	IdItem = GetDlgItem(hDlg, IDC_STATIC_STEPSIZE);
	ShowWindow(IdItem, SW_HIDE);
	IdItem = GetDlgItem(hDlg, IDSTEPSIZE);
	ShowWindow(IdItem, SW_HIDE);

	IdItem = GetDlgItem(hDlg, IDC_STATIC_DEFAULTVALUE);
	ShowWindow(IdItem, SW_HIDE);
	IdItem = GetDlgItem(hDlg, IDDEFAULTVALUE);
	ShowWindow(IdItem, SW_HIDE);

	IdItem = GetDlgItem(hDlg, IDC_STATIC_CURRENTVALUE);
	ShowWindow(IdItem, SW_HIDE);
	IdItem = GetDlgItem(hDlg, IDCURRENTVALUE);
	ShowWindow(IdItem, SW_HIDE);

	IdItem = GetDlgItem(hDlg, IDC_STATIC_NUMITEMS);
	ShowWindow(IdItem, SW_HIDE);
	IdItem = GetDlgItem(hDlg, IDNUMITEMS);
	ShowWindow(IdItem, SW_HIDE);

	IdItem = GetDlgItem(hDlg, IDC_STATIC_ITEMLISTARRAY);
	ShowWindow(IdItem, SW_HIDE);
	IdItem = GetDlgItem(hDlg, IDITEMLISTARRAY);
	ShowWindow(IdItem, SW_HIDE);

	IdItem = GetDlgItem(hDlg, IDC_STATIC_CURRENTINDEX);
	ShowWindow(IdItem, SW_HIDE);
	IdItem = GetDlgItem(hDlg, IDCURRENTINDEX);
	ShowWindow(IdItem, SW_HIDE);

	IdItem = GetDlgItem(hDlg, IDC_STATIC_DEFAULTINDEX);
	ShowWindow(IdItem, SW_HIDE);
	IdItem = GetDlgItem(hDlg, IDDEFAULTINDEX);
	ShowWindow(IdItem, SW_HIDE);

	IdItem = GetDlgItem(hDlg, IDC_STATIC_ITEMLISTENUM);
	ShowWindow(IdItem, SW_HIDE);
	IdItem = GetDlgItem(hDlg, IDITEMLISTENUM);
	ShowWindow(IdItem, SW_HIDE);

	IdItem = GetDlgItem(hDlg, IDEDIT);
	ShowWindow(IdItem, SW_HIDE);
	switch(iShowItem)
	{
		/*
		* OneValue Container
		* With defined Types
		*/
		case IDVALUECB:
			IdItem = GetDlgItem(hDlg, IDC_STATIC_ITEMTYPE);
			ShowWindow(IdItem, SW_SHOW);
			IdItem = GetDlgItem(hDlg, IDITEMTYPE);
			ShowWindow(IdItem, SW_SHOW);

			IdItem = GetDlgItem(hDlg, IDC_STATIC_VALUE);
			ShowWindow(IdItem, SW_SHOW);
			IdItem = GetDlgItem(hDlg, IDVALUECB);
			ShowWindow(IdItem, SW_SHOW);
			break;

		/*
		* OneValue Container
		*/
		case IDVALUE:
		{
			IdItem = GetDlgItem(hDlg, IDC_STATIC_ITEMTYPE);
			ShowWindow(IdItem, SW_SHOW);
			IdItem = GetDlgItem(hDlg, IDITEMTYPE);
			ShowWindow(IdItem, SW_SHOW);

			IdItem = GetDlgItem(hDlg, IDC_STATIC_VALUE);
			ShowWindow(IdItem, SW_SHOW);
			IdItem = GetDlgItem(hDlg, IDVALUE);
			ShowWindow(IdItem, SW_SHOW);
			SendDlgItemMessage(hDlg, IDVALUE, EM_REPLACESEL, 0, (LPARAM)(LPSTR)string);

			if((lstrcmp(szItemType, "TWTY_STR32") != 0) &&
				 (lstrcmp(szItemType, "TWTY_STR64") != 0) &&
				 (lstrcmp(szItemType, "TWTY_STR128") != 0) &&
				 (lstrcmp(szItemType, "TWTY_STR255") != 0))
			{
#ifdef WIN64
			  g_fnOldControl = (WNDPROC)SetWindowLongPtr(IdItem, GWLP_WNDPROC, 
										      (LPARAM)SubClassEditProc);
#else
			  g_fnOldControl = (WNDPROC)SetWindowLong(IdItem, GWL_WNDPROC, 
										      (LPARAM)SubClassEditProc);
#endif
				SetProp(IdItem, g_szSubClassEditBoxProc, g_fnOldControl);
				g_uiOldIdControl = iShowItem;
			}
		}
			break;

		/*
		* OneValue Container
		* Frame Type
		*/
		case IDLEFT:
		case IDTOP:
		case IDRIGHT:
		case IDBOTTOM:
			IdItem = GetDlgItem(hDlg, IDC_STATIC_ITEMTYPE);
			ShowWindow(IdItem, SW_SHOW);
			IdItem = GetDlgItem(hDlg, IDITEMTYPE);
			ShowWindow(IdItem, SW_SHOW);

			IdItem = GetDlgItem(hDlg, IDC_STATIC_LEFT);
			ShowWindow(IdItem, SW_SHOW);
			IdItem = GetDlgItem(hDlg, IDLEFT);
			ShowWindow(IdItem, SW_SHOW);
			SendDlgItemMessage(hDlg, IDLEFT, EM_REPLACESEL, 0, (LPARAM)(LPSTR)string);
#ifdef WIN64
		  g_fnOldControl = (WNDPROC)SetWindowLongPtr(IdItem, GWLP_WNDPROC, 
									      (LPARAM)SubClassEditProc);
#else
		  g_fnOldControl = (WNDPROC)SetWindowLong(IdItem, GWL_WNDPROC, 
									      (LPARAM)SubClassEditProc);
#endif
			SetProp(IdItem, g_szSubClassEditBoxProc, g_fnOldControl);

			IdItem = GetDlgItem(hDlg, IDC_STATIC_TOP);
			ShowWindow(IdItem, SW_SHOW);
			IdItem = GetDlgItem(hDlg, IDTOP);
			ShowWindow(IdItem, SW_SHOW);
			SendDlgItemMessage(hDlg, IDTOP, EM_REPLACESEL, 0, (LPARAM)(LPSTR)string);
#ifdef WIN64
		  g_fnOldControl = (WNDPROC)SetWindowLongPtr(IdItem, GWLP_WNDPROC, 
									      (LPARAM)SubClassEditProc);
#else
		  g_fnOldControl = (WNDPROC)SetWindowLong(IdItem, GWL_WNDPROC, 
									      (LPARAM)SubClassEditProc);
#endif
			SetProp(IdItem, g_szSubClassEditBoxProc, g_fnOldControl);

			IdItem = GetDlgItem(hDlg, IDC_STATIC_RIGHT);
			ShowWindow(IdItem, SW_SHOW);
			IdItem = GetDlgItem(hDlg, IDRIGHT);
			ShowWindow(IdItem, SW_SHOW);
			SendDlgItemMessage(hDlg, IDRIGHT, EM_REPLACESEL, 0, (LPARAM)(LPSTR)string);
#ifdef WIN64
		  g_fnOldControl = (WNDPROC)SetWindowLongPtr(IdItem, GWLP_WNDPROC, 
									      (LPARAM)SubClassEditProc);
#else
		  g_fnOldControl = (WNDPROC)SetWindowLong(IdItem, GWL_WNDPROC, 
									      (LPARAM)SubClassEditProc);
#endif
			SetProp(IdItem, g_szSubClassEditBoxProc, g_fnOldControl);

			IdItem = GetDlgItem(hDlg, IDC_STATIC_BOTTOM);
			ShowWindow(IdItem, SW_SHOW);
			IdItem = GetDlgItem(hDlg, IDBOTTOM);
			ShowWindow(IdItem, SW_SHOW);
			SendDlgItemMessage(hDlg, IDBOTTOM, EM_REPLACESEL, 0, (LPARAM)(LPSTR)string);
#ifdef WIN64
		  g_fnOldControl = (WNDPROC)SetWindowLongPtr(IdItem, GWLP_WNDPROC, 
									      (LPARAM)SubClassEditProc);
#else
		  g_fnOldControl = (WNDPROC)SetWindowLong(IdItem, GWL_WNDPROC, 
									      (LPARAM)SubClassEditProc);
#endif
			SetProp(IdItem, g_szSubClassEditBoxProc, g_fnOldControl);
			g_uiOldIdControl = iShowItem;
			break;

		/*
		* Range Container
		*/
		case IDMINVALUE:
		case IDMAXVALUE:
		case IDSTEPSIZE:
		case IDDEFAULTVALUE:
		case IDCURRENTVALUE:
			IdItem = GetDlgItem(hDlg, IDC_STATIC_ITEMTYPE);
			ShowWindow(IdItem, SW_SHOW);
			IdItem = GetDlgItem(hDlg, IDITEMTYPE);
			ShowWindow(IdItem, SW_SHOW);

			IdItem = GetDlgItem(hDlg, IDC_STATIC_MINVALUE);
			ShowWindow(IdItem, SW_SHOW);
			IdItem = GetDlgItem(hDlg, IDMINVALUE);
			ShowWindow(IdItem, SW_SHOW);
			SendDlgItemMessage(hDlg, IDMINVALUE, EM_REPLACESEL, 0, (LPARAM)(LPSTR)string);
#ifdef WIN64
		  g_fnOldControl = (WNDPROC)SetWindowLongPtr(IdItem, GWLP_WNDPROC, 
									      (LPARAM)SubClassEditProc);
#else
		  g_fnOldControl = (WNDPROC)SetWindowLong(IdItem, GWL_WNDPROC, 
									      (LPARAM)SubClassEditProc);
#endif
			SetProp(IdItem, g_szSubClassEditBoxProc, g_fnOldControl);

			IdItem = GetDlgItem(hDlg, IDC_STATIC_MAXVALUE);
			ShowWindow(IdItem, SW_SHOW);
			IdItem = GetDlgItem(hDlg, IDMAXVALUE);
			ShowWindow(IdItem, SW_SHOW);
			SendDlgItemMessage(hDlg, IDMAXVALUE, EM_REPLACESEL, 0, (LPARAM)(LPSTR)string);
#ifdef WIN64
		  g_fnOldControl = (WNDPROC)SetWindowLongPtr(IdItem, GWLP_WNDPROC, 
									      (LPARAM)SubClassEditProc);
#else
		  g_fnOldControl = (WNDPROC)SetWindowLong(IdItem, GWL_WNDPROC, 
									      (LPARAM)SubClassEditProc);
#endif
			SetProp(IdItem, g_szSubClassEditBoxProc, g_fnOldControl);

			IdItem = GetDlgItem(hDlg, IDC_STATIC_STEPSIZE);
			ShowWindow(IdItem, SW_SHOW);
			IdItem = GetDlgItem(hDlg, IDSTEPSIZE);
			ShowWindow(IdItem, SW_SHOW);
			SendDlgItemMessage(hDlg, IDSTEPSIZE, EM_REPLACESEL, 0, (LPARAM)(LPSTR)string);
#ifdef WIN64
		  g_fnOldControl = (WNDPROC)SetWindowLongPtr(IdItem, GWLP_WNDPROC, 
									      (LPARAM)SubClassEditProc);
#else
		  g_fnOldControl = (WNDPROC)SetWindowLong(IdItem, GWL_WNDPROC, 
									      (LPARAM)SubClassEditProc);
#endif
			SetProp(IdItem, g_szSubClassEditBoxProc, g_fnOldControl);

			IdItem = GetDlgItem(hDlg, IDC_STATIC_DEFAULTVALUE);
			ShowWindow(IdItem, SW_SHOW);
			IdItem = GetDlgItem(hDlg, IDDEFAULTVALUE);
			ShowWindow(IdItem, SW_SHOW);
			SendDlgItemMessage(hDlg, IDDEFAULTVALUE, EM_REPLACESEL, 0, (LPARAM)(LPSTR)string);
#ifdef WIN64
		  g_fnOldControl = (WNDPROC)SetWindowLongPtr(IdItem, GWLP_WNDPROC, 
									      (LPARAM)SubClassEditProc);
#else
		  g_fnOldControl = (WNDPROC)SetWindowLong(IdItem, GWL_WNDPROC, 
									      (LPARAM)SubClassEditProc);
#endif
			SetProp(IdItem, g_szSubClassEditBoxProc, g_fnOldControl);

			IdItem = GetDlgItem(hDlg, IDC_STATIC_CURRENTVALUE);
			ShowWindow(IdItem, SW_SHOW);
			IdItem = GetDlgItem(hDlg, IDCURRENTVALUE);
			ShowWindow(IdItem, SW_SHOW);
			SendDlgItemMessage(hDlg, IDCURRENTVALUE, EM_REPLACESEL, 0, (LPARAM)(LPSTR)string);
#ifdef WIN64
		  g_fnOldControl = (WNDPROC)SetWindowLongPtr(IdItem, GWLP_WNDPROC, 
									      (LPARAM)SubClassEditProc);
#else
		  g_fnOldControl = (WNDPROC)SetWindowLong(IdItem, GWL_WNDPROC, 
									      (LPARAM)SubClassEditProc);
#endif
			SetProp(IdItem, g_szSubClassEditBoxProc, g_fnOldControl);
			g_uiOldIdControl = iShowItem;
			break;

		/*
		* Array container
		*/
		case IDITEMLISTARRAY:
			IdItem = GetDlgItem(hDlg, IDC_STATIC_ITEMTYPE);
			ShowWindow(IdItem, SW_SHOW);
			IdItem = GetDlgItem(hDlg, IDITEMTYPE);
			ShowWindow(IdItem, SW_SHOW);

			IdItem = GetDlgItem(hDlg, IDC_STATIC_NUMITEMS);
			ShowWindow(IdItem, SW_SHOW);
			IdItem = GetDlgItem(hDlg, IDNUMITEMS);
			ShowWindow(IdItem, SW_SHOW);
			SendDlgItemMessage(hDlg, IDNUMITEMS, EM_REPLACESEL, 0, (LPARAM)(LPSTR)string);
			for(i = 0; i < MAX_TYPES; i++)
			{
				if(Types[i].ItemId == TWTY_UINT32)
				{
					GetCurrentItemType(hDlg, &Types[i]);
				}
			}
#ifdef WIN64
		  g_fnOldControl = (WNDPROC)SetWindowLongPtr(IdItem, GWLP_WNDPROC, 
									      (LPARAM)SubClassEditProc);
#else
		  g_fnOldControl = (WNDPROC)SetWindowLong(IdItem, GWL_WNDPROC, 
									      (LPARAM)SubClassEditProc);
#endif
			SetProp(IdItem, g_szSubClassEditBoxProc, g_fnOldControl);

			IdItem = GetDlgItem(hDlg, IDC_STATIC_ITEMLISTARRAY);
			ShowWindow(IdItem, SW_SHOW);
			IdItem = GetDlgItem(hDlg, IDITEMLISTARRAY);
			ShowWindow(IdItem, SW_SHOW);
			g_uiOldIdControl = iShowItem;
			break;

		/*
		* Enumeration Container
		*/
		case IDCURRENTINDEX:
		case IDDEFAULTINDEX:
			IdItem = GetDlgItem(hDlg, IDC_STATIC_ITEMTYPE);
			ShowWindow(IdItem, SW_SHOW);
			IdItem = GetDlgItem(hDlg, IDITEMTYPE);
			ShowWindow(IdItem, SW_SHOW);

			IdItem = GetDlgItem(hDlg, IDC_STATIC_NUMITEMS);
			ShowWindow(IdItem, SW_SHOW);
			IdItem = GetDlgItem(hDlg, IDNUMITEMS);
			ShowWindow(IdItem, SW_SHOW);
			SendDlgItemMessage(hDlg, IDNUMITEMS, EM_REPLACESEL, 0, (LPARAM)(LPSTR)string);
			for(i = 0; i < MAX_TYPES; i++)
			{
				if(Types[i].ItemId == TWTY_UINT32)
				{
					GetCurrentItemType(hDlg, &Types[i]);
				}
			}
#ifdef WIN64
		  g_fnOldControl = (WNDPROC)SetWindowLongPtr(IdItem, GWLP_WNDPROC, 
									      (LPARAM)SubClassEditProc);
#else
		  g_fnOldControl = (WNDPROC)SetWindowLong(IdItem, GWL_WNDPROC, 
									      (LPARAM)SubClassEditProc);
#endif
			SetProp(IdItem, g_szSubClassEditBoxProc, g_fnOldControl);

			IdItem = GetDlgItem(hDlg, IDC_STATIC_CURRENTINDEX);
			ShowWindow(IdItem, SW_SHOW);
			IdItem = GetDlgItem(hDlg, IDCURRENTINDEX);
			ShowWindow(IdItem, SW_SHOW);
			SendDlgItemMessage(hDlg, IDCURRENTINDEX, EM_REPLACESEL, 0, (LPARAM)(LPSTR)string);
#ifdef WIN64
		  g_fnOldControl = (WNDPROC)SetWindowLongPtr(IdItem, GWLP_WNDPROC, 
									      (LPARAM)SubClassEditProc);
#else
		  g_fnOldControl = (WNDPROC)SetWindowLong(IdItem, GWL_WNDPROC, 
									      (LPARAM)SubClassEditProc);
#endif
			SetProp(IdItem, g_szSubClassEditBoxProc, g_fnOldControl);

			IdItem = GetDlgItem(hDlg, IDC_STATIC_DEFAULTINDEX);
			ShowWindow(IdItem, SW_SHOW);
			IdItem = GetDlgItem(hDlg, IDDEFAULTINDEX);
			ShowWindow(IdItem, SW_SHOW);
			SendDlgItemMessage(hDlg, IDDEFAULTINDEX, EM_REPLACESEL, 0, (LPARAM)(LPSTR)string);
#ifdef WIN64
		  g_fnOldControl = (WNDPROC)SetWindowLongPtr(IdItem, GWLP_WNDPROC, 
									      (LPARAM)SubClassEditProc);
#else
		  g_fnOldControl = (WNDPROC)SetWindowLong(IdItem, GWL_WNDPROC, 
									      (LPARAM)SubClassEditProc);
#endif
			SetProp(IdItem, g_szSubClassEditBoxProc, g_fnOldControl);

			IdItem = GetDlgItem(hDlg, IDC_STATIC_ITEMLISTENUM);
			ShowWindow(IdItem, SW_SHOW);
			IdItem = GetDlgItem(hDlg, IDITEMLISTENUM);
			ShowWindow(IdItem, SW_SHOW);
			g_uiOldIdControl = iShowItem;
			break;
		/* 
		* Message Get
		*/
		case IDEDIT:
			IdItem = GetDlgItem(hDlg, IDEDIT);
			ShowWindow(IdItem, SW_SHOW);
			break;

		default:
			break;
	}
	return;
}

/*
* Function: SetConType
* Author: Nancy Létourneau / J.F.L. Peripheral Solutions Inc. / TWAIN Working Group
* Input:
*		pEntryConType - pointer to the table entry of Container types
*		hDlg - handle to the dialog window
*		IdItem - handle to the dialog item 
* Output: none
* Comments:
*		Display the good dialog item for the current Container Type.
*/
static void SetConType(pTABLEENTRY pEntryConType, HWND hDlg, HWND IdItem)
{
	TW_UINT16 i = 0;
	TW_STR64 text;
	pTABLECAP pCapEntry = NULL;

	memset(text, 0, sizeof(TW_STR64));
	ASSERT(pEntryConType);

	g_bNewDisplay = TRUE;
	/*
	* Get the current Data Type
	*/
	GetDlgItemText(hDlg, IDDAT, text, sizeof(text));

	if(lstrcmp(text, "DAT_CAPABILITY") == 0)
	{
		/*
		* Get the current capability
		*/
		GetDlgItemText(hDlg, IDCAP, text, sizeof(text));

		for (i = 0; i < MAX_CAP; i++)
		{
			if(lstrcmp(text, Caps[i].EntryHeader.pszItemName) == 0)
			{
				pCapEntry = &Caps[i];
				break;
			}
		}

		/*
		* Initialize the ItemType combo Box with the good value for each container
		*/
		if(pEntryConType->ItemId == TWON_RANGE)
		{
			InitTwainCombo(hDlg, IDITEMTYPE, TypesRange, MAX_TYPESRANGE);		
		}
		else
		{
			InitTwainCombo(hDlg, IDITEMTYPE, Types, MAX_TYPES);
		}

		/*
		* Place the good ItemType in the comboBox IDITEMTYPE
		*/
		for(i = 0; i < MAX_TYPES; i++)
		{
			if(pCapEntry->ExpectedType == Types[i].ItemId)
			{
				SendDlgItemMessage(hDlg, IDITEMTYPE, CB_SELECTSTRING, -1, 
													(LPARAM)Types[i].pszItemName);

				
				GetCurrentItemType(hDlg, &Types[i]);
				break;
			}
		}

		/*
		* Contition to chow the good Dialog Control for each container
		*/
		switch(pEntryConType->ItemId)
		{
			/* 
			* OneValue Container
			*/
			case TWCC_BADPROTOCOL:
			case TWON_ONEVALUE:
				/*
				* Frame Type
				*/
				if(pCapEntry->EntryHeader.ItemId == ICAP_FRAMES)
				{
					ShowDialogItem(hDlg, IdItem, IDLEFT);
					g_bFrameDlg = FALSE;
				}
				else
				{
					/* 
					* Defined type
					*/
					if(pCapEntry->pItemTable != NULL)
					{
						ShowDialogItem(hDlg, IdItem, IDVALUECB);
						InitTwainCombo(hDlg, IDVALUECB, pCapEntry->pItemTable, pCapEntry->ItemTableSize);
					}
					else
					{
						ShowDialogItem(hDlg, IdItem, IDVALUE);
					}
				}
				break;

			/*
			* Enumeration Container
			*/
			case TWON_ENUMERATION:
				ShowDialogItem(hDlg, IdItem, IDCURRENTINDEX);
				break;

			/*
			* Range Container
			*/
			case TWON_RANGE:
				ShowDialogItem(hDlg, IdItem, IDMINVALUE);
				break;

			/*
			* array container
			*/
			case TWON_ARRAY:
				ShowDialogItem(hDlg, IdItem, IDITEMLISTARRAY);
				break;

			default:
				break;
		}
	}
	else
	{
		ShowDialogItem(hDlg, IdItem, IDEDIT);
	}
	return;
}	

/*
* FUNCTION: AboutDlgProc
* ARGS:    
*		hWnd        handle to main app window
*		iMessage    windows message
*		wParam      1st windows parm (message specific information)
*		lParam      2nd windows parm
* RETURNS: 
*		TRUE, returned to windows to signal success
* NOTES:   
*		Processes messages for "About" dialog box.    Wait for user to
*		click on "OKAY" button, then close the dialog box.
*		1). messages serviced:
*				- WM_INITDIALOG     initialize dialog box
*				- WM_COMMAND        Input received
*				- TW_APP_ABOUT_OKAY     OKAY button in about dialog box
*				- ITWANCEL              CLOSE off the dialog box system menu
*/
INT_PTR FAR PASCAL AboutDlgProc (HWND hDlg,
                              UINT message,
                              WPARAM wParam,
                              LPARAM lParam)
{
	TW_UINT16 wmId = 0;
	TW_UINT16 wmEvent = 0;

#if defined(WIN32) || defined(WIN64)
		wmId = LOWORD(wParam);
		wmEvent = HIWORD (wParam);
	#else
		wmId = wParam;
		wmEvent = HIWORD (lParam);
	#endif

	ASSERT(hDlg);

	switch (message)
	{
		/*
		* message: initialize dialog box
		*/
		case WM_INITDIALOG:
			return (TRUE);
    /*
		* message: received a command
		*/
		case WM_COMMAND:
			if ((wmId == TW_APP_ABOUT_OKAY) ||(wmId == IDCANCEL))   
			{   
				/* 
				* Exits the dialog box
				*/
				EndDialog(hDlg, TRUE);
				return (TRUE);
			}
			break;

		default:
			break;
	}

	/*
	* idn't process a message
	*/
	return (FALSE);            
} 

/*
* Function: UpdateTripletMenus
* Author: TWAIN Working Group
* Input: 
*		hDlg - Handle on the Dialog Box
*		Updated -
* Output:	none
* Comment:
*
*/
void UpdateTripletMenus (HWND hDlg, TW_UINT16 Updated)
{
	TW_STR64    text;
	TW_STR64    text2;
	TW_INT16    dat = 0;
	TW_INT16    index = 0;
	TW_INT16		i = 0;
	pTABLEENTRY pEntry = NULL;
	int iCurrSel = 0;

	memset(text, 0, sizeof(TW_STR64));
	memset(text2, 0, sizeof(TW_STR64));
	ASSERT(hDlg);

	strcpy_s(text, sizeof(text), "");

	switch (Updated)
	{
		case IDDEST:
			/*
			* Get the current Dest
			*/
			GetDlgItemText(hDlg, IDDEST, text, sizeof(text));

			/*
			* Source Manager
			*/
			if (strcmp(text, Destination[0].pszItemName) == 0)
			{
				InitTwainCombo(hDlg, IDDG, DG_SourceManager, MAX_DGSM);
			}
			/*
			* Source
			*/
			else
			{
				InitTwainCombo(hDlg, IDDG, DG_Source, MAX_DGS);
        SendDlgItemMessage(hDlg, IDDG, CB_SELECTSTRING, -1, (LPARAM)DG_Source[0].pszItemName);
			}

		case IDDG:
			GetDlgItemText(hDlg, IDDEST, text, sizeof(text));

			/*
			* Source Manager
			*/
			if (strcmp(text, Destination[0].pszItemName) == 0)
			{
				InitTwainCombo(hDlg, IDDAT, DSM_DGControl, MAX_DSMDAT);
			}
			/*
			* Source
			*/
			else
			{
				GetDlgItemText(hDlg, IDDG, text2, sizeof(text2));
				/*
				* DG_CONTROL
				*/
				if (strcmp(text2, DG_Source[0].pszItemName) == 0)
				{
					InitTwainCombo(hDlg, IDDAT, DS_DGControl, MAX_DSCONTROL);                    
				}
				/*
				* DG_IMAGE
				*/
				else if(strcmp(text2, DG_Source[1].pszItemName) == 0)
				{
					InitTwainCombo(hDlg, IDDAT, DS_DGImage, MAX_DSIMAGE);                    
				}
				/*
				*	DG_AUDIO
				*/
				else
				{
					InitTwainCombo(hDlg, IDDAT, DS_DGAudio, MAX_DSAUDIO);
				}
			}

		case IDDAT:
			/*
			* Get the current DAT
			*/
			GetDlgItemText(hDlg, IDDAT, text, sizeof(text));

			for (index = 0; index < MAX_DAT; index++)
			{
				if(strcmp(text, DataType[index].pszItemName) == 0)
				{
					InitTwainCombo(hDlg, IDMSG, MsgTable[index].Table, 
													MsgTable[index].Max);	
					break;
				}
			}
			break;

		default:
			break;
	}

	/*
	* Enable or Disable Capability windows if/if not DAT_CAPABILITY
	*/
	EnableWindow(GetDlgItem(hDlg,IDCAPTEXT), DataType[index].ItemId==DAT_CAPABILITY);
	EnableWindow(GetDlgItem(hDlg,IDCAP), DataType[index].ItemId==DAT_CAPABILITY);
	EnableWindow(GetDlgItem(hDlg,IDTYPETEXT), DataType[index].ItemId==DAT_CAPABILITY);
	EnableWindow(GetDlgItem(hDlg,IDTYPE), DataType[index].ItemId==DAT_CAPABILITY);

	/*
	* update the edit box
	*/
	SendDlgItemMessage(hDlg, IDEDIT, WM_SETREDRAW, 0, 0L);
	SendDlgItemMessage(hDlg,IDEDIT,WM_SETTEXT,0,(LPARAM)(LPSTR)"");

	/*
	*	Get the current Data Group that has been selected
	*/
	iCurrSel = (int)SendDlgItemMessage(hDlg, (dat==DAT_CAPABILITY)? IDTYPE:IDDAT,
																CB_GETCURSEL, 0, 0);
	if(iCurrSel >= 0)
	{
		pEntry = (pTABLEENTRY)SendDlgItemMessage(hDlg, (dat==DAT_CAPABILITY)? IDTYPE:IDDAT,
																			CB_GETITEMDATA, iCurrSel, 0);

		/*
		*	This entry should be valid
		*/
		ASSERT(pEntry);																			 
	}

	/*
	* Update Data Structure Area
	*/

	for (i = 0; i < MAX_DATASTRUCTURE; i++)
	{
		if(lstrcmp(pEntry->pszItemName, DataStructure[i].pszDataType) == 0)
		{
			if(lstrcmp(DataStructure[i].pszDataType, "DAT_FILESYSTEM") == 0)
			{
				TW_STR64 TempText;
				GetDlgItemText(hDlg, IDMSG, TempText, sizeof(TempText));
				if(lstrcmp(TempText, "MSG_DELETE") == 0)
				{}
				else if(lstrcmp(TempText, "MSG_GETINFO") == 0)
				{
					i++;
				}
				else
				{
					i = i+2;
				}
			}
			InitTwainEdit(hDlg, IDEDIT, DataStructure[i].Max, 
										DataStructure[i].gszDataStructure);
			break;
		}
	}
	SendDlgItemMessage(hDlg, IDEDIT, WM_SETREDRAW, 1, 0L);
	InvalidateRect(GetDlgItem(hDlg, IDEDIT), NULL, 1);

	return;
}

/*
* Function: RemoveEquals -- 
* Author: TWAIN Working Group
* Input:
*		string - 
*		count - 
* Output: none
* Comments:
*		Receives the formatted text from the edit control and returns
* the string to the right of the equals sign.
*/
void RemoveEquals(char string[], LONG count)
{
	char *pval = NULL;
	int x = 0;

	string[count] = '\0';

	for (pval=string; *pval && *pval != '='; pval++);
	{
		pval++;
	}

	lstrcpyn(string, pval, lstrlen(pval)+1);

	/*
	* remove garbage placed at end of edit control line starts with \x 
	*/
	for (pval=string; *pval && isgraph(*pval) && *pval != '\x94'; pval++,x++);

	string[x] = '\0';
	return;
}

/*
* Function: ControlMsg -- 
* Author: TWAIN Working Group
* Input:
*		dat - The current data type
*		msg - the current message
*		hDlg - Handle to dialog box.
* Output:
*		TRUE is successful
* Comments:
*		Handler for DG_CONTROL messages
*/
BOOL ControlMsg(TW_UINT16 dat,TW_UINT16 msg,HWND hDlg)
{
	TW_STR64 string1;
	TW_STR64 string2;
	LONG count1 = 0, count2 = 0;
	TW_SETUPFILEXFER filesetup;
	TW_SETUPMEMXFER memsetup;
	char buffer[1000];
	TW_STATUS stat;
	TW_CUSTOMDSDATA CustDSData;
	TW_USERINTERFACE ui;
	TW_PENDINGXFERS pending;
	TW_IDENTITY Identity;
	TW_FILESYSTEM fileSystem;
	TW_PASSTHRU passthru;

	char *buf = NULL;
	TW_UINT16 ret = TWRC_FAILURE;
	pTABLECAP pTableCap = NULL;
	int iCurrSel = 0;

	memset(string1, 0, sizeof(TW_STR64));
	memset(string2, 0, sizeof(TW_STR64));
	memset(&filesetup, 0, sizeof(TW_SETUPFILEXFER));
	memset(&memsetup, 0, sizeof(TW_SETUPMEMXFER));
	memset(buffer, 0, sizeof(buffer));
	memset(&stat, 0, sizeof(TW_STATUS));
	memset(&CustDSData, 0, sizeof(TW_CUSTOMDSDATA));
	memset(&ui, 0, sizeof(TW_USERINTERFACE));
	memset(&pending, 0, sizeof(TW_PENDINGXFERS));
	memset(&Identity, 0, sizeof(TW_IDENTITY));
	memset(&fileSystem, 0, sizeof(TW_FILESYSTEM));
	memset(&passthru, 0, sizeof(TW_PASSTHRU));

	ASSERT(hDlg);

	switch (dat)
	{
		/*
		* Status
		*/
		case DAT_STATUS:
			{
				TW_UINT16 i = 0;

				ret = Status(&stat);

				/*
				* clear the edit control
				*/
				SetWindowText(GetDlgItem(hDlg,IDEDIT)," ");

				for(i= 0; i < MAX_CONDITIONCODE; i++)
				{
					if(ConditionCode[i].ItemId == stat.ConditionCode)
					{
						sprintf_s(buffer, sizeof(buffer), "ConditionCode = %s\r\n",ConditionCode[i].pszItemName);
						break;
					}
				}
				SendDlgItemMessage(hDlg,IDEDIT,EM_REPLACESEL,0,(LPARAM)(LPSTR)buffer);

				sprintf_s(buffer, sizeof(buffer), "Reserved = %ld\r\n",stat.Reserved);
				SendDlgItemMessage(hDlg,IDEDIT,EM_REPLACESEL,1,(LPARAM)(LPSTR)buffer);
			}
			break;

		/* 
		* Capability
		*/
		case DAT_CAPABILITY:
			/*
			*	Get the current Data Capability that has been selected
			*/
			iCurrSel = (int)SendDlgItemMessage(hDlg, IDCAP, CB_GETCURSEL, 0, 0);
			if(iCurrSel >= 0)
			{
				pTABLEENTRY pTmpCap = NULL;
				TW_UINT16 i = 0;
				/*
				*	Get the ID from the table entry
				*/
				pTmpCap = (pTABLEENTRY)SendDlgItemMessage(hDlg, IDCAP, CB_GETITEMDATA, 
										iCurrSel, 0);

				/*
				*	This entry should be valid
				*/
				ASSERT(pTmpCap);
				for(i = 0; i < MAX_CAP; i++)
				{
					if(pTmpCap->ItemId == Caps[i].EntryHeader.ItemId)
					{
						pTableCap = &Caps[i];
						break;
					}
				}

				if (lstrcmp(pTableCap->EntryHeader.pszItemName,"CAP_SUPPORTEDCAPS")==0)
				{
					CapSupportedCaps = TRUE;
				}

				if (msg == MSG_QUERYSUPPORT)
				{
					ret = QuerySupportMessage(pTableCap, hDlg, IDEDIT); 
				}
				else if (msg != MSG_SET)
				{
					ret = GetCapabilitySpecial(pTableCap, msg, hDlg, IDEDIT); 
				}
				else
				{
					pTABLEENTRY pConType = NULL;
					TW_UINT16 i = 0;
					TW_STR32 text;

					memset(text, 0, sizeof(TW_STR32));

					GetDlgItemText(hDlg, IDTYPE, text, sizeof(text));
					for(i = 0; i < MAX_CONTYPE; i++)
					{
						if(lstrcmp(text, ConType[i].pszItemName) == 0)
						{
							pConType = &ConType[i];
							break;
						}
					}
					ret = SetCapabilitySpecial(pTableCap, msg, hDlg, IDEDIT, pConType);
				}

				/*
				* patch for a problem in ConvertAttributeToText -- see notes there
				*/
				CapSupportedCaps = FALSE;
			}
			break;


		/*
		* CustomDSData
		*/
		case DAT_CUSTOMDSDATA:

			SetWindowText(GetDlgItem(hDlg,IDEDIT)," ");
			ret = CustomDSData(&CustDSData, msg);

			sprintf_s(buffer, sizeof(buffer), "InfoLength=%ld\r\n",CustDSData.InfoLength);
			SendDlgItemMessage(hDlg,IDEDIT,EM_REPLACESEL,0,(LPARAM)(LPSTR)buffer);
			break;


		/*
		* Event
		*/
		case DAT_EVENT:
			ShowRC_CC(hDlg,0,0,0,"Not supported from Specials menu","DAT_EVENT");
			ret = TWRC_FAILURE;
			break;

		/*
		*	Itentity
		*/
		case DAT_IDENTITY:
			SetWindowText(GetDlgItem(hDlg,IDEDIT)," ");

			if((msg == MSG_GETFIRST) || 
				 (msg == MSG_GETNEXT) || 
				 (msg == MSG_GETDEFAULT))
			{
				int i = 0;
				BOOL bFindGroup = FALSE;

				Identity = appID;
				ret = TWRC_SUCCESS;

				sprintf_s(buffer, sizeof(buffer), "Id=%ld\r\n",Identity.Id);
				SendDlgItemMessage(hDlg,IDEDIT,EM_REPLACESEL,0,(LPARAM)(LPSTR)buffer);

				sprintf_s(buffer, sizeof(buffer), "Version.MajorNum=%d\r\n",Identity.Version.MajorNum);
				SendDlgItemMessage(hDlg,IDEDIT,EM_REPLACESEL,0,(LPARAM)(LPSTR)buffer);           

				sprintf_s(buffer, sizeof(buffer), "Version.MinorNum=%d\r\n",Identity.Version.MinorNum);
				SendDlgItemMessage(hDlg,IDEDIT,EM_REPLACESEL,0,(LPARAM)(LPSTR)buffer);           

				for(i = 0; i < MAX_VERLANGUAGETYPE; i++)
				{
					if(VerLanguageType[i].ItemId == Identity.Version.Language)
					{
						sprintf_s(buffer, sizeof(buffer), "Version.Language=%s\r\n",VerLanguageType[i].pszItemName);
						SendDlgItemMessage(hDlg,IDEDIT,EM_REPLACESEL,0,(LPARAM)(LPSTR)buffer);           
						break;
					}
				}


				for(i = 0; i < MAX_VERCOUNTRYTYPE; i++)
				{
					if(VerCountryType[i].ItemId == Identity.Version.Country)
					{
						sprintf_s(buffer, sizeof(buffer), "Version.Country=%s\r\n",VerCountryType[i].pszItemName);
						SendDlgItemMessage(hDlg,IDEDIT,EM_REPLACESEL,0,(LPARAM)(LPSTR)buffer);           
						break;
					}
				}

				sprintf_s(buffer, sizeof(buffer), "Version.Info=%s\r\n",Identity.Version.Info);
				SendDlgItemMessage(hDlg,IDEDIT,EM_REPLACESEL,0,(LPARAM)(LPSTR)buffer);           

				sprintf_s(buffer, sizeof(buffer), "ProtocolMajor=%d\r\n",Identity.ProtocolMajor);
				SendDlgItemMessage(hDlg,IDEDIT,EM_REPLACESEL,0,(LPARAM)(LPSTR)buffer);           

				sprintf_s(buffer, sizeof(buffer), "ProtocolMinor=%d\r\n",Identity.ProtocolMinor);
				SendDlgItemMessage(hDlg,IDEDIT,EM_REPLACESEL,0,(LPARAM)(LPSTR)buffer);           

				for(i = 0; i < MAX_DG; i++)
				{
					if(DataGroup[i].ItemId == Identity.SupportedGroups)
					{
						bFindGroup = TRUE;
						sprintf_s(buffer, sizeof(buffer), "SupportedGroups=%s\r\n",DataGroup[i].pszItemName);
						SendDlgItemMessage(hDlg,IDEDIT,EM_REPLACESEL,0,(LPARAM)(LPSTR)buffer);           
						break;
					}
				}

				if(bFindGroup == FALSE)
				{
					if(Identity.SupportedGroups == 3)
					{
						sprintf_s(buffer, sizeof(buffer), "SupportedGroups=%s\r\n","DG_IMAGE | DG_CONTROL");
						SendDlgItemMessage(hDlg,IDEDIT,EM_REPLACESEL,0,(LPARAM)(LPSTR)buffer);           
					}
				}

				sprintf_s(buffer, sizeof(buffer), "Manufacturer=%s\r\n",Identity.Manufacturer);
				SendDlgItemMessage(hDlg,IDEDIT,EM_REPLACESEL,0,(LPARAM)(LPSTR)buffer);           

				sprintf_s(buffer, sizeof(buffer), "ProductFamily=%s\r\n",Identity.ProductFamily);
				SendDlgItemMessage(hDlg,IDEDIT,EM_REPLACESEL,0,(LPARAM)(LPSTR)buffer);           

				sprintf_s(buffer, sizeof(buffer), "ProductName=%s\r\n",Identity.ProductName);
				SendDlgItemMessage(hDlg,IDEDIT,EM_REPLACESEL,0,(LPARAM)(LPSTR)buffer);           
			}
			else
			{
				pTABLEENTRY pEntryMsg = NULL;
				int i = 0;

				for(i = 0; i < MAX_IDENTITY; i++)
				{
					if(MSG_Identity[i].ItemId == msg)
					{
						pEntryMsg = &MSG_Identity[i];
						break;
					}
				}

				sprintf_s(buffer, sizeof(buffer),  "DAT_IDENTITY/%s", pEntryMsg->pszItemName);
				ShowRC_CC(g_hMainDlg,0,0,0,"Not supported from Specials menu",buffer);
				ret = TWRC_FAILURE;
			}
			break;


		/*
		* PendingXfers
		*/
		case DAT_PENDINGXFERS:
			SetWindowText(GetDlgItem(hDlg,IDEDIT)," ");

			ret = PendingXfers(&pending,msg);

			sprintf_s(buffer, sizeof(buffer), "Count=%ld\r\n",pending.Count);
			SendDlgItemMessage(hDlg,IDEDIT,EM_REPLACESEL,0,(LPARAM)(LPSTR)buffer);
			sprintf_s(buffer, sizeof(buffer), "EOJ=%ld\r\n",pending.EOJ);
			SendDlgItemMessage(hDlg,IDEDIT,EM_REPLACESEL,0,(LPARAM)(LPSTR)buffer);           
			break;

		/*
		* SetupFileXfer
		*/
		case DAT_SETUPFILEXFER:
			{
				pTABLECAP pCapEntry = NULL;
				TW_UINT16 i = 0;
				TW_STR64 buf;

				memset(buf, 0, sizeof(TW_STR64));
				
				if(msg == MSG_SET)
				{
          *((LPWORD)string1) = sizeof(string1);
					count1 = (int)SendDlgItemMessage(hDlg,IDEDIT,EM_GETLINE,0,(LPARAM)(pTW_STR255)string1);

          *((LPWORD)string2) = sizeof(string2);
					count2 = (int)SendDlgItemMessage(hDlg,IDEDIT,EM_GETLINE,1,(LPARAM)(pTW_STR255)string2);

					RemoveEquals(string1, count1);
					RemoveEquals(string2, count2);

					ret = SetupFileXfer(string1,string2,&filesetup,msg);
				}
				else
				{
					SetWindowText(GetDlgItem(hDlg,IDEDIT)," ");

					ret = SetupFileXfer(string1,string2,&filesetup,msg);

					sprintf_s(buffer, sizeof(buffer), "FileName=%s\r\n",filesetup.FileName);
					SendDlgItemMessage(hDlg,IDEDIT,EM_REPLACESEL,0,(LPARAM)(LPSTR)buffer);

					/*
					* Find the Capability inside the Table entry.
					*/
					for (i = 0; i < MAX_CAP; i++)
					{
						if(ICAP_IMAGEFILEFORMAT == Caps[i].EntryHeader.ItemId)
						{
							pCapEntry = &Caps[i];
							break;
						}
					}	

					/*
					* Find the associate type
					*/
					for (i = 0; i < pCapEntry->ItemTableSize; i++)
					{
						if(filesetup.Format == pCapEntry->pItemTable[i].ItemId)
						{
							strcpy_s(buf, sizeof(buf), pCapEntry->pItemTable[i].pszItemName);
							break;
						}
					}

					lstrcpyn(buffer,"Format=",sizeof(buffer));
					lstrcat(buffer,buf);
					lstrcat(buffer,"\r\n");
					SendDlgItemMessage(hDlg,IDEDIT,EM_REPLACESEL,0,(LPARAM)(LPSTR)buffer);

					sprintf_s(buffer, sizeof(buffer), "VRefNum=%ld\r\n",filesetup.VRefNum);
					SendDlgItemMessage(hDlg,IDEDIT,EM_REPLACESEL,0,(LPARAM)(LPSTR)buffer);
				}
			}
			break;

		/*
		* SetupMemXfer
		*/
		case DAT_SETUPMEMXFER:
			SetWindowText(GetDlgItem(hDlg,IDEDIT)," ");

			ret = SetupMemXfer(&memsetup);

			sprintf_s(buffer, sizeof(buffer), "MinBufSize=%ld\r\n",memsetup.MinBufSize);
			SendDlgItemMessage(hDlg,IDEDIT,EM_REPLACESEL,0,(LPARAM)(LPSTR)buffer);

			sprintf_s(buffer, sizeof(buffer), "MaxBufSize=%ld\r\n",memsetup.MaxBufSize);
			SendDlgItemMessage(hDlg,IDEDIT,EM_REPLACESEL,1,(LPARAM)(LPSTR)buffer);

			sprintf_s(buffer, sizeof(buffer), "Preferred=%ld\r\n",memsetup.Preferred);
			SendDlgItemMessage(hDlg,IDEDIT,EM_REPLACESEL,2,(LPARAM)(LPSTR)buffer);
			break;

		/*
		* UserInterface
		*/
		case DAT_USERINTERFACE:
			ASSERT(IsWindow(hDlg));
			ASSERT(IsWindow(GetDlgItem(hDlg, IDEDIT)));
			ASSERT(count1 = (int)SendDlgItemMessage(hDlg, IDEDIT, EM_GETLINECOUNT, 0, 0));
			{
				int nLineIndex = 0;
        *((LPWORD)string1) = sizeof(string1);
				count1 = (int)SendDlgItemMessage(hDlg, IDEDIT, EM_GETLINE, nLineIndex, (LPARAM)(LPSTR)string1);
				count1 = (int)SendDlgItemMessage(hDlg, IDEDIT, EM_GETFIRSTVISIBLELINE, nLineIndex, (LPARAM)(LPSTR)string1);
				count1 = GetWindowText(GetDlgItem(hDlg, IDEDIT), string1, 255);
			}
			RemoveEquals(string1, count1);
			_strupr_s(string1, sizeof(string1));

			ASSERT(count2 = (int)SendDlgItemMessage(hDlg, IDEDIT, EM_GETLINECOUNT, 1, 0));
			{
        *((LPWORD)string2) = sizeof(string2);
				count2 = (int)SendDlgItemMessage(hDlg, IDEDIT, EM_GETLINE, 1, (LPARAM)(LPSTR)string2);
				count2 = GetWindowText(GetDlgItem(hDlg, IDEDIT), string2, 255);
			}
			RemoveEquals(string2, count2);
			_strupr_s(string2, sizeof(string2));

			ui.ShowUI = 0;
			ui.ModalUI = 0;
			ui.hParent = hDlg;

			if (lstrcmp(string1,"TRUE") == 0 || lstrcmp(string1,"1") == 0)
			{
				ui.ShowUI = TRUE;
			}
			if (lstrcmp(string2,"TRUE") == 0 || lstrcmp(string1,"1") == 0)
			{
				ui.ModalUI = TRUE;
			}

			ret = UserInterface(&ui,msg);

			SetWindowText(GetDlgItem(hDlg,IDEDIT)," ");

			if(ui.ShowUI == TRUE)
			{
				sprintf_s(buffer, sizeof(buffer), "ShowUI=%s\r\n","TRUE");
				SendDlgItemMessage(hDlg,IDEDIT,EM_REPLACESEL,0,(LPARAM)(LPSTR)buffer);
			}
			else
			{
				sprintf_s(buffer, sizeof(buffer), "ShowUI=%s\r\n","FALSE");
				SendDlgItemMessage(hDlg,IDEDIT,EM_REPLACESEL,0,(LPARAM)(LPSTR)buffer);
			}

			if(ui.ModalUI == TRUE)
			{
				sprintf_s(buffer, sizeof(buffer), "ModalUI=%s\r\n","TRUE");
				SendDlgItemMessage(hDlg,IDEDIT,EM_REPLACESEL,1,(LPARAM)(LPSTR)buffer);
			}
			else
			{
				sprintf_s(buffer, sizeof(buffer), "ModalUI=%s\r\n","FALSE");
				SendDlgItemMessage(hDlg,IDEDIT,EM_REPLACESEL,1,(LPARAM)(LPSTR)buffer);
			}
			break;

		/*
		* XferGroup
		*/
		case DAT_XFERGROUP:
			ShowRC_CC(hDlg,0,0,0,"Not supported from Specials menu","DAT_XFERGROUP");
			ret = TWRC_FAILURE;
			break;

		/*
		*	DeviceEvent
		*/
		case DAT_DEVICEEVENT:
			break;

		/*
		*	FileSystem
		*/
		case DAT_FILESYSTEM:
			{
				int i = 0;

				SetWindowText(GetDlgItem(hDlg,IDEDIT)," ");

				ret = FileSystem(&fileSystem,msg);

				if(msg == MSG_DELETE)
				{
					sprintf_s(buffer, sizeof(buffer), "Recursive=%d\r\n",fileSystem.Recursive);
					SendDlgItemMessage(hDlg,IDEDIT,EM_REPLACESEL,0,(LPARAM)(LPSTR)buffer);
				}
				else if(msg == MSG_GETINFO)
				{
					for(i = 0; i < MAX_FILETYPE; i++)
					{
						if(FileTypes[i].ItemId == fileSystem.FileType)
						{
							sprintf_s(buffer, sizeof(buffer), "FileType=%s\r\n",FileTypes[i].pszItemName);
							SendDlgItemMessage(hDlg,IDEDIT,EM_REPLACESEL,0,(LPARAM)(LPSTR)buffer);
							break;
						}
					}
					sprintf_s(buffer, sizeof(buffer), "Size=%ld\r\n",fileSystem.Size);
					SendDlgItemMessage(hDlg,IDEDIT,EM_REPLACESEL,0,(LPARAM)(LPSTR)buffer);

					strcpy_s(string1, sizeof(string1), fileSystem.CreateTimeDate); 
					sprintf_s(buffer, sizeof(buffer), "CreateTimeDate=%s\r\n",string1);
					SendDlgItemMessage(hDlg,IDEDIT,EM_REPLACESEL,0,(LPARAM)(LPSTR)buffer);

					strcpy_s(string1, sizeof(string1), fileSystem.ModifiedTimeDate); 
					sprintf_s(buffer, sizeof(buffer), "ModifiedTimeDate=%s\r\n",string1);
					SendDlgItemMessage(hDlg,IDEDIT,EM_REPLACESEL,0,(LPARAM)(LPSTR)buffer);

					sprintf_s(buffer, sizeof(buffer), "FreeSpace=%ld\r\n",fileSystem.FreeSpace);
					SendDlgItemMessage(hDlg,IDEDIT,EM_REPLACESEL,0,(LPARAM)(LPSTR)buffer);

					sprintf_s(buffer, sizeof(buffer), "NewImageSize=%ld\r\n",fileSystem.NewImageSize);
					SendDlgItemMessage(hDlg,IDEDIT,EM_REPLACESEL,0,(LPARAM)(LPSTR)buffer);

					sprintf_s(buffer, sizeof(buffer), "NumberOfFiles=%ld\r\n",fileSystem.NumberOfFiles);
					SendDlgItemMessage(hDlg,IDEDIT,EM_REPLACESEL,0,(LPARAM)(LPSTR)buffer);

					sprintf_s(buffer, sizeof(buffer), "NumberOfSnippets=%ld\r\n",fileSystem.NumberOfSnippets);
					SendDlgItemMessage(hDlg,IDEDIT,EM_REPLACESEL,0,(LPARAM)(LPSTR)buffer);

					sprintf_s(buffer, sizeof(buffer), "Reserved=%ld\r\n",fileSystem.Reserved);
					SendDlgItemMessage(hDlg,IDEDIT,EM_REPLACESEL,0,(LPARAM)(LPSTR)buffer);
				}
				else
				{
					strcpy_s(string1, sizeof(string1), fileSystem.InputName); 
					sprintf_s(buffer, sizeof(buffer), "InputName=%s\r\n",string1);
					SendDlgItemMessage(hDlg,IDEDIT,EM_REPLACESEL,0,(LPARAM)(LPSTR)buffer);

					strcpy_s(string1, sizeof(string1), fileSystem.OutputName); 
					sprintf_s(buffer, sizeof(buffer), "OutputName=%s\r\n",string1);
					SendDlgItemMessage(hDlg,IDEDIT,EM_REPLACESEL,0,(LPARAM)(LPSTR)buffer);
				}
			}
			break;

		/*
		*	Passthru
		*/
		case DAT_PASSTHRU:
				SetWindowText(GetDlgItem(hDlg,IDEDIT)," ");

				ret = Passthru(&passthru,msg);

				sprintf_s(buffer, sizeof(buffer), "pCommand=  \r\n");
				SendDlgItemMessage(hDlg,IDEDIT,EM_REPLACESEL,0,(LPARAM)(LPSTR)buffer);

				sprintf_s(buffer, sizeof(buffer), "CommandBytes=%ld\r\n",passthru.CommandBytes);
				SendDlgItemMessage(hDlg,IDEDIT,EM_REPLACESEL,0,(LPARAM)(LPSTR)buffer);

				sprintf_s(buffer, sizeof(buffer), "Direction=%ld\r\n",passthru.Direction);
				SendDlgItemMessage(hDlg,IDEDIT,EM_REPLACESEL,0,(LPARAM)(LPSTR)buffer);

				sprintf_s(buffer, sizeof(buffer), "pData=  \r\n");
				SendDlgItemMessage(hDlg,IDEDIT,EM_REPLACESEL,0,(LPARAM)(LPSTR)buffer);

				sprintf_s(buffer, sizeof(buffer), "DataBytes=%ld\r\n",passthru.DataBytes);
				SendDlgItemMessage(hDlg,IDEDIT,EM_REPLACESEL,0,(LPARAM)(LPSTR)buffer);

				sprintf_s(buffer, sizeof(buffer), "DataBytesXfered=%ld\r\n",passthru.DataBytesXfered);
				SendDlgItemMessage(hDlg,IDEDIT,EM_REPLACESEL,0,(LPARAM)(LPSTR)buffer);

			break;

		/*
		* NULL
		*/
		case DAT_NULL:
			ShowRC_CC(hDlg,0,0,0,"Not supported from Specials menu","DAT_NULL");
			ret = TWRC_FAILURE;
			break;

		default:
			ShowRC_CC(hDlg,0,0,0,"System Error", "Capability unknown");     
			ret = TWRC_FAILURE;
			break;
	}

	return(ret);    
}

/*
* Function: ImageMsg -- 
* Author: TWAIN Working Group
* Input:
*		dat - The current Data Type
*		msg - The current mesage
*		hDlg -
* Output:
*		TRUE is successful
* Comments:
*/
BOOL ImageMsg(TW_UINT16 dat,TW_UINT16 msg,HWND hDlg)
{
	TW_UINT16 ret = TWRC_FAILURE;
	HANDLE bmp = NULL;
	TW_IMAGEINFO Info;
	TW_IMAGELAYOUT Layout;
	TW_PALETTE8 pal8;
	TW_EXTIMAGEINFO ExtImgInfo;
	pTW_GRAYRESPONSE pGrayResp = NULL;
	char buffer[50];
	float fl = 0;
	TW_UINT16 x = 0;
	TW_STR64 string1;
	TW_STR64 string2;
	TW_HANDLE phandle;
	TW_CIECOLOR cie;
	int count = 0;

	memset(&Info, 0, sizeof(TW_IMAGEINFO));
	memset(&Layout, 0, sizeof(TW_IMAGELAYOUT));
	memset(&pal8, 0, sizeof(TW_PALETTE8));
	memset(&ExtImgInfo, 0, sizeof(TW_EXTIMAGEINFO));
	memset(buffer, 0, sizeof(char[50]));
	memset(string1, 0, sizeof(TW_STR64));
	memset(string2, 0, sizeof(TW_STR64));
	memset(&phandle, 0, sizeof(TW_HANDLE));
	memset(&cie, 0, sizeof(TW_CIECOLOR));
	ASSERT(hDlg);

	switch(dat)
	{
		/*
		* CIEColor
		*/
		case DAT_CIECOLOR:
			/*
			* check ImageInfo for TWPT_CIEXYZ
			*/
			if ((ret = CIEColor(&cie)) == TWRC_SUCCESS)
			{
				/*
				* Display MSG_GET result
				*/
				sprintf_s(buffer, sizeof(buffer), "ColorSpace=%ld\r\n",cie.ColorSpace);
				SendDlgItemMessage(hDlg,IDEDIT,EM_REPLACESEL,1,(LPARAM)(LPSTR)buffer);

				sprintf_s(buffer, sizeof(buffer), "LowEndian=%ld\r\n",cie.LowEndian);
				SendDlgItemMessage(hDlg,IDEDIT,EM_REPLACESEL,2,(LPARAM)(LPSTR)buffer);

				sprintf_s(buffer, sizeof(buffer), "DeviceDependent=%ld\r\n",cie.DeviceDependent);
				SendDlgItemMessage(hDlg,IDEDIT,EM_REPLACESEL,3,(LPARAM)(LPSTR)buffer);

				sprintf_s(buffer, sizeof(buffer), "VersionNumber=%ld\r\n",cie.VersionNumber);
				SendDlgItemMessage(hDlg,IDEDIT,EM_REPLACESEL,4,(LPARAM)(LPSTR)buffer);

				/*
				* only show first of Transforms
				*/
				sprintf_s(buffer, sizeof(buffer), "Decode[0].StageABC.StartIn=%.3f\r\n",FIX32ToFloat(cie.StageABC.Decode[0].StartIn));
				SendDlgItemMessage(hDlg,IDEDIT,EM_REPLACESEL,5,(LPARAM)(LPSTR)buffer);

				//DSM_Free(cie.Samples);	
			}
			break;

		/*
		* ExtImageInfo
		*/
		case DAT_EXTIMAGEINFO:

			/*
			* clear the edit control
			*/
			SetWindowText(GetDlgItem(hDlg,IDEDIT)," ");

			if ((ret = ExtImageInfo(&ExtImgInfo))==TWRC_SUCCESS)
			{
				sprintf_s(buffer, sizeof(buffer), "NumInfos=%ld\r\n",ExtImgInfo.NumInfos);
				SendDlgItemMessage(hDlg,IDEDIT,EM_REPLACESEL,2,(LPARAM)(LPSTR)buffer);
				for (x = 0; x < ExtImgInfo.NumInfos; x++)
				{
					sprintf_s(buffer, sizeof(buffer), "Info[%d]=\tInfoID=%d\r\n\tItemType=%d\r\n\tNumItems=%d\r\n\tCondCode=%d\r\n\tItem=%d\r\n",
					x,ExtImgInfo.Info[x].InfoID,ExtImgInfo.Info[x].ItemType, 
					ExtImgInfo.Info[x].NumItems, ExtImgInfo.Info[x].CondCode, 
					ExtImgInfo.Info[x].Item);
					
					SendDlgItemMessage(hDlg,IDEDIT,EM_REPLACESEL,x+2,(LPARAM)(LPSTR)buffer);
				}
			}

			break;

		/*
		* GrayResponse
		*/
		case DAT_GRAYRESPONSE:
		/*
		* Normally, the size of GrayResponse is related to the ImageInfo bitsPerPixel
		* for purposes of testing the functionality to a source, the bitsPerPixel is
		* assumed to be 4, or 16 colors.  Additionally, the PixelType must be set
		* to TWPT_GRAY
		*/
			if ((phandle = DSM_Alloc(sizeof(TW_ELEMENT8)*256))!=NULL)
			{ 
				pGrayResp = (pTW_GRAYRESPONSE)DSM_LockMemory(phandle);

				GetResponse(hDlg,pGrayResp,256);
        if(msg==MSG_SET)//test grayscale-> bitonal
        {
          TW_UINT16 x=0;

	        for (	;x<128;x++)//thresold value
	        {
		        pGrayResp->Response[x].Index = (TW_UINT8)x;
		        pGrayResp->Response[x].Channel1 = 0;
		        pGrayResp->Response[x].Channel2 = 0;
		        pGrayResp->Response[x].Channel3 = 0;
	        }
	        for (	;x<256;x++)
	        {
		        pGrayResp->Response[x].Index = (TW_UINT8)x;
		        pGrayResp->Response[x].Channel1 = 255;
		        pGrayResp->Response[x].Channel2 = 255;
		        pGrayResp->Response[x].Channel3 = 255;
	        }
        }
				DSM_UnlockMemory(phandle);

				if ((ret = GrayResponse(pGrayResp,msg)) == TWRC_SUCCESS)
				{
					DisplayResponse(hDlg,pGrayResp,256);
				}

				DSM_Free(phandle);
			}
			break;

		/*
		* ImageFileXfer
		*/
		case DAT_IMAGEFILEXFER:
			if ((ret = ImageFileXfer(bmp)) == TWRC_SUCCESS)
			{
				SendMessage(hMainWnd, PM_XFERDONE, (WPARAM)bmp, 0);
			}
			break;

		/*
		* ImageInfo
		*/
		case DAT_IMAGEINFO:
			ret = ImageInfo(&Info);

			/*
			* fill the Edit control           
			*/
			fl = FIX32ToFloat(Info.XResolution); 
			sprintf_s(buffer, sizeof(buffer), "XResolution=%.3f\r\n",fl);
			SendDlgItemMessage(hDlg,IDEDIT,EM_REPLACESEL,0,(LPARAM)(LPSTR)buffer);

			fl = FIX32ToFloat(Info.YResolution); 
			sprintf_s(buffer, sizeof(buffer), "YResolution=%.3f\r\n",fl);
			SendDlgItemMessage(hDlg,IDEDIT,EM_REPLACESEL,1,(LPARAM)(LPSTR)buffer);

			sprintf_s(buffer, sizeof(buffer), "ImageWidth=%ld\r\n",Info.ImageWidth);
			SendDlgItemMessage(hDlg,IDEDIT,EM_REPLACESEL,2,(LPARAM)(LPSTR)buffer);

			sprintf_s(buffer, sizeof(buffer), "ImageLength=%ld\r\n",Info.ImageLength);
			SendDlgItemMessage(hDlg,IDEDIT,EM_REPLACESEL,3,(LPARAM)(LPSTR)buffer);

			sprintf_s(buffer, sizeof(buffer), "SamplesPerPixel=%ld\r\n",Info.SamplesPerPixel);
			SendDlgItemMessage(hDlg,IDEDIT,EM_REPLACESEL,4,(LPARAM)(LPSTR)buffer);

			for (x=0;x<8;x++)
			{
				sprintf_s(buffer, sizeof(buffer), "BitsPerSample[%d]=%ld\r\n",x,Info.BitsPerSample[x]);
				SendDlgItemMessage(hDlg,IDEDIT,EM_REPLACESEL,5+x,(LPARAM)(LPSTR)buffer);
			}

			sprintf_s(buffer, sizeof(buffer), "BitsPerPixel=%ld\r\n",Info.BitsPerPixel);
			SendDlgItemMessage(hDlg,IDEDIT,EM_REPLACESEL,13,(LPARAM)(LPSTR)buffer);

			sprintf_s(buffer, sizeof(buffer), "Planar=%ld\r\n",Info.Planar);
			SendDlgItemMessage(hDlg,IDEDIT,EM_REPLACESEL,14,(LPARAM)(LPSTR)buffer);

			sprintf_s(buffer, sizeof(buffer), "PixelType=%ld\r\n",Info.PixelType);
			SendDlgItemMessage(hDlg,IDEDIT,EM_REPLACESEL,15,(LPARAM)(LPSTR)buffer);

			sprintf_s(buffer, sizeof(buffer), "Compression=%ld\r\n",Info.Compression);
			SendDlgItemMessage(hDlg,IDEDIT,EM_REPLACESEL,16,(LPARAM)(LPSTR)buffer);
			break;

		/*
		* ImageLayout
		*/
		case DAT_IMAGELAYOUT:
			if (msg == MSG_SET)
			{		
        *((LPWORD)string1) = sizeof(string1);
				count = (int)SendDlgItemMessage(hDlg,IDEDIT,EM_GETLINE,0,(LPARAM)(pTW_STR64)string1);
				RemoveEquals(string1, count);
				Layout.Frame.Left = FloatToFIX32((float)atof(string1));

				*((LPWORD)string1) = sizeof(string1);
				count = (int)SendDlgItemMessage(hDlg,IDEDIT,EM_GETLINE,1,(LPARAM)(pTW_STR64)string1);
				RemoveEquals(string1, count);
				Layout.Frame.Top = FloatToFIX32((float)atof(string1));

				*((LPWORD)string1) = sizeof(string1);
				count = (int)SendDlgItemMessage(hDlg,IDEDIT,EM_GETLINE,2,(LPARAM)(pTW_STR64)string1);
				RemoveEquals(string1, count);
				Layout.Frame.Right = FloatToFIX32((float)atof(string1));

				*((LPWORD)string1) = sizeof(string1);
				count = (int)SendDlgItemMessage(hDlg,IDEDIT,EM_GETLINE,3,(LPARAM)(pTW_STR64)string1);
				RemoveEquals(string1, count);
				Layout.Frame.Bottom = FloatToFIX32((float)atof(string1));

				*((LPWORD)string1) = sizeof(string1);
				count = (int)SendDlgItemMessage(hDlg,IDEDIT,EM_GETLINE,4,(LPARAM)(pTW_STR64)string1);
				RemoveEquals(string1, count);
				Layout.DocumentNumber = atoi(string1);

				*((LPWORD)string1) = sizeof(string1);
				count = (int)SendDlgItemMessage(hDlg,IDEDIT,EM_GETLINE,5,(LPARAM)(pTW_STR64)string1);
				RemoveEquals(string1, count);
				Layout.PageNumber = atoi(string1);

        *((LPWORD)string1) = sizeof(string1);
				count = (int)SendDlgItemMessage(hDlg,IDEDIT,EM_GETLINE,6,(LPARAM)(pTW_STR64)string1);
				RemoveEquals(string1, count);
				Layout.FrameNumber = atoi(string1);
			}

			ret = ImageLayout(&Layout,msg);

			/*
			* need to clear the display
			*/
			SetWindowText(GetDlgItem(hDlg,IDEDIT)," ");
			SendDlgItemMessage(hDlg,IDEDIT,WM_CLEAR,0,0L);

			/*
			* display results
			*/
			sprintf_s(buffer, sizeof(buffer), "Frame.Left=%.3f\r\n",FIX32ToFloat(Layout.Frame.Left));
			SendDlgItemMessage(hDlg,IDEDIT,EM_REPLACESEL,0,(LPARAM)(LPSTR)buffer);

			sprintf_s(buffer, sizeof(buffer), "Frame.Top=%.3f\r\n",FIX32ToFloat(Layout.Frame.Top));
			SendDlgItemMessage(hDlg,IDEDIT,EM_REPLACESEL,1,(LPARAM)(LPSTR)buffer);

			sprintf_s(buffer, sizeof(buffer), "Frame.Right=%.3f\r\n",FIX32ToFloat(Layout.Frame.Right));
			SendDlgItemMessage(hDlg,IDEDIT,EM_REPLACESEL,2,(LPARAM)(LPSTR)buffer);

			sprintf_s(buffer, sizeof(buffer), "Frame.Bottom=%.3f\r\n",FIX32ToFloat(Layout.Frame.Bottom));
			SendDlgItemMessage(hDlg,IDEDIT,EM_REPLACESEL,3,(LPARAM)(LPSTR)buffer);

			sprintf_s(buffer, sizeof(buffer), "DocumentNumber=%ld\r\n",Layout.DocumentNumber);
			SendDlgItemMessage(hDlg,IDEDIT,EM_REPLACESEL,4,(LPARAM)(LPSTR)buffer);

			sprintf_s(buffer, sizeof(buffer), "PageNumber=%ld\r\n",Layout.PageNumber);
			SendDlgItemMessage(hDlg,IDEDIT,EM_REPLACESEL,5,(LPARAM)(LPSTR)buffer);

			sprintf_s(buffer, sizeof(buffer), "FrameNumber=%ld\r\n",Layout.FrameNumber);
			SendDlgItemMessage(hDlg,IDEDIT,EM_REPLACESEL,6,(LPARAM)(LPSTR)buffer);
			break;

		/*
		* ImageMemXfer
		*/
		case DAT_IMAGEMEMXFER:
			if ((ret = ImageNativeXfer((TW_MEMREF)&bmp))==TWRC_SUCCESS)
			{
				SendMessage(hMainWnd, PM_XFERDONE, (WPARAM)bmp, 0);
			}
			break;

		/*
		* ImageNativeXfer
		*/
		case DAT_IMAGENATIVEXFER:
			if ((ret = ImageNativeXfer((TW_MEMREF)&bmp))==TWRC_SUCCESS)
			{
				SendMessage(hMainWnd, PM_XFERDONE, (WPARAM)bmp, 0);
			}
			break;

		/*
		* JPEGCompression
		*/
		case DAT_JPEGCOMPRESSION:
			break;

		/*
		* Palette8
		*/
		case DAT_PALETTE8:
			sprintf_s(buffer, sizeof(buffer), "NumColors=%ld\r\n",pal8.NumColors);
			SendDlgItemMessage(hDlg,IDEDIT,EM_REPLACESEL,0,(LPARAM)(LPSTR)buffer);

			sprintf_s(buffer, sizeof(buffer), "PaletteType=%ld\r\n",pal8.PaletteType);
			SendDlgItemMessage(hDlg,IDEDIT,EM_REPLACESEL,1,(LPARAM)(LPSTR)buffer);

			if ((ret = Palette8(&pal8,msg))==TWRC_SUCCESS)
			{
				for (x=0;x<pal8.NumColors;x++)
				{
					sprintf_s(buffer, sizeof(buffer), "Color[%d]=[%d,%d,%d]\r\n",x,pal8.Colors[x].Channel1,pal8.Colors[x].Channel2,pal8.Colors[x].Channel3);
					SendDlgItemMessage(hDlg,IDEDIT,EM_REPLACESEL,x+2,(LPARAM)(LPSTR)buffer);
				}
			}
			break;

		/*
		* RGBResponse
		*/
		case DAT_RGBRESPONSE:
		/*
		* the BitsPerPixel is assumed to be 4 or 16 colors.  Normally use ImageInfo to
		* determine size of structure required.  Since RGBResponse and GrayResponse are
		* identical structures in all ways except name, the GrayResponse structure
		* and functions are used here for simplicity.  If the TWAIN spec changes in the
		* future, this may not work
		*/
			if ((phandle = DSM_Alloc(sizeof(TW_ELEMENT8)*16))!=NULL)
			{
				pGrayResp = (pTW_GRAYRESPONSE)DSM_LockMemory(phandle);

				GetResponse(hDlg,pGrayResp,16);

				DSM_UnlockMemory(phandle);

				if ((ret = RGBResponse(pGrayResp,msg)) == TWRC_SUCCESS)
				{
					DisplayResponse(hDlg,pGrayResp,16);
				}

				DSM_Free(phandle);
			}
			break;

		default:
			break;
	}

	return(ret);
}

BOOL AudioMsg(TW_UINT16 dat,TW_UINT16 msg,HWND hDlg)
{
	TW_UINT16 ret = TWRC_FAILURE;

	char buffer[256];
	float fl = 0;
	TW_UINT16 x = 0;
	TW_STR255 string1;
	TW_AUDIOINFO Info;

	memset(&Info, 0, sizeof(TW_AUDIOINFO));
	memset(buffer, 0, sizeof(char[256]));
	memset(string1, 0, sizeof(TW_STR255));
	ASSERT(hDlg);

	switch(dat)
	{
		/*
		* AudioFileXfer
		*/
		case DAT_AUDIOFILEXFER:
			SetWindowText(GetDlgItem(hDlg,IDEDIT)," ");
			break;

		/*
		*	AudioInfo
		*/
		case DAT_AUDIOINFO:
			SetWindowText(GetDlgItem(hDlg,IDEDIT)," ");

			ret = AudioInfo(&Info);
						
			/*
			* fill the Edit control           
			*/
			strcpy_s(string1, sizeof(string1), Info.Name); 
			sprintf_s(buffer, sizeof(buffer), "Name=%s\r\n",string1);
			SendDlgItemMessage(hDlg,IDEDIT,EM_REPLACESEL,0,(LPARAM)(LPSTR)buffer);

			sprintf_s(buffer, sizeof(buffer), "Reserved=%ld\r\n",Info.Reserved);
			SendDlgItemMessage(hDlg,IDEDIT,EM_REPLACESEL,0,(LPARAM)(LPSTR)buffer);
			break;

		/*
		*	AudioNativeXfer
		*/
		case DAT_AUDIONATIVEXFER:
			break;
		
		default:
			break;
	}

	return(ret);
}


/*
* Function: GetResponse --
* Author: TWAIN Working Group
* Input:
*		hDlg - 
*		resp -
*		bts -
* Output:
*		TRUE is successful
* Comments:
*/
BOOL GetResponse(HWND hDlg, pTW_GRAYRESPONSE resp,TW_UINT16 bits)
{
	TW_UINT16 x = 0;

	ASSERT(hDlg);
	ASSERT(resp);

	for (x=0;x<bits;x++)
	{
		resp->Response[x].Index = (TW_UINT8)x;
		resp->Response[x].Channel1 = (TW_UINT8)x;
		resp->Response[x].Channel2 = (TW_UINT8)x;
		resp->Response[x].Channel3 = (TW_UINT8)x;
	}
	return(TRUE);
}

/*
* Function: DisplayResponse --
* Author: TWAIN Working Group
* Input:
*		hDlg - 
*		resp - 
*		bits -
* Output:
*		TRUE is successful
* Comments:
*/
BOOL DisplayResponse(HWND hDlg,pTW_GRAYRESPONSE resp,TW_UINT16 bits)
{  
	TW_UINT16 x = 0;
	char buffer[32];

	memset(buffer, 0, sizeof(char[32]));
	ASSERT(hDlg);
	ASSERT(resp);

	for (x=0;x<bits;x++)
	{
		sprintf_s(buffer, sizeof(buffer), "Response[%d]={%ld,%ld,%ld}\r\n",x,resp->Response[x].Channel1,resp->Response[x].Channel2,resp->Response[x].Channel3);
		SendDlgItemMessage(hDlg,IDEDIT,EM_REPLACESEL,x,(LPARAM)(LPSTR)buffer);
	}
	return(TRUE);
}

/*
* FUNCTION: AltTWItemSize
* ARGS:    ItemType    constant which serves as an index to appropiate
*                      data type
* RETURNS: Result      size of item in bytes
* NOTES:   The routine provides a look-up table to get actual size in bytes of 
* a particular data type.  Using the sizeof call should give the correct
* results on any machine type.  The value for the ItemType parm are found
* in the twain.h file and are indicated by TWTY_XXXX.. (TWTY_UINT16...)
* 
* This routine is provided to allow the Source to use this code intact.  The
* Source does not link to the TWA_GLUE.C module and thus does not have access
* to the TWItemSize array.
*/   
TW_UINT16 AltTWItemSize(TW_INT16 ItemType)
{
	TW_UINT16 result = sizeof(TW_UINT16);

	switch (ItemType)
	{
		case TWTY_INT8:
			result = sizeof(TW_INT8);
			break;
		case TWTY_UINT8:
			result = sizeof(TW_UINT8);
			break;
		case TWTY_INT16:
			result = sizeof(TW_INT16);
			break;
		case TWTY_UINT16:
			result = sizeof(TW_UINT16);
			break;
		case TWTY_INT32:
			result = sizeof(TW_INT32);
			break;
		case TWTY_UINT32:
			result = sizeof(TW_UINT32);
			break;
		case TWTY_FIX32:
			result = sizeof(TW_FIX32);
			break;
		case TWTY_FRAME:
			result = sizeof(TW_FRAME);
			break;
		case TWTY_STR32:
			result = sizeof(TW_STR32);
			break;
		case TWTY_STR64:
			result = sizeof(TW_STR64);
			break;
		case TWTY_STR128:
			result = sizeof(TW_STR128);
			break;
		case TWTY_STR255:
			result = sizeof(TW_STR255);
			break;
		default:
			break;
	}
	return(result);
}

/*
* Function: GetCurrentItemType
* Author: Nancy Létourneau / J.F.L. Peripheral Solutions Inc. / TWAIN Working Group
* Date: August 28/1998
* Input:
*		hDlg - 
* Output: none
* Comments:
*/
void GetCurrentItemType(HWND hDlg, pTABLEENTRY pEntry)
{
	g_pCurrentItemType = pEntry;
	
	switch(g_pCurrentItemType->ItemId)
	{
		case TWTY_FIX32:
		case TWTY_FRAME:
			g_bAllowFrac = TRUE;
			g_bNegative = TRUE;
			g_bString = FALSE;
			break;

		case TWTY_STR32:
		case TWTY_STR64:
		case TWTY_STR128:
		case TWTY_STR255:
			g_bString = TRUE;
			g_bAllowFrac = FALSE;
			g_bNegative = FALSE;
			break;

		case TWTY_INT8:
		case TWTY_INT16:
		case TWTY_INT32:
			g_bNegative = TRUE;
			g_bAllowFrac = FALSE;
			g_bString = FALSE;
			break;

		default:
			g_bAllowFrac = FALSE;
			g_bString = FALSE;
			g_bNegative = FALSE;
			break;
	}
	return;
}

/*
* Function: ClearTheControl
* Author: Nancy Létourneau / J.F.L. Peripheral Solutions Inc. / TWAIN Working Group
* Date: August 31, 1998
* Input:
*		hDlg: Handle tho the dialog window
* Output: none
* Comment:
*		Clear all the EditBox.
*/
void ClearTheControl(HWND hDlg)
{
	TW_STR64 text;
	pTABLECAP pTableCap = NULL;
	TW_UINT32 i = 0;

	memset(text, 0, sizeof(TW_STR64));

	/*
	* Get the current capability
	*/
	GetDlgItemText(hDlg, IDCAP, text, sizeof(text));

	for(i = 0; i < MAX_CAP; i++)
	{
		if(lstrcmp(text, Caps[i].EntryHeader.pszItemName) == 0)
		{
			pTableCap = &Caps[i];
			break;
		}
	}
	ASSERT(pTableCap);

	SetWindowText(GetDlgItem(hDlg, IDVALUE), "");
	SetWindowText(GetDlgItem(hDlg, IDLEFT), "");
	SetWindowText(GetDlgItem(hDlg, IDTOP), "");
	SetWindowText(GetDlgItem(hDlg, IDRIGHT), "");
	SetWindowText(GetDlgItem(hDlg, IDBOTTOM), "");
	SetWindowText(GetDlgItem(hDlg, IDMINVALUE), "0");
	SetWindowText(GetDlgItem(hDlg, IDMAXVALUE), "0");
	SetWindowText(GetDlgItem(hDlg, IDSTEPSIZE), "0");
	SetWindowText(GetDlgItem(hDlg, IDDEFAULTVALUE), "0");
	SetWindowText(GetDlgItem(hDlg, IDCURRENTVALUE), "0");
	SetWindowText(GetDlgItem(hDlg, IDNUMITEMS), "1");
	SetWindowText(GetDlgItem(hDlg, IDCURRENTINDEX), "0");
	SetWindowText(GetDlgItem(hDlg, IDDEFAULTINDEX), "0");
	/*
	* Get the current ConType
	*/
	GetDlgItemText(hDlg, IDTYPE, text, sizeof(text));
	if(lstrcmp(text, "TWON_ENUMERATION") == 0)
	{
		SendDlgItemMessage(hDlg, IDITEMLISTENUM, LB_RESETCONTENT, 0, 0);
		SetItemListBox(hDlg, 1, pTableCap, IDITEMLISTENUM);
	}
	else if(lstrcmp(text, "TWON_ARRAY") == 0)
	{
		SendDlgItemMessage(hDlg, IDITEMLISTARRAY, LB_RESETCONTENT, 0, 0);
		SetItemListBox(hDlg, 1, pTableCap, IDITEMLISTARRAY);
	}
	return;
}

/*
* Function: SetItemListBox
* Author: Nancy Létourneau / J.F.L. Peripheral Solutions Inc. / TWAIN Working Group
* Date: August 26, 1998
* Input:
*		hDlg - handle to the dialog window
*		uiItems - Number that the users write in the NumItems Edit Box. 
*		pTableCap - Pointer to the structure of the current capability.
*		ControlID - The current Control of the Dialog Box when this function is called.
* Output: none
* Comments:
*		Fill the List box with the good string on display.
*/
void SetItemListBox(HWND hDlg, TW_UINT32 uiItems, pTABLECAP pTableCap, TW_UINT32 ControlID)
{
	TW_STR64 text;
	TW_UINT32 temp = 0;
	TW_UINT32 i = 0;
	memset(text, 0, sizeof(TW_STR64));

	/*
	* To remove item in the List Box
	*/
	if(uiItems < uiNumItems)
	{
		temp = uiNumItems - uiItems;
		for(i = 1; i < temp + 1; i++)
		{
			SendDlgItemMessage(hDlg, ControlID, LB_DELETESTRING, uiNumItems - i, 0);
		}
	}
	/*
	* To add item in the List Box
	*/
	else if(uiItems > uiNumItems)
	{
		SendDlgItemMessage(hDlg, ControlID, LB_GETTEXT, uiNumItems - 1, (LPARAM)(LPSTR)text);
		temp = uiItems - uiNumItems;

		for(i = 1; i < temp+1; i++)
		{
			switch(pTableCap->ExpectedType)
			{
				case TWTY_FRAME:
					{
						TW_STR64 string;
						float fl = 0;
						memset(string, 0, sizeof(TW_STR64));

						lstrcpy(text, "L=");
						fl = FIX32ToFloat(g_Frame.Left);
						sprintf_s(string, sizeof(string), "%.3f", fl);
						lstrcat(text, string);

						lstrcat(text, ",T=");
						fl = FIX32ToFloat(g_Frame.Top);
						sprintf_s(string, sizeof(string), "%.3f", fl);
						lstrcat(text, string);

						lstrcat(text, ",R=");
						fl = FIX32ToFloat(g_Frame.Right);
						sprintf_s(string, sizeof(string), "%.3f", fl);
						lstrcat(text, string);

						lstrcat(text, ",B=");
						fl = FIX32ToFloat(g_Frame.Bottom);
						sprintf_s(string, sizeof(string), "%.3f", fl);
						lstrcat(text, string);
					}
					break;

				case TWTY_FIX32:
					lstrcpy(text, "0.000");
					break;

				case TWTY_INT8:
				case TWTY_INT16:
				case TWTY_INT32:
				case TWTY_UINT8:
				case TWTY_UINT16:
				case TWTY_UINT32:
				case TWTY_BOOL:
					if(pTableCap->pItemTable == NULL)
					{
						lstrcpy(text, "0");
					}
					else
					{
						lstrcpy(text, pTableCap->pItemTable[0].pszItemName);
					}
					break;

				case TWTY_STR32:
				case TWTY_STR64:
				case TWTY_STR128:
				case TWTY_STR255:
					lstrcpy(text, "String");
					break;

				default:
					break;
			}
			SendDlgItemMessage(hDlg, ControlID, LB_ADDSTRING, uiNumItems + i, (LPARAM)(LPSTR)text);
		}
	}
	uiNumItems = uiItems;
	return;
}

/*
* Function: UnSubClassEditBox
* Author: Nancy Létourneau / J.F.L. Peripheral Solutions Inc. / TWAIN Working Group
* Date: August 31, 1998
* Input:
*		uiOldIdControl - Id for the last control is display before changing.
*		hDlg - handle to the dialog window
* Output: none
* Comments:
*		Unsubclass the Edit box.
*/
void UnSubClassEditBox(TW_UINT32 uiOldIdControl, HWND hDlg)
{
	WNDPROC wp;

	switch(uiOldIdControl)
	{
		case IDVALUE:
			if(g_bNewEditBox)
			{
				wp = (WNDPROC)(LRESULT)RemoveProp(GetDlgItem(hDlg, uiOldIdControl), 
							g_szSubClassEditBoxProc);
				if(wp)
				{
#ifdef WIN64
		      SetWindowLongPtr(GetDlgItem(hDlg, uiOldIdControl), GWLP_WNDPROC, (LPARAM)wp);
#else
		      SetWindowLong(GetDlgItem(hDlg, uiOldIdControl), GWL_WNDPROC, (LPARAM)wp);
#endif
					g_bNewEditBox = FALSE;
				}
			}
			break;

		case IDLEFT:
		case IDTOP:
		case IDRIGHT:
		case IDBOTTOM:
			if(g_bNewEditBox)
			{
				/*
				* Control IDLEFT
				*/
				wp = (WNDPROC)(LRESULT)RemoveProp(GetDlgItem(hDlg, IDLEFT), 
							g_szSubClassEditBoxProc);
				if(wp)
				{
#ifdef WIN64
		      SetWindowLongPtr(GetDlgItem(hDlg, IDLEFT), GWLP_WNDPROC, (LPARAM)wp);
#else
		      SetWindowLong(GetDlgItem(hDlg, IDLEFT), GWL_WNDPROC, (LPARAM)wp);
#endif
				}

				/*
				* Control IDTOP
				*/
				wp = (WNDPROC)(LRESULT)RemoveProp(GetDlgItem(hDlg, IDTOP), 
							g_szSubClassEditBoxProc);
				if(wp)
				{
#ifdef WIN64
		      SetWindowLongPtr(GetDlgItem(hDlg, IDTOP), GWLP_WNDPROC, (LPARAM)wp);
#else
		      SetWindowLong(GetDlgItem(hDlg, IDTOP), GWL_WNDPROC, (LPARAM)wp);
#endif
				}

				/*
				* Control IDRIGHT
				*/
				wp = (WNDPROC)(LRESULT)RemoveProp(GetDlgItem(hDlg, IDRIGHT), 
							g_szSubClassEditBoxProc);
				if(wp)
				{
#ifdef WIN64
		      SetWindowLongPtr(GetDlgItem(hDlg, IDRIGHT), GWLP_WNDPROC, (LPARAM)wp);
#else
		      SetWindowLong(GetDlgItem(hDlg, IDRIGHT), GWL_WNDPROC, (LPARAM)wp);
#endif
				}

				/*
				* Control IDBOTTOM
				*/
				wp = (WNDPROC)(DWORD)RemoveProp(GetDlgItem(hDlg, IDBOTTOM), 
							g_szSubClassEditBoxProc);
				if(wp)
				{
#ifdef WIN64
		      SetWindowLongPtr(GetDlgItem(hDlg, IDBOTTOM), GWLP_WNDPROC, (LPARAM)wp);
#else
		      SetWindowLong(GetDlgItem(hDlg, IDBOTTOM), GWL_WNDPROC, (LPARAM)wp);
#endif
				}
				if(g_bFrameDlg == FALSE)
				{
					g_bNewEditBox = FALSE;
				}
			}
			break;

		case IDMINVALUE:
		case IDMAXVALUE:
		case IDSTEPSIZE:
		case IDDEFAULTVALUE:
		case IDCURRENTVALUE:
			if(g_bNewEditBox)
			{
				/*
				* Control IDMINVALUE
				*/
				wp = (WNDPROC)(LRESULT)RemoveProp(GetDlgItem(hDlg, IDMINVALUE), 
							g_szSubClassEditBoxProc);
				if(wp)
				{
#ifdef WIN64
		      SetWindowLongPtr(GetDlgItem(hDlg, IDMINVALUE), GWLP_WNDPROC, (LPARAM)wp);
#else
		      SetWindowLong(GetDlgItem(hDlg, IDMINVALUE), GWL_WNDPROC, (LPARAM)wp);
#endif				
        }

				/*
				* Control IDMAXVALUE
				*/
				wp = (WNDPROC)(LRESULT)RemoveProp(GetDlgItem(hDlg, IDMAXVALUE), 
							g_szSubClassEditBoxProc);
				if(wp)
				{
#ifdef WIN64
		      SetWindowLongPtr(GetDlgItem(hDlg, IDMAXVALUE), GWLP_WNDPROC, (LPARAM)wp);
#else
		      SetWindowLong(GetDlgItem(hDlg, IDMAXVALUE), GWL_WNDPROC, (LPARAM)wp);
#endif				
				}

				/*
				* Control IDSTEPSIZE
				*/
				wp = (WNDPROC)(LRESULT)RemoveProp(GetDlgItem(hDlg, IDSTEPSIZE), 
							g_szSubClassEditBoxProc);
				if(wp)
				{
#ifdef WIN64
		      SetWindowLongPtr(GetDlgItem(hDlg, IDSTEPSIZE), GWLP_WNDPROC, (LPARAM)wp);
#else
		      SetWindowLong(GetDlgItem(hDlg, IDSTEPSIZE), GWL_WNDPROC, (LPARAM)wp);
#endif				
				}

				/*
				* Control IDDEFAULTVALUE
				*/
				wp = (WNDPROC)(LRESULT)RemoveProp(GetDlgItem(hDlg, IDDEFAULTVALUE), 
							g_szSubClassEditBoxProc);
				if(wp)
				{
#ifdef WIN64
		      SetWindowLongPtr(GetDlgItem(hDlg, IDDEFAULTVALUE), GWLP_WNDPROC, (LPARAM)wp);
#else
		      SetWindowLong(GetDlgItem(hDlg, IDDEFAULTVALUE), GWL_WNDPROC, (LPARAM)wp);
#endif				
				}

				/*
				* Control IDCURRENTVALUE
				*/
				wp = (WNDPROC)(LRESULT)RemoveProp(GetDlgItem(hDlg, IDCURRENTVALUE), 
							g_szSubClassEditBoxProc);
				if(wp)
				{
#ifdef WIN64
		      SetWindowLongPtr(GetDlgItem(hDlg, IDCURRENTVALUE), GWLP_WNDPROC, (LPARAM)wp);
#else
		      SetWindowLong(GetDlgItem(hDlg, IDCURRENTVALUE), GWL_WNDPROC, (LPARAM)wp);
#endif				
				}
				g_bNewEditBox = FALSE;
			}
			break;

		case IDITEMLISTARRAY:
			if(g_bNewEditBox)
			{
				/*
				* Control IDNUMITEMS
				*/
				wp = (WNDPROC)(LRESULT)RemoveProp(GetDlgItem(hDlg, IDNUMITEMS), 
							g_szSubClassEditBoxProc);
				if(wp)
				{
#ifdef WIN64
		      SetWindowLongPtr(GetDlgItem(hDlg, IDNUMITEMS), GWLP_WNDPROC, (LPARAM)wp);
#else
		      SetWindowLong(GetDlgItem(hDlg, IDNUMITEMS), GWL_WNDPROC, (LPARAM)wp);
#endif				
				}
				g_bNewEditBox = FALSE;
				g_bNewDisplay = FALSE;
			}
			break;

		case IDCURRENTINDEX:
		case IDDEFAULTINDEX:
			if(g_bNewEditBox)
			{
				/*
				* Control IDNUMITEMS
				*/
				wp = (WNDPROC)(LRESULT)RemoveProp(GetDlgItem(hDlg, IDNUMITEMS), 
							g_szSubClassEditBoxProc);
				if(wp)
				{
#ifdef WIN64
		      SetWindowLongPtr(GetDlgItem(hDlg, IDNUMITEMS), GWLP_WNDPROC, (LPARAM)wp);
#else
		      SetWindowLong(GetDlgItem(hDlg, IDNUMITEMS), GWL_WNDPROC, (LPARAM)wp);
#endif				
				}

				/*
				* Control IDCURRENTINDEX
				*/
				wp = (WNDPROC)(LRESULT)RemoveProp(GetDlgItem(hDlg, IDCURRENTINDEX), 
							g_szSubClassEditBoxProc);
				if(wp)
				{
#ifdef WIN64
		      SetWindowLongPtr(GetDlgItem(hDlg, IDCURRENTINDEX), GWLP_WNDPROC, (LPARAM)wp);
#else
		      SetWindowLong(GetDlgItem(hDlg, IDCURRENTINDEX), GWL_WNDPROC, (LPARAM)wp);
#endif				
				}

				/*
				* Control IDDEFAULTINDEX
				*/
				wp = (WNDPROC)(LRESULT)RemoveProp(GetDlgItem(hDlg, IDDEFAULTINDEX), 
							g_szSubClassEditBoxProc);
				if(wp)
				{
#ifdef WIN64
		      SetWindowLongPtr(GetDlgItem(hDlg, IDDEFAULTINDEX), GWLP_WNDPROC, (LPARAM)wp);
#else
		      SetWindowLong(GetDlgItem(hDlg, IDDEFAULTINDEX), GWL_WNDPROC, (LPARAM)wp);
#endif				
				}
				g_bNewEditBox = FALSE;
				g_bNewDisplay = FALSE;
			}
			break;
		
		case IDEDITBOX:
			if(g_bNewEditBox)
			{
				/*
				* Control IDEDITBOX
				*/
				wp = (WNDPROC)(LRESULT)RemoveProp(GetDlgItem(hDlg, IDEDITBOX), 
							g_szSubClassEditBoxProc);
				if(wp)
				{
#ifdef WIN64
		      SetWindowLongPtr(GetDlgItem(hDlg, IDEDITBOX), GWLP_WNDPROC, (LPARAM)wp);
#else
		      SetWindowLong(GetDlgItem(hDlg, IDEDITBOX), GWL_WNDPROC, (LPARAM)wp);
#endif				
				}

				if(g_bNewDisplay == TRUE)
				{
					TW_STR64 text;

					memset(text, 0, sizeof(TW_STR64));
					
					GetDlgItemText(hDlg, IDTYPE, text, sizeof(text));
					if(lstrcmp(text, "TWON_ENUMERATION") == 0)
					{
						/*
						* Control IDNUMITEMS
						*/
						wp = (WNDPROC)(LRESULT)RemoveProp(GetDlgItem(hDlg, IDNUMITEMS), 
									g_szSubClassEditBoxProc);
						if(wp)
						{
#ifdef WIN64
		          SetWindowLongPtr(GetDlgItem(hDlg, IDNUMITEMS), GWLP_WNDPROC, (LPARAM)wp);
#else
		          SetWindowLong(GetDlgItem(hDlg, IDNUMITEMS), GWL_WNDPROC, (LPARAM)wp);
#endif				
						}

						/*
						* Control IDCURRENTINDEX
						*/
						wp = (WNDPROC)(LRESULT)RemoveProp(GetDlgItem(hDlg, IDCURRENTINDEX), 
									g_szSubClassEditBoxProc);
						if(wp)
						{
#ifdef WIN64
		          SetWindowLongPtr(GetDlgItem(hDlg, IDCURRENTINDEX), GWLP_WNDPROC, (LPARAM)wp);
#else
		          SetWindowLong(GetDlgItem(hDlg, IDCURRENTINDEX), GWL_WNDPROC, (LPARAM)wp);
#endif				
						}

						/*
						* Control IDDEFAULTINDEX
						*/
						wp = (WNDPROC)(LRESULT)RemoveProp(GetDlgItem(hDlg, IDDEFAULTINDEX), 
									g_szSubClassEditBoxProc);
						if(wp)
						{
#ifdef WIN64
		          SetWindowLongPtr(GetDlgItem(hDlg, IDDEFAULTINDEX), GWLP_WNDPROC, (LPARAM)wp);
#else
		          SetWindowLong(GetDlgItem(hDlg, IDDEFAULTINDEX), GWL_WNDPROC, (LPARAM)wp);
#endif				
						}
					}
					else if(lstrcmp(text, "TWON_ARRAY") == 0)
					{
						/*
						* Control IDNUMITEMS
						*/
						wp = (WNDPROC)(LRESULT)RemoveProp(GetDlgItem(hDlg, IDNUMITEMS), 
									g_szSubClassEditBoxProc);
						if(wp)
						{
#ifdef WIN64
		          SetWindowLongPtr(GetDlgItem(hDlg, IDNUMITEMS), GWLP_WNDPROC, (LPARAM)wp);
#else
		          SetWindowLong(GetDlgItem(hDlg, IDNUMITEMS), GWL_WNDPROC, (LPARAM)wp);
#endif				
						}
					}
					g_bNewEditBox = FALSE;
					g_bNewDisplay = FALSE;
				}
			}

			break;

		default:
			break;
	}
	return;
}

/*
* Function: SubClassEditProc
* Author: Nancy Létourneau / J.F.L. Peripheral Solutions Inc. / TWAIN Working Group
* Date: August 28/1998
* Input:
*		hWnd - handle to current control window
*		iMsg - windows message
*		wParam - 1st windows parm (message specific information)
*		lParam - 2nd windows parm
* Output:
*
* Comments:
*/
LRESULT CALLBACK SubClassEditProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT  result = 0;

	WNDPROC wp = GetProp(hWnd, g_szSubClassEditBoxProc);
  
  TRACE("SubClassEditProc Message 0x%x\n", iMsg);

	switch(iMsg)
	{
		case WM_CHAR:
			{
				char cInChar = (char)wParam;

				/*
				* Case if it's number TW_UINTxx, TW_INTxx, TW_FRAME, TW_FIX32
				*/
				if((g_bNegative && (cInChar == '-')) ||
					(g_bAllowFrac && (cInChar == '.')) ||
					((cInChar <= '9') && (cInChar >='0')) ||
					(cInChar == 0x08))
				{
					TW_STR32 szItem;
					int i = 0;
					DWORD dwStart = 0, dwEnd = 0;
					SendMessage(hWnd, EM_GETSEL, (WPARAM)&dwStart, (LPARAM)&dwEnd);

					GetWindowText(hWnd, szItem, 32);

					if(((cInChar == '-')&&(szItem[0]=='-'))||
							((cInChar == '-')&&((dwStart != 0)&&(dwEnd != 0))))
					{
						return 0;
					}

					/*
					* Case of TW_FIX32 or TW_FRAME
					*/
					if(cInChar == '.')
					{
						while(szItem[i] != '\0')
						{
							if(szItem[i] == '.')
							{
								return 0;
							}
							i++;
						}
					}
				}
				/*
				*	Case if it's a TW_STRxxx
				*/
				else
				{
					return 0;
				}
			}
			break;

		case WM_KILLFOCUS:
			{
				TW_STR32 szItem;
				float Value = 0;
				TW_STR64 string;
	
				GetWindowText(hWnd, (LPTSTR)szItem ,sizeof(szItem));

				if((g_pCurrentItemType->ItemId == TWTY_FIX32) ||
					 (g_pCurrentItemType->ItemId == TWTY_FRAME))
				{
					Value = (float)atof(szItem);
					sprintf_s(string, sizeof(string), "%.3f", Value);

					if(!g_bFrameDlg)
					{
						TW_UINT32 Id;

						Id = GetDlgCtrlID(hWnd);

						SetWindowText(GetDlgItem(g_hMainDlg,GetDlgCtrlID(hWnd)),"");
						SendDlgItemMessage(g_hMainDlg, GetDlgCtrlID(hWnd),EM_REPLACESEL,0,(LPARAM)(LPSTR)string);
					}
					else
					{
						TW_UINT32 Id;

						Id = GetDlgCtrlID(hWnd);

						SetWindowText(GetDlgItem(g_hMainFrameDlg,GetDlgCtrlID(hWnd)),"");
						SendDlgItemMessage(g_hMainFrameDlg, GetDlgCtrlID(hWnd),EM_REPLACESEL,0,(LPARAM)(LPSTR)string);
					}
				}
			}
			break;

		default:
			break;
	}
	
	ASSERT(wp);
	if(wp)
	{

		#ifdef STRICT
			result = CallWindowProc(wp, hWnd, iMsg, wParam, lParam);
	//		result = CallWindowProc(g_fnOldControl, hWnd, iMsg, wParam, lParam);
		#else
			result = CallWindowProc((DLGPROC)wp, hWnd, iMsg, wParam, lParam);
	//		result = CallWindowProc((FARPROC)g_fnOldControl, hWnd, iMsg, wParam, lParam);
		#endif
	}
	return result;

}