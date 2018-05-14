/*
* File: Triplets.h
* company: TWAIN Working Group
* Description:
*
*	Copyright © 1998 TWAIN Working Group: Adobe Systems Incorporated, 
*	Canon Information Systems, Eastman Kodak Company, 
*	Fujitsu Computer Products of America, Genoa Technology, 
*	Hewlett-Packard Company, Intel Corporation, Kofax Image Products, 
*	JFL Peripheral Solutions Inc., Ricoh Corporation, and Xerox Corporation.  
*	All rights reserved.
*/

#ifndef _inc_triplets_h
#define _inc_triplets_h

TW_UINT16 SetupFileXfer(char text[],char format[],pTW_SETUPFILEXFER pSetup,TW_UINT16 msg);
TW_UINT16 GetCapabilitySpecial(pTABLECAP pTableCap,TW_UINT16 gettype,HWND hDlg,int Id);
TW_UINT16 SetCapabilitySpecial(pTABLECAP pTableCap,TW_UINT16 settype,HWND hDlg, int Id, pTABLEENTRY pConType);


TW_UINT32 ConvertToNumber(pTABLECAP pTableCap,char *tok, TW_UINT16 type);
BOOL FillOutputString(pTABLECAP pTableCap,char value[],char string[],TW_UINT16 ItemType,void *Item);

TW_UINT16 CustomDSData(pTW_CUSTOMDSDATA pCustomDSData, TW_UINT16 msg);
TW_UINT16 PendingXfers(pTW_PENDINGXFERS pPendingXfers,TW_UINT16 msg);
TW_UINT16 SetupMemXfer(pTW_SETUPMEMXFER pSetup);
TW_UINT16 Status(pTW_STATUS pStatus);
TW_UINT16 UserInterface(pTW_USERINTERFACE pUI,TW_UINT16 msg);
TW_UINT16 ImageFileXfer();
TW_UINT16 ImageNativeXfer(TW_MEMREF Bitmap);
TW_UINT16 ImageMemXfer(TW_MEMREF Bitmap);
TW_UINT16 ImageLayout(pTW_IMAGELAYOUT layout,TW_UINT16 msg);
TW_UINT16 ImageInfo(pTW_IMAGEINFO info);
TW_UINT16 AudioInfo(pTW_AUDIOINFO info);
TW_UINT16 FileSystem(pTW_FILESYSTEM fileSystem, TW_UINT16 msg);
TW_UINT16 Passthru(pTW_PASSTHRU passthru, TW_UINT16 msg);

TW_UINT16 ExtImageInfo(pTW_EXTIMAGEINFO pExtImageInfo);


TW_UINT16 Palette8(pTW_PALETTE8 pal,TW_UINT16 msg);
TW_UINT16 GrayResponse(pTW_GRAYRESPONSE resp, TW_UINT16 msg);
TW_UINT16 RGBResponse(pTW_GRAYRESPONSE resp, TW_UINT16 msg);
TW_UINT16 CIEColor(pTW_CIECOLOR cie);
TW_UINT16 JPEGCompression(pTW_JPEGCOMPRESSION jpeg,TW_UINT16 msg);
TW_UINT16 QuerySupportMessage(pTABLECAP pTableCap, HWND hDlg, int Id);

#endif //_inc_triplets_h
