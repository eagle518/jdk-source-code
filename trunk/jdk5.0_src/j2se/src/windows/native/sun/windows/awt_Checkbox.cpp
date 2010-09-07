/*
 * @(#)awt_Checkbox.cpp	1.54 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "awt_Toolkit.h"
#include "awt_Checkbox.h"
#include "awt_KeyboardFocusManager.h"
#include "awt_Canvas.h"

/* IMPORTANT! Read the README.JNI file for notes on JNI converted AWT code.
 */

/************************************************************************
 * AwtCheckbox fields
 */

/* java.awt.Checkbox field IDs */
jfieldID AwtCheckbox::labelID;
jfieldID AwtCheckbox::groupID;
jfieldID AwtCheckbox::stateID;

const int AwtCheckbox::CHECK_SIZE = 13;

/************************************************************************
 * AwtCheckbox methods
 */

AwtCheckbox::AwtCheckbox() {

    m_fLButtonDowned = FALSE;
}

LPCTSTR AwtCheckbox::GetClassName() {
    return TEXT("BUTTON");  /* System provided checkbox class (a type of button) */
}

AwtCheckbox* AwtCheckbox::Create(jobject peer, jobject parent)
{
    JNIEnv *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);

    jstring label = NULL;
    jobject target = NULL;
    AwtCheckbox *checkbox = NULL;

    try {
        if (env->EnsureLocalCapacity(2) < 0) {
	    return NULL;
	}

	AwtComponent* awtParent;
	JNI_CHECK_NULL_GOTO(parent, "null parent", done);

	awtParent = (AwtComponent*)JNI_GET_PDATA(parent);
	JNI_CHECK_NULL_GOTO(awtParent, "null awtParent", done);

	target = env->GetObjectField(peer, AwtObject::targetID);
	JNI_CHECK_NULL_GOTO(target, "null target", done);

	checkbox = new AwtCheckbox();

	{
	    DWORD style = WS_CHILD | WS_CLIPSIBLINGS | BS_OWNERDRAW;
	    LPCWSTR defaultLabelStr = L"";
	    LPCWSTR labelStr = defaultLabelStr;
	    DWORD exStyle = 0;

	    if (GetRTL()) {
	        exStyle |= WS_EX_RIGHT;
		if (GetRTLReadingOrder())
		    exStyle |= WS_EX_RTLREADING;
	    }

	    label = (jstring)env->GetObjectField(target, AwtCheckbox::labelID);
	    if (label != NULL) {
	        labelStr = env->GetStringChars(label, 0);
	    }
	    if (labelStr != 0) {
	        jint x = env->GetIntField(target, AwtComponent::xID);
		jint y = env->GetIntField(target, AwtComponent::yID);
		jint width = env->GetIntField(target, AwtComponent::widthID);
		jint height = env->GetIntField(target, AwtComponent::heightID);
		checkbox->CreateHWnd(env, labelStr, style, exStyle,
				     x, y, width, height,
				     awtParent->GetHWnd(),
				     reinterpret_cast<HMENU>(static_cast<INT_PTR>(
                         awtParent->CreateControlID())),
				     ::GetSysColor(COLOR_WINDOWTEXT),
				     ::GetSysColor(COLOR_BTNFACE),
				     peer);

		if (labelStr != defaultLabelStr) {
		    env->ReleaseStringChars(label, labelStr);
		}
	    } else {
	        throw std::bad_alloc();
	    }
	}
    } catch (...) {
        env->DeleteLocalRef(label);
	env->DeleteLocalRef(target);
	throw;
    }

done:
    env->DeleteLocalRef(label);
    env->DeleteLocalRef(target);

    return checkbox;
}

BOOL AwtCheckbox::ActMouseMessage(MSG* pMsg) {
    if (!IsFocusingMessage(pMsg->message)) {
        return FALSE;
    }
    
    if (pMsg->message == WM_LBUTTONDOWN) {
        SendMessage(BM_SETSTATE, ~SendMessage(BM_GETSTATE, 0, 0), 0);
    }
    return TRUE;
}

MsgRouting 
AwtCheckbox::WmMouseUp(UINT flags, int x, int y, int button)
{
    MsgRouting mrResult = AwtComponent::WmMouseUp(flags, x, y, button);

    POINT p = {x, y};
    RECT rect;
    ::GetClientRect(GetHWnd(), &rect);

    if (::PtInRect(&rect, p) && button == LEFT_BUTTON) {
        WmNotify(BN_CLICKED);
    }

    return mrResult;
}

MsgRouting
AwtCheckbox::WmNotify(UINT notifyCode)
{
    if (notifyCode == BN_CLICKED) {
	BOOL fChecked = !GetState();
        DoCallback("handleAction", "(Z)V", fChecked);
    }
    return mrDoDefault;
}

BOOL AwtCheckbox::GetState()
{
    JNIEnv *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);

    if (env->EnsureLocalCapacity(2) < 0) {
	return NULL;
    }
    jobject target = GetTarget(env);
    jboolean result = JNI_FALSE;
    if (target != NULL) {
        result = env->GetBooleanField(target, AwtCheckbox::stateID);
    }

    env->DeleteLocalRef(target);

    return (BOOL)result;
}

int AwtCheckbox::GetCheckSize()
{
    /* using height of small icon for check mark size */
    return CHECK_SIZE;
}

MsgRouting
AwtCheckbox::OwnerDrawItem(UINT /*ctrlId*/, DRAWITEMSTRUCT& drawInfo)
{
    JNIEnv *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);

    if (env->EnsureLocalCapacity(4) < 0) {
	return mrConsume;
    }

    jobject self = GetPeer(env);
    jobject target = env->GetObjectField(self, AwtObject::targetID);

    HDC hDC = drawInfo.hDC;
    RECT rect = drawInfo.rcItem;
    int checkSize;
    UINT nState;
    SIZE size;

    jobject font = GET_FONT(target, self);
    jstring str = (jstring)env->GetObjectField(target, AwtCheckbox::labelID);
    size = AwtFont::getMFStringSize(hDC, font, str);

    jobject group = env->GetObjectField(target, AwtCheckbox::groupID);
    if (group != NULL)
	nState = DFCS_BUTTONRADIO;
    else
	nState = DFCS_BUTTONCHECK;

    if (GetState())
	nState |= DFCS_CHECKED;
    else
	nState &= ~DFCS_CHECKED;

    if (drawInfo.itemState & ODS_SELECTED)
	nState |= DFCS_PUSHED;

    if (drawInfo.itemAction & ODA_DRAWENTIRE) {
	VERIFY(::FillRect (hDC, &rect, GetBackgroundBrush()));
    }

    /* draw check mark */
    checkSize = GetCheckSize();
    RECT boxRect;

    boxRect.left = (GetRTL()) ? rect.right - checkSize : rect.left;
    boxRect.top = (rect.bottom - rect.top - checkSize)/2;
    boxRect.right = boxRect.left + checkSize;
    boxRect.bottom = boxRect.top + checkSize;
    ::DrawFrameControl(hDC, &boxRect, DFC_BUTTON, nState);

    /*
     * draw string
     *
     * 4 is a heuristic number
     */
    rect.left = rect.left + checkSize + checkSize/4; 
    if (drawInfo.itemAction & ODA_DRAWENTIRE) {
        BOOL bEnabled = isEnabled();

	int x = (GetRTL()) ? rect.right - (checkSize + checkSize / 4 + size.cx)
	                   : rect.left;
	int y = (rect.top + rect.bottom - size.cy) / 2;
        if (bEnabled) {
	    AwtComponent::DrawWindowText(hDC, font, str, x, y);
        } else {
            AwtComponent::DrawGrayText(hDC, font, str, x, y);
        }
    }

    /* Draw focus rect */
    RECT focusRect;
    const int margin = 2; /*  2 is a heuristic number */

    focusRect.left = (GetRTL()) ? rect.right - checkSize - checkSize / 4 -
                                      2 * margin - size.cx
                                : rect.left - margin;
    focusRect.top = (rect.top+rect.bottom-size.cy)/2;
    focusRect.right = (GetRTL()) ? rect.right - checkSize - checkSize / 4 +
                                      margin
                                 : focusRect.left + size.cx + 2 * margin;
    focusRect.bottom = focusRect.top + size.cy;
    
    /*  draw focus rect */
    if ((drawInfo.itemState & ODS_FOCUS) &&
	((drawInfo.itemAction & ODA_FOCUS)||
	 (drawInfo.itemAction &ODA_DRAWENTIRE))) {
        VERIFY(::DrawFocusRect(hDC, &focusRect));
    }
    /*  erase focus rect */
    else if (!(drawInfo.itemState & ODS_FOCUS) &&
             (drawInfo.itemAction & ODA_FOCUS)) {
        VERIFY(::DrawFocusRect(hDC, &focusRect));
    }

    /*  Notify any subclasses */
    rect = drawInfo.rcItem;
    DoCallback("handlePaint", "(IIII)V", rect.left, rect.top, 
               rect.right-rect.left, rect.bottom-rect.top);

    env->DeleteLocalRef(target);
    env->DeleteLocalRef(font);
    env->DeleteLocalRef(str);
    env->DeleteLocalRef(group);

    return mrConsume;
}

MsgRouting AwtCheckbox::WmPaint(HDC)
{
    /*  Suppress peer notification, because it's handled in WmDrawItem. */
    return mrDoDefault;
}

MsgRouting AwtCheckbox::HandleEvent(MSG *msg, BOOL synthetic)
{
    if (IsFocusable() && AwtComponent::sm_focusOwner != GetHWnd() &&
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

#ifdef DEBUG
void AwtCheckbox::VerifyState()
{
    if (AwtToolkit::GetInstance().VerifyComponents() == FALSE) {
        return;
    }

    if (m_callbacksEnabled == FALSE) {
        /*  Component is not fully setup yet. */
        return;
    }
    
    AwtComponent::VerifyState();
    JNIEnv *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);

    if (env->EnsureLocalCapacity(2) < 0) {
	return;
    }
    
    jobject target = GetTarget(env);

    /*  Check button style */
    DWORD style = ::GetWindowLong(GetHWnd(), GWL_STYLE);
    DASSERT(style & BS_OWNERDRAW);

    /*  Check label */
    int len = ::GetWindowTextLength(GetHWnd());
    LPTSTR peerStr;
    try {
        peerStr = new TCHAR[len+1];
    } catch (std::bad_alloc&) {
	env->DeleteLocalRef(target);
        throw;
    }        

    GetText(peerStr, len+1);
    jstring label = (jstring)env->GetObjectField(target, AwtCheckbox::labelID);
    DASSERT(_tcscmp(peerStr, JavaStringBuffer(env, label)) == 0);
    delete [] peerStr;

    env->DeleteLocalRef(target);
    env->DeleteLocalRef(label);
}
#endif


/************************************************************************
 * Checkbox native methods
 */

extern "C" {

/*
 * Class:     sun_awt_windows_WButtonPeer
 * Method:    initIDs
 * Signature: ()V
 */
JNIEXPORT void JNICALL
Java_java_awt_Checkbox_initIDs(JNIEnv *env, jclass cls) 
{
    TRY;

    AwtCheckbox::labelID = 
      env->GetFieldID(cls, "label", "Ljava/lang/String;");
    AwtCheckbox::groupID = 
      env->GetFieldID(cls, "group", "Ljava/awt/CheckboxGroup;");
    AwtCheckbox::stateID = 
      env->GetFieldID(cls, "state", "Z");

    DASSERT(AwtCheckbox::labelID != NULL);
    DASSERT(AwtCheckbox::groupID != NULL);
    DASSERT(AwtCheckbox::stateID != NULL);

    CATCH_BAD_ALLOC;
}
 
} /* extern "C" */


/************************************************************************
 * WCheckboxPeer native methods
 */

extern "C" {

/*
 * Class:     sun_awt_windows_WCheckboxPeer
 * Method:    getCheckMarkSize
 * Signature: ()I
 */
JNIEXPORT jint JNICALL
Java_sun_awt_windows_WCheckboxPeer_getCheckMarkSize(JNIEnv *env,
							  jclass cls)
{
    return (jint)AwtCheckbox::GetCheckSize();
}

/*
 * Class:     sun_awt_windows_WCheckboxPeer
 * Method:    setState
 * Signature: (Z)V
 */
JNIEXPORT void JNICALL 
Java_sun_awt_windows_WCheckboxPeer_setState(JNIEnv *env, jobject self,
					    jboolean state) 
{
    TRY;

    PDATA pData;
    JNI_CHECK_PEER_RETURN(self);
    AwtComponent* c = (AwtComponent*)JNI_GET_PDATA(self);
    /* 
     * when multifont and group checkbox receive setState native
     * method, it must be redraw to display correct check mark 
     */
    jobject target = env->GetObjectField(self, AwtObject::targetID);

    jobject group = env->GetObjectField(target, AwtCheckbox::groupID);
    if (group != NULL) {
	HWND hWnd = c->GetHWnd();
        RECT rect;
        VERIFY(::GetWindowRect(hWnd,&rect));
        VERIFY(::ScreenToClient(hWnd, (LPPOINT)&rect));
        VERIFY(::ScreenToClient(hWnd, ((LPPOINT)&rect)+1));
        VERIFY(::InvalidateRect(hWnd,&rect,TRUE));
        VERIFY(::UpdateWindow(hWnd));
    } else {
	c->SendMessage(BM_SETCHECK, (WPARAM)(state ? BST_CHECKED : 
					     BST_UNCHECKED));
	VERIFY(::InvalidateRect(c->GetHWnd(), NULL, FALSE));
    }
    c->VerifyState();

    CATCH_BAD_ALLOC;
}

/*
 * Class:     sun_awt_windows_WCheckboxPeer
 * Method:    setCheckboxGroup
 * Signature: (Ljava/awt/CheckboxGroup;)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_windows_WCheckboxPeer_setCheckboxGroup(JNIEnv *env, jobject self,
						    jobject group) 
{
    TRY;

    PDATA pData;
    JNI_CHECK_PEER_RETURN(self);

/* REMIND
#ifdef DEBUG
    if (group != NULL) {
        DASSERT(IsInstanceOf((HObject*)group, "java/awt/CheckboxGroup"));
    }
#endif
*/
    AwtComponent* c = (AwtComponent*)JNI_GET_PDATA(self);
    c->SendMessage(BM_SETSTYLE, (WPARAM)BS_OWNERDRAW, (LPARAM)TRUE);
    c->VerifyState();

    CATCH_BAD_ALLOC;
}

/*
 * Class:     sun_awt_windows_WCheckboxPeer
 * Method:    setLabel
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_windows_WCheckboxPeer_setLabel(JNIEnv *env, jobject self,
					    jstring label) 
{
    TRY;

    PDATA pData;
    JNI_CHECK_PEER_RETURN(self);
    AwtComponent* c = (AwtComponent*)JNI_GET_PDATA(self);

    LPCTSTR labelStr;

    // Fix for 4378378: by convention null label means empty string
    if (label == NULL) {
        labelStr = TEXT("");
    } else {
        labelStr = JNU_GetStringPlatformChars(env, label, JNI_FALSE);
    }

    if (labelStr == NULL) {
        throw std::bad_alloc();
    }
    c->SetText(labelStr);
    c->VerifyState();

    // Fix for 4378378: release StringPlatformChars only if label is not null
    if (label != NULL) {
        JNU_ReleaseStringPlatformChars(env, label, labelStr);
    }

    CATCH_BAD_ALLOC;
}

/*
 * Class:     sun_awt_windows_WCheckboxPeer
 * Method:    create
 * Signature: (Lsun/awt/windows/WComponentPeer;)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_windows_WCheckboxPeer_create(JNIEnv *env, jobject self, 
					  jobject parent) 
{
    TRY;

    PDATA pData;
    JNI_CHECK_PEER_RETURN(parent);
    AwtToolkit::CreateComponent(self, parent, 
				(AwtToolkit::ComponentFactory)
				AwtCheckbox::Create);
    JNI_CHECK_PEER_CREATION_RETURN(self);

#ifdef DEBUG
    ((AwtComponent*)JNI_GET_PDATA(self))->VerifyState();
#endif

    CATCH_BAD_ALLOC;
}

} /* extern "C" */
