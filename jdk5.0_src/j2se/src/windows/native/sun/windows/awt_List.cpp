/*
 * @(#)awt_List.cpp	1.79 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "awt_List.h"
#include "awt_KeyboardFocusManager.h"
#include "awt_Canvas.h"
#include "awt_Dimension.h"
#include "awt_Unicode.h"
#include "awt_Toolkit.h"

/* IMPORTANT! Read the README.JNI file for notes on JNI converted AWT code.
 */

/************************************************************************
 * AwtList methods
 */

AwtList::AwtList() {
    isMultiSelect = FALSE;
    isWrapperPrint = FALSE;
    m_listDefWindowProc = NULL;
    m_hList = NULL;    
}

AwtList::~AwtList()
{
    DestroyList();
}

LPCTSTR AwtList::GetClassName() {
    return TEXT("SunAwtListWrap");
}

/*
 * Window proc for the wrap. 
 * Only awt and list owner messages need special processing.
 */
LRESULT CALLBACK AwtList::WrapProc(HWND hWnd, UINT message, 
				       WPARAM wParam, LPARAM lParam)
{
    TRY;

    DASSERT(::IsWindow(hWnd));

    AwtList* self = (AwtList *)GetComponentImpl(hWnd);
    DASSERT(_tcscmp(self->GetClassName(), TEXT("SunAwtListWrap")) == 0);

    if (!self || self->GetHWnd() != hWnd) {
        return ::DefWindowProc(hWnd, message, wParam, lParam);
    }

    /* 
     * Trap WM_SETFOCUS for the wrapper to pass the focus to the listbox.
     * This would block the wrapper from ever getting the focus either 
     * from Java or the user.
     * Trap WM_PRINT for the wrapper to pass it to the listbox default
     * window procedure after non-default processing.
     */
    if (IsListOwnerMessage(message) || IsAwtMessage(message) || 
	message == WM_PRINT)
    {
        self->isWrapperPrint = (message == WM_PRINT);
        LRESULT lres = self->WindowProc(message, wParam, lParam);
        self->isWrapperPrint = FALSE;
        return lres;
    } else if (message == WM_SETFOCUS) {
	::SetFocus(self->GetListHandle());
	return 0;
    } else {
	return ::DefWindowProc(hWnd, message, wParam, lParam);
    }

    CATCH_BAD_ALLOC_RET(0);
}

/*
 * Window proc for the listbox. 
 * Passes all messages to the parent window proc. 
 */
LRESULT CALLBACK AwtList::ListProc(HWND hWnd, UINT message, 
					  WPARAM wParam, LPARAM lParam)
{
    TRY;

    DASSERT(::IsWindow(hWnd));

    AwtList* parent = (AwtList*)GetComponentImpl(::GetParent(hWnd));

    if (!parent || parent->GetListHandle() != hWnd || message == AwtComponent::WmAwtIsComponent ) {
        return ::DefWindowProc(hWnd, message, wParam, lParam);
    }

    return parent->WindowProc(message, wParam, lParam);

    CATCH_BAD_ALLOC_RET(0);
}

/*
 * If the message was initially sent to a listbox owner (i.e. wrap) 
 * pass it to the wrap's default window proc. Otherwise call the 
 * listbox's default window proc, or if none set, call the stock one. 
 */
LRESULT AwtList::DefWindowProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
    DASSERT(!IsAwtMessage(msg));
    if (IsListOwnerMessage(msg)) {
        return AwtComponent::DefWindowProc(msg, wParam, lParam);
    }
    return m_listDefWindowProc
        ? ::CallWindowProc(m_listDefWindowProc, 
                           GetListHandle(), msg, 
                           wParam, lParam)
        : ::DefWindowProc(GetListHandle(), msg, wParam, lParam);
}

/* Create a new AwtList object and window.   */
AwtList* AwtList::Create(jobject peer, jobject parent)
{
    JNIEnv *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);

    jobject target = NULL;
    AwtList* c = NULL;

    try {
        if (env->EnsureLocalCapacity(1) < 0) {
	    return NULL;
	}

	PDATA pData;
	AwtCanvas* awtParent;
	JNI_CHECK_PEER_GOTO(parent, done);

	awtParent = (AwtCanvas*)pData;
	JNI_CHECK_NULL_GOTO(awtParent, "null awtParent", done);

	/* target is Hjava_awt_List * */
	target = env->GetObjectField(peer, AwtObject::targetID);
	JNI_CHECK_NULL_GOTO(target, "null target", done);

	c = new AwtList();
	
	{

	    /*
	     * NOTE: WS_CLIPCHILDREN is excluded so that repaint requests 
	     * from Java will pass through the wrap to the native listbox. 
	     */
	    DWORD wrapStyle = WS_CHILD | WS_CLIPSIBLINGS; 
	    DWORD wrapExStyle = 0;

	    DWORD style = WS_CHILD | WS_CLIPSIBLINGS | WS_VSCROLL | WS_HSCROLL |
	      LBS_NOINTEGRALHEIGHT | LBS_NOTIFY | LBS_OWNERDRAWFIXED |
	      (IS_WIN4X ? 0 : WS_BORDER);
	    DWORD exStyle = IS_WIN4X ? WS_EX_CLIENTEDGE : 0;

	    /*
	     * NOTE: WS_VISIBLE is always set for the listbox. Listbox 
	     * visibility is controlled by toggling the wrap's WS_VISIBLE bit. 
	     */
	    style |= WS_VISIBLE;

	    if (GetRTL()) {
	        exStyle |= WS_EX_RIGHT | WS_EX_LEFTSCROLLBAR;
		if (GetRTLReadingOrder())
		    exStyle |= WS_EX_RTLREADING;
	    }

	    jint x = env->GetIntField(target, AwtComponent::xID);
	    jint y = env->GetIntField(target, AwtComponent::yID);
	    jint width = env->GetIntField(target, AwtComponent::widthID);
	    jint height = env->GetIntField(target, AwtComponent::heightID);

	    c->CreateHWnd(env, L"", wrapStyle, wrapExStyle,
			  x, y, width, height,
			  awtParent->GetHWnd(),
			  NULL,
			  ::GetSysColor(COLOR_WINDOWTEXT),
			  ::GetSysColor(COLOR_WINDOW),
			  peer
			  );

	    c->CreateList(style, exStyle, width, height);

	    /* suppress inheriting awtParent's color. */
	    c->m_backgroundColorSet = TRUE;
            c->UpdateBackground(env, target);
	}
    } catch (...) {
        env->DeleteLocalRef(target);
	throw;
    }

done:
    env->DeleteLocalRef(target);
    return c;
}

/*
 * Install common window proc, save off the previous proc as the default.
 * Then install listbox-specific wrap's window proc.
 */
void AwtList::SubclassHWND()
{
    AwtComponent::SubclassHWND();
    ::SetWindowLongPtr(GetHWnd(), GWLP_WNDPROC, 
                    (UINT_PTR)AwtList::WrapProc);
}

void AwtList::CreateList(DWORD style, DWORD exStyle, int w, int h) {

    DASSERT(GetHWnd() != NULL);
    DASSERT(GetListHandle() == NULL);

    HWND hList = ::CreateWindowEx(exStyle,
				  TEXT("LISTBOX"),
				  TEXT(""),
				  style,
				  0, 0, w, h,
				  GetHWnd(),
				  NULL,
				  AwtToolkit::GetInstance().GetModuleHandle(),
				  NULL);
    DASSERT(hList != NULL);
    SetListHandle(hList);
    
    ::ImmAssociateContext(GetListHandle(),NULL);
    
    /* Subclass the window now so that we can snoop on its messages */
    m_listDefWindowProc = 
	(WNDPROC)::SetWindowLongPtr(GetListHandle(), GWLP_WNDPROC, 
				 (UINT_PTR)AwtList::ListProc);
    SetWindowPos(hList, 0, 0, 0, w, h, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOCOPYBITS);    
}

void AwtList::DestroyList() {
    
    HWND hList = SetListHandle(NULL);
    ::SetWindowLongPtr(hList, GWLP_WNDPROC, 
                    (UINT_PTR)m_listDefWindowProc);
    ::DestroyWindow(hList);
}

BOOL AwtList::ActMouseMessage(MSG * pMsg) {
    if (!IsFocusingMessage(pMsg->message)) {
        return FALSE;
    }
    
    if (pMsg->message == WM_LBUTTONDOWN) {
        LONG item = static_cast<LONG>(SendListMessage(LB_ITEMFROMPOINT, 0, pMsg->lParam));
        if (item != LB_ERR) {
            if (isMultiSelect) {
                if (IsItemSelected(item)) {
                    Deselect(item);
                } else {
                    Select(item);
                }
            } else {
                Select(item);
            }
        }
    }
    return TRUE;
}

void AwtList::SetDragCapture(UINT flags)
{
    // don't want to interfere with other controls
    if (::GetCapture() == NULL) {
        ::SetCapture(GetWrappeeHandle());
    }
}

void AwtList::ReleaseDragCapture(UINT flags)
{
    if ((::GetCapture() == GetWrappeeHandle()) && ((flags & ALL_MK_BUTTONS) == 0)) {
        ::ReleaseCapture();
    }
}

void AwtList::Reshape(int x, int y, int w, int h)
{
    AwtComponent::Reshape(x, y, w, h);

    HWND hList = GetListHandle();
    if (hList != NULL) {
        long flags = SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOCOPYBITS;	
        /*
         * Fix for bug 4046446.
         */
        SetWindowPos(hList, 0, 0, 0, w, h, flags);
    }
}

//Netscape : Override the AwtComponent method so we can set the item height
//for each item in the list.  Modified by echawkes to avoid race condition.  

void AwtList::SetFont(AwtFont* font)
{
    DASSERT(font != NULL);
    if (font->GetAscent() < 0)
    {
        AwtFont::SetupAscent(font);
    }
    HANDLE hFont = font->GetHFont();
    SendListMessage(WM_SETFONT, (WPARAM)hFont, MAKELPARAM(FALSE, 0));

    HDC hDC = ::GetDC(GetHWnd());
    
    TEXTMETRIC tm;
    VERIFY(::SelectObject(hDC, hFont) != NULL);
    VERIFY(::GetTextMetrics(hDC, &tm));

    ::ReleaseDC(GetHWnd(), hDC);

    long h = tm.tmHeight + tm.tmExternalLeading;

    int nCount = (int)SendListMessage(LB_GETCOUNT);
    int i;
    for(i=0; i<nCount; i++)
    {
        VERIFY(SendListMessage(LB_SETITEMHEIGHT, i, MAKELPARAM(h, 0)) != LB_ERR);
    }
    VERIFY(::RedrawWindow(GetHWnd(), NULL, NULL, RDW_INVALIDATE |RDW_FRAME |RDW_ERASE));
}

void AwtList::SetMultiSelect(BOOL ms) {
    if (ms == isMultiSelect) {
        return;
    }
    isMultiSelect = ms;

    /* Copy current box's contents to string array */
    int nCount = GetCount();
    LPTSTR * strings = new LPTSTR[nCount];
 
    for(int i = 0; i < nCount; i++) {
        LRESULT len = SendListMessage(LB_GETTEXTLEN, i);
	LPTSTR text = NULL;
	try {
	    text = new TCHAR[len + 1];
	} catch (std::bad_alloc&) {
	    // free char * already allocated
	    for (int j = 0; j < i; j++) {
	        delete [] strings[j];
	    }
	    delete [] strings;
	    throw;
	}

        VERIFY(SendListMessage(LB_GETTEXT, i, (LPARAM)text) != LB_ERR);
        strings[i] = text;
    }
    int nCurSel = GetCurrentSelection();
    /*
     * GetCurrentSelection() returns 0 for multi-select
     * lists even if no items were selected.
     * Set nCurSel to -1 in this case for consistency.
     *
     * We set nCurSel to -1 also in the case when
     * the selection mode is changed from multiple-selection to
     * single-selection and no selected item has the 
     * location cursor (the focus rectangle).
     */
    if (!isMultiSelect 
        && (SendListMessage(LB_GETSELCOUNT) == 0 
            || SendListMessage(LB_GETSEL, nCurSel) == 0)) {
        nCurSel = -1;
    }

    /* Save old list box's attributes */
    RECT rect;
    GetWindowRect(GetListHandle(), &rect);
    MapWindowPoints(0, GetHWnd(), (LPPOINT)&rect, 2);

    HANDLE font = (HANDLE)SendListMessage(WM_GETFONT);
    DWORD style = ::GetWindowLong(GetListHandle(), GWL_STYLE) | WS_VSCROLL | WS_HSCROLL;
    if (isMultiSelect) {
        style |= LBS_MULTIPLESEL;
    } else {    
        style &= ~LBS_MULTIPLESEL;
    } 
    DWORD exStyle = ::GetWindowLong(GetListHandle(), GWL_EXSTYLE);

    DestroyList();
    CreateList(style, exStyle, rect.right-rect.left, rect.bottom-rect.top);

    SendListMessage(WM_SETFONT, (WPARAM)font, (LPARAM)FALSE);
    SendListMessage(LB_RESETCONTENT);  
    for (i = 0; i < nCount; i++) {
        InsertString(i, strings[i]);
        delete [] strings[i];
    }
    delete[] strings;
    if (nCurSel != -1) {
        Select(nCurSel);
    }
    AdjustHorizontalScrollbar();
}

/*
 * There currently is no good place to cache java.awt.Dimension field
 * ids. If this method gets called a lot, one such place should be found.
 * -- br 07/18/97.
 */
jobject AwtList::PreferredItemSize(JNIEnv *env)
{
    jobject peer = GetPeer(env);
    jobject dimension = JNU_CallMethodByName(env, NULL, peer, "preferredSize", 
					     "(I)Ljava/awt/Dimension;",
					     1).l;
					      
    DASSERT(!safe_ExceptionOccurred(env));
    if (dimension == NULL) {
	return NULL;
    }
    /* This size is too big for each item height. */
    (env)->SetIntField(dimension, AwtDimension::heightID, GetFontHeight(env));

    return dimension;
}

// Every time something gets added to the list, we increase the max width 
// of items that have ever been added.  If it surpasses the width of the 
// listbox, we show the scrollbar.  When things get deleted, we shrink 
// the scroll region back down and hide the scrollbar, if needed.
void AwtList::AdjustHorizontalScrollbar()
{
    // The border width is added to the horizontal extent to ensure that we
    // can view all of the text when we move the horz. scrollbar to the end.
    int  cxBorders = GetSystemMetrics( SM_CXBORDER ) * 2;
    RECT rect;
    VERIFY(::GetClientRect(GetListHandle(), &rect));
    LRESULT iHorzExt = SendListMessage(LB_GETHORIZONTALEXTENT, 0, 0L ) - cxBorders;
    if ( (m_nMaxWidth > rect.left)  // if strings wider than listbox
      || (iHorzExt != m_nMaxWidth) ) //   or scrollbar not needed anymore.
    {
        SendListMessage(LB_SETHORIZONTALEXTENT, m_nMaxWidth + cxBorders, 0L);
    }
}

// This function goes through all strings in the list to find the width, 
// in pixels, of the longest string in the list.
void AwtList::UpdateMaxItemWidth()
{
    m_nMaxWidth = 0;

    JNIEnv *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);
    if (env->EnsureLocalCapacity(2) < 0)
        return;

    HDC hDC = ::GetDC(GetHWnd());

    jobject self = GetPeer(env);
    DASSERT(self);

    /* target is java.awt.List */
    jobject target = env->GetObjectField(self, AwtObject::targetID);
    jobject font = GET_FONT(target, self);

    int nCount = GetCount();
    for ( int i=0; i < nCount; i++ )
    {
        jstring jstr = GetItemString( env, target, i );
        SIZE size = AwtFont::getMFStringSize( hDC, font, jstr );
        if ( size.cx > m_nMaxWidth )
            m_nMaxWidth = size.cx;
        env->DeleteLocalRef( jstr );
    }

    // free up the shared DC and release local refs
    ::ReleaseDC(GetHWnd(), hDC);
    env->DeleteLocalRef( target );
    env->DeleteLocalRef( font );

    // Now adjust the horizontal scrollbar extent
    AdjustHorizontalScrollbar();
}

MsgRouting
AwtList::WmSize(UINT type, int w, int h)
{
    AdjustHorizontalScrollbar();
    return AwtComponent::WmSize(type, w, h);
}

MsgRouting
AwtList::OwnerDrawItem(UINT /*ctrlId*/, DRAWITEMSTRUCT& drawInfo)
{
    AwtComponent::DrawListItem((JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2), drawInfo);
    return mrConsume;
}

MsgRouting 
AwtList::OwnerMeasureItem(UINT /*ctrlId*/, MEASUREITEMSTRUCT& measureInfo)
{
    AwtComponent::MeasureListItem((JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2), measureInfo);
    return mrConsume;
}

MsgRouting 
AwtList::WmMouseUp(UINT flags, int x, int y, int button)
{
    if (button == LEFT_BUTTON) {
        WmCommand(0, GetWrappeeHandle(), LBN_SELCHANGE);
    }
    return AwtComponent::WmMouseUp(flags, x, y, button);
}

MsgRouting 
AwtList::WmCtlColor(HDC hDC, HWND hCtrl, UINT ctlColor, HBRUSH& retBrush)
{
    DASSERT(ctlColor == CTLCOLOR_LISTBOX);
    DASSERT(hCtrl == GetListHandle());
    ::SetBkColor(hDC, GetBackgroundColor());
    ::SetTextColor(hDC, GetColor());
    retBrush = GetBackgroundBrush();
    return mrConsume;
}

// Override WmSetFocus and WmKillFocus so that they operate on the List handle
// instead of the wrapper handle. Otherwise, the methods are the same as their
// AwtComponent counterparts.

MsgRouting AwtList::WmSetFocus(HWND hWndLostFocus) {
    if (sm_focusOwner == GetListHandle()) {
        sm_realFocusOpposite = NULL;
        return mrConsume;
    }

    sm_focusOwner = GetListHandle();

    if (sm_realFocusOpposite != NULL) {
        hWndLostFocus = sm_realFocusOpposite;
        sm_realFocusOpposite = NULL;
    }

    SendFocusEvent(java_awt_event_FocusEvent_FOCUS_GAINED, hWndLostFocus);

    return mrDoDefault;
}

MsgRouting AwtList::WmKillFocus(HWND hWndGotFocus) {
    if (sm_focusOwner != NULL && sm_focusOwner == hWndGotFocus) {
        return mrConsume;
    }

    if (sm_focusOwner != GetListHandle()) {
        if (sm_focusOwner != NULL) {
            if (hWndGotFocus != NULL &&
                AwtComponent::GetComponent(hWndGotFocus) != NULL)
		{
		    sm_realFocusOpposite = sm_focusOwner;
		}
            ::SendMessage(sm_focusOwner, WM_KILLFOCUS, (WPARAM)hWndGotFocus,
                          0);
        }
        return mrConsume;
    }

    sm_focusOwner = NULL;

    SendFocusEvent(java_awt_event_FocusEvent_FOCUS_LOST, hWndGotFocus);
    return mrDoDefault;
}

MsgRouting AwtList::HandleEvent(MSG *msg, BOOL synthetic)
{
    if (AwtComponent::sm_focusOwner != GetListHandle() &&
        (msg->message == WM_LBUTTONDOWN || msg->message == WM_LBUTTONDBLCLK))
    {
        JNIEnv *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);
        jobject target = GetTarget(env);
        env->CallStaticVoidMethod
            (AwtKeyboardFocusManager::keyboardFocusManagerCls,
             AwtKeyboardFocusManager::heavyweightButtonDownMID,
             target, ((jlong)msg->time) & 0xFFFFFFFF);
        env->DeleteLocalRef(target);
    }
    return AwtComponent::HandleEvent(msg, synthetic);
}

// Fix for 4665745.
// Override WmPrint to catch when the list control (not wrapper) should
// operate WM_PRINT to be compatible with the "smooth scrolling" feature.
MsgRouting AwtList::WmPrint(HDC hDC, LPARAM flags)
{
    if (!isWrapperPrint && IS_WIN4X 
            && (flags & PRF_CLIENT) 
            && (GetStyleEx() & WS_EX_CLIENTEDGE)) {

        int nOriginalDC = ::SaveDC(hDC);
        DASSERT(nOriginalDC != 0);
        // Save a copy of the DC for WmPrintClient
        VERIFY(::SaveDC(hDC));
        DefWindowProc(WM_PRINT, (WPARAM) hDC,
            (flags & (PRF_CLIENT | PRF_CHECKVISIBLE | PRF_ERASEBKGND)));
        VERIFY(::RestoreDC(hDC, nOriginalDC));

        flags &= ~PRF_CLIENT;
    }

    return AwtComponent::WmPrint(hDC, flags);
}

MsgRouting
AwtList::WmNotify(UINT notifyCode)
{
    if (notifyCode == LBN_SELCHANGE || notifyCode == LBN_DBLCLK) {
        /* Fixed an asserion failure when clicking on an empty List. */
        int nCurrentSelection = GetCurrentSelection();
        if (nCurrentSelection != LB_ERR) {
            if (notifyCode == LBN_SELCHANGE) {
                DoCallback("handleListChanged", "(I)V", nCurrentSelection);
            }
            else if (notifyCode == LBN_DBLCLK) {
                DoCallback("handleAction", "(IJI)V", nCurrentSelection,
                           nowMillisUTC(),
                           (jint)AwtComponent::GetJavaModifiers());
            }
        }
    }
    return mrDoDefault;
}

MsgRouting
AwtList::WmKeyDown(UINT wkey, UINT repCnt, UINT flags, BOOL system)
{
    if (wkey == VK_RETURN) {
        WmNotify(LBN_DBLCLK);
    }
    return AwtComponent::WmKeyDown(wkey, repCnt, flags, system);
}

BOOL AwtList::InheritsNativeMouseWheelBehavior() {return true;}

/************************************************************************
 * WListPeer native methods
 *
 * This class seems to have numerous bugs in it, but they are all bugs
 * which were present before conversion to JNI. -br.  
 */

extern "C" {

/*
 * Class:     sun_awt_windows_WListPeer
 * Method:    getMaxWidth
 * Signature: ()I
 */
JNIEXPORT jint JNICALL
Java_sun_awt_windows_WListPeer_getMaxWidth(JNIEnv *env, jobject self)
{
    TRY;

    PDATA pData;
    JNI_CHECK_PEER_RETURN_NULL(self);

    AwtList* l = (AwtList*)pData;
    return l->GetMaxWidth();

    CATCH_BAD_ALLOC_RET(0);
}

/*
 * Class:     sun_awt_windows_WListPeer
 * Method:    updateMaxItemWidth
 * Signature: ()V
 */
JNIEXPORT void JNICALL
Java_sun_awt_windows_WListPeer_updateMaxItemWidth(JNIEnv *env, jobject self)
{
    TRY;

    PDATA pData;
    JNI_CHECK_PEER_RETURN(self);

    AwtList* l = (AwtList*)pData;
    l->UpdateMaxItemWidth();

    CATCH_BAD_ALLOC;
}

/*
 * Class:     sun_awt_windows_WListPeer
 * Method:    addItems
 * Signature: ([Ljava/lang/String;II)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_windows_WListPeer_addItems(JNIEnv *env, jobject self,
				        jobjectArray items, jint index, jint width)
{
    TRY;

    PDATA pData;
    JNI_CHECK_PEER_RETURN(self);
    JNI_CHECK_NULL_RETURN(items, "null items");

    int itemCount = env->GetArrayLength(items);
    if (itemCount > 0) 
    {
        AwtList* l = (AwtList*)pData;
        l->SendListMessage(WM_SETREDRAW, (WPARAM)FALSE, 0);
        for (jsize i=0; i < itemCount; i++)
        {
            jstring item = (jstring)env->GetObjectArrayElement(items, i);
            LPTSTR itemPtr = (LPTSTR)JNU_GetStringPlatformChars(env, item, 0);

            if (itemPtr == NULL) {
                throw std::bad_alloc();
            }

            l->InsertString(index+i, itemPtr);
            JNU_ReleaseStringPlatformChars(env, item, itemPtr);
            env->DeleteLocalRef(item);
        }
        l->SendListMessage(WM_SETREDRAW, (WPARAM)TRUE, 0);
        l->InvalidateList(NULL, TRUE);
        l->CheckMaxWidth(width);
    }

    CATCH_BAD_ALLOC;
}

/*
 * Class:     sun_awt_windows_WListPeer
 * Method:    delItems
 * Signature: (II)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_windows_WListPeer_delItems(JNIEnv *env, jobject self,
					jint start, jint end)
{
    TRY;

    PDATA pData;
    JNI_CHECK_PEER_RETURN(self);

    AwtList* l = (AwtList*)pData;
    int count = l->GetCount();

    if (start == 0 && end == count) {
        l->SendListMessage(LB_RESETCONTENT);
    }
    else for (int i = start; i <= end; i++) {
        l->SendListMessage(LB_DELETESTRING, start);
    }

    l->UpdateMaxItemWidth();

    CATCH_BAD_ALLOC;
}

/*
 * Class:     sun_awt_windows_WListPeer
 * Method:    select
 * Signature: (I)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_windows_WListPeer_select(JNIEnv *env, jobject self,
				      jint pos)
{
    TRY;

    PDATA pData;
    JNI_CHECK_PEER_RETURN(self);
    AwtList* l = (AwtList*)pData;
    l->Select(pos);

    CATCH_BAD_ALLOC;
}

/*
 * Class:     sun_awt_windows_WListPeer
 * Method:    deselect
 * Signature: (I)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_windows_WListPeer_deselect(JNIEnv *env, jobject self,
					jint pos)
{
    TRY;

    PDATA pData;
    JNI_CHECK_PEER_RETURN(self);
    AwtList* l = (AwtList*)pData;
    l->Deselect(pos);

    CATCH_BAD_ALLOC;
}

/*
 * Class:     sun_awt_windows_WListPeer
 * Method:    makeVisible
 * Signature: (I)V
 */
JNIEXPORT void JNICALL 
Java_sun_awt_windows_WListPeer_makeVisible(JNIEnv *env, jobject self,
					   jint pos)
{
    TRY;

    PDATA pData;
    JNI_CHECK_PEER_RETURN(self);
    AwtList* l = (AwtList*)pData;
    l->SendListMessage(LB_SETTOPINDEX, pos);

    CATCH_BAD_ALLOC;
}

/*
 * Class:     sun_awt_windows_WListPeer
 * Method:    setMultipleSelections
 * Signature: (Z)V
 */
JNIEXPORT void JNICALL 
Java_sun_awt_windows_WListPeer_setMultipleSelections(JNIEnv *env, jobject self,
						     jboolean on)
{
    TRY;

    PDATA pData;
    JNI_CHECK_PEER_RETURN(self);
    AwtList* l = (AwtList*)pData;
    AwtToolkit::GetInstance().SendMessage(WM_AWT_LIST_SETMULTISELECT,
					  (WPARAM) l, on);

    CATCH_BAD_ALLOC;
}

/*
 * Class:     sun_awt_windows_WListPeer
 * Method:    create
 * Signature: (Lsun/awt/windows/WComponentPeer;)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_windows_WListPeer_create(JNIEnv *env, jobject self,
				      jobject parent)
{
    TRY;

    PDATA pData;
    JNI_CHECK_PEER_RETURN(parent);
    AwtToolkit::CreateComponent(self, parent, 
				(AwtToolkit::ComponentFactory)AwtList::Create);
    JNI_CHECK_PEER_CREATION_RETURN(self);

    CATCH_BAD_ALLOC;
}

/*
 * Class:     sun_awt_windows_WListPeer
 * Method:    isSelected
 * Signature: (I)Z
 */
JNIEXPORT jboolean JNICALL
Java_sun_awt_windows_WListPeer_isSelected(JNIEnv *env, jobject self,
					  jint index)
{
    TRY;

    PDATA pData;
    JNI_CHECK_PEER_RETURN_NULL(self);
    AwtList* l = (AwtList*)pData;
    return l->IsItemSelected(index);

    CATCH_BAD_ALLOC_RET(FALSE);
}

} /* extern "C" */
