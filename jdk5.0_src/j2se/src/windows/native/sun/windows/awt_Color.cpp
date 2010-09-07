/*
 * @(#)awt_Color.cpp	1.14 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "awt.h"
#include "awt_Color.h"


/************************************************************************
 * AwtColor fields
 */

jmethodID AwtColor::getRGBMID;


/************************************************************************
 * Color native methods
 */

extern "C" {

/*
 * Class:     java_awt_Color
 * Method:    initIDs
 * Signature: ()V;
 */
JNIEXPORT void JNICALL
Java_java_awt_Color_initIDs(JNIEnv *env, jclass cls)
{
    TRY;
  
    AwtColor::getRGBMID = env->GetMethodID(cls, "getRGB", "()I");
    DASSERT(AwtColor::getRGBMID != NULL);

    CATCH_BAD_ALLOC;
}

} /* extern "C" */

/************************************************************************
 * WColor native methods
 */

extern "C" {

/*
 * Class:     sun_awt_windows_WColor
 * Method:    getDefaultColor
 * Signature: (I)Ljava/awt/Color;
 */
JNIEXPORT jobject JNICALL
Java_sun_awt_windows_WColor_getDefaultColor(JNIEnv *env, jclass cls,
					    jint index) 
{
    TRY;
  
    int iColor = 0;
    switch(index) {

    case sun_awt_windows_WColor_WINDOW_BKGND: 
        iColor = COLOR_WINDOW; 
        break;
    case sun_awt_windows_WColor_WINDOW_TEXT:
        iColor = COLOR_WINDOWTEXT; 
        break;
    case sun_awt_windows_WColor_FRAME:
        iColor = COLOR_WINDOWFRAME; 
        break;
    case sun_awt_windows_WColor_SCROLLBAR:
        iColor = COLOR_SCROLLBAR; 
        break;
    case sun_awt_windows_WColor_MENU_BKGND:
        iColor = COLOR_MENU; 
        break;
    case sun_awt_windows_WColor_MENU_TEXT:
        iColor = COLOR_MENUTEXT; 
        break;
    case sun_awt_windows_WColor_BUTTON_BKGND:
        iColor = (IS_NT) ? COLOR_BTNFACE : COLOR_3DFACE; 
        break;
    case sun_awt_windows_WColor_BUTTON_TEXT:
        iColor = COLOR_BTNTEXT; 
        break;
    case sun_awt_windows_WColor_HIGHLIGHT:
        iColor = COLOR_HIGHLIGHT; 
        break;

    default:
	return NULL;
    }
    DWORD c = ::GetSysColor(iColor);
   
    jobject wColor = JNU_NewObjectByName(env, "java/awt/Color", "(III)V",
					 GetRValue(c), GetGValue(c), 
					 GetBValue(c));

    DASSERT(!safe_ExceptionOccurred(env));
    return wColor;

    CATCH_BAD_ALLOC_RET(NULL);
}

} /* extern "C" */


