/*
 * @(#)awt_PrintControl.h	1.15 04/03/23
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef _AWT_PRINT_CONTROL_H_
#define _AWT_PRINT_CONTROL_H_

#include "stdhdrs.h"
#include <commdlg.h>

// names of instance variables in WPrinterJob
static const char *PRINTHDMODE_STR = "mPrintHDevMode";   // The print devmode
static const char *PRINTDNAME_STR = "mPrintHDevNames";  // The print devnames
static const char *PRINTDC_STR = "mPrintDC";	// The print device context.


/************************************************************************
 * AwtPrintControl class
 */

class AwtPrintControl {
public:

    /* sun.awt.windows.AwtPrintControl */

    static jfieldID  dialogOwnerPeerID;
    static jfieldID  driverDoesMultipleCopiesID;
    static jfieldID  driverDoesCollationID;
    static jfieldID  printdcID;
    static jfieldID  devnameID;
    static jfieldID  devmodeID;

    static jmethodID getWin32MediaID;
    static jmethodID setWin32MediaID;

    /* sun.awt.print.PrintControl */

    static jmethodID getColorID;
    static jmethodID getCopiesID;
    static jmethodID getSelectID;
    static jmethodID getDestID;
    static jmethodID getDialogID;
    static jmethodID getFromPageID;
    static jmethodID getMaxPageID;
    static jmethodID getMinPageID;
    static jmethodID getMDHID;
    static jmethodID getOrientID;
    static jmethodID getQualityID;
    static jmethodID getPrintToFileEnabledID;
    static jmethodID getPrinterID;
    static jmethodID setPrinterID;
    static jmethodID getResID;
    static jmethodID getSidesID;
    static jmethodID setSidesID;
    static jmethodID getToPageID;
    static jmethodID setToPageID;
    static jmethodID setNativeAttID;
    static jmethodID setRangeCopiesID;
    static jmethodID setResID;

    static void initIDs(JNIEnv *env, jclass cls);
    static BOOL FindPrinter(jstring printerName, LPBYTE pPrinterEnum,
			    LPDWORD pcbBuf, LPTSTR * foundPrinter,
			    LPTSTR * foundPORT);
    // This function determines whether the printer driver
    // for the passed printer handle supports PRINTER_INFO
    // structure of level dwLevel.
    static BOOL IsSupportedLevel(HANDLE hPrinter, DWORD dwLevel);
    static BOOL CreateDevModeAndDevNames(PRINTDLG *ppd,
					       LPTSTR pPrinterName,
					       LPTSTR pPortName);
    static BOOL InitPrintDialog(JNIEnv *env,
		 		      jobject printCtrl, PRINTDLG &pd);
    static BOOL UpdateAttributes(JNIEnv *env, 
				      jobject printCtrl, PRINTDLG &pd);
    static WORD getNearestMatchingPaper(LPTSTR printer, LPTSTR port, 
				      double origWid, double origHgt,
				      double* newWid, double *newHgt); 

    static BOOL getDevmode(HANDLE hPrinter,
				 LPTSTR pPrinterName,
				 LPDEVMODE *pDevMode);
    
    inline static  HDC getPrintDC(JNIEnv *env, jobject self) {
      return (HDC)env->GetLongField(self, printdcID);	
    }

    inline static void setPrintDC(JNIEnv *env, jobject self, HDC printDC) {
      env->SetLongField(self, printdcID, (jlong)printDC);
    }

    inline static HGLOBAL getPrintHDMode(JNIEnv *env, jobject self) {
      return (HGLOBAL) env->GetLongField(self, devmodeID);
    }

    inline static void setPrintHDMode(JNIEnv *env, jobject self, 
				      HGLOBAL hGlobal) {      		   
      env->SetLongField(self, devmodeID, reinterpret_cast<jlong>(hGlobal));
    }

    inline static HGLOBAL getPrintHDName(JNIEnv *env, jobject self) {
      return (HGLOBAL) env->GetLongField(self, devnameID);
    }

    inline static void setPrintHDName(JNIEnv *env, jobject self, 
				      HGLOBAL hGlobal) {
      env->SetLongField(self, devnameID, reinterpret_cast<jlong>(hGlobal));
    }
    
};

#endif
