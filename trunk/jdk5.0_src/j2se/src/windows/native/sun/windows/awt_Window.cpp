/*
 * @(#)awt_Window.cpp	1.122 04/06/16
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <windowsx.h>

#include "awt_Component.h"
#include "awt_Container.h"
#include "awt_Frame.h"
#include "awt_Insets.h"
#include "awt_Panel.h"
#include "awt_Toolkit.h"
#include "awt_Window.h"
#include "awt_dlls.h"
#include "ddrawUtils.h"
#include "awt_Win32GraphicsDevice.h"

#include "java_awt_Insets.h"
#include <java_awt_Container.h>
#include <java_awt_event_ComponentEvent.h>
#include "sun_awt_windows_WCanvasPeer.h"

int getScreenFromMHND(MHND mon);

// Used for Swing's Menu/Tooltip animation Support
const int UNSPECIFIED = 0;
const int TOOLTIP = 1;
const int MENU = 2;
const int SUBMENU = 3;
const int POPUPMENU = 4;
const int COMBOBOX_POPUP = 5;
const int TYPES_COUNT = 6;
jint windowTYPES[TYPES_COUNT];


/* IMPORTANT! Read the README.JNI file for notes on JNI converted AWT code.
 */

/************************************************************************
 * AwtWindow fields
 */

jfieldID AwtWindow::warningStringID;
jfieldID AwtWindow::focusableWindowID;
jfieldID AwtWindow::locationByPlatformID;

/************************************************************************
 * AwtWindow class methods
 */

AwtWindow::AwtWindow() {
    m_resizing = FALSE;
    m_sizePt.x = m_sizePt.y = 0;
    m_owningFrameDialog = NULL;
    VERIFY(::SetRectEmpty(&m_insets));
    VERIFY(::SetRectEmpty(&m_old_insets));
    VERIFY(::SetRectEmpty(&m_warningRect));

    // what's the best initial value?
    m_screenNum = -1;
}

AwtWindow::~AwtWindow() {
}

LPCTSTR AwtWindow::GetClassName() {
  return TEXT("SunAwtWindow");
}

void AwtWindow::FillClassInfo(WNDCLASS *lpwc)
{
    AwtComponent::FillClassInfo(lpwc);
    /*
     * This line causes bug #4189244 (Swing Popup menu is not being refreshed (cleared) under a Dialog)
     * so it's comment out (son@sparc.spb.su)
     *
     * lpwc->style     |= CS_SAVEBITS; // improve pull-down menu performance
     */
    lpwc->cbWndExtra = DLGWINDOWEXTRA;
}


/* Create a new AwtWindow object and window.   */
AwtWindow* AwtWindow::Create(jobject self, jobject parent)
{
    JNIEnv *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);

    jobject target = NULL;
    AwtWindow* window = NULL;

    try {
        if (env->EnsureLocalCapacity(1) < 0) {
            return NULL;
        }

        PDATA pData;
        AwtWindow* awtParent;
        JNI_CHECK_PEER_GOTO(parent, done);

        awtParent = (AwtWindow*)pData;
        JNI_CHECK_NULL_GOTO(awtParent, "null awtParent", done);

        target = env->GetObjectField(self, AwtObject::targetID);
        JNI_CHECK_NULL_GOTO(target, "null target", done);

        window = new AwtWindow();

        {
            DWORD style = WS_CLIPCHILDREN | WS_POPUP;
            DWORD exStyle = 0;
            if (GetRTL()) {
                exStyle |= WS_EX_RIGHT | WS_EX_LEFTSCROLLBAR;
                if (GetRTLReadingOrder())
                    exStyle |= WS_EX_RTLREADING;
            }
            window->InitOwner(awtParent);
            window->CreateHWnd(env, L"",
                               style, exStyle,
                               0, 0, 0, 0,
                               awtParent->GetHWnd(),
                               NULL,
                               ::GetSysColor(COLOR_WINDOWTEXT),
                               ::GetSysColor(COLOR_WINDOW),
                               self);

            jint x = env->GetIntField(target, AwtComponent::xID);
            jint y = env->GetIntField(target, AwtComponent::yID);
            jint width = env->GetIntField(target, AwtComponent::widthID);
            jint height = env->GetIntField(target, AwtComponent::heightID);

            /* 
             * Reshape here instead of during create, so that a WM_NCCALCSIZE
             * is sent. 
             */
            window->Reshape(x, y, width, height);
        }
    } catch (...) {
        env->DeleteLocalRef(target);
        throw;
    }

done:
    env->DeleteLocalRef(target);
    return window;
}

void AwtWindow::InitOwner(AwtWindow *owner)
{
    DASSERT(owner != NULL);
    while (owner != NULL &&
        _tcscmp(owner->GetClassName(), TEXT("SunAwtWindow")) == 0) {

        HWND ownerOwnerHWND = ::GetWindow(owner->GetHWnd(), GW_OWNER);
        if (ownerOwnerHWND == NULL) {
            owner = NULL;
            break;
        }
        owner = (AwtWindow *)AwtComponent::GetComponent(ownerOwnerHWND);        
    }
    m_owningFrameDialog = (AwtFrame *)owner;
}

//
// Override AwtComponent's Reshape to handle minimized/maximized
// windows, fixes 4065534 (robi.khan@eng).
// NOTE: See also AwtWindow::WmSize
//
void AwtWindow::Reshape(int x, int y, int width, int height) 
{
    HWND    hWndSelf = GetHWnd();

    if ( IsIconic(hWndSelf)) {
    // normal AwtComponent::Reshape will not work for iconified windows so...
        WINDOWPLACEMENT wp;
        POINT       ptMinPosition = {x,y};
        POINT       ptMaxPosition = {0,0};
        RECT        rcNormalPosition = {x,y,x+width,y+height};
        RECT        rcWorkspace;
        HWND        hWndDesktop = GetDesktopWindow();

        // SetWindowPlacement takes workspace coordinates, but
        // if taskbar is at top of screen, workspace coords !=
        // screen coords, so offset by workspace origin
        VERIFY(::SystemParametersInfo(SPI_GETWORKAREA, 0, (PVOID)&rcWorkspace, 0));
        ::OffsetRect(&rcNormalPosition, -rcWorkspace.left, -rcWorkspace.top);

        // set the window size for when it is not-iconified
        wp.length = sizeof(wp);
        wp.flags = WPF_SETMINPOSITION;
        wp.showCmd = SW_SHOWNA;
        wp.ptMinPosition = ptMinPosition;
        wp.ptMaxPosition = ptMaxPosition;
        wp.rcNormalPosition = rcNormalPosition;
        SetWindowPlacement(hWndSelf, &wp);
        return;
    }
    
    if (IsZoomed(hWndSelf)) {
    // setting size of maximized window, we remove the
    // maximized state bit (matches Motif behaviour)
    // (calling ShowWindow(SW_RESTORE) would fire an
    //  activation event which we don't want)
        LONG    style = GetStyle();
        DASSERT(style & WS_MAXIMIZE);
        style ^= WS_MAXIMIZE;
        SetStyle(style);
    }

    AwtComponent::Reshape(x, y, width, height);
}

void AwtWindow::moveToDefaultLocation() {
    HWND boggy = ::CreateWindow(GetClassName(), L"BOGGY", WS_OVERLAPPED, CW_USEDEFAULT, 0 ,0, 0, 
        NULL, NULL, NULL, NULL);
    RECT defLoc;
    VERIFY(::GetWindowRect(boggy, &defLoc));
    VERIFY(::DestroyWindow(boggy));
    VERIFY(::SetWindowPos(GetHWnd(), NULL, defLoc.left, defLoc.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER));
}

void AwtWindow::Show()
{
    JNIEnv *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);
    BOOL  done = false;
    HWND hWnd = GetHWnd();

    if (env->EnsureLocalCapacity(2) < 0) {
        return;
    }
    jobject target = GetTarget(env);
    INT nCmdShow;

    BOOL focusableWindow =
        (env->GetBooleanField(GetPeer(env), AwtWindow::focusableWindowID) ==
         JNI_TRUE);

    AwtFrame* owningFrame = GetOwningFrameOrDialog();
    if (focusableWindow &&
        owningFrame != NULL &&
        ::GetForegroundWindow() == owningFrame->GetHWnd())
    {
        nCmdShow = SW_SHOW;
    } else {
        nCmdShow = SW_SHOWNA;
    }

    BOOL locationByPlatform = env->GetBooleanField(GetTarget(env), AwtWindow::locationByPlatformID);

    if (locationByPlatform) {
         moveToDefaultLocation();
    }
    
    // The following block exists to support Menu/Tooltip animation for
    // Swing programs in a way which avoids introducing any new public api into 
    // AWT or Swing.
    // This code should eventually be replaced by a better longterm solution
    // which might involve tagging java.awt.Window instances with a semantic
    // property so platforms can animate/decorate/etc accordingly.
    //
    if ((IS_WIN98 || IS_WIN2000) &&
        JNU_IsInstanceOfByName(env, target, "com/sun/java/swing/plaf/windows/WindowsPopupWindow") > 0) {
        static jfieldID windowTypeFID = NULL;
        jint windowType = 0;
        BOOL  animateflag = FALSE;
        BOOL  fadeflag = FALSE;
            DWORD animateStyle = 0;

        if (windowTypeFID == NULL) {
            // Initialize Window type constants ONCE...

            jfieldID windowTYPESFID[TYPES_COUNT];
            jclass cls = env->GetObjectClass(target);
            windowTypeFID = env->GetFieldID(cls, "windowType", "I");

            windowTYPESFID[UNSPECIFIED] = env->GetStaticFieldID(cls, "UNDEFINED_WINDOW_TYPE", "I");
            windowTYPESFID[TOOLTIP] = env->GetStaticFieldID(cls, "TOOLTIP_WINDOW_TYPE", "I");
            windowTYPESFID[MENU] = env->GetStaticFieldID(cls, "MENU_WINDOW_TYPE", "I");
            windowTYPESFID[SUBMENU] = env->GetStaticFieldID(cls, "SUBMENU_WINDOW_TYPE", "I");
            windowTYPESFID[POPUPMENU] = env->GetStaticFieldID(cls, "POPUPMENU_WINDOW_TYPE", "I");
            windowTYPESFID[COMBOBOX_POPUP] = env->GetStaticFieldID(cls, "COMBOBOX_POPUP_WINDOW_TYPE", "I");

            for (int i=0; i < 6; i++) {
                windowTYPES[i] = env->GetStaticIntField(cls, windowTYPESFID[i]);
            }
            env->DeleteLocalRef(cls);
        }
        windowType = env->GetIntField(target, windowTypeFID);

        if (windowType == windowTYPES[TOOLTIP]) {
            if (IS_WIN2000) {
                SystemParametersInfo(SPI_GETTOOLTIPANIMATION, 0, &animateflag, 0);
                SystemParametersInfo(SPI_GETTOOLTIPFADE, 0, &fadeflag, 0);
            } else {
                // use same setting as menus
                SystemParametersInfo(SPI_GETMENUANIMATION, 0, &animateflag, 0);
            }
            if (animateflag) {
              // AW_BLEND currently produces runtime parameter error
              // animateStyle = fadeflag? AW_BLEND : AW_SLIDE | AW_VER_POSITIVE;
                 animateStyle = fadeflag? 0 : AW_SLIDE | AW_VER_POSITIVE;
            }
        } else if (windowType == windowTYPES[MENU] || windowType == windowTYPES[SUBMENU] || 
                   windowType == windowTYPES[POPUPMENU]) {
            SystemParametersInfo(SPI_GETMENUANIMATION, 0, &animateflag, 0);
            if (animateflag) {

                if (IS_WIN2000) {
                    SystemParametersInfo(SPI_GETMENUFADE, 0, &fadeflag, 0);
                    if (fadeflag) {
                      // AW_BLEND currently produces runtime parameter error
                      //animateStyle = AW_BLEND;                      
                    } 
                }
                if (animateStyle == 0 && !fadeflag) {
                    animateStyle = AW_SLIDE;
                    if (windowType == windowTYPES[MENU]) {
                      animateStyle |= AW_VER_POSITIVE;
                    } else if (windowType == windowTYPES[SUBMENU]) {
                      animateStyle |= AW_HOR_POSITIVE;
                    } else { /* POPUPMENU */
                      animateStyle |= (AW_VER_POSITIVE | AW_HOR_POSITIVE);
                    }
                }
            }
        } else if (windowType == windowTYPES[COMBOBOX_POPUP]) {
            SystemParametersInfo(SPI_GETCOMBOBOXANIMATION, 0, &animateflag, 0);
            if (animateflag) {
                 animateStyle = AW_SLIDE | AW_VER_POSITIVE;
            }
        } 

        if (animateStyle != 0) {
            load_user_procs();

            if (fn_animate_window != NULL) {
                BOOL result = (*fn_animate_window)(hWnd, (DWORD)200, animateStyle);
                if (result == 0) {
                    LPTSTR      msgBuffer = NULL;
                    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                      FORMAT_MESSAGE_FROM_SYSTEM |
                      FORMAT_MESSAGE_IGNORE_INSERTS,
                      NULL,
                      GetLastError(),
                      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                      (LPTSTR)&msgBuffer, // it's an output parameter when allocate buffer is used
                      0,
                      NULL);

                    if (msgBuffer == NULL) {
                        msgBuffer = TEXT("<Could not get GetLastError() message text>");
                    }
                    _ftprintf(stderr,TEXT("AwtWindow::Show: AnimateWindow: "));
                    _ftprintf(stderr,msgBuffer);
                    LocalFree(msgBuffer);
                } else {
                  // WM_PAINT is not automatically sent when invoking AnimateWindow,
                  // so force an expose event
                    RECT rect;
                    ::GetWindowRect(hWnd,&rect);
                    ::ScreenToClient(hWnd, (LPPOINT)&rect);
                    ::InvalidateRect(hWnd,&rect,TRUE);
                    ::UpdateWindow(hWnd); 
                    done = TRUE;
                }
            }
        } 
    }
    if (!done) {
        ::ShowWindow(GetHWnd(), nCmdShow);
    }
    env->DeleteLocalRef(target);
}

/*
 * Get and return the insets for this window (container, really).
 * Calculate & cache them while we're at it, for use by AwtGraphics
 */
BOOL AwtWindow::UpdateInsets(jobject insets)
{
    JNIEnv *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);
    DASSERT(GetPeer(env) != NULL);
    if (env->EnsureLocalCapacity(2) < 0) {
        return FALSE;
    }

    // fix 4167248 : don't update insets when frame is iconified
    // to avoid bizarre window/client rectangles
    if (::IsIconic(GetHWnd())) {
        return FALSE;
    }

    /*
     * Code to calculate insets. Stores results in frame's data
     * members, and in the peer's Inset object.
     */
    RECT outside;
    RECT inside;
    int extraBottomInsets = 0;

    ::GetClientRect(GetHWnd(), &inside);
    ::GetWindowRect(GetHWnd(), &outside);

    jobject target = GetTarget(env);
    jstring warningString = 
      (jstring)(env)->GetObjectField(target, AwtWindow::warningStringID);
    if (warningString != NULL) {
        ::CopyRect(&inside, &outside);
        DefWindowProc(WM_NCCALCSIZE, FALSE, (LPARAM)&inside);
        /*
         * Fix for BugTraq ID 4304024.
         * Calculate client rectangle in client coordinates. 
         */
        VERIFY(::OffsetRect(&inside, -inside.left, -inside.top));
        extraBottomInsets = ::GetSystemMetrics(SM_CYCAPTION) +
            ((GetStyle() & WS_THICKFRAME) ? 2 : -2);
    }
    env->DeleteLocalRef(target);
    env->DeleteLocalRef(warningString);

    /* Update our inset member */
    if (outside.right - outside.left > 0 && outside.bottom - outside.top > 0) {
        ::MapWindowPoints(GetHWnd(), 0, (LPPOINT)&inside, 2);
        m_insets.top = inside.top - outside.top;
        m_insets.bottom = outside.bottom - inside.bottom + extraBottomInsets;
        m_insets.left = inside.left - outside.left;
        m_insets.right = outside.right - inside.right;
    } else {
        m_insets.top = -1;
    }
    if (m_insets.left < 0 || m_insets.top < 0 ||
        m_insets.right < 0 || m_insets.bottom < 0)
    {
        /* This window hasn't been sized yet -- use system metrics. */
        jobject target = GetTarget(env);
        if (JNU_IsInstanceOfByName(env, target, "java/awt/Frame") > 0 ||
            JNU_IsInstanceOfByName(env, target, "java/awt/Dialog") > 0) {
            if (IsUndecorated() == FALSE) {
                /* Get outer frame sizes. */
                LONG style = GetStyle();
                if (style & WS_THICKFRAME) {
                    m_insets.left = m_insets.right = 
                        ::GetSystemMetrics(SM_CXSIZEFRAME);
                    m_insets.top = m_insets.bottom = 
                        ::GetSystemMetrics(SM_CYSIZEFRAME);
                } else {
                    m_insets.left = m_insets.right = 
                        ::GetSystemMetrics(SM_CXDLGFRAME);
                    m_insets.top = m_insets.bottom = 
                        ::GetSystemMetrics(SM_CYDLGFRAME);
                }
              

                /* Add in title. */
                m_insets.top += ::GetSystemMetrics(SM_CYCAPTION);
            }
            else { 
                /* fix for 4418125: Undecorated frames are off by one */
                /* undo the -1 set above */
                /* Additional fix for 5059656 */
                ::memset(&m_insets, 0, sizeof(m_insets));
            }

            /* Add in menuBar, if any. */
            if (JNU_IsInstanceOfByName(env, target, "java/awt/Frame") > 0 &&
                ((AwtFrame*)this)->GetMenuBar()) {
                m_insets.top += ::GetSystemMetrics(SM_CYMENU);
            }
        }
        m_insets.bottom += extraBottomInsets;
        env->DeleteLocalRef(target);
    }

    jobject peer = GetPeer(env);
    /* Get insets into our peer directly */
    jobject peerInsets = (env)->GetObjectField(peer, AwtPanel::insets_ID);
    DASSERT(!safe_ExceptionOccurred(env));
    if (peerInsets != NULL) { // may have been called during creation
        (env)->SetIntField(peerInsets, AwtInsets::topID, m_insets.top);
        (env)->SetIntField(peerInsets, AwtInsets::bottomID, 
                           m_insets.bottom);
        (env)->SetIntField(peerInsets, AwtInsets::leftID, m_insets.left);
        (env)->SetIntField(peerInsets, AwtInsets::rightID, m_insets.right);
    }
    /* Get insets into the Inset object (if any) that was passed */
    if (insets != NULL) {
        (env)->SetIntField(insets, AwtInsets::topID, m_insets.top);
        (env)->SetIntField(insets, AwtInsets::bottomID, m_insets.bottom);
        (env)->SetIntField(insets, AwtInsets::leftID, m_insets.left);
        (env)->SetIntField(insets, AwtInsets::rightID, m_insets.right);
    }
    env->DeleteLocalRef(peerInsets);

    BOOL        insetsChanged = !::EqualRect( &m_old_insets, &m_insets );       
    ::CopyRect( &m_old_insets, &m_insets );

    if (insetsChanged && DDCanReplaceSurfaces(GetHWnd())) {
        // Since insets are changed we need to update the surfaceData object
        // to reflect that change
        env->CallVoidMethod(peer, AwtComponent::replaceSurfaceDataLaterMID);
    }

    return insetsChanged;
}

/**
 * Sometimes we need the hWnd that actually owns this Window's hWnd (if
 * there is an owner).
 */
HWND AwtWindow::GetTopLevelHWnd()
{
    return m_owningFrameDialog ? m_owningFrameDialog->GetHWnd() :
                                 GetHWnd();
}

/*
 * Although this function sends ComponentEvents, it needs to be defined
 * here because only top-level windows need to have move and resize
 * events fired from native code.  All contained windows have these events
 * fired from common Java code.
 */
void AwtWindow::SendComponentEvent(jint eventId)
{
    JNIEnv *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);

    static jclass classEvent = NULL;
    if (classEvent == NULL) {
        if (env->PushLocalFrame(1) < 0)
            return;
        classEvent = env->FindClass("java/awt/event/ComponentEvent");
        if (classEvent != NULL) {
            classEvent = (jclass)env->NewGlobalRef(classEvent);
        }
        env->PopLocalFrame(0);
    }
    static jmethodID eventInitMID = NULL;
    if (eventInitMID == NULL) {
        eventInitMID = env->GetMethodID(classEvent, "<init>",
                                        "(Ljava/awt/Component;I)V");
        if (eventInitMID == NULL) {
            return;
        }
    }
    if (env->EnsureLocalCapacity(2) < 0) {
        return;
    }
    jobject target = GetTarget(env);
    jobject event = env->NewObject(classEvent, eventInitMID, 
                                   target, eventId);
    DASSERT(!safe_ExceptionOccurred(env));
    DASSERT(event != NULL);
    SendEvent(event);

    env->DeleteLocalRef(target);
    env->DeleteLocalRef(event);
}

void AwtWindow::SendWindowEvent(jint id, HWND opposite,
                                jint oldState, jint newState)
{
    JNIEnv *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);

    static jclass wClassEvent;
    if (wClassEvent == NULL) {
        if (env->PushLocalFrame(1) < 0)
            return;
        wClassEvent = env->FindClass("java/awt/event/WindowEvent");
        if (wClassEvent != NULL) {
            wClassEvent = (jclass)env->NewGlobalRef(wClassEvent);
        }
        env->PopLocalFrame(0);
        if (wClassEvent == NULL) {
            return;
        }
    }

    static jmethodID wEventInitMID;
    if (wEventInitMID == NULL) {
        wEventInitMID =
            env->GetMethodID(wClassEvent, "<init>",
                             "(Ljava/awt/Window;ILjava/awt/Window;II)V");
        DASSERT(wEventInitMID);
        if (wEventInitMID == NULL) {
            return;
        }
    }

    static jclass sequencedEventCls;
    if (sequencedEventCls == NULL) {
        jclass sequencedEventClsLocal
            = env->FindClass("java/awt/SequencedEvent");
        DASSERT(sequencedEventClsLocal);
        if (sequencedEventClsLocal == NULL) {
            /* exception already thrown */
            return;
        }
        sequencedEventCls =
            (jclass)env->NewGlobalRef(sequencedEventClsLocal);
        env->DeleteLocalRef(sequencedEventClsLocal);
    }

    static jmethodID sequencedEventConst;
    if (sequencedEventConst == NULL) {
        sequencedEventConst =
            env->GetMethodID(sequencedEventCls, "<init>",
                             "(Ljava/awt/AWTEvent;)V");
    }

    if (env->EnsureLocalCapacity(3) < 0) {
        return;
    }

    jobject target = GetTarget(env);
    jobject jOpposite = NULL;
    if (opposite != NULL) {
        AwtComponent *awtOpposite = AwtComponent::GetComponent(opposite);
        if (awtOpposite != NULL) {
            jOpposite = awtOpposite->GetTarget(env);
        }
    }
    jobject event = env->NewObject(wClassEvent, wEventInitMID, target, id,
                                   jOpposite, oldState, newState);
    DASSERT(!safe_ExceptionOccurred(env));
    DASSERT(event != NULL);
    if (jOpposite != NULL) {
        env->DeleteLocalRef(jOpposite); jOpposite = NULL;
    }
    env->DeleteLocalRef(target); target = NULL;

    if (id == java_awt_event_WindowEvent_WINDOW_GAINED_FOCUS ||
        id == java_awt_event_WindowEvent_WINDOW_LOST_FOCUS)
    {
        jobject sequencedEvent = env->NewObject(sequencedEventCls,
                                                sequencedEventConst,
                                                event);
        DASSERT(!safe_ExceptionOccurred(env));
        DASSERT(sequencedEvent != NULL);
        env->DeleteLocalRef(event);
        event = sequencedEvent;
    }

    SendEvent(event);

    env->DeleteLocalRef(event);
}

MsgRouting AwtWindow::WmActivate(UINT nState, BOOL fMinimized, HWND opposite)
{
    jint type;

    if (nState != WA_INACTIVE) {
        ::SetFocus((sm_focusOwner == NULL ||
                    AwtComponent::GetTopLevelParentForWindow(sm_focusOwner) !=
                    GetHWnd()) ? NULL : sm_focusOwner);
        type = java_awt_event_WindowEvent_WINDOW_GAINED_FOCUS;
        AwtToolkit::GetInstance().
            InvokeFunctionLater(BounceActivation, this);
        sm_focusedWindow = GetHWnd();
    } else {
        type = java_awt_event_WindowEvent_WINDOW_LOST_FOCUS;
        sm_focusedWindow = NULL;
    }

    SendWindowEvent(type, opposite);
    return mrConsume;
}

void AwtWindow::BounceActivation(void *self) {
    AwtWindow *wSelf = (AwtWindow *)self;

    if (::GetActiveWindow() == wSelf->GetHWnd()) {
        AwtFrame *owner = wSelf->GetOwningFrameOrDialog();

        if (owner != NULL) {
            sm_suppressFocusAndActivation = TRUE;
            ::SetActiveWindow(owner->GetHWnd());
            ::SetFocus(owner->GetProxyFocusOwner());
            sm_suppressFocusAndActivation = FALSE;
        }
    }
}

MsgRouting AwtWindow::WmDDEnterFullScreen(HMONITOR monitor) {
    /**
     * DirectDraw expects to receive a top-level window. This object may
     * be an AwtWindow instance, which has an owning AwtFrame window, or
     * an AwtFrame object which does not have an owner.
     * What we want is the top-level Frame hWnd, whether we were handed a
     * top-level AwtFrame, or some owned AwtWindow.  We get this by calling
     * GetTopLevelHWnd(), which returns the hwnd of a top-level AwtFrame
     * object (if this window has an owner) which we then pass
     * into DirectDraw.
     */
    HWND hWnd = GetTopLevelHWnd();
    if (!::IsWindowVisible(hWnd)) {
        // Sometimes there are problems going into fullscreen on an owner frame
        // that is not yet visible; make sure the FS window is visible first
        ::ShowWindow(hWnd, SW_SHOWNA);
    }
    DDEnterFullScreen(monitor, GetHWnd(), hWnd);
    return mrDoDefault;
}

MsgRouting AwtWindow::WmDDExitFullScreen(HMONITOR monitor) {
    HWND hWnd = GetTopLevelHWnd();
    DDExitFullScreen(monitor, hWnd);
    return mrDoDefault;
}

MsgRouting AwtWindow::WmCreate()
{
    return mrDoDefault;  
}

MsgRouting AwtWindow::WmClose()
{
    SendWindowEvent(java_awt_event_WindowEvent_WINDOW_CLOSING);

    /* Rely on above notification to handle quitting as needed */
    return mrConsume;  
}

MsgRouting AwtWindow::WmDestroy()
{
    SendWindowEvent(java_awt_event_WindowEvent_WINDOW_CLOSED);
    return AwtComponent::WmDestroy();
}

/*
 * Override AwtComponent's move handling to first update the
 * java AWT target's position fields directly, since Windows
 * and below can be resized from outside of java (by user)
 */
MsgRouting AwtWindow::WmMove(int x, int y)
{
    if ( IsIconic(GetHWnd()) ) {
    // fixes 4065534 (robi.khan@eng)
    // if a window is iconified we don't want to update
    // it's target's position since minimized Win32 windows
    // move to -32000, -32000 for whatever reason
    // NOTE: See also AwtWindow::Reshape
        return mrDoDefault;
    }

    if (m_screenNum == -1) {
    // Set initial value
        m_screenNum = GetScreenImOn();
    } 
    else {
        CheckIfOnNewScreen();
    }

    /* Update the java AWT target component's fields directly */
    JNIEnv *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);
    if (env->EnsureLocalCapacity(1) < 0) {
        return mrConsume;
    }
    jobject peer = GetPeer(env);
    jobject target = env->GetObjectField(peer, AwtObject::targetID);

    RECT rect;
    ::GetWindowRect(GetHWnd(), &rect);

    jint targX = (env)->GetIntField(target, AwtComponent::xID);
    jint targY = (env)->GetIntField(target, AwtComponent::yID);
    if (targX != rect.left || targY != rect.top) {
        (env)->SetIntField(target, AwtComponent::xID, rect.left);
        (env)->SetIntField(target, AwtComponent::yID, rect.top);
        SendComponentEvent(java_awt_event_ComponentEvent_COMPONENT_MOVED);
    }

    env->DeleteLocalRef(target);
    return AwtComponent::WmMove(x, y);
}

/*
 * Override AwtComponent's size handling to first update the 
 * java AWT target's dimension fields directly, since Windows
 * and below can be resized from outside of java (by user)
 */
MsgRouting AwtWindow::WmSize(UINT type, int w, int h)
{
    if (type == SIZE_MINIMIZED) {
        return mrDoDefault;
    }

    JNIEnv *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);
    if (env->EnsureLocalCapacity(1) < 0)
        return mrDoDefault;
    jobject target = GetTarget(env);
    // fix 4167248 : ensure the insets are up-to-date before using
    BOOL insetsChanged = UpdateInsets(NULL);
    jint width = (env)->GetIntField(target, AwtComponent::widthID);
    jint height = (env)->GetIntField(target, AwtComponent::heightID);
    int newWidth = w + m_insets.left + m_insets.right;
    int newHeight = h + m_insets.top + m_insets.bottom;

    if (width != newWidth || height != newHeight) {
        (env)->SetIntField(target, AwtComponent::widthID, newWidth);
        (env)->SetIntField(target, AwtComponent::heightID, newHeight);
        if (!m_resizing) {
            WindowResized();
        }
    }

    env->DeleteLocalRef(target);
    return AwtComponent::WmSize(type, w, h);
}

MsgRouting AwtWindow::WmPaint(HDC)
{
    PaintUpdateRgn(&m_insets);
    return mrConsume;
}

MsgRouting AwtWindow::WmSysCommand(UINT uCmdType, int xPos, int yPos)
{
    if (uCmdType == SC_SIZE) {
        m_resizing = TRUE;
    }
    return mrDoDefault;
}

MsgRouting AwtWindow::WmExitSizeMove()
{
    if (m_resizing) {
        WindowResized();
        m_resizing = FALSE;
    }
    return mrDoDefault;
}

MsgRouting AwtWindow::WmSettingChange(UINT wFlag, LPCTSTR pszSection)
{
    if (wFlag == SPI_SETNONCLIENTMETRICS) {
    // user changed window metrics in
    // Control Panel->Display->Appearance
    // which may cause window insets to change
        UpdateInsets(NULL);
        
    // [rray] fix for 4407329 - Changing Active Window Border width in display 
    //  settings causes problems
        WindowResized();
        Invalidate(NULL);

        return mrConsume;
    }
    return mrDoDefault;
}

MsgRouting AwtWindow::WmNcCalcSize(BOOL fCalcValidRects, 
                                   LPNCCALCSIZE_PARAMS lpncsp, LRESULT& retVal)
{
    MsgRouting mrRetVal = mrDoDefault;

    if (fCalcValidRects == FALSE) {
        return mrDoDefault;
    }
    JNIEnv *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);
    if (env->EnsureLocalCapacity(2) < 0) {
        return mrConsume;
    }
    jobject target = GetTarget(env);
    jstring warningString = 
      (jstring)(env)->GetObjectField(target, AwtWindow::warningStringID);
    if (warningString != NULL) {
        RECT r;
        ::CopyRect(&r, &lpncsp->rgrc[0]);
        retVal = static_cast<UINT>(DefWindowProc(WM_NCCALCSIZE, fCalcValidRects,
        reinterpret_cast<LPARAM>(lpncsp)));

        /* Adjust non-client area for warning banner space. */
        m_warningRect.left = lpncsp->rgrc[0].left;
        m_warningRect.right = lpncsp->rgrc[0].right;
        m_warningRect.bottom = lpncsp->rgrc[0].bottom;
        m_warningRect.top = 
            m_warningRect.bottom - ::GetSystemMetrics(SM_CYCAPTION);
        if (GetStyle() & WS_THICKFRAME) {
            m_warningRect.top -= 2;
        } else {
            m_warningRect.top += 2;
        }

        lpncsp->rgrc[0].bottom = (m_warningRect.top >= lpncsp->rgrc[0].top)
            ? m_warningRect.top
            : lpncsp->rgrc[0].top;

        /* Convert to window-relative coordinates. */
        ::OffsetRect(&m_warningRect, -r.left, -r.top);

        /* Notify target of Insets change. */
        UpdateInsets(NULL);

        mrRetVal = mrConsume;
    } else {
        // WM_NCCALCSIZE is usually in response to a resize, but
        // also can be triggered by SetWindowPos(SWP_FRAMECHANGED),
        // which means the insets will have changed - rnk 4/7/1998
        retVal = static_cast<UINT>(DefWindowProc(
        WM_NCCALCSIZE, fCalcValidRects, reinterpret_cast<LPARAM>(lpncsp)));
        UpdateInsets(NULL);
        mrRetVal = mrConsume;
    }
    env->DeleteLocalRef(target);
    env->DeleteLocalRef(warningString);
    return mrRetVal;
}

MsgRouting AwtWindow::WmNcPaint(HRGN hrgn)
{
    DefWindowProc(WM_NCPAINT, (WPARAM)hrgn, 0);

    JNIEnv *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);
    if (env->EnsureLocalCapacity(2) < 0) {
        return mrConsume;
    }
    jobject target = GetTarget(env);
    jstring warningString =
      (jstring)(env)->GetObjectField(target, AwtWindow::warningStringID);
    if (warningString != NULL) {
        RECT r;
        ::CopyRect(&r, &m_warningRect);
        HDC hDC = ::GetWindowDC(GetHWnd());
        DASSERT(hDC);
        int iSaveDC = ::SaveDC(hDC);
        VERIFY(::SelectClipRgn(hDC, NULL) != NULL);
        VERIFY(::FillRect(hDC, &m_warningRect, (HBRUSH)::GetStockObject(BLACK_BRUSH)));

        if (GetStyle() & WS_THICKFRAME) {
            /* draw edge */
            VERIFY(::DrawEdge(hDC, &r, EDGE_RAISED, BF_TOP));
            r.top += 2;
            VERIFY(::DrawEdge(hDC, &r, EDGE_SUNKEN, BF_RECT));
            ::InflateRect(&r, -2, -2);
        }

        /* draw warning text */
        LPWSTR text = TO_WSTRING(warningString);
        VERIFY(::SetBkColor(hDC, ::GetSysColor(COLOR_BTNFACE)) != CLR_INVALID);
        VERIFY(::SetTextColor(hDC, ::GetSysColor(COLOR_BTNTEXT)) != CLR_INVALID);
        VERIFY(::SelectObject(hDC, ::GetStockObject(DEFAULT_GUI_FONT)) != NULL);
        VERIFY(::SetTextAlign(hDC, TA_LEFT | TA_BOTTOM) != GDI_ERROR);
        VERIFY(::ExtTextOutW(hDC, r.left+2, r.bottom-1,
                             ETO_CLIPPED | ETO_OPAQUE,
                             &r, text, static_cast<UINT>(wcslen(text)), NULL));
        VERIFY(::RestoreDC(hDC, iSaveDC));
        ::ReleaseDC(GetHWnd(), hDC);
    }

    env->DeleteLocalRef(target);
    env->DeleteLocalRef(warningString);
    return mrConsume;
}

/*
MsgRouting AwtWindow::WmNcHitTest(UINT x, UINT y, LRESULT& retVal)
{
    POINT pt = { x, y };
    ::ScreenToClient(GetHWnd(), &pt);
    if (::PtInRect(&m_warningRect, pt)) {
        retVal = HTNOWHERE;
    } else {
        retVal = DefWindowProc(WM_NCHITTEST, 0, MAKELPARAM(x, y));
    }
    return mrConsume;
}
*/

/*
 * Fix for BugTraq ID 4041703: keyDown not being invoked.
 * This method overrides AwtCanvas::HandleEvent() since 
 * an empty Window always receives the focus on the activation
 * so we don't have to modify the behavior.
 */
MsgRouting AwtWindow::HandleEvent(MSG *msg, BOOL synthetic)
{
    return AwtComponent::HandleEvent(msg, synthetic);
}

void AwtWindow::WindowResized()
{
    SendComponentEvent(java_awt_event_ComponentEvent_COMPONENT_RESIZED);
    // Need to replace surfaceData on resize to catch changes to 
    // various component-related values, such as insets
    if (DDCanReplaceSurfaces(GetHWnd())) {
        JNIEnv *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);
        env->CallVoidMethod(m_peerObject, AwtComponent::replaceSurfaceDataLaterMID);
    }
}

BOOL CALLBACK InvalidateChildRect(HWND hWnd, LPARAM)
{
    TRY;

    ::InvalidateRect(hWnd, NULL, TRUE);
    return TRUE;

    CATCH_BAD_ALLOC_RET(FALSE);
}

void AwtWindow::Invalidate(RECT* r)
{
    ::InvalidateRect(GetHWnd(), NULL, TRUE);
    ::EnumChildWindows(GetHWnd(), (WNDENUMPROC)InvalidateChildRect, 0);
}

BOOL AwtWindow::IsResizable() {
    return (GetStyle() & WS_MAXIMIZEBOX) == WS_MAXIMIZEBOX;
}

void AwtWindow::SetResizable(BOOL isResizable)
{
    if (IsEmbeddedFrame()) {
        return;
    }
    LONG style = GetStyle();
    LONG resizeStyle = WS_MAXIMIZEBOX;

    if (IsUndecorated() == FALSE) {
        resizeStyle |= WS_THICKFRAME;
    }

    if (isResizable) {
        style |= resizeStyle;
    } else {
        style &= ~resizeStyle;
    }
    SetStyle(style);
    RedrawNonClient();
}

// SetWindowPos flags to cause frame edge to be recalculated
static const UINT SwpFrameChangeFlags =
    SWP_FRAMECHANGED | /* causes WM_NCCALCSIZE to be called */
    SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
    SWP_NOACTIVATE | SWP_NOCOPYBITS | 
    SWP_NOREPOSITION | SWP_NOSENDCHANGING;

//
// Forces WM_NCCALCSIZE to be called to recalculate
// window border (updates insets) without redrawing it
//
void AwtWindow::RecalcNonClient()
{
    ::SetWindowPos(GetHWnd(), (HWND) NULL, 0, 0, 0, 0, SwpFrameChangeFlags|SWP_NOREDRAW);
}

//
// Forces WM_NCCALCSIZE to be called to recalculate
// window border (updates insets) and redraws border to match
//
void AwtWindow::RedrawNonClient()
{
    ::SetWindowPos(GetHWnd(), (HWND) NULL, 0, 0, 0, 0, SwpFrameChangeFlags|SWP_ASYNCWINDOWPOS);
}

void AwtWindow::ToBack(void *data) {
    HWND hwnd = (HWND)data;
    if (AwtComponent::GetComponent(hwnd) == NULL) {
        // Window was disposed. Don't bother.
        return;
    }

    ::SetWindowPos(hwnd, HWND_BOTTOM, 0,0,0,0,
                   SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);

    // If hwnd is the foreground window or if *any* of its owners are, then
    // we have to reset the foreground window. The reason is that when we send
    // hwnd to back, all of its owners are sent to back as well. If any one of
    // them is the foreground window, then it's possible that we could end up
    // with a foreground window behind a window of another application.
    HWND foregroundWindow = ::GetForegroundWindow();
    BOOL adjustForegroundWindow;
    HWND toTest = hwnd;
    do {
        adjustForegroundWindow = (toTest == foregroundWindow);
        if (adjustForegroundWindow) {
            break;
        }
        toTest = ::GetWindow(toTest, GW_OWNER);
    } while (toTest != NULL);

    if (adjustForegroundWindow) {
        HWND foregroundSearch = hwnd, newForegroundWindow = NULL;
        while (1) {
            foregroundSearch = ::GetNextWindow(foregroundSearch,
                                               GW_HWNDPREV);
            if (foregroundSearch == NULL) {
                break;
            }
            LONG style = static_cast<LONG>(
            ::GetWindowLongPtr(foregroundSearch, GWL_STYLE));
            if ((style & WS_CHILD) || !(style & WS_VISIBLE)) {
                continue;
            }

            if (AwtComponent::GetComponent(foregroundSearch) != NULL) {
                newForegroundWindow = foregroundSearch;
            }
        }
        if (newForegroundWindow != NULL) {
            ::SetWindowPos(newForegroundWindow, HWND_TOP, 0,0,0,0,
                           SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
            if (((AwtWindow*)AwtComponent::GetComponent(newForegroundWindow))->IsFocusableWindow()) {
                ::SetForegroundWindow(newForegroundWindow);
            }
        } else {
            // We *have* to set the active HWND to something new. We simply
            // cannot risk having an active Java HWND which is behind an HWND
            // of a native application. This really violates the Windows user
            // experience.
            //
            // Windows won't allow us to set the foreground window to NULL,
            // so we use the desktop window instead. To the user, it appears
            // that there is no foreground window system-wide.
            ::SetForegroundWindow(::GetDesktopWindow());
        }
    }
}

int AwtWindow::GetScreenImOn() {
    MHND hmon;
    int scrnNum;

    hmon = ::MonitorFromWindow(GetHWnd(), MONITOR_DEFAULT_TO_PRIMARY);
    DASSERT(hmon != NULL);
    
    scrnNum = getScreenFromMHND(hmon);
    DASSERT(scrnNum > -1);

    return scrnNum;
}

/* Check to see if we've been moved onto another screen.
 * If so, update internal data, surfaces, etc.
 */

void AwtWindow::CheckIfOnNewScreen() {
    int curScrn = GetScreenImOn();

    if (curScrn != m_screenNum) {  // we've been moved 
        JNIEnv *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);

        jclass peerCls = env->GetObjectClass(m_peerObject);
        DASSERT(peerCls);

        jmethodID draggedID = env->GetMethodID(peerCls, "draggedToNewScreen",
                                               "()V");
        DASSERT(draggedID);

        env->CallVoidMethod(m_peerObject, draggedID);
        m_screenNum = curScrn;
    }
}

BOOL AwtWindow::IsFocusableWindow() {
    JNIEnv *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);
    if (env->EnsureLocalCapacity(2) < 0) {
        return FALSE;
    }
    jobject target = GetTarget(env);
    static jfieldID FocusableWindowStateID = 0;
    if (FocusableWindowStateID == 0) {
        jclass windowClass = env->GetObjectClass(target);
        FocusableWindowStateID = env->GetFieldID(windowClass, "focusableWindowState", "Z");
        env->DeleteLocalRef(windowClass);
    }
    
    BOOL focusability = env->GetBooleanField(target, FocusableWindowStateID);
    env->DeleteLocalRef(target);
    return focusability;
}

extern "C" {

/*
 * Class:     java_awt_Window
 * Method:    initIDs
 * Signature: ()V
 */
JNIEXPORT void JNICALL
Java_java_awt_Window_initIDs(JNIEnv *env, jclass cls)
{
    TRY;

    AwtWindow::warningStringID =
        env->GetFieldID(cls, "warningString", "Ljava/lang/String;");
    AwtWindow::locationByPlatformID =
        env->GetFieldID(cls, "locationByPlatform", "Z");

    CATCH_BAD_ALLOC;
}

} /* extern "C" */


/************************************************************************
 * WindowPeer native methods
 */

extern "C" {

/*
 * Class:     sun_awt_windows_WWindowPeer
 * Method:    initIDs
 * Signature: ()V
 */
JNIEXPORT void JNICALL
Java_sun_awt_windows_WWindowPeer_initIDs(JNIEnv *env, jclass cls)
{
    TRY;

    AwtWindow::focusableWindowID =
        env->GetFieldID(cls, "focusableWindow", "Z");

    CATCH_BAD_ALLOC;
}

/*
 * Class:     sun_awt_windows_WWindowPeer
 * Method:    toFront
 * Signature: ()V
 */
JNIEXPORT void JNICALL
Java_sun_awt_windows_WWindowPeer__1toFront(JNIEnv *env, jobject self)
{
    TRY;

    PDATA pData;
    JNI_CHECK_PEER_RETURN(self);
    AwtWindow* w = (AwtWindow*)pData;

    ::SetWindowPos(w->GetHWnd(), HWND_TOP, 0,0,0,0,
                   SWP_NOMOVE|SWP_NOSIZE);
    if (w->IsFocusableWindow()) {
        ::SetForegroundWindow(w->GetHWnd());
    }

    CATCH_BAD_ALLOC;
}

/*
 * Class:     sun_awt_windows_WWindowPeer
 * Method:    toBack
 * Signature: ()V
 */
JNIEXPORT void JNICALL
Java_sun_awt_windows_WWindowPeer_toBack(JNIEnv *env, jobject self)
{
    TRY;

    PDATA pData;
    JNI_CHECK_PEER_RETURN(self);
    AwtWindow* w = (AwtWindow*)pData;

    AwtToolkit::GetInstance().InvokeFunction(AwtWindow::ToBack, w->GetHWnd());

    CATCH_BAD_ALLOC;
}

/*
 * Class:     sun_awt_windows_WWindowPeer
 * Method:    setAlwaysOnTop
 * Signature: (Z)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_windows_WWindowPeer_setAlwaysOnTop(JNIEnv *env, jobject self,
                                                jboolean value)
{
  TRY;
  
  PDATA pData;
  JNI_CHECK_PEER_RETURN(self);
  AwtWindow* w = (AwtWindow*)pData;
  
  w->SendMessage(WM_AWT_SETALWAYSONTOP, (WPARAM)value, (LPARAM)w);
  
  CATCH_BAD_ALLOC;
}

/*
 * Class:     sun_awt_windows_WWindowPeer
 * Method:    _setTitle
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_windows_WWindowPeer__1setTitle(JNIEnv *env, jobject self,
                                            jstring title)
{
    TRY;

    PDATA pData;
    JNI_CHECK_PEER_RETURN(self);
    JNI_CHECK_NULL_RETURN(title, "null title");

    AwtWindow* w = (AwtWindow*)pData;

    int length = env->GetStringLength(title);
    WCHAR *buffer = new WCHAR[length + 1];
    env->GetStringRegion(title, 0, length, buffer);
    buffer[length] = L'\0';
    VERIFY(::SetWindowTextW(w->GetHWnd(), buffer));
    delete[] buffer;

    CATCH_BAD_ALLOC;
}

/*
 * Class:     sun_awt_windows_WWindowPeer
 * Method:    _setResizable
 * Signature: (Z)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_windows_WWindowPeer__1setResizable(JNIEnv *env, jobject self,
                                                jboolean resizable)
{
    TRY;

    PDATA pData;
    JNI_CHECK_PEER_RETURN(self);

    AwtWindow* w = (AwtWindow*)pData;
    DASSERT(!IsBadReadPtr(w, sizeof(AwtWindow)));
    w->SetResizable(resizable != 0);

    CATCH_BAD_ALLOC;
}

/*
 * Class:     sun_awt_windows_WWindowPeer
 * Method:    create
 * Signature: (Lsun/awt/windows/WComponentPeer;)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_windows_WWindowPeer_createAwtWindow(JNIEnv *env, jobject self, 
                                                 jobject parent)
{
    TRY;

    PDATA pData;
    JNI_CHECK_PEER_RETURN(parent);
    AwtToolkit::CreateComponent(self, parent,
                                (AwtToolkit::ComponentFactory)
                                AwtWindow::Create);
    JNI_CHECK_PEER_CREATION_RETURN(self);

    CATCH_BAD_ALLOC;
}

/*
 * Class:     sun_awt_windows_WWindowPeer
 * Method:    updateInsets
 * Signature: (Ljava/awt/Insets;)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_windows_WWindowPeer_updateInsets(JNIEnv *env, jobject self,
                                              jobject insets)
{
    TRY;

    PDATA pData;
    JNI_CHECK_PEER_RETURN(self);
    JNI_CHECK_NULL_RETURN(insets, "null insets");

    AwtWindow* w = (AwtWindow*)pData;
    w->UpdateInsets(insets);

    CATCH_BAD_ALLOC;
}

/*
 * Class:     sun_awt_windows_WWindowPeer
 * Method:    getContainerElement
 * Signature: (Ljava/awt/Container;I)Ljava/awt/Component;
 */
JNIEXPORT jobject JNICALL
Java_sun_awt_windows_WWindowPeer_getContainerElement(JNIEnv *env, jobject self,
                                                     jobject container, 
                                                     jint index)
{
    TRY;

    static jfieldID Container_ncomponents_FID = 0;
    static jfieldID Container_component_FID = 0;

    JNI_CHECK_NULL_RETURN_NULL(container, "null container");
 
    if ((env)->GetIntField(container, AwtContainer::ncomponentsID) > 0) {
        jobjectArray comps = 
            (jobjectArray)(env)->GetObjectField(container, 
                                                AwtContainer::componentID);
        return env->GetObjectArrayElement(comps, index);
    }
    return NULL;

    CATCH_BAD_ALLOC_RET(NULL);
}

/*
 * Class:     sun_awt_windows_WWindowPeer
 * Method:    reshapeFrame
 * Signature: (IIII)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_windows_WWindowPeer_reshapeFrame(JNIEnv *env, jobject self,
                                        jint x, jint y, jint w, jint h) 
{
    TRY;

    PDATA pData;
    JNI_CHECK_PEER_RETURN(self);

    if (env->EnsureLocalCapacity(1) < 0) {
        return;
    }

    AwtFrame* p = (AwtFrame *)pData;

    jobject target = env->GetObjectField(self, AwtObject::targetID);
    JNI_CHECK_NULL_RETURN(target, "null target");

    // enforce tresholds before sending the event
    // Fix for 4459064 : do not enforce thresholds for embedded frames
    if (!p->IsEmbeddedFrame()) {
        int minWidth = ::GetSystemMetrics(SM_CXMIN);
        int minHeight = ::GetSystemMetrics(SM_CYMIN);
        if (w < minWidth)
            env->SetIntField(target, AwtComponent::widthID,
                w = minWidth);
        if (h < minHeight)
            env->SetIntField(target, AwtComponent::heightID,
                h = minHeight);
    }

    env->DeleteLocalRef(target);

    RECT *r = new RECT;
    ::SetRect(r, x, y, x + w, y + h);
    p->SendMessage(WM_AWT_RESHAPE_COMPONENT, 0, (LPARAM)r);

    CATCH_BAD_ALLOC;
}

/*
 * Class:     sun_awt_windows_WWindowPeer
 * Method:    getSysMinWidth
 * Signature: ()I
 */
JNIEXPORT jint JNICALL
Java_sun_awt_windows_WWindowPeer_getSysMinWidth(JNIEnv *env, jclass self)
{
    TRY;

    return ::GetSystemMetrics(SM_CXMIN);

    CATCH_BAD_ALLOC_RET(0);
}

/*
 * Class:     sun_awt_windows_WWindowPeer
 * Method:    getSysMinHeight
 * Signature: ()I
 */
JNIEXPORT jint JNICALL
Java_sun_awt_windows_WWindowPeer_getSysMinHeight(JNIEnv *env, jclass self)
{
    TRY;

    return ::GetSystemMetrics(SM_CYMIN);

    CATCH_BAD_ALLOC_RET(0);
}

/*
 * Class:     sun_awt_windows_WWindowPeer
 * Method:    getScreenImOn
 * Signature: ()I
 */
JNIEXPORT jint JNICALL
Java_sun_awt_windows_WWindowPeer_getScreenImOn(JNIEnv *env, jobject self)
{
    TRY;

    jint retVal;
    PDATA pData;

    // It's entirely possible that our native resources have been destroyed
    // before our java peer - if we're dispose()d, for instance.
    // Alert caller w/ IllegalComponentStateException.
    if (self == NULL) {
        JNU_ThrowByName(env, "java/awt/IllegalComponentStateException",
                        "Peer null in JNI");
        return 0;
    }
    pData = JNI_GET_PDATA(self);
    if (pData == NULL) {
        JNU_ThrowByName(env, "java/awt/IllegalComponentStateException",
                        "Native resources unavailable");
        return 0;
    }

    AwtWindow* p = (AwtWindow*)pData;
    retVal = (jint)(p->GetScreenImOn());
    return retVal;

    CATCH_BAD_ALLOC_RET(-1);
}

/*
 * Class:     sun_awt_windows_WWindowPeer
 * Method:    resetTargetGC
 * Signature: ()V
 */
JNIEXPORT void JNICALL
Java_sun_awt_windows_WWindowPeer_resetTargetGC(JNIEnv *env, jobject self)
{
    Java_sun_awt_windows_WCanvasPeer_resetTargetGC(env, self);
}

} /* extern "C" */
