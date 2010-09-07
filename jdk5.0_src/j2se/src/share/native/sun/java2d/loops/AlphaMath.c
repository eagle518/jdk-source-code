/*
 * @(#)AlphaMath.c	1.5 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "AlphaMath.h"

unsigned char mul8table[256][256];
unsigned char div8table[256][256];

void initAlphaTables()
{
    unsigned int i;
    unsigned int j;

    for (i = 1; i < 256; i++) {			/* SCALE == (1 << 24) */
	int inc = (i << 16) + (i<<8) + i;	/* approx. SCALE * (i/255.0) */
	int val = inc + (1 << 23);		/* inc + SCALE*0.5 */
        for (j = 1; j < 256; j++) {
            mul8table[i][j] = (val >> 24);	/* val / SCALE */
	    val += inc;
        }
    }

    for (i = 1; i < 256; i++) {
	unsigned int inc;
	unsigned int val;
	inc = 0xff;
	inc = ((inc << 24) + i/2) / i;
	val = (1 << 23);
	for (j = 0; j < i; j++) {
	    div8table[i][j] = (val >> 24);
	    val += inc;
	}
	for (j = i; j < 256; j++) {
	    div8table[i][j] = 255;
	}
    }
}
