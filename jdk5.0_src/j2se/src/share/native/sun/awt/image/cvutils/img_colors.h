/*
 * @(#)img_colors.h	1.10 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

extern void img_makePalette(int cmapsize, int tablesize, int lookupsize,
			    float lscale, float weight,
			    int prevclrs, int doMac,
			    unsigned char *reds,
			    unsigned char *greens,
			    unsigned char *blues,
			    unsigned char *lookup);
