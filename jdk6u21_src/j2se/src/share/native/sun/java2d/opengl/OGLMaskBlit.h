/*
 * @(#)OGLMaskBlit.h	1.4 10/03/23
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef OGLMaskBlit_h_Included
#define OGLMaskBlit_h_Included

#include "OGLContext.h"

void OGLMaskBlit_MaskBlit(JNIEnv *env, OGLContext *oglc,
                          jint dstx, jint dsty,
                          jint width, jint height,
                          void *pPixels);

#endif /* OGLMaskBlit_h_Included */
