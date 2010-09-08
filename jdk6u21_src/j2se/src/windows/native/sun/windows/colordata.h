/*
 * @(#)colordata.h	1.13 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
#ifndef _COLORDATA_H_
#define _COLORDATA_H_

#include "img_globals.h"

typedef struct _ColorData {
    char* img_oda_red;
    char* img_oda_green;
    char* img_oda_blue;
    unsigned char* img_clr_tbl;
    int *pGrayInverseLutData;
} ColorData;

#define CANFREE(pData) (pData)

#endif
