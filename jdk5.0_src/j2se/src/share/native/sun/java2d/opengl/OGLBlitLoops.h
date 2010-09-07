/*
 * @(#)OGLBlitLoops.h	1.1 04/03/08
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef OGLBlitLoops_h_Included
#define OGLBlitLoops_h_Included

#include "OGLSurfaceData.h"

void OGLBlitSurfaceToSurface(OGLSDOps *srcOps, OGLSDOps *dstOps,
                             jint srcx, jint srcy, jint dstx, jint dsty,
                             jint srcw, jint srch, jint dstw, jint dsth);

#endif /* OGLBlitLoops_h_Included */
