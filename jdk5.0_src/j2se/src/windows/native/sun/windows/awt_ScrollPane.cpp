/*
 * @(#)awt_ScrollPane.cpp	1.53 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "awt_ScrollPane.h"

#include "awt_Container.h"
#include "awt_Insets.h"
#include "awt_Panel.h"
#include "awt_Scrollbar.h"   // static #defines
#include "awt_Toolkit.h"

#include <java_awt_Adjustable.h>
#include <java_awt_ScrollPane.h>
#include <java_awt_ScrollPaneAdjustable.h>
#include <java_awt_event_AdjustmentEvent.h>


/* IMPORTANT! Read the README.JNI file for notes on JNI converted AWT code.
 */

/************************************************************************
 * AwtScrollPane fields
 */

jfieldID AwtScrollPane::scrollbarDisplayPolicyID;
jfieldID AwtScrollPane::hAdjustableID;
jfieldID AwtScrollPane::vAdjustableID;
jfieldID AwtScrollPane::unitIncrementID;
jfieldID AwtScrollPane::blockIncrementID;
jmethodID AwtScrollPane::postScrollEventID;

/************************************************************************
 * AwtScrollPane methods
 */

AwtScrollPane::AwtScrollPane() {
}

LPCTSTR AwtScrollPane::GetClassName() {
    return TEXT("SunAwtScrollPane");
}

/* Create a new AwtScrollPane object and window.   */
AwtScrollPane* AwtScrollPane::Create(jobject self, jobject parent)
{
    JNIEnv *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);
    jobject target = NULL;
    AwtScrollPane* c = NULL;

    try {
        if (env->EnsureLocalCapacity(1) < 0) {
	    return NULL;
	}

	PDATA pData;
	AwtComponent* awtParent;
	JNI_CHECK_PEER_GOTO(parent, done);

	awtParent = (AwtComponent*)pData;
	JNI_CHECK_NULL_GOTO(awtParent, "null awtParent", done);

	target = env->GetObjectField(self, AwtObject::targetID);
	JNI_CHECK_NULL_GOTO(target, "null target", done);

	c = new AwtScrollPane();

	{
	    DWORD style = WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
	    if (!IS_WIN4X) {
	        /* 
		 * It's been decided by the UI folks that 3.X ScrollPanes
		 * should have borders...  
		 */
	        style |= WS_BORDER;
	    }
	    jint scrollbarDisplayPolicy = 
	        env->GetIntField(target, scrollbarDisplayPolicyID);

	    if (scrollbarDisplayPolicy
		    == java_awt_ScrollPane_SCROLLBARS_ALWAYS) {
	        style |= WS_HSCROLL | WS_VSCROLL;
	    }
	    DWORD exStyle = IS_WIN4X ? WS_EX_CLIENTEDGE : 0;

	    if (GetRTL()) {
	        exStyle |= WS_EX_RIGHT | WS_EX_LEFTSCROLLBAR;
		if (GetRTLReadingOrder())
		    exStyle |= WS_EX_RTLREADING;
	    }

	    jint x = env->GetIntField(target, AwtComponent::xID);
	    jint y = env->GetIntField(target, AwtComponent::yID);
	    jint width = env->GetIntField(target, AwtComponent::widthID);
	    jint height = env->GetIntField(target, AwtComponent::heightID);
	    c->CreateHWnd(env, L"", style, exStyle,
			  x, y, width, height,
			  awtParent->GetHWnd(),
			  reinterpret_cast<HMENU>(static_cast<INT_PTR>(
                awtParent->CreateControlID())),
			  ::GetSysColor(COLOR_WINDOWTEXT),
			  ::GetSysColor(COLOR_WINDOW),
			  self);
	}
    } catch (...) {
        env->DeleteLocalRef(target);
	throw;
    }

done:
    env->DeleteLocalRef(target);
    return c;
}

void AwtScrollPane::SetInsets(JNIEnv *env)
{
    RECT outside;
    RECT inside;
    ::GetWindowRect(GetHWnd(), &outside);
    ::GetClientRect(GetHWnd(), &inside);
    ::MapWindowPoints(GetHWnd(), 0, (LPPOINT)&inside, 2);
    
    if (env->EnsureLocalCapacity(1) < 0) {
	return;
    }
    jobject insets =
      (env)->GetObjectField(GetPeer(env), AwtPanel::insets_ID);

    DASSERT(!safe_ExceptionOccurred(env));

    if (insets != NULL && (inside.top-outside.top) != 0) {
        (env)->SetIntField(insets, AwtInsets::topID, inside.top - outside.top);
        (env)->SetIntField(insets, AwtInsets::leftID, inside.left - outside.left);
        (env)->SetIntField(insets, AwtInsets::bottomID, outside.bottom - inside.bottom);
        (env)->SetIntField(insets, AwtInsets::rightID, outside.right - inside.right);
    }

    env->DeleteLocalRef(insets);
}

void AwtScrollPane::SetScrollInfo(int orient, int max, int page, 
                                  BOOL disableNoScroll)
{
    DTRACE_PRINTLN4("AwtScrollPane::SetScrollInfo %d, %d, %d, %d", orient, max, page, disableNoScroll);
    SCROLLINFO si;
    int posBefore;
    int posAfter;
    
    posBefore = GetScrollPos(orient);
    si.cbSize = sizeof(SCROLLINFO);
    si.nMin = 0;
    si.nMax = max;
    si.fMask = SIF_RANGE;
    if (disableNoScroll) {
        si.fMask |= SIF_DISABLENOSCROLL;
    }
    if (page > 0) {
        si.fMask |= SIF_PAGE;
        si.nPage = page;
    }
    ::SetScrollInfo(GetHWnd(), orient, &si, TRUE);
    // scroll position may have changed when thumb is at the end of the bar
    // and the page size changes
    posAfter = GetScrollPos(orient);
    if (posBefore != posAfter) {
	PostScrollEvent(orient, SB_THUMBPOSITION, posAfter);
    }
}

void AwtScrollPane::RecalcSizes(int parentWidth, int parentHeight,
				int childWidth, int childHeight)
{
    JNIEnv *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);
    if (env->EnsureLocalCapacity(2) < 0) {
	return;
    }

    /* Determine border width without scrollbars. */
    int horzBorder;
    int vertBorder;
    if (IS_WIN4X) {
	horzBorder = ::GetSystemMetrics(SM_CXEDGE);
	vertBorder = ::GetSystemMetrics(SM_CYEDGE);
    } else {
        horzBorder = ::GetSystemMetrics(SM_CXBORDER);
        vertBorder = ::GetSystemMetrics(SM_CYBORDER);
    }
    
    parentWidth -= (horzBorder * 2);
    parentHeight -= (vertBorder * 2);

    /* Enable each scrollbar as needed. */
    jobject target = AwtObject::GetTarget(env);
    jint policy = env->GetIntField(target, 
				   AwtScrollPane::scrollbarDisplayPolicyID);

    BOOL needsHorz=(policy == java_awt_ScrollPane_SCROLLBARS_ALWAYS ||
		    (policy == java_awt_ScrollPane_SCROLLBARS_AS_NEEDED && 
		     childWidth > parentWidth));
    if (needsHorz) {
	parentHeight -= ::GetSystemMetrics(SM_CYHSCROLL);
    }
    BOOL needsVert=(policy == java_awt_ScrollPane_SCROLLBARS_ALWAYS ||
		    (policy ==java_awt_ScrollPane_SCROLLBARS_AS_NEEDED && 
		     childHeight > parentHeight));
    if (needsVert) {
	parentWidth -= ::GetSystemMetrics(SM_CXVSCROLL);
    }
    /*
     * Since the vertical scrollbar may have reduced the parent width
     * enough to now require a horizontal scrollbar, we need to
     * recalculate the horizontal metrics and scrollbar boolean.
     */
    if (!needsHorz) {
	needsHorz = (policy == java_awt_ScrollPane_SCROLLBARS_ALWAYS ||
		     (policy == java_awt_ScrollPane_SCROLLBARS_AS_NEEDED && 
		      childWidth > parentWidth));
	if (needsHorz) {
	    parentHeight -= ::GetSystemMetrics(SM_CYHSCROLL);
	}
    }

    /* Now set ranges -- setting the min and max the same disables them. */
    if (needsHorz) {
	jobject hAdj = 
	    env->GetObjectField(target, AwtScrollPane::hAdjustableID);
	env->SetIntField(hAdj, AwtScrollPane::blockIncrementID, parentWidth);
        SetScrollInfo(SB_HORZ, childWidth - 1, parentWidth, 
                      (policy == java_awt_ScrollPane_SCROLLBARS_ALWAYS));
	env->DeleteLocalRef(hAdj);
    } else {
        SetScrollInfo(SB_HORZ, 0, 0, 
                      (policy == java_awt_ScrollPane_SCROLLBARS_ALWAYS));
    }

    if (needsVert) {
	jobject vAdj = 
	    env->GetObjectField(target, AwtScrollPane::vAdjustableID);
	env->SetIntField(vAdj, AwtScrollPane::blockIncrementID, parentHeight);
        SetScrollInfo(SB_VERT, childHeight - 1, parentHeight, 
                      (policy == java_awt_ScrollPane_SCROLLBARS_ALWAYS));
	env->DeleteLocalRef(vAdj);
    } else {
        SetScrollInfo(SB_VERT, 0, 0, 
                      (policy == java_awt_ScrollPane_SCROLLBARS_ALWAYS));
    }

    env->DeleteLocalRef(target);
}

void AwtScrollPane::Reshape(int x, int y, int w, int h)
{
    AwtComponent::Reshape(x, y, w, h);
}

void AwtScrollPane::Show(JNIEnv *env)
{
    SetInsets(env);
    SendMessage(WM_AWT_COMPONENT_SHOW);
}

void AwtScrollPane::PostScrollEvent(int orient, int scrollCode, int pos) {
    if (scrollCode == SB_ENDSCROLL) {
	return;
    }

    // convert Windows scroll bar ident to peer ident
    jint jorient;
    if (orient == SB_VERT) {
	jorient = java_awt_Adjustable_VERTICAL;
    } else if (orient == SB_HORZ) {
	jorient = java_awt_Adjustable_HORIZONTAL;
    } else {
	DASSERT(FALSE);
	return;
    }

    // convert Windows scroll code to adjustment type and isAdjusting status
    jint jscrollcode;
    jboolean jadjusting = JNI_FALSE;
    SCROLLINFO si;
    switch (scrollCode) {
      case SB_LINEUP:
	  jscrollcode = java_awt_event_AdjustmentEvent_UNIT_DECREMENT;
	  break;
      case SB_LINEDOWN:
	  jscrollcode = java_awt_event_AdjustmentEvent_UNIT_INCREMENT;
	  break;
      case SB_PAGEUP:
	  jscrollcode = java_awt_event_AdjustmentEvent_BLOCK_DECREMENT;
	  break;
      case SB_PAGEDOWN:
	  jscrollcode = java_awt_event_AdjustmentEvent_BLOCK_INCREMENT;
	  break;
      case SB_TOP:
	  jscrollcode = java_awt_event_AdjustmentEvent_TRACK;
	  ZeroMemory(&si, sizeof(si));
	  si.cbSize = sizeof(si);
	  si.fMask = SIF_RANGE;
	  ::GetScrollInfo(GetHWnd(), orient, &si);
	  pos = si.nMin;
  	  break;
      case SB_BOTTOM:
	  jscrollcode = java_awt_event_AdjustmentEvent_TRACK;
	  ZeroMemory(&si, sizeof(si));
	  si.cbSize = sizeof(si);
	  si.fMask = SIF_RANGE;
	  ::GetScrollInfo(GetHWnd(), orient, &si);
	  pos = si.nMax;
	  break;
      case SB_THUMBTRACK:
	  jscrollcode = java_awt_event_AdjustmentEvent_TRACK;
	  jadjusting = JNI_TRUE;
	  break;
      case SB_THUMBPOSITION:
	  jscrollcode = java_awt_event_AdjustmentEvent_TRACK;
	  break;
      default:
	  DASSERT(FALSE);
	  return;
    }

    JNIEnv *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);
    env->CallVoidMethod(GetPeer(env), AwtScrollPane::postScrollEventID,
			jorient, jscrollcode, (jint)pos, jadjusting);
    DASSERT(!safe_ExceptionOccurred(env));
}

BOOL AwtScrollPane::ActMouseMessage(MSG* pMsg) {
    if (!IsFocusingMessage(pMsg->message)) {
        return FALSE;
    }
    return TRUE;
}

MsgRouting AwtScrollPane::WmVScroll(UINT scrollCode, UINT pos, HWND hScrollPane) 
{
    // While user scrolls using tracker, SCROLLINFO.nPos is not changed, SCROLLINFO.nTrackPos is changed instead.
    int dragP = scrollCode == SB_THUMBPOSITION || scrollCode == SB_THUMBTRACK;
    int newPos = GetScrollPos(SB_VERT);
    if ( dragP ) {
        SCROLLINFO si;
        ZeroMemory(&si, sizeof(si));
        si.cbSize = sizeof(si);
        si.fMask = SIF_TRACKPOS;
        ::GetScrollInfo(GetHWnd(), SB_VERT, &si);
        newPos = si.nTrackPos;
    }
    PostScrollEvent(SB_VERT, scrollCode, newPos);
    return mrConsume;
}

MsgRouting AwtScrollPane::WmHScroll(UINT scrollCode, UINT pos, HWND hScrollPane) 
{
    // While user scrolls using tracker, SCROLLINFO.nPos is not changed, SCROLLINFO.nTrackPos is changed instead.
    int dragP = scrollCode == SB_THUMBPOSITION || scrollCode == SB_THUMBTRACK;
    int newPos = GetScrollPos(SB_HORZ);
    if ( dragP ) {
        SCROLLINFO si;
        ZeroMemory(&si, sizeof(si));
        si.cbSize = sizeof(si);
        si.fMask = SIF_TRACKPOS;
        ::GetScrollInfo(GetHWnd(), SB_HORZ, &si);
        newPos = si.nTrackPos;
    }
    PostScrollEvent(SB_HORZ, scrollCode, newPos);
    return mrConsume;
}

/*
 * Fix for BugTraq ID 4041703: keyDown not being invoked.
 * This method overrides AwtCanvas::HandleEvent() since we 
 * don't want ScrollPanel to receive focus on mouse press.
 */
MsgRouting AwtScrollPane::HandleEvent(MSG *msg, BOOL synthetic)
{
    return AwtComponent::HandleEvent(msg, synthetic);
}

int AwtScrollPane::GetScrollPos(int orient) 
{
    SCROLLINFO si;
    ZeroMemory(&si, sizeof(si));
    si.cbSize = sizeof(si);
    si.fMask = SIF_POS;
    ::GetScrollInfo(GetHWnd(), orient, &si);
    return si.nPos;
}
 
void AwtScrollPane::SetScrollPos(int orient, int pos) 
{
    SCROLLINFO si;
     
    ZeroMemory(&si, sizeof(si));
    si.fMask = SIF_POS;
    si.cbSize = sizeof(si);
    si.nPos = pos;
    ::SetScrollInfo(GetHWnd(), orient, &si, TRUE);
}
 

#ifdef DEBUG
void AwtScrollPane::VerifyState()
{
    JNIEnv *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);
    if (env->EnsureLocalCapacity(3) < 0) {
	return;
    }
    
    if (AwtToolkit::GetInstance().VerifyComponents() == FALSE) {
        return;
    }

    if (m_callbacksEnabled == FALSE) {
        /* Component is not fully setup yet. */
        return;
    }

    AwtComponent::VerifyState();

    jobject target = AwtObject::GetTarget(env);
    jobject child = JNU_CallMethodByName(env, NULL, GetPeer(env), 
					 "getScrollSchild",
					 "()Ljava/awt/Component;").l;

    DASSERT(!safe_ExceptionOccurred(env));

    if (child != NULL) {
	jobject childPeer = 
	    (env)->GetObjectField(child, AwtComponent::peerID);
	PDATA pData;
	JNI_CHECK_PEER_RETURN(childPeer);
	AwtComponent* awtChild = (AwtComponent *)pData;

	/* Verify child window is positioned correctly. */
	RECT rect, childRect;
	::GetClientRect(GetHWnd(), &rect);
	::MapWindowPoints(GetHWnd(), 0, (LPPOINT)&rect, 2);
	::GetWindowRect(awtChild->GetHWnd(), &childRect);
	DASSERT(childRect.left <= rect.left && childRect.top <= rect.top);

	env->DeleteLocalRef(childPeer);
    }
    env->DeleteLocalRef(target);
    env->DeleteLocalRef(child);
}
#endif

/************************************************************************
 * ScrollPane native methods
 */

extern "C" {

/*
 * Class:     java_awt_ScrollPane
 * Method:    initIDs
 * Signature: ()V
 */
JNIEXPORT void JNICALL 
Java_java_awt_ScrollPane_initIDs(JNIEnv *env, jclass cls)
{
    TRY;

    AwtScrollPane::scrollbarDisplayPolicyID = 
	env->GetFieldID(cls, "scrollbarDisplayPolicy", "I");
    AwtScrollPane::hAdjustableID =
	env->GetFieldID(cls, "hAdjustable", "Ljava/awt/ScrollPaneAdjustable;");
    AwtScrollPane::vAdjustableID = 
	env->GetFieldID(cls, "vAdjustable", "Ljava/awt/ScrollPaneAdjustable;");
    DASSERT(AwtScrollPane::scrollbarDisplayPolicyID != NULL);
    DASSERT(AwtScrollPane::hAdjustableID != NULL);
    DASSERT(AwtScrollPane::vAdjustableID != NULL); 

    CATCH_BAD_ALLOC;
}

} /* extern "C" */


/************************************************************************
 * ScrollPaneAdjustable native methods
 */

extern "C" {

/*
 * Class:     java_awt_ScrollPaneAdjustable
 * Method:    initIDs
 * Signature: ()V
 */
JNIEXPORT void JNICALL 
Java_java_awt_ScrollPaneAdjustable_initIDs(JNIEnv *env, jclass cls)
{
    TRY;

    AwtScrollPane::unitIncrementID = env->GetFieldID(cls,"unitIncrement", "I");
    AwtScrollPane::blockIncrementID = 
	env->GetFieldID(cls,"blockIncrement", "I");

    DASSERT(AwtScrollPane::unitIncrementID != NULL);
    DASSERT(AwtScrollPane::blockIncrementID != NULL);

    CATCH_BAD_ALLOC;
}

} /* extern "C" */


/************************************************************************
 * ScrollPanePeer native methods
 */

extern "C" {

JNIEXPORT void JNICALL 
Java_sun_awt_windows_WScrollPanePeer_initIDs(JNIEnv *env, jclass cls)
{
    TRY;

    AwtScrollPane::postScrollEventID =
	env->GetMethodID(cls, "postScrollEvent", "(IIIZ)V");
    DASSERT(AwtScrollPane::postScrollEventID != NULL); 

    CATCH_BAD_ALLOC;
}
    
/*
 * Class:     sun_awt_windows_WScrollPanePeer
 * Method:    create
 * Signature: (Lsun/awt/windows/WComponentPeer;)V
 */
JNIEXPORT void JNICALL 
Java_sun_awt_windows_WScrollPanePeer_create(JNIEnv *env, jobject self, 
					    jobject parent)
{
    TRY;

    DTRACE_PRINTLN2("%x: WScrollPanePeer.create(%x)", self, parent);

    PDATA pData;
    JNI_CHECK_PEER_RETURN(parent);
    AwtToolkit::CreateComponent(self, parent, 
				(AwtToolkit::ComponentFactory)
				AwtScrollPane::Create);
    JNI_CHECK_PEER_CREATION_RETURN(self);
    ((AwtScrollPane*)pData)->VerifyState();

    CATCH_BAD_ALLOC;
}

/*
 * Class:     sun_awt_windows_WScrollPanePeer
 * Method:    getOffset
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL 
Java_sun_awt_windows_WScrollPanePeer_getOffset(JNIEnv *env, jobject self,
					       jint orient)
{
    TRY;

    DTRACE_PRINTLN2("%x: WScrollPanePeer.getOffset(%d)", self, orient);

    PDATA pData;
    JNI_CHECK_PEER_RETURN_NULL(self);
    AwtScrollPane* pane = (AwtScrollPane *)pData;
    pane->VerifyState();
    int nBar = (orient == java_awt_Adjustable_HORIZONTAL) ? SB_HORZ : SB_VERT;
    return pane->GetScrollPos(nBar);

    CATCH_BAD_ALLOC_RET(0);
}

/*
 * Class:     sun_awt_windows_WScrollPanePeer
 * Method:    setInsets
 * Signature: ()V
 */
JNIEXPORT void JNICALL
Java_sun_awt_windows_WScrollPanePeer_setInsets(JNIEnv *env, jobject self)
{
    TRY

    DTRACE_PRINTLN1("%x: WScrollPanePeer.setInsets()", self);

    PDATA pData;
    JNI_CHECK_PEER_RETURN(self);
    AwtScrollPane* pane = (AwtScrollPane *)pData;
    pane->SetInsets(env);
    pane->VerifyState();

    CATCH_BAD_ALLOC;
}

/*
 * Class:     sun_awt_windows_WScrollPanePeer
 * Method:    setScrollPosition
 * Signature: (II)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_windows_WScrollPanePeer_setScrollPosition(JNIEnv *env, 
						       jobject self,
						       jint x, jint y)
{
    TRY;

    DTRACE_PRINTLN3("%x: WScrollPanePeer.setScrollPosition(%d, %d)", self, x, y);

    PDATA pData;
    JNI_CHECK_PEER_RETURN(self);
    AwtScrollPane* pane = (AwtScrollPane *)pData;
    pane->SetScrollPos(SB_HORZ, x);
    pane->SetScrollPos(SB_VERT, y);
    CATCH_BAD_ALLOC;
}

/*
 * Class:     sun_awt_windows_WScrollPanePeer
 * Method:    _getHScrollbarHeight
 * Signature: ()I
 */
JNIEXPORT jint JNICALL
Java_sun_awt_windows_WScrollPanePeer__1getHScrollbarHeight(JNIEnv *env,
							   jobject self)
{
    TRY;

    DTRACE_PRINTLN1("%x: WScrollPanePeer._getHScrollbarHeight()", self);
    return ::GetSystemMetrics(SM_CYHSCROLL);

    CATCH_BAD_ALLOC_RET(0);
}

/*
 * Class:     sun_awt_windows_WScrollPanePeer
 * Method:    _getVScrollbarWidth
 * Signature: ()I
 */
JNIEXPORT jint JNICALL
Java_sun_awt_windows_WScrollPanePeer__1getVScrollbarWidth(JNIEnv *env, 
							  jobject self)
{
    TRY;

    DTRACE_PRINTLN1("%x: WScrollPanePeer._getVScrollbarHeight()", self);
    return ::GetSystemMetrics(SM_CXVSCROLL);

    CATCH_BAD_ALLOC_RET(0);
}

/*
 * Class:     sun_awt_windows_WScrollPanePeer
 * Method:    setSpans
 * Signature: (IIII)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_windows_WScrollPanePeer_setSpans(JNIEnv *env, jobject self,
					      jint parentWidth,
					      jint parentHeight, 
					      jint childWidth,
					      jint childHeight)
{
    TRY;

    DTRACE_PRINTLN5("%x: WScrollPanePeer.setSpans(%d, %d, %d, %d)", self, 
               parentWidth, parentHeight, childWidth, childHeight);
    PDATA pData;
    JNI_CHECK_PEER_RETURN(self);
    AwtScrollPane* pane = (AwtScrollPane *)pData;
    pane->RecalcSizes(parentWidth, parentHeight, childWidth, childHeight);
    pane->VerifyState();

    CATCH_BAD_ALLOC;
}

/*
 * Class:     sun_awt_windows_WScrollPanePeer
 * Method:    setTypedValue
 * Signature: (Ljava/awt/ScrollPaneAdjustable;II)V
 */
JNIEXPORT void JNICALL 
Java_sun_awt_windows_WScrollPanePeer_setTypedValue(JNIEnv *env, jobject peer, jobject adjustable, jint value, jint type) 
{
    static jmethodID setTypedValueMID = 0;
    if (setTypedValueMID == NULL) {
        jclass clazz = env->FindClass("java/awt/ScrollPaneAdjustable");
        if (safe_ExceptionOccurred(env)) {
            env->ExceptionDescribe();
            env->ExceptionClear();
        }
        setTypedValueMID = env->GetMethodID(clazz, "setTypedValue", "(II)V");
        env->DeleteLocalRef(clazz);
                                               
        DASSERT(setTypedValueMID != NULL);
    }
    env->CallVoidMethod(adjustable, setTypedValueMID, value, type);
}

} /* extern "C" */
