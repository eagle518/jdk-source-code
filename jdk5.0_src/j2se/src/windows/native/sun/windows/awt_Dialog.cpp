/*
 * @(#)awt_Dialog.cpp	1.83 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "awt_Toolkit.h"
#include "awt_Dialog.h"

#include "java_awt_Dialog.h"

/* IMPORTANT! Read the README.JNI file for notes on JNI converted AWT code.
 */

/************************************************************************
 * AwtDialog fields
 */

jfieldID AwtDialog::titleID;
jfieldID AwtDialog::undecoratedID;

#if defined(DEBUG)
// counts how many nested modal dialogs are open, a sanity
// check to ensure the somewhat complicated disable/enable
// code is working properly
int AwtModalityNestCounter = 0;
#endif

/************************************************************************
 * AwtDialog class methods
 */

AwtDialog::AwtDialog() {
    m_modalWnd = NULL;
}

AwtDialog::~AwtDialog() {
    if (m_modalWnd != NULL) {
	WmEndModal();
    }
    // Fix 4745575 GDI Resource Leak
    // MSDN 
    // Before a window is destroyed (that is, before it returns from processing 
    // the WM_NCDESTROY message), an application must remove all entries it has 
    // added to the property list. The application must use the RemoveProp function
    // to remove the entries. 
    ::RemoveProp(GetHWnd(), ModalDisableProp);
}

LPCTSTR AwtDialog::GetClassName() {
  return TEXT("SunAwtDialog");
}

void AwtDialog::FillClassInfo(WNDCLASS *lpwc)
{
    AwtWindow::FillClassInfo(lpwc);
    // Dialog inherits icon from its owner dinamically
    lpwc->hIcon = NULL;
}

/*
 * Create a new AwtDialog object and window.   
 */
AwtDialog* AwtDialog::Create(jobject peer, jobject parent) 
{
    JNIEnv *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);

    jobject background = NULL;
    jobject target = NULL;
    AwtDialog* dialog = NULL;

    try {
        if (env->EnsureLocalCapacity(2) < 0) {
	    return NULL;
	}

	PDATA pData;
	HWND hwndParent = NULL;
	target = env->GetObjectField(peer, AwtObject::targetID);
	JNI_CHECK_NULL_GOTO(target, "null target", done);
	
	if (parent != NULL) {
	    JNI_CHECK_PEER_GOTO(parent, done);
	    {
	        AwtFrame* awtParent = (AwtFrame *)(JNI_GET_PDATA(parent));
		hwndParent = awtParent->GetHWnd();
	    }
	}
	dialog = new AwtDialog();
	
	{
	    int colorId = IS_WIN4X ? COLOR_3DFACE : COLOR_WINDOW;
	    DWORD style = WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_CLIPCHILDREN;
	    style &= ~(WS_MINIMIZEBOX|WS_MAXIMIZEBOX);
	    DWORD exStyle = IS_WIN4X ? WS_EX_WINDOWEDGE | WS_EX_DLGMODALFRAME
	                             : 0;

	    if (GetRTL()) {
	        exStyle |= WS_EX_RIGHT | WS_EX_LEFTSCROLLBAR;
		if (GetRTLReadingOrder())
		    exStyle |= WS_EX_RTLREADING;
	    }


            if (env->GetBooleanField(target, AwtDialog::undecoratedID) == JNI_TRUE) {
                style = WS_POPUP | WS_CLIPCHILDREN;
                exStyle = 0;
                dialog->m_isUndecorated = TRUE;
            }

	    jint x = env->GetIntField(target, AwtComponent::xID);
	    jint y = env->GetIntField(target, AwtComponent::yID);
	    jint width = env->GetIntField(target, AwtComponent::widthID);
	    jint height = env->GetIntField(target, AwtComponent::heightID);

	    dialog->CreateHWnd(env, L"",
			       style, exStyle,
			       x, y, width, height,
			       hwndParent,
			       NULL,
			       ::GetSysColor(COLOR_WINDOWTEXT),
			       ::GetSysColor(colorId),
			       peer);

	    dialog->RecalcNonClient();
	    dialog->UpdateDialogIcon();
	    dialog->UpdateSystemMenu();

	    background = env->GetObjectField(target,
					     AwtComponent::backgroundID);
	    if (background == NULL) {
 	        JNU_CallMethodByName(env, NULL,
 				     peer, "setDefaultColor", "()V");
	    }
	}
    } catch (...) {
        env->DeleteLocalRef(background);
	env->DeleteLocalRef(target);
	throw;
    }

done:
    env->DeleteLocalRef(background);
    env->DeleteLocalRef(target);

    return dialog;
}

MsgRouting AwtDialog::WmNcMouseDown(WPARAM hitTest, int x, int y, int button) {
    if (!IsFocusableWindow()) {
        // Dialog is non-maximizable
        if ((button & DBL_CLICK) == DBL_CLICK && hitTest == HTCAPTION) {
            return mrConsume;
        }
    }
    return AwtFrame::WmNcMouseDown(hitTest, x, y, button);
}
//
// Disables top-level windows for a modal dialog
//	hWndDlg		Modal dialog 
//
void AwtDialog::ModalDisable(HWND hWndDlg)
{
#if defined(DEBUG)
    DASSERT(AwtModalityNestCounter >= 0 && AwtModalityNestCounter <= 1000); // sanity check
    AwtModalityNestCounter++;
#endif
    if (hWndDlg != NULL) {
        AwtDialog::ResetDisabledLevel(hWndDlg);
    }
    ::EnumThreadWindows(AwtToolkit::MainThread(),
		    (WNDENUMPROC)DisableTopLevelsCallback,(LPARAM)hWndDlg) ;
}

// 
// Re-enables top-level windows for a modal dialog
//	hWndDlg		Modal dialog
//
void AwtDialog::ModalEnable(HWND hWndDlg)
{
#if defined(DEBUG)
    DASSERT(AwtModalityNestCounter > 0 && AwtModalityNestCounter <= 1000); // sanity check
    AwtModalityNestCounter--;
#endif
    int disabledLevel = (hWndDlg != NULL)
        ? AwtDialog::GetDisabledLevel(hWndDlg)
        : 0;
    ::EnumThreadWindows(AwtToolkit::MainThread(),
		    (WNDENUMPROC)EnableTopLevelsCallback, (LPARAM)disabledLevel);
    if (hWndDlg != NULL) {
        AwtDialog::ResetDisabledLevel(hWndDlg);
    }
}

//
// Brings a window to foreground when modal dialog goes away
// (the Win32 Z-order is very screwy if another app is active
//  when the modal dialog goes away, so we explicitly activate 
//  the first enabled, visible, window we find)
//
//	hWndParent	Parent window of the dialog
//
void AwtDialog::ModalNextWindowToFront(HWND hWndParent)
{
    if ( ::IsWindowEnabled(hWndParent) && ::IsWindowVisible(hWndParent) ) {
    // always give parent first shot at coming to the front
	::SetForegroundWindow(hWndParent);
    } else {
    // otherwise, activate the first enabled, visible window we find
	::EnumThreadWindows(AwtToolkit::MainThread(),
		    (WNDENUMPROC)NextWindowToFrontCallback, 0L );
    }
}

//
// Sets the disabled level of a new Frame to the greatest value of all
// top-level HWNDs. That way, the Frame will not be reenabled until all
// modal dialogs have been hidden
//
//      frame          AwtFrame object whose disabled level should be set
//
void AwtDialog::SetDisabledLevelToGreatest(AwtFrame *frame)
{
    int greatest = 0;

    ::EnumThreadWindows(AwtToolkit::MainThread(),
			(WNDENUMPROC)FindGreatestDisabledLevelCallback,
                        (LPARAM)&greatest);
    DASSERT(greatest >= 0);
    if (greatest > 0) {
        SetDisabledLevel(frame->GetHWnd(), greatest);
        ::EnableWindow(frame->GetHWnd(), FALSE);
    }
}

//
// Disables top-level windows, incrementing a counter on each disabled
// window to indicate how many nested modal dialogs need the window disabled
//
BOOL CALLBACK AwtDialog::DisableTopLevelsCallback(HWND hWnd, LPARAM lParam)
{
    TRY;

    HWND	hWndDlg = (HWND)lParam;

    if (hWnd == hWndDlg || hWnd == AwtToolkit::GetInstance().GetHWnd()) {
	return TRUE; // ignore myself and toolkit when disabling
    }

    //skip windows that are descendants of the dialog.  Fix for 4490830.
    if (hWndDlg != NULL) {
        HWND  hWndNext = hWnd;
        while(hWndNext != NULL) {
            if ((hWndNext = ::GetParent(hWndNext)) == hWndDlg) {
                return TRUE;
            }
        }
    }

	
    // the drop-down list portion of combo-boxes are actually top-level
    // windows with WS_CHILD, so ignore these guys...
    if ( (::GetWindowLong(hWnd, GWL_STYLE) & WS_CHILD) != 0) {
	return TRUE;
    }

    DASSERT(hWndDlg == NULL || IsWindow(hWndDlg));

    AwtDialog::IncrementDisabledLevel(hWnd, 1);
    return TRUE;

    CATCH_BAD_ALLOC_RET(FALSE);
}

//
// Enables top-level window if no modal dialogs need it disabled
//
BOOL CALLBACK AwtDialog::EnableTopLevelsCallback(HWND hWnd, LPARAM lParam)
{
    TRY;

    int   disabledLevel = AwtDialog::GetDisabledLevel(hWnd);
    int   dlgDisabledLevel = (int)lParam;

    if (disabledLevel == 0 || disabledLevel  < dlgDisabledLevel) {
    // skip enabled windows or windows disabled after the 
    // given modal dialog became modal
	return TRUE;
    }

    AwtDialog::IncrementDisabledLevel(hWnd, -1);
    return TRUE;

    CATCH_BAD_ALLOC_RET(FALSE);
}

//
// Brings the first enabled, visible window to the foreground
//
BOOL CALLBACK AwtDialog::NextWindowToFrontCallback(HWND hWnd, LPARAM lParam)
{
    TRY;

    if ( ::IsWindowVisible(hWnd) && ::IsWindowEnabled(hWnd) ) {
	::SetForegroundWindow(hWnd);
	return FALSE; // stop callbacks
    }
    return TRUE;

    CATCH_BAD_ALLOC_RET(FALSE);
}

//
// Iff hWnd's disabled level > lParam, set lParam to hWnd's disabled level
//
BOOL CALLBACK AwtDialog::FindGreatestDisabledLevelCallback(HWND hWnd,
                                                           LPARAM lParam)
{
    TRY;

    int *greatest = (int *)lParam;

    int disabledLevel = GetDisabledLevel(hWnd);
    if (disabledLevel > *greatest) {
        *greatest = disabledLevel;
    }

    return TRUE;

    CATCH_BAD_ALLOC_RET(FALSE);
}
    
int AwtDialog::GetDisabledLevel(HWND hWnd) {
    int disableProp = static_cast<int>(reinterpret_cast<INT_PTR>(
        ::GetProp(hWnd, ModalDisableProp)));
    int disabledLevel = disableProp & DISABLED_LEVEL_MASK;
    DASSERT(disabledLevel >= 0 && disabledLevel <= 1000);
    return disabledLevel;
}

void AwtDialog::SetDisabledLevel(HWND hWnd, int disabledLevel) {
    DASSERT(disabledLevel >= 0 && disabledLevel <= 1000);
    int disableProp = static_cast<int>(reinterpret_cast<INT_PTR>(
        ::GetProp(hWnd, ModalDisableProp)));
    disableProp = disabledLevel | (disableProp & INITIALLY_DISABLED_MASK);
    ::SetProp(hWnd, ModalDisableProp, reinterpret_cast<HANDLE>(
        static_cast<INT_PTR>(disableProp)));
}

BOOL AwtDialog::IsInitiallyDisabled(HWND hWnd) {
    int disableProp = static_cast<int>(reinterpret_cast<INT_PTR>(
        ::GetProp(hWnd, ModalDisableProp)));
    return (disableProp & INITIALLY_DISABLED_MASK) != 0;
}

void AwtDialog::SetInitiallyDisabled(HWND hWnd) {
    int disableProp = static_cast<int>(reinterpret_cast<INT_PTR>(
        ::GetProp(hWnd, ModalDisableProp)));
    ::SetProp(hWnd, ModalDisableProp, reinterpret_cast<HANDLE>(
        static_cast<INT_PTR>(disableProp | INITIALLY_DISABLED_MASK)));
}

#if 0 /* defined(DEBUG) */
// debugging-only function; useful for tracking down disable/enable problems
static void DbgShowDisabledLevel(HWND hWnd, int disabledLevel)
{    
    // put disabled counter in title bar text
    char	szDisabled[256];
    wsprintf(szDisabled, "disabledLevel = %d", disabledLevel);
    ::SetWindowText(hWnd, szDisabled);
}
    #define DBG_SHOW_DISABLED_LEVEL( _win, _dl ) \
	    DbgShowDisabledLevel( (_win), (_dl) )
#else    
    #define DBG_SHOW_DISABLED_LEVEL( _win, _dl )
#endif

void AwtDialog::IncrementDisabledLevel(HWND hWnd, int increment) {
    int disabledLevel = GetDisabledLevel(hWnd);
    DASSERT( increment == -1 || increment == 1); // only allow increment/decrement
    BOOL enabled = FALSE;
    disabledLevel += increment;
    AwtComponent* hWndComponent = AwtComponent::GetComponent(hWnd);
    if (disabledLevel == 0 ) {
        if (hWndComponent != NULL)  {          //in case hWnd isn't from AWT
            enabled = hWndComponent->isEnabled();
        } else {
            enabled = !IsInitiallyDisabled(hWnd);
        }
	// don't need the property any more...
	ResetDisabledLevel(hWnd);
    } else {
        // remember the initial state of non-AWT window for later restoration
        if ((hWndComponent == NULL) && 
	    (disabledLevel == 1) && (increment == 1) &&
	    !::IsWindowEnabled(hWnd)) {
	    SetInitiallyDisabled(hWnd);
	}
	SetDisabledLevel(hWnd, disabledLevel);
    }

    ::EnableWindow(hWnd, enabled);
    DBG_SHOW_DISABLED_LEVEL(hWnd, GetDisabledLevel(hWnd));
}

void AwtDialog::ResetDisabledLevel(HWND hWnd) {
    ::RemoveProp(hWnd, ModalDisableProp);
    DBG_SHOW_DISABLED_LEVEL(hWnd, GetDisabledLevel(hWnd));
}

void AwtDialog::Show()
{
    JNIEnv *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);
    BOOL focusableWindow =
        (env->GetBooleanField(GetPeer(env), AwtWindow::focusableWindowID) ==
         JNI_TRUE);

    BOOL locationByPlatform = env->GetBooleanField(GetTarget(env), AwtWindow::locationByPlatformID);
    if (locationByPlatform) {
         moveToDefaultLocation();
    }

    if (!focusableWindow) {
        ::ShowWindow(GetHWnd(), SW_SHOWNA);
    } else {
        ::ShowWindow(GetHWnd(), SW_SHOW);
    }
}

MsgRouting AwtDialog::WmShowModal()
{
    DASSERT(::GetCurrentThreadId() == AwtToolkit::MainThread());

    // disable top-level windows
    AwtDialog::ModalDisable( GetHWnd() );
    // enable myself (could have been explicitly disabled via setEnabled)
    ::EnableWindow(GetHWnd(), TRUE); // is this the right behaviour? (rnk 6/4/98)
    // show window and set focus to it
    SendMessage(WM_AWT_COMPONENT_SHOW);
    // normally done by DefWindowProc(WM_ACTIVATE),
    // but we override that message so we do it explicitly
    ::SetFocus(GetHWnd());

    m_modalWnd = GetHWnd();

    return mrConsume;
}

MsgRouting AwtDialog::WmEndModal()
{
    DASSERT( ::GetCurrentThreadId() == AwtToolkit::MainThread() );
    DASSERT( ::IsWindowVisible( GetHWnd() ) );
    DASSERT( ::IsWindow(m_modalWnd) );

    // re-enable top-level windows
    AwtDialog::ModalEnable( GetHWnd() );
    
    HWND	hWndParent = ::GetParent( GetHWnd() );
    BOOL	isEnabled = ::IsWindowEnabled(hWndParent);

    // Need to temporarily enable the dialog's owner window.
    // If we don't enable the owner, Windows will activate
    // the last active top-level window, which could be
    // another application entirely (the dreaded bug 4065506).
    ::EnableWindow(hWndParent, TRUE);
    // hide the dialog
    SendMessage(WM_AWT_COMPONENT_HIDE);
    // restore the owner's true enabled state
    ::EnableWindow(hWndParent, isEnabled);

    // bring the next window in the stack to the front
    AwtDialog::ModalNextWindowToFront( hWndParent );
    m_modalWnd = NULL;

    return mrConsume;
}

void AwtDialog::SetResizable(BOOL isResizable)
{
    // call superclass
    AwtFrame::SetResizable(isResizable);

    LONG    style = GetStyle();
    LONG    xstyle = GetStyleEx();
    if (isResizable || IsUndecorated()) {
    // remove modal frame
	xstyle &= ~WS_EX_DLGMODALFRAME;
    } else {
    // add modal frame
	xstyle |= WS_EX_DLGMODALFRAME;
    }
    // dialogs are never minimizable/maximizable, so remove those bits
    style &= ~(WS_MINIMIZEBOX|WS_MAXIMIZEBOX);
    SetStyle(style);
    SetStyleEx(xstyle);
    RedrawNonClient();
}

// Adjust system menu so that:
//  Non-resizable dialogs only have Move and Close items
//  Resizable dialogs also have Size
// Normally, Win32 dialog system menu handling is done via
// CreateDialog/DefDlgProc, but our dialogs are using DefWindowProc
// so we handle the system menu ourselves
// fixes 4057529 - robi.khan@eng
void AwtDialog::UpdateSystemMenu()
{
    HWND    hWndSelf = GetHWnd();
    BOOL    isResizable = ((GetStyle() & WS_THICKFRAME) != 0);
    BOOL    isMaximized = ::IsZoomed(hWndSelf);

    // before restoring the default menu, check if there is an
    // InputMethodManager menu item already.  Note that it assumes
    // that the length of the InputMethodManager menu item string
    // should not be longer than 256 bytes.
    MENUITEMINFO  mii;
    memset(&mii, 0, sizeof(MENUITEMINFO));
    TCHAR         immItem[256];
    BOOL          hasImm;
    mii.cbSize = sizeof(MENUITEMINFO);
    mii.fMask = MIIM_TYPE;
    mii.cch = sizeof(immItem);
    mii.dwTypeData = immItem;
    hasImm = ::GetMenuItemInfo(GetSystemMenu(hWndSelf, FALSE),
                               SYSCOMMAND_IMM, FALSE, &mii);

    // restore the default menu
    ::GetSystemMenu(hWndSelf, TRUE);
    // now get a working copy of the menu
    HMENU   hMenuSys = GetSystemMenu(hWndSelf, FALSE);
    // remove inapplicable sizing commands
    ::DeleteMenu(hMenuSys, SC_MINIMIZE, MF_BYCOMMAND);
    ::DeleteMenu(hMenuSys, SC_RESTORE, MF_BYCOMMAND);
    ::DeleteMenu(hMenuSys, SC_MAXIMIZE, MF_BYCOMMAND);
    if (!isResizable) {
	::DeleteMenu(hMenuSys, SC_SIZE, MF_BYCOMMAND);
    }
    // remove separator if only 3 items left (Move, Separator, and Close)
    if (::GetMenuItemCount(hMenuSys) == 3) {
	MENUITEMINFO	mi;
        memset(&mi, 0, sizeof(MENUITEMINFO));
	mi.cbSize = sizeof(MENUITEMINFO);
	mi.fMask = MIIM_TYPE;
	::GetMenuItemInfo(hMenuSys, 1, TRUE, &mi);
	if (mi.fType & MFT_SEPARATOR) {
	    ::DeleteMenu(hMenuSys, 1, MF_BYPOSITION);
	}
    }

    // if there was the InputMethodManager menu item, restore it.
    if (hasImm)
        ::AppendMenu(hMenuSys, MF_STRING, SYSCOMMAND_IMM, immItem);
}

// Call this when dialog isResizable property changes, to 
// hide or show the small icon in the upper left corner
void AwtDialog::UpdateDialogIcon()
{
    BOOL isResizable = ((GetStyle() & WS_THICKFRAME) != 0);

    if (isResizable) { // if we're resizable, use the owner's icon

        HWND hValidIconOwner = NULL; // Last Non-NULL owner of the icon
        HWND hIconOwner = GetHWnd(); // Owner of the icon

        HICON hBigIcon = NULL;
        HICON hSmallIcon = NULL;
        BOOL ContinueLooking = TRUE;

        do {
            // Try to get a valid icon from the current window
            if (hSmallIcon == NULL) {
                hSmallIcon = (HICON)::SendMessage(hIconOwner, WM_GETICON, ICON_SMALL, NULL);
            }
	if (hBigIcon == NULL) {
                hBigIcon = (HICON)::SendMessage(hIconOwner, WM_GETICON, ICON_BIG, NULL);
	}
            if (hIconOwner != NULL) {
                hValidIconOwner = hIconOwner;
            }
            // If the current window doesn't have a valid icon, ascend one level
            hIconOwner = ::GetWindow(hIconOwner, GW_OWNER);

            if (hIconOwner == NULL) {
                // If we failed to get a valid icon from the parents, acquire
                // default Java Coffee Cup icon
	if (hSmallIcon == NULL) {
                    hSmallIcon = (HICON)::GetClassLongPtr(hValidIconOwner, GCLP_HICONSM);
	}
                if (hBigIcon == NULL) {
                    hBigIcon = (HICON)::GetClassLongPtr(hValidIconOwner, GCLP_HICON);
                }
                // If we failed to retrieve java coffee cup icon (This should
                // never happen!), assign default window icon and exit the loop
	if (hSmallIcon == NULL) {
                    hSmallIcon = (HICON)::LoadIcon(NULL, IDI_WINLOGO);
                    ContinueLooking = FALSE;
	}
                if (hBigIcon == NULL) {
                    hBigIcon = (HICON)::LoadIcon(NULL, IDI_WINLOGO);
                    ContinueLooking = FALSE;
                }
            }

        } while ((hSmallIcon == NULL || hBigIcon == NULL) && ContinueLooking);

	SendMessage(WM_SETICON, ICON_SMALL, (LPARAM)hSmallIcon);
	SendMessage(WM_SETICON, ICON_BIG,   (LPARAM)hBigIcon);

    } else {
	// if we're not resizable remove the icon
	SendMessage(WM_SETICON, ICON_BIG,   NULL);
	SendMessage(WM_SETICON, ICON_SMALL, NULL);
    }
}

// Override WmStyleChanged to adjust system menu for sizable/non-resizable dialogs
MsgRouting AwtDialog::WmStyleChanged(int wStyleType, LPSTYLESTRUCT lpss)
{
    UpdateSystemMenu();
    UpdateDialogIcon();
    return mrConsume;
}

MsgRouting AwtDialog::WmSize(UINT type, int w, int h)
{
    UpdateSystemMenu(); // adjust to reflect restored vs. maximized state
    return AwtFrame::WmSize(type, w, h);
}

LRESULT AwtDialog::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    MsgRouting mr = mrDoDefault;
    LRESULT retValue = 0L;

    switch(message) {
	case WM_AWT_DLG_SHOWMODAL:
	    mr = WmShowModal();
	    break;
	case WM_AWT_DLG_ENDMODAL:
	    mr = WmEndModal();
	    break;
    }

    if (mr != mrConsume) {
        retValue = AwtFrame::WindowProc(message, wParam, lParam);
    }
    return retValue;
}

/************************************************************************
 * Dialog native methods
 */

extern "C" {

JNIEXPORT void JNICALL
Java_java_awt_Dialog_initIDs(JNIEnv *env, jclass cls)
{
    TRY;

    AwtDialog::titleID = env->GetFieldID(cls, "title", "Ljava/lang/String;");
    DASSERT(AwtDialog::titleID != NULL); 
        
    AwtDialog::undecoratedID = env->GetFieldID(cls,"undecorated","Z");
    DASSERT(AwtDialog::undecoratedID != NULL);     

    CATCH_BAD_ALLOC;
}

} /* extern "C" */


/************************************************************************
 * DialogPeer native methods
 */

extern "C" {

/*
 * Class:     sun_awt_windows_WDialogPeer
 * Method:    create
 * Signature: (Lsun/awt/windows/WComponentPeer;)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_windows_WDialogPeer_create(JNIEnv *env, jobject self,
					jobject parent) 
{
    TRY;

    PDATA pData;
    AwtToolkit::CreateComponent(self, parent, 
				(AwtToolkit::ComponentFactory)
				AwtDialog::Create);
    JNI_CHECK_PEER_CREATION_RETURN(self);

    CATCH_BAD_ALLOC;
}

/*
 * Class:     sun_awt_windows_WDialogPeer
 * Method:    _show
 * Signature: ()V
 */
JNIEXPORT void JNICALL
Java_sun_awt_windows_WDialogPeer_showModal(JNIEnv *env, jobject self)
{
    TRY;

    PDATA pData;
    JNI_CHECK_PEER_RETURN(self);
    AwtDialog* d = (AwtDialog*)pData;
    d->SendMessage(WM_AWT_DLG_SHOWMODAL);

    CATCH_BAD_ALLOC;
}

/*
 * Class:     sun_awt_windows_WDialogPeer
 * Method:    _hide
 * Signature: ()V
 */
JNIEXPORT void JNICALL 
Java_sun_awt_windows_WDialogPeer_endModal(JNIEnv *env, jobject self)
{
    TRY;

    PDATA pData;
    JNI_CHECK_PEER_RETURN(self);
    AwtDialog* d = (AwtDialog*)pData;
    d->SendMessage(WM_AWT_DLG_ENDMODAL);

    CATCH_BAD_ALLOC;
}

/*
 * Class:     sun_awt_windows_WFramePeer
 * Method:    pSetIMMOption
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_windows_WDialogPeer_pSetIMMOption(JNIEnv *env, jobject self,
					       jstring option) 
{
    TRY;

    PDATA pData;
    LPCTSTR coption;
    LPTSTR empty = TEXT("InputMethod");
    JNI_CHECK_PEER_RETURN(self);
    JNI_CHECK_NULL_RETURN(option, "IMMOption argument");

    AwtDialog* dialog = (AwtDialog *)pData;

    if (JNU_IsNull(env, option)) {
        coption = empty;
    } else {
        coption = JNU_GetStringPlatformChars(env, option, NULL);
	if (coption == NULL) {
	    throw std::bad_alloc();
	}
    }

    HMENU hSysMenu = ::GetSystemMenu(dialog->GetHWnd(), FALSE);
    ::AppendMenu(hSysMenu,  MF_STRING, SYSCOMMAND_IMM, coption);

    if (coption != empty)
        JNU_ReleaseStringPlatformChars(env, option, coption);

    CATCH_BAD_ALLOC;
}
} /* extern "C" */
