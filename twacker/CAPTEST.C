/*
* File:         captest.c
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
*/
//----------------------------------------------------------------------
//                            I n c l u d e s
//----------------------------------------------------------------------
#include <windows.h>         // Req. for twain.h type defs and ...
#include <stdio.h>  
#include <stdlib.h>
#include <string.h> 
#include <commdlg.h>

#include "..\\twcommon\\twain.h"
#include "..\\twcommon\\twndebug.h"

#include "table.h"
#include "dca_app.h"
#include "dca_glue.h"
#include "dca_acq.h"
#include "captest.h" 
#include "special.h"
#include "twacker.h"
#ifdef WIN64
#include "res_64.h"
#elif defined(WIN32)
#include "res_32.h"
#else
#include "res_16.h"
#endif

#define CAP_AUTOFINAL CAP_CUSTOMBASE+1

//----------------------------------------------------------------------
//                            V a r i a b l e s
//----------------------------------------------------------------------
static int AutoFeedBOOL = FALSE;
static int MessageLevelVAR = ERROR; //To display what is happening
extern HWND g_hMainDlg;

//----------------------------------------------------------------------
//                            F u n c t i o n s
//----------------------------------------------------------------------

TW_UINT16 AltTWItemSize(TW_INT16 ItemType);

void GetItem (char *Text, TW_UINT16 datatype, LPSTR pData);

/*
* Function: TWXferMech -- 
* Author: TWAIN Working Group
* Input:
*		hWnd - handle to window
* Output:
*		TW_INT16
* Comments:
*		Set the current transfer mechanism for Twain based on the menus
* that have been checked by the user.
*/
TW_INT16 TWXferMech(HWND hWnd)

{
	TW_CAPABILITY   cap;
	pTW_ONEVALUE    pval = NULL;
	TW_INT16        status = TWRC_FAILURE;  
	OPENFILENAME    ofn;
	char szDirName[256];
	char szFile[256];
	char szFileTitle[256];
	char chReplace = '|';
	char szFilter[256];
	UINT i = 0, cbString = 0;

	/*
	*	Initialize all structures
	*/
	memset(szDirName, 0, sizeof(char[256]));
	memset(szFile, 0, sizeof(char[256]));
	memset(szFileTitle, 0, sizeof(char[256]));
	memset(szFilter, 0, sizeof(char[256]));
	memset(&cap, 0, sizeof(TW_CAPABILITY));
	memset(&ofn, 0, sizeof(OPENFILENAME));

	ASSERT(hWnd);

	LogMessage("TWXferMech\r\n");

	cap.Cap = ICAP_XFERMECH;
	cap.ConType = TWON_ONEVALUE;

	/*
	* alloc the container
	*/
	if (cap.hContainer = DSM_Alloc(sizeof(TW_ONEVALUE)))
	{
		pval = (pTW_ONEVALUE)DSM_LockMemory(cap.hContainer);
		pval->ItemType = TWTY_UINT16;

		if (GetMenuState (GetMenu (hWnd), TW_APP_BUFFER, MF_BYCOMMAND) & MF_CHECKED)       
		{
			/*
			* memory transfer
			*/
			pval->Item = TWSX_MEMORY;
		}
		else if (GetMenuState (GetMenu (hWnd), TW_APP_FILE, MF_BYCOMMAND) & MF_CHECKED)       
		{
			/*
			* file transfer
			*/
			pval->Item = TWSX_FILE;

			/*
			* file transfer currently fixed to bitmap format
			*
			* get the filename to save as
			* check formats supported by the source            
			*/

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
			ofn.nFileOffset = 0;
			ofn.nFileExtension = 0;
			ofn.lpstrDefExt = (LPSTR)"bmp";
			ofn.lCustData = 0L;
			ofn.lpfnHook = NULL;
			ofn.lpTemplateName = (LPSTR)NULL;

			if (GetSaveFileName(&ofn)==FALSE)
			{
				DSM_UnlockMemory(cap.hContainer);
				DSM_Free(cap.hContainer);
				cap.hContainer = NULL;
				return(TWRC_FAILURE);
			}
			else
			{
				strcpy_s(Filename, sizeof(Filename), szFileTitle);
			}
		}
		else 
		{
			pval->Item = TWSX_NATIVE;
		}

		DSM_UnlockMemory(cap.hContainer);

		status = CallDSMEntry(&appID,
						&dsID,
						DG_CONTROL, 
						DAT_CAPABILITY, 
						MSG_SET,
						(TW_MEMREF)&cap);

		DSM_Free((HANDLE)cap.hContainer);

		if (status != TWRC_SUCCESS)
		{
			if (MessageLevel() >= ML_ERROR)
			{
				ShowRC_CC(hWnd, 1, status, 1, "","MSG_SET of ICAP_XFERMECH"); 
			}
		} 
	}
	else
	{
		status = TWRC_FAILURE;
		if (MessageLevel() >= ML_ERROR)
		{
			ShowRC_CC(hWnd, 0, 0, 0, "Memory Allocation Failed","MSG_SET of ICAP_XFERMECH"); 
		}
	}
	return status;
}  

/*
* Function: TWAutofeed -- 
* Author: TWAIN Working Group
* Input:
*		hWnd - handle to window
* Output:
*		TW_INT16
* Comments:
*		Checks if the autofeed option on the FILE menu is
* checked and if so begins a capability negotiation.  If the ADF is already
* enabled, (check this) the return value is ??? otherwise the application
* negotiates with the source to enable the capability
*
* If the ADF at the source cannot be enabled, the application should post an 
* error and not allow a transfer (ie: the Source's UI to come up).
*/
TW_INT16 TWAutofeed(HWND hWnd)
{
	TW_CAPABILITY   cap;
	pTW_ONEVALUE    pval = NULL;
	TW_INT16        status = TWRC_SUCCESS;
	char            Details[128];

	memset(&cap, 0, sizeof(TW_CAPABILITY));
	memset(Details, 0, sizeof(char[128]));

	ASSERT(hWnd);

	AutoFeedBOOL = FALSE;
	strcpy_s(Details, sizeof(Details), "");

	/*
	* If Automatic Feeding > CAP_AUTOFEED On is checked
	*/
	if (!(GetMenuState (GetMenu (hWnd), TW_APP_AUTOFEED, MF_BYCOMMAND) & MF_CHECKED))
	{
		return (status);
	}

	cap.Cap = CAP_FEEDERENABLED;
	cap.ConType = TWON_ONEVALUE;

	status = CallDSMEntry(&appID,
					&dsID,
					DG_CONTROL, 
					DAT_CAPABILITY, 
					MSG_GET,
					(TW_MEMREF)&cap);

	if (status != TWRC_SUCCESS)
	{
		status = TWRC_FAILURE;
		strcpy_s(Details, sizeof(Details), "CAP_FEEDERENABLED");
		DSM_Free(cap.hContainer);
	}
	else
	{   
		pval = (pTW_ONEVALUE)DSM_LockMemory(cap.hContainer);
		if (pval->Item == TRUE)
		{   
			/*
			* Feeder is enabled no need to negotiate
			*/
			DSM_UnlockMemory(cap.hContainer);
			DSM_Free((HANDLE)cap.hContainer); 
		}
		else     
		{
			/*
			* Negotiate with the source
			* Try to turn on CAP_FEEDERENABLED
			*/
			pval->ItemType = TWTY_BOOL;
			pval->Item = TRUE;
			DSM_UnlockMemory(cap.hContainer);

			status = CallDSMEntry(&appID,
							&dsID,
							DG_CONTROL, 
							DAT_CAPABILITY, 
							MSG_SET,
							(TW_MEMREF)&cap);

			/*
			* free here because the GET call will allocate a new container
			*/
			DSM_Free(cap.hContainer);

			if (status == TWRC_SUCCESS)
			{   
				/*
				* Verify that CAP_FEEDERENABLED is now TRUE
				*/
				status = CallDSMEntry(&appID,
								&dsID,
								DG_CONTROL, 
								DAT_CAPABILITY, 
								MSG_GET,
								(TW_MEMREF)&cap);

				if (status == TWRC_SUCCESS)
				{
					pval = (pTW_ONEVALUE)DSM_LockMemory(cap.hContainer);
					if (pval->Item != TRUE) 
					{
						/*
						* Verification failed -- CAP_FEEDERENABLED is FALSE even after successful set to TRUE
						*/
						status = TWRC_FAILURE;
						strcpy_s(Details, sizeof(Details), "CAP_FEEDERENABLED");
					}
					DSM_UnlockMemory(cap.hContainer);
					DSM_Free((HANDLE)cap.hContainer);    
				}
				else
				{
					status = TWRC_FAILURE;                  
					strcpy_s(Details, sizeof(Details), "CAP_FEEDERENABLED");                   
				}               
			}
			else
			{    
				/*
				* MSG_SET of CAP_FEEDERENABLED to TRUE did not succeed
				*/
				strcpy_s(Details, sizeof(Details), "CAP_FEEDERENABLED");
				status = TWRC_FAILURE;
			}
		}

		if (status == TWRC_SUCCESS)
		{
			/*
			* CAP_AUTOFEED
			*/
			cap.Cap = CAP_AUTOFEED;
			cap.ConType = TWON_ONEVALUE;

			status = CallDSMEntry(&appID,
							&dsID,
							DG_CONTROL, 
							DAT_CAPABILITY, 
							MSG_GET,
							(TW_MEMREF)&cap);

			if (status != TWRC_SUCCESS)
			{
				/*
				* MSG_GET on CAP_AUTOFEED did not succeed
				*/
				status = TWRC_FAILURE;
				strcpy_s(Details, sizeof(Details), "CAP_AUTOFEED");            
			}

			/*
			* MSG_GET on CAP_AUTOFEED returned success
			*/
			pval = (pTW_ONEVALUE)DSM_LockMemory(cap.hContainer);
			if (pval->Item == TRUE)
			{
				/*
				* CAP_AUTOFEED is on
				*/
				DSM_UnlockMemory(cap.hContainer);
				DSM_Free((HANDLE)cap.hContainer);         
			}
			else
			{
				/*
				* Try to set CAP_AUTOFEED to TRUE
				*/
				pval->ItemType = TWTY_BOOL;
				pval->Item = TRUE;
				DSM_UnlockMemory(cap.hContainer);

				status = CallDSMEntry(&appID,
									&dsID,
									DG_CONTROL, 
									DAT_CAPABILITY, 
									MSG_SET,
									(TW_MEMREF)&cap);

				DSM_Free((HANDLE)cap.hContainer);     

				if (status == TWRC_SUCCESS)
				{   
					/*
					* Verify that CAP_AUTOFEED is on
					*/
					status = CallDSMEntry(&appID,
									&dsID,
									DG_CONTROL, 
									DAT_CAPABILITY, 
									MSG_GET,
									(TW_MEMREF)&cap);

					if (status == TWRC_SUCCESS)
					{
						pval = (pTW_ONEVALUE)DSM_LockMemory(cap.hContainer);
						if (pval->Item != TRUE)
						{
							status = TWRC_FAILURE;
							strcpy_s(Details, sizeof(Details), "CAP_AUTOFEED");            
							/*
							* CAP_AUTOFEED returns FALSE even after succesful set to TRUE
							*/
						}
						DSM_UnlockMemory(cap.hContainer);
						DSM_Free((HANDLE)cap.hContainer); 
					}
					else
					{
						status = TWRC_FAILURE;
						strcpy_s(Details, sizeof(Details), "CAP_AUTOFEED");            
					}
				}
				else
				{
					/*
					* MSG_SET of CAP_AUTOFEED to TRUE did not succeed
					*/
					status = TWRC_FAILURE;
					strcpy_s(Details, sizeof(Details), "CAP_AUTOFEED");
				}
			}   
		}   
	}   

	/*
	* Set Local Autofeed Variable on/off        
	*/
	if (status == TWRC_SUCCESS)
	{
		AutoFeedBOOL = TRUE;
	}
	else
	{
		AutoFeedBOOL = FALSE;
		if (MessageLevel() >= ML_ERROR)
		{
			ShowRC_CC(hWnd, 0, 0, 0, Details, "CAP_AUTOFEED ERROR");
		}
	}
	return status;
}  

///////////////////////////////////////////////////////////////////////////
// FUNCTION: AutoFeedOn
//
// ARGS:    none
//
// RETURNS: current state of Source Manager (open/closed)
//
// NOTES:   Just a way to reduce the number of global vars
//
BOOL AutoFeedOn (VOID)
{
	return (AutoFeedBOOL);
}
 
///////////////////////////////////////////////////////////////////////////
// Function: TWAutofeedMenu -- 
// Author: TWAIN Working Group
// Input:
//		hWnd - handle to window
// Output:
//		TW_INT16
// Comments:
//		Set the current Autofeed Options and Flags based on the menus
// that have been checked by the user.
//
TW_INT16 TWAutofeedMenu(HWND hWnd)
{
	TW_INT16      status = TWRC_FAILURE;

	ASSERT(hWnd);

	status = TWAutofeed(hWnd);
	return (status);    
}

/*
* Function: FloatToFIX32 -- 
* Author: TWAIN Working Group
* Input:
*		floater -
* Output:
* TW_FIX32
* Comments:
*		Convert a floating point value into a FIX32.
*/
TW_FIX32 FloatToFIX32(float floater)
{
	TW_FIX32 Fix32_value;
	TW_BOOL sign = (floater < 0)?TRUE:FALSE;
	TW_INT32 value = (TW_INT32) (floater * 65536.0 + (sign?(-0.5):0.5));

	memset(&Fix32_value, 0, sizeof(TW_FIX32));

	Fix32_value.Whole = LOWORD(value >> 16);
	Fix32_value.Frac = LOWORD(value & 0x0000ffffL);

	return (Fix32_value);
}

///////////////////////////////////////////////////////////////////////////
// Function: FIX32ToFloat -- 
// Author: TWAIN Working Group
// Input:
//		fix32
// Output:
//		float
// Comments:
//		Convert a FIX32 value into a floating point value
//
float FIX32ToFloat (TW_FIX32 fix32)
{
	float   floater = 0;

	floater = (float) fix32.Whole + (float) (fix32.Frac / 65536.0);
	return(floater);
}

////////////////////////////////////////////////////////////////////////////
// Function: ShowImageInfo --
// Author: TWAIN Working Group
// Input:
//		hWndofApp - handle to window
// Output: none
// Comments:
//
void ShowImageInfo (HWND hWndofApp)
{
	TW_UINT16       twRC = TWRC_FAILURE;
	TW_UINT16       ccRC = TWRC_FAILURE;
	TW_IMAGEINFO    twImageInfo;
	char            Details [255];
	char            Details2 [1024];
	pTW_FIX32       pTWFix32 = NULL;
	TW_UINT16       index = 0;

	memset(&twImageInfo, 0, sizeof(TW_IMAGEINFO));
	memset(Details, 0, sizeof(char[255]));
	memset(Details2, 0, sizeof(char[1024]));

	ASSERT(hWndofApp);

	// Check ImageInfo information
	twRC = CallDSMEntry(&appID,
					&dsID, 
					DG_IMAGE,
					DAT_IMAGEINFO, 
					MSG_GET, 
					(TW_MEMREF)&twImageInfo);

	//Return Code from Operation
	wsprintf(Details2, "%s %d\n", "RETURN CODE:", twRC);

	switch (twRC)
	{
		case TWRC_SUCCESS:
			sprintf_s (Details, sizeof(Details), "\n%s %.2f", "XResolution:", FIX32ToFloat(twImageInfo.XResolution));
			strcat_s(Details2, sizeof(Details2), Details);

			pTWFix32 = &(twImageInfo.YResolution);
			sprintf_s (Details, sizeof(Details), "\n%s %.2f", "YResolution:", FIX32ToFloat(*pTWFix32));
			strcat_s(Details2, sizeof(Details2), Details);

			sprintf_s (Details, sizeof(Details), "\n%s %ld", "ImageWidth:", twImageInfo.ImageWidth);
			strcat_s(Details2, sizeof(Details2), Details);

			sprintf_s (Details, sizeof(Details), "\n%s %ld", "ImageLength:", twImageInfo.ImageLength);
			strcat_s(Details2, sizeof(Details2), Details);

			sprintf_s (Details, sizeof(Details), "\n%s %d", "SamplesPerPixel:", twImageInfo.SamplesPerPixel);
			strcat_s(Details2, sizeof(Details2), Details);

			sprintf_s (Details, sizeof(Details), "\n%s %d", "BitsPerSample:", twImageInfo.BitsPerSample[0]);
			strcat_s(Details2, sizeof(Details2), Details);
			for (index = 1; index < 8; index++)
			{
				sprintf_s (Details, sizeof(Details), ", %d", twImageInfo.BitsPerSample[index]);
				strcat_s(Details2, sizeof(Details2), Details);
			}           

			sprintf_s (Details, sizeof(Details), "\n%s %d", "BitsPerPixel:", twImageInfo.BitsPerPixel);
			strcat_s(Details2, sizeof(Details2), Details);

			sprintf_s (Details, sizeof(Details), "\n%s %d", "Planar:", twImageInfo.Planar);
			strcat_s(Details2, sizeof(Details2), Details);

			sprintf_s (Details, sizeof(Details), "\n%s %d", "PixelType:", twImageInfo.PixelType);
			strcat_s(Details2, sizeof(Details2), Details);

			sprintf_s (Details, sizeof(Details), "\n%s %d", "Compression:", twImageInfo.Compression);
			strcat_s(Details2, sizeof(Details2), Details);
			break;

		case TWRC_FAILURE:
		default:
			// determine details of failure from Source

			if (ccRC == TWRC_SUCCESS)
			{
				sprintf_s (Details, sizeof(Details), "\n%s %d", "Condition Code:", gGlobalStatus.ConditionCode);
				strcat_s(Details2, sizeof(Details2), Details);
			} 
			else 
			{
				sprintf_s (Details, sizeof(Details), "\n%s", "Condition Code Check Failed");
				strcat_s(Details2, sizeof(Details2), Details);
			}
			break;
	}

	ShowRC_CC(hWndofApp,0,0,0,Details2,"TW_IMAGEINFO");
	return;
}

////////////////////////////////////////////////////////////////////////////
// Function: ShowImageLayout
// Author: TWAIN Working Group
// Input:
//		hWndofApp - handle to window
// Output: none
// Comments:
//
void ShowImageLayout (HWND hWndofApp)

{
	TW_UINT16       twRC = TWRC_FAILURE;
	TW_UINT16       ccRC = TWRC_FAILURE;
	TW_IMAGELAYOUT  twImageLayout;
	char            Details [255];
	char            Details2 [1024];

	memset(&twImageLayout, 0, sizeof(TW_IMAGELAYOUT));
	memset(Details, 0, sizeof(char[255]));
	memset(Details2, 0, sizeof(char[1024]));

	ASSERT(hWndofApp);

	// Check ImageLayout information
	twRC = CallDSMEntry(&appID,
					&dsID, 
					DG_IMAGE,
					DAT_IMAGELAYOUT, 
					MSG_GET, 
					(TW_MEMREF)&twImageLayout);

	//Return Code from Operation
	wsprintf(Details2, "%s %d\n", "RETURN CODE:", twRC);

	switch (twRC)
	{
		case TWRC_SUCCESS:
			sprintf_s (Details, sizeof(Details), "\n%s %.2f", "Frame.Left:", FIX32ToFloat(twImageLayout.Frame.Left));
			strcat_s(Details2, sizeof(Details2), Details);
			sprintf_s (Details, sizeof(Details), "\n%s %.2f", "Frame.Top:", FIX32ToFloat(twImageLayout.Frame.Top));
			strcat_s(Details2, sizeof(Details2), Details);
			sprintf_s (Details, sizeof(Details), "\n%s %.2f", "Frame.Right:", FIX32ToFloat(twImageLayout.Frame.Right));
			strcat_s(Details2, sizeof(Details2), Details);
			sprintf_s (Details, sizeof(Details), "\n%s %.2f", "Frame.Bottom:", FIX32ToFloat(twImageLayout.Frame.Bottom));
			strcat_s(Details2, sizeof(Details2), Details);

			sprintf_s (Details, sizeof(Details), "\n%s %d", "DocumentNumber:", twImageLayout.DocumentNumber);
			strcat_s(Details2, sizeof(Details2), Details);

			sprintf_s (Details, sizeof(Details), "\n%s %d", "PageNumber:", twImageLayout.PageNumber);
			strcat_s(Details2, sizeof(Details2), Details);

			sprintf_s (Details, sizeof(Details), "\n%s %d", "FrameNumber:", twImageLayout.FrameNumber);
			strcat_s(Details2, sizeof(Details2), Details);
			break;

		case TWRC_FAILURE:
		default:
			// determine details of failure from Source

			if (ccRC == TWRC_SUCCESS)
			{
				sprintf_s (Details, sizeof(Details), "\n%s %d", "Condition Code:", gGlobalStatus.ConditionCode);
				strcat_s(Details2, sizeof(Details2), Details);
			} 
			else 
			{
				sprintf_s (Details, sizeof(Details), "\n%s", "Condition Code Check Failed");
				strcat_s(Details2, sizeof(Details2), Details);
			}                   
			break;
	}

	ShowRC_CC(hWndofApp,0,0,0,Details2,"TW_IMAGELAYOUT");
	return;
}

///////////////////////////////////////////////////////////////////////////
// Function: ShowTW_ID --
// Author: TWAIN Working Group
// Input:
//		hWndofApp - handle to window
//		twID - 
//		text - pointer on
// Output: none
// Comments:
//
void ShowTW_ID (HWND hWndofApp, TW_IDENTITY twID, char *text)
{
	char Details [255];
	char Details2 [1024];

	memset(Details, 0, sizeof(char[255]));
	memset(Details2, 0, sizeof(char[1024]));

	ASSERT(hWndofApp);
	ASSERT(text);

	wsprintf(Details2, "\n%s %d", "Id:", twID.Id);
	wsprintf (Details, "\n%s %d", "TW_VERSION.MajorNum:", twID.Version.MajorNum);
	strcat_s(Details2, sizeof(Details2), Details);
	wsprintf (Details, "\n%s %d", "TW_VERSION.MinorNum:", twID.Version.MinorNum);
	strcat_s(Details2, sizeof(Details2), Details);
	wsprintf (Details, "\n%s %d", "TW_VERSION.Language:", twID.Version.Language);
	strcat_s(Details2, sizeof(Details2), Details);
	wsprintf (Details, "\n%s %d", "TW_VERSION.Country:", twID.Version.Country);
	strcat_s(Details2, sizeof(Details2), Details);
	wsprintf (Details, "\n%s %s", "TW_VERSION.Info:", twID.Version.Info);
	strcat_s(Details2, sizeof(Details2), Details);
	wsprintf (Details, "\n%s %d", "ProtocolMajor:", twID.ProtocolMajor);
	strcat_s(Details2, sizeof(Details2), Details);
	wsprintf (Details, "\n%s %d", "ProtocolMinor:", twID.ProtocolMinor);
	strcat_s(Details2, sizeof(Details2), Details);
	wsprintf (Details, "\n%s %d", "SupportedGroups:", twID.SupportedGroups);
	strcat_s(Details2, sizeof(Details2), Details);
	wsprintf (Details, "\n%s %s", "Manufacturer:", twID.Manufacturer);
	strcat_s(Details2, sizeof(Details2), Details);
	wsprintf (Details, "\n%s %s", "ProductFamily:", twID.ProductFamily);
	strcat_s(Details2, sizeof(Details2), Details);
	wsprintf (Details, "\n%s %s", "ProductName:", twID.ProductName);
	strcat_s(Details2, sizeof(Details2), Details);

	ShowRC_CC(hWndofApp,0,0,0,Details2,text);
	return;
}

////////////////////////////////////////////////////////////////////////////
// Function: ShowCapability --
// Author: TWAIN Working Group
// Input:
//		hWndofApp - handle to window
//		Cap - 
// Output: none
// Comments:
//
void ShowCapability (HWND hWndofApp, TW_UINT16 Cap)
{
	TW_UINT16           twRC = TWRC_FAILURE;
	TW_UINT16           ccRC = TWRC_FAILURE;
	TW_CAPABILITY       twCapability;
	char                Details [255];
	char                Details2 [1024];
	TW_UINT16           index = 0;
	pTW_ENUMERATION     pEnum = NULL;
	pTW_ONEVALUE        pOneV = NULL;
	pTW_RANGE           pRange = NULL;
	pTW_ARRAY           pArray = NULL;
	LPSTR               pData = NULL;
	TW_INT16            datasize = 0;

	memset(&twCapability, 0, sizeof(TW_CAPABILITY));
	memset(Details, 0, sizeof(char[255]));
	memset(Details2, 0, sizeof(char[1024]));

	ASSERT(hWndofApp);

	twCapability.Cap = Cap;
	twCapability.ConType = TWON_DONTCARE16;

	// Check Capability information
	twRC = CallDSMEntry(&appID,
					&dsID, 
					DG_CONTROL,
					DAT_CAPABILITY, 
					MSG_GET, 
					(TW_MEMREF)&twCapability);

	//Return Code from Operation
	sprintf_s (Details, sizeof(Details), "%s %d\n", "RETURN CODE:", twRC);
	strcpy_s(Details2, sizeof(Details2), Details);

	//Show TW_CAPABILITY Contents
	MatchTwainInt(Capability, MAX_CAP, twCapability.Cap, 
								Details);
	strcat_s(Details2, sizeof(Details2), "\nCap: ");
	strcat_s(Details2, sizeof(Details2), Details);

	switch (twRC)
	{
		case TWRC_SUCCESS:

			MatchTwainInt(ConType, MAX_CONTYPE, twCapability.ConType, 
										Details);
			strcat_s(Details2, sizeof(Details2), "\nConType: ");
			strcat_s(Details2, sizeof(Details2), Details);

			//Show Container Contents
			strcat_s(Details2, sizeof(Details2), "\nhContainer.");
			switch(twCapability.ConType)
			{
				case TWON_ONEVALUE:
					pOneV = (pTW_ONEVALUE)DSM_LockMemory(twCapability.hContainer);

					strcat_s(Details2, sizeof(Details2), "\n     ItemType: ");
					MatchTwainInt(Types, MAX_TYPES, pOneV->ItemType, 
												Details);
					strcat_s(Details2, sizeof(Details2), Details);

					strcat_s(Details2, sizeof(Details2), "\n     Item: ");
					GetItem(Details, pOneV->ItemType, (LPSTR)&(pOneV->Item));
					strcat_s(Details2, sizeof(Details2), Details);

					DSM_UnlockMemory(twCapability.hContainer);
					DSM_Free((HANDLE)twCapability.hContainer);   
					break;

				case TWON_ENUMERATION:
					pEnum = (pTW_ENUMERATION)DSM_LockMemory(twCapability.hContainer);

					strcat_s(Details2, sizeof(Details2), "\n     ItemType: ");
					MatchTwainInt(Types, MAX_TYPES, pEnum->ItemType, 
												Details);

					datasize = (TW_INT16)AltTWItemSize(pEnum->ItemType);
					strcat_s(Details2, sizeof(Details2), Details);

					wsprintf (Details, "\n     %s %d", "NumItems:", pEnum->NumItems);
					strcat_s(Details2, sizeof(Details2), Details);

					wsprintf (Details, "\n     %s %d", "CurrentIndex:", pEnum->CurrentIndex);
					strcat_s(Details2, sizeof(Details2), Details);

					wsprintf (Details, "\n     %s %d", "DefaultIndex:", pEnum->DefaultIndex);
					strcat_s(Details2, sizeof(Details2), Details);

					strcat_s(Details2, sizeof(Details2), "\n     ItemList: ");
					pData = pEnum->ItemList;
					//index=0
					GetItem(Details, pEnum->ItemType, pData);
					strcat_s(Details2, sizeof(Details2), Details);
					pData+=datasize;
					for (index=1; index < pEnum->NumItems; index++)
					{
						strcat_s(Details2, sizeof(Details2), ", ");
						GetItem(Details, pEnum->ItemType, pData);
						strcat_s(Details2, sizeof(Details2), Details);
						pData+=datasize;
					}
					DSM_Free(twCapability.hContainer);   
					break;

				case TWON_RANGE:
					pRange = (pTW_RANGE)DSM_LockMemory(twCapability.hContainer);

					strcat_s(Details2, sizeof(Details2), "\n     ItemType: ");
					MatchTwainInt(Types, MAX_TYPES, pRange->ItemType, 
												Details);
					strcat_s(Details2, sizeof(Details2), Details);

					strcat_s(Details2, sizeof(Details2), "\n     MinValue: ");
					GetItem(Details, pRange->ItemType, (LPSTR)&(pRange->MinValue));
					strcat_s(Details2, sizeof(Details2), Details);

					strcat_s(Details2, sizeof(Details2), "\n     MaxValue: ");
					GetItem(Details, pRange->ItemType, (LPSTR)&(pRange->MaxValue));
					strcat_s(Details2, sizeof(Details2), Details);

					strcat_s(Details2, sizeof(Details2), "\n     StepSize: ");
					GetItem(Details, pRange->ItemType, (LPSTR)&(pRange->StepSize));
					strcat_s(Details2, sizeof(Details2), Details);

					strcat_s(Details2, sizeof(Details2), "\n     DefaultValue: ");
					GetItem(Details, pRange->ItemType, (LPSTR)&(pRange->DefaultValue));
					strcat_s(Details2, sizeof(Details2), Details);

					strcat_s(Details2, sizeof(Details2), "\n     CurrentValue: ");
					GetItem(Details, pRange->ItemType, (LPSTR)&(pRange->CurrentValue));
					strcat_s(Details2, sizeof(Details2), Details);

					DSM_Free(twCapability.hContainer);     //bad
					break;

				case TWON_ARRAY:
					pArray = (pTW_ARRAY)DSM_LockMemory(twCapability.hContainer);

					strcat_s(Details2, sizeof(Details2), "\n     ItemType: ");
					MatchTwainInt(Types, MAX_TYPES, pArray->ItemType, 
												Details);
					datasize = (TW_INT16)AltTWItemSize(pArray->ItemType);
					strcat_s(Details2, sizeof(Details2), Details);

					wsprintf (Details, "\n     %s %d", "NumItems:", pArray->NumItems);
					strcat_s(Details2, sizeof(Details2), Details);

					strcat_s(Details2, sizeof(Details2), "\n     ItemList: ");
					pData = pArray->ItemList;
					//index=0
					GetItem(Details, pArray->ItemType, pData);
					strcat_s(Details2, sizeof(Details2), Details);
					pData+=datasize;
					for (index=1; index < pArray->NumItems; index++)
					{
						strcat_s(Details2, sizeof(Details2), ", ");
						GetItem(Details, pArray->ItemType, pData);
						strcat_s(Details2, sizeof(Details2), Details);
						pData+=datasize;
					}
					DSM_Free(twCapability.hContainer);    //bad
					break;

				default:
					break;
			}
			break;

		case TWRC_FAILURE:		
			// determine details of failure from Source

			if (ccRC == TWRC_SUCCESS)
			{
				sprintf_s (Details, sizeof(Details), "\n%s %d", "Condition Code:", gGlobalStatus.ConditionCode);
				strcat_s(Details2, sizeof(Details2), Details);
			} 
			else 
			{
				sprintf_s (Details, sizeof(Details), "\n%s %d", "Condition Code:", gGlobalStatus.ConditionCode);
				strcat_s(Details2, sizeof(Details2), Details);         
			}
			break;
	}

	ShowRC_CC(hWndofApp,0,0,0,Details,"TW_CAPABILITY");
	return;
}

////////////////////////////////////////////////////////////////////////////
// Function: GetItem -- 
// Author: TWAIN Working Group
// Input:
//		test - 
//		datatype - 
//		pData - 
// Output: none
// Comments:
//
void GetItem (char * Text, TW_UINT16 datatype, LPSTR pData)
{
	ASSERT(pData);
	ASSERT(Text);

	switch (datatype)
	{
		case 0: //TWTY_INT8
			{
				TW_INT8 value = *(pTW_INT8)pData;
				wsprintf(Text, "%d", (TW_INT16)value);
			}   
			break;

		case 3: // TWTY_UINT8
			{
				TW_UINT8 value = *(pTW_UINT8)pData;
				wsprintf(Text,"%u", (TW_UINT16)value);
			}   
			break;

		case 1: // TWTY_INT16
		case 44:// TWTY_HANDLE
			{
				TW_INT16 value = *(pTW_INT16)pData;
				wsprintf(Text,"%d", value);
			}   
			break;

		case 4: // TWTY_UINT16
		case 6: // TWTY_BOOL
			{
				TW_UINT16 value = *(pTW_UINT16)pData;
				wsprintf(Text,"%u", value);
			}   
			break;

		case 2: // TWTY_INT32
			{
				TW_INT32 value = *(pTW_INT32)pData;
				wsprintf(Text,"%ld", value);
			}   
			break;

		case 5: // TWTY_UINT32
		case 43: // TWTY_MEMREF
			{
				TW_UINT32 value = *(pTW_UINT32)pData;
				wsprintf(Text,"%lu", value);
			}   
			break;

		case 7: // TWTY_FIX32
			{
				float value = 0;
				TW_FIX32 fix32;

				memset(&fix32, 0, sizeof(TW_FIX32));

				fix32.Whole = ((pTW_FIX32)pData)->Whole;
				fix32.Frac = ((pTW_FIX32)pData)->Frac;
				value = FIX32ToFloat(fix32);
				sprintf_s(Text, sizeof(Text), "%f", value);
			}   
			break;

		case 9:  //TWTY_STR32
		case 10: //TWTY_STR64
		case 11: //TWTY_STR128
		case 12: //TWTY_STR255
		default:
			strcpy_s(Text, sizeof(Text), pData);
			break;
	}   
	return;
}

///////////////////////////////////////////////////////////////////////////
// Function: ShowPendingXfers --
// Author: TWAIN Working Group
// Input:
//		hWndofApp - handle to window
// Output: none
// Comments:
//
void ShowPendingXfers (HWND hWndofApp)

{
	TW_UINT16           twRC = TWRC_FAILURE;
	TW_UINT16           ccRC = TWRC_FAILURE;
	TW_PENDINGXFERS     twPendingXfers;
	char                Details [255];
	char                Details2 [1024];

	memset(&twPendingXfers, 0, sizeof(TW_PENDINGXFERS));
	memset(Details, 0, sizeof(char[255]));
	memset(Details2, 0, sizeof(char[1024]));

	ASSERT(hWndofApp);

	// Inquire Pending Xfers
	twRC = CallDSMEntry(&appID,
				&dsID, 
				DG_CONTROL,
				DAT_PENDINGXFERS, 
				MSG_GET, 
				(TW_MEMREF)&twPendingXfers);

	// Return Code from Operation
	sprintf_s(Details2, sizeof(Details2), "%s %d\n", "RETURN CODE:", twRC);

	switch (twRC)
	{
		case TWRC_SUCCESS:
			sprintf_s (Details, sizeof(Details), "\n%s %d", "Count:", twPendingXfers.Count);
			strcat_s(Details2, sizeof(Details2), Details);
			break;

		case TWRC_FAILURE:
		default:
			// determine details of failure from SM

			if (ccRC == TWRC_SUCCESS)
			{
				sprintf_s (Details, sizeof(Details), "\n%s %d", "Condition Code:", gGlobalStatus.ConditionCode);
				strcat_s(Details2, sizeof(Details2), Details);
			}
			else
			{
				sprintf_s (Details, sizeof(Details), "\n%s %d", "Condition Code:", gGlobalStatus.ConditionCode);
				strcat_s(Details2, sizeof(Details2), Details);
			}
			break;
	}

	ShowRC_CC(hWndofApp,0,0,0,Details2,"ADF Pending Transfers");
	return;
}   

///////////////////////////////////////////////////////////////////////////
// function: ShowRC_CC -- 
// Author: TWAIN Working Group
// Input:
//		hWndofApp - handle to window
//		DisplayRC -
//		ReturnCode - 
//		Dest - 
//		Text - 
//		Title - 
// Output: none
// Comments:
//		Main message notification routine.  Displays the failure, condition
// codes and message associated with the problem.  Additionally, all log messages
// pass thru this function.
//
void ShowRC_CC (HWND        hWndofApp,
                TW_UINT16   DisplayRC,      //0-No, 1-Yes
                TW_UINT16   uiReturnCode,
                TW_UINT16   Dest,           //0-Source Manager or 1-Source 
                char *      Text,           //Additional Information        
                char *      Title)          //Triplet or Cap
{
	char            Details[255];
	char            Details2[1024];
	char outstring[1024];
	char outtitle[255];

	memset(Details, 0, sizeof(char[255]));
	memset(Details2, 0, sizeof(char[1024]));
	memset(outstring, 0, sizeof(char[1024]));
	memset(outtitle, 0, sizeof(char[255]));

	ASSERT(Text);
	ASSERT(Title);

	strcpy_s(Details, sizeof(Details), "");
	strcpy_s(Details2, sizeof(Details2), "");                         

	strcpy_s(outstring, sizeof(outstring), Text);
	strcpy_s(outtitle, sizeof(outtitle), Title);

	// if display check and display condition code
	if(DisplayRC == 1)  
	{
		//Get Return Code to be displayed, place in Details2
		MatchTwainInt(ReturnCode, MAX_RETURNCODE, (TW_INT32)uiReturnCode, 
									Details2);

		MatchTwainInt(ConditionCode,MAX_CONDITIONCODE, 
									(TW_INT32)gGlobalStatus.ConditionCode, 
									Details);

		strcpy_s(outtitle, sizeof(outtitle), "Operation Status");

		strcpy_s(outstring, sizeof(outstring), "Operation: ");
		lstrcat(outstring,Title);
		lstrcat(outstring,"\r\n");
		lstrcat(outstring,"Return Code: ");
		lstrcat(outstring,Details2);
		lstrcat(outstring,"\r\n");
		lstrcat(outstring,"Condition Code: ");
		lstrcat(outstring,Details);
	}   

	MessageBox (hWndofApp, outstring, outtitle, MB_OK);
	return;
}  

////////////////////////////////////////////////////////////////////////////
// FUNCTION: MessageLevel
//
// ARGS:    none
//
// RETURNS: current state of Source Manager (open/closed)
//
// NOTES:   Just a way to reduce the number of global vars
//
int MessageLevel (VOID)
{
	return (MessageLevelVAR);
}

////////////////////////////////////////////////////////////////////////////
// FUNCTION: SetMessageLevel
//
// ARGS:    none
//
// RETURNS: current state of Source Manager (open/closed)
//
// NOTES:   Just a way to reduce the number of global vars
//
void SetMessageLevel (int Level)
{
	MessageLevelVAR = Level;
	return;
}

/*
* Function: MsgQuerySupport 
* Author: TWAIN Working Group
* Input:
*		cap -
*		type - 
*		ptwCapability
* Output:
*		TRUE is successful
* Comments:
*
*/
BOOL MsgQuerySupport(TW_UINT16 cap,TW_UINT16 type, pTW_CAPABILITY ptwCapability)
{
	TW_INT16 rc = TWRC_FAILURE;
	char string[50];

	memset(string, 0, sizeof(char[50]));

	ASSERT(ptwCapability);

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
	* Open DSM
	*/
	if (TWOpenDSM() != TRUE)
	{
		if (MessageLevel() >= ML_ERROR)
		{
			ShowRC_CC(NULL, 0, 0, 0,"Failed to Open DSM","Capabilities");
		}
		return(FALSE);
	}
	/*
	* Open DS
	*/
	if (TWOpenDS() != TRUE)
		{
		if (MessageLevel() >= ML_ERROR)
		{
			ShowRC_CC(NULL,0,0,0,"Failed to open DS","Capabilities");
		}
		return(FALSE);
	}

	ptwCapability->Cap = cap;
	ptwCapability->ConType = TWON_DONTCARE16;
	ptwCapability->hContainer = NULL;

	rc = CallDSMEntry(&appID,
				&dsID,
				DG_CONTROL,
				DAT_CAPABILITY,
				type,
				(TW_MEMREF)ptwCapability);

	if (rc == TWRC_SUCCESS)
	{
		return(TRUE);    
	}
	else
	{      
		memset(string,'\0',sizeof(string));
		lstrcat(string,"DG_CONTROL/DAT_CAPABILITY/");
		if(type == MSG_QUERYSUPPORT)
		{
			lstrcat(string, "MSG_QUERYSUPPORT");
		}    

		/*
		* Error message using condition state of the Source
		*/
		ShowRC_CC(g_hMainDlg,1,rc,1,"",string);
		return(FALSE);
	}
}

//////////////////////////////////////////////////////////////////////////
// Function: MsgGetCapability --  
// Author: TWAIN Working Group
// Input:
//		cap -
//		type - 
//		ptwCapability
// Output:
//		TRUE is successful
// Comments:
//		Sends one of the three GET triplets to the 
// source and returns the TW_CAPABILITY structure to the caller as a 
// parameter.  If the DSM_Entry call fails, FALSE is returned 
//
BOOL MsgGetCapability(TW_UINT16 cap,TW_UINT16 type, pTW_CAPABILITY ptwCapability)
{
	TW_INT16 rc = TWRC_FAILURE;
	char string[50];

	memset(string, 0, sizeof(char[50]));

	ASSERT(ptwCapability);

	//	Setup dsID for default Source                             
	if (!TWIsDSOpen())
	{
		dsID.Id = 0;
		dsID.ProductName[0] = 0;
		appID.Id = 0;	
	}
	//	Open DSM
	if (TWOpenDSM() != TRUE)
	{
		if (MessageLevel() >= ML_ERROR)
		{
			ShowRC_CC(NULL, 0, 0, 0,"Failed to Open DSM","Capabilities");
		}
		return(FALSE);
	}
	//	Open DS
	if (TWOpenDS() != TRUE)
	{
		if (MessageLevel() >= ML_ERROR)
		{
			ShowRC_CC(NULL,0,0,0,"Failed to open DS","Capabilities");
		}
		return(FALSE);
	}

	ptwCapability->Cap = cap;
	ptwCapability->ConType = TWON_DONTCARE16;
	ptwCapability->hContainer = NULL;


	rc = CallDSMEntry(&appID,
				&dsID,
				DG_CONTROL,
				DAT_CAPABILITY,
				type,
				(TW_MEMREF)ptwCapability);

	if (rc == TWRC_SUCCESS)
	{
		return(TRUE);    
	}
	else
	{      
		memset(string,'\0',sizeof(string));
		lstrcat(string,"DG_CONTROL/DAT_CAPABILITY/");
		switch(type)
		{
			case MSG_GET:
				lstrcat(string,"MSG_GET");
				break;
			case MSG_GETDEFAULT:
				lstrcat(string,"MSG_GETDEFAULT");
				break;
			case MSG_GETCURRENT:            
				lstrcat(string,"MSG_GETCURRENT");
				break;
			default:
				break;
		}    

		// Error message using condition state of the Source
		ShowRC_CC(g_hMainDlg,1,rc,1,"",string);
		return(FALSE);
	}
}

//////////////////////////////////////////////////////////////////////////
// Function: MsgSetCapability --  
// Author: TWAIN Working Group
// Input:
//		cap -
//		type - 
//		ptwCapability
// Output:
//		TRUE is successful
// Comments:
//		Sends one of the two SET triplets to the 
// source and returns the TW_CAPABILITY structure to the caller as a 
// parameter.  If the DSM_Entry call fails, FALSE is returned. 
//
BOOL MsgSetCapability(TW_UINT16 cap,TW_UINT16 type, pTW_CAPABILITY ptwCapability)
{
	TW_INT16 rc = TWRC_FAILURE;
	char string[50];

	memset(string, 0, sizeof(char[50]));

	ASSERT(ptwCapability);

	//	Setup dsID for default Source                             
	if (!TWIsDSOpen())
	{
		dsID.Id = 0;
		dsID.ProductName[0] = 0;
		appID.Id = 0;	//Why do this?
	}
	//	Open DSM
	if (TWOpenDSM() != TRUE)
		{
		if (MessageLevel() >= ML_ERROR)
		{
			ShowRC_CC(NULL, 0, 0, 0,"Failed to Open DSM","Capabilities");
		}
		return(FALSE);
	}
	//	Open DS
	if (TWOpenDS() != TRUE)
	{
		if (MessageLevel() >= ML_ERROR)
		{
			ShowRC_CC(NULL,0,0,0,"Failed to open DS","Capabilities");
		}
		return(FALSE);
	}

	if (type == MSG_RESET)
	{
		ptwCapability->Cap = cap;
		ptwCapability->ConType = TWON_DONTCARE16;
		ptwCapability->hContainer = NULL;
	}
	if (type == MSG_RESETALL)
	{
		ptwCapability->Cap = CAP_SUPPORTEDCAPS;
		ptwCapability->ConType = TWON_DONTCARE16;
		ptwCapability->hContainer = NULL;
	}
	rc = CallDSMEntry(&appID,
				&dsID,
				DG_CONTROL,
				DAT_CAPABILITY,
				type,
				(TW_MEMREF)ptwCapability);

	if (rc == TWRC_SUCCESS)
	{
		return(TRUE);    
	}
	else
	{      
		memset(string,'\0',sizeof(string));
		lstrcat(string,"DG_CONTROL/DAT_CAPABILITY/");
		switch(type)
		{
			case MSG_SET:
				lstrcat(string,"MSG_SET");
				break;
			case MSG_RESET:
				lstrcat(string,"MSG_RESET");
				break;
			case MSG_RESETALL:
				lstrcat(string,"MSG_RESETALL");
				break;
			default:
				break;
		}    

		// Error message using condition state of the Source
		ShowRC_CC(g_hMainDlg,1,rc,1,"",string);
		return(FALSE);
	}
}