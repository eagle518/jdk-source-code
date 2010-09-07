/*
 * @(#)awt_Label.cpp	1.51 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "awt_Toolkit.h"
#include "awt_Label.h"
#include "awt_Canvas.h"
#include "awt_Win32GraphicsDevice.h"

/* IMPORTANT! Read the README.JNI file for notes on JNI converted AWT code.
 */

/************************************************************************
 * AwtLabel fields
 */

jfieldID AwtLabel::textID;
jfieldID AwtLabel::alignmentID;


/************************************************************************
 * AwtLabel methods
 */

AwtLabel::AwtLabel() {
    m_needPaint = FALSE;
}

LPCTSTR AwtLabel::GetClassName() {
    return TEXT("SunAwtLabel");
}

/* Create a new AwtLabel object and window. */
AwtLabel* AwtLabel::Create(jobject labelPeer, jobject parent)
{
    JNIEnv *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);

    jobject target = NULL;
    AwtLabel* awtLabel = NULL;

    try {
        if (env->EnsureLocalCapacity(1) < 0) {
	    return NULL;
	}

	PDATA pData;
	AwtCanvas* awtParent;

	JNI_CHECK_PEER_GOTO(parent, done);
	awtParent = (AwtCanvas*)pData;
	JNI_CHECK_NULL_GOTO(awtParent, "awtParent", done);
	target  = env->GetObjectField(labelPeer, AwtObject::targetID);
	JNI_CHECK_NULL_GOTO(target, "target", done);

	awtLabel = new AwtLabel();

	{
	    DWORD style = WS_CHILD | WS_CLIPSIBLINGS;

	    DWORD exStyle = 0;
	    if (GetRTLReadingOrder())
	        exStyle |= WS_EX_RTLREADING;

	    jint x = env->GetIntField(target, AwtComponent::xID);
	    jint y = env->GetIntField(target, AwtComponent::yID);
	    jint width = env->GetIntField(target, AwtComponent::widthID);
	    jint height = env->GetIntField(target, AwtComponent::heightID);
	    awtLabel->CreateHWnd(env, L"", style, exStyle,
				 x, y, width, height,
				 awtParent->GetHWnd(),
				 NULL,
				 ::GetSysColor(COLOR_WINDOWTEXT),
				 ::GetSysColor(COLOR_BTNFACE),
				 labelPeer);
	}
    } catch (...) {
        env->DeleteLocalRef(target);
	throw;
    }

done:
    env->DeleteLocalRef(target);
    return awtLabel;
}

void AwtLabel::DoPaint(HDC hDC, RECT& r)
{
    JNIEnv *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);

    if ((r.right-r.left) > 0 && (r.bottom-r.top) > 0 &&
        m_peerObject != NULL && m_callbacksEnabled) {

        if (env->EnsureLocalCapacity(3) < 0)
	    return;
        long x,y;
        SIZE size;

	/* self is sun.awt.windows.WLabelPeer  */

	jobject self = GetPeer(env);
	DASSERT(self);

	/* target is java.awt.Label */
	jobject target = env->GetObjectField(self, AwtObject::targetID);
	jobject font = GET_FONT(target, self);
	jstring text = (jstring)env->GetObjectField(target, AwtLabel::textID);

        size = AwtFont::getMFStringSize(hDC, font, text);
        ::SetTextColor(hDC, GetColor());
        /* Redraw whole label to eliminate display noise during resizing. */
        VERIFY(::GetClientRect(GetHWnd(), &r));
        VERIFY(::FillRect (hDC, &r, GetBackgroundBrush()));
        y = (r.top + r.bottom - size.cy) / 2;

	jint alignment = env->GetIntField(target, AwtLabel::alignmentID);
        switch (alignment) {
	   case java_awt_Label_LEFT:
              x = r.left + 2;
              break;
          case java_awt_Label_CENTER:
              x = (r.left + r.right - size.cx) / 2;
              break;
          case java_awt_Label_RIGHT:
              x = r.right - 2 - size.cx;
              break;
        }
        /* draw string */
        if (isEnabled()) {
            AwtComponent::DrawWindowText(hDC, font, text, x, y);
        } else {
            AwtComponent::DrawGrayText(hDC, font, text, x, y);
        }
        DoCallback("handlePaint", "(IIII)V",
                   r.left, r.top, r.right-r.left, r.bottom-r.top);
	env->DeleteLocalRef(target);
	env->DeleteLocalRef(font);
	env->DeleteLocalRef(text);
    }
}

void AwtLabel::LazyPaint()
{
    if (m_callbacksEnabled && m_needPaint ) {
        ::InvalidateRect(GetHWnd(), NULL, TRUE);
        m_needPaint = FALSE;
    }
}

void AwtLabel::Enable(BOOL bEnable)
{
    HANDLE disabledLevel = ::GetProp(GetHWnd(), ModalDisableProp);
    BOOL bState = bEnable ? (disabledLevel == NULL) : FALSE;
    ::EnableWindow(GetHWnd(), bState);
    // Fix for Bug #4038881 Labels don't enable and disable properly
    // Fix for Bug #4096745 disable()/enable() make AWT components blink
    // This fix is moved from awt_Component.cpp for Bug #4096745
    ::InvalidateRect(GetHWnd(), NULL, FALSE);
    CriticalSection::Lock l(GetLock());
    VerifyState();
}


MsgRouting AwtLabel::WmEraseBkgnd(HDC hDC, BOOL& didErase)
{
    RECT r;

    ::GetClipBox(hDC, &r);
    ::FillRect(hDC, &r, this->GetBackgroundBrush());
    didErase = TRUE;
    return mrConsume;
}

MsgRouting AwtLabel::WmPaint(HDC)
{
    PAINTSTRUCT ps;
    HDC hDC = ::BeginPaint(GetHWnd(), &ps);/* the passed-in HDC is ignored. */
    DASSERT(hDC);
    
    /* fix for 4408606 - incorrect color palette used in 256 color mode */
    
    int screen = AwtWin32GraphicsDevice::DeviceIndexForWindow(GetHWnd());
    AwtWin32GraphicsDevice::SelectPalette(hDC, screen);
    
    RECT& r = ps.rcPaint;
    if (!m_callbacksEnabled) {
        m_needPaint = TRUE;
    } else {
        DoPaint(hDC, r);
    }
    VERIFY(::EndPaint(GetHWnd(), &ps));
    return mrConsume;
}

MsgRouting AwtLabel::WmPrintClient(HDC hDC, LPARAM)
{
    RECT r;

    // obtain valid DC from GDI stack
    ::RestoreDC(hDC, -1);

    ::GetClipBox(hDC, &r);
    DoPaint(hDC, r);
    return mrConsume;
}


/************************************************************************
 * Label native methods
 */

extern "C" {

JNIEXPORT void JNICALL
Java_java_awt_Label_initIDs(JNIEnv *env, jclass cls)
{
    TRY;

    /* init field ids */
    AwtLabel::textID = env->GetFieldID(cls, "text", "Ljava/lang/String;");
    AwtLabel::alignmentID = env->GetFieldID(cls, "alignment", "I");

    DASSERT(AwtLabel::textID != NULL);
    DASSERT(AwtLabel::alignmentID != NULL);

    CATCH_BAD_ALLOC;
}

} /* extern "C" */


/************************************************************************
 * WLabelPeer native methods
 */

extern "C" {

/*
 * Class:     sun_awt_windows_WLabelPeer
 * Method:    setText
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_windows_WLabelPeer_setText(JNIEnv *env, jobject self,
					jstring text)
{
    TRY;

    PDATA pData;
    JNI_CHECK_PEER_RETURN(self);

    AwtComponent* c = (AwtComponent*)pData;
    c->SetText(JavaStringBuffer(env, text));
    VERIFY(::InvalidateRect(c->GetHWnd(), NULL, TRUE));

    CATCH_BAD_ALLOC;
}

/*
 * Class:     sun_awt_windows_WLabelPeer
 * Method:    setAlignment
 * Signature: (I)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_windows_WLabelPeer_setAlignment(JNIEnv *env, jobject self,
					     jint alignment)
{
    TRY;

    /*
     * alignment argument of multifont label is referred to in
     * WmDrawItem method
     */
    PDATA pData;
    JNI_CHECK_PEER_RETURN(self);

    AwtComponent* c = (AwtComponent*)pData;
    VERIFY(::InvalidateRect(c->GetHWnd(), NULL, TRUE));

    CATCH_BAD_ALLOC;
}

/*
 * Class:     sun_awt_windows_WLabelPeer
 * Method:    create
 * Signature: (Lsun/awt/windows/WComponentPeer;)V
 */
JNIEXPORT void JNICALL
Java_sun_awt_windows_WLabelPeer_create(JNIEnv *env, jobject self,
				       jobject parent)
{
    TRY;

    PDATA pData;
    JNI_CHECK_PEER_RETURN(parent);
    AwtToolkit::CreateComponent(self, parent,
				(AwtToolkit::ComponentFactory)
				AwtLabel::Create);
    JNI_CHECK_PEER_CREATION_RETURN(self);

    CATCH_BAD_ALLOC;
}

/*
 * Class:     sun_awt_windows_WLabelPeer
 * Method:    lazyPaint
 * Signature: ()V
 */
JNIEXPORT void JNICALL
Java_sun_awt_windows_WLabelPeer_lazyPaint(JNIEnv *env, jobject self)
{
    TRY;

    PDATA pData;
    JNI_CHECK_PEER_RETURN(self);
    AwtLabel* p = (AwtLabel *)pData;
    p->LazyPaint();

    CATCH_BAD_ALLOC;
}

} /* export "C" */
