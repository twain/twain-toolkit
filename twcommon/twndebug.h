/* 
* File: twndebug.h
* Company: JFL Peripheral Solutions Inc.
* Date: March 1998
* Descripton:
*   macro definitions for debug instrumentation
*
*	Copyright © 1998 TWAIN Working Group: Adobe Systems Incorporated, 
*	Canon Information Systems, Eastman Kodak Company, 
*	Fujitsu Computer Products of America, Genoa Technology, 
*	Hewlett-Packard Company, Intel Corporation, Kofax Image Products, 
*	JFL Peripheral Solutions Inc., Ricoh Corporation, and Xerox Corporation.  
*	All rights reserved.
*/

#ifndef __twndebug_h
#define __twndebug_h

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#ifdef _DEBUG

/*
* Core functions used to implemented various debug macros
*/
void DebugTraceMessage(LPCSTR lpszFormat, ...);
BOOL TriggerAssertionFailed(LPCSTR lpszFileName, int nLine);

/*
*	Debug version, all macros enabled
*/
#define TRACE DebugTraceMessage
#ifdef WIN64
#include <crtdbg.h>
#define ASSERT(cond) _ASSERT(cond)
#else
#define ASSERT(cond) if(!(cond)) if(TriggerAssertionFailed(__FILE__, __LINE__)) _asm {int 3}
#endif
#define HALTCOND(cond) if(!(cond)) _asm {int 3 }
#define VERIFY(cond) ASSERT(cond)

#else //_DEBUG

/*
*	Release version, all macros disabled
*/
#define TRACE

#define ASSERT(cond)
#define HALTCOND(cond)
#define VERIFY(cond) cond

#endif //_DEBUG

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //__twndebug_h
