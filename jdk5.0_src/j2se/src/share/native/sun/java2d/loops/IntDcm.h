/*
 * @(#)IntDcm.h	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef IntDcm_h_Included
#define IntDcm_h_Included

typedef jint	IntDcmPixelType;
typedef jint	IntDcmElemType;

#define SwapIntDcmComponentsX123ToX321(pixel) \
    (((pixel) << 16) | \
     ((pixel) & 0xff00) | \
     (((pixel) >> 16) & 0xff))

#define SwapIntDcmComponentsX123ToC321(pixel) \
    (((pixel & 0xff) << 16) | \
     ((pixel) & 0xff00) | \
     (((pixel) >> 16) & 0xff))

#define SwapIntDcmComponentsX123ToS321(pixel) \
    (0xff000000 | \
     ((pixel) << 16) | \
     ((pixel) & 0xff00) | \
     (((pixel) >> 16) & 0xff))

#define SwapIntDcmComponents4123To4321(pixel) \
    ((((pixel) & 0xff) << 16) | \
     ((pixel) & 0xff00ff00) | \
     (((pixel) >> 16) & 0xff))

#define ExtractIntDcmComponentsX123(pixel, c1, c2, c3) \
    do { \
	(c3) = (pixel) & 0xff; \
	(c2) = ((pixel) >> 8) & 0xff; \
	(c1) = ((pixel) >> 16) & 0xff; \
    } while (0)

#define ExtractIntDcmComponents123X(pixel, c1, c2, c3) \
    do { \
	(c3) = ((pixel) >> 8) & 0xff; \
	(c2) = ((pixel) >> 16) & 0xff; \
	(c1) = ((pixel) >> 24) & 0xff; \
    } while (0)

#define ExtractIntDcmComponents1234(pixel, c1, c2, c3, c4) \
    do { \
	(c4) = (pixel) & 0xff; \
	(c3) = ((pixel) >> 8) & 0xff; \
	(c2) = ((pixel) >> 16) & 0xff; \
	(c1) = ((pixel) >> 24) & 0xff; \
    } while (0)

#define ComposeIntDcmComponentsX123(c1, c2, c3) \
    (((((c1) << 8) | (c2)) << 8) | (c3))

#define ComposeIntDcmComponents123X(c1, c2, c3) \
    ((((((c1) << 8) | (c2)) << 8) | (c3)) << 8)

#define ComposeIntDcmComponents1234(c1, c2, c3, c4) \
    (((((((c1) << 8) | (c2)) << 8) | (c3)) << 8) | (c4))

#endif /* IntDcm_h_Included */
