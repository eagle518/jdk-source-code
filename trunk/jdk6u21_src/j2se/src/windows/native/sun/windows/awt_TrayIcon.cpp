/*
 * @(#)awt_TrayIcon.cpp	1.14 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved. Use is
 * subject to license terms.
 */

#include <windows.h>
#include <windowsx.h>
#include <shellapi.h>

#include "awt_Toolkit.h"
#include "awt_TrayIcon.h"
#include "awt_AWTEvent.h"

#include <java_awt_event_InputEvent.h>

/***********************************************************************/
// Struct for _SetToolTip() method
struct SetToolTipStruct {
    jobject trayIcon;
    jstring tooltip;
};
// Struct for _SetIcon() method
struct SetIconStruct {
    jobject trayIcon;
    HICON hIcon;
};
// Struct for _UpdateIcon() method
struct UpdateIconStruct {
    jobject trayIcon;
    jboolean update;
};
// Struct for _DisplayMessage() method
struct DisplayMessageStruct {
    jobject trayIcon;
    jstring caption;
    jstring text;
    jstring msgType;
};

typedef struct tagBitmapheader  {
    BITMAPINFOHEADER bmiHeader;
    DWORD            dwMasks[256];
} Bitmapheader, *LPBITMAPHEADER;


/************************************************************************
 * AwtTrayIcon fields
 */

jfieldID AwtTrayIcon::idID;
jfieldID AwtTrayIcon::actionCommandID;

HWND AwtTrayIcon::sm_msgWindow = NULL;
AwtTrayIcon::TrayIconListItem* AwtTrayIcon::sm_trayIconList = NULL;
int AwtTrayIcon::sm_instCount = 0;


/************************************************************************
 * AwtTrayIcon methods
 */

AwtTrayIcon::AwtTrayIcon() {
    ::ZeroMemory(&m_nid, sizeof(m_nid));

    if (sm_instCount++ == 0 && AwtTrayIcon::sm_msgWindow == NULL) {
        sm_msgWindow = AwtTrayIcon::CreateMessageWindow();
    }
}

AwtTrayIcon::~AwtTrayIcon() {
    SendTrayMessage(NIM_DELETE);
    UnlinkObjects();

    if (--sm_instCount == 0) {
        AwtTrayIcon::DestroyMessageWindow();
    }
}

LPCTSTR AwtTrayIcon::GetClassName() {
    return TEXT("SunAwtTrayIcon");
}

void AwtTrayIcon::FillClassInfo(WNDCLASS *lpwc)
{
    lpwc->style         = 0L;
    lpwc->lpfnWndProc   = (WNDPROC)TrayWindowProc;
    lpwc->cbClsExtra    = 0;
    lpwc->cbWndExtra    = 0;
    lpwc->hInstance     = AwtToolkit::GetInstance().GetModuleHandle(),
    lpwc->hIcon         = AwtToolkit::GetInstance().GetAwtIcon();
    lpwc->hCursor       = NULL;
    lpwc->hbrBackground = NULL;
    lpwc->lpszMenuName  = NULL;
    lpwc->lpszClassName = AwtTrayIcon::GetClassName();
}

void AwtTrayIcon::RegisterClass()
{
    WNDCLASS  wc;  

    ::ZeroMemory(&wc, sizeof(wc));

    if (!::GetClassInfo(AwtToolkit::GetInstance().GetModuleHandle(),
                        AwtTrayIcon::GetClassName(), &wc))
    {
        AwtTrayIcon::FillClassInfo(&wc);
        ATOM atom = ::RegisterClass(&wc);
        DASSERT(atom != 0);
    }
}

void AwtTrayIcon::UnregisterClass()
{
    ::UnregisterClass(AwtTrayIcon::GetClassName(), AwtToolkit::GetInstance().GetModuleHandle());
}

HWND AwtTrayIcon::CreateMessageWindow()
{
    AwtTrayIcon::RegisterClass();

    HWND hWnd = ::CreateWindow(AwtTrayIcon::GetClassName(), TEXT("TrayMessageWindow"),
                               0, 0, 0, 0, 0, NULL, NULL,
                               AwtToolkit::GetInstance().GetModuleHandle(), NULL);
    return hWnd;
}

void AwtTrayIcon::DestroyMessageWindow()
{
    ::DestroyWindow(AwtTrayIcon::sm_msgWindow);
    AwtTrayIcon::sm_msgWindow = NULL;
    AwtTrayIcon::UnregisterClass();
}

AwtTrayIcon* AwtTrayIcon::Create(jobject self, jobject parent)
{
    JNIEnv *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);
    jobject target = NULL;
    AwtTrayIcon* awtTrayIcon = NULL;

    target  = env->GetObjectField(self, AwtObject::targetID);
    DASSERT(target);

    awtTrayIcon = new AwtTrayIcon();
    awtTrayIcon->LinkObjects(env, self);
    awtTrayIcon->InitNID(env->GetIntField(target, AwtTrayIcon::idID));
    awtTrayIcon->AddTrayIconItem(awtTrayIcon->GetID());

    env->DeleteLocalRef(target);
    return awtTrayIcon;
}
typedef struct _SDLLVERSIONINFO
{
    DWORD cbSize;
    DWORD dwMajorVersion;                   // Major version
    DWORD dwMinorVersion;                   // Minor version
    DWORD dwBuildNumber;                    // Build number
    DWORD dwPlatformID;                     // DLLVER_PLATFORM_*
} SDLLVERSIONINFO;
typedef HRESULT (CALLBACK* SDLLGETVERSIONPROC)(SDLLVERSIONINFO *);

void AwtTrayIcon::InitNID(UINT uID)
{
    // fix for 6271589: we MUST set the size of the structure to match
    // the shell version, otherwise some errors may occur (like missing
    // balloon messages on win2k)
    SDLLVERSIONINFO dllVersionInfo;
    dllVersionInfo.cbSize = sizeof(SDLLVERSIONINFO);
    int shellVersion = 4; // WIN_98
    HMODULE hShell = LoadLibrary(TEXT("Shell32.dll"));
    if (hShell != NULL) {
        SDLLGETVERSIONPROC proc = (SDLLGETVERSIONPROC)GetProcAddress(hShell, "DllGetVersion");
        if (proc != NULL) {
            if (proc(&dllVersionInfo) == NOERROR) {
                shellVersion = dllVersionInfo.dwMajorVersion;
            }
        }
    }
    FreeLibrary(hShell);
    switch (shellVersion) {
        case 5: // WIN_2000, WIN_ME
            m_nid.cbSize = (BYTE *)(&m_nid.guidItem) - (BYTE *)(&m_nid.cbSize);
            break;
        case 6: // WIN_XP
            m_nid.cbSize = sizeof(m_nid);
            break;
        default: // WIN_98, WIN_NT
            m_nid.cbSize = (BYTE *)(&m_nid.szTip) - (BYTE *)(&m_nid.cbSize) + sizeof(m_nid.szTip) / 2;
    }
    m_nid.hWnd = AwtTrayIcon::sm_msgWindow;
    m_nid.uID = uID;
    m_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    m_nid.uCallbackMessage = WM_AWT_TRAY_NOTIFY;
    m_nid.hIcon = AwtToolkit::GetInstance().GetAwtIcon();
    m_nid.szTip[0] = '\0';
    m_nid.uVersion = IS_WIN2000 ? AWT_NOTIFYICON_VERSION : 0;

    SendTrayMessage(AWT_NIM_SETVERSION);
}

BOOL AwtTrayIcon::SendTrayMessage(DWORD dwMessage)
{
    return Shell_NotifyIcon(dwMessage, (PNOTIFYICONDATA)&m_nid);
}

static UINT lastMessage = WM_NULL;

LRESULT CALLBACK AwtTrayIcon::TrayWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT retValue = 0;
    MsgRouting mr = mrDoDefault;

    if(uMsg == WM_AWT_TRAY_NOTIFY && hwnd == AwtTrayIcon::sm_msgWindow)
    {
        AwtTrayIcon* trayIcon = AwtTrayIcon::SearchTrayIconItem((UINT)wParam);

        if (trayIcon != NULL) {
            POINT pos = {0, 0};
            ::GetCursorPos(&pos);

            lastMessage = (UINT)lParam;
        
            switch((UINT)lParam)
            {
            case WM_MOUSEMOVE:
                mr = trayIcon->WmMouseMove(0, pos.x, pos.y);
                break;
            case WM_LBUTTONDBLCLK:
            case WM_LBUTTONDOWN:
                mr = trayIcon->WmMouseDown(0, pos.x, pos.y, LEFT_BUTTON); 
                break;
            case WM_LBUTTONUP:
                mr = trayIcon->WmMouseUp(0, pos.x, pos.y, LEFT_BUTTON);
                break;
            case WM_RBUTTONDBLCLK:
            case WM_RBUTTONDOWN:
                mr = trayIcon->WmMouseDown(0, pos.x, pos.y, RIGHT_BUTTON);
                break; 
            case WM_RBUTTONUP:
                mr = trayIcon->WmMouseUp(0, pos.x, pos.y, RIGHT_BUTTON);
                break;
            case WM_MBUTTONDBLCLK:
            case WM_MBUTTONDOWN:
                mr = trayIcon->WmMouseDown(0, pos.x, pos.y, MIDDLE_BUTTON);
                break; 
            case WM_MBUTTONUP:
                mr = trayIcon->WmMouseUp(0, pos.x, pos.y, MIDDLE_BUTTON);
                break;
            case WM_CONTEXTMENU:
                break;
            case AWT_NIN_KEYSELECT:
                mr = trayIcon->WmKeySelect(0, pos.x, pos.y);
                break;
            case AWT_NIN_SELECT:
                mr = trayIcon->WmSelect(0, pos.x, pos.y);
                break;
            case AWT_NIN_BALLOONUSERCLICK:
                mr = trayIcon->WmBalloonUserClick(0, pos.x, pos.y);
                break;
            }
        }
    }
    
    if (mr != mrConsume) {
        retValue = ::DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return retValue;
}

/* Double-click variables. */
static jlong multiClickTime = ::GetDoubleClickTime();
static int multiClickMaxX = ::GetSystemMetrics(SM_CXDOUBLECLK);
static int multiClickMaxY = ::GetSystemMetrics(SM_CYDOUBLECLK);
static AwtTrayIcon* lastClickTrIc = NULL;
static jlong lastTime = 0;
static int lastClickX = 0;
static int lastClickY = 0;
static int lastButton = 0;
static int clickCount = 0;

MsgRouting AwtTrayIcon::WmMouseDown(UINT flags, int x, int y, int button)
{
    jlong now = TimeHelper::windowsToUTC(GetTickCount());
    jint javaModif = AwtComponent::GetJavaModifiers();

    if (lastClickTrIc == this &&  
        lastButton == button &&
        (now - lastTime) <= multiClickTime && 
        abs(x - lastClickX) <= multiClickMaxX &&
        abs(y - lastClickY) <= multiClickMaxY)
    {
        clickCount++;
    } else {
        clickCount = 1;
        lastClickTrIc = this;
        lastButton = button;
        lastClickX = x;
        lastClickY = y;
    }
    lastTime = now;

    MSG msg;
    AwtComponent::InitMessage(&msg, lastMessage, flags, MAKELPARAM(x, y), x, y);

    SendMouseEvent(java_awt_event_MouseEvent_MOUSE_PRESSED, now, x, y,
                   javaModif, clickCount, JNI_FALSE,
                   AwtComponent::GetButton(button), &msg);
/***
    Showing the popup in this way doesn't allow a user
    to navigate it with a keyboard.

    if (clickCount == 1 &&
        javaModif & java_awt_event_InputEvent_BUTTON3_DOWN_MASK)
    {
        JNIEnv *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);
        jobject peer = GetPeer(env);
        if (peer != NULL) {
            JNU_CallMethodByName(env, NULL, peer, "showPopupMenu", 
                                 "(II)V", x, y);
        }
    } else
***/


    /*
     * ActionEvent is generated when:
     * - left mouse button is clicked twice;
     * - SPACE key is pressed twice (Windows emulates left mouse button single click);
     * - ENTER key is pressed ones (Windows emulates left mouse button double click).
     * When SPACE or ENTER is pressed on the tray icon, they are sent by
     *   native system as WM_LBUTTONDOWN event(s) - see MSDN - so track them here.
     */
    if ((clickCount == 2) && (button == LEFT_BUTTON))
    {
        // No modifiers are printed in Java for ActionEvent.
        // The problem is in ActionEvent.paramString itself.
        // It calls KeyEvent.getKeyModifiersText() that operates
        // flag set which are too reductive and don't reflect
        // all the modifiers possible.
        SendActionEvent(java_awt_event_ActionEvent_ACTION_PERFORMED, now,
                        javaModif, &msg);
    }

    return mrConsume;
}

MsgRouting AwtTrayIcon::WmMouseUp(UINT flags, int x, int y, int button)
{
    MSG msg;
    AwtComponent::InitMessage(&msg, lastMessage, flags, MAKELPARAM(x, y), x, y);

    SendMouseEvent(java_awt_event_MouseEvent_MOUSE_RELEASED, TimeHelper::getMessageTimeUTC(), 
                   x, y, AwtComponent::GetJavaModifiers(), clickCount, 
                   (AwtComponent::GetButton(button) == java_awt_event_MouseEvent_BUTTON3 ?
                    TRUE : FALSE), AwtComponent::GetButton(button), &msg); 

    SendMouseEvent(java_awt_event_MouseEvent_MOUSE_CLICKED, 
                   TimeHelper::getMessageTimeUTC(), x, y, AwtComponent::GetJavaModifiers(), 
                   clickCount, JNI_FALSE, AwtComponent::GetButton(button));

    return mrConsume;
}

MsgRouting AwtTrayIcon::WmMouseMove(UINT flags, int x, int y)
{
    MSG msg;
    AwtComponent::InitMessage(&msg, lastMessage, flags, MAKELPARAM(x, y), x, y);
        
    SendMouseEvent(java_awt_event_MouseEvent_MOUSE_MOVED, TimeHelper::getMessageTimeUTC(), x, y,
                   AwtComponent::GetJavaModifiers(), 0, JNI_FALSE, 
                   java_awt_event_MouseEvent_NOBUTTON, &msg);

    return mrConsume;
}

MsgRouting AwtTrayIcon::WmBalloonUserClick(UINT flags, int x, int y)
{
    if (AwtComponent::GetJavaModifiers() & java_awt_event_InputEvent_BUTTON1_DOWN_MASK) {
        MSG msg;
        AwtComponent::InitMessage(&msg, lastMessage, flags, MAKELPARAM(x, y), x, y);
        SendActionEvent(java_awt_event_ActionEvent_ACTION_PERFORMED, TimeHelper::getMessageTimeUTC(),
                        AwtComponent::GetJavaModifiers(), &msg);
    }
    return mrConsume; 
}

MsgRouting AwtTrayIcon::WmKeySelect(UINT flags, int x, int y)
{
    MSG msg;
    AwtComponent::InitMessage(&msg, lastMessage, flags, MAKELPARAM(x, y), x, y);
    SendActionEvent(java_awt_event_ActionEvent_ACTION_PERFORMED, TimeHelper::getMessageTimeUTC(),
                    AwtComponent::GetJavaModifiers(), &msg);
    return mrConsume;    
}

MsgRouting AwtTrayIcon::WmSelect(UINT flags, int x, int y)
{
    MSG msg;
    AwtComponent::InitMessage(&msg, lastMessage, flags, MAKELPARAM(x, y), x, y);
    SendActionEvent(java_awt_event_ActionEvent_ACTION_PERFORMED, TimeHelper::getMessageTimeUTC(),
                    AwtComponent::GetJavaModifiers(), &msg);
    return mrConsume;    
}

void AwtTrayIcon::SendMouseEvent(jint id, jlong when, jint x, jint y, 
                                 jint modifiers, jint clickCount, 
                                 jboolean popupTrigger, jint button,
                                 MSG *pMsg)
{
    JNIEnv *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);
    if (GetPeer(env) == NULL) {
        /* event received during termination. */
        return;
    }

    static jclass mouseEventCls;
    if (mouseEventCls == NULL) {
        jclass mouseEventClsLocal = 
            env->FindClass("java/awt/event/MouseEvent");
        if (!mouseEventClsLocal) {
            /* exception already thrown */
            return;
        }
        mouseEventCls = (jclass)env->NewGlobalRef(mouseEventClsLocal);
        env->DeleteLocalRef(mouseEventClsLocal);
    }

    static jmethodID mouseEventConst;
    if (mouseEventConst == NULL) {
        mouseEventConst = 
            env->GetMethodID(mouseEventCls, "<init>", 
                             "(Ljava/awt/Component;IJIIIIIIZI)V");
        DASSERT(mouseEventConst);
    }
    if (env->EnsureLocalCapacity(2) < 0) {
        return;
    }
    jobject target = GetTarget(env);
    jobject mouseEvent = env->NewObject(mouseEventCls, mouseEventConst, 
                                        target,
                                        id, when, modifiers, 
                                        x, y, // no client area coordinates
                                        x, y,
                                        clickCount, popupTrigger, button);

    if (safe_ExceptionOccurred(env)) {
        env->ExceptionDescribe();
        env->ExceptionClear();
    }

    DASSERT(mouseEvent != NULL);
    if (pMsg != 0) {
        AwtAWTEvent::saveMSG(env, pMsg, mouseEvent);
    }
    SendEvent(mouseEvent);

    env->DeleteLocalRef(mouseEvent);
    env->DeleteLocalRef(target);
}

void AwtTrayIcon::SendActionEvent(jint id, jlong when, jint modifiers, MSG *pMsg)
{
    JNIEnv *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);
    if (GetPeer(env) == NULL) {
        /* event received during termination. */
        return;
    }

    static jclass actionEventCls;
    if (actionEventCls == NULL) {
        jclass actionEventClsLocal = 
            env->FindClass("java/awt/event/ActionEvent");
        if (!actionEventClsLocal) {
            /* exception already thrown */
            return;
        }
        actionEventCls = (jclass)env->NewGlobalRef(actionEventClsLocal);
        env->DeleteLocalRef(actionEventClsLocal);
    }

    static jmethodID actionEventConst;
    if (actionEventConst == NULL) {
        actionEventConst = 
            env->GetMethodID(actionEventCls, "<init>", 
                             "(Ljava/lang/Object;ILjava/lang/String;JI)V");
        DASSERT(actionEventConst);
    }
    if (env->EnsureLocalCapacity(2) < 0) {
        return;
    }
    jobject target = GetTarget(env);
    jstring actionCommand = (jstring)env->GetObjectField(target, AwtTrayIcon::actionCommandID);
    jobject actionEvent = env->NewObject(actionEventCls, actionEventConst, 
                                         target, id, actionCommand, when, modifiers);

    if (safe_ExceptionOccurred(env)) {
        env->ExceptionDescribe();
        env->ExceptionClear();
    }

    DASSERT(actionEvent != NULL);
    if (pMsg != 0) {
        AwtAWTEvent::saveMSG(env, pMsg, actionEvent);
    }
    SendEvent(actionEvent);

    env->DeleteLocalRef(actionEvent);
    env->DeleteLocalRef(target);
    env->DeleteLocalRef(actionCommand);
}

AwtTrayIcon* AwtTrayIcon::SearchTrayIconItem(UINT id) {
    TrayIconListItem* item;
    for (item = sm_trayIconList; item != NULL; item = item->m_next) {
        if (item->m_ID == id) {
            return item->m_trayIcon;
        }
    }
    /* 
     * DASSERT(FALSE);
     * This should not be happend if all tray icons are recorded
     */
    return NULL;
}

void AwtTrayIcon::RemoveTrayIconItem(UINT id) {
    TrayIconListItem* item = sm_trayIconList;
    TrayIconListItem* lastItem = NULL;
    while (item != NULL) {
        if (item->m_ID == id) {
            if (lastItem == NULL) {
                sm_trayIconList = item->m_next;
            } else {
                lastItem->m_next = item->m_next;
            }
            item->m_next = NULL;
            DASSERT(item != NULL);
            delete item;
            return;
        }
        lastItem = item;
        item = item->m_next;
    }
}

void AwtTrayIcon::LinkObjects(JNIEnv *env, jobject peer)
{
    if (m_peerObject == NULL) {
        m_peerObject = env->NewGlobalRef(peer);
    }

    /* Bind JavaPeer -> C++*/
    JNI_SET_PDATA(peer, this);
}

void AwtTrayIcon::UnlinkObjects()
{
    JNIEnv *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);
    if (m_peerObject) {
        JNI_SET_PDATA(m_peerObject, static_cast<PDATA>(NULL));
        env->DeleteGlobalRef(m_peerObject);
        m_peerObject = NULL;
    }
}

HBITMAP AwtTrayIcon::CreateBMP(HWND hW,int* imageData,int nSS, int nW, int nH)
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
//  ::GdiFlush();
    return hBitmap;
}

void AwtTrayIcon::SetToolTip(LPCTSTR tooltip)
{
    if (tooltip == NULL) {
        m_nid.szTip[0] = '\0';
    } else if (lstrlen(tooltip) > TRAY_ICON_TOOLTIP_MAX_SIZE) {
        _tcsncpy(m_nid.szTip, tooltip, TRAY_ICON_TOOLTIP_MAX_SIZE);
        m_nid.szTip[TRAY_ICON_TOOLTIP_MAX_SIZE - 1] = '\0';
    } else {
        _tcscpy(m_nid.szTip, tooltip);
    }

    SendTrayMessage(NIM_MODIFY);
}

void AwtTrayIcon::_SetToolTip(void *param)
{
    JNIEnv *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);
    SetToolTipStruct *sts = (SetToolTipStruct *)param;
    jobject self = sts->trayIcon;
    jstring jtooltip = sts->tooltip;
    AwtTrayIcon *trayIcon = NULL;
    LPCTSTR tooltipStr = NULL;

    PDATA pData;
    JNI_CHECK_PEER_GOTO(self, ret);
    trayIcon = (AwtTrayIcon *)pData;

    if (jtooltip == NULL) {
        trayIcon->SetToolTip(NULL);
        goto ret;
    }

    tooltipStr = env->GetStringChars(jtooltip, (jboolean *)NULL);
    trayIcon->SetToolTip(tooltipStr);
    env->ReleaseStringChars(jtooltip, tooltipStr);
ret:
    env->DeleteGlobalRef(self);
    env->DeleteGlobalRef(jtooltip);
    delete sts;
}

void AwtTrayIcon::SetIcon(HICON hIcon)
{
    ::DestroyIcon(m_nid.hIcon);
    m_nid.hIcon = hIcon;
}

void AwtTrayIcon::_SetIcon(void *param)
{
    JNIEnv *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);
    SetIconStruct *sis = (SetIconStruct *)param;
    jobject self = sis->trayIcon;
    HICON hIcon = sis->hIcon;
    AwtTrayIcon *trayIcon = NULL;

    PDATA pData;
    JNI_CHECK_PEER_GOTO(self, ret);
    trayIcon = (AwtTrayIcon *)pData;

    trayIcon->SetIcon(hIcon);

ret:
    env->DeleteGlobalRef(self);
    delete sis;
}

void AwtTrayIcon::_UpdateIcon(void *param)
{
    JNIEnv *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);
    UpdateIconStruct *uis = (UpdateIconStruct *)param;
    jobject self = uis->trayIcon;
    jboolean jupdate = uis->update;
    AwtTrayIcon *trayIcon = NULL;

    PDATA pData;
    JNI_CHECK_PEER_GOTO(self, ret);
    trayIcon = (AwtTrayIcon *)pData;

    trayIcon->SendTrayMessage(jupdate == TRUE ? NIM_MODIFY : NIM_ADD);

ret:
    env->DeleteGlobalRef(self);
    delete uis;
}

void AwtTrayIcon::DisplayMessage(LPCTSTR caption, LPCTSTR text, LPCTSTR msgType)
{
    if (!IS_WIN2000)
        return;

    m_nid.uFlags |= AWT_NIF_INFO;
    m_nid.uTimeout = 10000;

    if (lstrcmp(msgType, TEXT("ERROR")) == 0) {
        m_nid.dwInfoFlags = AWT_NIIF_ERROR;
    } else if (lstrcmp(msgType, TEXT("WARNING")) == 0) {
        m_nid.dwInfoFlags = AWT_NIIF_WARNING;
    } else if (lstrcmp(msgType, TEXT("INFO")) == 0) {
        m_nid.dwInfoFlags = AWT_NIIF_INFO;
    } else if (lstrcmp(msgType, TEXT("NONE")) == 0) {
        m_nid.dwInfoFlags = AWT_NIIF_NONE;
    } else {
        m_nid.dwInfoFlags = AWT_NIIF_NONE; 
    }

    if (caption[0] == '\0') {
        m_nid.szInfoTitle[0] = '\0';

    } else if (lstrlen(caption) > TRAY_ICON_BALLOON_TITLE_MAX_SIZE) {

        _tcsncpy(m_nid.szInfoTitle, caption, TRAY_ICON_BALLOON_TITLE_MAX_SIZE);
        m_nid.szInfoTitle[TRAY_ICON_BALLOON_TITLE_MAX_SIZE - 1] = '\0';

    } else {
        _tcscpy(m_nid.szInfoTitle, caption);
    }

    if (text[0] == '\0') {
        m_nid.szInfo[0] = ' ';
        m_nid.szInfo[1] = '\0';

    } else if (lstrlen(text) > TRAY_ICON_BALLOON_INFO_MAX_SIZE) {

        _tcsncpy(m_nid.szInfo, text, TRAY_ICON_BALLOON_INFO_MAX_SIZE);
        m_nid.szInfo[TRAY_ICON_BALLOON_INFO_MAX_SIZE - 1] = '\0';

    } else {
        _tcscpy(m_nid.szInfo, text);
    }

    SendTrayMessage(NIM_MODIFY);
    m_nid.uFlags &= ~AWT_NIF_INFO;
}

void AwtTrayIcon::_DisplayMessage(void *param)
{
    JNIEnv *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);
    DisplayMessageStruct *dms = (DisplayMessageStruct *)param;
    jobject self = dms->trayIcon;
    jstring jcaption = dms->caption;
    jstring jtext = dms-> text;
    jstring jmsgType = dms->msgType;
    AwtTrayIcon *trayIcon = NULL;
    LPCTSTR captionStr = NULL;
    LPCTSTR textStr = NULL;
    LPCTSTR msgTypeStr = NULL;

    PDATA pData;
    JNI_CHECK_PEER_GOTO(self, ret);
    trayIcon = (AwtTrayIcon *)pData;

    captionStr = env->GetStringChars(jcaption, (jboolean *)NULL);
    textStr = env->GetStringChars(jtext, (jboolean *)NULL);
    msgTypeStr = env->GetStringChars(jmsgType, (jboolean *)NULL);

    trayIcon->DisplayMessage(captionStr, textStr, msgTypeStr);

    env->ReleaseStringChars(jcaption, captionStr);
    env->ReleaseStringChars(jtext, textStr);
    env->ReleaseStringChars(jmsgType, msgTypeStr);
ret:
    env->DeleteGlobalRef(self);
    env->DeleteGlobalRef(jcaption);
    env->DeleteGlobalRef(jtext);
    env->DeleteGlobalRef(jmsgType);
    delete dms;
}

/************************************************************************
 * TrayIcon native methods
 */

extern "C" {

/*
 * Class:     java_awt_TrayIcon
 * Method:    initIDs
 * Signature: ()V
 */
JNIEXPORT void JNICALL
Java_java_awt_TrayIcon_initIDs(JNIEnv *env, jclass cls)
{
    TRY;

    /* init field ids */
    AwtTrayIcon::idID = env->GetFieldID(cls, "id", "I");
    AwtTrayIcon::actionCommandID = env->GetFieldID(cls, "actionCommand", "Ljava/lang/String;");

    DASSERT(AwtTrayIcon::idID != NULL);
    DASSERT(AwtTrayIcon::actionCommandID != NULL);

    CATCH_BAD_ALLOC;
}

/*
 * Class:     sun_awt_windows_WTrayIconPeer
 * Method:    create
 * Signature: ()V
 */
JNIEXPORT void JNICALL
Java_sun_awt_windows_WTrayIconPeer_create(JNIEnv *env, jobject self)
{
    TRY;

    AwtToolkit::CreateComponent(self, NULL,
                                (AwtToolkit::ComponentFactory)
                                AwtTrayIcon::Create);
    PDATA pData;
    JNI_CHECK_PEER_CREATION_RETURN(self);

    CATCH_BAD_ALLOC;
}

/*
 * Class:     sun_awt_windows_WTrayIconPeer
 * Method:    _dispose
 * Signature: ()V
 */
JNIEXPORT void JNICALL 
Java_sun_awt_windows_WTrayIconPeer__1dispose(JNIEnv *env, jobject self)
{
    TRY;

    jobject selfGlobalRef = env->NewGlobalRef(self);
    AwtToolkit::GetInstance().SyncCall(AwtObject::_Dispose, selfGlobalRef);
    // selfGlobalRef is disposed in AwtToolkit::WndProc

    CATCH_BAD_ALLOC;
}

/*
 * Class:     sun_awt_windows_WTrayIconPeer
 * Method:    _setToolTip
 * Signature: ()V
 */
JNIEXPORT void JNICALL 
Java_sun_awt_windows_WTrayIconPeer_setToolTip(JNIEnv *env, jobject self,
                                              jstring tooltip)
{
    TRY;

    SetToolTipStruct *sts = new SetToolTipStruct;
    sts->trayIcon = env->NewGlobalRef(self);
    if (tooltip != NULL) {
        sts->tooltip = (jstring)env->NewGlobalRef(tooltip);
    } else {
        sts->tooltip = NULL;
    }

    AwtToolkit::GetInstance().SyncCall(AwtTrayIcon::_SetToolTip, sts);
    // global ref and sts are deleted in _SetToolTip

    CATCH_BAD_ALLOC;
}

/*
 * Class:     sun_awt_windows_WTrayIconPeer
 * Method:    setNativeIcon
 * Signature: (I[B[IIIII)V
 */
JNIEXPORT void JNICALL 
Java_sun_awt_windows_WTrayIconPeer_setNativeIcon(JNIEnv *env, jobject self,
                                                 jintArray intRasterData, jbyteArray andMask, 
                                                 jint nSS, jint nW, jint nH)
{
    TRY;

    int length = env->GetArrayLength(andMask);
    jbyte *andMaskPtr = new jbyte[length];

    env->GetByteArrayRegion(andMask, 0, length, andMaskPtr);
    
    HBITMAP hMask = ::CreateBitmap(nW, nH, 1, 1, (BYTE *)andMaskPtr);
//    ::GdiFlush();

    delete[] andMaskPtr;
    
    jint *intRasterDataPtr = NULL;
    HBITMAP hColor = NULL;
    try {
        intRasterDataPtr = 
            (jint *)env->GetPrimitiveArrayCritical(intRasterData, 0);
        hColor = AwtTrayIcon::CreateBMP(NULL, (int *)intRasterDataPtr, nSS, nW, nH);
    } catch (...) {
        if (intRasterDataPtr != NULL) {
            env->ReleasePrimitiveArrayCritical(intRasterData, intRasterDataPtr, 0);
        }
        ::DeleteObject(hMask);
        throw;
    }
    
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

    //////////////////////////////////////////

    SetIconStruct *sis = new SetIconStruct;
    sis->trayIcon = env->NewGlobalRef(self);
    sis->hIcon = hIcon;

    AwtToolkit::GetInstance().SyncCall(AwtTrayIcon::_SetIcon, sis);
    // global ref is deleted in _SetIcon

    CATCH_BAD_ALLOC;
}

/*
 * Class:     sun_awt_windows_WTrayIconPeer
 * Method:    updateNativeIcon
 * Signature: (Z)V
 */
JNIEXPORT void JNICALL 
Java_sun_awt_windows_WTrayIconPeer_updateNativeIcon(JNIEnv *env, jobject self,
                                                    jboolean doUpdate)
{
    TRY;

    UpdateIconStruct *uis = new UpdateIconStruct;
    uis->trayIcon = env->NewGlobalRef(self);
    uis->update = doUpdate;

    AwtToolkit::GetInstance().SyncCall(AwtTrayIcon::_UpdateIcon, uis);
    // global ref is deleted in _UpdateIcon

    CATCH_BAD_ALLOC;
}

/*
 * Class:     sun_awt_windows_WTrayIconPeer
 * Method:    displayMessage
 * Signature: ()V;
 */
JNIEXPORT void JNICALL 
Java_sun_awt_windows_WTrayIconPeer__1displayMessage(JNIEnv *env, jobject self,
    jstring caption, jstring text, jstring msgType)
{
    TRY;

    if (IS_WIN2000) {
        DisplayMessageStruct *dms = new DisplayMessageStruct;
        dms->trayIcon = env->NewGlobalRef(self);
        dms->caption = (jstring)env->NewGlobalRef(caption);
        dms->text = (jstring)env->NewGlobalRef(text);
        dms->msgType = (jstring)env->NewGlobalRef(msgType);
        
        AwtToolkit::GetInstance().SyncCall(AwtTrayIcon::_DisplayMessage, dms);
        // global ref is deleted in _DisplayMessage
    }

    CATCH_BAD_ALLOC(NULL);
}

} /* extern "C" */
