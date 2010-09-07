/*
 * @(#)scalerMethods.c	1.13 05/28/04
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "t2k.h"
#include "sunt2kscaler.h"
#include "sunfontids.h"
#include "gdefs.h"
#include "sun_font_FontManager.h"
#include "sun_font_FileFont.h"
#include "sun_font_FileFontStrike.h"
#include "sun_font_StrikeCache.h"
#include "sun_font_TrueTypeFont.h"
#include "sun_font_Type1Font.h"

// !!! temp
#include "math.h"

Int32 t2kMagnitude(Int32 x, Int32 y) {
    int	bits = 32;
    UInt32 root = 0;
    UInt32 currH = 0;
    UInt32 currL = 0;
    UInt32 guessH, guessL;
    
    Int32 hi, hi1, hi2;
    UInt32 lo, lo1, lo2;

  // !!! fails when x or y is negative, so force >= 0
  if (x < 0) x = -x;
  if (y < 0) y = -y;

    WIDE_MUL(hi1, lo1, x, x);
    WIDE_MUL(hi2, lo2, y, y);

    lo = lo1+lo2;
    hi = hi1+hi2;
    if (lo < (lo1|lo2)) {
	hi++;
    }

    do {
	WIDE_SHIFTLEFT(currH, currL, currH, currL, 2);
	currL |= (UInt32)hi >> 30;
	WIDE_SHIFTLEFT(hi, lo, hi, lo, 2);
	
	WIDE_SHIFTLEFT(guessH, guessL, 0, root, 2);
	root <<= 1;
	if (WIDE_LESSTHAN(guessH, guessL, currH, currL)) {
	    WIDE_ADDPOS(guessH, guessL, 1);
	    WIDE_SUBWIDE(currH, currL, guessH, guessL);
	    root |= 1;
	}
    } while (--bits);

    return (Int32)root;
}

hsFract t2kFracMul(hsFract src1, hsFract src2) {
    UInt32 lo;
    Int32 hi;

    WIDE_MUL(hi, lo, src1, src2);
    WIDE_ADDPOS(hi, lo, 1L << 29);
    lo = (lo >> 30) | (hi << 2);
    hi = hi >> 30;		

    return lo;
}

Int32 t2kFracDiv(hsFract numerator, hsFract denom) {
    Int32 hi = 0;
    UInt32 lo = numerator;
    int shiftBits = 30;

    /* Set the wide numerator */
    if (numerator < 0) {
	hi = -1L;
    }
    
    /* Multiply by 2^30 */
    hi = (hi << shiftBits) | (lo >> (32 - shiftBits));
    lo = lo << shiftBits;

    /* Divide by the denominator */
    if (denom == 0) {
	if (hi < 0) {
	    hi = kNegInfinity32;
	    lo = 0;
	} else {
	    hi = kPosInfinity32;
	    lo = 0;
	}
    } else {
	UInt32 curr;
        int    i, neg  = 0;
	Int32  resH = 0;
	UInt32 resL = 0;
	Int32  numerH = hi;
	UInt32 numerL = lo;

	if (denom < 0) {
	    denom = -denom;
	    neg = ~0;
	}
        if ((Int32)numerH < 0) {
	    WIDE_NEGATE(numerH, numerL);
	    neg = ~neg;
	}

	/* add denom/2 to get a round result */
        WIDE_ADDPOS(numerH, numerL, denom >> 1);

        curr = (UInt32)numerH >> 31;

	for (i = 0; i < 64; i++) {
	    WIDE_SHIFTLEFT(resH, resL, resH, resL, 1);
	    if ((UInt32)denom <= curr) {
		resL |= 1;
		curr -= denom;
	    }
	    WIDE_SHIFTLEFT(numerH, numerL, numerH, numerL, 1);
	    curr = (curr << 1) | ((UInt32)numerH >> 31);
	}

        if (neg) {
	    WIDE_NEGATE(resH, resL);
	}

	hi = resH;
	lo = resL;       
    }

    return (Int32)lo;
}

/*
 * Class:	sun_font_FontManager
 * Method:    getPlatformFontVar
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL
Java_sun_font_FontManager_getPlatformFontVar(JNIEnv *env, jclass cl) {
    char *c = getenv("JAVA2D_USEPLATFORMFONT");
    if (c) {
	return JNI_TRUE;
    } else {
	return JNI_FALSE;
    }
}


FontManagerNativeIDs sunFontIDs;

JNIEXPORT void JNICALL
Java_sun_font_FontManager_initIDs
    (JNIEnv *env, jclass cls) {
     
     jclass tmpClass = (*env)->FindClass(env, "java/awt/Font");
     sunFontIDs.getFont2DMID =
	 (*env)->GetMethodID(env, tmpClass, "getFont2D",
			     "()Lsun/font/Font2D;");
     sunFontIDs.font2DHandle =
       (*env)->GetFieldID(env, tmpClass,
			  "font2DHandle", "Lsun/font/Font2DHandle;");

     tmpClass = (*env)->FindClass(env, "sun/font/TrueTypeFont");
     sunFontIDs.ttFontClass = (jclass)(*env)->NewGlobalRef(env, tmpClass);
     sunFontIDs.ttReadBlockMID =
	 (*env)->GetMethodID(env, tmpClass, "readBlock",
			     "(Ljava/nio/ByteBuffer;II)I");
     sunFontIDs.ttReadBytesMID =
	 (*env)->GetMethodID(env, tmpClass, "readBytes", "(II)[B");

     tmpClass = (*env)->FindClass(env, "sun/font/Type1Font");
     sunFontIDs.t1ReadBlockMID =
	 (*env)->GetMethodID(env, tmpClass,
			     "readFile", "(Ljava/nio/ByteBuffer;)V");
/* 			     "readBlock", "(II)Ljava/nio/ByteBuffer;"); */

     tmpClass = (*env)->FindClass(env, "java/awt/geom/Point2D$Float");
     sunFontIDs.pt2DFloatClass = (jclass)(*env)->NewGlobalRef(env, tmpClass);
     sunFontIDs.pt2DFloatCtr =
	 (*env)->GetMethodID(env, sunFontIDs.pt2DFloatClass, "<init>","(FF)V");

     sunFontIDs.xFID =
	 (*env)->GetFieldID(env, sunFontIDs.pt2DFloatClass, "x", "F");
     sunFontIDs.yFID =
	 (*env)->GetFieldID(env, sunFontIDs.pt2DFloatClass, "y", "F");

     tmpClass = (*env)->FindClass(env, "sun/font/StrikeMetrics");
     sunFontIDs.strikeMetricsClass=(jclass)(*env)->NewGlobalRef(env, tmpClass);
     
     sunFontIDs.strikeMetricsCtr =
	 (*env)->GetMethodID(env, sunFontIDs.strikeMetricsClass,
			     "<init>", "(FFFFFFFFFF)V");

     tmpClass = (*env)->FindClass(env, "java/awt/geom/Rectangle2D$Float");
     sunFontIDs.rect2DFloatClass = (jclass)(*env)->NewGlobalRef(env, tmpClass);
     sunFontIDs.rect2DFloatCtr =
       (*env)->GetMethodID(env, sunFontIDs.rect2DFloatClass, "<init>", "()V");
     sunFontIDs.rect2DFloatCtr4 =
       (*env)->GetMethodID(env, sunFontIDs.rect2DFloatClass, "<init>", "(FFFF)V");
     sunFontIDs.rectF2DX =
	 (*env)->GetFieldID(env, sunFontIDs.rect2DFloatClass, "x", "F");
     sunFontIDs.rectF2DY =
	 (*env)->GetFieldID(env, sunFontIDs.rect2DFloatClass, "y", "F");
     sunFontIDs.rectF2DWidth =
	 (*env)->GetFieldID(env, sunFontIDs.rect2DFloatClass, "width", "F");
     sunFontIDs.rectF2DHeight =
	 (*env)->GetFieldID(env, sunFontIDs.rect2DFloatClass, "height", "F");

     tmpClass = (*env)->FindClass(env, "java/awt/geom/GeneralPath");
     sunFontIDs.gpClass = (jclass)(*env)->NewGlobalRef(env, tmpClass);
     sunFontIDs.gpCtr =
       (*env)->GetMethodID(env, sunFontIDs.gpClass, "<init>", "(I[BI[FI)V");

     tmpClass = (*env)->FindClass(env, "sun/font/Font2D");
     sunFontIDs.f2dCharToGlyphMID =
	 (*env)->GetMethodID(env, tmpClass, "charToGlyph", "(I)I");
     sunFontIDs.getMapperMID =
	 (*env)->GetMethodID(env, tmpClass, "getMapper",
			     "()Lsun/font/CharToGlyphMapper;");
     sunFontIDs.getTableBytesMID =
	 (*env)->GetMethodID(env, tmpClass, "getTableBytes", "(I)[B");
     sunFontIDs.canDisplayMID =
	 (*env)->GetMethodID(env, tmpClass, "canDisplay", "(C)Z");

     tmpClass = (*env)->FindClass(env, "sun/font/CharToGlyphMapper");    
     sunFontIDs.charToGlyphMID =
	(*env)->GetMethodID(env, tmpClass, "charToGlyph", "(I)I");

     tmpClass = (*env)->FindClass(env, "sun/font/PhysicalStrike");
     sunFontIDs.getGlyphMetricsMID =
	 (*env)->GetMethodID(env, tmpClass, "getGlyphMetrics",
			     "(I)Ljava/awt/geom/Point2D$Float;");
     sunFontIDs.getGlyphPointMID =
	 (*env)->GetMethodID(env, tmpClass, "getGlyphPoint",
			     "(II)Ljava/awt/geom/Point2D$Float;");
     sunFontIDs.pScalerContextFID =
	 (*env)->GetFieldID(env, tmpClass, "pScalerContext", "J");

     tmpClass = (*env)->FindClass(env, "sun/font/GlyphList");
     sunFontIDs.glyphListX = (*env)->GetFieldID(env, tmpClass, "x", "F");
     sunFontIDs.glyphListY = (*env)->GetFieldID(env, tmpClass, "y", "F");
     sunFontIDs.glyphListLen = (*env)->GetFieldID(env, tmpClass, "len", "I");
     sunFontIDs.glyphImages =
         (*env)->GetFieldID(env, tmpClass, "images", "[J");
     sunFontIDs.glyphListUsePos =
         (*env)->GetFieldID(env, tmpClass, "usePositions", "Z");
     sunFontIDs.glyphListPos = 
         (*env)->GetFieldID(env, tmpClass, "positions", "[F");

     tmpClass = (*env)->FindClass(env, "sun/font/FileFont");
     sunFontIDs.pScaler = (*env)->GetFieldID(env, tmpClass, "pScaler", "J");
}

JNIEXPORT jobject JNICALL
Java_sun_font_FontManager_getFont2D(
  JNIEnv *env,
  jclass clsFM,
  jobject javaFont) {

    return (*env)->CallObjectMethod(env, javaFont, sunFontIDs.getFont2DMID);
}

JNIEXPORT void JNICALL
Java_sun_font_FontManager_setFont2D(
  JNIEnv *env,
  jclass clsFM,
  jobject javaFont,
  jobject fontHandle) {
    (*env)->SetObjectField(env, javaFont, sunFontIDs.font2DHandle, fontHandle);
}

/* A singleton null (empty) scaler object.
 * Null fields (ie a NULL t2k) can be used to identify the null scaler.
 * C/C++ Methods which take a pScaler need to check its not the null
 * scaler.
 */
T2KScalerInfo *theNullScaler = NULL;

JNIEXPORT jlong JNICALL
    Java_sun_font_FileFont_getNullScaler
    (JNIEnv *env, jclass font2D) {

    if (theNullScaler == NULL) {
	theNullScaler = (T2KScalerInfo*)malloc(sizeof(T2KScalerInfo));
	memset(theNullScaler, 0, sizeof(T2KScalerInfo));
    }
    return (jlong)(uintptr_t)theNullScaler;
}

JNIEXPORT jlong JNICALL
    Java_sun_font_Type1Font_createScaler
    (JNIEnv *env, jobject font2D, jint fileSize) {

    int errCode = 0;
    T2K_AlgStyleDescriptor    t2kDesc;
    tsiMemObject* memHandler;
    InputStream *stream;
    sfntClass *fontClass;
    T2KScalerInfo *scalerInfo = (T2KScalerInfo*)malloc(sizeof(T2KScalerInfo));
    tt_uint32 destLength = (tt_uint32)fileSize;
    unsigned char *destBuffer;
    jobject bBuffer;

    if (scalerInfo == NULL) {
	return 0L;
    }
    scalerInfo->env = env;
    scalerInfo->font2D = font2D;
    scalerInfo->pathType = CUBICPATHTYPE; /* for Type1 */
    scalerInfo->fontData = malloc(fileSize);
    scalerInfo->fontDataLength = 0;
    scalerInfo->fontDataOffset = 0;
    scalerInfo->fileSize = fileSize;
    scalerInfo->directBuffer = NULL;
    scalerInfo->layoutTables = NULL;
    
    memHandler = tsi_NewMemhandler(&errCode);

    /* T2K claims to destroys all its internal objects on hitting an error
     * so we don't (must not) free T2K objects on hitting an error code.
     */
    if (errCode) {
	t2kIfDebugMessage(errCode, "tsi_NewMemhandler failed", errCode);
	free(scalerInfo);
	return 0L;
    }

    destBuffer = (unsigned char*)scalerInfo->fontData;

    bBuffer = (*env)->NewDirectByteBuffer(env, destBuffer, fileSize);
    (*env)->CallObjectMethod(env, font2D, sunFontIDs.t1ReadBlockMID,
			     bBuffer/*, 0, fileSize*/);

    if (*destBuffer == 0x80) {  /* PFB file, remove the headers */
        destBuffer = ExtractPureT1FromPCType1(destBuffer, &destLength);
	if (destBuffer == NULL) {
	    /* Free handler explicitly in this case because we detected
	     * the error (not T2K) so T2K needs to be instructed to free
	     * its resources.
	     */
	    tsi_DeleteMemhandler(memHandler);
	    if (scalerInfo->fontData != NULL) {
		free(scalerInfo->fontData);
	    }
	    free(scalerInfo);
	    return 0L;
	}
    }

    stream = New_InputStream3(memHandler, destBuffer, destLength, &errCode);

    if (errCode) {
	t2kIfDebugMessage(errCode, "New_NonRamInputStream failed", errCode);
	free(scalerInfo);
	return 0L;
    }

    fontClass = New_sfntClassLogical(memHandler, FONT_TYPE_1,
				     0, stream, NULL, &errCode);
	  
    if (errCode) {
	t2kIfDebugMessage(errCode, "New_sfntClass failed", errCode);
	free(scalerInfo);
	return 0L;
    }

    scalerInfo->t2k = NewT2K(memHandler, fontClass, &errCode);
    return (jlong)(uintptr_t)scalerInfo;
}

T2KScalerContext *theNullScalerContext = NULL;
/* This method should be called with theNullScaler  */
JNIEXPORT jlong JNICALL
Java_sun_font_FileFontStrike_getNullScalerContext
    (JNIEnv *env, jclass strikeClass, jlong pScaler) {
    
    if (theNullScalerContext == NULL) {
	theNullScalerContext =
	    (T2KScalerContext*)malloc(sizeof(T2KScalerContext));
	theNullScalerContext->scalerInfo = (T2KScalerInfo*)pScaler;
    }
    return (jlong)(uintptr_t)theNullScalerContext;
}

JNIEXPORT jlong JNICALL
Java_sun_font_FileFontStrike_createScalerContext
    (JNIEnv *env, jobject strike, jlong pScaler, jdoubleArray matrix,
     jboolean ttFont, jboolean aa, jboolean fm,
     jboolean algoStyle, jfloat boldness, jfloat italic) {
    
    double dmat[4];
    T2KScalerContext *context =
	(T2KScalerContext*)malloc(sizeof(T2KScalerContext));
    context->scalerInfo = (T2KScalerInfo*)pScaler;

    if (context->scalerInfo == NULL ||
	context->scalerInfo->t2k == NULL) { /* bad/null scaler */
      free((void*)context);
      return (jlong)0;
    }
    context->doAA = aa;
    context->doFM = fm;
    context->greyLevel =
	(aa) ? GREY_SCALE_BITMAP_HIGH_QUALITY : BLACK_AND_WHITE_BITMAP;
    context->doAlgoStyle = algoStyle;
    if (algoStyle) {
      context->styling.StyleMetricsFunc  = tsi_SHAPET_BOLD_METRICS;
      context->styling.StyleFuncPost     = tsi_SHAPET_BoldItalic_GLYPH_Hinted;
      context->styling.params[0]         = t2kFloatToFixed(boldness);
      context->styling.params[1]         = t2kFloatToFixed(italic);
      context->styling.params[2] = 0;
      context->styling.params[3] = 0;	
    }
    (*env)->GetDoubleArrayRegion(env, matrix, 0, 4, dmat);
   
    context->t2kMatrix.t00 =  t2kFloatToFixed((float)dmat[0]);
    context->t2kMatrix.t10 = -t2kFloatToFixed((float)dmat[1]);
    context->t2kMatrix.t01 = -t2kFloatToFixed((float)dmat[2]);
    context->t2kMatrix.t11 =  t2kFloatToFixed((float)dmat[3]);

    context->t2kFlags = T2K_GRID_FIT | T2K_CODE_IS_GINDEX;
    if (ttFont) {
	context->pathType = QUADPATHTYPE;
    } else {
	context->pathType = CUBICPATHTYPE;
    }

    return (jlong)(uintptr_t)context;
}

/*
 * We need to make an up-call to Java to read the file contents, as
 * the file is managed via the font and read via NIO APIs.
 * However since T2K supplies a buffer to fill, for larger reads
 * we can wrap it with a direct buffer and then make the up-call.
 * This avoids any copying of the data.
 * As soon as the call returns we can and do drop the reference.
 * I have verified with the NIO spec lead that the pointer to the data
 * buffer will be left well alone, and that the buffer will be rapidly
 * freed since direct buffers allocated through JNI have no finalization.
 * 
 * T2K tends to read the first few hundred bytes of a file in a zillion
 * little upcalls.
 * Also I have seen cases where thousands of calls are made each reading
 * the next 2 bytes. So for smaller reads (<=1024 bytes) we fill a cache
 * and attempt to read from that. Re-using a DirectByteBuffer for this
 * purpose means that many cache hits reduce calling this function to
 * a memcpy whereas it was a system call in previous releases.
 * Also note that in t2kstrm.h I have increased the amount
 * of data to pre-cache to reduce the number of upcalls made.
 */
#define FILEDATACACHESIZE 1024

static void ReadTTFontFileFunc(void *id, tt_uint8 *destBuffer,
			       tt_int32 offset, tt_int32 numBytes) {
    int error;
    size_t count;
    T2KScalerInfo *scalerInfo = (T2KScalerInfo *) id;
    JNIEnv* env = scalerInfo->env;
    jobject bBuffer;
    int bread = 0;

    /* Large reads will bypass the cache and data copying */
    if (numBytes > FILEDATACACHESIZE) {
	bBuffer = (*env)->NewDirectByteBuffer(env, destBuffer, numBytes);
	if (bBuffer != NULL) {
	    /* Loop until the read succeeds (or EOF).
	     * This should improve robustness in the event of a problem in
	     * the I/O system. If we find that we ever end up spinning here
	     * we are going to have to do some serious work to recover.
	     * Just returning without reading the data will cause a crash.
	     */
	    while (bread == 0) {
		bread = (*env)->CallIntMethod(env, scalerInfo->font2D,
					       sunFontIDs.ttReadBlockMID,
					       bBuffer, offset, numBytes);
	    }
	    return;
	} else {
	    /* We probably hit bug bug 4845371. For reasons that
	     * are currently unclear, the call stacks after the initial
	     * createScaler call that read large amounts of data seem to
	     * be OK and can create the byte buffer above, but this code
	     * is here just in case.
	     * 4845371 is fixed now so I don't expect this code path to
	     * ever get called but its harmless to leave it here on the
	     * small chance its needed.
	     */
	    jbyteArray byteArray = (jbyteArray)
		(*env)->CallObjectMethod(env, scalerInfo->font2D,
					 sunFontIDs.ttReadBytesMID,
					 offset, numBytes);
	    (*env)->GetByteArrayRegion(env, byteArray,
				       0, numBytes, (jbyte*)destBuffer);
	    return;
	}
    } /* Do we have a cache hit? */
    else if (scalerInfo->fontDataOffset <= offset &&
	       scalerInfo->fontDataOffset+scalerInfo->fontDataLength >=
	       offset+numBytes) {
	int cacheOffset = offset - scalerInfo->fontDataOffset;

	memcpy(destBuffer, scalerInfo->fontData+(size_t)cacheOffset, numBytes);
    } else {
      /* Must fill the cache */
	scalerInfo->fontDataOffset = offset;
	scalerInfo->fontDataLength = 
	    (offset + FILEDATACACHESIZE > scalerInfo->fileSize) ?
	    scalerInfo->fileSize - offset : FILEDATACACHESIZE;
	bBuffer = scalerInfo->directBuffer;
	/* Loop until all the read succeeds (or EOF).
	 * This should improve robustness in the event of a problem in
	 * the I/O system. If we find that we ever end up spinning here
	 * we are going to have to do some serious work to recover.
	 * Just returning without reading the data will cause a crash.
	 */
	while (bread == 0) {
	    bread = (*env)->CallIntMethod(env, scalerInfo->font2D,
					  sunFontIDs.ttReadBlockMID,
					  bBuffer, offset,
					  scalerInfo->fontDataLength);
	}

	memcpy(destBuffer, scalerInfo->fontData, numBytes);
    }
}


JNIEXPORT void JNICALL
    Java_sun_font_FileFont_freeScaler
    (JNIEnv *env, jclass fileFontClass, jlong pScaler) {

    int errCode = 0;
    T2KScalerInfo *scalerInfo = (T2KScalerInfo*)pScaler;
    tsiMemObject  *mem = scalerInfo->t2k->mem;
    sfntClass     *fontClass = scalerInfo->t2k->font;
    InputStream   *stream = fontClass->in;

    scalerInfo->env = env;
    scalerInfo->font2D = NULL; /* can't be needed as the font is gc'd */
    DeleteT2K(scalerInfo->t2k, &errCode);
    t2kIfDebugMessage(errCode, "DeleteT2K failed", errCode);
    Delete_sfntClass(fontClass, &errCode);
    t2kIfDebugMessage(errCode, "Delete_sfntClass failed", errCode);
    Delete_InputStream(stream, &errCode);
    t2kIfDebugMessage(errCode, "Delete_InputStream failed", errCode);
    tsi_DeleteMemhandler(mem);
    if (scalerInfo->fontData != NULL) {
      free(scalerInfo->fontData);
    }
    if (scalerInfo->directBuffer != NULL) {
	(*env)->DeleteGlobalRef(env, scalerInfo->directBuffer);
	scalerInfo->directBuffer = NULL;
    }
    freeLayoutTableCache(scalerInfo->layoutTables);
    free(scalerInfo);
}

TTLayoutTableCache* newLayoutTableCache() {
  TTLayoutTableCache* ltc = malloc(sizeof(TTLayoutTableCache));
  if (ltc) {
    ltc->gsub = 0;
    ltc->gpos = 0;
    ltc->gdef = 0;
    ltc->mort = 0;
    ltc->gsub_len = -1;
    ltc->gpos_len = -1;
    ltc->gdef_len = -1;
    ltc->mort_len = -1;
  }
  return ltc;
}

void freeLayoutTableCache(TTLayoutTableCache* ltc) {
  if (ltc) {
    if (ltc->gsub) free(ltc->gsub);
    if (ltc->gpos) free(ltc->gpos);
    if (ltc->gdef) free(ltc->gdef);
    if (ltc->mort) free(ltc->mort);
    free(ltc);
  }
}

JNIEXPORT jlong JNICALL
    Java_sun_font_TrueTypeFont_createScaler
    (JNIEnv *env, jobject font2D, jint fileSize, jint fontNumber) {

    int errCode = 0;
    tsiMemObject* memHandler;
    InputStream *stream;
    sfntClass *fontClass;
 
    T2KScalerInfo *scalerInfo = (T2KScalerInfo*)malloc(sizeof(T2KScalerInfo));

    if (scalerInfo == NULL) {
	return 0L;
    }
    scalerInfo->env = env;
    scalerInfo->font2D = font2D;
    scalerInfo->pathType = QUADPATHTYPE; /* for TrueType */
    scalerInfo->fontData = (tt_uint8*)malloc(FILEDATACACHESIZE);
    scalerInfo->fontDataOffset = 0;
    scalerInfo->fontDataLength = 0;
    scalerInfo->fileSize = fileSize;
    scalerInfo->directBuffer =
	(*env)->NewDirectByteBuffer(env,
				    scalerInfo->fontData, FILEDATACACHESIZE);
    scalerInfo->directBuffer =
	(*env)->NewGlobalRef(env, scalerInfo->directBuffer);
    scalerInfo->layoutTables = NULL;

    memHandler = tsi_NewMemhandler(&errCode);

    /* T2K claims to destroys all its internal objects on hitting an error
     * so we don't (must not) free T2K objects on hitting an error code.
     */
    if (errCode) {
	t2kIfDebugMessage(errCode, "tsi_NewMemhandler failed", errCode);
	// nothing to dispose in layout tables
	(*env)->DeleteGlobalRef(env, scalerInfo->directBuffer);
	free(scalerInfo->fontData);
	free(scalerInfo);
	return 0L;
    }

    stream = New_NonRamInputStream(memHandler, (void *)scalerInfo,
						ReadTTFontFileFunc, fileSize,
						&errCode);

    if (errCode) {
	t2kIfDebugMessage(errCode, "New_NonRamInputStream failed", errCode);
	(*env)->DeleteGlobalRef(env, scalerInfo->directBuffer);
	free(scalerInfo->fontData);
	free(scalerInfo);
	return 0L;
    }

    /* T2K supports algorithmic styling of TrueType fonts.
     * Unfortunately T2K doesn't come with a public call to set up styling
     * except at scaler instance creation time.
     * Since we want to share a single T2K scaler per font resource (which
     * usually means a file) we have exposed an internal T2K API to set that
     * up as part of the context along with the transform. So what is passed
     * in here is always NULL (the final arg before errCode).
     */	
    fontClass = New_sfntClassLogical(memHandler, FONT_TYPE_TT_OR_T2K,
				     fontNumber, stream,
				     NULL, &errCode);
	  
    if (errCode) {
	t2kIfDebugMessage(errCode, "New_sfntClass failed", errCode);
	(*env)->DeleteGlobalRef(env, scalerInfo->directBuffer);
	free(scalerInfo->fontData);
	free(scalerInfo);
	return 0L;
    }

    scalerInfo->t2k = NewT2K(memHandler, fontClass, &errCode);

    if (errCode) {
	t2kIfDebugMessage(errCode, "NewT2K failed", errCode);
	(*env)->DeleteGlobalRef(env, scalerInfo->directBuffer);
	free(scalerInfo->fontData);
	free(scalerInfo);
	return 0L;
    }
    return (jlong)(uintptr_t)scalerInfo;
}

#define T2KByteToAlpha255(value) (((value) << 4) + (value) >> 3)

static void CopyBW2Grey8(const void* srcImage, int srcRowBytes,
                         void* dstImage, int dstRowBytes,
                         int width, int height)
{
    const UInt8* srcRow = (UInt8*)srcImage;
    UInt8* dstRow = (UInt8*)dstImage;
    int wholeByteCount = width >> 3;
    int remainingBitsCount = width & 7;
    int i, j;

    while (height--) {
        const UInt8* src8 = srcRow;
        UInt8* dstByte = dstRow;
        unsigned srcValue;

        srcRow += srcRowBytes;
        dstRow += dstRowBytes;

        for (i = 0; i < wholeByteCount; i++) {
            srcValue = *src8++;
            for (j = 0; j < 8; j++) {
                *dstByte++ = (srcValue & 0x80) ? 0xFF : 0;
                srcValue <<= 1;
            }
        }
        if (remainingBitsCount) {
            srcValue = *src8;
            for (j = 0; j < remainingBitsCount; j++) {
                *dstByte++ = (srcValue & 0x80) ? 0xFF : 0;
                srcValue <<= 1;
            }
        }
    }
}


int setupT2KContext(JNIEnv *env, jobject font2D, 
		    T2KScalerInfo *scalerInfo, T2KScalerContext *context,
		    jboolean sbits) {
    int errCode = 0;
    T2K_TRANS_MATRIX t2kMatrix;
    T2K *t2k = scalerInfo->t2k;

    scalerInfo->env = env;
    scalerInfo->font2D = font2D;

    if (context->doAlgoStyle) {
	t2k_SetStyling(t2k->font, &context->styling);
    } else {
	t2k_SetStyling(t2k->font, NULL);
    }

    /* T2K_NewTransformation updates the matrix parameter(!) so pass in a copy
     */
    t2kMatrix.t00 = context->t2kMatrix.t00;
    t2kMatrix.t01 = context->t2kMatrix.t01;
    t2kMatrix.t10 = context->t2kMatrix.t10;
    t2kMatrix.t11 = context->t2kMatrix.t11; 
    T2K_NewTransformation(t2k, true, 72, 72, &t2kMatrix, sbits, &errCode);

    return errCode;
}

JNIEXPORT jfloat JNICALL
Java_sun_font_FileFont_getGlyphAdvance
    (JNIEnv *env, jobject font2D, jlong pScalerContext, jint glyphCode) {
  
    int errCode = 0;
    T2KScalerContext *context = (T2KScalerContext*)pScalerContext;
    T2KScalerInfo *scalerInfo = context->scalerInfo;
    T2K *t2k = scalerInfo->t2k;
    jboolean sbits = !((context->doAlgoStyle && context->styling.params[1] != 0L) 
                         || context->doAA || context->doFM);
    UInt8 renderFlags = context->t2kFlags | T2K_SCAN_CONVERT |T2K_SKIP_SCAN_BM;
    int fAdvanceX;

    if (scalerInfo == theNullScaler || context == theNullScalerContext) {
	return (jfloat)0;
    }

    if (glyphCode >= INVISIBLE_GLYPHS) {
	return (jfloat)0;
    }
    errCode = setupT2KContext(env, font2D, scalerInfo, context, sbits);
    if (errCode) {
	return (jfloat)0;
    }

    T2K_RenderGlyph(t2k, glyphCode, 0, 0, context->greyLevel,
                    renderFlags, &errCode);
 
   if (errCode) {
        return (jfloat)0;
    }

    if (context->doFM) {
	fAdvanceX = t2k->xLinearAdvanceWidth16Dot16;
    } else {
	/* rounding advances in both x and y causes falloff from the baseline
	 * so in this case we don't round. Rounding the magnitude of the
	 * advance vector really has no point to it -- caller can round 
	 * resulting positions if desired.
	 */
	if (!t2k->yAdvanceWidth16Dot16) {
	    fAdvanceX = (t2kFixedRound ( t2k->xAdvanceWidth16Dot16 )) << 16;

	} else if (!t2k->xAdvanceWidth16Dot16) {
	    fAdvanceX = 0;

	} else {
	    fAdvanceX = t2k->xAdvanceWidth16Dot16;
	}
    }

    T2K_PurgeMemory(t2k, 1, &errCode);  // to relase the bitmap/outline
    t2kIfDebugMessage(errCode, "T2K_PurgeMemory failed", errCode);
 
    /* Note this value is in device space. The caller needs to convert
     * it into user space
     */
    return (jfloat)t2kFixedToFloat(fAdvanceX);
}

JNIEXPORT void JNICALL
Java_sun_font_FileFont_getGlyphMetrics
   (JNIEnv *env, jobject font2D, jlong pScalerContext,
    jint glyphCode, jobject metricsPt) {
  
    int errCode = 0;
    T2KScalerContext *context = (T2KScalerContext*)pScalerContext;
    T2KScalerInfo *scalerInfo = context->scalerInfo;
    T2K *t2k = scalerInfo->t2k;
    jboolean sbits = !((context->doAlgoStyle && context->styling.params[1] != 0L) 
                       || context->doAA || context->doFM);
    UInt8 renderFlags = context->t2kFlags | T2K_SCAN_CONVERT |T2K_SKIP_SCAN_BM;
    int fAdvanceX, fAdvanceY;

    if (metricsPt == NULL) {
	return;
    }

    if (scalerInfo == theNullScaler || context == theNullScalerContext ||
	glyphCode >= INVISIBLE_GLYPHS ||
	setupT2KContext(env, font2D, scalerInfo, context, sbits) !=0) {
	(*env)->SetFloatField(env, metricsPt, sunFontIDs.xFID, (jfloat)0);
	(*env)->SetFloatField(env, metricsPt, sunFontIDs.yFID, (jfloat)0);
	return;
    }


    T2K_RenderGlyph(t2k, glyphCode, 0, 0, context->greyLevel,
                    renderFlags, &errCode);
    if (errCode) {
	(*env)->SetFloatField(env, metricsPt, sunFontIDs.xFID, (jfloat)0);
	(*env)->SetFloatField(env, metricsPt, sunFontIDs.yFID, (jfloat)0);
        return;
    }

    if (context->doFM) {
	fAdvanceX =  t2k->xLinearAdvanceWidth16Dot16;
	fAdvanceY = -t2k->yLinearAdvanceWidth16Dot16;
    } else {
	/* rounding advances in both x and y causes falloff from the baseline
	 * so in this case we don't round. Rounding the magnitude of the
	 * advance vector really has no point to it -- caller can round 
	 * resulting positions if desired.
	 */
	if (!t2k->yAdvanceWidth16Dot16) {
	    fAdvanceX =  (t2kFixedRound(t2k->xAdvanceWidth16Dot16)) << 16;
	    fAdvanceY = 0;
	} else if (!t2k->xAdvanceWidth16Dot16) {
	    fAdvanceX = 0;
	    fAdvanceY = -(t2kFixedRound(t2k->yAdvanceWidth16Dot16)) << 16;
	} else {
	    fAdvanceX =  t2k->xAdvanceWidth16Dot16;
	    fAdvanceY = -t2k->yAdvanceWidth16Dot16;
	}
	
    }

    T2K_PurgeMemory(t2k, 1, &errCode);  // to relase the bitmap/outline
    t2kIfDebugMessage(errCode, "T2K_PurgeMemory failed", errCode);

    (*env)->SetFloatField(env, metricsPt, sunFontIDs.xFID,
			  (jfloat)t2kFixedToFloat(fAdvanceX));
    (*env)->SetFloatField(env, metricsPt, sunFontIDs.yFID,
			  (jfloat)t2kFixedToFloat(fAdvanceY));
}

static jlong getNullGlyphImage() {
    GlyphInfo *glyphInfo =  (GlyphInfo*)malloc(sizeof(GlyphInfo));
    memset(glyphInfo, 0, sizeof(GlyphInfo));
    return (jlong)(uintptr_t)glyphInfo;
}

/*
 * This function retrieves metrics and image for a glyphcode/strike.
 * The glyph image data (pixels) is stored contiguously which reduces
 * malloc/free overhead.
 * The metrics data is in device space. Java clients which want to
 * extract this data will need to convert it to user space.
 */
JNIEXPORT jlong JNICALL
Java_sun_font_FileFont_getGlyphImage
    (JNIEnv *env, jobject font2D, jlong pScalerContext, jint glyphCode) {
  
    GlyphInfo *glyphInfo;
    T2KScalerContext *context = (T2KScalerContext*)pScalerContext;
    T2KScalerInfo *scalerInfo = context->scalerInfo;
    T2K *t2k = scalerInfo->t2k;
    jboolean sbits = !((context->doAlgoStyle && context->styling.params[1] != 0L) 
                         || context->doAA || context->doFM);
    UInt8 renderFlags = context->t2kFlags | T2K_SCAN_CONVERT;
    short width, height;
    int imageSize;
    int ii;
    int errCode;

    if (scalerInfo == theNullScaler || context == theNullScalerContext) {
	return getNullGlyphImage();
    }

    errCode = setupT2KContext(env, font2D, scalerInfo, context, sbits);

    if (errCode) {
	return getNullGlyphImage();
    }

    T2K_RenderGlyph(t2k, glyphCode, 0, 0, context->greyLevel,
                    renderFlags, &errCode);
    if (errCode) {
        return getNullGlyphImage();
    }

    width = (UInt16)t2k->width;
    height = (UInt16)t2k->height;
    imageSize = width*height;
    glyphInfo = (GlyphInfo*)malloc(sizeof(GlyphInfo)+imageSize);
    glyphInfo->cellInfo = NULL;
    glyphInfo->width     = width;
    glyphInfo->height    = height;
    glyphInfo->topLeftX = t2kFixedToFloat(F26Dot6ToFixed(t2k->fLeft26Dot6));
    glyphInfo->topLeftY = -t2kFixedToFloat(F26Dot6ToFixed(t2k->fTop26Dot6));
    if (context->doFM) {
	glyphInfo->advanceX = t2kFixedToFloat(t2k->xLinearAdvanceWidth16Dot16);
	glyphInfo->advanceY =-t2kFixedToFloat(t2k->yLinearAdvanceWidth16Dot16);
    } else {
	/* rounding advances in both x and y causes falloff from the baseline
	 * so in this case we don't round. Rounding the magnitude of the
	 * advance vector really has no point to it -- caller can round 
	 * resulting positions if desired.
	 */
	if (!t2k->yAdvanceWidth16Dot16) {
	    glyphInfo->advanceX = t2kFixedToFloat
		((t2kFixedRound(t2k->xAdvanceWidth16Dot16 )) << 16);
	    glyphInfo->advanceY = 0;
	} else if (!t2k->xAdvanceWidth16Dot16) {
	    glyphInfo->advanceX = 0;
	    glyphInfo->advanceY = -t2kFixedToFloat
		((t2kFixedRound ( t2k->yAdvanceWidth16Dot16 )) << 16);
	} else {
	    glyphInfo->advanceX =  t2kFixedToFloat(t2k->xAdvanceWidth16Dot16);
	    glyphInfo->advanceY = -t2kFixedToFloat(t2k->yAdvanceWidth16Dot16);
	}
    }

    /* Now need to retrieve the glyph so can store the image data.
     * The image data is stored contiguously with the info structure as its
     * memory is allocated in the same block
     */
    if (imageSize == 0) {
	glyphInfo->image = NULL;
    } else {
	glyphInfo->image = (unsigned char*)glyphInfo+sizeof(GlyphInfo);
	if (context->greyLevel == BLACK_AND_WHITE_BITMAP) {
	    CopyBW2Grey8(t2k->baseAddr, t2k->rowBytes,
			 (void *)glyphInfo->image, width, width, height);
	} else {
	    UInt8* dstRow = (UInt8*)glyphInfo->image;
	    const UInt8* srcRow = (UInt8*)t2k->baseAddr;
	    int x,y;
	    for (y = 0; y < glyphInfo->height; y++) {
		for (x = 0; x < width; x++) {
		    dstRow[x] = T2KByteToAlpha255(srcRow[x]);
		}
		dstRow += width;
		srcRow += t2k->rowBytes;
	    }
	}
    }

    T2K_PurgeMemory(t2k, 1, &errCode);  // to release the bitmap/outline
    t2kIfDebugMessage(errCode, "T2K_PurgeMemory failed", errCode);
 
    return (jlong)(uintptr_t)glyphInfo;
}


/* This native method is called by the OpenType layout engine.
 */
JNIEXPORT jobject JNICALL
Java_sun_font_TrueTypeFont_getGlyphPoint
    (JNIEnv *env, jobject font2D, jlong pScalerContext,
     jint glyphCode, jint pointNumber) {

    jobject point = NULL;
    T2KScalerContext *context = (T2KScalerContext*)pScalerContext;
    T2KScalerInfo *scalerInfo = context->scalerInfo;
    T2K *t2k = scalerInfo->t2k;
    jboolean sbits = false;
    UInt8 renderFlags = context->t2kFlags | 
	T2K_SCAN_CONVERT | T2K_RETURN_OUTLINES;
    int errCode;

    if (scalerInfo == theNullScaler || context == theNullScalerContext) {
	return (*env)->NewObject(env, sunFontIDs.pt2DFloatClass,
				      sunFontIDs.pt2DFloatCtr, 0, 0);
    }

    errCode = setupT2KContext(env, font2D, scalerInfo, context, sbits);

    if (errCode) {
	return (*env)->NewObject(env, sunFontIDs.pt2DFloatClass,
				 sunFontIDs.pt2DFloatCtr, 0, 0);
    }

    T2K_RenderGlyph(t2k, glyphCode, 0, 0, context->greyLevel, renderFlags, 
		    &errCode);
    t2kIfDebugMessage(errCode, "T2K_RenderGlyph failed", errCode);

    if (!t2k->embeddedBitmapWasUsed) {
        if (pointNumber < t2k->glyph->pointCount) {
	    /* Convert from T2K's 26.6 format (64 == 2^6) */
	    float x = (float)(t2k->glyph->x[pointNumber] / 64.0 );
	    /* convert to java's "+y is down" coordinate system */
	    float y = -(float)(t2k->glyph->y[pointNumber] / 64.0 );

	    point = (*env)->NewObject(env, sunFontIDs.pt2DFloatClass,
				      sunFontIDs.pt2DFloatCtr, x, y);
        }
    }
    T2K_PurgeMemory(t2k, 1, &errCode);  // to release the bitmap/outline
    t2kIfDebugMessage(errCode, "T2K_PurgeMemory failed", errCode);
    return point;
}

/* This function is called by the OpenType layout engine.
 */
int getUnitsPerEmForLayout(T2KScalerInfo *scalerInfo) {

    int upem = 2048;
    T2K *t2k;

    if (scalerInfo != NULL && scalerInfo != theNullScaler) {
	t2k = scalerInfo->t2k;
	if (t2k->font && t2k->font->head) {
	    upem = t2k->font->head->unitsPerEm;
	}
    }
    return upem;
}

static void ProjectUnitVector(hsFixed projX, hsFixed projY,
                              hsFract dirX, hsFract dirY,
                              hsFract baseX, hsFract baseY,
                              hsFixed *fX, hsFixed *fY)
{
    hsFixed dist = t2kFracMul(projX, baseY) - t2kFracMul(projY, baseX);
    hsFixed scale = t2kFracDiv(dist,
			       t2kFracMul(dirX, baseY) -
			       t2kFracMul(dirY, baseX));
    *fX = t2kFracMul(dirX, scale);
    *fY = t2kFracMul(dirY, scale);
}

JNIEXPORT jobject JNICALL
Java_sun_font_FileFont_getFontMetrics
    (JNIEnv *env, jobject font2D, jlong pScalerContext) {
 
    T2KScalerContext *context = (T2KScalerContext*)pScalerContext;
    T2KScalerInfo *scalerInfo = context->scalerInfo;
    T2K *t2k = scalerInfo->t2k;
    jboolean sbits = !((context->doAlgoStyle && context->styling.params[1] != 0L)
                       || context->doAA || context->doFM);
    hsFixed mag, fX, fY;
    hsFract caretX, caretY, baseX, baseY;
    jobject metrics;
    jfloat ax, ay, dx, dy, bx, by, lx, ly, mx, my;
    jfloat f0 = 0.0;

    int errCode;

    if (scalerInfo == theNullScaler || context == theNullScalerContext) {
	return (*env)->NewObject(env, sunFontIDs.strikeMetricsClass,
				sunFontIDs.strikeMetricsCtr,
				f0,f0,f0,f0,f0,f0,f0,f0,f0,f0);
    }

    errCode = setupT2KContext(env, font2D, scalerInfo, context, sbits);

    if (errCode) {
	return (*env)->NewObject(env, sunFontIDs.strikeMetricsClass,
				 sunFontIDs.strikeMetricsCtr,
				 f0,f0,f0,f0,f0,f0,f0,f0,f0,f0);
    }

    mag = t2kMagnitude(t2k->caretDx, t2k->caretDy);

    caretX = t2kFracDiv(t2k->caretDx, mag);
    caretY = t2kFracDiv(t2k->caretDy, mag);

    mag = t2kMagnitude(t2k->xMaxLinearAdvanceWidth,
		       t2k->yMaxLinearAdvanceWidth);

    baseX = t2kFracDiv(t2k->xMaxLinearAdvanceWidth, mag);
    baseY = t2kFracDiv(t2k->yMaxLinearAdvanceWidth, mag);

    /* ascent */
    ProjectUnitVector(t2k->xAscender + (t2k->xLineGap >> 1),
		      - t2k->yAscender - (t2k->yLineGap >> 1),
		      caretX, - caretY, baseX, - baseY, &fX, &fY);
    ax = (jfloat)t2kFixedToFloat(fX);
    ay = (jfloat)t2kFixedToFloat(fY);

    /* descent */
    ProjectUnitVector(t2k->xDescender + (t2k->xLineGap >> 1),
		      - t2k->yDescender - (t2k->yLineGap >> 1),
		      - caretX, caretY, baseX, - baseY, &fX, &fY);
    dx = (jfloat)t2kFixedToFloat(fX);
    dy = (jfloat)t2kFixedToFloat(fY);

    /* baseline */
    bx = (jfloat)t2kFixedToFloat(baseX >> 16);
    by = (jfloat)t2kFixedToFloat(baseY >> 16);

    /* leading */
    ProjectUnitVector(t2k->xLineGap,
		      - t2k->yLineGap,
		      - caretX, caretY, baseX, - baseY, &fX, &fY);
    lx = (jfloat)-t2kFixedToFloat(fX);
    ly = (jfloat)-t2kFixedToFloat(fY);

    /* max advance */
    mx = (jfloat)t2kFixedToFloat(t2k->xMaxLinearAdvanceWidth);
    my = (jfloat)t2kFixedToFloat(t2k->yMaxLinearAdvanceWidth);
    
    metrics = (*env)->NewObject(env, sunFontIDs.strikeMetricsClass,
				sunFontIDs.strikeMetricsCtr,
				ax, ay, dx, dy, bx, by, lx, ly, mx, my);
/*     printf("ax=%f ay=%f dx=%f dy=%f lx=%f ly=%f\n",ax,ay,dx,dy,lx,ly); */

    return metrics;
}

JNIEXPORT void JNICALL
Java_sun_font_StrikeCache_getGlyphCacheDescription
  (JNIEnv *env, jclass cls, jlongArray results) {

    jlong* nresults;
    GlyphInfo *info;
    size_t baseAddr;

    if ((*env)->GetArrayLength(env, results) < 10) {
	return;
    }
 
    nresults = (jlong*)(*env)->GetPrimitiveArrayCritical(env, results, NULL);
    if (nresults == NULL) {
	return;
    }
    info = (GlyphInfo*)malloc(sizeof(GlyphInfo));
    baseAddr = (size_t)info;
    memset(info, 0, sizeof(GlyphInfo));
    nresults[0] = sizeof(void*);
    nresults[1] = sizeof(GlyphInfo);
    nresults[2] = 0;
    nresults[3] = (size_t)&(info->advanceY)-baseAddr;
    nresults[4] = (size_t)&(info->width)-baseAddr;
    nresults[5] = (size_t)&(info->height)-baseAddr;
    nresults[6] = (size_t)&(info->topLeftX)-baseAddr;
    nresults[7] = (size_t)&(info->topLeftY)-baseAddr;
    nresults[8] = (size_t)&(info->image)-baseAddr;
    nresults[9] = (jlong)(uintptr_t)info; /* invisible glyph */
    (*env)->ReleasePrimitiveArrayCritical(env, results, nresults, 0);
}

/*
 * Class:     sun_font_StrikeCache
 * Method:    freeIntPointer
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_sun_font_StrikeCache_freeIntPointer
    (JNIEnv *env, jclass cacheClass, jint ptr) {

    /* Note this is used for freeing a glyph which was allocated
     * but never placed into the glyph cache. The caller holds the
     * only reference, therefore it is unnecessary to invalidate any
     * accelerated glyph cache cells as we do in freeInt/LongMemory().
     */
    if (ptr != 0) {
	free((void*)ptr);
    }
}

/*
 * Class:     sun_font_StrikeCache
 * Method:    freeLongPointer
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_sun_font_StrikeCache_freeLongPointer
    (JNIEnv *env, jclass cacheClass, jlong ptr) {

    /* Note this is used for freeing a glyph which was allocated
     * but never placed into the glyph cache. The caller holds the
     * only reference, therefore it is unnecessary to invalidate any
     * accelerated glyph cache cells as we do in freeInt/LongMemory().
     */
    if (ptr != 0L) {
	free((void*)ptr);
    }
}

/*
 * Class:     sun_font_StrikeCache
 * Method:    freeIntMemory
 * Signature: ([I)V
 */
JNIEXPORT void JNICALL Java_sun_font_StrikeCache_freeIntMemory
    (JNIEnv *env, jclass cacheClass, jintArray jmemArray, jlong pContext) {

    int len = (*env)->GetArrayLength(env, jmemArray);
    jint* ptrs =
	(jint*)(*env)->GetPrimitiveArrayCritical(env, jmemArray, NULL);
    int i;

    if (ptrs) {
	for (i=0; i< len; i++) {
	    if (ptrs[i] != 0) {
                GlyphInfo *ginfo = (GlyphInfo *)ptrs[i];
                if (ginfo->cellInfo != NULL) {
                    // invalidate this glyph's accelerated cache cell
                    ginfo->cellInfo->glyphInfo = NULL;
                }
		free((void*)ginfo);
	    }
	}
	(*env)->ReleasePrimitiveArrayCritical(env, jmemArray, ptrs, JNI_ABORT);
    }
    if (pContext !=0) {
	free((void*)pContext);
    }
}

/*
 * Class:     sun_font_StrikeCache
 * Method:    freeLongMemory
 * Signature: ([J)V
 */
JNIEXPORT void JNICALL Java_sun_font_StrikeCache_freeLongMemory
    (JNIEnv *env, jclass cacheClass, jlongArray jmemArray, jlong pContext) {

    int len = (*env)->GetArrayLength(env, jmemArray);
    jlong* ptrs =
	(jlong*)(*env)->GetPrimitiveArrayCritical(env, jmemArray, NULL);
    int i;

    if (ptrs) {
	for (i=0; i< len; i++) {
	    if (ptrs[i] != 0L) {
                GlyphInfo *ginfo = (GlyphInfo *)ptrs[i];
                if (ginfo->cellInfo != NULL) {
                    // invalidate this glyph's accelerated cache cell
                    ginfo->cellInfo->glyphInfo = NULL;
                }
		free((void*)ginfo);
	    }
	}
	(*env)->ReleasePrimitiveArrayCritical(env, jmemArray, ptrs, JNI_ABORT);
    }
    if (pContext !=0) {
	free((void*)pContext);
    }
}

JNIEXPORT jint JNICALL
Java_sun_font_Type1Font_getNumGlyphs
(JNIEnv *env, jobject t1font, jlong pScaler) {

    T2KScalerInfo *scalerInfo = (T2KScalerInfo*)pScaler;
    T2K *t2k = scalerInfo->t2k;

    if (t2k == NULL) { /* bad/null scaler */
       return (jint)0;
    }

    return (jint)GetNumGlyphs_sfntClass(t2k->font);
}


JNIEXPORT jint JNICALL
Java_sun_font_Type1Font_getMissingGlyphCode
(JNIEnv *env, jobject t1font, jlong pScaler) {

    T2KScalerInfo *scalerInfo = (T2KScalerInfo*)pScaler;
    T2K *t2k = scalerInfo->t2k;

    if (t2k == NULL) { /* bad/null scaler */
	return (jint)0;
    }

    return (jint)t2k->font->T1->notdefGlyphIndex;
}

JNIEXPORT jint JNICALL
Java_sun_font_Type1Font_getGlyphCode
(JNIEnv *env, jobject t1font, jlong pScaler, jchar charCode) {

    T2KScalerInfo *scalerInfo = (T2KScalerInfo*)pScaler;
    T2K *t2k = scalerInfo->t2k;

    if (t2k == NULL) { /* bad/null scaler */
	return (jint)0;
    }

    return (jint)T2K_GetGlyphIndex(t2k, charCode);
}
