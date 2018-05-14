/* 
* File: twndebug.c
* Company: JFL Peripheral Solutions Inc.
* Date: March 1998
* Descripton:
*		implementation of debug instrumentation functions
*
*	Copyright © 1998 TWAIN Working Group: Adobe Systems Incorporated, 
*	Canon Information Systems, Eastman Kodak Company, 
*	Fujitsu Computer Products of America, Genoa Technology, 
*	Hewlett-Packard Company, Intel Corporation, Kofax Image Products, 
*	JFL Peripheral Solutions Inc., Ricoh Corporation, and Xerox Corporation.  
*	All rights reserved.
*/

/*
*	Remove entire module during release build
*/
#ifdef _DEBUG

/*
*	standard windows includes
*/
#include <windows.h>
#include <stdio.h>
#include <stdarg.h>

/*
*	function and macro definitions
*/
#include "twndebug.h"

/*
*	Maximum size of any string used for debugging
*/
#define MAXDEBUGSTRING 512

/*
* Function: TriggerAssertionFailed
* Inputs:
*		lpszFileName - contains name of file where assertion was triggered
*		nLine - containes the line index where assertion was triggered
* Outputs:
*		FALSE if the user chose to ignore the warning
* Comments:
*/
BOOL TriggerAssertionFailed(LPCSTR lpszFileName, int nLine)
{
	/*
	* Initialized locals, allocate string memory
	* (string is not allocated on the stack to reduce possible impact)
	*/
	int nCode = IDIGNORE;
	char *pszDebugString = malloc(MAXDEBUGSTRING);

	/*
	*	Ensure string is valid (don't assert...)
	*/
	if(pszDebugString)
	{
		/*
		*	Prepare and send debug window output message
		*/
		wsprintf(pszDebugString, "Assertion Failed. Line %d, File %s.\n", nLine, lpszFileName);
		OutputDebugString(pszDebugString);

		/*
		*	Prepare alternate message for alert box
		*/
		wsprintf(pszDebugString, "File %s\n Line %d", lpszFileName, nLine);

		/*
		*	Display alert message box and accept user response
		*/
    #if defined(WIN32) || defined(WIN64)
		nCode = MessageBox(NULL, pszDebugString, "Assertion Failed!",
		MB_TASKMODAL|MB_ICONHAND|MB_RETRYCANCEL|MB_SETFOREGROUND);
		#else
		nCode = MessageBox(NULL, pszDebugString, "Assertion Failed!",
		MB_TASKMODAL|MB_ICONHAND|MB_RETRYCANCEL);
		#endif //_WIN32	

		/*
		*	Cleanup local string
		*/
		free(pszDebugString);
		pszDebugString = NULL;
	}

	if (nCode == IDIGNORE)
	{
		/*
		*	User chose to ignore, calling macro should proceed
		*/
		return FALSE;
	}

	/*
	*	User chose to calling macro should attempt a break for the debugger
	*/
	return TRUE; 
}

/*
* Function: DebugTraceMessage
* Inputs:
*		lpszFormat - printf style format string
*		... - variable # of stacked parameters
* Outputs: none
* Comments:
*/
void DebugTraceMessage(LPCSTR lpszFormat, ...)
{
	/*
	* duplicate printf functionality for OutputDebugString
	*/
	char *pszDebugString = malloc(MAXDEBUGSTRING);
	va_list args;
	va_start(args, lpszFormat);
	
	if(pszDebugString)
	{
		/*
		* format and output the debug string
		*/
		vsprintf_s(pszDebugString, MAXDEBUGSTRING, lpszFormat, args);	
		OutputDebugString(pszDebugString);

		/*
		* Cleanup local string
		*/
		free(pszDebugString);
		pszDebugString = NULL;
	}
	va_end(args);
	return;
}

#endif //_DEBUG

