/*
 * @(#)OGLFuncs_md.h	1.1 04/03/30
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef OGLFuncs_md_h_Included
#define OGLFuncs_md_h_Included

#include <windows.h>
#include "J2D_GL/wglext.h"
#include "OGLFuncMacros.h"

/**
 * Core WGL functions
 */
typedef HGLRC (GLAPIENTRY *wglCreateContextType)(HDC hdc);
typedef BOOL  (GLAPIENTRY *wglDeleteContextType)(HGLRC hglrc);
typedef BOOL  (GLAPIENTRY *wglMakeCurrentType)(HDC hdc, HGLRC hglrc);
typedef HGLRC (GLAPIENTRY *wglGetCurrentContextType)();
typedef HDC   (GLAPIENTRY *wglGetCurrentDCType)();
typedef PROC  (GLAPIENTRY *wglGetProcAddressType)(LPCSTR procName);
typedef BOOL  (GLAPIENTRY *wglShareListsType)(HGLRC hglrc1, HGLRC hglrc2);

/**
 * WGL extension function pointers
 */
typedef BOOL (GLAPIENTRY *wglChoosePixelFormatARBType)(HDC hdc, const int *pAttribIList, const FLOAT *pAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats);
typedef BOOL (GLAPIENTRY *wglGetPixelFormatAttribivARBType)(HDC, int, int, UINT, const int *, int *);
typedef HPBUFFERARB (GLAPIENTRY *wglCreatePbufferARBType)(HDC, int, int, int, const int *);
typedef HDC  (GLAPIENTRY *wglGetPbufferDCARBType)(HPBUFFERARB);
typedef int  (GLAPIENTRY *wglReleasePbufferDCARBType)(HPBUFFERARB, HDC);
typedef BOOL (GLAPIENTRY *wglDestroyPbufferARBType)(HPBUFFERARB);
typedef BOOL (GLAPIENTRY *wglQueryPbufferARBType)(HPBUFFERARB, int, int *);
typedef BOOL (GLAPIENTRY *wglBindTexImageARBType)(HPBUFFERARB, int);
typedef BOOL (GLAPIENTRY *wglReleaseTexImageARBType)(HPBUFFERARB, int);
typedef const char *(GLAPIENTRY *wglGetExtensionsStringARBType)(HDC hdc);

#define OGL_LIB_HANDLE hDllOpenGL
#define OGL_DECLARE_LIB_HANDLE() \
    static HMODULE OGL_LIB_HANDLE = 0
#define OGL_LIB_IS_UNINITIALIZED() \
    (OGL_LIB_HANDLE == 0)
#define OGL_OPEN_LIB() \
    OGL_LIB_HANDLE = LoadLibrary(L"opengl32.dll")
#define OGL_CLOSE_LIB() \
    FreeLibrary(OGL_LIB_HANDLE)
#define OGL_GET_PROC_ADDRESS(f) \
    GetProcAddress(OGL_LIB_HANDLE, #f)
#define OGL_GET_EXT_PROC_ADDRESS(f) \
    j2d_wglGetProcAddress((LPCSTR)#f)

#define OGL_EXPRESS_PLATFORM_FUNCS(action) \
    OGL_##action##_FUNC(wglCreateContext); \
    OGL_##action##_FUNC(wglDeleteContext); \
    OGL_##action##_FUNC(wglMakeCurrent); \
    OGL_##action##_FUNC(wglGetCurrentContext); \
    OGL_##action##_FUNC(wglGetCurrentDC); \
    OGL_##action##_FUNC(wglGetProcAddress); \
    OGL_##action##_FUNC(wglShareLists);

#define OGL_EXPRESS_PLATFORM_EXT_FUNCS(action) \
    OGL_##action##_EXT_FUNC(wglChoosePixelFormatARB); \
    OGL_##action##_EXT_FUNC(wglGetPixelFormatAttribivARB); \
    OGL_##action##_EXT_FUNC(wglCreatePbufferARB); \
    OGL_##action##_EXT_FUNC(wglGetPbufferDCARB); \
    OGL_##action##_EXT_FUNC(wglReleasePbufferDCARB); \
    OGL_##action##_EXT_FUNC(wglDestroyPbufferARB); \
    OGL_##action##_EXT_FUNC(wglQueryPbufferARB); \
    OGL_##action##_EXT_FUNC(wglBindTexImageARB); \
    OGL_##action##_EXT_FUNC(wglReleaseTexImageARB); \
    OGL_##action##_EXT_FUNC(wglGetExtensionsStringARB);

#endif /* OGLFuncs_md_h_Included */
