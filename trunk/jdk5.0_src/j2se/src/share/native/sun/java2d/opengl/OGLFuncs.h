/*
 * @(#)OGLFuncs.h	1.6 04/03/30
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef OGLFuncs_h_Included
#define OGLFuncs_h_Included

#include "jni.h"
#include "J2D_GL/gl.h"
#include "J2D_GL/glext.h"
#include "OGLFuncMacros.h"
#include "OGLFuncs_md.h"
#include "Trace.h"

jboolean OGLFuncs_OpenLibrary();
void     OGLFuncs_CloseLibrary();
jboolean OGLFuncs_InitPlatformFuncs();
jboolean OGLFuncs_InitBaseFuncs();
jboolean OGLFuncs_InitExtFuncs();

/**
 * Core OpenGL 1.1 function typedefs
 */
typedef void (GLAPIENTRY *glAlphaFuncType)(GLenum func, GLclampf ref);
typedef GLboolean (GLAPIENTRY *glAreTexturesResidentType)(GLsizei n, const GLuint *textures, GLboolean *residences);
typedef void (GLAPIENTRY *glBeginType)(GLenum mode);
typedef void (GLAPIENTRY *glBindTextureType)(GLenum target, GLuint texture);
typedef void (GLAPIENTRY *glBitmapType)(GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap);
typedef void (GLAPIENTRY *glBlendFuncType)(GLenum sfactor, GLenum dfactor);
typedef void (GLAPIENTRY *glClearType)(GLbitfield mask);
typedef void (GLAPIENTRY *glClearColorType)(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
typedef void (GLAPIENTRY *glClearStencilType)(GLint s);
typedef void (GLAPIENTRY *glColor3ubType)(GLubyte red, GLubyte green, GLubyte blue);
typedef void (GLAPIENTRY *glColor4fType)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
typedef void (GLAPIENTRY *glColor4ubType)(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha);
typedef void (GLAPIENTRY *glColorMaskType)(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
typedef void (GLAPIENTRY *glCopyPixelsType)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum type);
typedef void (GLAPIENTRY *glCopyTexSubImage2DType)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (GLAPIENTRY *glDeleteTexturesType)(GLsizei n, const GLuint *textures);
typedef void (GLAPIENTRY *glDisableType)(GLenum cap);
typedef void (GLAPIENTRY *glDrawBufferType)(GLenum mode);
typedef void (GLAPIENTRY *glDrawPixelsType)(GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (GLAPIENTRY *glEnableType)(GLenum cap);
typedef void (GLAPIENTRY *glEndType)(void);
typedef void (GLAPIENTRY *glFinishType)(void);
typedef void (GLAPIENTRY *glFlushType)(void);
typedef void (GLAPIENTRY *glGenTexturesType)(GLsizei n, GLuint *textures);
typedef void (GLAPIENTRY *glGetBooleanvType)(GLenum pname, GLboolean *params);
typedef void (GLAPIENTRY *glGetDoublevType)(GLenum pname, GLdouble *params);
typedef GLenum (GLAPIENTRY *glGetErrorType)(void);
typedef void (GLAPIENTRY *glGetFloatvType)(GLenum pname, GLfloat *params);
typedef void (GLAPIENTRY *glGetIntegervType)(GLenum pname, GLint *params);
typedef const GLubyte * (GLAPIENTRY *glGetStringType)(GLenum name);
typedef void (GLAPIENTRY *glGetTexLevelParameterivType)(GLenum target, GLint level, GLenum pname, GLint *params);
typedef void (GLAPIENTRY *glHintType)(GLenum target, GLenum mode);
typedef GLboolean (GLAPIENTRY *glIsEnabledType)(GLenum cap);
typedef GLboolean (GLAPIENTRY *glIsTextureType)(GLuint texture);
typedef void (GLAPIENTRY *glLoadIdentityType)(void);
typedef void (GLAPIENTRY *glLoadMatrixdType)(const GLdouble *m);
typedef void (GLAPIENTRY *glLogicOpType)(GLenum opcode);
typedef void (GLAPIENTRY *glMatrixModeType)(GLenum mode);
typedef void (GLAPIENTRY *glOrthoType)(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
typedef void (GLAPIENTRY *glPixelMapusvType)(GLenum map, GLsizei mapsize, const GLushort *values);
typedef void (GLAPIENTRY *glPixelStoreiType)(GLenum pname, GLint param);
typedef void (GLAPIENTRY *glPixelTransferfType)(GLenum pname, GLfloat param);
typedef void (GLAPIENTRY *glPixelZoomType)(GLfloat xfactor, GLfloat yfactor);
typedef void (GLAPIENTRY *glPolygonOffsetType)(GLfloat factor, GLfloat units);
typedef void (GLAPIENTRY *glPopMatrixType)(void);
typedef void (GLAPIENTRY *glPrioritizeTexturesType)(GLsizei n, const GLuint *textures, const GLclampf *priorities);
typedef void (GLAPIENTRY *glPushMatrixType)(void);
typedef void (GLAPIENTRY *glRasterPos2iType)(GLint x, GLint y);
typedef void (GLAPIENTRY *glReadBufferType)(GLenum mode);
typedef void (GLAPIENTRY *glReadPixelsType)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels);
typedef void (GLAPIENTRY *glRectiType)(GLint x1, GLint y1, GLint x2, GLint y2);
typedef void (GLAPIENTRY *glScalefType)(GLfloat x, GLfloat y, GLfloat z);
typedef void (GLAPIENTRY *glScissorType)(GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (GLAPIENTRY *glStencilFuncType)(GLenum func, GLint ref, GLuint mask);
typedef void (GLAPIENTRY *glStencilOpType)(GLenum fail, GLenum zfail, GLenum zpass);
typedef void (GLAPIENTRY *glTexCoord2fType)(GLfloat s, GLfloat t);
typedef void (GLAPIENTRY *glTexEnviType)(GLenum target, GLenum pname, GLint param);
typedef void (GLAPIENTRY *glTexGeniType)(GLenum coord, GLenum pname, GLint param);
typedef void (GLAPIENTRY *glTexGendvType)(GLenum coord, GLenum pname, const GLdouble *params);
typedef void (GLAPIENTRY *glTexImage1DType)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (GLAPIENTRY *glTexImage2DType)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (GLAPIENTRY *glTexParameteriType)(GLenum target, GLenum pname, GLint param);
typedef void (GLAPIENTRY *glTexSubImage1DType)(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (GLAPIENTRY *glTexSubImage2DType)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (GLAPIENTRY *glTranslatefType)(GLfloat x, GLfloat y, GLfloat z);
typedef void (GLAPIENTRY *glVertex2fType)(GLfloat x, GLfloat y);
typedef void (GLAPIENTRY *glVertex2iType)(GLint x, GLint y);
typedef void (GLAPIENTRY *glViewportType)(GLint x, GLint y, GLsizei width, GLsizei height);

/**
 * OpenGL 1.2 and extension function typedefs
 */
typedef void (GLAPIENTRY *glColorTableType)(GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *table);
typedef void (GLAPIENTRY *glBlendFuncSeparateEXTType)(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);
typedef void (GLAPIENTRY *glActiveTextureARBType)(GLenum texture);
typedef void (GLAPIENTRY *glMultiTexCoord2fARBType)(GLenum texture, GLfloat s, GLfloat t);

/**
 * REMIND: this caused an internal error in the MS compiler!?!?
 *
 *#define OGL_CHECK_FUNC_ERR(f) \
 *    J2dTrace1(J2D_TRACE_ERROR, "could not load function: %s", #f)
 */

#define OGL_CHECK_FUNC_ERR(f) \
    J2dTraceLn(J2D_TRACE_ERROR, #f)

#define OGL_INIT_FUNC(f) \
    OGL_J2D_MANGLE(f) = (OGL_FUNC_TYPE(f)) OGL_GET_PROC_ADDRESS(f)

#define OGL_INIT_AND_CHECK_FUNC(f) \
    OGL_INIT_FUNC(f); \
    if (OGL_J2D_MANGLE(f) == NULL) { \
        OGL_CHECK_FUNC_ERR(f); \
        return JNI_FALSE; \
    }

#define OGL_INIT_EXT_FUNC(f) \
    OGL_J2D_MANGLE(f) = (OGL_FUNC_TYPE(f)) OGL_GET_EXT_PROC_ADDRESS(f)

#define OGL_INIT_AND_CHECK_EXT_FUNC(f) \
    OGL_INIT_EXT_FUNC(f); \
    if (OGL_J2D_MANGLE(f) == NULL) { \
        OGL_CHECK_FUNC_ERR(f); \
        return JNI_FALSE; \
    }

#define OGL_EXPRESS_BASE_FUNCS(action) \
    OGL_##action##_FUNC(glAlphaFunc); \
    OGL_##action##_FUNC(glAreTexturesResident); \
    OGL_##action##_FUNC(glBegin); \
    OGL_##action##_FUNC(glBindTexture); \
    OGL_##action##_FUNC(glBitmap); \
    OGL_##action##_FUNC(glBlendFunc); \
    OGL_##action##_FUNC(glClear); \
    OGL_##action##_FUNC(glClearColor); \
    OGL_##action##_FUNC(glClearStencil); \
    OGL_##action##_FUNC(glColor3ub); \
    OGL_##action##_FUNC(glColor4f); \
    OGL_##action##_FUNC(glColor4ub); \
    OGL_##action##_FUNC(glColorMask); \
    OGL_##action##_FUNC(glCopyPixels); \
    OGL_##action##_FUNC(glCopyTexSubImage2D); \
    OGL_##action##_FUNC(glDeleteTextures); \
    OGL_##action##_FUNC(glDisable); \
    OGL_##action##_FUNC(glDrawBuffer); \
    OGL_##action##_FUNC(glDrawPixels); \
    OGL_##action##_FUNC(glEnable); \
    OGL_##action##_FUNC(glEnd); \
    OGL_##action##_FUNC(glFinish); \
    OGL_##action##_FUNC(glFlush); \
    OGL_##action##_FUNC(glGenTextures); \
    OGL_##action##_FUNC(glGetBooleanv); \
    OGL_##action##_FUNC(glGetDoublev); \
    OGL_##action##_FUNC(glGetError); \
    OGL_##action##_FUNC(glGetFloatv); \
    OGL_##action##_FUNC(glGetIntegerv); \
    OGL_##action##_FUNC(glGetString); \
    OGL_##action##_FUNC(glGetTexLevelParameteriv); \
    OGL_##action##_FUNC(glHint); \
    OGL_##action##_FUNC(glIsEnabled); \
    OGL_##action##_FUNC(glIsTexture); \
    OGL_##action##_FUNC(glLoadIdentity); \
    OGL_##action##_FUNC(glLoadMatrixd); \
    OGL_##action##_FUNC(glLogicOp); \
    OGL_##action##_FUNC(glMatrixMode); \
    OGL_##action##_FUNC(glOrtho); \
    OGL_##action##_FUNC(glPixelMapusv); \
    OGL_##action##_FUNC(glPixelStorei); \
    OGL_##action##_FUNC(glPixelTransferf); \
    OGL_##action##_FUNC(glPixelZoom); \
    OGL_##action##_FUNC(glPolygonOffset); \
    OGL_##action##_FUNC(glPopMatrix); \
    OGL_##action##_FUNC(glPrioritizeTextures); \
    OGL_##action##_FUNC(glPushMatrix); \
    OGL_##action##_FUNC(glRasterPos2i); \
    OGL_##action##_FUNC(glReadBuffer); \
    OGL_##action##_FUNC(glReadPixels); \
    OGL_##action##_FUNC(glRecti); \
    OGL_##action##_FUNC(glScalef); \
    OGL_##action##_FUNC(glScissor); \
    OGL_##action##_FUNC(glStencilFunc); \
    OGL_##action##_FUNC(glStencilOp); \
    OGL_##action##_FUNC(glTexCoord2f); \
    OGL_##action##_FUNC(glTexEnvi); \
    OGL_##action##_FUNC(glTexGeni); \
    OGL_##action##_FUNC(glTexGendv); \
    OGL_##action##_FUNC(glTexImage1D); \
    OGL_##action##_FUNC(glTexImage2D); \
    OGL_##action##_FUNC(glTexParameteri); \
    OGL_##action##_FUNC(glTexSubImage1D); \
    OGL_##action##_FUNC(glTexSubImage2D); \
    OGL_##action##_FUNC(glTranslatef); \
    OGL_##action##_FUNC(glVertex2f); \
    OGL_##action##_FUNC(glVertex2i); \
    OGL_##action##_FUNC(glViewport);

#define OGL_EXPRESS_EXT_FUNCS(action) \
    OGL_##action##_EXT_FUNC(glColorTable); \
    OGL_##action##_EXT_FUNC(glBlendFuncSeparateEXT); \
    OGL_##action##_EXT_FUNC(glActiveTextureARB); \
    OGL_##action##_EXT_FUNC(glMultiTexCoord2fARB);

#define OGL_EXPRESS_ALL_FUNCS(action) \
    OGL_EXPRESS_BASE_FUNCS(action) \
    OGL_EXPRESS_EXT_FUNCS(action) \
    OGL_EXPRESS_PLATFORM_FUNCS(action) \
    OGL_EXPRESS_PLATFORM_EXT_FUNCS(action)

OGL_EXPRESS_ALL_FUNCS(EXTERN)

#endif /* OGLFuncs_h_Included */
