/*
 * @(#)DrawPath.h	1.4 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef DrawPath_h_Included
#define DrawPath_h_Included

typedef struct {
    SurfaceDataRasInfo* pRasInfo;
    jint pixel;
    NativePrimitive* pPrim;
    CompositeInfo* pCompInfo;
} DrawHandlerData;

#define DHND(HND) ((DrawHandlerData*)((HND)->pData))

#endif


