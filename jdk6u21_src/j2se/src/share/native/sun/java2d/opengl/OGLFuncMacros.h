/*
 * @(#)OGLFuncMacros.h	1.3 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
