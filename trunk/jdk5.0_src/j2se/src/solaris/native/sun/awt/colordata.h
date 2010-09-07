/*
 * @(#)colordata.h	1.13 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
#ifndef _COLORDATA_H_
#define _COLORDATA_H_

#include "img_globals.h"

typedef struct ColorEntry {
	unsigned char r, g, b;
	unsigned char flags;
} ColorEntry;

typedef struct _ColorData {
    ColorEntry  *awt_Colors;
    int         awt_numICMcolors;
    int         *awt_icmLUT;
    unsigned char *awt_icmLUT2Colors;
    unsigned char *img_grays;
    unsigned char *img_clr_tbl;
    char* img_oda_red;
    char* img_oda_green;
    char* img_oda_blue;
    int *pGrayInverseLutData;
    int screendata;
} ColorData;


#define CANFREE(pData) (pData  && (pData->screendata == 0))

#endif           /* _COLORDATA_H_ */
