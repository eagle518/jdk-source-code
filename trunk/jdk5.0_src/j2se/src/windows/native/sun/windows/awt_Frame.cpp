/*
 * @(#)awt_Frame.cpp	1.137 04/06/08
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "awt_Toolkit.h"
#include "awt_Frame.h"
#include "awt_MenuBar.h"
#include "awt_Dialog.h"
#include "awt_IconCursor.h"
#include "awt_Win32GraphicsDevice.h"
#include <windowsx.h>

#include <java_lang_Integer.h>
#include <sun_awt_EmbeddedFrame.h>
#include <sun_awt_windows_WEmbeddedFrame.h>
#include <sun_awt_windows_WEmbeddedFramePeer.h>


BOOL isAppActive = FALSE;

/* IMPORTANT! Read the README.JNI file for notes on JNI converted AWT code.
 */

/************************************************************************
 * AwtFrame fields
 */

jfieldID AwtFrame::handleID;
jfieldID AwtFrame::stateID;
jfieldID AwtFrame::undecoratedID;


/************************************************************************
 * AwtFrame methods
 */

AwtFrame::AwtFrame() {
    m_hIcon = NULL;
    m_parentWnd = NULL;
    menuBar = NULL;
    m_isEmbedded = FALSE;
    m_ignoreWmSize = FALSE;
    m_isMenuDropped = FALSE;
    m_isInputMethodWindow = FALSE;
    m_windowClassName = NULL;
    m_isUndecorated = FALSE;
    m_proxyFocusOwner = NULL;
    m_actualFocusedWindow = NULL;
    m_iconic = FALSE;
    m_zoomed = FALSE;
    m_maxBoundsSet = FALSE;
    m_isEmbeddedFrameActivationRequest = FALSE;

    isInManualMoveOrSize = FALSE;
    grabbedHitTest = 0;
}

AwtFrame::~AwtFrame() {

    if (m_windowClassName) {
	delete m_windowClassName;
    }

    if (m_hIcon != NULL) {
        ::DestroyIcon(m_hIcon);
    }
    DestroyProxyFocusOwner();
}

LPCTSTR AwtFrame::GetClassName() {

	if (m_windowClassName == NULL)
	{
	    JNIEnv *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);
		if (env->EnsureLocalCapacity(2) < 0) {
			m_windowClassName = new TCHAR[_tcslen(AWT_FRAME_WINDOW_CLASS_NAME) + 1];
			_tcscpy(m_windowClassName,AWT_FRAME_WINDOW_CLASS_NAME);
	    }
		else
		{
		    jobject frame = GetTarget(env);
		    jclass  frameClass = env->GetObjectClass(frame);

			JavaStringBuffer jsb = JavaStringBuffer(env,JVM_GetClassName(env,frameClass));

			m_windowClassName = new TCHAR[_tcslen(jsb) + 1];
			_tcscpy(m_windowClassName,jsb);

                    env->DeleteLocalRef(frameClass);
                    env->DeleteLocalRef(frame);

		}
	}
	return m_windowClassName;
}

/*
 * Create a new AwtFrame object and window.
 */
AwtFrame* AwtFrame::Create(jobject self, jobject parent)
{
    JNIEnv *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);
    if (env->EnsureLocalCapacity(1) < 0) {
	return NULL;
    }

    PDATA pData;
    HWND hwndParent = NULL;
    AwtFrame* frame;
    jclass cls = NULL;
    jclass inputMethodWindowCls = NULL;
    jobject target = NULL;

    try {
        target = env->GetObjectField(self, AwtObject::targetID);
	JNI_CHECK_NULL_GOTO(target, "target", done);

	if (parent != NULL) {
	    JNI_CHECK_PEER_GOTO(parent, done);
	    {
	        AwtFrame* parent = (AwtFrame *)pData;
		hwndParent = parent->GetHWnd();
	    }
	}

	frame = new AwtFrame();
	
	{
	    /*
	     * A variation on Netscape's hack for embedded frames: the client
	     * area of the browser is a Java Frame for parenting purposes, but
	     * really a Windows child window
	     */
	    cls = env->FindClass("sun/awt/EmbeddedFrame");
	    if (cls == NULL) {
	        return NULL;
	    }
	    INT_PTR handle;
	    jboolean isEmbeddedInstance = env->IsInstanceOf(target, cls);
	    jboolean isEmbedded = FALSE;

	    if (isEmbeddedInstance) {
                handle = static_cast<INT_PTR>(env->GetLongField(target, AwtFrame::handleID));
		if (handle != 0) {
		    isEmbedded = TRUE;
		}
	    }
	    frame->m_isEmbedded = isEmbedded;
	    
	    if (isEmbedded) {
                hwndParent = (HWND)handle;
                RECT rect;
                ::GetClientRect(hwndParent, &rect);
                /* 
                 * Fix for BugTraq ID 4337754.
                 * Initialize m_peerObject before the first call 
                 * to AwtFrame::GetClassName().
                 */
                frame->m_peerObject = env->NewGlobalRef(self);
                frame->RegisterClass();
                DWORD exStyle = WS_EX_NOPARENTNOTIFY;

                if (GetRTL()) {
                    exStyle |= WS_EX_RIGHT | WS_EX_LEFTSCROLLBAR;
                    if (GetRTLReadingOrder())
                        exStyle |= WS_EX_RTLREADING;
                }

                frame->m_hwnd = ::CreateWindowEx(exStyle,
                                                 frame->GetClassName(), TEXT(""),
                                                 WS_CHILD | WS_CLIPCHILDREN,
                                                 0, 0,
                                                 rect.right, rect.bottom,
                                                 hwndParent, NULL,
                                                 AwtToolkit::GetInstance().GetModuleHandle(),
                                                 NULL);
                frame->LinkObjects(env, self);
                frame->SubclassHWND();

                // Update target's dimensions to reflect this embedded window.
                ::GetClientRect(frame->m_hwnd, &rect);
                ::MapWindowPoints(frame->m_hwnd, hwndParent, (LPPOINT)&rect,
                                  2);                

                env->SetIntField(target, AwtComponent::xID, rect.left);
                env->SetIntField(target, AwtComponent::yID, rect.top);
                env->SetIntField(target, AwtComponent::widthID,
                                 rect.right-rect.left);
                env->SetIntField(target, AwtComponent::heightID,
                                 rect.bottom-rect.top);
                frame->InitPeerGraphicsConfig(env, self);
	    } else {
	        jint state = env->GetIntField(target, AwtFrame::stateID);
		DWORD exStyle;
		DWORD style;

		// for input method windows, use minimal decorations
		inputMethodWindowCls = env->FindClass("sun/awt/im/InputMethodWindow");
	        if ((inputMethodWindowCls != NULL) && env->IsInstanceOf(target, inputMethodWindowCls)) {
		    exStyle = WS_EX_PALETTEWINDOW;
		    style = WS_CLIPCHILDREN;
		    frame->m_isInputMethodWindow = TRUE;
                } else if (env->GetBooleanField(target, AwtFrame::undecoratedID) == JNI_TRUE) {
                    exStyle = 0;
                    style = WS_POPUP | WS_SYSMENU | WS_CLIPCHILDREN |
                        WS_MAXIMIZEBOX | WS_MINIMIZEBOX;
                  if (state & java_awt_Frame_ICONIFIED) {
                      style |= WS_ICONIC;
                      frame->setIconic(TRUE);
                  }
                    frame->m_isUndecorated = TRUE;
		} else {
		    exStyle = WS_EX_WINDOWEDGE;
		    style = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN;
                  if (state & java_awt_Frame_ICONIFIED) {
                      style |= WS_ICONIC;
                      frame->setIconic(TRUE);
                  }
		}

		if (GetRTL()) {
		    exStyle |= WS_EX_RIGHT | WS_EX_LEFTSCROLLBAR;
		    if (GetRTLReadingOrder())
		        exStyle |= WS_EX_RTLREADING;
		}

                jint x = env->GetIntField(target, AwtComponent::xID);
                jint y = env->GetIntField(target, AwtComponent::yID);
                jint width = env->GetIntField(target, AwtComponent::widthID);
                jint height = env->GetIntField(target, AwtComponent::heightID);

		frame->CreateHWnd(env, L"",
		                  style,
				  exStyle,
				  0, 0, 0, 0,
				  hwndParent,
				  NULL,
				  ::GetSysColor(COLOR_WINDOWTEXT),
				  ::GetSysColor(COLOR_WINDOWFRAME),
				  self);

		/* 
		 * Reshape here instead of during create, so that a
		 * WM_NCCALCSIZE is sent. 
		 */
		frame->Reshape(x, y, width, height);
	    }
	    
	    AwtDialog::SetDisabledLevelToGreatest(frame);
	}
    } catch (...) {
        env->DeleteLocalRef(target);
	env->DeleteLocalRef(cls);
	env->DeleteLocalRef(inputMethodWindowCls);
	throw;
    }

done:
    env->DeleteLocalRef(target);
    env->DeleteLocalRef(cls);
    env->DeleteLocalRef(inputMethodWindowCls);

    return frame;
}

LRESULT CALLBACK AwtFrame::ProxyWindowProc(HWND hwnd, UINT message,
					   WPARAM wParam, LPARAM lParam)
{
    TRY;

    DASSERT(::IsWindow(hwnd));

    AwtFrame *parent = (AwtFrame *)
        AwtComponent::GetComponentImpl(::GetParent(hwnd));

    if (!parent || parent->GetProxyFocusOwner() != hwnd ||
	message == AwtComponent::WmAwtIsComponent)
    {
        return ::DefWindowProc(hwnd, message, wParam, lParam);
    }

    // IME and input language related messages need to be sent to a window 
    // which has the Java input focus
    switch (message) {
	case WM_IME_STARTCOMPOSITION:
	case WM_IME_ENDCOMPOSITION:
	case WM_IME_COMPOSITION:
	case WM_IME_SETCONTEXT:
	case WM_IME_NOTIFY:
	case WM_IME_CONTROL:
	case WM_IME_COMPOSITIONFULL:
	case WM_IME_SELECT:
	case WM_IME_CHAR:
	case 0x0288: //WM_IME_REQUEST
	case WM_IME_KEYDOWN:
	case WM_IME_KEYUP:
	case WM_INPUTLANGCHANGEREQUEST:
	case WM_INPUTLANGCHANGE:
	    AwtComponent *p = AwtComponent::GetComponent(sm_focusOwner);
	    if  (p!= NULL) {
		return p->WindowProc(message, wParam, lParam);
	    }
	    break;
    }

    return parent->WindowProc(message, wParam, lParam);

    CATCH_BAD_ALLOC_RET(0);
}

void AwtFrame::CreateProxyFocusOwner()
{
    DASSERT(m_proxyFocusOwner == NULL);
    DASSERT(AwtToolkit::MainThread() == ::GetCurrentThreadId());

    m_proxyFocusOwner = ::CreateWindow(TEXT("STATIC"), 
                                       TEXT("ProxyFocusOwner"), 
                                       WS_CHILD,
				       0, 0, 0, 0, GetHWnd(), NULL,
				       AwtToolkit::GetInstance().
				           GetModuleHandle(),
				       NULL);
    m_proxyDefWindowProc = (WNDPROC)
        ::SetWindowLongPtr(m_proxyFocusOwner, GWLP_WNDPROC, (INT_PTR)ProxyWindowProc);
}

void AwtFrame::DestroyProxyFocusOwner()
{
    DASSERT(AwtToolkit::MainThread() == ::GetCurrentThreadId());

    if (m_proxyFocusOwner != NULL) {
        HWND toDestroy = m_proxyFocusOwner;
	m_proxyFocusOwner = NULL;
	::SetWindowLongPtr(toDestroy, GWLP_WNDPROC, (INT_PTR)m_proxyDefWindowProc);
	::DestroyWindow(toDestroy);
    }
}

MsgRouting AwtFrame::WmMouseUp(UINT flags, int x, int y, int button) {
    if (isInManualMoveOrSize) {
        isInManualMoveOrSize = FALSE;
        ::ReleaseCapture();
        return mrConsume;
    }
    return AwtWindow::WmMouseUp(flags, x, y, button);
}

MsgRouting AwtFrame::WmMouseMove(UINT flags, int x, int y) {
    /**
     * If this Frame is non-focusable then we should implement move and size operation for it by 
     * ourselfves because we don't dispatch appropriate mouse messages to default window procedure.
     */
    if (!IsFocusableWindow() && isInManualMoveOrSize) {        
        DWORD curPos = ::GetMessagePos();
        x = GET_X_LPARAM(curPos);
        y = GET_Y_LPARAM(curPos);
        RECT r;
        ::GetWindowRect(GetHWnd(), &r);
        POINT mouseLoc = {x, y};
        mouseLoc.x -= savedMousePos.x;
        mouseLoc.y -= savedMousePos.y;
        savedMousePos.x = x;
        savedMousePos.y = y;
        if (grabbedHitTest == HTCAPTION) {
            ::SetWindowPos(GetHWnd(), NULL, r.left+mouseLoc.x, r.top+mouseLoc.y, 
                           r.right-r.left, r.bottom-r.top, 
                           SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
        } else {
            switch (grabbedHitTest) {
            case HTTOP:
                r.top += mouseLoc.y;
                break;
            case HTBOTTOM: 
                r.bottom += mouseLoc.y;
                break;
            case HTRIGHT:
                r.right += mouseLoc.x;
                break;
            case HTLEFT:
                r.left += mouseLoc.x;
                break;
            case HTTOPLEFT:
                r.left += mouseLoc.x;
                r.top += mouseLoc.y;
                break;
            case HTTOPRIGHT:
                r.top += mouseLoc.y;
                r.right += mouseLoc.x;
                break;
            case HTBOTTOMLEFT:
                r.left += mouseLoc.x;
                r.bottom += mouseLoc.y;
                break;
            case HTBOTTOMRIGHT:
            case HTSIZE:
                r.right += mouseLoc.x;
                r.bottom += mouseLoc.y;
                break;                
            }
                
            ::SetWindowPos(GetHWnd(), NULL, r.left, r.top, 
                           r.right-r.left, r.bottom-r.top, 
                           SWP_NOACTIVATE | SWP_NOSENDCHANGING | SWP_NOZORDER | 
                           SWP_NOCOPYBITS | SWP_DEFERERASE);
        }
        return mrConsume;
    } else {
        return AwtWindow::WmMouseMove(flags, x, y);
    }
}

MsgRouting AwtFrame::WmNcMouseDown(WPARAM hitTest, int x, int y, int button) {
    if (!IsFocusableWindow()) {        
        /**
         * If this Frame is non-focusable then we should implement move and size operation for it by 
         * ourselfves because we don't dispatch appropriate mouse messages to default window procedure.
         */
        if ((button & DBL_CLICK) == DBL_CLICK && hitTest == HTCAPTION) {
            // Double click on caption - maximize or restore Frame.
            if (IsResizable()) {
                if (::IsZoomed(GetHWnd())) {
                    ::ShowWindow(GetHWnd(), SW_SHOWNOACTIVATE);
                } else {                             
                    ::ShowWindow(GetHWnd(), SW_MAXIMIZE);
                }
            }
            return mrConsume;
        }
        switch (hitTest) {
          case HTMAXBUTTON:
              if (IsResizable()) {
                  if (::IsZoomed(GetHWnd())) {
                      ::ShowWindow(GetHWnd(), SW_SHOWNOACTIVATE);
                  } else {                             
                      ::ShowWindow(GetHWnd(), SW_MAXIMIZE);
                  }
              }            
              return mrConsume;
          case HTCAPTION: 
          case HTTOP:
          case HTBOTTOM:
          case HTLEFT:
          case HTRIGHT:
          case HTTOPLEFT:
          case HTTOPRIGHT:
          case HTBOTTOMLEFT:
          case HTBOTTOMRIGHT:
          case HTSIZE:
              // We are going to perform default mouse action on non-client area of this window
              // Grab mouse for this purpose and store coordinates for motion vector calculation
              savedMousePos.x = x;
              savedMousePos.y = y;
              ::SetCapture(GetHWnd());        
              isInManualMoveOrSize = TRUE;
              grabbedHitTest = hitTest;
              return mrConsume;
          default:
              return mrDoDefault;
        }
    } 
    return AwtWindow::WmNcMouseDown(hitTest, x, y, button);
}

/* Show the frame in it's current state */
void
AwtFrame::Show()
{
    HWND hwnd = GetHWnd();
    JNIEnv *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);
    BOOL focusableWindow =
        (env->GetBooleanField(GetPeer(env), AwtWindow::focusableWindowID) ==
         JNI_TRUE);

    DTRACE_PRINTLN3("AwtFrame::Show:%s%s%s",
                  m_iconic ? " iconic" : "",
                  m_zoomed ? " zoomed" : "",
                  m_iconic || m_zoomed ? "" : " normal");

    BOOL locationByPlatform = env->GetBooleanField(GetTarget(env), AwtWindow::locationByPlatformID);

    if (locationByPlatform) {
         moveToDefaultLocation();
    }

    if (m_iconic) {
	if (m_zoomed) {
	    // This whole function could probably be rewritten to use
	    // ::SetWindowPlacement but MS docs doesn't tell if
	    // ::SetWindowPlacement is a proper superset of
	    // ::ShowWindow.  So let's be conservative and only use it
	    // here, where we really do need it.
	    DTRACE_PRINTLN("AwtFrame::Show(SW_SHOWMINIMIZED, WPF_RESTORETOMAXIMIZED");
	    WINDOWPLACEMENT wp;
	    ::ZeroMemory(&wp, sizeof(WINDOWPLACEMENT));
	    wp.length = sizeof(WINDOWPLACEMENT);
	    ::GetWindowPlacement(hwnd, &wp);
            if (!focusableWindow) {
                wp.showCmd = SW_SHOWMINNOACTIVE;
            } else {
                wp.showCmd = SW_SHOWMINIMIZED;
            }
	    wp.flags |= WPF_RESTORETOMAXIMIZED;
	    ::SetWindowPlacement(hwnd, &wp);
	}
	else {
	    DTRACE_PRINTLN("AwtFrame::Show(SW_SHOWMINIMIZED)");
            if (!focusableWindow) {
                ::ShowWindow(hwnd, SW_SHOWMINNOACTIVE);
            } else {
                ::ShowWindow(hwnd, SW_SHOWMINIMIZED);
            }
	}
    }
    else if (m_zoomed) {
	DTRACE_PRINTLN("AwtFrame::Show(SW_SHOWMAXIMIZED)");
        if (!focusableWindow) {
            ::ShowWindow(hwnd, SW_MAXIMIZE);
        } else {
            ::ShowWindow(hwnd, SW_SHOWMAXIMIZED);
        }
    }
    else if (m_isInputMethodWindow) {
	// Don't activate input methow window 
	DTRACE_PRINTLN("AwtFrame::Show(SW_SHOWNA)");
	::ShowWindow(hwnd, SW_SHOWNA);
    }
    else {
	// Nor iconic, nor zoomed (handled above) - so use SW_RESTORE
	// to show in "normal" state regardless of whatever stale
	// state might the invisible window still has.
	DTRACE_PRINTLN("AwtFrame::Show(SW_RESTORE)");
        if (!focusableWindow) {
            ::ShowWindow(hwnd, SW_SHOWNOACTIVATE);
        } else {
            ::ShowWindow(hwnd, SW_RESTORE);
        }
    }
}

void
AwtFrame::SendWindowStateEvent(int oldState, int newState)
{
    SendWindowEvent(java_awt_event_WindowEvent_WINDOW_STATE_CHANGED,
		    NULL, oldState, newState);
}

void
AwtFrame::ClearMaximizedBounds()
{
    m_maxBoundsSet = FALSE;
}

void
AwtFrame::SetMaximizedBounds(int x, int y, int w, int h)
{
    m_maxPos.x  = x;
    m_maxPos.y  = y;
    m_maxSize.x = w;
    m_maxSize.y = h;
    m_maxBoundsSet = TRUE;
}

MsgRouting AwtFrame::WmGetMinMaxInfo(LPMINMAXINFO lpmmi)
{
    if (!m_maxBoundsSet) {
	return mrDoDefault;
    }

    if (m_maxPos.x != java_lang_Integer_MAX_VALUE)
	lpmmi->ptMaxPosition.x = m_maxPos.x;
    if (m_maxPos.y != java_lang_Integer_MAX_VALUE)
	lpmmi->ptMaxPosition.y = m_maxPos.y;
    if (m_maxSize.x != java_lang_Integer_MAX_VALUE)
	lpmmi->ptMaxSize.x = m_maxSize.x;
    if (m_maxSize.y != java_lang_Integer_MAX_VALUE)
	lpmmi->ptMaxSize.y = m_maxSize.y;
    return mrConsume;
}

MsgRouting AwtFrame::WmSize(UINT type, int w, int h)
{
    if (m_ignoreWmSize) {
        return mrDoDefault;
    }

    DTRACE_PRINTLN6("AwtFrame::WmSize: %dx%d,%s visible, state%s%s%s",
                  w, h,
                  ::IsWindowVisible(GetHWnd()) ? "" : " not",
                  m_iconic ? " iconic" : "",
                  m_zoomed ? " zoomed" : "",
                  m_iconic || m_zoomed ? "" : " normal");

    jint oldState = java_awt_Frame_NORMAL;
    if (m_iconic) {
	oldState |= java_awt_Frame_ICONIFIED;
    }
    if (m_zoomed) {
	oldState |= java_awt_Frame_MAXIMIZED_BOTH;
    }

    jint newState = java_awt_Frame_NORMAL;
    if (type == SIZE_MINIMIZED) {
      DTRACE_PRINTLN("AwtFrame::WmSize: SIZE_MINIMIZED");
	newState |= java_awt_Frame_ICONIFIED;
	if (m_zoomed) {
	    newState |= java_awt_Frame_MAXIMIZED_BOTH;
	}
	m_iconic = TRUE;
    }
    else if (type == SIZE_MAXIMIZED) {
      DTRACE_PRINTLN("AwtFrame::WmSize: SIZE_MAXIMIZED");
	newState |= java_awt_Frame_MAXIMIZED_BOTH;
	m_iconic = FALSE;
	m_zoomed = TRUE;
    }
    else if (type == SIZE_RESTORED) {
      DTRACE_PRINTLN("AwtFrame::WmSize: SIZE_RESTORED");
	m_iconic = FALSE;
	m_zoomed = FALSE;
    }

    jint changed = oldState ^ newState;
    if (changed != 0) {
      DTRACE_PRINTLN2("AwtFrame::WmSize: reporting state change %x -> %x",
                      oldState, newState);
	// report (de)iconification to old clients
	if (changed & java_awt_Frame_ICONIFIED) {
	    if (newState & java_awt_Frame_ICONIFIED) {
		SendWindowEvent(java_awt_event_WindowEvent_WINDOW_ICONIFIED);
	    } else {
		SendWindowEvent(java_awt_event_WindowEvent_WINDOW_DEICONIFIED);
	    }
	}

	// New (since 1.4) state change event
	SendWindowStateEvent(oldState, newState);
    }

    // If window is in iconic state, do not send COMPONENT_RESIZED event
    if (m_iconic) {
	return mrDoDefault;
    }

    return AwtWindow::WmSize(type, w, h);
}

MsgRouting AwtFrame::WmActivate(UINT nState, BOOL fMinimized, HWND opposite)
{
    // Process WM_ACTIVATE for EmbeddedFrame by request only
    if (IsEmbeddedFrame() && m_isEmbeddedFrameActivationRequest == FALSE) {
        return mrConsume;
    }
    m_isEmbeddedFrameActivationRequest = FALSE;

    jint type;
    BOOL doActivateFrame = TRUE;

    if (nState != WA_INACTIVE) {
        ::SetFocus(NULL); // The KeyboardFocusManager will set focus later
        type = java_awt_event_WindowEvent_WINDOW_GAINED_FOCUS;
        isAppActive = TRUE;
        sm_focusedWindow = GetHWnd();

	/* 
	 * Fix for 4823903.
	 * If the window to be focused is actually not this Frame
	 * and it's visible then send it WM_ACTIVATE.
	 */
	if (m_actualFocusedWindow != NULL) {
	    HWND hwnd = m_actualFocusedWindow->GetHWnd();

	    if (hwnd != NULL && ::IsWindowVisible(hwnd)) {

	        ::SendMessage(hwnd, WM_ACTIVATE, MAKEWPARAM(nState, fMinimized), (LPARAM)opposite);
	        doActivateFrame = FALSE;
	    }	    
	    m_actualFocusedWindow = NULL;
	}

    } else {

        // If actual focused window is not this Frame
	if (sm_focusedWindow != GetHWnd()) {

	    // Check that the Frame is going to be really inactive (i.e. the opposite is not its owned window)
	    if (opposite != NULL) {
		AwtWindow *wOpposite = (AwtWindow *)AwtComponent::GetComponent(opposite);

		if (wOpposite != NULL &&
		    wOpposite->GetOwningFrameOrDialog() != this)
		{
		    AwtWindow *window = (AwtWindow *)AwtComponent::GetComponent(sm_focusedWindow);
	    
		    // If actual focused window is one of Frame's owned windows
		    if (window != NULL && window->GetOwningFrameOrDialog() == this) {
		        m_actualFocusedWindow = window;	
		    }	
		}
	    }
	}	

        type = java_awt_event_WindowEvent_WINDOW_LOST_FOCUS;
        isAppActive = FALSE;
        sm_focusedWindow = NULL;
    }

    if (doActivateFrame) {
        SendWindowEvent(type, opposite);
    }
    return mrConsume;
}

MsgRouting AwtFrame::WmEnterMenuLoop(BOOL isTrackPopupMenu)
{
    if ( !isTrackPopupMenu ) {
	m_isMenuDropped = TRUE;
    }
    return mrDoDefault;
}

MsgRouting AwtFrame::WmExitMenuLoop(BOOL isTrackPopupMenu)
{
    if ( !isTrackPopupMenu ) {
	m_isMenuDropped = FALSE;
    }
    return mrDoDefault;
}

AwtMenuBar* AwtFrame::GetMenuBar()
{
    return menuBar;
}

void AwtFrame::SetMenuBar(AwtMenuBar* mb)
{
    menuBar = mb;
    if (mb == NULL) {
        // Remove existing menu bar, if any.
        ::SetMenu(GetHWnd(), NULL);
    } else {
        if (menuBar->GetHMenu() != NULL) {
            ::SetMenu(GetHWnd(), menuBar->GetHMenu());
        }
    }
}

MsgRouting AwtFrame::WmDrawItem(UINT ctrlId, DRAWITEMSTRUCT& drawInfo)
{
    JNIEnv *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);

    // if the item to be redrawn is the menu bar, then do it
    AwtMenuBar* awtMenubar = GetMenuBar();
    if (drawInfo.CtlType == ODT_MENU && (awtMenubar != NULL) &&
        (::GetMenu( GetHWnd() ) == (HMENU)drawInfo.hwndItem) )
	{
		awtMenubar->DrawItem(drawInfo);
		return mrConsume;
    }

	return AwtComponent::WmDrawItem(ctrlId, drawInfo);
}

MsgRouting AwtFrame::WmMeasureItem(UINT ctrlId, MEASUREITEMSTRUCT& measureInfo)
{
	JNIEnv *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);
	AwtMenuBar* awtMenubar = GetMenuBar();
	if ((measureInfo.CtlType == ODT_MENU) && (awtMenubar != NULL))
	{
		// AwtMenu instance is stored in itemData. Use it to check if this
		// menu is the menu bar.
		AwtMenu * pMenu = (AwtMenu *) measureInfo.itemData;
		DASSERT(pMenu != NULL);
		if ( pMenu == awtMenubar )
		{
			HWND hWnd = GetHWnd();
			HDC hDC = ::GetDC(hWnd);
			DASSERT(hDC != NULL);
			awtMenubar->MeasureItem(hDC, measureInfo);
			VERIFY(::ReleaseDC(hWnd, hDC));
			return mrConsume;
		}
	}

	return AwtComponent::WmMeasureItem(ctrlId, measureInfo);
}

static
HBITMAP create_mask(int hW,int hH)
{
    HBITMAP     hbmRC = NULL;
    int         numBytes = (hW * hH + 7) / 8;
    BYTE *pMask = new BYTE[numBytes];
    memset(pMask, 0, numBytes);
    hbmRC = ::CreateBitmap(hW, hH, 1, 1, pMask);
    ::GdiFlush();
    delete [] pMask;
    return hbmRC;
}

void AwtFrame::MakeSetIcon(JNIEnv *env, jintArray intRasterData,
			   jbyteArray byteMaskData, int nSS, int nW, int nH)
{
    HBITMAP hMask = NULL;
    BYTE* pMask = NULL;

    if (::IsIconic(GetHWnd())) {
        ::InvalidateRect(GetHWnd(), NULL, TRUE);
    }
    if (m_hIcon != NULL) {
        ::DestroyIcon(m_hIcon);
        m_hIcon = NULL;
    }

    if (intRasterData != NULL) {
        if (byteMaskData != NULL) {
            try {
                pMask = (BYTE*) env->GetPrimitiveArrayCritical(byteMaskData, 0);
                JNI_CHECK_NULL_RETURN(pMask, "byteMaskData data");
                hMask = ::CreateBitmap(nW, nH, 1, 1, pMask);
            }
            catch(...) {
                if (pMask != NULL) {
                    env->ReleasePrimitiveArrayCritical(byteMaskData, pMask, 0);
                }
                throw;
            }
            env->ReleasePrimitiveArrayCritical(byteMaskData, pMask, 0);
        }
        else {
            hMask = create_mask(nW, nH);
        }

	int *pINTRGBImageBuffer = NULL;
	HBITMAP hColor = NULL;
	try {
	    pINTRGBImageBuffer = (int *)
	        env->GetPrimitiveArrayCritical(intRasterData, 0);

	    JNI_CHECK_NULL_RETURN(pINTRGBImageBuffer, "intRasterData data");

	    hColor = create_BMP(m_parentWnd, pINTRGBImageBuffer,
				    nSS, nW, nH);
	} catch (...) {
	    if (pINTRGBImageBuffer != NULL) {
	        env->ReleasePrimitiveArrayCritical(intRasterData,
						   pINTRGBImageBuffer, 0);
	    }
	    throw;
	}

	env->ReleasePrimitiveArrayCritical(intRasterData, pINTRGBImageBuffer,
					   0);
        if (hMask && hColor) {
            ICONINFO icnInfo;
            memset(&icnInfo, 0, sizeof(ICONINFO));
            icnInfo.hbmMask = hMask;
            icnInfo.hbmColor = hColor;
            icnInfo.fIcon = TRUE;
            m_hIcon = ::CreateIconIndirect(&icnInfo);

            destroy_BMP(hColor);
            destroy_BMP(hMask);
        }
    }

    // If m_hIcon is NULL, Windows will use class default icon.
    SendMessage(WM_SETICON, ICON_SMALL, (LPARAM)m_hIcon);
    SendMessage(WM_SETICON, ICON_BIG,   (LPARAM)m_hIcon);

    // set icon on owned windows
    ::EnumThreadWindows(AwtToolkit::MainThread(),
			(WNDENUMPROC)OwnedSetIcon, (LPARAM)this);
}

// set icon on windows owned by this one
BOOL CALLBACK AwtFrame::OwnedSetIcon(HWND hWndDlg, LPARAM lParam)
{
    TRY;

    AwtFrame *frame = (AwtFrame *)lParam;
    HWND hwndDlgOwner = ::GetWindow(hWndDlg, GW_OWNER);

    while (hwndDlgOwner != NULL) {
	if (hwndDlgOwner == frame->GetHWnd()) {
	    break;
	}
        hwndDlgOwner = ::GetWindow(hwndDlgOwner, GW_OWNER);
    }

    if (hwndDlgOwner == NULL)
	return TRUE;

    // if owned window has an icon, change it to match the frame
    if (::SendMessage(hWndDlg, WM_GETICON, ICON_SMALL, NULL) != NULL) {
	HWND hWndOwner = frame->GetHWnd();

	HICON hBigIcon = frame->m_hIcon;
	//    = (HICON)::SendMessage(hWndOwner, WM_GETICON, ICON_BIG, NULL);
	if (hBigIcon == NULL) {
	    hBigIcon = (HICON)::GetClassLongPtr(hWndOwner, GCLP_HICON);
	}
	// We know that AwtFrame has at least class' big icon.
	DASSERT(hBigIcon != NULL);

	HICON hSmallIcon
	    = (HICON)::SendMessage(hWndOwner, WM_GETICON, ICON_SMALL, NULL);
	if (hSmallIcon == NULL) {
	    // If we just use the default AWT (big) icon for
	    // ICON_SMALL, it looks pretty jagged, so use the one
	    // autoconverted by windows.
	    hSmallIcon = (HICON)::GetClassLongPtr(hWndOwner, GCLP_HICONSM);
	}
	if (hSmallIcon == NULL) {
	    hSmallIcon = hBigIcon;
	}
	// We know that AwtFrame has at least class' small icon
	// (autoconverted by windows, as we use WNDCLASS that doesn't
	// have a slot for small icon).
	DASSERT(hSmallIcon != NULL);

	::SendMessage(hWndDlg, WM_SETICON, ICON_SMALL, (LPARAM)hSmallIcon);
	::SendMessage(hWndDlg, WM_SETICON, ICON_BIG,   (LPARAM)hBigIcon);
    }

    return TRUE;

    CATCH_BAD_ALLOC_RET(FALSE);
}

  
static BOOL keepOnMinimize(jobject peer) {
    static BOOL checked = FALSE; 
    static BOOL keep = FALSE;
    if (!checked) {
        keep = (JNU_GetStaticFieldByName(AwtToolkit::GetEnv(), NULL, 
            "sun/awt/windows/WFramePeer", "keepOnMinimize", "Z").z) == JNI_TRUE;
        checked = TRUE;
    }
    return keep;
}

MsgRouting AwtFrame::WmSysCommand(UINT uCmdType, int xPos, int yPos)
{
    if (uCmdType == (SYSCOMMAND_IMM & 0xFFF0)){
        JNIEnv* env = AwtToolkit::GetEnv();
        JNU_CallMethodByName(env, NULL, m_peerObject, 
            "notifyIMMOptionChange", "()V");
        DASSERT(!safe_ExceptionOccurred(env));  
        return mrConsume;
    }
    if ((uCmdType == SC_MINIMIZE) && keepOnMinimize(m_peerObject)) {
        ::ShowWindow(GetHWnd(),SW_SHOWMINIMIZED);
        return mrConsume; 
    }			   
    return AwtWindow::WmSysCommand(uCmdType, xPos, yPos);
}

LRESULT AwtFrame::WinThreadExecProc(ExecuteArgs * args)
{
    switch( args->cmdId ) {
	case FRAME_SETMENUBAR:
	{
    	    jobject  mbPeer = (jobject)args->param1;

	    // cancel any currently dropped down menus
	    if (m_isMenuDropped) {
		SendMessage(WM_CANCELMODE);
	    }

	    if (mbPeer == NULL) {
		// Remove existing menu bar, if any
		SetMenuBar(NULL);
	    } else {
		JNIEnv *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);
		AwtMenuBar* menuBar = (AwtMenuBar *)JNI_GET_PDATA(mbPeer);
		SetMenuBar(menuBar);
	    }
	    DrawMenuBar();
	    break;
	}

	default:
	    AwtWindow::WinThreadExecProc(args);
	    break;
    }

    return 0L;
}

/*
 * Execute on Toolkit only.
 */
void AwtFrame::ActivateEmbeddedFrame(void *param) {
    HWND hwndParent = (HWND)param; // EmbeddedFrame's control window.
    HWND hwndEmbFrame = ::GetWindow(hwndParent, GW_CHILD);
    PerformEmbeddedFrameActivation(hwndEmbFrame, TRUE);
}
void AwtFrame::DeactivateEmbeddedFrame(void *param) {
    HWND hwndParent = (HWND)param;
    HWND hwndEmbFrame = ::GetWindow(hwndParent, GW_CHILD);    
    PerformEmbeddedFrameActivation(hwndEmbFrame, FALSE);
}
void AwtFrame::PerformEmbeddedFrameActivation(HWND hwndEmbFrame, BOOL b) {
    if (::IsWindowVisible(hwndEmbFrame)) {

        AwtFrame *frame = (AwtFrame*)AwtComponent::GetComponent(hwndEmbFrame);
        frame->m_isEmbeddedFrameActivationRequest = TRUE;

        ::SendMessage(hwndEmbFrame, WM_ACTIVATE, MAKEWPARAM((b == TRUE ? WA_ACTIVE : WA_INACTIVE), FALSE), NULL);
    }
}

/************************************************************************
 * WFramePeer native methods
 */

extern "C" {

/*
 * Class:     sun_awt_windows_WFramePeer
 * Method:    initIDs
 * Signature: ()V
 */
JNIEXPORT void JNICALL
Java_java_awt_Frame_initIDs(JNIEnv *env, jclass cls)
{
    TRY;

    AwtFrame::stateID = env->GetFieldID(cls, "state", "I");
    DASSERT(AwtFrame::stateID != NULL);
        
    AwtFrame::undecoratedID = env->GetFieldID(cls,"undecorated","Z");
    DASSERT(AwtFrame::undecoratedID != NULL);    

    CATCH_BAD_ALLOC;
}

/*
 * Class:     sun_awt_windows_WFramePeer
 * Method:    setState
 * Signature: (I)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_windows_WFramePeer_setState(JNIEnv *env, jobject self,
    jint state)
{
    TRY;

    PDATA pData;
    JNI_CHECK_PEER_RETURN(self);

    AwtFrame* w = (AwtFrame*)pData;
    DASSERT(!IsBadReadPtr(w, sizeof(AwtFrame)));

    BOOL iconify = (state & java_awt_Frame_ICONIFIED) != 0;
    BOOL zoom = (state & java_awt_Frame_MAXIMIZED_BOTH)
			== java_awt_Frame_MAXIMIZED_BOTH;

    HWND hwnd = w->GetHWnd();
    BOOL focusable = w->IsFocusableWindow();

    DTRACE_PRINTLN4("WFramePeer.setState:%s%s ->%s%s",
                  w->isIconic() ? " iconic" : "",
                  w->isZoomed() ? " zoomed" : "",
                  iconify       ? " iconic" : "",
                  zoom          ? " zoomed" : "");

    if (::IsWindowVisible(hwnd)) {
	// Iconify first if necessary, so that for a complex state
	// transition zoom state is changed when we are iconified - to
	// reduce window flicker.
	if (!w->isIconic() && iconify) {
            if (focusable) {
                ::ShowWindow(hwnd, SW_MINIMIZE);
            } else {
                ::ShowWindow(hwnd, SW_SHOWMINNOACTIVE);
            }
	}

	// If iconified, handle zoom state change while/when in iconic state
	if (zoom != w->isZoomed()) {
	    if (::IsIconic(hwnd)) {
		// Arrange for window to be restored to specified state
		WINDOWPLACEMENT wp;
		::ZeroMemory(&wp, sizeof(WINDOWPLACEMENT));
		wp.length = sizeof(WINDOWPLACEMENT);
		::GetWindowPlacement(hwnd, &wp);

		if (zoom) {
		    wp.flags |= WPF_RESTORETOMAXIMIZED;
		} else {
		    wp.flags &= ~WPF_RESTORETOMAXIMIZED;
		}
		::SetWindowPlacement(hwnd, &wp);
	    }
	    else {
		// Not iconified - just maximize it
                if (focusable) {
                    ::ShowWindow(hwnd, zoom ? SW_SHOWMAXIMIZED : SW_RESTORE);
                } else {
                    ::ShowWindow(hwnd, zoom ? SW_MAXIMIZE : SW_SHOWNOACTIVATE);
                }
	    }
	}

	// Handle deiconify if necessary.
	if (w->isIconic() && !iconify) {
            if (focusable) {
                ::ShowWindow(hwnd, SW_RESTORE);
            } else {
                ::ShowWindow(hwnd, SW_SHOWNOACTIVATE);
            }
	}
    }
#ifdef DEBUG
    else {
      DTRACE_PRINTLN("  not visible, just recording the requested state");
    }
#endif

    w->setIconic(iconify);
    w->setZoomed(zoom);

    CATCH_BAD_ALLOC;
}

/*
 * Class:     sun_awt_windows_WFramePeer
 * Method:    getState
 * Signature: ()I
 */
JNIEXPORT jint JNICALL
Java_sun_awt_windows_WFramePeer_getState(JNIEnv *env, jobject self)
{
    TRY;

    PDATA pData;
    JNI_CHECK_PEER_RETURN_VAL(self, java_awt_Frame_NORMAL);

    AwtFrame* w = (AwtFrame*)pData;
    DASSERT(!IsBadReadPtr(w, sizeof(AwtFrame)));

    jint state = java_awt_Frame_NORMAL;
    if (w->isIconic()) {
	state |= java_awt_Frame_ICONIFIED;
    }
    if (w->isZoomed()) {
	state |= java_awt_Frame_MAXIMIZED_BOTH;
    }

    DTRACE_PRINTLN2("WFramePeer.getState:%s%s",
                  w->isIconic() ? " iconic" : "",
                  w->isZoomed() ? " zoomed" : "");

    return state;

    CATCH_BAD_ALLOC_RET(java_awt_Frame_NORMAL);
}


/*
 * Class:     sun_awt_windows_WFramePeer
 * Method:    setMaximizedBounds
 * Signature: (IIII)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_windows_WFramePeer_setMaximizedBounds(JNIEnv *env, jobject self,
    jint x, jint y, jint width, jint height)
{
    TRY;

    PDATA pData;
    JNI_CHECK_PEER_RETURN(self);

    AwtFrame *w = (AwtFrame *)pData;
    DASSERT(!::IsBadReadPtr(w, sizeof(AwtFrame)));

    w->SetMaximizedBounds(x, y, width, height);

    CATCH_BAD_ALLOC;
}


/*
 * Class:     sun_awt_windows_WFramePeer
 * Method:    clearMaximizedBounds
 * Signature: ()V
 */
JNIEXPORT void JNICALL
Java_sun_awt_windows_WFramePeer_clearMaximizedBounds(JNIEnv *env, jobject self)
{
    TRY;

    PDATA pData;
    JNI_CHECK_PEER_RETURN(self);

    AwtFrame *w = (AwtFrame *)pData;
    DASSERT(!::IsBadReadPtr(w, sizeof(AwtFrame)));

    w->ClearMaximizedBounds();

    CATCH_BAD_ALLOC;
}


/*
 * Class:     sun_awt_windows_WFramePeer
 * Method:    setMenuBar0
 * Signature: (Lsun/awt/windows/WMenuBarPeer;)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_windows_WFramePeer_setMenuBar0(JNIEnv *env, jobject self,
					    jobject mbPeer)
{
    TRY;

    PDATA pData;
    JNI_CHECK_PEER_RETURN(self);
   
    /* Since Local references are not valid in another Thread, we need to
       create a global reference before we send this to the Toolkit thread. 
    */
     
    jobject gref = env->NewGlobalRef(mbPeer);
    AwtObject::WinThreadExec(self, AwtFrame::FRAME_SETMENUBAR, (LPARAM)gref);
    env->DeleteGlobalRef(gref);

    CATCH_BAD_ALLOC;
}

/*
 * Class:     sun_awt_windows_WFramePeer
 * Method:    create
 * Signature: (Lsun/awt/windows/WComponentPeer;)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_windows_WFramePeer_createAwtFrame(JNIEnv *env, jobject self,
                                               jobject parent)
{
    TRY;

    AwtToolkit::CreateComponent(self, parent,
				(AwtToolkit::ComponentFactory)
				AwtFrame::Create);
    PDATA pData;
    JNI_CHECK_PEER_CREATION_RETURN(self);

    CATCH_BAD_ALLOC;
}
/*
 * Class:     sun_awt_windows_WFramePeer
 * Method:    getSysIconHeight
 * Signature: ()I
 */
JNIEXPORT jint JNICALL
Java_sun_awt_windows_WFramePeer_getSysIconHeight(JNIEnv *env, jclass self)
{
    TRY;

    return ::GetSystemMetrics(SM_CYICON);

    CATCH_BAD_ALLOC_RET(0);
}

/*
 * Class:     sun_awt_windows_WFramePeer
 * Method:    getSysIconWidth
 * Signature: ()I
 */
JNIEXPORT jint JNICALL
Java_sun_awt_windows_WFramePeer_getSysIconWidth(JNIEnv *env, jclass self)
{
    TRY;

    return ::GetSystemMetrics(SM_CXICON);

    CATCH_BAD_ALLOC_RET(0);
}

/*
 * Class:     sun_awt_windows_WFramePeer
 * Method:    getSysMenuHeight
 * Signature: ()I
 */
JNIEXPORT jint JNICALL
Java_sun_awt_windows_WFramePeer_getSysMenuHeight(JNIEnv *env, jclass self)
{
    TRY;

    return ::GetSystemMetrics(SM_CYMENUSIZE);

    CATCH_BAD_ALLOC_RET(0);
}

/*
 * Class:     sun_awt_windows_WFramePeer
 * Method:    setIconImageFromIntRasterData
 * Signature: ([I)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_windows_WFramePeer_setIconImageFromIntRasterData
    (JNIEnv *env, jobject self, jintArray intRasterData,
     jbyteArray byteMaskData, jint scanStride, jint nW, jint nH)
{
    TRY;

    PDATA pData;
    JNI_CHECK_PEER_RETURN(self);
    // ok to pass null intRasterData: default AWT icon

    AwtFrame* frame = (AwtFrame *)pData;
    frame->MakeSetIcon(env, intRasterData, byteMaskData, (int)scanStride,
                       (int)nW, (int)nH);

    CATCH_BAD_ALLOC;
}

/*
 * Class:     sun_awt_windows_WFramePeer
 * Method:    pSetIMMOption
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_windows_WFramePeer_pSetIMMOption(JNIEnv *env, jobject self,
					       jstring option) 
{
    TRY;

    PDATA pData;
    LPCTSTR coption;
    LPTSTR empty = TEXT("InputMethod");
    JNI_CHECK_PEER_RETURN(self);
    JNI_CHECK_NULL_RETURN(option, "IMMOption argument");

    AwtFrame* frame = (AwtFrame *)pData;

    if (JNU_IsNull(env, option)) {
        coption = empty;
    } else {
        coption = JNU_GetStringPlatformChars(env, option, NULL);
        if (coption == NULL) {
	    throw std::bad_alloc();
        }
    }

    HMENU hSysMenu = ::GetSystemMenu(frame->GetHWnd(), FALSE);
    ::AppendMenu(hSysMenu,  MF_STRING, SYSCOMMAND_IMM, coption);

    if (coption != empty)
        JNU_ReleaseStringPlatformChars(env, option, coption);

    CATCH_BAD_ALLOC;
}

} /* extern "C" */


/************************************************************************
 * EmbeddedFrame native methods
 */

extern "C" {

/*
 * Class:     sun_awt_EmbeddedFrame
 * Method:    setPeer
 * Signature: (Ljava/awt/peer/ComponentPeer;)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_EmbeddedFrame_setPeer(JNIEnv *env, jobject self, jobject lpeer)
{
    TRY;

    jclass cls;
    jfieldID fid;

    cls = env->GetObjectClass(self);
    fid = env->GetFieldID(cls, "peer", "Ljava/awt/peer/ComponentPeer;");
    env->SetObjectField(self, fid, lpeer);

    CATCH_BAD_ALLOC;
}

} /* extern "C" */


/************************************************************************
 * WEmbeddedFrame native methods
 */

extern "C" {

/*
 * Class:     sun_awt_windows_WFramePeer
 * Method:    initIDs
 * Signature: (Lsun/awt/windows/WMenuBarPeer;)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_windows_WEmbeddedFrame_initIDs(JNIEnv *env, jclass cls)
{
    TRY;

    AwtFrame::handleID = env->GetFieldID(cls, "handle", "J");
    DASSERT(AwtFrame::handleID != NULL);

    CATCH_BAD_ALLOC;
}

JNIEXPORT void JNICALL
Java_sun_awt_windows_WEmbeddedFrame_synthesizeWmActivate(JNIEnv *env, jclass cls,
                                                         jlong parent, jboolean b)
{
    TRY;

    if (b == TRUE) {
        AwtToolkit::GetInstance().InvokeFunction(AwtFrame::ActivateEmbeddedFrame, (HWND)parent);
    } else {
        AwtToolkit::GetInstance().InvokeFunction(AwtFrame::DeactivateEmbeddedFrame, (HWND)parent);
    }
    
    CATCH_BAD_ALLOC;
}

} /* extern "C" */


/************************************************************************
 * WEmbeddedFramePeer native methods
 */

extern "C" {

JNIEXPORT void JNICALL
Java_sun_awt_windows_WEmbeddedFramePeer_create(JNIEnv *env, jobject self,
					       jobject parent)
{
    TRY;

    JNI_CHECK_NULL_RETURN(self, "peer");
    AwtToolkit::CreateComponent(self, parent,
				(AwtToolkit::ComponentFactory)
				AwtFrame::Create);
    PDATA pData;
    JNI_CHECK_PEER_CREATION_RETURN(self);

    CATCH_BAD_ALLOC;
}

} /* extern "C" */
