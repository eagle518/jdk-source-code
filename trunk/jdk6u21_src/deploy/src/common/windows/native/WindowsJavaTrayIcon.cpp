/*
 * @(#)WindowsJavaTrayIcon.cpp	1.7 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <stdafx.h>
#include "com_sun_deploy_ui_WindowsJavaTrayIcon.h"
#import <shdocvw.dll>
#include <exdisp.h>

#define CURRENT_DLL_NAME "deploy.dll"

static JNIEnv* wndProcJNIEnv = NULL;
static jmethodID wndProcID = NULL;
static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    jobject obj = (jobject) ::GetWindowLongPtr(hWnd, GWLP_USERDATA);
    if (wndProcID == NULL) {
        return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    if (obj == NULL) {
        return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

    wndProcJNIEnv->ExceptionClear();
    LRESULT res = (LRESULT) wndProcJNIEnv->CallLongMethod(obj, wndProcID,
                                                          (jlong) hWnd,
                                                          (jint) uMsg,
                                                          (jlong) wParam,
                                                          (jlong) lParam);
    wndProcJNIEnv->ExceptionClear();
    return res;
}

/*
 * Class:     com_sun_deploy_ui_WindowsJavaTrayIcon
 * Method:    isEnabled0
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_com_sun_deploy_ui_WindowsJavaTrayIcon_isEnabled0
(JNIEnv *env, jclass unused, jstring javaVersion)
{
    const char* version = env->GetStringUTFChars(javaVersion, NULL);
    if (version == NULL) {
        // Assume we should show it
        return JNI_TRUE;
    }

    // NOTE: this replicates logic in src/plugin/win32/common/SysTrayReg.cpp
    CRegKey swKey, jsKey, jpiKey, verKey;
    // NOTE: we choose a different default value than in earlier releases
    // to get the icon to show up in more cases
    jboolean ret = JNI_TRUE;
    if (swKey.Open(HKEY_LOCAL_MACHINE, "SOFTWARE", KEY_READ) == ERROR_SUCCESS) {
        if (jsKey.Open(swKey, "JavaSoft", KEY_READ) == ERROR_SUCCESS) {
            if (jpiKey.Open(jsKey, "Java Plug-in", KEY_READ) == ERROR_SUCCESS) {
                if (verKey.Open(jpiKey, version, KEY_READ) == ERROR_SUCCESS) {
                    DWORD dwValue;
                    if (verKey.QueryValue(dwValue, "HideSystemTrayIcon") == ERROR_SUCCESS) {
                        ret = (dwValue > 0) ? JNI_FALSE : JNI_TRUE;
                    }
                }
            }
        }
    }
    env->ReleaseStringUTFChars(javaVersion, version);
    return ret;
}

/*
 * Class:     com_sun_deploy_ui_WindowsJavaTrayIcon
 * Method:    getCurrentProcessId
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_com_sun_deploy_ui_WindowsJavaTrayIcon_getCurrentProcessId
(JNIEnv *env, jclass unused)
{
    return ::GetCurrentProcessId();
}

/*
 * Class:     com_sun_deploy_ui_WindowsJavaTrayIcon
 * Method:    registerClass
 * Signature: (Ljava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_com_sun_deploy_ui_WindowsJavaTrayIcon_registerClass
(JNIEnv *env, jclass unused, jstring className)
{
    WNDCLASS wndClass;
    wndClass.style = 0; 
    wndClass.lpfnWndProc = WindowProc;
    wndClass.cbClsExtra = 0;
    wndClass.cbWndExtra = 0;
    wndClass.hInstance = GetModuleHandle(CURRENT_DLL_NAME);
    wndClass.hIcon = NULL;
    wndClass.hCursor = NULL;
    wndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wndClass.lpszMenuName = NULL;
    const char* classNameChars = env->GetStringUTFChars(className, NULL);
    jboolean ret = JNI_FALSE;
    if (classNameChars != NULL) {
        wndClass.lpszClassName = classNameChars;
        if (::RegisterClass(&wndClass) != 0) {
            ret = JNI_TRUE;
        }
        env->ReleaseStringUTFChars(className, classNameChars);
    }
    return ret;
}

/*
 * Class:     com_sun_deploy_ui_WindowsJavaTrayIcon
 * Method:    createWindow
 * Signature: (Ljava/lang/String;)J
 */
JNIEXPORT jlong JNICALL Java_com_sun_deploy_ui_WindowsJavaTrayIcon_createWindow
(JNIEnv *env, jclass unused, jstring className)
{
    const char* classNameChars = env->GetStringUTFChars(className, NULL);
    HWND hWnd = ::CreateWindow(classNameChars,
                               _T("Java Sys Tray"), 
                               WS_OVERLAPPEDWINDOW,
                               CW_USEDEFAULT,
                               CW_USEDEFAULT,
                               CW_USEDEFAULT,
                               CW_USEDEFAULT,
                               NULL,
                               NULL,
                               GetModuleHandle(CURRENT_DLL_NAME),
                               NULL);
    env->ReleaseStringUTFChars(className, classNameChars);
    return (jlong) hWnd;
}

/*
 * Class:     com_sun_deploy_ui_WindowsJavaTrayIcon
 * Method:    showWindow
 * Signature: (JI)V
 */
JNIEXPORT void JNICALL Java_com_sun_deploy_ui_WindowsJavaTrayIcon_showWindow
(JNIEnv *env, jclass unused, jlong hWnd, jint command)
{
    ::ShowWindow((HWND) hWnd, command);
}

/*
 * Class:     com_sun_deploy_ui_WindowsJavaTrayIcon
 * Method:    setUserData
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_com_sun_deploy_ui_WindowsJavaTrayIcon_setUserData
(JNIEnv *env, jobject jthis, jlong hWnd)
{
    ::SetWindowLongPtr((HWND) hWnd, GWLP_USERDATA, (LONG_PTR) env->NewGlobalRef(jthis));
}

/*
 * Class:     com_sun_deploy_ui_WindowsJavaTrayIcon
 * Method:    mainLoop
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_sun_deploy_ui_WindowsJavaTrayIcon_mainLoop
(JNIEnv *env, jclass clazz)
{
    wndProcJNIEnv = env;
    wndProcID = env->GetMethodID(clazz, "wndProc", "(JIJJ)J");
    if (wndProcID == NULL) {
        ::MessageBox(NULL, "wndProcID was NULL in mainLoop()", NULL, 0);
    }
    MSG msg;
    while(GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

/*
 * Class:     com_sun_deploy_ui_WindowsJavaTrayIcon
 * Method:    hasBalloonTooltipShown0
 * Signature: (Ljava/lang/String;Ljava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_com_sun_deploy_ui_WindowsJavaTrayIcon_hasBalloonTooltipShown0
(JNIEnv *env, jclass unused, jstring jrePathKey, jstring firstRunKey)
{
    // NOTE: this replicates logic in src/plugin/win32/jpishare/Utils.cpp
    const char* jrePathKeyChars  = env->GetStringUTFChars(jrePathKey,  NULL);
    const char* firstRunKeyChars = env->GetStringUTFChars(firstRunKey, NULL);
    jboolean ret = JNI_FALSE;
    if (jrePathKeyChars != NULL && firstRunKeyChars != NULL) {
	HKEY hKey;
	LONG lRet = ::RegOpenKey(HKEY_CURRENT_USER, jrePathKeyChars, &hKey);

	//  Key does not exist
	if (ERROR_SUCCESS != lRet) {
            lRet = ::RegCreateKey(HKEY_CURRENT_USER, jrePathKeyChars, &hKey);
	}

	if (ERROR_SUCCESS == lRet) {
            DWORD dwType;
            DWORD dwData;
            DWORD dwSize = sizeof(dwData);

            lRet = ::RegQueryValueEx(hKey, firstRunKeyChars, 
                                     NULL, &dwType, (LPBYTE)&dwData, &dwSize);
            ::RegCloseKey(hKey);

            if (ERROR_SUCCESS == lRet)
                ret = ((BOOL) dwData) ? JNI_TRUE : JNI_FALSE;
	}
    }

    if (jrePathKeyChars != NULL) {
        env->ReleaseStringUTFChars(jrePathKey, jrePathKeyChars);
    }
    if (firstRunKeyChars != NULL) {
        env->ReleaseStringUTFChars(firstRunKey, firstRunKeyChars);
    }
    return ret;
}

/*
 * Class:     com_sun_deploy_ui_WindowsJavaTrayIcon
 * Method:    setBalloonTooltipShown0
 * Signature: (Ljava/lang/String;Ljava/lang/String;Z)V
 */
JNIEXPORT void JNICALL Java_com_sun_deploy_ui_WindowsJavaTrayIcon_setBalloonTooltipShown0
(JNIEnv *env, jclass unused, jstring jrePathKey, jstring firstRunKey, jboolean shown)
{
    // NOTE: this replicates logic in src/plugin/win32/jpishare/Utils.cpp
    const char* jrePathKeyChars  = env->GetStringUTFChars(jrePathKey,  NULL);
    const char* firstRunKeyChars = env->GetStringUTFChars(firstRunKey, NULL);
    if (jrePathKeyChars != NULL && firstRunKeyChars != NULL) {
	HKEY hKey;
	LONG lRet = ::RegOpenKey(HKEY_CURRENT_USER, jrePathKeyChars, &hKey);

	// Key does not exist
	if (ERROR_SUCCESS != lRet) {
            lRet = ::RegCreateKey(HKEY_CURRENT_USER, jrePathKeyChars, &hKey);
	}

	if (ERROR_SUCCESS == lRet) {
            DWORD dwType = REG_DWORD;
            DWORD dwData = (DWORD) shown;
            DWORD dwSize = sizeof(dwData);

            lRet = ::RegSetValueEx(hKey, firstRunKeyChars,
                                   NULL, dwType, (LPBYTE)&dwData, dwSize);
            ::RegCloseKey(hKey);
	}
    }

    if (jrePathKeyChars != NULL) {
        env->ReleaseStringUTFChars(jrePathKey, jrePathKeyChars);
    }
    if (firstRunKeyChars != NULL) {
        env->ReleaseStringUTFChars(firstRunKey, firstRunKeyChars);
    }
}

typedef struct tagBitmapheader  {
    BITMAPINFOHEADER bmiHeader;
    DWORD            dwMasks[256];
} Bitmapheader, *LPBITMAPHEADER;

static HBITMAP CreateBMP(HWND hW, int* imageData,int nSS, int nW, int nH)
{
    Bitmapheader    bmhHeader;
    HDC             hDC;
    char            *ptrImageData;
    HBITMAP         hbmpBitmap;
    HBITMAP         hBitmap;
    int             nNumChannels    = 3;
    
    if (!hW) {
        hW = ::GetDesktopWindow();
    }
    hDC = ::GetDC(hW);
    if (!hDC) {
        return NULL;
    }
    
    memset(&bmhHeader, 0, sizeof(Bitmapheader));
    bmhHeader.bmiHeader.biSize              = sizeof(BITMAPINFOHEADER);
    bmhHeader.bmiHeader.biWidth             = nW;
    bmhHeader.bmiHeader.biHeight            = -nH;
    bmhHeader.bmiHeader.biPlanes            = 1;
    
    bmhHeader.bmiHeader.biBitCount          = 24;
    bmhHeader.bmiHeader.biCompression       = BI_RGB;
    
    hbmpBitmap = ::CreateDIBSection(hDC, (BITMAPINFO*)&(bmhHeader),
                                    DIB_RGB_COLORS,
                                    (void**)&(ptrImageData),
                                    NULL, 0);
    int  *srcPtr = imageData;
    char *dstPtr = ptrImageData;
    if (!dstPtr) {
        ReleaseDC(hW, hDC);
        return NULL;
    }
    for (int nOutern = 0; nOutern < nH; nOutern++) {
        for (int nInner = 0; nInner < nSS; nInner++) {
            dstPtr[2] = (*srcPtr >> 0x10) & 0xFF;
            dstPtr[1] = (*srcPtr >> 0x08) & 0xFF;
            dstPtr[0] = *srcPtr & 0xFF;
            
            srcPtr++;
            dstPtr += nNumChannels;
        }
    }
    
    // convert it into DDB to make CustomCursor work on WIN95
    hBitmap = CreateDIBitmap(hDC, 
                             (BITMAPINFOHEADER*)&bmhHeader,
                             CBM_INIT,
                             (void *)ptrImageData,
                             (BITMAPINFO*)&bmhHeader,
                             DIB_RGB_COLORS);
    
    ::DeleteObject(hbmpBitmap);
    ::ReleaseDC(hW, hDC);
    return hBitmap;
}

#define TRAY_ICON_X_HOTSPOT 0
#define TRAY_ICON_Y_HOTSPOT 0

/*
 * Class:     com_sun_deploy_ui_WindowsJavaTrayIcon
 * Method:    createNativeIcon
 * Signature: ([I[BIII)J
 */
JNIEXPORT jlong JNICALL Java_com_sun_deploy_ui_WindowsJavaTrayIcon_createNativeIcon
(JNIEnv *env, jclass unused, jintArray intRasterData, jbyteArray andMask, jint nSS, jint nW, jint nH)
{
    int length = env->GetArrayLength(andMask);
    jbyte *andMaskPtr = new jbyte[length];

    env->GetByteArrayRegion(andMask, 0, length, andMaskPtr);
    
    HBITMAP hMask = ::CreateBitmap(nW, nH, 1, 1, (BYTE *)andMaskPtr);

    delete[] andMaskPtr;
    
    jint *intRasterDataPtr = NULL;
    intRasterDataPtr = 
        (jint *)env->GetPrimitiveArrayCritical(intRasterData, 0);
    HBITMAP hColor = CreateBMP(NULL, (int *)intRasterDataPtr, nSS, nW, nH);
    env->ReleasePrimitiveArrayCritical(intRasterData, intRasterDataPtr, 0);
    intRasterDataPtr = NULL;
    
    HICON hIcon = NULL;
    
    if (hMask && hColor) {
        ICONINFO icnInfo;
        memset(&icnInfo, 0, sizeof(ICONINFO));
        icnInfo.hbmMask = hMask;
        icnInfo.hbmColor = hColor;
        icnInfo.fIcon = TRUE;
        icnInfo.xHotspot = TRAY_ICON_X_HOTSPOT;
        icnInfo.yHotspot = TRAY_ICON_Y_HOTSPOT;
        
        hIcon = ::CreateIconIndirect(&icnInfo);
    }
    ::DeleteObject(hColor);
    ::DeleteObject(hMask);
    return (jlong) hIcon;
}

/*
 * Class:     com_sun_deploy_ui_WindowsJavaTrayIcon
 * Method:    destroyIcon
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_com_sun_deploy_ui_WindowsJavaTrayIcon_destroyIcon
(JNIEnv *env, jclass unused, jlong hIcon)
{
    ::DestroyIcon((HICON) hIcon);
}

/*
 * Class:     com_sun_deploy_ui_WindowsJavaTrayIcon
 * Method:    createPopupMenu
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_com_sun_deploy_ui_WindowsJavaTrayIcon_createPopupMenu
(JNIEnv *env, jclass unused)
{
    return (jlong) ::CreatePopupMenu();
}

/*
 * Class:     com_sun_deploy_ui_WindowsJavaTrayIcon
 * Method:    appendMenu
 * Signature: (JIJLjava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_com_sun_deploy_ui_WindowsJavaTrayIcon_appendMenu
(JNIEnv *env, jclass unused, jlong hMenu, jint flags, jlong newItemID, jstring newItem)
{
    // Note explicit use of Unicode version to avoid having to deal with Java-to-MBCS conversion
    const jchar* newItemChars = NULL;
    if (newItem != NULL) {
        newItemChars = env->GetStringChars(newItem, NULL);
    }
    if (newItem == NULL || newItemChars != NULL) {
        ::AppendMenuW((HMENU) hMenu, (UINT) flags, (UINT_PTR) newItemID, newItemChars);
    }
    if (newItem != NULL && newItemChars != NULL) {
        env->ReleaseStringChars(newItem, newItemChars);
    }
}

/*
 * Class:     com_sun_deploy_ui_WindowsJavaTrayIcon
 * Method:    modifyMenu
 * Signature: (JIJLjava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_com_sun_deploy_ui_WindowsJavaTrayIcon_modifyMenu
(JNIEnv *env, jclass unused, jlong hMenu, jint oldItemID, jlong newItemID, jstring newItem)
{
    // Note explicit use of Unicode version to avoid having to deal with Java-to-MBCS conversion
    const jchar* newItemChars = NULL;
    if (newItem != NULL) {
        newItemChars = env->GetStringChars(newItem, NULL);
    }

    if ( newItemChars != NULL) {
        ::ModifyMenuW((HMENU)hMenu, (UINT)oldItemID, MF_BYCOMMAND|MF_STRING, (UINT_PTR)newItemID, newItemChars);
    }

    if (newItem != NULL && newItemChars != NULL) {
        env->ReleaseStringChars(newItem, newItemChars);
    }
}

/*
 * Class:     com_sun_deploy_ui_WindowsJavaTrayIcon
 * Method:    setMenuDefaultItem
 * Signature: (JIZ)Z
 */
JNIEXPORT jboolean JNICALL Java_com_sun_deploy_ui_WindowsJavaTrayIcon_setMenuDefaultItem
(JNIEnv *env, jclass unused, jlong hMenu, jint item, jboolean byPos)
{
    return (jboolean) ::SetMenuDefaultItem((HMENU) hMenu, (UINT) item, (UINT) byPos);
}

/*
 * Class:     com_sun_deploy_ui_WindowsJavaTrayIcon
 * Method:    showPopupMenu
 * Signature: (JJ)V
 */
JNIEXPORT void JNICALL Java_com_sun_deploy_ui_WindowsJavaTrayIcon_showPopupMenu
(JNIEnv *env, jclass unused, jlong hWnd, jlong hMenu)
{
    POINT point;
    ::GetCursorPos(&point);
    ::SetForegroundWindow((HWND) hWnd);
    ::TrackPopupMenu((HMENU) hMenu, TPM_RIGHTBUTTON, point.x, point.y, 0, (HWND) hWnd, NULL);
    ::PostMessage((HWND) hWnd, WM_NULL, 0, 0);
}

static HWND GetSystemTrayWnd() {
    HWND hWnd = ::FindWindowEx(NULL, NULL, "Shell_TrayWnd", NULL);
    if (NULL != hWnd)
        hWnd = ::FindWindowEx(hWnd, NULL, "TrayNotifyWnd", NULL);
    return hWnd;
}

/*
 * Class:     com_sun_deploy_ui_WindowsJavaTrayIcon
 * Method:    isBalloonClickInBounds
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_com_sun_deploy_ui_WindowsJavaTrayIcon_isBalloonClickInBounds
(JNIEnv *env, jclass unused)
{
    POINT	pt;
    RECT	rct;
    ::GetCursorPos(&pt);
    HWND h = GetSystemTrayWnd();
    ::GetWindowRect(h, &rct);
    if (pt.x < rct.left || pt.x > rct.right || pt.y < rct.top || pt.y > rct.bottom) {
        return JNI_TRUE;
    }
    return JNI_FALSE;
}

/*
 * Class:     com_sun_deploy_ui_WindowsJavaTrayIcon
 * Method:    showSysTray
 * Signature: (Z)V
 */
JNIEXPORT void JNICALL Java_com_sun_deploy_ui_WindowsJavaTrayIcon_showSysTray
(JNIEnv *env, jclass unused, jstring javaVersion, jboolean show)
{
    // NOTE: this replicates logic in src/plugin/win32/common/SysTrayReg.cpp
    // NOTE: this won't work on Windows Vista
    const char* javaVersionChars = env->GetStringUTFChars(javaVersion, NULL);
    if (javaVersionChars != NULL) {
        // Find JPI directory
        CRegKey swKey, jsKey, jpiKey, verKey;

        if (swKey.Open(HKEY_LOCAL_MACHINE, "SOFTWARE", KEY_READ) != ERROR_SUCCESS)
            return;

        if (jsKey.Open(swKey, "JavaSoft", KEY_READ) != ERROR_SUCCESS)
            return;

        if (jpiKey.Open(jsKey, "Java Plug-in", KEY_READ) != ERROR_SUCCESS)
            return;

        if (verKey.Open(jpiKey, javaVersionChars, KEY_READ|KEY_WRITE) != ERROR_SUCCESS)
            return;

        // Set HideSystemTrayIcon
        verKey.SetValue((DWORD)!show, "HideSystemTrayIcon");
    }
    env->ReleaseStringUTFChars(javaVersion, javaVersionChars);
}

////
//// Tray notification definitions
////
typedef struct _S_NOTIFYICONDATAA {
        DWORD cbSize;
        HWND hWnd;
        UINT uID;
        UINT uFlags;
        UINT uCallbackMessage;
        HICON hIcon;
        CHAR   szTip[128];
        DWORD dwState;
        DWORD dwStateMask;
        CHAR   szInfo[256];
        union {
            UINT  uTimeout;
            UINT  uVersion;
        } DUMMYUNIONNAME;
        CHAR   szInfoTitle[64];
        DWORD dwInfoFlags;
#if (_WIN32_IE >= 0x600)
        GUID guidItem;
#endif
} S_NOTIFYICONDATAA, *P_S_NOTIFYICONDATAA;

typedef struct _S_NOTIFYICONDATAW {
        DWORD cbSize;
        HWND hWnd;
        UINT uID;
        UINT uFlags;
        UINT uCallbackMessage;
        HICON hIcon;
        WCHAR  szTip[128];
        DWORD dwState;
        DWORD dwStateMask;
        WCHAR  szInfo[256];
        union {
            UINT  uTimeout;
            UINT  uVersion;
        } DUMMYUNIONNAME;
        WCHAR  szInfoTitle[64];
        DWORD dwInfoFlags;
#if (_WIN32_IE >= 0x600)
        GUID guidItem;
#endif
} S_NOTIFYICONDATAW, *P_S_NOTIFYICONDATAW;

#ifdef UNICODE
	typedef S_NOTIFYICONDATAW S_NOTIFYICONDATA;
	typedef P_S_NOTIFYICONDATAW P_S_NOTIFYICONDATA;
#else
	typedef S_NOTIFYICONDATAA S_NOTIFYICONDATA;
	typedef P_S_NOTIFYICONDATAA P_S_NOTIFYICONDATA;
#endif // UNICODE

/*
 * Class:     com_sun_deploy_ui_WindowsJavaTrayIcon
 * Method:    notifyShell
 * Signature: (JJIIIJLjava/lang/String;Ljava/lang/String;Ljava/lang/String;I)V
 */
JNIEXPORT void JNICALL Java_com_sun_deploy_ui_WindowsJavaTrayIcon_notifyShell
(JNIEnv *env, jclass unused, jlong message, jlong hWnd, jint id, jint flags,
 jint callbackMessage, jlong hIcon, jstring tooltip, jstring balloonTitle, jstring balloonText, jint timeout)
{
    // Note explicit use of Unicode version to avoid needing to convert Java strings to MBCS
    S_NOTIFYICONDATAW notifyIconData;
    ::ZeroMemory(&notifyIconData, sizeof(notifyIconData));
    notifyIconData.cbSize = sizeof(notifyIconData);
    notifyIconData.hWnd = (HWND) hWnd;
    notifyIconData.uID = id;
    notifyIconData.uFlags = flags;
    notifyIconData.uCallbackMessage = callbackMessage;
    notifyIconData.hIcon = (HICON) hIcon;
    if (tooltip != NULL) {
        const jchar* chars = env->GetStringChars(tooltip, NULL);
        if (chars != NULL) {
            wcsncpy(notifyIconData.szTip, chars, sizeof(notifyIconData.szTip) / sizeof(WCHAR));
            env->ReleaseStringChars(tooltip, chars);
        }
    }
    if (balloonTitle != NULL) {
        const jchar* chars = env->GetStringChars(balloonTitle, NULL);
        if (chars != NULL) {
            wcsncpy(notifyIconData.szInfoTitle, chars, sizeof(notifyIconData.szInfoTitle) / sizeof(WCHAR));
            env->ReleaseStringChars(balloonTitle, chars);
        }
    }
    if (balloonText != NULL) {
        const jchar* chars = env->GetStringChars(balloonText, NULL);
        if (chars != NULL) {
            wcsncpy(notifyIconData.szInfo, chars, sizeof(notifyIconData.szInfo) / sizeof(WCHAR));
            env->ReleaseStringChars(balloonText, chars);
        }
    }
    ::Shell_NotifyIconW(message, (NOTIFYICONDATAW*) &notifyIconData);
}

#ifndef MAX_BUFFER
#define MAX_BUFFER 1024
#endif

/*  Go one level up to get the parent directory
 *  In: Full Path 
 */
static void GetParentDir(char pathname[], char parentdir[]) {
    BOOLEAN bFirstTime = TRUE;
    for (int i = lstrlen(pathname) - 1; i >= 0; i--) {
        if (!bFirstTime) {
            parentdir[i] = pathname[i];
        }
        if ((pathname[i] == '\\') && bFirstTime) {
            parentdir[i] = '\0';
            bFirstTime = FALSE;
        }
    }
} 

/*
 * Class:     com_sun_deploy_ui_WindowsJavaTrayIcon
 * Method:    openControlPanel
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_sun_deploy_ui_WindowsJavaTrayIcon_openControlPanel
(JNIEnv *env, jclass unused)
{
    // Determine the module directory 	
    char szPathName[MAX_BUFFER];
    char szDirectory[MAX_BUFFER];
    char szCommandLine[MAX_BUFFER];

    HMODULE h = GetModuleHandle(CURRENT_DLL_NAME);
    // Obtain current module path
    GetModuleFileName(h, (LPSTR)szPathName, sizeof(szPathName));
    
    // Obtain directory of the module
    GetParentDir(szPathName, szDirectory);
    
    // Setup command line
    wsprintf(szCommandLine, "%s\\javacpl.exe", szDirectory);  

    SHELLEXECUTEINFO ShExecInfo = {0};
    ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
    ShExecInfo.fMask = 0;
    ShExecInfo.hwnd = NULL;
    ShExecInfo.lpVerb = NULL;
    ShExecInfo.lpFile = szCommandLine;
    ShExecInfo.lpParameters = "";
    ShExecInfo.lpDirectory = NULL;
    ShExecInfo.nShow = SW_SHOW;
    ShExecInfo.hInstApp = NULL;
    
    if (::ShellExecuteEx(&ShExecInfo) == FALSE) {
        ::MessageBox(NULL, szCommandLine, 
                     "Unable to Start Java(TM) Plug-in Control Panel", 
                     MB_ICONSTOP | MB_OK);
    }
}

/*
 * Class:     com_sun_deploy_ui_WindowsJavaTrayIcon
 * Method:    postQuitMessage
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_com_sun_deploy_ui_WindowsJavaTrayIcon_postQuitMessage
(JNIEnv *env, jclass unused, jint returnValue)
{
    ::PostQuitMessage(returnValue);
}

/*
 * Class:     com_sun_deploy_ui_WindowsJavaTrayIcon
 * Method:    defWindowProc
 * Signature: (JIJJ)J
 */
JNIEXPORT jlong JNICALL Java_com_sun_deploy_ui_WindowsJavaTrayIcon_defWindowProc
(JNIEnv *env, jclass unused, jlong hWnd, jint msg, jlong wParam, jlong lParam)
{
    return ::DefWindowProc((HWND) hWnd, (UINT) msg, (WPARAM) wParam, (LPARAM) lParam);
}
