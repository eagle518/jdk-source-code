/*
 * @(#)OGLFuncMacros.h	1.1 04/03/08
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef OGLFuncMacros_h_Included
#define OGLFuncMacros_h_Included

#define OGL_FUNC_TYPE(f)        f##Type
#define OGL_J2D_MANGLE(f)       j2d_##f
#define OGL_DECLARE_FUNC(f)     OGL_FUNC_TYPE(f) OGL_J2D_MANGLE(f)
#define OGL_DECLARE_EXT_FUNC(f) OGL_DECLARE_FUNC(f)
#define OGL_EXTERN_FUNC(f)      extern OGL_DECLARE_FUNC(f)
#define OGL_EXTERN_EXT_FUNC(f)  OGL_EXTERN_FUNC(f)

#endif /* OGLFuncMacros_h_Included */
