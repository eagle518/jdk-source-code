/*
 * @(#)WGLGraphicsConfig.c	1.2 04/04/14
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <stdlib.h>
#include <string.h>

#include "sun_java2d_opengl_WGLGraphicsConfig.h"

#include "jni.h"
#include "jni_util.h"
#include "jlong.h"
#include "Disposer.h"
#include "WGLGraphicsConfig.h"
#include "WGLSurfaceData.h"

extern void OGLSD_LockImpl(JNIEnv *env);
extern void OGLSD_UnlockImpl(JNIEnv *env, jint flushFlag);

/**
 * This is a globally shared context used when creating textures.  When any
 * new contexts are created, they specify this context as the "share list"
 * context, which means any texture objects created when this shared context
 * is current will be available to any other context in any other thread.
 * Note that all regular contexts that share this context must exist in the
 * same address space.
 */
OGLContext *sharedContext = NULL;
static HPBUFFERARB sharedSurface = 0;
static HDC sharedSurfaceDC = 0;
static WGLGraphicsConfigInfo *sharedConfigInfo = NULL;

static GeneralDisposeFunc WGLGC_DisposeOGLContext;

/**
 * Attempts to initialize WGL and the core OpenGL library.  For this method
 * to return JNI_TRUE, the following must be true:
 *     - opengl32.dll must be loaded successfully (via LoadLibrary)
 *     - all core WGL/OGL function symbols from opengl32.dll must be
 *       available and loaded properly
 * If any of these requirements are not met, this method will return
 * JNI_FALSE, indicating there is no hope of using WGL/OpenGL for any
 * GraphicsConfig in the environment.
 */
static jboolean
WGLGC_InitWGL()
{
    J2dTraceLn(J2D_TRACE_INFO, "in WGLGC_InitWGL");

    if (!OGLFuncs_OpenLibrary()) {
        return JNI_FALSE;
    }

    if (!OGLFuncs_InitPlatformFuncs() ||
        !OGLFuncs_InitBaseFuncs())
    {
        OGLFuncs_CloseLibrary();
        return JNI_FALSE;
    }

    return JNI_TRUE;
}

/**
 * Disposes all memory and resources allocated for the given OGLContext,
 * which was previously stored in thread-local storage and is about to be
 * destroyed.
 */
static void
WGLGC_DestroyOGLContext(JNIEnv *env, OGLContext *oglc)
{
    J2dTraceLn(J2D_TRACE_INFO, "in WGLGC_DestroyOGLContext");

    if (oglc != NULL) {
        WGLCtxInfo *ctxinfo = (WGLCtxInfo *)oglc->ctxInfo;

        // invalidate the current context
        OGLContext_InvalidateCurrentContext(env);
        j2d_wglMakeCurrent(NULL, NULL);

        if (ctxinfo != NULL) {
            j2d_wglDeleteContext(ctxinfo->context);
            free(ctxinfo);
        }

        if (oglc->xformMatrix != NULL) {
            free(oglc->xformMatrix);
        }

        if (oglc->maskTextureID != 0) {
            j2d_glDeleteTextures(1, &oglc->maskTextureID);
        }

        if (oglc->blitTextureID != 0) {
            j2d_glDeleteTextures(1, &oglc->blitTextureID);
        }

        free(oglc);
    }
}

/**
 * This is the native dispose method that gets invoked when the Java-level
 * thread associated with this context is about to go away.  This method
 * invokes the AWT_LOCK mechanism for thread safety, then destroys the
 * OGLContext kept in thread-local storage.
 */
static void
WGLGC_DisposeOGLContext(JNIEnv *env, jlong pData)
{
    OGLContext *oglc = (OGLContext *)jlong_to_ptr(pData);

    J2dTraceLn(J2D_TRACE_INFO, "in WGLGC_DisposeOGLContext");

    OGLSD_LockImpl(env);
    WGLGC_DestroyOGLContext(env, oglc);
    OGLSD_UnlockImpl(env, OGLSD_NO_FLUSH);
}

/**
 * Creates a temporary (non-visible) window that can be used for querying
 * the OpenGL capabilities of a given device.
 *
 * REMIND: should be able to create a window on a specific device...
 */
HWND
WGLGC_CreateScratchWindow(jint screennum)
{
    static jboolean firsttime = JNI_TRUE;

    J2dTraceLn(J2D_TRACE_INFO, "in WGLGC_CreateScratchWindow");

    if (firsttime) {
        WNDCLASS wc;

        // setup window class information
        ZeroMemory(&wc, sizeof(WNDCLASS));
        wc.hInstance = GetModuleHandle(NULL);
        wc.lpfnWndProc = DefWindowProc;
        wc.lpszClassName = L"Tmp";
        if (RegisterClass(&wc) == 0) {
            J2dTraceLn(J2D_TRACE_ERROR, "error registering window class");
            return 0;
        }

        firsttime = JNI_FALSE;
    }

    // create scratch window
    return CreateWindow(L"Tmp", L"Tmp", 0,
                        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                        CW_USEDEFAULT, NULL, NULL,
                        GetModuleHandle(NULL), NULL);
}

/**
 * Returns a pixel format identifier that is suitable for Java 2D's needs
 * (must have a stencil buffer, support for pbuffers, etc).  If no
 * appropriate pixel format can be found, this method returns 0.
 */
static int
WGLGC_GetPixelFormatForDC(HDC hdc)
{
    int attrs[] = {
        WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
        WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
        WGL_DRAW_TO_PBUFFER_ARB, GL_TRUE,
        WGL_BIND_TO_TEXTURE_RGB_ARB, GL_TRUE,
        WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
        WGL_STENCIL_BITS_ARB, 1,
        0
    };
    int pixfmt;
    int nfmts;

    J2dTraceLn(J2D_TRACE_INFO, "in WGLGC_GetPixelFormatForDC");

    // find pixel format
    if (!j2d_wglChoosePixelFormatARB(hdc, attrs, NULL, 1, &pixfmt, &nfmts)) {
        J2dTraceLn(J2D_TRACE_ERROR, "error choosing pixel format");
        return 0;
    }

    if (nfmts <= 0) {
        J2dTraceLn(J2D_TRACE_ERROR, "no pixel formats found");
        return 0;
    }

    J2dTraceLn1(J2D_TRACE_VERBOSE, "chose pixel format: %d", pixfmt);

    return pixfmt;
}

/**
 * Sets a "basic" pixel format for the given HDC.  This method is used only
 * for initializing a scratch window far enough such that we can load
 * GL/WGL extension function pointers using wglGetProcAddress.  (This method
 * differs from the one above in that it does not use wglChoosePixelFormatARB,
 * which is a WGL extension function, since we can't use that method without
 * first loading the extension functions under a "basic" pixel format.)
 */
static jboolean
WGLGC_SetBasicPixelFormatForDC(HDC hdc)
{
    PIXELFORMATDESCRIPTOR pfd;
    int pixfmt;

    J2dTraceLn(J2D_TRACE_INFO, "in WGLGC_SetBasicPixelFormatForDC");

    // find pixel format
    ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cStencilBits = 1;
    pixfmt = ChoosePixelFormat(hdc, &pfd);

    if (!SetPixelFormat(hdc, pixfmt, &pfd)) {
        J2dTraceLn(J2D_TRACE_ERROR, "error setting pixel format");
        return JNI_FALSE;
    }

    return JNI_TRUE;
}

/**
 * Creates a context that is compatible with the given pixel format
 * identifier.  Returns 0 if the context could not be created properly.
 */
static HGLRC
WGLGC_CreateContext(jint screennum, jint pixfmt)
{
    PIXELFORMATDESCRIPTOR pfd;
    HWND hwnd;
    HDC hdc;
    HGLRC hglrc;

    J2dTraceLn(J2D_TRACE_INFO, "in WGLGC_CreateContext");

    hwnd = WGLGC_CreateScratchWindow(screennum);
    if (hwnd == 0) {
        J2dTraceLn(J2D_TRACE_ERROR, "could not create scratch window");
        return 0;
    }

    // get the HDC for the scratch window
    hdc = GetDC(hwnd);
    if (hdc == 0) {
        J2dTraceLn(J2D_TRACE_ERROR, "could not get dc for scratch window");
        DestroyWindow(hwnd);
        return 0;
    }

    // set the pixel format for the scratch window
    if (!SetPixelFormat(hdc, pixfmt, &pfd)) {
        J2dTraceLn(J2D_TRACE_ERROR, "error setting pixel format");
        ReleaseDC(hwnd, hdc);
        DestroyWindow(hwnd);
        return 0;
    }

    // create a context based on the scratch window
    hglrc = j2d_wglCreateContext(hdc);

    // release the temporary resources
    ReleaseDC(hwnd, hdc);
    DestroyWindow(hwnd);

    return hglrc;
}

/**
 * Initializes the extension function pointers for the given device.  Note
 * that under WGL, extension functions have different entrypoints depending
 * on the device, so we must first make a context current for the given
 * device before attempting to load the function pointers via
 * wglGetProcAddress.
 *
 * REMIND: ideally the extension function pointers would not be global, but
 *         rather would be stored in a structure associated with the
 *         WGLGraphicsConfig, so that we use the correct function entrypoint
 *         depending on the destination device...
 */
static jboolean
WGLGC_InitExtFuncs(jint screennum)
{
    HWND hwnd;
    HDC hdc;
    HGLRC context;

    J2dTraceLn(J2D_TRACE_INFO, "in WGLGC_InitExtFuncs");

    // create a scratch window
    hwnd = WGLGC_CreateScratchWindow(screennum);
    if (hwnd == 0) {
        return JNI_FALSE;
    }

    // get the HDC for the scratch window
    hdc = GetDC(hwnd);
    if (hdc == 0) {
        DestroyWindow(hwnd);
        return JNI_FALSE;
    }

    // find and set a basic pixel format for the scratch window
    if (!WGLGC_SetBasicPixelFormatForDC(hdc)) {
        J2dTraceLn(J2D_TRACE_ERROR, "could not find appropriate pixfmt");
        ReleaseDC(hwnd, hdc);
        DestroyWindow(hwnd);
        return JNI_FALSE;
    }

    // create a temporary context
    context = j2d_wglCreateContext(hdc);
    if (context == 0) {
        J2dTraceLn(J2D_TRACE_ERROR, "could not create temp WGL context");
        ReleaseDC(hwnd, hdc);
        DestroyWindow(hwnd);
        return JNI_FALSE;
    }

    // make the context current so that we can load the function pointers
    // using wglGetProcAddress
    if (!j2d_wglMakeCurrent(hdc, context)) {
        J2dTraceLn(J2D_TRACE_ERROR, "could not make temp context current");
        j2d_wglDeleteContext(context);
        ReleaseDC(hwnd, hdc);
        DestroyWindow(hwnd);
        return JNI_FALSE;
    }

    if (!OGLFuncs_InitExtFuncs()) {
        J2dTraceLn(J2D_TRACE_ERROR, "could not initialize extension funcs");
        j2d_wglMakeCurrent(NULL, NULL);
        j2d_wglDeleteContext(context);
        ReleaseDC(hwnd, hdc);
        DestroyWindow(hwnd);
        return JNI_FALSE;
    }

    // destroy the temporary resources
    j2d_wglMakeCurrent(NULL, NULL);
    j2d_wglDeleteContext(context);
    ReleaseDC(hwnd, hdc);
    DestroyWindow(hwnd);

    return JNI_TRUE;
}

/**
 * Initializes a new OGLContext, which includes the native WGL context handle
 * and some other important information such as the associated pixel format.
 * If sharedctx is non-null, its texture objects will be shared with the newly
 * created HGLRC.  If useDisposer is JNI_TRUE, a new DisposerTarget
 * (with the current Java-level thread object as the referent) will be
 * created and enqueued in the Disposer, so that this OGLContext will be
 * disposed of properly when its associated thread exits.  Contexts that are
 * intended to remain for the lifetime of the app (such as the sharedContext)
 * should specify JNI_FALSE for useDisposer.
 */
OGLContext *
WGLGC_InitOGLContext(JNIEnv *env, WGLGraphicsConfigInfo *wglinfo,
                     jboolean useDisposer)
{
    OGLContext *oglc;
    WGLCtxInfo *ctxinfo;
    HGLRC context;
    static jboolean firstTime = JNI_TRUE;

    J2dTraceLn(J2D_TRACE_INFO, "in WGLGC_InitOGLContext");

    oglc = (OGLContext *)malloc(sizeof(OGLContext));
    if (oglc == NULL) {
        J2dTraceLn(J2D_TRACE_ERROR, "could not allocate memory for oglc");
        return NULL;
    }

    memset(oglc, 0, sizeof(OGLContext));

    ctxinfo = (WGLCtxInfo *)malloc(sizeof(WGLCtxInfo));
    if (ctxinfo == NULL) {
        J2dTraceLn(J2D_TRACE_ERROR, "could not allocate memory for ctxinfo");
        free(oglc);
        return NULL;
    }

    context = WGLGC_CreateContext(wglinfo->screen, wglinfo->pixfmt);
    if (context == 0) {
        J2dTraceLn(J2D_TRACE_ERROR, "could not create new WGL context");
        free(oglc);
        free(ctxinfo);
        return NULL;
    }

    // REMIND: when using wglShareLists, the two contexts must use an
    //         identical pixel format...
    if (sharedContext != NULL) {
        WGLCtxInfo *sharedCtxInfo = (WGLCtxInfo *)sharedContext->ctxInfo;
        if (!j2d_wglShareLists(sharedCtxInfo->context, context)) {
            J2dTraceLn(J2D_TRACE_WARNING, "unable to share lists");
        }
    }

    ctxinfo->context = context;
    ctxinfo->configInfo = wglinfo;
    oglc->ctxInfo = ctxinfo;
    oglc->extInfo = &wglinfo->extInfo;

    // initialize this value to 1.0f as it is sometimes used when uploading
    // textures (see OGLBlitLoops_Blit()), and in the case of the shared
    // context, we skip the InitComposite() step that would otherwise
    // initialize the oglc->extraAlpha value...
    oglc->extraAlpha = 1.0f;

    /**
     * REMIND: The following is a total hack.  The first time we enter
     * this method, we must be on the main thread attempting to initialize
     * the default GraphicsConfig.  Each time we enter this method, we would
     * like to upcall to the static EventQueue.isDispatchThread() method.
     * But the first time we call that static method, we may end up implicitly
     * loading WToolkit, which will also attempt to call getDefaultConfig().
     * But since we still haven't fully initialized the default config, we
     * re-enter the getDefaultConfig() method (in the same thread), and then
     * come down through this same method.  JNI doesn't seem to like this
     * situation, and we end up with a seg fault.  This may be a JNI bug,
     * but for now we'll work around it, by avoiding the upcall the first
     * time around.  After that, we should be able to make this upcall without
     * the nasty side effects.
     */
    if (firstTime) {
        oglc->onJED = JNI_FALSE;
        firstTime = JNI_FALSE;
    } else {
        oglc->onJED = JNU_CallStaticMethodByName(env, NULL,
                                                 "java/awt/EventQueue",
                                                 "isDispatchThread",
                                                 "()Z").z;
    }

    if (useDisposer) {
        // initialize a new Disposer target (when the Java-level thread object
        // is about to go away, Disposer will invoke our native dispose
        // method, which destroys this OGLContext and its associated entries)
        jobject thread = JNU_CallStaticMethodByName(env, NULL,
                                                    "java/lang/Thread",
                                                    "currentThread",
                                                    "()Ljava/lang/Thread;").l;
        if (thread == NULL) {
            J2dTraceLn(J2D_TRACE_ERROR, "could not fetch current thread");
            j2d_wglDeleteContext(context);
            free(oglc);
            free(ctxinfo);
            return NULL;
        }

        Disposer_AddRecord(env, thread,
                           WGLGC_DisposeOGLContext, ptr_to_jlong(oglc));
    }

    return oglc;
}

/**
 * Initializes the shared context and shared surface.
 */
static jint
WGLGC_InitSharedContext(JNIEnv *env, WGLGraphicsConfigInfo *wglinfo)
{
    HWND hwnd;
    HDC hdc;

    J2dTraceLn(J2D_TRACE_INFO, "in WGLGC_InitSharedContext");

    sharedContext = WGLGC_InitOGLContext(env, wglinfo, JNI_FALSE);
    if (sharedContext == NULL) {
        J2dTraceLn(J2D_TRACE_ERROR, "could not create shared context");
        return SD_FAILURE;
    }

    hwnd = WGLGC_CreateScratchWindow(wglinfo->screen);
    if (hwnd == 0) {
        J2dTraceLn(J2D_TRACE_ERROR, "could not create scratch window");
        return SD_FAILURE;
    }

    // get the HDC for the scratch window
    hdc = GetDC(hwnd);
    if (hdc == 0) {
        J2dTraceLn(J2D_TRACE_ERROR, "could not get dc for scratch window");
        DestroyWindow(hwnd);
        return SD_FAILURE;
    }

    sharedSurface = j2d_wglCreatePbufferARB(hdc, wglinfo->pixfmt,
                                            1, 1, NULL);

    ReleaseDC(hwnd, hdc);
    DestroyWindow(hwnd);

    if (sharedSurface == 0) {
        J2dTraceLn(J2D_TRACE_ERROR, "could not create shared surface");
        WGLGC_DestroyOGLContext(env, sharedContext);
        sharedContext = NULL;
        return SD_FAILURE;
    }

    sharedSurfaceDC = j2d_wglGetPbufferDCARB(sharedSurface);
    if (sharedSurfaceDC == 0) {
        J2dTraceLn(J2D_TRACE_ERROR, "could not get hdc for shared pbuffer");
        WGLGC_DestroyOGLContext(env, sharedContext);
        sharedContext = NULL;
        j2d_wglDestroyPbufferARB(sharedSurface);
        sharedSurface = 0;
        return SD_FAILURE;
    }

    sharedConfigInfo = wglinfo;

    return SD_SUCCESS;
}

/**
 * Determines whether the WGL pipeline can be used for a given GraphicsConfig
 * provided its screen number and visual ID.  If the minimum requirements are
 * met, the native WGLGraphicsConfigInfo structure is initialized for this
 * GraphicsConfig with the necessary information (pixel format, etc.)
 * and a pointer to this structure is returned as a jlong.  If
 * initialization fails at any point, zero is returned, indicating that WGL
 * cannot be used for this GraphicsConfig (we should fallback on the existing
 * DX pipeline).
 */
static jlong
WGLGC_GetWGLConfigInfo(JNIEnv *env, jint screennum, jint pixfmt)
{
    PIXELFORMATDESCRIPTOR pfd;
    HWND hwnd;
    HDC hdc;
    HGLRC context;
    WGLGraphicsConfigInfo *wglinfo;
    const unsigned char *versionstr;
    const char *extstr;
    int attr[] = { WGL_DOUBLE_BUFFER_ARB, 0 };
    int db;

    J2dTraceLn(J2D_TRACE_INFO, "in WGLGC_GetWGLConfigInfo");

    // initialize GL/WGL extension functions
    if (!WGLGC_InitExtFuncs(screennum)) {
        J2dTraceLn(J2D_TRACE_ERROR, "could not initialize extension funcs");
        return 0L;
    }

    // create the WGLGraphicsConfigInfo record for this config
    wglinfo = (WGLGraphicsConfigInfo *)malloc(sizeof(WGLGraphicsConfigInfo));
    if (wglinfo == NULL) {
        J2dTraceLn(J2D_TRACE_ERROR, "could not allocate memory for wglinfo");
        return 0L;
    }

    // create a scratch window
    hwnd = WGLGC_CreateScratchWindow(screennum);
    if (hwnd == 0) {
        free(wglinfo);
        return 0L;
    }

    // get the HDC for the scratch window
    hdc = GetDC(hwnd);
    if (hdc == 0) {
        J2dTraceLn(J2D_TRACE_ERROR, "could not get dc for scratch window");
        DestroyWindow(hwnd);
        free(wglinfo);
        return 0L;
    }

    if (pixfmt == 0) {
        // find an appropriate pixel format
        pixfmt = WGLGC_GetPixelFormatForDC(hdc);
        if (pixfmt == 0) {
            J2dTraceLn(J2D_TRACE_ERROR, "could not find appropriate pixfmt");
            ReleaseDC(hwnd, hdc);
            DestroyWindow(hwnd);
            free(wglinfo);
            return 0L;
        }
    }

    // set the pixel format for the scratch window
    if (!SetPixelFormat(hdc, pixfmt, &pfd)) {
        J2dTraceLn(J2D_TRACE_ERROR, "error setting pixel format");
        ReleaseDC(hwnd, hdc);
        DestroyWindow(hwnd);
        free(wglinfo);
        return 0L;
    }

    // create a temporary context
    context = j2d_wglCreateContext(hdc);
    if (context == 0) {
        J2dTraceLn(J2D_TRACE_ERROR, "could not create temp WGL context");
        ReleaseDC(hwnd, hdc);
        DestroyWindow(hwnd);
        free(wglinfo);
        return 0L;
    }

    // make the context current so that we can query the OpenGL version
    // and extension strings
    if (!j2d_wglMakeCurrent(hdc, context)) {
        J2dTraceLn(J2D_TRACE_ERROR, "could not make temp context current");
        j2d_wglDeleteContext(context);
        ReleaseDC(hwnd, hdc);
        DestroyWindow(hwnd);
        free(wglinfo);
        return 0L;
    }

    // invalidate the current context
    OGLContext_InvalidateCurrentContext(env);

    // get version and extension strings
    versionstr = j2d_glGetString(GL_VERSION);
    extstr = j2d_wglGetExtensionsStringARB(hdc);
    OGLContext_GetExtensionInfo(&wglinfo->extInfo);

    J2dTraceLn1(J2D_TRACE_INFO, "OpenGL version: %s", versionstr);

    if (!OGLContext_IsVersionSupported(versionstr)) {
        J2dTraceLn(J2D_TRACE_ERROR, "invalid OpenGL version; 1.2 is required");
        j2d_wglMakeCurrent(NULL, NULL);
        j2d_wglDeleteContext(context);
        ReleaseDC(hwnd, hdc);
        DestroyWindow(hwnd);
        free(wglinfo);
        return 0L;
    }

    // check for required WGL extensions
    if (!OGLContext_IsExtensionAvailable(extstr, "WGL_ARB_pbuffer") ||
        !OGLContext_IsExtensionAvailable(extstr, "WGL_ARB_render_texture") ||
        !OGLContext_IsExtensionAvailable(extstr, "WGL_ARB_pixel_format"))
    {
        J2dTraceLn(J2D_TRACE_ERROR, "required extension(s) not available");
        j2d_wglMakeCurrent(NULL, NULL);
        j2d_wglDeleteContext(context);
        ReleaseDC(hwnd, hdc);
        DestroyWindow(hwnd);
        free(wglinfo);
        return 0L;
    }

    // check whether pixel format is double buffered
    j2d_wglGetPixelFormatAttribivARB(hdc, pixfmt, 0, 1, attr, &db);

    // destroy the temporary resources
    j2d_wglMakeCurrent(NULL, NULL);
    j2d_wglDeleteContext(context);
    ReleaseDC(hwnd, hdc);
    DestroyWindow(hwnd);

    J2dTraceLn(J2D_TRACE_VERBOSE,
               "successfully finished checking dependencies");

    wglinfo->screen = screennum;
    wglinfo->pixfmt = pixfmt;
    wglinfo->isDoubleBuffered = db;

    // create the single shared context (if it hasn't been created already)
    if (sharedContext == NULL) {
        if (WGLGC_InitSharedContext(env, wglinfo) == SD_FAILURE) {
            J2dTraceLn(J2D_TRACE_ERROR, "could not init shared context");
            free(wglinfo);
            return 0L;
        }
    }

    return ptr_to_jlong(wglinfo);
}

JNIEXPORT jboolean JNICALL
Java_sun_java2d_opengl_WGLGraphicsConfig_initWGL(JNIEnv *env, jclass wglgc)
{
    J2dTraceLn(J2D_TRACE_INFO, "in WGLGraphicsConfig_initWGL");

    return WGLGC_InitWGL();
}

JNIEXPORT jlong JNICALL
Java_sun_java2d_opengl_WGLGraphicsConfig_getWGLConfigInfo(JNIEnv *env,
                                                          jclass wglgc,
                                                          jint screennum,
                                                          jint pixfmt)
{
    J2dTraceLn(J2D_TRACE_INFO, "in WGLGraphicsConfig_getWGLConfigInfo");

    return WGLGC_GetWGLConfigInfo(env, screennum, pixfmt);
}

JNIEXPORT jint JNICALL
Java_sun_java2d_opengl_WGLGraphicsConfig_getDefaultPixFmt(JNIEnv *env,
                                                          jclass wglgc,
                                                          jint screennum)
{
    J2dTraceLn(J2D_TRACE_INFO, "in WGLGraphicsConfig_getDefaultPixFmt");

    // REMIND: eventually we should implement this method so that it finds
    //         the most appropriate default pixel format for the given
    //         device; for now, we'll just return 0, and then we'll find
    //         an appropriate pixel format in WGLGC_GetWGLConfigInfo()...
    return 0;
}

JNIEXPORT jboolean JNICALL
Java_sun_java2d_opengl_WGLGraphicsConfig_isDoubleBuffered(JNIEnv *env,
                                                          jobject wglgc,
                                                          jlong configInfo)
{
    WGLGraphicsConfigInfo *wglinfo =
        (WGLGraphicsConfigInfo *)jlong_to_ptr(configInfo);

    J2dTraceLn(J2D_TRACE_INFO,"in WGLGraphicsConfig_isDoubleBuffered");

    if (wglinfo == NULL) {
	return JNI_FALSE;        
    }

    return wglinfo->isDoubleBuffered;
}

JNIEXPORT jboolean JNICALL
Java_sun_java2d_opengl_WGLGraphicsConfig_isBlendPremultAvailable(JNIEnv *env,
                                                             jclass wglgc,
                                                             jlong configInfo)
{
    WGLGraphicsConfigInfo *wglinfo =
        (WGLGraphicsConfigInfo *)jlong_to_ptr(configInfo);

    J2dTraceLn(J2D_TRACE_INFO,"in WGLGraphicsConfig_isBlendPremultAvailable");

    if (wglinfo == NULL) {
	return JNI_FALSE;
    }

    return wglinfo->extInfo.blendPremult;
}

JNIEXPORT jboolean JNICALL
Java_sun_java2d_opengl_WGLGraphicsConfig_isTexNonPow2Available(JNIEnv *env,
                                                             jclass wglgc,
                                                             jlong configInfo)
{
    WGLGraphicsConfigInfo *wglinfo =
        (WGLGraphicsConfigInfo *)jlong_to_ptr(configInfo);

    J2dTraceLn(J2D_TRACE_INFO, "in WGLGraphicsConfig_isTexNonPow2Available");

    if (wglinfo == NULL) {
	return JNI_FALSE;
    }

    return wglinfo->extInfo.texNonPow2;
}

JNIEXPORT void JNICALL
Java_sun_java2d_opengl_WGLGraphicsConfig_swapBuffers(JNIEnv *env,
                                                     jobject wglgc,
                                                     jlong pPeerData)
{
    HWND window;
    HDC hdc;

    J2dTraceLn(J2D_TRACE_INFO, "in WGLGraphicsConfig_swapBuffers");

    window = AwtComponent_GetHWnd(env, pPeerData);
    if (!IsWindow(window)) {
        J2dTraceLn(J2D_TRACE_ERROR, "disposed component");
        return;
    }

    hdc = GetDC(window);
    if (hdc == 0) {
        J2dTraceLn(J2D_TRACE_ERROR, "invalid hdc");
        return;
    }

    if (!SwapBuffers(hdc)) {
        J2dTraceLn(J2D_TRACE_ERROR, "error in SwapBuffers");
    }

    if (!ReleaseDC(window, hdc)) {
        J2dTraceLn(J2D_TRACE_ERROR, "error while releasing dc");
    }
}

JNIEXPORT jlong JNICALL
Java_sun_java2d_opengl_WGLGraphicsConfig_initNativeSharedContext
    (JNIEnv *env, jobject wglsd)
{
    OGLContext *oglc;

    J2dTraceLn(J2D_TRACE_INFO,
               "in WGLGraphicsConfig_initNativeSharedContext");

    if (sharedContext == NULL || sharedConfigInfo == NULL) {
        J2dTraceLn(J2D_TRACE_ERROR, "shared context not yet inited");
        return 0L;
    }

    oglc = WGLGC_InitOGLContext(env, sharedConfigInfo, JNI_TRUE);
    if (oglc == NULL) {
        J2dTraceLn(J2D_TRACE_ERROR, "could not create native shared context");
        return 0L;
    }

    return ptr_to_jlong(oglc);
}

JNIEXPORT jlong JNICALL
Java_sun_java2d_opengl_WGLGraphicsConfig_makeNativeSharedContextCurrent
    (JNIEnv *env, jobject wglsd, jlong pCtx)
{
    OGLContext *oglc = (OGLContext *)jlong_to_ptr(pCtx);
    WGLCtxInfo *ctxinfo;

    J2dTraceLn(J2D_TRACE_INFO,
               "in WGLGraphicsConfig_makeNativeSharedContextCurrent");

    if (oglc == NULL) {
        J2dTraceLn(J2D_TRACE_ERROR, "shared context is null");
        return 0L;
    }

    ctxinfo = (WGLCtxInfo *)oglc->ctxInfo;
    if (!j2d_wglMakeCurrent(sharedSurfaceDC, ctxinfo->context)) {
        J2dTraceLn(J2D_TRACE_ERROR,
                   "could not make shared WGL context current");
        return 0L;
    }

    return pCtx;
}
