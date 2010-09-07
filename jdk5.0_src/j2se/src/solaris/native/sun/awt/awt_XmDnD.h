/*
 * @(#)awt_XmDnD.h	1.11 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
