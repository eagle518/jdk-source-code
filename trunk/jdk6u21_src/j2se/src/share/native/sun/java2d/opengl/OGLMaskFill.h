/*
 * @(#)OGLMaskFill.h	1.5 10/03/23
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef OGLMaskFill_h_Included
#define OGLMaskFill_h_Included

#include "OGLContext.h"

void OGLMaskFill_MaskFill(OGLContext *oglc,
                          jint x, jint y, jint w, jint h,
                          jint maskoff, jint maskscan, jint masklen,
                          unsigned char *pMask);

#endif /* OGLMaskFill_h_Included */
