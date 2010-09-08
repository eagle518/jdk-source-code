/*
 * @(#)awt_XmDnD.h	1.13 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <Xm/Display.h>
#include <Xm/DropSMgr.h>
#include <Xm/DropTrans.h>
#include <inttypes.h>

/**
 *
 */

typedef struct DropSiteInfo {
        Widget                  tlw;

        jobject                 component;
        Boolean                 isComposite;
        uint32_t                dsCnt;
} DropSiteInfo;
