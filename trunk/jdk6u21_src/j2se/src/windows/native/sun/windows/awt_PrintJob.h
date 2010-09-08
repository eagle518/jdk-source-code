/*
 * @(#)awt_PrintJob.h	1.18 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef AWT_PRINT_JOB_H
#define AWT_PRINT_JOB_H

#include "stdhdrs.h"

/************************************************************************
 * PrintJob class
 */

class AwtPrintJob {
public:
    static jfieldID pageDimensionID;
    static jfieldID pageResolutionID;
    static jfieldID truePageResolutionID;
    static jfieldID graphicsID;
};

#endif /* AWT_PRINT_JOB_H */x
