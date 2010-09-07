/*
 * @(#)img_fsutil.h	1.11 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * This file contains utility macros used by the Floyd-Steinberg
 * algorithms used in some of the other include files.
 */

#ifndef IMG_FSUTIL_H
#define IMG_FSUTIL_H

#define DitherDist(ep, e1, e2, e3, ec, c)		\
    do {						\
	e3 = (ec << 1);					\
	e1 = e3 + ec;					\
	e2 = e3 + e1;					\
	e3 += e2;					\
							\
	ep[0].c += e1 >>= 4;				\
	ep[1].c += e2 >>= 4;				\
	ep[2].c += e3 >>= 4;				\
	ec -= e1 + e2 + e3;				\
    } while (0)

#endif /* IMG_FSUTIL_H */
