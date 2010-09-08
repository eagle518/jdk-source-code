/*
 * @(#)img_colors.h	1.12 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

extern void img_makePalette(int cmapsize, int tablesize, int lookupsize,
			    float lscale, float weight,
			    int prevclrs, int doMac,
			    unsigned char *reds,
			    unsigned char *greens,
			    unsigned char *blues,
			    unsigned char *lookup);
