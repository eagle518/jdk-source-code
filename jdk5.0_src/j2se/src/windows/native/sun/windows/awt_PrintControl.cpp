/*
 * @(#)awt_PrintControl.cpp	1.26 04/03/23
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "awt_Component.h"
#include "awt_PrintControl.h"
#include "awt.h"
#include "awt_PrintDialog.h"
#include <winspool.h>
#include <float.h>
#include <math.h>

#define ROUNDTOINT(x) ((int)((x)+0.5))
static const int DEFAULT_RES = 72;
static const double TENTHS_MM_TO_POINTS = 3.527777778;
static const double LOMETRIC_TO_POINTS = (72.0 / 254.0);


/* Values must match those defined in WPrinterJob.java */
static const DWORD SET_COLOR = 0x00000200;
static const DWORD SET_ORIENTATION = 0x00004000;
static const DWORD SET_DUP_VERTICAL = 0x00000010;
static const DWORD SET_DUP_HORIZONTAL = 0x00000020;
static const DWORD SET_RES_HIGH = 0x00000040;
static const DWORD SET_RES_LOW = 0x00000080;


/* sun.awt.print.WPrinterJob */

jfieldID  AwtPrintControl::dialogOwnerPeerID;
jfieldID  AwtPrintControl::printdcID;
jfieldID  AwtPrintControl::devmodeID;
jfieldID  AwtPrintControl::devnameID;
jfieldID  AwtPrintControl::driverDoesMultipleCopiesID;
jfieldID  AwtPrintControl::driverDoesCollationID;
jmethodID AwtPrintControl::getWin32MediaID;
jmethodID AwtPrintControl::setWin32MediaID;


/* sun.awt.print.PrintControl */

jmethodID AwtPrintControl::getColorID;
jmethodID AwtPrintControl::getCopiesID;
jmethodID AwtPrintControl::getSelectID;
jmethodID AwtPrintControl::getDestID;
jmethodID AwtPrintControl::getDialogID;
jmethodID AwtPrintControl::getFromPageID;
jmethodID AwtPrintControl::getMaxPageID;
jmethodID AwtPrintControl::getMinPageID;
jmethodID AwtPrintControl::getMDHID;
jmethodID AwtPrintControl::getOrientID;
jmethodID AwtPrintControl::getQualityID;
jmethodID AwtPrintControl::getPrintToFileEnabledID;
jmethodID AwtPrintControl::getPrinterID;
jmethodID AwtPrintControl::setPrinterID;
jmethodID AwtPrintControl::getResID;
jmethodID AwtPrintControl::getSidesID;
jmethodID AwtPrintControl::setSidesID;
jmethodID AwtPrintControl::getToPageID;
jmethodID AwtPrintControl::setToPageID;
jmethodID AwtPrintControl::setNativeAttID;
jmethodID AwtPrintControl::setRangeCopiesID;
jmethodID AwtPrintControl::setResID;


BOOL AwtPrintControl::IsSupportedLevel(HANDLE hPrinter, DWORD dwLevel) {
    BOOL isSupported = FALSE;
    DWORD cbBuf = 0;
    LPBYTE pPrinter = NULL;

    DASSERT(hPrinter != NULL);

    VERIFY(::GetPrinter(hPrinter, dwLevel, NULL, 0, &cbBuf) == 0);
    if (::GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
        pPrinter = new BYTE[cbBuf];
        if (::GetPrinter(hPrinter, dwLevel, pPrinter, cbBuf, &cbBuf)) {
            isSupported = TRUE;
        }
        delete[] pPrinter;
    }

    return isSupported;
}

BOOL AwtPrintControl::FindPrinter(jstring printerName, LPBYTE pPrinterEnum,
				  LPDWORD pcbBuf, LPTSTR * foundPrinter,
				  LPTSTR * foundPort)
{
    JNIEnv *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);

    BOOL nt = IS_NT;
    DWORD cReturned = 0;

    if (pPrinterEnum == NULL) {
        // Compute size of buffer
        DWORD cbNeeded = 0;
        if (nt) {
	    ::EnumPrinters(PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS,
			   NULL, 2, NULL, 0, &cbNeeded, &cReturned);
	}
	::EnumPrinters(PRINTER_ENUM_LOCAL,
		       NULL, 5, NULL, 0, pcbBuf, &cReturned);
	if (cbNeeded > (*pcbBuf)) {
	    *pcbBuf = cbNeeded;
	}
	return TRUE;
    }

    DASSERT(printerName != NULL);

    DWORD cbBuf = *pcbBuf, dummyWord = 0;

    JavaStringBuffer printerNameBuf(env, printerName);
    LPTSTR lpcPrinterName = (LPTSTR)printerNameBuf;
    DASSERT(lpcPrinterName != NULL);

    // For NT, first do a quick check of all remote and local printers.
    // This only allows us to search by name, though. PRINTER_INFO_4
    // doesn't support port searches. So, if the user has specified the
    // printer name as "LPT1:" (even though this is actually a port
    // name), we won't find the printer here.
    if (nt) {
        if (!::EnumPrinters(PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS,
			    NULL, 4, pPrinterEnum, cbBuf, &dummyWord, &cReturned)) {
	    return FALSE;
	}

	for (DWORD i = 0; i < cReturned; i++) {
	    PRINTER_INFO_4 *info4 = (PRINTER_INFO_4 *)
	        (pPrinterEnum + i * sizeof(PRINTER_INFO_4));
	    if (info4->pPrinterName != NULL &&
		_tcsicmp(lpcPrinterName, info4->pPrinterName) == 0) {

                // Fix for BugTraq Id 4281380.
                // Get the port name since some drivers may require
                // this name to be passed to ::DeviceCapabilities().
                HANDLE hPrinter = NULL;
                if (::OpenPrinter(info4->pPrinterName, &hPrinter, NULL)) {
                    // Fix for BugTraq Id 4286812.
                    // Some drivers don't support PRINTER_INFO_5.
                    // In this case we try PRINTER_INFO_2, and if that 
                    // isn't supported as well return NULL port name.
                    try {
                        if (AwtPrintControl::IsSupportedLevel(hPrinter, 5)) {
                            VERIFY(::GetPrinter(hPrinter, 5, pPrinterEnum, cbBuf,
                                                &dummyWord));
                            PRINTER_INFO_5 *info5 = (PRINTER_INFO_5 *)pPrinterEnum;
                            *foundPrinter = info5->pPrinterName;
                            // pPortName may specify multiple ports. We only want one.
			    *foundPort = (info5->pPortName != NULL)
			        ? _tcstok(info5->pPortName, TEXT(",")) : NULL;
                        } else if (AwtPrintControl::IsSupportedLevel(hPrinter, 2)) {
                            VERIFY(::GetPrinter(hPrinter, 2, pPrinterEnum, cbBuf,
                                                &dummyWord));
                            PRINTER_INFO_2 *info2 = (PRINTER_INFO_2 *)pPrinterEnum;
                            *foundPrinter = info2->pPrinterName;
                            // pPortName may specify multiple ports. We only want one.
			    *foundPort = (info2->pPortName != NULL)
			        ? _tcstok(info2->pPortName, TEXT(",")) : NULL;
                        } else {
                            *foundPrinter = info4->pPrinterName;
                            // We failed to determine port name for the found printer.
                            *foundPort = NULL;
                        }
                    } catch (std::bad_alloc&) {
                        VERIFY(::ClosePrinter(hPrinter));
                        throw;
                    }

                    VERIFY(::ClosePrinter(hPrinter));

		    return TRUE;
		}

                return FALSE;
	    }
	}
    }

    // We still haven't found the printer, or we're using 95/98.
    // PRINTER_INFO_5 supports both printer name and port name, so
    // we'll test both. On NT, PRINTER_ENUM_LOCAL means just local
    // printers. This is what we want, because we already tested all
    // remote printer names above (and remote printer port names are
    // the same as remote printer names). On 95/98, PRINTER_ENUM_LOCAL
    // means both remote and local printers. This is also what we want
    // because we haven't tested any printers yet.
    if (!::EnumPrinters(PRINTER_ENUM_LOCAL,
			NULL, 5, pPrinterEnum, cbBuf, &dummyWord, &cReturned)) {
        return FALSE;
    }

    for (DWORD i = 0; i < cReturned; i++) {
        PRINTER_INFO_5 *info5 = (PRINTER_INFO_5 *)
	    (pPrinterEnum + i * sizeof(PRINTER_INFO_5));
	if (nt) {
	    // pPortName can specify multiple ports. Test them one at
	    // a time.
	    if (info5->pPortName != NULL) {
	        LPTSTR port = _tcstok(info5->pPortName, TEXT(","));
		while (port != NULL) {
		    if (_tcsicmp(lpcPrinterName, port) == 0) {
		        *foundPrinter = info5->pPrinterName;
			*foundPort = port;
			return TRUE;
		    }
		    port = _tcstok(NULL, TEXT(","));
		}
	    }
	} else {
	    if ((info5->pPrinterName != NULL &&
		 _tcsicmp(lpcPrinterName, info5->pPrinterName) == 0) ||
		(info5->pPortName != NULL &&
		 _tcsicmp(lpcPrinterName, info5->pPortName) == 0)) {
	        *foundPrinter = info5->pPrinterName;
                *foundPort = info5->pPortName;
		return TRUE;
	    }
	}
    }

    return FALSE;
}


void AwtPrintControl::initIDs(JNIEnv *env, jclass cls)
{
    TRY;

    jclass cls = env->FindClass("sun/awt/windows/WPrinterJob");

    AwtPrintControl::dialogOwnerPeerID =
      env->GetFieldID(cls, "dialogOwnerPeer", "Ljava/awt/peer/ComponentPeer;");
    AwtPrintControl::printdcID = 
      env->GetFieldID(cls, PRINTDC_STR, "J");
    AwtPrintControl::devnameID = 
      env->GetFieldID(cls, PRINTDNAME_STR, "J");
    AwtPrintControl::devmodeID =  
      env->GetFieldID(cls, PRINTHDMODE_STR, "J");
    AwtPrintControl::driverDoesMultipleCopiesID = 
      env->GetFieldID(cls, "driverDoesMultipleCopies", "Z");
    AwtPrintControl::driverDoesCollationID = 
      env->GetFieldID(cls, "driverDoesCollation", "Z");
    AwtPrintControl::getCopiesID =
      env->GetMethodID(cls, "getCopiesAttrib", "()I");
    AwtPrintControl::getMDHID = 
      env->GetMethodID(cls, "getMDHAttrib","()Z");
    AwtPrintControl::getOrientID = 
      env->GetMethodID(cls, "getOrientAttrib", "()I");
    AwtPrintControl::getFromPageID = 
      env->GetMethodID(cls, "getFromPageAttrib", "()I");
    AwtPrintControl::getToPageID = 
      env->GetMethodID(cls, "getToPageAttrib", "()I");
    AwtPrintControl::getMinPageID = 
      env->GetMethodID(cls, "getMinPageAttrib", "()I");
    AwtPrintControl::getMaxPageID = 
      env->GetMethodID(cls, "getMaxPageAttrib", "()I");
    AwtPrintControl::getDestID = 
      env->GetMethodID(cls, "getDestAttrib", "()Z");
    AwtPrintControl::getQualityID = 
      env->GetMethodID(cls, "getQualityAttrib",	"()I");
    AwtPrintControl::getColorID = 
      env->GetMethodID(cls, "getColorAttrib", "()I");
    AwtPrintControl::getSidesID = 
      env->GetMethodID(cls, "getSidesAttrib", "()I");
    AwtPrintControl::getPrinterID = 
      env->GetMethodID(cls, "getPrinterAttrib", "()Ljava/lang/String;"); 
    AwtPrintControl::getWin32MediaID =
        env->GetMethodID(cls, "getWin32MediaAttrib", "()[I");  
    AwtPrintControl::getSelectID = 
      env->GetMethodID(cls, "getSelectAttrib", "()I");
    AwtPrintControl::getPrintToFileEnabledID = 
      env->GetMethodID(cls, "getPrintToFileEnabled", "()Z");

    AwtPrintControl::setNativeAttID = 
      env->GetMethodID(cls, "setNativeAttributes", "(II)V");  

    AwtPrintControl::setRangeCopiesID = 
      env->GetMethodID(cls, "setRangeCopiesAttribute", "(IIZI)V"); 
    AwtPrintControl::setResID = 
      env->GetMethodID(cls, "setResolutionDPI", "(II)V");  
    AwtPrintControl::setWin32MediaID = 
      env->GetMethodID(cls, "setWin32MediaAttrib", "(III)V");  
    AwtPrintControl::setPrinterID = 
      env->GetMethodID(cls, "setPrinterNameAttrib", "(Ljava/lang/String;)V");  

    DASSERT(AwtPrintControl::driverDoesMultipleCopiesID != NULL);
    DASSERT(AwtPrintControl::printdcID != NULL);
    DASSERT(AwtPrintControl::devnameID != NULL);
    DASSERT(AwtPrintControl::devmodeID != NULL);
    DASSERT(AwtPrintControl::driverDoesCollationID != NULL);
    DASSERT(AwtPrintControl::setWin32MediaID != NULL);
    DASSERT(AwtPrintControl::setRangeCopiesID != NULL);
    DASSERT(AwtPrintControl::setResID != NULL);
    DASSERT(AwtPrintControl::setNativeAttID != NULL);
    DASSERT(AwtPrintControl::dialogOwnerPeerID != NULL);
    DASSERT(AwtPrintControl::getCopiesID != NULL);
    DASSERT(AwtPrintControl::getOrientID != NULL);
    DASSERT(AwtPrintControl::getPrinterID != NULL);
    DASSERT(AwtPrintControl::getMDHID != NULL);
    DASSERT(AwtPrintControl::getFromPageID != NULL);
    DASSERT(AwtPrintControl::getToPageID != NULL);
    DASSERT(AwtPrintControl::getMinPageID != NULL);
    DASSERT(AwtPrintControl::getMaxPageID != NULL);
    DASSERT(AwtPrintControl::getDestID != NULL);
    DASSERT(AwtPrintControl::getQualityID != NULL);
    DASSERT(AwtPrintControl::getColorID != NULL);
    DASSERT(AwtPrintControl::getSidesID != NULL);  
    DASSERT(AwtPrintControl::getWin32MediaID != NULL);  
    DASSERT(AwtPrintControl::getSelectID != NULL);
    DASSERT(AwtPrintControl::getPrintToFileEnabledID != NULL);


    CATCH_BAD_ALLOC;
}

BOOL CALLBACK PrintDlgHook(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    TRY;

    if (iMsg == WM_INITDIALOG) {
	SetForegroundWindow(hDlg);
	return FALSE;
    }
    return FALSE;

    CATCH_BAD_ALLOC_RET(TRUE);
}

BOOL AwtPrintControl::CreateDevModeAndDevNames(PRINTDLG *ppd,
					       LPTSTR pPrinterName,
					       LPTSTR pPortName)
{
    DWORD cbNeeded = 0;
    LPBYTE pPrinter = NULL;
    BOOL retval = FALSE;
    HANDLE hPrinter;

    try {
        if (!::OpenPrinter(pPrinterName, &hPrinter, NULL)) {
	    goto done;
	}
	VERIFY(::GetPrinter(hPrinter, 2, NULL, 0, &cbNeeded) == 0);
	if (::GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
	    goto done;
	}
	pPrinter = new BYTE[cbNeeded];
	if (!::GetPrinter(hPrinter, 2, pPrinter, cbNeeded, &cbNeeded)) {
	    goto done;
	}
	PRINTER_INFO_2 *info2 = (PRINTER_INFO_2 *)pPrinter;

	// Create DEVMODE, if it exists.
	if (info2->pDevMode != NULL) {
	    size_t devmodeSize =
	        sizeof(DEVMODE) + info2->pDevMode->dmDriverExtra;
	    ppd->hDevMode = ::GlobalAlloc(GHND, devmodeSize);
	    if (ppd->hDevMode == NULL) {
	        throw std::bad_alloc();
	    }
	    DEVMODE *devmode = (DEVMODE *)::GlobalLock(ppd->hDevMode);
	    DASSERT(!::IsBadWritePtr(devmode, devmodeSize));
	    memcpy(devmode, info2->pDevMode, devmodeSize);
	    VERIFY(::GlobalUnlock(ppd->hDevMode) == 0);
	    DASSERT(::GetLastError() == NO_ERROR);
	}

	// Create DEVNAMES.
	if (IS_NT) {
	    if (pPortName != NULL) {
	        info2->pPortName = pPortName;
	    } else if (info2->pPortName != NULL) {
	        // pPortName may specify multiple ports. We only want one.
	        info2->pPortName = _tcstok(info2->pPortName, TEXT(","));
	    }
	}

	size_t lenDriverName = ((info2->pDriverName != NULL)
				    ? _tcslen(info2->pDriverName)
				    : 0) + 1;
	size_t lenPrinterName = ((pPrinterName != NULL)
				     ? _tcslen(pPrinterName)
				     : 0) + 1;
	size_t lenOutputName = ((info2->pPortName != NULL)
				    ? _tcslen(info2->pPortName)
				    : 0) + 1;
	size_t devnameSize= sizeof(DEVNAMES) +
			lenDriverName*sizeof(TCHAR) +
			lenPrinterName*sizeof(TCHAR) +
			lenOutputName*sizeof(TCHAR);
	
	ppd->hDevNames = ::GlobalAlloc(GHND, devnameSize);
	if (ppd->hDevNames == NULL) {
	    throw std::bad_alloc();
	}

	DEVNAMES *devnames =
	    (DEVNAMES *)::GlobalLock(ppd->hDevNames);
	DASSERT(!IsBadWritePtr(devnames, devnameSize));
	LPTSTR lpcDevnames = (LPTSTR)devnames;

	// note: all sizes are in characters, not in bytes
	devnames->wDriverOffset = sizeof(DEVNAMES)/sizeof(TCHAR);
	devnames->wDeviceOffset = 
	    static_cast<WORD>(sizeof(DEVNAMES)/sizeof(TCHAR) + lenDriverName);
	devnames->wOutputOffset =
	    static_cast<WORD>(sizeof(DEVNAMES)/sizeof(TCHAR) + lenDriverName + lenPrinterName);
	if (info2->pDriverName != NULL) {
	    _tcscpy(lpcDevnames + devnames->wDriverOffset, info2->pDriverName);
	} else {
	    *(lpcDevnames + devnames->wDriverOffset) = _T('\0');
	}
	if (pPrinterName != NULL) {
	    _tcscpy(lpcDevnames + devnames->wDeviceOffset, pPrinterName);
	} else {
	    *(lpcDevnames + devnames->wDeviceOffset) = _T('\0');
	}
	if (info2->pPortName != NULL) {
	    _tcscpy(lpcDevnames + devnames->wOutputOffset, info2->pPortName);
	} else {
	    *(lpcDevnames + devnames->wOutputOffset) = _T('\0');
	}
	VERIFY(::GlobalUnlock(ppd->hDevNames) == 0);
	DASSERT(::GetLastError() == NO_ERROR);
    } catch (std::bad_alloc&) {
        if (ppd->hDevNames != NULL) {
	    VERIFY(::GlobalFree(ppd->hDevNames) == NULL);
	    ppd->hDevNames = NULL;
	}
	if (ppd->hDevMode != NULL) {
	    VERIFY(::GlobalFree(ppd->hDevMode) == NULL);
	    ppd->hDevMode = NULL;
	}
	delete [] pPrinter;
	VERIFY(::ClosePrinter(hPrinter));
	hPrinter = NULL;
	throw;
    }

    retval = TRUE;

done:
    delete [] pPrinter;
    if (hPrinter) {
        VERIFY(::ClosePrinter(hPrinter));
	hPrinter = NULL;
    }

    return retval;
}


WORD AwtPrintControl::getNearestMatchingPaper(LPTSTR printer, LPTSTR port, 
			       	      double origWid, double origHgt,
			       	      double* newWid, double *newHgt) {
    const double epsilon = 0.50;
    const double tolerance = (1.0 * 72.0);  // # inches * 72
    int numPaperSizes = 0;
    WORD *papers = NULL;
    POINT *paperSizes = NULL;

    if ((printer== NULL) || (port == NULL)) {
        return 0;
    }

    SAVE_CONTROLWORD
    numPaperSizes = (int)DeviceCapabilities(printer, port, DC_PAPERSIZE,
					      NULL, NULL);

    if (numPaperSizes > 0) {
        papers = (WORD*)safe_Malloc(sizeof(WORD) * numPaperSizes);
	paperSizes = (POINT *)safe_Malloc(sizeof(*paperSizes) * 
					  numPaperSizes);

	DWORD result1 = DeviceCapabilities(printer, port,
				       DC_PAPERS, (LPTSTR) papers, NULL);

	DWORD result2 = DeviceCapabilities(printer, port,
				       DC_PAPERSIZE, (LPTSTR) paperSizes,
				       NULL);

	// REMIND: cache in papers and paperSizes
	if (result1 == -1 || result2 == -1 ) {
  	    free((LPTSTR) papers);
	    papers = NULL;
	    free((LPTSTR) paperSizes);
	    paperSizes = NULL;
	}
    }  
    RESTORE_CONTROLWORD

    double closestWid = 0.0;
    double closestHgt = 0.0;
    WORD   closestMatch = 0;
  
    if (paperSizes != NULL) {

      /* Paper sizes are in 0.1mm units. Convert to 1/72"
       * For each paper size, compute the difference from the paper size
       * passed in. Use a least-squares difference, so paper much different
       * in x or y should score poorly
       */
        double diffw = origWid;
	double diffh = origHgt;
	double least_square = diffw * diffw + diffh * diffh;
	double tmp_ls;
	double widpts, hgtpts;

	for (int i=0;i<numPaperSizes;i++) {
	    widpts = paperSizes[i].x * LOMETRIC_TO_POINTS;
	    hgtpts = paperSizes[i].y * LOMETRIC_TO_POINTS;
	  
	    if ((fabs(origWid - widpts) < epsilon) &&
		(fabs(origHgt - hgtpts) < epsilon)) {
	        closestWid = origWid;
		closestHgt = origHgt;
		closestMatch = papers[i];
		break;
	    }
      
	    diffw = fabs(widpts - origWid);
	    diffh = fabs(hgtpts - origHgt);
	    tmp_ls = diffw * diffw + diffh * diffh;
	    if ((diffw < tolerance) && (diffh < tolerance) &&
		(tmp_ls < least_square)) {
 	        least_square = tmp_ls;
		closestWid = widpts;
		closestHgt = hgtpts;
		closestMatch = papers[i];
	    }
	}
    }

    if (closestWid > 0) {
        *newWid = closestWid;
    }
    if (closestHgt > 0) {
        *newHgt = closestHgt;
    }

    if (papers != NULL) {
        free((LPTSTR)papers);
    }

    if (paperSizes != NULL) {
        free((LPTSTR)paperSizes);
    }  

    return closestMatch;  
}

/*
 * Copy settings into a print dialog & any devmode
 */
BOOL AwtPrintControl::InitPrintDialog(JNIEnv *env,
				      jobject printCtrl, PRINTDLG &pd) {
    HWND hwndOwner = NULL;
    jobject dialogOwner =
        env->GetObjectField(printCtrl, AwtPrintControl::dialogOwnerPeerID);
    if (dialogOwner != NULL) {
        AwtComponent *dialogOwnerComp = 
	  (AwtComponent *)JNI_GET_PDATA(dialogOwner);

	hwndOwner = dialogOwnerComp->GetHWnd();
	env->DeleteLocalRef(dialogOwner);
	dialogOwner = NULL;
    }
    jobject mdh = NULL;
    jobject dest = NULL;
    jobject select = NULL;
    jobject dialog = NULL;
    LPTSTR printName = NULL;
    LPTSTR portName = NULL;

    // If the user didn't specify a printer, then this call returns the
    // name of the default printer.
    jstring printerName = (jstring)
      env->CallObjectMethod(printCtrl, AwtPrintControl::getPrinterID);

    if (printerName != NULL) {

	pd.hDevMode = AwtPrintControl::getPrintHDMode(env, printCtrl);
	pd.hDevNames = AwtPrintControl::getPrintHDName(env, printCtrl);
        
	LPTSTR getName = (LPTSTR)JNU_GetStringPlatformChars(env, 
						      printerName, NULL);

	BOOL samePrinter = FALSE;
      
	// check if given printername is same as the currently saved printer
	if (pd.hDevNames != NULL ) {

	    DEVNAMES *devnames = (DEVNAMES *)::GlobalLock(pd.hDevNames);
	    if (devnames != NULL) {
	        LPTSTR lpdevnames = (LPTSTR)devnames;
		printName = lpdevnames+devnames->wDeviceOffset;
		
		if (!_tcscmp(printName, getName)) {
	
		    samePrinter = TRUE;		
		    printName = _tcsdup(lpdevnames+devnames->wDeviceOffset);   
		    portName = _tcsdup(lpdevnames+devnames->wOutputOffset);
	
		}
	    }
	}
   
	if (!samePrinter) {
 	    LPTSTR foundPrinter = NULL;
	    LPTSTR foundPort = NULL;
	    DWORD cbBuf = 0;
	    VERIFY(AwtPrintControl::FindPrinter(NULL, NULL, &cbBuf, 
						NULL, NULL));
	    LPBYTE buffer = new BYTE[cbBuf];
      
	    if (AwtPrintControl::FindPrinter(printerName, buffer, &cbBuf,
					     &foundPrinter, &foundPort) &&
		(foundPrinter != NULL) && (foundPort != NULL)) {

	        printName = _tcsdup(foundPrinter);   
		portName = _tcsdup(foundPort);
			
	        if (!AwtPrintControl::CreateDevModeAndDevNames(&pd,
						   foundPrinter, foundPort)) {
		    delete [] buffer;		    
		    if (printName != NULL) {
		      free(printName);
		    }
		    if (portName != NULL) {
		      free(portName);
		    }
		    return FALSE;
		}

		DASSERT(pd.hDevNames != NULL);
	    } else {
	        delete [] buffer;		
		if (printName != NULL) {
		  free(printName);
		}
		if (portName != NULL) {
		  free(portName);
		}
		JNU_ThrowIllegalArgumentException(env, 
		   "Invalid value for property printer or null printer port");
		return FALSE;
	    }    
      
	    delete [] buffer;
	}
    } else {

        // There is no default printer. This means that there are no
        // printers installed at all.
        // If we were to display the native print dialog, instead
        // display a native warning message by calling PrintDlg
        // without the PD_RETURNDEFAULT flag set.
        AwtPrintDialog::PrintDlg(&pd);

	if (printName != NULL) {
	  free(printName);
	}
	if (portName != NULL) {
	  free(portName);
	}
	return FALSE;
    }

    // Now, set-up the struct for the real calls to ::PrintDlg and ::CreateDC

    pd.hwndOwner = hwndOwner;
    pd.Flags = PD_ENABLEPRINTHOOK | PD_RETURNDC;
    pd.lpfnPrintHook = (LPPRINTHOOKPROC)PrintDlgHook;

    if (env->CallBooleanMethod(printCtrl, AwtPrintControl::getMDHID)) {
        pd.Flags |= PD_COLLATE;
    }
	
    pd.nCopies = (WORD)env->CallIntMethod(printCtrl,
					  AwtPrintControl::getCopiesID);
    pd.nFromPage = (WORD)env->CallIntMethod(printCtrl, 
					    AwtPrintControl::getFromPageID);
    pd.nToPage = (WORD)env->CallIntMethod(printCtrl, 
					  AwtPrintControl::getToPageID);
    pd.nMinPage = (WORD)env->CallIntMethod(printCtrl,
					   AwtPrintControl::getMinPageID);
    jint maxPage = env->CallIntMethod(printCtrl, 
				      AwtPrintControl::getMaxPageID);
    pd.nMaxPage = (maxPage <= (jint)((WORD)-1)) ? (WORD)maxPage : (WORD)-1;
    
    if (env->CallBooleanMethod(printCtrl, 
			       AwtPrintControl::getDestID)) {
      pd.Flags |= PD_PRINTTOFILE;
    }

    jint selectType = env->CallIntMethod(printCtrl, 
					 AwtPrintControl::getSelectID);

    // selectType identifies whether No selection (2D) or 
    // SunPageSelection (AWT)
    if (selectType != 0) {
      pd.Flags |= selectType;
    }
  
    if (!env->CallBooleanMethod(printCtrl,
				AwtPrintControl::getPrintToFileEnabledID)) {
      pd.Flags |= PD_DISABLEPRINTTOFILE;
    }
    
    if (pd.hDevMode != NULL) {	
      DEVMODE *devmode = (DEVMODE *)::GlobalLock(pd.hDevMode);
      DASSERT(!IsBadWritePtr(devmode, sizeof(DEVMODE)));
      
      devmode->dmFields |= DM_COPIES | DM_COLLATE | DM_ORIENTATION |
	DM_PAPERSIZE | DM_PRINTQUALITY | DM_COLOR |
	DM_PRINTQUALITY | DM_COLOR |
	DM_DUPLEX;
      
      devmode->dmCopies = pd.nCopies;   
      
      jint orient = env->CallIntMethod(printCtrl,
				       AwtPrintControl::getOrientID);
      if (orient == 0) {
	devmode->dmOrientation = DMORIENT_LANDSCAPE;
      } else if (orient == 1) {
	devmode->dmOrientation = DMORIENT_PORTRAIT;
      }
      
      devmode->dmCollate = (pd.Flags & PD_COLLATE) ? DMCOLLATE_TRUE
	: DMCOLLATE_FALSE;
      
      int quality = env->CallIntMethod(printCtrl, 
				       AwtPrintControl::getQualityID);        
      if (quality) {
	devmode->dmPrintQuality = quality;
      }
      
      int color = env->CallIntMethod(printCtrl,
				     AwtPrintControl::getColorID);
      if (color) {
	devmode->dmColor = color;
      } 
      
      int sides = env->CallIntMethod(printCtrl,
				     AwtPrintControl::getSidesID);
      if (sides) {
	devmode->dmDuplex = (int)sides;
      }
      
      jintArray obj = (jintArray)env->CallObjectMethod(printCtrl, 
	       			       AwtPrintControl::getWin32MediaID);
      jboolean isCopy;
      jint *wid_ht = env->GetIntArrayElements(obj,
					      &isCopy);
      
      double newWid = 0.0, newHt = 0.0;
      if (wid_ht != NULL && wid_ht[0] != 0 && wid_ht[1] != 0) { 
	devmode->dmPaperSize = AwtPrintControl::getNearestMatchingPaper(
					     printName,
					     portName,
			    		     (double)wid_ht[0], 
					     (double)wid_ht[1],
					     &newWid, &newHt);

      }
      env->ReleaseIntArrayElements(obj, wid_ht, 0);
      ::GlobalUnlock(pd.hDevMode);
      devmode = NULL;
    }

    if (printName != NULL) {
      free(printName);
    }
    if (portName != NULL) {
      free(portName);
    }

    return TRUE;
}


/*
 * Copy settings from print dialog & any devmode back into attributes
 * or properties.
 */
BOOL AwtPrintControl::UpdateAttributes(JNIEnv *env,
				       jobject printCtrl, PRINTDLG &pd) {

    DEVNAMES *devnames = NULL;
    DEVMODE *devmode = NULL;
    unsigned int copies = 1;
    DWORD pdFlags = pd.Flags;
    DWORD dmFields;
    bool newDC = false;

    if (pd.hDevMode != NULL) {
        devmode = (DEVMODE *)::GlobalLock(pd.hDevMode);
	DASSERT(!IsBadReadPtr(devmode, sizeof(DEVMODE)));
    }
  
    if (devmode != NULL) {
        dmFields = devmode->dmFields;
      
	if (devmode->dmFields & DM_COPIES) {
	    copies = devmode->dmCopies;
	    if (pd.nCopies == 1) {
	        env->SetBooleanField(printCtrl, 
				     driverDoesMultipleCopiesID, 
				     JNI_TRUE); 
	    } else {
	      copies = pd.nCopies;
	    }
	}
	
	if (devmode->dmFields & DM_PAPERSIZE) {      
	    env->CallVoidMethod(printCtrl, AwtPrintControl::setWin32MediaID, 
			      devmode->dmPaperSize, devmode->dmPaperWidth,
			      devmode->dmPaperLength);

	}

	if (devmode->dmFields & DM_COLOR) {
	    if (devmode->dmColor == DMCOLOR_COLOR) {
	        dmFields |= SET_COLOR;
	    } else {
	        dmFields &= ~SET_COLOR;
	    }
	}
 
	if (devmode->dmFields & DM_ORIENTATION) { 
	    if (devmode->dmOrientation == DMORIENT_LANDSCAPE) {
	        dmFields |= SET_ORIENTATION;
	    } else {
	        dmFields &= ~SET_ORIENTATION;
	    }
	}
	
	if (devmode->dmFields & DM_COLLATE) {
	    if (devmode->dmCollate == DMCOLLATE_TRUE) {
	        pdFlags |= PD_COLLATE;
		env->SetBooleanField(printCtrl, 
				     driverDoesCollationID, 
				     JNI_TRUE); 
	    } else {
	        pdFlags &= ~PD_COLLATE;
	    }
	}

	if (devmode->dmFields & DM_PRINTQUALITY) {
	    if (devmode->dmPrintQuality == DMRES_HIGH) {
	        dmFields |= SET_RES_HIGH;
	    } else if ((devmode->dmPrintQuality == DMRES_LOW) ||
		       (devmode->dmPrintQuality == DMRES_DRAFT)) {
	        dmFields |= SET_RES_LOW;
	    } else if (devmode->dmPrintQuality == DMRES_MEDIUM) { 	
 	        dmFields &= ~(SET_RES_HIGH | SET_RES_LOW);
	    } else {	
	        int xRes = devmode->dmPrintQuality;
	        int yRes = (devmode->dmFields & DM_YRESOLUTION) ? 
		  devmode->dmYResolution : devmode->dmPrintQuality;
		env->CallVoidMethod(printCtrl, AwtPrintControl::setResID, 
				    xRes, yRes);
		dmFields &= ~DM_PRINTQUALITY;
	    }
	}

	if (devmode->dmFields & DM_DUPLEX) {
	    if (devmode->dmDuplex == DMDUP_HORIZONTAL) {
	        dmFields |= SET_DUP_HORIZONTAL;
	    } else if (devmode->dmDuplex == DMDUP_VERTICAL) {
	        dmFields |= SET_DUP_VERTICAL;
	    } else {
	        dmFields &= ~(SET_DUP_HORIZONTAL | SET_DUP_VERTICAL);
	    } 
	}
	

	::GlobalUnlock(pd.hDevMode);
	devmode = NULL;
    } else {
        copies = pd.nCopies;
    }

    if (pd.hDevNames != NULL) {
        DEVNAMES *devnames = (DEVNAMES*)::GlobalLock(pd.hDevNames);
	DASSERT(!IsBadReadPtr(devnames, sizeof(DEVNAMES)));
	LPTSTR lpcNames = (LPTSTR)devnames;
	LPTSTR pbuf = (_tcslen(lpcNames + devnames->wDeviceOffset) == 0 ?
                      TEXT("") : lpcNames + devnames->wDeviceOffset);
	if (pbuf != NULL) {
            jstring jstr = JNU_NewStringPlatform(env, pbuf);
            env->CallVoidMethod(printCtrl,
				AwtPrintControl::setPrinterID,
				jstr);
	    env->DeleteLocalRef(jstr);
	}
	::GlobalUnlock(pd.hDevNames);
	devnames = NULL;
    }  


    env->CallVoidMethod(printCtrl, AwtPrintControl::setNativeAttID, 
			pdFlags,  dmFields);    
    

    // copies  & range are always set so no need to check for any flags
    env->CallVoidMethod(printCtrl, AwtPrintControl::setRangeCopiesID, 
			pd.nFromPage, pd.nToPage, (pdFlags & PD_PAGENUMS),
			copies);

    // repeated calls to printDialog should not leak handles 
    HDC oldDC = AwtPrintControl::getPrintDC(env, printCtrl);
    if (pd.hDC != oldDC) {
        if (oldDC != NULL) {
	    ::DeleteDC(oldDC);
	}
	AwtPrintControl::setPrintDC(env, printCtrl, pd.hDC);
	newDC = true;
    }

    HGLOBAL oldG = AwtPrintControl::getPrintHDMode(env, printCtrl);
    if (pd.hDevMode != oldG) {
        if (oldG != NULL) {
	    ::GlobalFree(oldG);
	}
	AwtPrintControl::setPrintHDMode(env, printCtrl, pd.hDevMode);
    }
    
    oldG = AwtPrintControl::getPrintHDName(env, printCtrl);
    if (pd.hDevNames != oldG) {
        if (oldG != NULL) {
	    ::GlobalFree(oldG);
	}
	AwtPrintControl::setPrintHDName(env, printCtrl, pd.hDevNames);
    }

    return newDC;
}


BOOL AwtPrintControl::getDevmode( HANDLE hPrinter,
				 LPTSTR printerName,
				 LPDEVMODE *pDevMode) {

    if (hPrinter == NULL || printerName == NULL || pDevMode == NULL) {
      return FALSE;
    }

    SAVE_CONTROLWORD

    DWORD dwNeeded = ::DocumentProperties(NULL, hPrinter, printerName,
					NULL, NULL, 0);

    RESTORE_CONTROLWORD
    
    if (dwNeeded <= 0) {
        *pDevMode = NULL;
        return FALSE;
    }

    *pDevMode = (LPDEVMODE)GlobalAlloc(GPTR, dwNeeded);

    if (*pDevMode == NULL) {
        return FALSE;
    }

    DWORD dwRet = ::DocumentProperties(NULL,
				       hPrinter,
				       printerName,
				       *pDevMode, 
				       NULL,        
				       DM_OUT_BUFFER); 

    RESTORE_CONTROLWORD

    if (dwRet != IDOK)	{
        /* if failure, cleanup and return failure */
        GlobalFree(pDevMode);
	*pDevMode = NULL;
	return FALSE;
    }

    return TRUE;
}

