/*
 * @(#)GlyphImageRef.h	1.8 04/03/30
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef GlyphImageRef_h_Included
#define GlyphImageRef_h_Included

#ifdef  __cplusplus
extern "C" {
#endif 

/*
 * Previously private structure in GlyphVector.cpp, exposed in order
 * to allow C code to access this without making C++ method calls in C
 * only library.
 */

typedef struct {
    void *glyphInfo;
    const void *pixels;
    int width;
    int height;
    int x;
    int y;
} ImageRef;

#ifdef  __cplusplus
}
#endif 


#endif /* GlyphImageRef_h_Included */
