//
// File:         twa_acq.c
// Description:  Capability negotiation for Twacker
// Author:       TWAIN Working Group
// Created:      Feb 5,92
// Modified:     "
// Language:     C
// Package:      N/A
// Status:       Test tool
//
// (c) Copyright 1992, Hewlett-Packard Company, all rights reserved.
//
/*	Copyright © 1998 TWAIN Working Group: Adobe Systems Incorporated, 
*	Canon Information Systems, Eastman Kodak Company, 
*	Fujitsu Computer Products of America, Genoa Technology, 
*	Hewlett-Packard Company, Intel Corporation, Kofax Image Products, 
*	JFL Peripheral Solutions Inc., Ricoh Corporation, and Xerox Corporation.  
*	All rights reserved.
*/
//----------------------------------------------------------------------
//                            I n c l u d e s
//----------------------------------------------------------------------
#include <windows.h>            // Req. for twain.h type defs and ...
#include <memory.h>
#include <stdio.h>              // needed for wsprintf  
#include <string.h>

#if defined(WIN32) || defined(WIN64)
#else
#include <toolhelp.h>
#endif

#include "..\\twcommon\\twain.h"            // needed for TWAIN definitions
#include "..\\twcommon\\twndebug.h"

#include "table.h"
#include "dca_glue.h"    			       	// for function prototypes of glue code
#include "dca_acq.h"            			// contain TWAIN sample support code
#include "dca_app.h"
#include "captest.h"
#include "twacker.h"

TW_UINT16 Status(pTW_STATUS pStatus);

#ifdef NULL
#undef NULL                      
#endif
#define NULL 0

// to allow compatibility between 32 and 16 bit _llseek
#if !defined(WIN32) && !defined(WIN64)
#define FILE_BEGIN      0
#define FILE_CURRENT    1
#define FILE_END        2
#endif

#if defined(WIN32) || defined(WIN64)
#define TW_HUGE
#else
#define TW_HUGE huge
#endif

static TW_INT16 AcqFlag = 0;
char Filename[256];
extern BOOL g_bSpecialMenu;
//----------------------------------------------------------------------
//                            F u n c t i o n s
//----------------------------------------------------------------------
static void DoNativeTransfer(HWND hWnd);
static void DoFileTransfer(HWND hWnd);
static void DoMemTransfer(HWND hWnd);
void FlipBitMap(HWND hWnd, HANDLE hBM, TW_INT16 PixType);

/////////////////////////////////////////////////////////////////////////////
// FUNCTION: SetAcqFlag
// 
// ARGS:     
//			Flag -
// RETURNS: None
// 
// AcqFlag = 0 Do Not Accept MSG_XFERREADY
//           1 Disable/CloseDS/CloseDSM
//           2 Disable Only
//           3 Do Not Disable - only if ShowUI=TRUE 
//
void SetAcqFlag(TW_INT16 Flag)
{
	AcqFlag = Flag;
	return;
}

/////////////////////////////////////////////////////////////////////////
//  FUNCTION: TWAcquire
// 
//  ARGS:    
//		hWnd - handle to window
//		Show -
//		Flag - 
//  RETURNS: TRUE is successful
// 
//  NOTES: - opendsm, open the Source Manager
//            - opends, open the Source
//            - enable Source
//            - wait for a message from Source (usually XFERREADY)
//       Flag =  0 Do Not Accept MSG_XFERREADY
//               1 Disable/CloseDS/CloseDSM
//               2 Disable Only
//               3 Do Not Disable - only if ShowUI=TRUE
//
BOOL TWAcquire (HWND hWnd, BOOL Show, TW_INT16 Flag)
{
	TW_INT16 result = TRUE;

	ASSERT(hWnd);

	LogMessage("TWAcquire entry\r\n");

	AcqFlag = 0;    //Not ready to transfer yet
	if (TWOpenDSM () == TRUE)
	{
		// Please note that another Source may change the system default while
		// your not looking and simply getting the default will not guarentee
		// you get what you want.  Suggest saving the dsID structure privately
		// after you open it to assure subsequent connections are to your
		// intended Source -- ed
		// Also note the the DSM will maintain a static list of all Sources in
		// the system at the moment it is opened.  Changes in available Sources
		// while the DSM is open may cause unpredictable results. -- ed

		if (TWOpenDS() == TRUE)
		{
			if (TWXferMech (hWnd) == TWRC_SUCCESS)
			{
				if (TWAutofeedMenu (hWnd) == TWRC_SUCCESS)
				{
					if (!TWIsDSEnabled())
					{
						result = TWEnableDS ((TW_BOOL)Show);

						AcqFlag = Flag;
						//AcqFlag = 0 Do Not Accept MSG_XFERREADY
						//          1 Disable/CloseDS/CloseDSM
						//          2 Disable Only
						//          3 Do Not Disable - only if ShowUI=TRUE
					}
				}
			}
			else
			{
				if (TWCloseDS())
				{
					if (TWCloseDSM(NULL))
					{
						CheckSpecialMenu(hWnd, TW_APP_CLOSESM);
						AcqFlag = 0;                        
					}
					else
					{
						CheckSpecialMenu(hWnd, TW_APP_CLOSEDS);                 
					}
				}
				return(FALSE);
			}
		}
		else
		{
			LogMessage("OpenDS failed -- TWAcquire\r\n");
			TWCloseDSM(NULL);
			return(FALSE);
		}

		/*
		* Cannot Enable Source
		*/
		if (result == FALSE) 
		{    
			if (TWCloseDS())
			{
				if (TWCloseDSM(NULL))
				{
					CheckSpecialMenu(hWnd, TW_APP_CLOSESM);
					AcqFlag = 0;                        
				}
				else
				{
					CheckSpecialMenu(hWnd, TW_APP_CLOSEDS);                 
				}
			}
			else
			{
			CheckSpecialMenu(hWnd, TW_APP_DISABLE);              
			}
		} 
	}
	else
	{
		return(FALSE);
	}

	return(result);  
}   

/*
* Function: TWSetup
* Author: Nancy Létourneau / J.F.L. Peripheral Solutions / TWAIN Working Group
* Date: July 98
* Input:
*		hWnd - handle to window
*		Flag - 
* Output:
*		TRUE is successful
*Comments: 
*		- opendsm, open the Source Manager
*		- opends, open the Source
*		- enable Source
*		- wait for a message from Source (usually XFERREADY)
*		Flag =  0 Do Not Accept MSG_XFERREADY
*						1 Disable/CloseDS/CloseDSM
*						2 Disable Only
*						3 Do Not Disable - only if ShowUI=TRUE
*/
BOOL TWSetup (HWND hWnd, TW_INT16 Flag)
{
	TW_INT16 result = TRUE;

	ASSERT(hWnd);

	LogMessage("TWSetup entry\r\n");

	AcqFlag = 0;    //Not ready to transfer yet
	if (TWOpenDSM () == TRUE)
	{
		/*
		* Please note that another Source may change the system default while
		* your not looking and simply getting the default will not guarentee
		* you get what you want.  Suggest saving the dsID structure privately
		* after you open it to assure subsequent connections are to your
		* intended Source -- ed
		* Also note the the DSM will maintain a static list of all Sources in
		* the system at the moment it is opened.  Changes in available Sources
		* while the DSM is open may cause unpredictable results. -- ed
		*/

		if (TWOpenDS() == TRUE)
		{
			TW_CAPABILITY twCapability;
			pTW_ONEVALUE pOneValue = NULL;
			TW_INT16 rc = TWRC_FAILURE;

			memset(&twCapability, 0, sizeof(TW_CAPABILITY));

			twCapability.Cap = CAP_ENABLEDSUIONLY;
			twCapability.ConType = TWON_DONTCARE16;
			twCapability.hContainer = NULL;

			rc = CallDSMEntry(&appID,
						&dsID,
						DG_CONTROL,
						DAT_CAPABILITY,
						MSG_GET,
						(TW_MEMREF)&twCapability);

			/* 
			* Look if CAP_ENABLEDSUIONLY is supported. 
			* If yes, look if the value is TRUE.
			*/
			if( rc == TWRC_SUCCESS)
			{
				pOneValue = (pTW_ONEVALUE)DSM_LockMemory(twCapability.hContainer);
				if(pOneValue && pOneValue->Item == TRUE)
				{
					if (!TWIsDSEnabled())
					{
						result = TWEnableDSUIOnly();

						AcqFlag = Flag;
						/*
						* AcqFlag = 0 Do Not Accept MSG_XFERREADY
						*						1 Disable/CloseDS/CloseDSM
						*						2 Disable Only
						*/
					}
				}
				else
				{
					result = FALSE;
					ShowRC_CC(hWnd, 0, 0, 0,
								"Cannot Enable Source\nThe value of CAP_ENABLEDSUIONLY is FALSE", 
								"TWAIN Error");
				}
				DSM_UnlockMemory(twCapability.hContainer);
				DSM_Free(twCapability.hContainer);
			}
			else
			{
				result = FALSE;
				ShowRC_CC(hWnd, 0, 0, 0,
							"Cannot Enable Source\nCAP_ENABLEDSUIONLY is not supported", 
							"TWAIN Error");
			}
		}
		else
		{
			LogMessage("OpenDS failed -- TWSetup\r\n");
			TWCloseDSM(NULL);
			return(FALSE);
		}

		/*
		* Cannot Enable Source
		*/
		if (result == FALSE) 
		{    
			if (TWCloseDS())
			{
				if (TWCloseDSM(NULL))
				{
					CheckSpecialMenu(hWnd, TW_APP_CLOSESM);
					AcqFlag = 0;                        
				}
				else
				{
					CheckSpecialMenu(hWnd, TW_APP_CLOSEDS);                 
				}
			}
			else
			{
			CheckSpecialMenu(hWnd, TW_APP_DISABLE);              
			}
		} 
	}
	else
	{
		return(FALSE);
	}

	return(result);  
}   

////////////////////////////////////////////////////////////////////////////
// FUNCTION: ProcessTWMessage
//
//
// ARGS:     lpMsg  Pointer to Windows msg retrieved by GetMessage
//           hWnd   Application's main window handle
//
// RETURNS: TRUE  if application should process message as usual
//              FALSE if application should skip processing of this message
//
// NOTES:   1). be sure both Source Manager and Source are open
//              2). two way message traffic:
//                  - relay windows messages down to Source's modeless dialog
//                  - retrieve TWAIN messages from the Source
//
// COMMENT: ProcessSourceMessage is designed for applications that can only
// have one Source open at a time.  If you wish your application to have more
// than one Source open at a time please consult the TWAIN ToolKit for
// event handling instructions.
//
BOOL ProcessTWMessage(LPMSG lpMsg, HWND hWnd)
{
	TW_UINT16  twRC = TWRC_NOTDSEVENT;
	TW_EVENT   twEvent;

	memset(&twEvent, 0, sizeof(TW_EVENT));

	ASSERT(lpMsg);
	ASSERT(hWnd);
	// Only ask Source Manager to process event if there is a Source connected.

	if ((TWIsDSMOpen()) && (TWIsDSOpen()))
	{
		// A Source provides a modeless dialog box as its user interface.
		// The following call relays Windows messages down to the Source's
		// UI that were intended for its dialog box.  It also retrieves TWAIN
		// messages sent from the Source to our  Application.
		//

		twEvent.pEvent = (TW_MEMREF)lpMsg;

		twRC = CallDSMEntry(&appID,
						&dsID, 
						DG_CONTROL, 
						DAT_EVENT,
						MSG_PROCESSEVENT, 
						(TW_MEMREF)&twEvent);

		switch (twEvent.TWMessage)
		{
			case MSG_XFERREADY:
				//If AcqFlag >0 then we are in state 5
				if (AcqFlag)
				{
					TWTransferImage(hWnd);
				}
				else if (MessageLevel() >= ML_ERROR)
				{
					ShowRC_CC(hWnd, 0, 0, 0, 
								"Received while not in state 5", 
								"MSG_XFERREADY");
				}
				break;

			case MSG_CLOSEDSREQ:
        {
				  TW_UINT16 i = 0;
	        TW_STATUS stat;

          LogMessage("CloseDSReq\r\n");
				  Status(&stat);

				  /*
				  * clear the edit control
				  */
          if(TWCC_SUCCESS != stat.ConditionCode)
          {
				    for(i= 0; i < MAX_CONDITIONCODE; i++)
				    {
					    if(ConditionCode[i].ItemId == stat.ConditionCode)
					    {
                char buffer[256];
						    wsprintf(buffer,"ConditionCode = %s\r\n",ConditionCode[i].pszItemName);
                MessageBox(NULL,buffer, "Error", MB_OK);
					    }
				    }
          }
          //Fall through and close the DS
        }
			case MSG_CLOSEDSOK:
				/*
				* Disable, CloseDS, CloseDSM    
				*/
				LogMessage("CloseDSOK\r\n");
				if (TWDisableDS())
				{
					if (TWCloseDS())
					{
						if (TWCloseDSM(NULL))
						{
							// SUCCESS
							CheckSpecialMenu(hWnd, TW_APP_CLOSESM);
						}
						else
						{ 
							CheckSpecialMenu(hWnd, TW_APP_CLOSEDS);
						}
					}
					else 
					{
						CheckSpecialMenu(hWnd, TW_APP_DISABLE);
					}
				}
				break;
			
		// No message from the Source to the App break;
		// possible new message
			case MSG_NULL:
			default:
				break;
		}   
	} 
	// tell the caller what happened
	return (twRC==TWRC_DSEVENT);           // returns TRUE or FALSE
} 

/*
* Function: TWTransferImage
* Input:     
*		hWnd - Handle to the Main Window
* Output: none
* Note:
*		1). delete any bit maps laying around
*		2). mention those who do not want Native need CAP nego. ICAP_XFERMECH
*		3). get a little information about image, for form, I do not use it
*		4). set up a for form loop to pull image(s) from the Source
*		5). call for GetCompleteImage from Source
*		6). be sure to send a MSG_ENDXFER as a seperator between images
*		7). after the images are transfered I like to shut down the Source
* Comment: 
*		Setup for a transfer in the routine called as a response to
* XFERREADY.  Then has a nested loop do/while on the routine which
* actually pulls in the image or GetCompleteImage.  The GetCompleteImage
* routine also deals with the cancel, xferdone, success messages from
* Source.
*/
VOID TWTransferImage(HWND hWnd)
{
	ASSERT(hWnd);

	/* 
	* Do MSG_GET for the ICAP_XFERMECH capability when it's call in the
	* Special Menu to know the Transfer Mode
	*/
	if(g_bSpecialMenu)
	{
		TW_CAPABILITY twCapability;
		pTW_ENUMERATION pEnumeration = NULL;		
		TW_UINT16 index = 0, i = 0;
		TW_STR64 buffer;
		TW_UINT16 ModeType = 0;

		memset(buffer, 0, sizeof(TW_STR64));		
		memset(&twCapability, 0, sizeof(TW_CAPABILITY));
		twCapability.Cap = ICAP_XFERMECH;

		/*
		* Call the triplet with MSG_GET to get the current Value of 
		* ICAP_XFERMECH
		*/
		CallDSMEntry(&appID,
					&dsID,
					DG_CONTROL,
					DAT_CAPABILITY,
					MSG_GET,
					(TW_MEMREF)&twCapability);

		pEnumeration = (pTW_ENUMERATION)DSM_LockMemory(twCapability.hContainer);

		for (index = 0;index < pEnumeration->NumItems; index++)
		{
			if(index == pEnumeration->CurrentIndex)
			{
				for(i = 0; i < MAX_SETUPXFERTYPE; i++)
				{
					if(*(TW_UINT16*)&pEnumeration->ItemList[index*(sizeof(TW_UINT16))] == SetupXferType[i].ItemId)
					{
						TRACE("Value = %s.\n", SetupXferType[i].pszItemName);
						ModeType = SetupXferType[i].ItemId;
						break;
					}
				}
			}
		}
		DSM_UnlockMemory(twCapability.hContainer);
		DSM_Free(twCapability.hContainer);

		if(ModeType == TWSX_MEMORY)
		{
			DoMemTransfer(hWnd);
		}
		else if(ModeType == TWSX_FILE)
		{
			DoFileTransfer(hWnd);
		}
		else if(ModeType == TWSX_NATIVE)
		{
			DoNativeTransfer(hWnd);
		}
	}
	/*
	*	Look the check Menu in File Menu to know the Transfer Mode
	*/
	else
	{
		/*
		* Tell the Source what type of transfer you want.
		*/
		if (GetMenuState(GetMenu(hWnd), TW_APP_BUFFER, MF_BYCOMMAND) & MF_CHECKED)
		{
			DoMemTransfer(hWnd);
		}
		else if (GetMenuState(GetMenu(hWnd), TW_APP_FILE, MF_BYCOMMAND) & MF_CHECKED)
		{
			DoFileTransfer(hWnd);
		}
		else
		{
			DoNativeTransfer(hWnd);
		}
	}

	if (AcqFlag == 0)
	{
		if(!TWIsDSMOpen())
		{
			CheckSpecialMenu(hWnd, TW_APP_CLOSESM);             	
		}
	}
	else if (AcqFlag == 1)
	{
		if(!TWIsDSMOpen())
		{
			CheckSpecialMenu(hWnd, TW_APP_CLOSESM);             	
			AcqFlag = 0;                    
		}
	}
	else if (AcqFlag == 2) 
	{ 
		/*
		* Only Disable for Special...Transfer 
		*/
		if (TWDisableDS())
		{
			CheckSpecialMenu(hWnd, TW_APP_DISABLE);             
			AcqFlag = 0;
		}
	}
	else if (AcqFlag == 3)
	{ 
		/*
		* Do not Disable 
		*/
		CheckSpecialMenu(hWnd, TW_APP_TRANSFER);                        
	}
	return;
}   

/*
* Function: DoNativeTransfer -- 
* Author: TWAIN Working Group
* Input:     
*		hWnd - Handle to the Window
* Output: none
* Comment: 
*/
static void DoNativeTransfer(HWND hWnd)
{
	TW_PENDINGXFERS     twPendingXfer;
	TW_UINT16           twRC = TWRC_FAILURE;
	TW_UINT16           twRC2 = TWRC_FAILURE;
	TW_UINT32           hBitMap = NULL;
	HANDLE              hbm_acq = NULL;     // handle to bit map from Source to ret to App
	char buffer[2048];

	LPBITMAPINFOHEADER lpdib = NULL;

	memset(&twPendingXfer, 0, sizeof(TW_PENDINGXFERS));
	memset(buffer, 0, sizeof(char[2048]));

	ASSERT(hWnd);

	/*
	* Do until there are no more pending transfers
	* explicitly initialize the our flags
	*/
	twPendingXfer.Count = 0;
	do 
	{
		if (MessageLevel() >= ML_INFO)
		{
			ShowImageInfo(hWnd);
			ShowImageLayout(hWnd);
			ShowCapability(hWnd, ICAP_PIXELFLAVOR);
			ShowCapability(hWnd, ICAP_PIXELTYPE);
		}

		/*
		* Initiate Native Transfer
		*/
		twRC = CallDSMEntry(&appID,
						&dsID, 
						DG_IMAGE,
						DAT_IMAGENATIVEXFER, 
						MSG_GET, 
						(TW_MEMREF)&hBitMap);

		switch (twRC)
		{
			case TWRC_XFERDONE:  // Session is in State 7
				if (MessageLevel() >= ML_FULL)
				{
					ShowRC_CC(hWnd, 0, 0, 0, "TWRC_XFERDONE", 
							"DG_IMAGE/DAT_IMAGENATIVEXFER/MSG_GET");
				}                
				hbm_acq = (HBITMAP)hBitMap;

				/*
				* Acknowledge the end of the transfer 
				* and transition to state 6/5
				*/
				twRC2 = CallDSMEntry(&appID,
									&dsID, 
									DG_CONTROL,
									DAT_PENDINGXFERS, 
									MSG_ENDXFER,
									(TW_MEMREF)&twPendingXfer);

				if (twRC2 != TWRC_SUCCESS)
				{
					if (MessageLevel()  >= ML_ERROR)
					{
						ShowRC_CC(hWnd, 1, twRC2, 1, 
									"", 
									"DG_CONTROL / DAT_PENDINGXFERS / MSG_ENDXFER");
					}
				}

				wsprintf(buffer,"Pending Xfers = %d\r\n",twPendingXfer.Count);
				LogMessage(buffer);


				if (hbm_acq >= (HANDLE)VALID_HANDLE)
				{
					SendMessage(hWnd, PM_XFERDONE, (WPARAM)hbm_acq, 0);
				}
				else
				{
					SendMessage(hWnd, PM_XFERDONE, NULL, 0);
				}
				if (twPendingXfer.Count == 0)
				{	
					if (hbm_acq)
					{
						if(!g_bSpecialMenu)
						{
							/*
							* close the DSM and DS
							*/
							CloseConnection(NULL);
						}
					}
				}

				/* 
				* showRC_CC is a safe operation here since there will be no triplet
				* calls generated 
				*/
				if (MessageLevel() >= ML_INFO || AutoFeedOn())
				{
					wsprintf(buffer,"Images = %d",twPendingXfer.Count);
					ShowRC_CC(NULL,0,0,0,buffer,"Pending Transfers");
				}
				break;

			/*
			* the user canceled or wants to rescan the image
			* something wrong, abort the transfer and delete the image
			* pass a null ptr back to App
			*/
			case TWRC_CANCEL:   // Session is in State 7
				if (MessageLevel()  >= ML_ERROR)
				{
					ShowRC_CC(hWnd, 0, 0, 0, "TWRC_CANCEL", 
								"DG_IMAGE/DAT_IMAGENATIVEXFER/MSG_GET");                
				}

				/*
				* Source (or User) Canceled Transfer
				* transistion to state 6/5
				*/
				twRC2 = CallDSMEntry(&appID,
								&dsID, 
								DG_CONTROL,
								DAT_PENDINGXFERS, 
								MSG_ENDXFER,
								(TW_MEMREF)&twPendingXfer);

				if (twRC2 != TWRC_SUCCESS)
				{
					if (MessageLevel()  >= ML_ERROR)
					{
						ShowRC_CC(hWnd, 1, twRC2, 1, 
									"", 
									"DG_CONTROL / DAT_PENDINGXFERS / MSG_ENDXFER");             
					}
				}

				if (twPendingXfer.Count == 0)
				{
					if(!g_bSpecialMenu)
					{
						/*
						* close the DSM and DS
						*/
						CloseConnection(NULL);
					}
				}

				SendMessage(hWnd, PM_XFERDONE, NULL, 0);
				break;

			case TWRC_FAILURE:  //Session is in State 6
				/*
				* The transfer failed
				*/
				if (MessageLevel()  >= ML_ERROR)
				{
					ShowRC_CC(hWnd, 1, TWRC_FAILURE, 1, 
								"", "DG_IMAGE/DAT_IMAGENATIVEXFER/MSG_GET");                
				}

				/*
				* Abort the image
				* Enhancement: Check Condition Code and attempt recovery
				*/
				twRC2 = CallDSMEntry(&appID,
								&dsID, 
								DG_CONTROL,
								DAT_PENDINGXFERS, 
								MSG_ENDXFER,
								(TW_MEMREF)&twPendingXfer); 

				if (twRC2 != TWRC_SUCCESS)
				{
					if (MessageLevel()  >= ML_ERROR)
					{
						ShowRC_CC(hWnd, 1, twRC2, 1, 
									"", 
									"DG_CONTROL / DAT_PENDINGXFERS / MSG_ENDXFER");             
					}
				}

				if (twPendingXfer.Count == 0)
				{
					if(!g_bSpecialMenu)
					{
						/*
						* close the DSM and DS
						*/
						CloseConnection(NULL);
					}
				}

				SendMessage(hWnd, PM_XFERDONE, NULL, 0);
				break;

			default:    //Sources should never return any other RC
				if (MessageLevel()  >= ML_ERROR)
				{
					ShowRC_CC(hWnd, 0, 0, 0, "Unknown Return Code", 
								"DG_IMAGE/DAT_IMAGENATIVEXFER/MSG_GET");
				}

				/*
				* Abort the image
				* Enhancement: Check Condition Code and attempt recovery instead
				*/
				twRC2 = CallDSMEntry(&appID,
								&dsID, 
								DG_CONTROL,
								DAT_PENDINGXFERS, 
								MSG_ENDXFER,
								(TW_MEMREF)&twPendingXfer);

				if (twRC2 != TWRC_SUCCESS)
				{
					if (MessageLevel()  >= ML_ERROR)
					{
						ShowRC_CC(hWnd, 1, twRC2, 1, 
									"", 
									"DG_CONTROL / DAT_PENDINGXFERS / MSG_ENDXFER");
					}
				}

				if (twPendingXfer.Count == 0)
				{
					if(!g_bSpecialMenu)
					{
						/*
						* close the DSM and DS
						*/
						CloseConnection(NULL);
					}
				}

				SendMessage(hWnd, PM_XFERDONE, NULL, 0);
				break;
		}   

	} while (twPendingXfer.Count != 0);

	AcqFlag = 0;
	return;
}   

/*
* Function: DoFileTransfer -- 
* Author: TWAIN Working Group
* Input:     
*		hWnd - Handle to the Window
* Output: none
* Comment: 
*/
static void DoFileTransfer(HWND hWnd)
{
	TW_PENDINGXFERS     twPendingXfer;
	TW_UINT16           twRC = TWRC_FAILURE;
	TW_UINT16           twRC2 = TWRC_FAILURE;
	TW_SETUPFILEXFER    setup;
	TW_SETUPFILEXFER    SetupMsgGet;
	HANDLE              hbm_acq = NULL;     // handle to bit map from Source to ret to App
	HFILE               hFile = NULL;
	OFSTRUCT            of;
	char                buffer[2048];

	memset(&twPendingXfer, 0, sizeof(TW_PENDINGXFERS));
	memset(&setup, 0, sizeof(TW_SETUPFILEXFER));
	memset(&of, 0, sizeof(OFSTRUCT));
	memset(buffer, 0, sizeof(char[2048]));

	ASSERT(hWnd);

	/*
	* Do until there are no more pending transfers
	* explicitly initialize the our flags
	*/
	twPendingXfer.Count = 0;  
	do
	{
		if (MessageLevel() >= ML_INFO)
		{
			ShowImageInfo(hWnd);
			ShowImageLayout(hWnd);
			ShowCapability(hWnd, ICAP_PIXELFLAVOR);
			ShowCapability(hWnd, ICAP_PIXELTYPE);
		}

		/*
		* NOTE: Twacker and sample source only support BMP transfers at this
		* time - SMC 11 MAY 95
		*/

		if(g_bSpecialMenu)
		{
			CallDSMEntry(&appID,
							&dsID,
							DG_CONTROL,
							DAT_SETUPFILEXFER,
							MSG_GET,
							&SetupMsgGet);

			lstrcpy(Filename, SetupMsgGet.FileName);
		}
		lstrcpy(setup.FileName,Filename);//,sizeof(Filename));
		setup.Format = TWFF_TIFF;
		setup.VRefNum = 0;

		/*
		* create the file and close
		*/
		if ((hFile = OpenFile(Filename, &of, OF_CREATE)) == HFILE_ERROR)
		{
			twRC = TWRC_FAILURE;
			ShowRC_CC(hWnd, 0, twRC, 0, "Unable to create file for file transfer",
			"Application Error");
		}
		else
		{
			_lclose(hFile);    

			if ((twRC = CallDSMEntry(&appID,
							&dsID, 
							DG_CONTROL,
							DAT_SETUPFILEXFER, 
							MSG_SET, 
							(TW_MEMREF)&setup)) != TWRC_SUCCESS)
			{
				if (MessageLevel() >= ML_ERROR)
				{
					ShowRC_CC(hWnd, 1, twRC, 1, "", 
								"DG_CONTROL/DAT_SETUPFILEXFER/MSG_SET");                
				}
			}
			else
			{ 
				/*
				* Initiate File Transfer
				*/
				twRC = CallDSMEntry(&appID,
								&dsID, 
								DG_IMAGE,
								DAT_IMAGEFILEXFER, 
								MSG_GET, 
								(TW_MEMREF)NULL);
			}   
		}

		switch (twRC)
		{
			case TWRC_XFERDONE:
				{
					/*
					* Successful Transfer
					*/
					OFSTRUCT of;
					BITMAPFILEHEADER header;
					TW_INT16 hfile = NULL;
					TW_UINT16 num = 0x8000;
					TW_UINT32 count = 0;
					unsigned char TW_HUGE *ptr = NULL;

					memset(&of, 0, sizeof(OFSTRUCT));
					memset(&header, 0, sizeof(BITMAPFILEHEADER));

					if (MessageLevel() >= ML_FULL)
					{
						ShowRC_CC(hWnd, 0, 0, 0,
									"TWRC_XFERDONE",
									"DG_IMAGE/DAT_IMAGEFILEXFER/MSG_GET");              
					}

					/*
					* read the bitmap header and verify the transfer is a valid bmp
					* and create a handle to that bitmap
					*/
					hbm_acq = 0;
					if ((hfile = OpenFile(setup.FileName, &of, OF_READ)) != -1)
					{
						/*
						* How big is this file anyway
						*/
						DWORD dwSize = GetFileSize((HANDLE)hFile, NULL);
						
						_lread(hfile, (LPSTR)&header, sizeof(BITMAPFILEHEADER));

						/*
						* Subtract the size of the bitmapfileheader
						*/
						dwSize -= sizeof(BITMAPFILEHEADER);
						if(!header.bfSize)
						{
							/*
							*	Replace a bad value with something reasonable
							*/
							header.bfSize = dwSize;
						}

						/*
						* Allocate a DIB to hold the BMP file
						*/
						if (hbm_acq = DSM_Alloc(header.bfSize))
						{
							ptr = DSM_LockMemory(hbm_acq);
							for (count=header.bfSize-sizeof(BITMAPFILEHEADER)
											; count; count-=num, ptr+=num)
							{
								if (count < num) 
								{
									num = (TW_UINT16) count;
								}
								_lread(hfile, ptr, num);
							}   
							DSM_UnlockMemory(hbm_acq);
						}   
						_lclose(hfile);
					}   

					/*
					* Acknowledge the end of the transfer
					* and transition to state 6/5
					*/
					twRC2 = CallDSMEntry(&appID,
									&dsID, 
									DG_CONTROL,
									DAT_PENDINGXFERS, 
									MSG_ENDXFER,
									(TW_MEMREF)&twPendingXfer);

					if (twRC2 != TWRC_SUCCESS)
					{
						if (MessageLevel()  >= ML_ERROR)
						{
							ShowRC_CC(hWnd, 1, twRC2, 1, 
										"", 
										"DG_CONTROL / DAT_PENDINGXFERS / MSG_ENDXFER");             
						}
					}

					if (twPendingXfer.Count == 0)
					{
						if(!g_bSpecialMenu)
						{
							/*
							* close the DSM and DS
							*/
							CloseConnection(NULL);
						}
					}

					SendMessage(hWnd, PM_XFERDONE, (WPARAM)hbm_acq, 0);

					if (!AutoFeedOn())
					{
						AcqFlag = 1;
					}

					/*
					* showRC_CC is a safe operation here since there will be no triplet
					* calls generated 
					*/
					if (MessageLevel() >= ML_INFO || AutoFeedOn())
					{
						wsprintf(buffer,"Images = %d",twPendingXfer.Count);
						ShowRC_CC(NULL,0,0,0,buffer,"Pending Transfers");
					}
				}   
				break;

			/*
			* the user canceled or wants to rescan the image
			* something wrong, abort the transfer and delete the image
			* pass a null ptr back to App
			*/
			case TWRC_CANCEL:
				/*
				* The Source is in state 7
				* transistion to state 6/5
				*/
				if (MessageLevel()  >= ML_ERROR)
				{
					ShowRC_CC(hWnd, 0, 0, 0,
								"TWRC_CANCEL",
								"DG_IMAGE/DAT_IMAGEFILEXFER/MSG_GET");              
				}

				twRC2 = CallDSMEntry(&appID,
								&dsID, 
								DG_CONTROL,
								DAT_PENDINGXFERS, 
								MSG_ENDXFER,
								(TW_MEMREF)&twPendingXfer); 

				if (twRC2 != TWRC_SUCCESS)
				{
					if (MessageLevel()  >= ML_ERROR)
					{
						ShowRC_CC(hWnd, 1, twRC2, 1, 
									"", 
									"DG_CONTROL / DAT_PENDINGXFERS / MSG_ENDXFER");             
					}
				}

				if (twPendingXfer.Count == 0)
				{
					if(!g_bSpecialMenu)
					{
						/*
						* close the DSM and DS
						*/
						CloseConnection(NULL);
					}
				}

				SendMessage(hWnd, PM_XFERDONE, NULL, 0);
				break;

			case TWRC_FAILURE:
				/*
				* The transfer failed
				* Determine Condition Code
				*/
				if (MessageLevel() >= ML_ERROR) 
				{
					ShowRC_CC(hWnd, 1, TWRC_FAILURE, 1,
								"", "DG_IMAGE/DAT_IMAGEFILEXFER/MSG_GET");              
				}
				/*
				* Abort the image
				* Enhancement: Check Condition Code and attempt recovery instead
				*/
				twRC2 = CallDSMEntry(&appID,
								&dsID, 
								DG_CONTROL,
								DAT_PENDINGXFERS, 
								MSG_ENDXFER,
								(TW_MEMREF)&twPendingXfer); 

				if (twRC2 != TWRC_SUCCESS)
				{
					if (MessageLevel()  >= ML_ERROR)
					{
						ShowRC_CC(hWnd, 1, twRC2, 1, 
									"", 
									"DG_CONTROL / DAT_PENDINGXFERS / MSG_ENDXFER"); 
					}
				}

				if (twPendingXfer.Count == 0)
				{
					if(!g_bSpecialMenu)
					{
						/*
						* close the DSM and DS
						*/
						CloseConnection(NULL);
					}
				}

				SendMessage(hWnd, PM_XFERDONE, NULL, 0);
				break;

			default:
				if (MessageLevel() >= ML_ERROR)
				{
					ShowRC_CC(hWnd, 0, 0, 0,
								"Unknown Failure",
								"DG_IMAGE/DAT_IMAGEFILEXFER/MSG_GET");              
				}

				/*
				* Abort the image
				*/
				twRC2 = CallDSMEntry(&appID,
								&dsID, 
								DG_CONTROL,
								DAT_PENDINGXFERS, 
								MSG_ENDXFER,
								(TW_MEMREF)&twPendingXfer);

				if (twRC2 != TWRC_SUCCESS)
				{
					if (MessageLevel()  >= ML_ERROR)
					{
						ShowRC_CC(hWnd, 1, twRC2, 1, 
									"", 
									"DG_CONTROL / DAT_PENDINGXFERS / MSG_ENDXFER"); 
					}
				}

				if (twPendingXfer.Count == 0)
				{
					if(!g_bSpecialMenu)
					{
						/*
						* close the DSM and DS
						*/
						CloseConnection(NULL);
					}
				}

				SendMessage(hWnd, PM_XFERDONE, NULL, 0);

				/*
				* showRC_CC is a safe operation here since there will be no triplet
				* calls generated 
				*/
				if (MessageLevel() >= ML_INFO || AutoFeedOn())
				{
					wsprintf(buffer,"Images = %d",twPendingXfer.Count);
					ShowRC_CC(NULL,0,0,0,buffer,"Pending Transfers");
				}
				break;
		}   

	} while (twPendingXfer.Count != 0  && twRC != TWRC_FAILURE);
    
	return;
}   

/*
* Function: DoMemTransfer -- 
* Author: TWAIN Working Group
* Input:     
*		hWnd - Handle to the Window
* Output: none
* Comment: 
*/
static void DoMemTransfer(HWND hWnd)
{
	TW_PENDINGXFERS			twPendingXfer;
	TW_UINT16           twRC2 = TWRC_FAILURE;
	TW_UINT16           twRC = TWRC_FAILURE;
	HANDLE              hbm_acq = NULL;     // handle to bit map from Source to ret to App
	TW_IMAGEMEMXFER     xfer;
	TW_SETUPMEMXFER     setup;
	TW_IMAGEINFO        info;
	TW_PALETTE8         pal;
	LPBITMAPINFO        pdib = NULL;
	unsigned char TW_HUGE  *ptr = NULL;
	TW_UINT16           index = 0;
	TW_UINT32           size = 0;
	TW_CAPABILITY       cap;
	TW_UINT16           PixelFlavor = 0;   
	pTW_ONEVALUE        pOneV = NULL;
	TW_UINT16           Units = 0;
	float               XRes = 0, YRes = 0;
	HCURSOR             hWait = NULL;
	HCURSOR             hReady = NULL;
	char                buffer[2048];
	int                 blocks = 0;

	memset(&twPendingXfer, 0, sizeof(TW_PENDINGXFERS));
	memset(&xfer, 0, sizeof(TW_IMAGEMEMXFER));
	memset(&setup, 0, sizeof(TW_SETUPMEMXFER));
	memset(&info, 0, sizeof(TW_IMAGEINFO));
	memset(&pal, 0, sizeof(TW_PALETTE8));
	memset(&cap, 0, sizeof(TW_CAPABILITY));
	memset(buffer, 0, sizeof(char[2048]));

	ASSERT(hWnd);

	LogMessage("DoMemTransfer\r\n");
	/*
	* set the cursor to wait as a memory transfer may take a long time based on
	* the size of the transfered chunks.  Smaller buffers will produce very slow
	* transfers, especially when thunking
	*/
	hWait = LoadCursor(NULL, IDC_WAIT);
	hReady = LoadCursor(NULL, IDC_ARROW);
	SetCursor(hWait);

	/*
	* Do until there are no more pending transfers
	* explicitly initialize the our flags
	*/
	twPendingXfer.Count = 0;
	do
	{
		if (MessageLevel() >= ML_INFO)
		{
			ShowImageInfo(hWnd);
			ShowImageLayout(hWnd);
			ShowCapability(hWnd, ICAP_PIXELFLAVOR);
			ShowCapability(hWnd, ICAP_PIXELTYPE);
		} 
        
		twRC = CallDSMEntry(&appID,
						&dsID, 
						DG_IMAGE, 
						DAT_IMAGEINFO,
						MSG_GET, 
						(TW_MEMREF)&info);

		if (twRC != TWRC_SUCCESS)
		{  
			if (MessageLevel() >= ML_ERROR)
			{
				ShowRC_CC(hWnd, 1, twRC, 1,
							"Memory Transfer",
							"DG_IMAGE/DAT_IMAGEINFO/MSG_GET");              
			}
		}
		else
		{  
			/*
			* Limited to 256 colors in Palette
			*/
			size = (((((TW_INT32)info.ImageWidth*info.BitsPerPixel+31)/32)*4)
							* info.ImageLength);

			/*
			* make the size an integral of the preferred transfer size
			*/
			twRC = CallDSMEntry(&appID,
							&dsID, 
							DG_CONTROL,
							DAT_SETUPMEMXFER, 
							MSG_GET, 
							(TW_MEMREF)&setup);

			blocks = (int)(size / setup.Preferred);
			size = (blocks +1) * setup.Preferred;

			hbm_acq = DSM_Alloc(
			size + sizeof(BITMAPINFOHEADER) + 256*sizeof(RGBQUAD));

			if (hbm_acq == NULL)
			{   
				/*
				* GlobalAlloc Failed
				*/
				if (MessageLevel() >= ML_ERROR)
				{
					ShowRC_CC(hWnd, 0, 0, 0,
								"GlobalAlloc Failed in DoMemTransfer",
								"Memory Error");
				}
			}
			else
			{
				/*
				* Lock the Memory
				*/
				pdib = (LPBITMAPINFO)DSM_LockMemory(hbm_acq);

				/*
				* fill in the image information 
				*/
				pdib->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
				pdib->bmiHeader.biWidth = info.ImageWidth;
				pdib->bmiHeader.biHeight = info.ImageLength;
				/*
				* Only 1 is supported
				*/
				pdib->bmiHeader.biPlanes = 1;
				pdib->bmiHeader.biBitCount = info.BitsPerPixel;
				/*
				* This application does not support compression
				*/
				pdib->bmiHeader.biCompression = BI_RGB;
				pdib->bmiHeader.biSizeImage = size;

				/*
				* Get Units and calculate PelsPerMeter
				*/
				cap.Cap = ICAP_UNITS;
				cap.ConType = TWON_DONTCARE16;
				cap.hContainer = NULL;
                
				twRC = CallDSMEntry(&appID,
								&dsID, 
								DG_CONTROL, 
								DAT_CAPABILITY, 
								MSG_GETCURRENT, 
								(TW_MEMREF)&cap);

				if (twRC != TWRC_SUCCESS)
				{
					if (MessageLevel() >= ML_ERROR)
					{
						ShowRC_CC(hWnd, 1, twRC, 1,
									"ICAP_UNITS",
									"DG_CONTROL/DAT_CAPABILITY/MSG_GETCURRENT");
					}

					pdib->bmiHeader.biXPelsPerMeter = 0;
					pdib->bmiHeader.biYPelsPerMeter = 0;
				}
				else
				{
					pOneV = (pTW_ONEVALUE)DSM_LockMemory(cap.hContainer);
					Units = (TW_UINT16)(pOneV->Item);
					DSM_UnlockMemory(cap.hContainer);
					DSM_Free((HANDLE)cap.hContainer);

					XRes = FIX32ToFloat(info.XResolution);
					YRes = FIX32ToFloat(info.YResolution);
                    
					switch(Units)
					{
						case TWUN_INCHES:
							pdib->bmiHeader.biXPelsPerMeter = (LONG)((XRes*2.54)*100);
							pdib->bmiHeader.biYPelsPerMeter = (LONG)((YRes*2.54)*100);
							break;
						case TWUN_CENTIMETERS:
							pdib->bmiHeader.biXPelsPerMeter = (LONG)(XRes*100);
							pdib->bmiHeader.biYPelsPerMeter = (LONG)(YRes*100);
							break;
						case TWUN_PICAS:
						case TWUN_POINTS:
						case TWUN_TWIPS:
						case TWUN_PIXELS:
						default:
							pdib->bmiHeader.biXPelsPerMeter = 0;
							pdib->bmiHeader.biYPelsPerMeter = 0;
							break;
					}
				}
                
				/*
				* Setup Palette -- if the palettes are B/W or shades of gray, the color
				* table is built here.  If the image is 8 bit color, a call to the
				* source is made to retrieve the correct set of colors.  If the call 
				* fails, the color table is constructed with 256 shades of gray inorder
				* to provide some image reference
				*/
				switch (info.PixelType)
				{
					case TWPT_BW:
						pdib->bmiHeader.biClrUsed = 2;
						pdib->bmiHeader.biClrImportant = 0;
                                                       
						/*
						* Get CAP_PIXELFLAVOR to determine colors
						* fill in the palette information
						*/
						cap.Cap = ICAP_PIXELFLAVOR;
						cap.ConType = TWON_DONTCARE16;
						cap.hContainer = NULL;

						twRC = CallDSMEntry(&appID,
										&dsID, 
										DG_CONTROL, 
										DAT_CAPABILITY, 
										MSG_GETCURRENT, 
										(TW_MEMREF)&cap);

						if (twRC != TWRC_SUCCESS)
						{   
							if (MessageLevel() >= ML_INFO)
							{
								ShowRC_CC(hWnd, 1, twRC, 1,
											"MSG_GET",
											"ICAP_PIXELFLAVOR");
							}
							PixelFlavor = TWPF_CHOCOLATE;
						}
						else
						{
							if (cap.ConType != TWON_ONEVALUE)
							{
								PixelFlavor = TWPF_CHOCOLATE;
							}
							else
							{                            
								pOneV = (pTW_ONEVALUE)DSM_LockMemory(cap.hContainer);
								PixelFlavor = (TW_UINT16)(pOneV->Item);
								DSM_UnlockMemory(cap.hContainer);
							}                       
							DSM_Free((HANDLE)cap.hContainer);
						}
						if (PixelFlavor == 0)
						{
							/*
							* 0=Black
							*/
							pdib->bmiColors[0].rgbRed = 0x0000;
							pdib->bmiColors[0].rgbGreen = 0x0000;
							pdib->bmiColors[0].rgbBlue = 0x0000;
							pdib->bmiColors[0].rgbReserved = 0;

							pdib->bmiColors[1].rgbRed = 0x00FF;
							pdib->bmiColors[1].rgbGreen = 0x00FF;
							pdib->bmiColors[1].rgbBlue = 0x00FF;
							pdib->bmiColors[1].rgbReserved = 0;
						}
						else
						{
							/*
							* ICAP_PIXELFLAVOR == 1
							* 0=White
							*/
							pdib->bmiColors[0].rgbRed = 0x00FF;
							pdib->bmiColors[0].rgbGreen = 0x00FF;
							pdib->bmiColors[0].rgbBlue = 0x00FF;
							pdib->bmiColors[0].rgbReserved = 0;

							pdib->bmiColors[1].rgbRed = 0x0000;
							pdib->bmiColors[1].rgbGreen = 0x0000;
							pdib->bmiColors[1].rgbBlue = 0x0000;
							pdib->bmiColors[1].rgbReserved = 0;
						}
						break;
                    
					case TWPT_GRAY:
						pdib->bmiHeader.biClrUsed = 1 << pdib->bmiHeader.biBitCount;
						{
							int iIncr = 0xFF / ( pdib->bmiHeader.biClrUsed - 1 );
							int iVal = 0;
							for (index=0; index<pdib->bmiHeader.biClrUsed; index++)
							{
								if(PixelFlavor == TWPF_CHOCOLATE)
								{
									iVal = index * iIncr;
								}
								else
								{
									iVal = 0xFF - index* iIncr; 
								}
								pdib->bmiColors[index].rgbRed = (BYTE) iVal;
								pdib->bmiColors[index].rgbGreen = (BYTE) iVal;
								pdib->bmiColors[index].rgbBlue = (BYTE) iVal;
								pdib->bmiColors[index].rgbReserved = 0;
							}
						}
						break;

					case TWPT_RGB:
						pdib->bmiHeader.biClrUsed = 0;
						break;
                    
					case TWPT_PALETTE:
					case TWPT_CMY:
					case TWPT_CMYK:
					case TWPT_YUV:
					case TWPT_YUVK:
					case TWPT_CIEXYZ:
					default:
						/*
						* fill in the palette information
						*/
						twRC = CallDSMEntry(&appID,
										&dsID, 
										DG_IMAGE, 
										DAT_PALETTE8, 
										MSG_GET, 
										(TW_MEMREF)&pal);

						if (twRC != TWRC_SUCCESS)
						{
							if (MessageLevel() >= ML_ERROR)
							{
								ShowRC_CC(hWnd, 1, twRC, 1,
											"",
											"DG_IMAGE/DAT_PALETTE8/MSG_GET -- defaulting to 256 gray image palette");
							}

							pdib->bmiHeader.biClrImportant = 0;
							pdib->bmiHeader.biClrUsed = 256;
							for (index=0; index<pal.NumColors; index++)
							{
								pdib->bmiColors[index].rgbRed = (BYTE)index;
								pdib->bmiColors[index].rgbGreen = (BYTE)index;
								pdib->bmiColors[index].rgbBlue = (BYTE)index;
								pdib->bmiColors[index].rgbReserved = 0;
							}   
						}
						else
						{
							pdib->bmiHeader.biClrUsed = pal.NumColors;
							pdib->bmiHeader.biClrImportant = 0;
							for (index=0; index<pal.NumColors; index++)
							{
								pdib->bmiColors[index].rgbRed = pal.Colors[index].Channel1;
								pdib->bmiColors[index].rgbGreen = pal.Colors[index].Channel2;
								pdib->bmiColors[index].rgbBlue = pal.Colors[index].Channel3;
								pdib->bmiColors[index].rgbReserved = 0;
							}   
						}
						break;              
				}   //end switch(PixelType)

				/*
				* locate the start of the bitmap data 
				*/
				ptr = (unsigned char TW_HUGE *) pdib;
				ptr += sizeof(BITMAPINFOHEADER);
				ptr += pdib->bmiHeader.biClrUsed * sizeof(RGBQUAD);

				/*
				* determine the buffer size 
				*/
				twRC = CallDSMEntry(&appID,
								&dsID, 
								DG_CONTROL,
								DAT_SETUPMEMXFER, 
								MSG_GET, 
								(TW_MEMREF)&setup);

				if (twRC != TWRC_SUCCESS)
				{
					if (MessageLevel() >= ML_ERROR)
					{
						ShowRC_CC(hWnd, 1, twRC, 1,
									"",
									"DG_CONTROL/DAT_SETUPMEMXFER/MSG_GET");
					}
				}
				else
				{
					/*
					* we will use a pointer to shared memory
					*/
					xfer.Memory.Flags = TWMF_APPOWNS | TWMF_POINTER;
					xfer.Memory.Length = setup.Preferred;
					xfer.Memory.TheMem = ptr;

					/*
					* transfer the data -- loop until done or canceled 
					*/
					do
					{
						twRC = CallDSMEntry(&appID,
										&dsID, 
										DG_IMAGE,
										DAT_IMAGEMEMXFER, 
										MSG_GET, 
										(TW_MEMREF)&xfer);

						switch (twRC)
						{
							case TWRC_SUCCESS:
								ptr += xfer.BytesWritten;
								xfer.Memory.TheMem = ptr;
								break;
    
							case TWRC_XFERDONE:
								/*
								* Successful Transfer
								*/
								if (MessageLevel() >= ML_FULL)
								{
									ShowRC_CC(hWnd, 0, 0, 0,
												"TWRC_XFERDONE",
												"DG_IMAGE/DAT_IMAGEMEMXFER/MSG_GET");               
								}
								DSM_UnlockMemory(hbm_acq);

								/*
								* since the sample source is reading a DIB, the bitmap
								* is already right side up.  A memory transfer from a real
								* source will be the reverse and needs to be flipped
								*/
								FlipBitMap(hWnd, hbm_acq, info.PixelType);

								SetCursor(hReady);                              

								/*
								* Acknowledge the end of the transfer
								* and transition to state 6/5
								*/
								twRC2 = CallDSMEntry(&appID,
												&dsID, 
												DG_CONTROL,
												DAT_PENDINGXFERS, 
												MSG_ENDXFER,
												(TW_MEMREF)&twPendingXfer);

								if (twRC2 != TWRC_SUCCESS)
								{
									if (MessageLevel()  >= ML_ERROR)
									{
										ShowRC_CC(hWnd, 1, twRC2, 1, 
													"", 
													"DG_CONTROL / DAT_PENDINGXFERS / MSG_ENDXFER");             
									}
								}

								if (twPendingXfer.Count == 0)
								{
									if(!g_bSpecialMenu)
									{
										/*
										* close the DSM and DS
										*/
										CloseConnection(NULL);
									}
								}

								SendMessage(hWnd, PM_XFERDONE, (WPARAM)hbm_acq, 0);

								/*
								* showRC_CC is a safe operation here since there will be no triplet
								* calls generated 
								*/
								if (MessageLevel() >= ML_INFO || AutoFeedOn())
								{
									wsprintf(buffer,"Images = %d",twPendingXfer.Count);
									ShowRC_CC(NULL,0,0,0,buffer,"Pending Transfers");
								}
								break;
    
							case TWRC_CANCEL:
								SetCursor(hReady);

								/*
								* The Source is in state 7
								* transistion to state 6/5
								*/
								if (MessageLevel() >= ML_ERROR)
								{
									ShowRC_CC(hWnd, 0, 0, 0,
												"TWRC_CANCEL",
												"DG_IMAGE/DAT_IMAGEMEMXFER/MSG_GET");               
								}

								twRC2 = CallDSMEntry(&appID,
											&dsID, 
											DG_CONTROL,
											DAT_PENDINGXFERS, 
											MSG_ENDXFER,
											(TW_MEMREF)&twPendingXfer);

								if (twRC2 != TWRC_SUCCESS)
								{
									if (MessageLevel()  >= ML_ERROR)
									{
										ShowRC_CC(hWnd, 1, twRC2, 1, 
													"", 
													"DG_CONTROL / DAT_PENDINGXFERS / MSG_ENDXFER");             
									}
								}
        
								DSM_UnlockMemory(hbm_acq);
								DSM_Free(hbm_acq);

								if (twPendingXfer.Count == 0)
								{
									if(!g_bSpecialMenu)
									{
										/*
										* close the DSM and DS
										*/
										CloseConnection(NULL);
									}
								}

								SendMessage(hWnd, PM_XFERDONE, NULL, 0);
								break;
                            
							case TWRC_FAILURE:
								SetCursor(hReady);

								/*
								* The transfer failed
								* Enhancement: Check Condition Code and attempt recovery
								*/
								if (MessageLevel() >= ML_ERROR)
								{
									ShowRC_CC(hWnd, 1, TWRC_FAILURE, 1,
												"",
												"DG_IMAGE/DAT_IMAGEMEMXFER/MSG_GET");               
								}

								twRC2 = CallDSMEntry(&appID,
												&dsID, 
												DG_CONTROL,
												DAT_PENDINGXFERS, 
												MSG_ENDXFER,
												(TW_MEMREF)&twPendingXfer);

								if (twRC2 != TWRC_SUCCESS)
								{
									if (MessageLevel()  >= ML_ERROR)
									{
										ShowRC_CC(hWnd, 1, twRC2, 1, 
													"", 
													"DG_CONTROL / DAT_PENDINGXFERS / MSG_ENDXFER");             
									}
								}

								DSM_UnlockMemory(hbm_acq);
								DSM_Free(hbm_acq);

								if (twPendingXfer.Count == 0)               
								{
									if(!g_bSpecialMenu)
									{
										/*
										* close the DSM and DS
										*/
										CloseConnection(NULL);
									}
								}

								SendMessage(hWnd, PM_XFERDONE, NULL, 0);
								break;
                            
							default:
								SetCursor(hReady);
								if (MessageLevel() >= ML_ERROR)
								{
									ShowRC_CC(hWnd, 0, 0, 0,
												"Unknown Return Code",
												"DG_IMAGE/DAT_IMAGEMEMXFER/MSG_GET");               
								}
								/*
								* Abort the image
								*/
								twRC2 = CallDSMEntry(&appID,
												&dsID, 
												DG_CONTROL,
												DAT_PENDINGXFERS, 
												MSG_ENDXFER,
												(TW_MEMREF)&twPendingXfer);

								if (twRC2 != TWRC_SUCCESS)
								{
									if (MessageLevel()  >= ML_ERROR)
									{
										ShowRC_CC(hWnd, 1, twRC2, 1, 
													"", 
													"DG_CONTROL / DAT_PENDINGXFERS / MSG_ENDXFER");             
									}
								}

								DSM_UnlockMemory(hbm_acq);
								DSM_Free(hbm_acq);

								if (twPendingXfer.Count == 0)
								{
									if(!g_bSpecialMenu)
									{
										/*
										* close the DSM and DS
										*/
										CloseConnection(NULL);
									}
								}

								SendMessage(hWnd, PM_XFERDONE, NULL, 0);

								/*
								* showRC_CC is a safe operation here since there will be no triplet
								* calls generated 
								*/
								if (MessageLevel() >= ML_INFO || AutoFeedOn())
								{
									wsprintf(buffer,"Images = %d",twPendingXfer.Count);
									ShowRC_CC(NULL,0,0,0,buffer,"Pending Transfers");
								}
								break;
						} // switch
					} while (twRC == TWRC_SUCCESS);
				} 
			} 
		}  
	} while (twPendingXfer.Count != 0);

	return;
}   

///////////////////////////////////////////////////////////////////////////////
// FUNCTION: LogMessage -- 
//
// ARGS:     msg
//
// RETURNS: none
//
// NOTES:
//
// COMMENTS: 
//	Writes debug strings to log file at c:\twacker.log.  Only
// runs when code is compiled in debug mode. 
//
void LogMessage(char msg[])
{ 
	#ifdef _DEBUG

	HFILE file = NULL;
	OFSTRUCT of;

	memset(&of, 0, sizeof(OFSTRUCT));

	if (OpenFile("c:\\twacker.log",&of,OF_EXIST)==HFILE_ERROR)
	{
		file = OpenFile("c:\\twacker.log",&of,OF_CREATE|OF_WRITE|OF_SHARE_DENY_NONE);
	}
	else
	{
		file = OpenFile("c:\\twacker.log",&of,OF_WRITE|OF_SHARE_DENY_NONE);
	}

	if (_llseek(file,0,FILE_END)==-1)
	{
		OutputDebugString("LogMessage _llseek failed\n");
	}

	if (_lwrite(file,msg,lstrlen(msg))==-1)
	{
		OutputDebugString("LogMessage _lwrite failed\n");
	}

	if (_lclose(file)==-1) 
	{
		OutputDebugString("LogMessage _lclose failed\n");
	}

	#endif
	return;
}

//////////////////////////////////////////////////////////////////////
// FUNCTION: CloseConnection -- 
//
// ARGS:     hWnd
//
// RETURNS: none
//
// NOTES:
//
// COMMENTS: 
//		Disables the data source UI, closes the data source,
// and closes the DSM
//
void CloseConnection(HANDLE bitmap)
{	
	if (TWDisableDS())
	{
		if (TWCloseDS())
		{
			if (TWCloseDSM(bitmap))
			{
				//SUCCESS
				//CheckSpecialMenu(hWnd, TW_APP_CLOSESM);
				AcqFlag = 0;                    
			}
			else
			{
				//CheckSpecialMenu(hWnd, TW_APP_CLOSEDS);                 
				AcqFlag = 0;                    
			}
		}
		else
		{
			AcqFlag = 0;                                
			//CheckSpecialMenu(hWnd, TW_APP_DISABLE);             
		} 
	}
	return;
}

//////////////////////////////////////////////////////////////////////////////
// FUNCTION: FlipBitMap
//
// ARGS:     
//		hwnd - 
//		hBM -
//		PixType -
//
// RETURNS: none
//
// NOTES:
//
// COMMENTS: 
//		Takes a memory transfer buffer and changes it to a DIB format
//
//    i.e.    Memory Format
//                         1  2  3  4  5
//                         6  7  8  9 10
//                        11 12 13 14 15
//
//            DIB bitmap Format
//                        11 12 13 14 15
//                         6  7  8  9 10
//                         1  2  3  4  5                      
//
//    Memory RGBQuad order: RGB
//    Windows DIB RGBQuad order: BGR
//        
void FlipBitMap(HWND hWnd, HANDLE hBM, TW_INT16 PixType)

{
	HANDLE                  temp;
	LPBITMAPINFO            pdib;
	BYTE*                   pDib;
	unsigned char TW_HUGE   *pbuffer;
	unsigned char TW_HUGE   *tempptr;
	unsigned char TW_HUGE   *tempptrsave;
	LONG                    Width;
	LONG                    Height;
	LONG                    Linelength;
	LONG                    indexH;
	DWORD                   Size;
	DWORD                   SizeImage;
	WORD                    BitCount;
	DWORD                   ClrUsed;   
	DWORD                   offset;
	TW_UINT16               pixels;
	TW_INT32                items;
	TW_UINT32               i;
	BYTE                    SaveRed;
	BYTE                    SaveBlue;

	pDib = (BYTE*)DSM_LockMemory(hBM);
	pdib = (LPBITMAPINFO)pDib;

	Width = pdib->bmiHeader.biWidth;
	Height = pdib->bmiHeader.biHeight;
	Size = pdib->bmiHeader.biSize;
	SizeImage = pdib->bmiHeader.biSizeImage;
	BitCount = pdib->bmiHeader.biBitCount;
	ClrUsed = pdib->bmiHeader.biClrUsed;

	if (temp = DSM_Alloc( SizeImage))
	{
		tempptr = DSM_LockMemory(temp);
		tempptrsave = tempptr;

		// calculate offset to start of the bitmap data
		offset = sizeof(BITMAPINFOHEADER);
		offset += pdib->bmiHeader.biClrUsed * sizeof(RGBQUAD);

		Linelength = (((Width*BitCount+31)/32)*4);

		//Goto Last line in bitmap
		offset += (Linelength * (Height-1));

		#if defined(WIN32) || defined(WIN64)
			pDib = pDib + offset - Linelength;
		#endif

		//For each line
		for (indexH = 1; indexH < Height; indexH++)
		{
      #if defined(WIN32) || defined(WIN64)
				memcpy(tempptr, pDib, Linelength);
				pDib -= (Linelength);
			#else
				MemoryRead(hBM, offset, tempptr, Linelength);
				offset -= (Linelength);
			#endif
				tempptr += Linelength;
		}

		// Copy temp over hBM
		pbuffer = (unsigned char TW_HUGE *) pdib;
		pbuffer += sizeof(BITMAPINFOHEADER);
		pbuffer += pdib->bmiHeader.biClrUsed * sizeof(RGBQUAD);

    #if defined(WIN32) || defined(WIN64)
			memcpy(pbuffer, tempptrsave, SizeImage);
		#else
			MemoryRead(temp, 0, pbuffer, SizeImage);
		#endif

		//Flip RGB color table
		if (PixType == TWPT_RGB)
		{
			pbuffer = (unsigned char TW_HUGE *)pdib;
			pbuffer += sizeof(BITMAPINFOHEADER);
			pbuffer += pdib->bmiHeader.biClrUsed * sizeof(RGBQUAD);

			pixels = (TW_UINT16)pdib->bmiHeader.biWidth;
			for (items = 0; items < Height; items++)
			{
				tempptr = pbuffer;
				for (i=0; i<pixels; i++)
				{
					//Switch Red byte and Blue byte
					SaveRed = (BYTE)*tempptr;
					SaveBlue = (BYTE)*(tempptr+2);
					(BYTE)*tempptr = SaveBlue;
					(BYTE)*(tempptr+2) = SaveRed;
					//increment to next triplet
					tempptr += 3;
				}
				pbuffer += Linelength;
			}
		} 

		//Unlock
		DSM_UnlockMemory(hBM);
		DSM_UnlockMemory(temp);

		//Free
		DSM_Free(temp);
	}
	else
	{
		if (MessageLevel() >= ML_ERROR)
		{
			ShowRC_CC(hWnd, 0, 0, 0,
			"Could not allocate enough memory to flip image", 
			"Error in FixBitMap in TWA_ACQ.C");
		}
		DSM_UnlockMemory(hBM);
	}
	return;
}
