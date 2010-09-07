/*
 * @(#)sunt2kscaler.h	1.3 12/19/03
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef SunT2KScalerIncludesDefined
#define SunT2KScalerIncludesDefined


#include "jni.h"
#include <math.h>
#include "t2k.h"
#include "glyphblitting.h"
#include "fontscalerdefs.h"

#ifdef  __cplusplus
extern "C" {
#endif 


#define WIDE_SHIFTLEFT(outH, outL, inH, inL, shift) \
  { \
   (outH) = ((inH) << (shift)) | ((inL) >> (32 - (shift))); \
   (outL) = (inL) << (shift); \
  }

#define WIDE_NEGATE(hi, lo) \
  {  \
    (hi) = ~(hi); \
    if (((lo) = -(Int32)lo) == 0) { \
       (hi) += 1; \
    } \
  }

#define WIDE_ADDPOS(hi, lo, scaler) \
  { \
     UInt32 tmp = (lo) + (scaler); \
     if (tmp < (lo)) { \
        (hi) += 1; \
     } \
     (lo) = tmp; \
  }

#define WIDE_LESSTHAN(hi, lo, hi2, lo2) \
    ((hi) < (hi2) || (hi) == (hi2) && (lo) < (lo2))

#define WIDE_SUBWIDE(hi, lo, subhi, sublo) \
 { \
    (hi) -= (subhi); \
    if ((lo) < (sublo)) { \
       (hi) -= 1; \
    } \
    (lo) -= (sublo); \
 }

#define WIDE_MUL(hi, lo, src1, src2) \
 { \
    int neg = 0; \
    UInt32 a,b,c,d; \
    UInt32 high, middle, low; \
 \
    if (src1 < 0) { \
	src1 = -src1; \
	neg = ~0; \
    } \
    if (src2 < 0) { \
	src2 = -src2; \
	neg = ~neg; \
    } \
 \
    a = src1 >> 16; \
    b = (UInt16)src1; \
    c = src2 >> 16; \
    d = (UInt16)src2; \
 \
    high = a * c; \
    middle = a * d + c * b; \
    low = b *d; \
 \
    lo = low + (middle << 16); \
    hi = high + (middle >> 16) + (((low >> 16) + (UInt16)middle) >> 16); \
 \
    if (neg) { \
	WIDE_NEGATE(hi, lo); \
    } \
}

Int32 t2kMagnitude(Int32 x, Int32 y);
hsFract t2kFracMul(hsFract src1, hsFract src2);
Int32 t2kFracDiv(hsFract numerator, hsFract denom);

#ifdef T2K_DEBUGGING

#define t2kIfDebugMessage(clause, message, refcon)  \
                         if (clause) hsDebugMessage(message, refcon);

#else

#define t2kIfDebugMessage(clause, message, refcon)  ((void) 0)

#endif

#define QUADPATHTYPE  1
#define CUBICPATHTYPE 2

typedef struct TTLayoutTableCache {
    tt_uint8* gsub;
    tt_uint8* gpos;
    tt_uint8* gdef;
    tt_uint8* mort;
    int gsub_len;
    int gpos_len;
    int gdef_len;
    int mort_len;
} TTLayoutTableCache;

TTLayoutTableCache* newLayoutTableCache();
void freeLayoutTableCache(TTLayoutTableCache* ltc);

#define GSUB_TAG 0x47535542
#define GPOS_TAG 0x47504F53
#define GDEF_TAG 0x47444546
#define MORT_TAG 0x6D6F7274

typedef struct T2KScalerInfo {
    /* Every call down must copy its "env" and "font2D" variable into the
     * appropriate members of this struct.
     */
    JNIEnv*   env;
    T2K*      t2k;
    tt_uint8* fontData;
    jobject   font2D;
    jobject   directBuffer;
    int       fontDataOffset;
    int       fontDataLength;
    int       fileSize;
    int       pathType;
    TTLayoutTableCache* layoutTables;
} T2KScalerInfo;

/*
 * styling is set via making a T2K internal API public so that we
 * can re-use a T2K instance.
 * This looks perfectly OK as that internal API (SetStyling in truetype.c)
 * just sets instance variables.
 */
typedef struct T2KScalerContext {
    T2KScalerInfo*    scalerInfo;
    T2K_TRANS_MATRIX  t2kMatrix;     /* glyph TX - includes dev TX */
    T2K_AlgStyleDescriptor styling;
    jboolean          doAA;
    jboolean          doFM;
    jboolean          doAlgoStyle;
    t2kFixed          boldness;
    t2kFixed          italic;
    int               greyLevel;
    int               t2kFlags;
    int               pathType;
} T2KScalerContext;


extern int setupT2KContext(JNIEnv *env, jobject font2D, 
			   T2KScalerInfo *scalerInfo,
			   T2KScalerContext *context,
			   jboolean sbits);

extern int getUnitsPerEmForLayout(T2KScalerInfo *scalerInfo);

extern T2KScalerInfo *theNullScaler;
extern T2KScalerContext *theNullScalerContext;

#ifdef  __cplusplus
}
#endif 

#endif
