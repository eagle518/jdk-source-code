/*
 * @(#)D3DMaskBlit.h	1.2 10/03/23
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef D3DMaskBlit_h_Included
#define D3DMaskBlit_h_Included

#include "D3DContext.h"

HRESULT D3DMaskBlit_MaskBlit(JNIEnv *env, D3DContext *d3dc,
                             jint dstx, jint dsty,
                             jint width, jint height,
                             void *pPixels);

#endif /* D3DMaskBlit_h_Included */
