/*
 * @(#)awt_Canvas.cpp	1.90 10/03/23
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "awt_Toolkit.h"
#include "awt_Canvas.h"
#include "awt_Window.h"
#include "awt_Win32GraphicsConfig.h"
#include "awt_KeyboardFocusManager.h"

/* IMPORTANT! Read the README.JNI file for notes on JNI converted AWT code.
 */

// Struct for _SetEraseBackground() method
struct SetEraseBackgroundStruct {
    jobject canvas;
    jboolean doErase;
    jboolean doEraseOnResize;
};

/************************************************************************
 * AwtCanvas methods
 */

AwtCanvas::AwtCanvas() {
    m_eraseBackground = JNI_TRUE;
    m_eraseBackgroundOnResize = JNI_TRUE;
}

AwtCanvas::~AwtCanvas() {
}

LPCTSTR AwtCanvas::GetClassName() {
    return TEXT("SunAwtCanvas");
}

/*
 * Create a new AwtCanvas object and window.
 */
AwtCanvas* AwtCanvas::Create(jobject self, jobject hParent)
{
    TRY;
    JNIEnv *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);

    jobject target = NULL;
    AwtCanvas *canvas = NULL;

    try {
	if (env->EnsureLocalCapacity(1) < 0) {
	    return NULL;
	}

	AwtComponent* parent;

	JNI_CHECK_NULL_GOTO(hParent, "null hParent", done);

	parent = (AwtComponent*)JNI_GET_PDATA(hParent);
	JNI_CHECK_NULL_GOTO(parent, "null parent", done);

	target = env->GetObjectField(self, AwtObject::targetID);
	JNI_CHECK_NULL_GOTO(target, "null target", done);

	canvas = new AwtCanvas();

	{
	    jint x = env->GetIntField(target, AwtComponent::xID);
	    jint y = env->GetIntField(target, AwtComponent::yID);
	    jint width = env->GetIntField(target, AwtComponent::widthID);
	    jint height = env->GetIntField(target, AwtComponent::heightID);

	    canvas->CreateHWnd(env, L"",
			       WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 0,
			       x, y, width, height,
			       parent->GetHWnd(),
			       NULL,
			       ::GetSysColor(COLOR_WINDOWTEXT),
			       ::GetSysColor(COLOR_WINDOW),
			       self);

        // Set the pixel format of the HWND if a GraphicsConfiguration
        // was provided to the Canvas constructor.

        jclass canvasClass = env->FindClass("java/awt/Canvas");
        if ( env->IsInstanceOf( target, canvasClass ) ) {

            // Get GraphicsConfig from our target
            jobject graphicsConfig = env->GetObjectField(target,
                AwtComponent::graphicsConfigID);
            if (graphicsConfig != NULL) {

                jclass win32cls = env->FindClass("sun/awt/Win32GraphicsConfig");
                DASSERT (win32cls != NULL);

                if ( env->IsInstanceOf( graphicsConfig, win32cls ) ) {
                    // Get the visual ID member from our GC
                    jint visual = env->GetIntField(graphicsConfig,
                          AwtWin32GraphicsConfig::win32GCVisualID);
                    if (visual > 0) {
                        HDC hdc = ::GetDC(canvas->m_hwnd);
                        // Set our pixel format
                        PIXELFORMATDESCRIPTOR pfd;
                        BOOL ret = ::SetPixelFormat(hdc, (int)visual, &pfd);
                        ::ReleaseDC(canvas->m_hwnd, hdc);
                        //Since a GraphicsConfiguration was specified, we should
                        //throw an exception if the PixelFormat couldn't be set.
                        if (ret == FALSE) {
                            DASSERT(!safe_ExceptionOccurred(env)); 
                            jclass excCls = env->FindClass(
                             "java/lang/RuntimeException");
                            DASSERT(excCls);
                            env->ExceptionClear();
                            env->ThrowNew(excCls,
                             "\nUnable to set Pixel format on Canvas");
                            env->DeleteLocalRef(target);
                            return canvas;
                        }
                    }
                }
            }
        }
    }
    } catch (...) {
        env->DeleteLocalRef(target);
	throw;
    }

done:
    env->DeleteLocalRef(target);
    return canvas;
    CATCH_BAD_ALLOC_RET(0);
}

MsgRouting AwtCanvas::WmEraseBkgnd(HDC hDC, BOOL& didErase)
{
    extern BOOL windowSizing;
    if (m_eraseBackground || 
        (m_eraseBackgroundOnResize && windowSizing)) 
    {
       RECT     rc;
       ::GetClipBox(hDC, &rc);
       ::FillRect(hDC, &rc, this->GetBackgroundBrush());
    }

    didErase = TRUE;
    return mrConsume;
}

/*
 * This routine is duplicated in AwtWindow.
 */
MsgRouting AwtCanvas::WmPaint(HDC)
{
    PaintUpdateRgn(NULL);
    return mrConsume;
}

MsgRouting AwtCanvas::HandleEvent(MSG *msg, BOOL synthetic)
{
    if (msg->message == WM_LBUTTONDOWN || msg->message == WM_LBUTTONDBLCLK) {
	/*
	 * Fix for BugTraq ID 4041703: keyDown not being invoked.
	 * Give the focus to a Canvas or Panel if it doesn't have heavyweight 
	 * subcomponents so that they will behave the same way as on Solaris 
	 * providing a possibility of giving keyboard focus to an empty Applet. 
	 * Since ScrollPane doesn't receive focus on mouse press on Solaris, 
	 * HandleEvent() is overriden there to do nothing with focus.
	 */
	if (AwtComponent::sm_focusOwner != GetHWnd() &&
            ::GetWindow(GetHWnd(), GW_CHILD) == NULL)
        {
            HeavyweightButtonDown((jlong)msg->time);
            AwtSetFocus();
	}
    }
    return AwtComponent::HandleEvent(msg, synthetic);
}

void AwtCanvas::_SetEraseBackground(void *param)
{
    JNIEnv *env = (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);

    SetEraseBackgroundStruct *sebs = (SetEraseBackgroundStruct *)param;
    jobject canvas = sebs->canvas;
    jboolean doErase = sebs->doErase;
    jboolean doEraseOnResize = sebs->doEraseOnResize;

    PDATA pData;
    JNI_CHECK_PEER_GOTO(canvas, ret);

    AwtCanvas *c = (AwtCanvas*)pData;
    c->m_eraseBackground = doErase;
    c->m_eraseBackgroundOnResize = doEraseOnResize;

ret:
    env->DeleteGlobalRef(canvas);
    delete sebs;
}


/************************************************************************
 * WCanvasPeer native methods
 */

extern "C" {

JNIEXPORT void JNICALL
Java_sun_awt_windows_WCanvasPeer_create(JNIEnv *env, jobject self,
					jobject parent)
{
    TRY;

    PDATA pData;
    JNI_CHECK_PEER_RETURN(parent);
    AwtToolkit::CreateComponent(self, parent,
				(AwtToolkit::ComponentFactory)
				AwtCanvas::Create);
    JNI_CHECK_PEER_CREATION_RETURN(self);

    CATCH_BAD_ALLOC;
}

/*
 * Class:     sun_awt_windows_WCanvasPeer
 * Method:    setNativeBackgroundErase
 * Signature: (Z)V
 */
 JNIEXPORT void JNICALL
 Java_sun_awt_windows_WCanvasPeer_setNativeBackgroundErase(JNIEnv *env,
                                                           jobject self,
                                                           jboolean doErase,
                                                           jboolean doEraseOnResize)
{
    TRY;

    SetEraseBackgroundStruct *sebs = new SetEraseBackgroundStruct;
    sebs->canvas = env->NewGlobalRef(self);
    sebs->doErase = doErase;
    sebs->doEraseOnResize = doEraseOnResize;

    AwtToolkit::GetInstance().SyncCall(AwtCanvas::_SetEraseBackground, sebs);
    // sebs and global ref are deleted in _SetEraseBackground()

    CATCH_BAD_ALLOC;
}

} /* extern "C" */
