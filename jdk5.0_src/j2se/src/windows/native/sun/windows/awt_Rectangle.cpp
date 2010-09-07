/*
 * @(#)awt_Rectangle.cpp	1.9 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "awt_Rectangle.h"
#include "awt.h"

/************************************************************************
 * AwtRectangle fields
 */

jfieldID AwtRectangle::xID;
jfieldID AwtRectangle::yID;
jfieldID AwtRectangle::widthID;
jfieldID AwtRectangle::heightID;

/************************************************************************
 * AwtRectangle native methods
 */

extern "C" {

JNIEXPORT void JNICALL 
Java_java_awt_Rectangle_initIDs(JNIEnv *env, jclass cls) {
    TRY;

    AwtRectangle::xID = env->GetFieldID(cls, "x", "I");
    AwtRectangle::yID = env->GetFieldID(cls, "y", "I");
    AwtRectangle::widthID = env->GetFieldID(cls, "width", "I");
    AwtRectangle::heightID = env->GetFieldID(cls, "height", "I");

    DASSERT(AwtRectangle::xID != NULL);
    DASSERT(AwtRectangle::yID != NULL);
    DASSERT(AwtRectangle::widthID != NULL);
    DASSERT(AwtRectangle::heightID != NULL);

    CATCH_BAD_ALLOC;
}

} /* extern "C" */
