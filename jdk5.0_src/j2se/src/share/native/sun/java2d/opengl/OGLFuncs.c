/*
 * @(#)OGLFuncs.c	1.2 04/03/30
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "OGLFuncs.h"

OGL_EXPRESS_ALL_FUNCS(DECLARE)

OGL_DECLARE_LIB_HANDLE();

jboolean
OGLFuncs_OpenLibrary()
{
    J2dTraceLn(J2D_TRACE_INFO, "in OGLFuncs_OpenLibrary");

    OGL_OPEN_LIB();
    if (OGL_LIB_IS_UNINITIALIZED()) {
        J2dTraceLn(J2D_TRACE_ERROR, "could not open library");
        return JNI_FALSE;
    }

    return JNI_TRUE;
}

void
OGLFuncs_CloseLibrary()
{
    J2dTraceLn(J2D_TRACE_INFO, "in OGLFuncs_CloseLibrary");

    if (OGL_LIB_IS_UNINITIALIZED()) {
        J2dTraceLn(J2D_TRACE_ERROR, "library not yet initialized");
        return;
    }

    if (OGL_CLOSE_LIB()) {
        J2dTraceLn(J2D_TRACE_ERROR, "could not close library");
    }
}

jboolean
OGLFuncs_InitPlatformFuncs()
{
    J2dTraceLn(J2D_TRACE_INFO, "in OGLFuncs_InitPlatformFuncs");

    if (OGL_LIB_IS_UNINITIALIZED()) {
        J2dTraceLn(J2D_TRACE_ERROR, "library not yet initialized");
        return JNI_FALSE;
    }

    OGL_EXPRESS_PLATFORM_FUNCS(INIT_AND_CHECK)

    J2dTraceLn(J2D_TRACE_VERBOSE, "successfully loaded all platform symbols");

    return JNI_TRUE;
}

jboolean
OGLFuncs_InitBaseFuncs()
{
    J2dTraceLn(J2D_TRACE_INFO, "in OGLFuncs_InitBaseFuncs");

    if (OGL_LIB_IS_UNINITIALIZED()) {
        J2dTraceLn(J2D_TRACE_ERROR, "library not yet initialized");
        return JNI_FALSE;
    }

    OGL_EXPRESS_BASE_FUNCS(INIT_AND_CHECK)

    J2dTraceLn(J2D_TRACE_VERBOSE, "successfully loaded all base symbols");

    return JNI_TRUE;
}

jboolean
OGLFuncs_InitExtFuncs()
{
    J2dTraceLn(J2D_TRACE_INFO, "in OGLFuncs_InitExtFuncs");

    if (OGL_LIB_IS_UNINITIALIZED()) {
        J2dTraceLn(J2D_TRACE_ERROR, "library not yet initialized");
        return JNI_FALSE;
    }

    OGL_EXPRESS_EXT_FUNCS(INIT)
    OGL_EXPRESS_PLATFORM_EXT_FUNCS(INIT_AND_CHECK)

    J2dTraceLn(J2D_TRACE_VERBOSE, "successfully loaded all ext symbols");

    return JNI_TRUE;
}
