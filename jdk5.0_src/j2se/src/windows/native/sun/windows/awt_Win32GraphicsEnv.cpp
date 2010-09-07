/*
 * @(#)awt_Win32GraphicsEnv.cpp	1.19 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <windows.h>
#include <jni.h>
#include <awt.h>
#include <sun_awt_Win32GraphicsEnvironment.h>
#include "awt_Canvas.h"
#include "awt_Win32GraphicsDevice.h"
#include "MTSafeArray.h"
#include "dxInit.h"

int awt_numScreens;
static MHND *monHds;
MTSafeArray *devices = NULL;

void initScreens(JNIEnv *env) {
    awt_numScreens = ::CountMonitors();
    monHds = (MHND *)safe_Malloc(awt_numScreens * sizeof(MHND));
    AwtWin32GraphicsDevice **tempDevArray;
    if (awt_numScreens != ::CollectMonitors(monHds, awt_numScreens)) {
        JNU_ThrowInternalError(env, "Failed to get all monitor handles.");
    }

    if (devices) {
	devices->RemoveReference();
	// Lock access to devices array until we've created the new
	// array.  Prevents things like accessing out-of-date color
	// models and partially-created devices array.
    }
    MTSafeArray *tmpDevices = new MTSafeArray(awt_numScreens);
    // Add reference for the overall array - don't want to delete it just
    // because there are no surfaceData objects referencing it
    tmpDevices->AddReference();

    // Create all devices first, then initialize them.  This allows
    // correct configuration of devices after contruction of the
    // primary device (which may not be device 0).
    tempDevArray = (AwtWin32GraphicsDevice**)
	safe_Malloc(awt_numScreens * sizeof(AwtWin32GraphicsDevice));
    for (int i = 0; i < awt_numScreens; ++i) {
	tempDevArray[i] = new AwtWin32GraphicsDevice(i, tmpDevices);
    }
    for (i = 0; i < awt_numScreens; ++i) {
	tempDevArray[i]->Initialize();
	tmpDevices->AddElement(tempDevArray[i], i);
    }
    free(tempDevArray);
    devices = tmpDevices;
    InitDirectX();
}

HDC getDCFromScreen(int screen) {
    return MakeDCFromMonitor(monHds[screen]);
}

MHND getMHNDFromScreen(int screen) {
    return monHds[screen];
}

/* Compare elements of MONITOR_INFO structures for the given MHNDs.
 * If equal, return TRUE
 */
BOOL areSameMonitors(MHND mon1, MHND mon2) {
    DASSERT(mon1 != NULL);
    DASSERT(mon2 != NULL);

    MONITOR_INFO mi1; 
    MONITOR_INFO mi2; 

    memset((void*)(&mi1),0,sizeof(MONITOR_INFO));
    mi1.dwSize = sizeof(MONITOR_INFO);
    memset((void*)(&mi2),0,sizeof(MONITOR_INFO));
    mi2.dwSize = sizeof(MONITOR_INFO);

    if (::GetMonitorInfo(mon1,&mi1) != 0 &&
        ::GetMonitorInfo(mon2,&mi2) != 0 ) {

        if (::EqualRect(&mi1.rMonitor,&mi2.rMonitor) &&
            ::EqualRect(&mi1.rWork,&mi2.rWork) &&
            mi1.dwFlags  == mi1.dwFlags) {

            return TRUE;
        }
    }
    return FALSE;
}

int getScreenFromMHND(MHND mon) {
    DASSERT(mon != NULL);

    for (int i = 0; i < awt_numScreens; i++) {
        if (areSameMonitors(mon, getMHNDFromScreen(i))) {
            return i;
        }
    }
    DTRACE_PRINTLN("getScreenFromMHND(): couldn't find screen, return default");
    return AwtWin32GraphicsDevice::GetDefaultDeviceIndex();
}

/*
 * Class:     sun_awt_Win32GraphicsEnvironment
 * Method:    initDisplay
 * Signature: ()V
 */
JNIEXPORT void JNICALL
Java_sun_awt_Win32GraphicsEnvironment_initDisplay(JNIEnv *env,
                                                  jclass thisClass)
{
    initScreens(env);
}

/*
 * Class:     sun_awt_Win32GraphicsEnvironment
 * Method:    getNumScreens
 * Signature: ()I
 */
JNIEXPORT jint JNICALL
Java_sun_awt_Win32GraphicsEnvironment_getNumScreens(JNIEnv *env,
                                                    jobject thisobj)
{
    return awt_numScreens;
}

/*
 * Class:     sun_awt_Win32GraphicsEnvironment
 * Method:    getDefaultScreen
 * Signature: ()I
 */
JNIEXPORT jint JNICALL
Java_sun_awt_Win32GraphicsEnvironment_getDefaultScreen(JNIEnv *env,
						       jobject thisobj)
{
    return AwtWin32GraphicsDevice::GetDefaultDeviceIndex();
}

#define FR_PRIVATE 0x10 /* from wingdi.h */
typedef int (WINAPI *AddFontResourceExType)(LPCTSTR,DWORD,VOID*);
typedef int (WINAPI *RemoveFontResourceExType)(LPCTSTR,DWORD,VOID*);

static AddFontResourceExType procAddFontResourceEx = NULL;
static RemoveFontResourceExType procRemoveFontResourceEx = NULL;

static int winVer = -1;

static int getWinVer() {
    if (winVer == -1) {
        OSVERSIONINFO osvi;
        osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
        GetVersionEx(&osvi);
        winVer = osvi.dwMajorVersion;
	if (winVer >= 5) {
	  // REMIND verify on 64 bit windows
	  HMODULE hGDI = LoadLibrary(TEXT("gdi32.dll"));
	  if (hGDI != NULL) {
	    procAddFontResourceEx = 
	      (AddFontResourceExType)GetProcAddress(hGDI,"AddFontResourceExW");
	    if (procAddFontResourceEx == NULL) {
	      winVer = 0;
	    }
	    procRemoveFontResourceEx = 
	      (RemoveFontResourceExType)GetProcAddress(hGDI,
						      "RemoveFontResourceExW");
	    if (procRemoveFontResourceEx == NULL) {
	      winVer = 0;
	    }
	    FreeLibrary(hGDI);
	  }
	}
    }

    return winVer;
}

/*
 * Class:     sun_awt_Win32GraphicsEnvironment
 * Method:    registerFontWithPlatform
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_Win32GraphicsEnvironment_registerFontWithPlatform(JNIEnv *env,
							      jclass cl,
							      jstring fontName)
{
    if (getWinVer() >= 5 && procAddFontResourceEx != NULL) {
      LPTSTR file = (LPTSTR)JNU_GetStringPlatformChars(env, fontName, NULL);
      (*procAddFontResourceEx)(file, FR_PRIVATE, NULL);
    }
}


/*
 * Class:     sun_awt_Win32GraphicsEnvironment
 * Method:    deRegisterFontWithPlatform
 * Signature: (Ljava/lang/String;)V
 *
 * This method intended for future use.
 */
JNIEXPORT void JNICALL
Java_sun_awt_Win32GraphicsEnvironment_deRegisterFontWithPlatform(JNIEnv *env,
							      jclass cl,
							      jstring fontName)
{
    if (getWinVer() >= 5 && procRemoveFontResourceEx != NULL) {
      LPTSTR file = (LPTSTR)JNU_GetStringPlatformChars(env, fontName, NULL);
      (*procRemoveFontResourceEx)(file, FR_PRIVATE, NULL);
    }
}

#define EUDCKEY_JA_JP  L"EUDC\\932"
#define EUDCKEY_ZH_CN  L"EUDC\\936"
#define EUDCKEY_ZH_TW  L"EUDC\\950"
#define EUDCKEY_KO_KR  L"EUDC\\949"
#define LANGID_JA_JP   0x411
#define LANGID_ZH_CN   0x0804
#define LANGID_ZH_SG   0x1004
#define LANGID_ZH_TW   0x0404
#define LANGID_ZH_HK   0x0c04
#define LANGID_ZH_MO   0x1404
#define LANGID_KO_KR   0x0412


JNIEXPORT jstring JNICALL
Java_sun_awt_Win32GraphicsEnvironment_getEUDCFontFile(JNIEnv *env, jclass cl) {
    int	   rc;
    HKEY   key;
    DWORD  type;
    WCHAR  fontPathBuf[MAX_PATH + 1];
    unsigned long fontPathLen = MAX_PATH + 1;
    WCHAR  tmpPath[MAX_PATH + 1];
    LPWSTR fontPath = fontPathBuf;
    LPWSTR eudcKey = NULL;
      
    LANGID langID = GetSystemDefaultLangID();
    //lookup for encoding ID, EUDC only supported in
    //codepage 932, 936, 949, 950 (and unicode)
    if (langID == LANGID_JA_JP) {
	eudcKey = EUDCKEY_JA_JP;
    } else if (langID == LANGID_ZH_CN || langID == LANGID_ZH_SG) {
	eudcKey = EUDCKEY_ZH_CN;
    } else if (langID == LANGID_ZH_HK || langID == LANGID_ZH_TW ||
	       langID == LANGID_ZH_MO) {
      eudcKey = EUDCKEY_ZH_TW;
    } else if (langID == LANGID_KO_KR) {
	eudcKey = EUDCKEY_KO_KR;
    } else {
	return NULL;
    }

    rc = RegOpenKeyEx(HKEY_CURRENT_USER, eudcKey, 0, KEY_READ, &key);
    if (rc != ERROR_SUCCESS) {
	return NULL;
    }
    rc = RegQueryValueEx(key,
			 L"SystemDefaultEUDCFont",
			 0,
			 &type,
			 (LPBYTE)fontPath,
			 &fontPathLen);
    RegCloseKey(key);
    if (rc != ERROR_SUCCESS || type != REG_SZ) {
	return NULL;
    }
    fontPath[fontPathLen] = L'\0';
    if (wcsstr(fontPath, L"%SystemRoot%")) {
	//if the fontPath includes %SystemRoot%
	LPWSTR systemRoot = _wgetenv(L"SystemRoot");
	if (systemRoot != NULL
	    && swprintf(tmpPath, L"%s%s", systemRoot, fontPath + 12) != -1) {
	    fontPath = tmpPath;
	}
	else {
	    return NULL;
	}
    } else if (wcscmp(fontPath, L"EUDC.TTE") == 0) {
	//else to see if it only inludes "EUDC.TTE"
	WCHAR systemRoot[MAX_PATH + 1];
	if (GetWindowsDirectory(systemRoot, MAX_PATH + 1) != 0) {
	    swprintf(tmpPath, L"%s\\FONTS\\EUDC.TTE", systemRoot);
	    fontPath = tmpPath;
	}
	else {
	    return NULL;
	}
    }
    return JNU_NewStringPlatform(env, fontPath);
}
