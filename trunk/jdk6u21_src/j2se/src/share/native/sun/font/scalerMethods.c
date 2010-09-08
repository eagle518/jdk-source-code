/*
 * @(#)scalerMethods.c	1.39 03/23/10
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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

extern void AccelGlyphCache_RemoveAllCellInfos(GlyphInfo *glyph);

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

/* LCD text options */
static int lcdscale = 6; // required to be a multiple of 3
void initLCDGammaTables();

FontManagerNativeIDs sunFontIDs;

/* experimental: disabling hinting */
static int isHintingDisabled = 0;
 
JNIEXPORT void JNICALL
Java_sun_font_FontManager_initIDs
    (JNIEnv *env, jclass cls, jboolean disableHints) {
         
     jclass tmpClass = (*env)->FindClass(env, "java/awt/Font");
     sunFontIDs.getFont2DMID =
	 (*env)->GetMethodID(env, tmpClass, "getFont2D",
			     "()Lsun/font/Font2D;");
     sunFontIDs.font2DHandle =
       (*env)->GetFieldID(env, tmpClass,
			  "font2DHandle", "Lsun/font/Font2DHandle;");

     sunFontIDs.createdFont =
       (*env)->GetFieldID(env, tmpClass, "createdFont", "Z");

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
     sunFontIDs.adjustPointMID =
         (*env)->GetMethodID(env, tmpClass, "adjustPoint",
                             "(Ljava/awt/geom/Point2D$Float;)V");
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
     sunFontIDs.lcdRGBOrder = 
         (*env)->GetFieldID(env, tmpClass, "lcdRGBOrder", "Z");
     sunFontIDs.lcdSubPixPos = 
         (*env)->GetFieldID(env, tmpClass, "lcdSubPixPos", "Z");

     tmpClass = (*env)->FindClass(env, "sun/font/FileFont");
     sunFontIDs.pScaler = (*env)->GetFieldID(env, tmpClass, "pScaler", "J");

     initLCDGammaTables();

     if (disableHints == JNI_TRUE || getenv("JAVA2D_EXPERIMENTAL_NOGRIDFITTING")) {
         isHintingDisabled = 1;
     }
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

JNIEXPORT void JNICALL
Java_sun_font_FontManager_setCreatedFont(
  JNIEnv *env,
  jclass clsFM,
  jobject javaFont) {
    (*env)->SetBooleanField(env, javaFont, sunFontIDs.createdFont, JNI_TRUE);
}

JNIEXPORT jboolean JNICALL
Java_sun_font_FontManager_isCreatedFont(
  JNIEnv *env,
  jclass clsFM,
  jobject javaFont) {
    return (*env)->GetBooleanField(env, javaFont, sunFontIDs.createdFont);
}

/* A singleton null (empty) scaler object.
 * Null fields (ie a NULL t2k) can be used to identify the null scaler.
 * C/C++ Methods which take a pScaler need to check its not the null
 * scaler.
 */
T2KScalerInfo *theNullScaler = NULL;

static T2KScalerInfo* getNullScaler() {
    if (theNullScaler == NULL) {
	theNullScaler = (T2KScalerInfo*)malloc(sizeof(T2KScalerInfo));
	memset(theNullScaler, 0, sizeof(T2KScalerInfo));
    }
    return theNullScaler;
}

/* If a scaler is bad and has been 'nulled out' then memHandler
 * or T2K will be null.
 */
int isNullScaler(T2KScalerInfo *scaler) {
    return
        scaler == NULL ||
        scaler->memHandler == NULL ||
        scaler == getNullScaler();
}

JNIEXPORT jlong JNICALL
    Java_sun_font_FileFont_getNullScaler
    (JNIEnv *env, jclass font2D) {

    return (jlong)(uintptr_t)getNullScaler();
}

JNIEXPORT jlong JNICALL
    Java_sun_font_Type1Font_createScaler
    (JNIEnv *env, jobject font2D, jint fileSize) {

    int errCode = 0;
    tsiMemObject* memHandler;
    InputStream *stream;
    sfntClass *fontClass;
    T2KScalerInfo *scalerInfo =
      (T2KScalerInfo*)calloc(1, sizeof(T2KScalerInfo));
    tt_uint32 destLength = (tt_uint32)fileSize;
    unsigned char *destBuffer;
    jobject bBuffer;

    if (scalerInfo == NULL) {
	return 0L;
    }
    scalerInfo->env = env;
    scalerInfo->font2D = font2D;
    scalerInfo->pathType = CUBICPATHTYPE; /* for Type1 */
    scalerInfo->supportsCJK = JNI_FALSE;  /* used only by TrueType */
    scalerInfo->fontData = malloc(fileSize);
    scalerInfo->fontDataLength = 0;
    scalerInfo->fontDataOffset = 0;
    scalerInfo->fileSize = fileSize;
    scalerInfo->directBuffer = NULL;
    scalerInfo->layoutTables = NULL;
    scalerInfo->bwGlyphCnt = 0;
    scalerInfo->bwGlyphs = NULL;
    
    memHandler = tsi_NewMemhandler(&errCode);

    /* T2K claims to destroys all its internal objects on hitting an error
     * so we don't (must not) free T2K objects on hitting an error code.
     */
    if (errCode) {
	t2kIfDebugMessage(errCode, "tsi_NewMemhandler failed", errCode);
	free(scalerInfo);
	return 0L;
    }
    scalerInfo->memHandler = memHandler;

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

/* Hinting with non-identity transform may cause glyph to look distorted
   because hinting logic from the font itself was not designed for this case.

   This is ok and we unlikely can do anything to improve this in general case.
   However, sometimes transform is not identity due to precision loss doing
   math computations. In these cases scale factors are almost identical but this
   tiny difference will increase as they will be converted to fixed point.

   To compensate for this we are explicitly making them identical if they are
   very close. Average value is used to be as close to true value as possible.

   Delta is 1/65536 because fixed point math uses 16 bit precision at most.
   In fact it is likely to be safe to use fewer bits because lots of code uses
   6 bit for fractional part anyway. However, 1/65536 works for now.
*/
#define NDELTA ( (1 / (double)65536))
#define ALMOST_THE_SAME(x, y) (((x) != (y)) && (fabs((y) - (x)) <= NDELTA))
#define NORM(x, y) (((x) + (y))/2)

JNIEXPORT jlong JNICALL
Java_sun_font_FileFontStrike_createScalerContext
    (JNIEnv *env, jobject strike, jlong pScaler, jdoubleArray matrix,
     jboolean ttFont, jint aa, jint fm,
     jboolean algoStyle, jfloat boldness, jfloat italic,
     jboolean disableHinting) {
    
    double dmat[4];
    T2KScalerContext *context =
	(T2KScalerContext*)malloc(sizeof(T2KScalerContext));
    context->scalerInfo = (T2KScalerInfo*)pScaler;

    if (context->scalerInfo == NULL ||
	context->scalerInfo->t2k == NULL) { /* bad/null scaler */
      free((void*)context);
      return (jlong)0;
    }

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

    /* workaround for almost identity transforms.
       see comment above macros definition */
    if (ALMOST_THE_SAME(dmat[0], dmat[3])) {
      context->t2kMatrix.t00 =  t2kFloatToFixed((float) NORM(dmat[0], dmat[3]));
      context->t2kMatrix.t11 =  context->t2kMatrix.t00;
    } else {
      context->t2kMatrix.t00 =  t2kFloatToFixed((float) dmat[0]);
      context->t2kMatrix.t11 =  t2kFloatToFixed((float) dmat[3]);
    }
    if (ALMOST_THE_SAME(dmat[1], dmat[2])) {
      context->t2kMatrix.t10 = -t2kFloatToFixed((float) NORM(dmat[1], dmat[2]));
      context->t2kMatrix.t01 = context->t2kMatrix.t10;
    } else {
      context->t2kMatrix.t10 = -t2kFloatToFixed((float) dmat[1]);
      context->t2kMatrix.t01 = -t2kFloatToFixed((float) dmat[2]);
    }

    context->doAA = aa != TEXT_AA_OFF;
    context->doFM = fm != TEXT_FM_OFF;
    context->aaType = aa;
    context->fmType = fm;

    /* Below, if FM is "ON" then we disable (do not request) sbits.
     * sbits retrieves embedded bitmaps from a TrueType font. These are
     * rare except for CJK fonts in which case they help quality a great deal.
     * This means that in order to get the linearly scaled metrics from
     * we want from the htx table, we also end up adversely affecting the
     * image quality when really it should not change at all.
     * This is very noticeable for CJK glyphs.
     * This needs to be fixed to be able to get linear metrics along with
     * embedded bitmaps.
     */
    context->sbits = !(algoStyle && italic != 0) &&
      !context->doFM && (context->aaType != TEXT_AA_ON);

    context->greyLevel = BLACK_AND_WHITE_BITMAP;
    if (context->aaType == TEXT_AA_ON) {
        if (dmat[1] != 0 || dmat[2] != 0) {
            context->greyLevel = GREY_SCALE_BITMAP_LOW_QUALITY;
        } else {
            context->greyLevel = GREY_SCALE_BITMAP_HIGH_QUALITY; 
        }
    }

    context->t2kFlags = T2K_GRID_FIT | T2K_CODE_IS_GINDEX;

    if (isHintingDisabled || /* global setting */
        disableHinting || /* per strike setting */
       (context->doFM && context->aaType==TEXT_AA_ON))
    {
      context->t2kFlags &= ~T2K_GRID_FIT;
    }
    
    if (context->aaType >= TEXT_AA_LCD_HRGB) {
        /* Prefer LCD glyphs to embedded bitmaps except for CJK fonts */
        if (context->sbits) {
            context->sbits = context->scalerInfo->supportsCJK;
        }
	/* This only affects bitmaps from T2K. Outlines are not scaled */
	if (context->aaType == TEXT_AA_LCD_HRGB) {
	    context->t2kFlags |= T2K_LCD_MODE;
	} else {
	    context->t2kFlags |= T2K_LCD_MODE_V;
	}
	context->t2kFlags |= (lcdscale << 24); // oversample scale.
    }

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
    T2KScalerInfo *scalerInfo = (T2KScalerInfo *) id;
    JNIEnv* env = scalerInfo->env;
    jobject bBuffer;
    int bread = 0;


    /* If the read is to a negative offset, something is badly wrong,
     * and if the offset is past the end of file, we can't return valid
     * data, so the read will fail. Perhaps we should assert immediately,
     * but we need to let such cases proceed to call up into Java so
     * that it can invalidate the font.
     */

    if (numBytes <= 0) {
        return; /* some bad fonts have zero-length tables */
    }
    /* Large reads will bypass the cache and data copying */
    if (numBytes > FILEDATACACHESIZE) {
	bBuffer = (*env)->NewDirectByteBuffer(env, destBuffer, numBytes);
	if (bBuffer != NULL) {
	    /* Loop until the read succeeds (or EOF).
	     * Just returning without reading the data will cause a crash.
             * If no font data can be read, tsi_Assert jumps out of T2K.
	     */
	    while (bread == 0) {
		bread = (*env)->CallIntMethod(env, scalerInfo->font2D,
					       sunFontIDs.ttReadBlockMID,
					       bBuffer, offset, numBytes);
	    }
            /* If no font data can be read, we need to jump out of T2K */
            tsi_Assert(scalerInfo->memHandler, (bread > 0), T2K_ERR_BAD_READ);
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
             * One scenario in which it is entered is OutOfMemoryError in
             * which case this last gasp try is also likely to fail.
	     */
	    jbyteArray byteArray = (jbyteArray)
		(*env)->CallObjectMethod(env, scalerInfo->font2D,
					 sunFontIDs.ttReadBytesMID,
					 offset, numBytes);
            /* If there's an OutofMemoryError then byteArray will be null */
            if (byteArray != NULL) {
	        (*env)->GetByteArrayRegion(env, byteArray,
				           0, numBytes, (jbyte*)destBuffer);
            }
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
        tsi_Assert(scalerInfo->memHandler, (bread > 0), T2K_ERR_BAD_READ);
	memcpy(destBuffer, scalerInfo->fontData, numBytes);
    }
}


static void freeScalerInfo(JNIEnv *env, T2KScalerInfo *scalerInfo) {
    if (scalerInfo->fontData != NULL) {
        free(scalerInfo->fontData);
    }
    if (scalerInfo->directBuffer != NULL) {
        (*env)->DeleteGlobalRef(env, scalerInfo->directBuffer);
        scalerInfo->directBuffer = NULL;
    }
    freeLayoutTableCache(scalerInfo->layoutTables);
    if (scalerInfo->bwGlyphs != NULL) {
        free(scalerInfo->bwGlyphs);
    }
    /* clear memory for 'fast-fail' in event we still try to use it.
     * Also 'freeScaler' needs t2k NULLed out so it doesn' try to re-free
     * Need to leave freeing the memory for the struct to that code.
     */
    memset(scalerInfo, 0, sizeof(T2KScalerInfo));
}

/* This is called when T2K has returned an error code and
 * T2K functions cannot be called. The T2K struct it references and
 * its memHandler will already have been freed. However we still need to
 * free scalerInfo and ensure the reference points to the null scaler.
 * So it has a subset of the operations in the JNI freeScaler();
 */
void freeScalerInfoAfterError(JNIEnv *env, T2KScalerContext *context) {
    T2KScalerInfo *scalerInfo = context->scalerInfo;
    if (isNullScaler(scalerInfo)) {
        return;
    }
    freeScalerInfo(env, scalerInfo);
    context->scalerInfo = getNullScaler();
}

JNIEXPORT void JNICALL
    Java_sun_font_FileFont_freeScaler
    (JNIEnv *env, jclass fileFontClass, jlong pScaler) {

    int errCode = 0;
    tsiMemObject  *mem;
    sfntClass     *fontClass;
    InputStream   *stream;
    T2KScalerInfo *scalerInfo = (T2KScalerInfo*)pScaler;

    if (isNullScaler(scalerInfo)) {
        return; // nothing to do.
    }
    if (scalerInfo->memHandler == NULL || scalerInfo->t2k == NULL) {
        free(scalerInfo); //just free the memory.
        return;
    }

    mem = scalerInfo->memHandler;
    fontClass = scalerInfo->t2k->font;
    stream = fontClass->in;
    scalerInfo->env = env;
    scalerInfo->font2D = NULL; /* can't be needed as the font is gc'd */
    DeleteT2K(scalerInfo->t2k, &errCode);
    t2kIfDebugMessage(errCode, "DeleteT2K failed", errCode);
    Delete_sfntClass(fontClass, &errCode);
    t2kIfDebugMessage(errCode, "Delete_sfntClass failed", errCode);
    Delete_InputStream(stream, &errCode);
    t2kIfDebugMessage(errCode, "Delete_InputStream failed", errCode);
    tsi_DeleteMemhandler(mem);
    freeScalerInfo(env, scalerInfo);
    free(scalerInfo);
}

TTLayoutTableCache* newLayoutTableCache() {
  TTLayoutTableCache* ltc = malloc(sizeof(TTLayoutTableCache));
  if (ltc) {
    ltc->gsub = 0;
    ltc->gpos = 0;
    ltc->gdef = 0;
    ltc->mort = 0;
    ltc->kern = 0;
    ltc->kernPairs = 0;
    ltc->gsub_len = -1;
    ltc->gpos_len = -1;
    ltc->gdef_len = -1;
    ltc->mort_len = -1;
    ltc->kern_len = -1;
  }
  return ltc;
}

void freeLayoutTableCache(TTLayoutTableCache* ltc) {
  if (ltc) {
    if (ltc->gsub) free(ltc->gsub);
    if (ltc->gpos) free(ltc->gpos);
    if (ltc->gdef) free(ltc->gdef);
    if (ltc->mort) free(ltc->mort);
    if (ltc->kern) free(ltc->kern);
    if (ltc->kernPairs) free(ltc->kernPairs);
    free(ltc);
  }
}

JNIEXPORT jlong JNICALL
    Java_sun_font_TrueTypeFont_createScaler
    (JNIEnv *env, jobject font2D, jint fileSize, jint fontNumber,
     jboolean supportsCJK, jintArray bwGlyphArray) {

    int errCode = 0;
    tsiMemObject* memHandler;
    InputStream *stream;
    sfntClass *fontClass;
 
    T2KScalerInfo *scalerInfo =
      (T2KScalerInfo*)calloc(1, sizeof(T2KScalerInfo));

    if (scalerInfo == NULL) {
	return 0L;
    }
    memset(scalerInfo, 0, sizeof(T2KScalerInfo));
    scalerInfo->env = env;
    scalerInfo->font2D = font2D;
    scalerInfo->pathType = QUADPATHTYPE; /* for TrueType */
    scalerInfo->supportsCJK = supportsCJK; /* for TrueType bitmaps */
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
    scalerInfo->bwGlyphCnt = 0;
    scalerInfo->bwGlyphs = NULL;

    if (bwGlyphArray != NULL) {
        int len = (*env)->GetArrayLength(env, bwGlyphArray);
        jint* gids =
            (jint*)(*env)->GetPrimitiveArrayCritical(env, bwGlyphArray, NULL);
        if (gids) {
            int i;
            scalerInfo->bwGlyphCnt = len;
            scalerInfo->bwGlyphs = (int*)(calloc(len, sizeof(int)));
            for (i=0; i<len; i++) {
                scalerInfo->bwGlyphs[i] = gids[i];
            }
            (*env)->ReleasePrimitiveArrayCritical(env, bwGlyphArray,
                                                  gids, JNI_ABORT);
        }
    }

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
    scalerInfo->memHandler = memHandler;

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

/*
 * - input image is T2K's 1 bit per sample format with
 *   "scale" samples per destination pixel
 *
 * - output image is 3 byte samples per destination pixel
 *   thus, 1 byte sample per RGB component in the destination
 *
 * - default scale of 6 means that 2 bits in the input must be
 *   combined into a single output byte.
 *   This provides for a better approximation to coverage
 *   intensity so combining the benefits of over-sampling with
 *   the benefits of coverage on transition edges, which can help
 *   reduce the transitions which contribute to colour fringing (see
 *   filtering in the next paragraph)
 *
 * - furthermore, the code that uses these LCD glyphs
 *   will apply a filter that is 3 samples wide so that
 *   energy from any given sample can spread to adjacent
 *   samples, thus we need at least one byte on both sides
 *   of the image to absorb this extra energy.  (The
 *   code actually pads by more than that for other
 *   reasons - see the sub-pixel positioning note below).
 *
 * - T2K produced a glyph image assuming that each bit was a
 *   discrete output pixel, but our real addressability
 *   can be either one destination component (with lcd
 *   subpixel positioning turned on) or one destination
 *   pixel (with lcd subpixel pos turned off).  For simplicity
 *   we will modify the glyph origin and the output image
 *   so that the image is ready to be applied to a whole
 *   pixel boundary.  If lcd sub-pixel positioning is
 *   turned on, then further adjustments may be necessary
 *   at the rendering stage. The sub-pixel positioning implementation
 *   requires that we further pad by an additional two bytes each
 *   side of the image.
 *
 * - To adjust the image position we first round the left edge of
 *   the T2K glyph image to the nearest destination pixel
 *   and then produce a bitmap relative to that destination
 *   pixel location.  This may involve a virtual shift of
 *   the samples left or right depending on whether the
 *   nearest destination pixel location was to the left or
 *   right of the actual left edge of the T2K bit samples.
 * 
 * - Note this function relies on its caller to call it only if width>0.
 *
 * - Earlier we had defined PADRIGHT to be 2 as that was sufficient
 *   for the software loops, but this resulted in non-integral
 *   row lengths, which is something that the OpenGL pipeline
 *   cannot handle.  Therefore, we now require that PADLEFT and PADRIGHT
 *   each be a multiple of 3.  It is unfortunate that this will "waste"
 *   one byte per row in the glyph image (compared to before), but that
 *   seems to be the price we pay to get hardware acceleration of
 *   LCD-optimized text.
 */
#define PADLEFT 3
#define PADRIGHT 3
static void CopyBW2LCDH(T2K *t2k, int xscale, void **dstImage,
			int *dstBytesWidth, float *rndTopLeftX)
{
    const UInt8* srcRow = t2k->baseAddr;
    int srcRowBytes     = t2k->rowBytes;
    int width           = t2k->width;
    int height          = t2k->height;
    int wholeByteCount  = width >> 3;
    int remainingBitsCount = width & 7;
    UInt8* dstRow;
    float topLeftX;
    int dstRowBytes, bitsWidth, rndbits, padleft;
    int shiftMajor=0, shiftMinor=0, subpixelShift;
    int average = xscale/3; /* scale will and must be a multiple of 3. */
    int round_average = average/2; /* used for rounding */

    /* Round the top left x to the nearest whole pixel position and
     * calculate the subpixelShift.
     * subpixelShift is the difference in glyph image position between
     * the rounded (whole pixel) position and the scaled (sub pixel) position.
     * For example, suppose scale is 6 and the scaled topLeftX is 26.0.
     * Trying to be compatible with the unscaled glyph position this is
     * rounded up to 29 before scaling down : floor((26.0 + (6/2))/6) = 4;
     * So "4" is our computed whole pixel left edge position, which
     * corresponds to "24" in the scaled T2K coordinate system.
     * Since T2K had returned that value as "26" we want to adjust the
     * sub-pixel image by (26-24)==2 to compensate for this rounding
     * a +ve value for (topLeftX-rndTopLeftX) means we have rounded down
     * so will need to shift the image to the right.
     * The range of the shift for a scale of 6 is -3 <= shift <= +2
     * or more generally from -scale/2 -> scale/2-1 which amounts to half a
     * pixel at most, so padding of one full RGB pixel is always sufficient.
     * 
     * shiftMajor is the number of RGB subpixels to shift
     * shiftMinor is the position within an RGB subpixel to shift - ie in
     * the range 0 .. average-1
     * Perhaps it would have been possible to make T2K return the image
     * already shifted and with an already adjusted topLeftX but right now
     * this seems easier.
     */
    topLeftX = t2kFixedToFloat(F26Dot6ToFixed(t2k->fLeft26Dot6));
    *rndTopLeftX = (int)floor((topLeftX + (xscale/2.0)) / (float)xscale);
    subpixelShift = (topLeftX-*rndTopLeftX*xscale);
    shiftMajor = (int)floor(subpixelShift/(float)average);
    if ((subpixelShift % average) != 0) {
	shiftMinor = subpixelShift-(shiftMajor*average);
    }

    /* bitsWidth is the width we need, including padding on the right
     * average is the number of scaled bits corresponding to one subpixel
     * component, so is the needed amount of padding on the right for
     * the filter.
     * bitsWidth then needs to be rounded up to a whole number of pixels
     * Then we can calculate how many whole bytes are needed per row.
     */
    bitsWidth = width + subpixelShift + average;
    rndbits = bitsWidth % xscale;
    if (rndbits != 0) {
	bitsWidth+= xscale - rndbits;
    }
    
    /* The width is adjusted with 3 or 6 bytes padding on the left (padleft,
     * which is either PADLEFT or PADLEFT*2) and 3 bytes padding on the
     * right (PADRIGHT). padleft is 3 when shiftMajor >=0 and 6 when it is <=0
     * We copy the down-sampled source into the destination row at a byte
     * offset determined by this padding and shiftMajor.
     * If shiftMajor is >=0 (which empirically is the usual case) then
     * the offset skips past the "padleft bytes and past the "shiftMajor"
     * bytes.
     * If shiftMajor is < 0 we copy into the padding bytes, so will need
     * 3 additional padding bytes for subpixel positioning, in which case
     * padleft is 6. In this scenario we have 6 bytes per row which is
     * used only with fractional metrics/subpixel positioning. This is
     * somewhat wasteful but seems rare.
     * In both cases we draw the image to the left of its previously
     * calculated position so that the original glyph image ends up at
     * the same position it would have done. Its drawn 1 pixel to the left
     * if padleft==3 and 2 pixels to the left if padleft==6
     *
     * To recap, the padding is needed for two reasons :
     * filtering needs a byte to the left and right to diffuse coverage.
     * This padding byte needs to be contained in a whole pixel (ie 3 bytes).
     * The left pading is added here, but the right padding is added above
     * in the calculation for bitsWidth
     * Subpixel positioning needs 2 bytes at the left of the image so it
     * can safely index past at rendering time and a matching 2 bytes
     * at the right of the image of the image.
     * The rendering loops need to make only a minor adjustment to use
     * these bytes for subpixel positioning by offsetting into the image row.
     */
    if (shiftMajor >= 0) {
      padleft = PADLEFT;
      *rndTopLeftX-=1.0;
    } else {
      padleft = PADLEFT*2;
      *rndTopLeftX-=2.0;
    }
    *dstBytesWidth = dstRowBytes = padleft + (bitsWidth/average) + PADRIGHT;
    *dstImage = malloc(dstRowBytes*height);
    memset(*dstImage, 0, dstRowBytes*height);
    dstRow = (UInt8*)*dstImage;

    while (height--) {
	int i,j;
	jint srcValue;  /* 32 bits signed */
	int count = shiftMinor;
	/* Initialise "sum" to average/2 so that the calculation
	 * "sum/average" rounds to the nearest. In practice this will mean
	 * round up if sum is odd. Note that this will never cause us to
	 * end up with 256 (==0 in a byte) since the maximum inputs will
	 * be 255 + 255 + (2/1) = 511, and 511/2 = 255.
	*/
	int sum = round_average;

	const UInt8* src8 = srcRow;
	UInt8 *dstByte = dstRow + padleft + shiftMajor;

        srcRow += srcRowBytes;
        dstRow += dstRowBytes;

        for (i = 0; i < wholeByteCount; i++) {
            srcValue = (*src8++) << 24;
            for (j = 0; j < 8; j++) {
		sum += (srcValue >> 31) & 0xff;
		count++;
		if (count == average) {
		    *dstByte++ = (sum/average);
		    sum = round_average;
		    count = 0;
		}
                srcValue <<= 1;
            }
        }
        if (remainingBitsCount) {
            srcValue = (*src8) << 24;
            for (j = 0; j < remainingBitsCount; j++) {
		sum += (srcValue >> 31) & 0xFF;
		count++;
		if (count == average) {
		    *dstByte++ = (sum/average);
		    sum = round_average;
		    count = 0;
		}		
                srcValue <<= 1;
            }
        }
	if (count > 0) {
	    *dstByte = sum/average;
	}
    }
}

static void getTBFromBW2(T2K *t2k, int *topmost, int *bottommost) {
    const UInt8* srcRow = t2k->baseAddr;
    int height = t2k->height;
    int srcRowBytes = t2k->rowBytes;

    int tmsp=0x7fffffff, bmsp=0;
    int i, y;

    for (y=0;y<height;y++) {
        const UInt8* src8 = srcRow;

        srcRow += srcRowBytes;

        for (i = 0; i < srcRowBytes; i++) {
	    if (*src8++) {
		if (y<tmsp) tmsp = y;
		if (y>bmsp) bmsp = y;
		break;
	    }
        }
    }
    /* just in case we somehow have an image that's all zeroes,
     * set top == bottom == 0.
     */
    if (tmsp > bmsp) tmsp = bmsp;
    *topmost = tmsp;
    *bottommost = bmsp;
    return;
}

/*
 * The source image is in T2K packed output format.
 * The output image is 3 bytes per pixel - ie 1 byte per subpixel
 * The subpixels here increase the VERTICAL resolution
 */
static void CopyBW2LCDV(T2K *t2k, int scale, void **dstImage,
			int *dstBytesWidth, int *dstHeight, float *rndTopLeftY)
{
    const UInt8* srcImage = t2k->baseAddr;
    int srcRowBytes       = t2k->rowBytes;
    int width             = t2k->width;
    int height            = t2k->height;
    UInt8 *dstRow, *image;
    int dstRowBytes, paddedHeight;
    int shiftMajor=0, shiftMinor=0;
    int padtop=0, x, y;
    int miny, maxy, sminy, smaxy, subpixelShift;
    int average = scale/3;
    int round_average = average/2; /* used for rounding. SeeCopyBW2LCDH  */

    /* t2k->fTop26Dot6 is relative to the origin of the glyph
     * Since the glyph origin is at the baseline (for latin text) this
     * this means a greater value is in the "up the page" direction.
     * round top left y up to next integral (pixel) position, but adjust
     * the image downwards to compensate. This is just the image position
     * relative to the glyph position. It doesn't affect anything else.
     * subpixelShift is a measure of how much shift is introduced
     * by the above calculation at the scale of the image.
     * Break this down as follows :
     * shiftMajor is the number of RGB subpixels to shift
     * shiftMinor is the position within an RGB subpixel to shift.
     */
    float topLeftY = t2kFixedToFloat(F26Dot6ToFixed(t2k->fTop26Dot6));
    *rndTopLeftY = (float)ceil(topLeftY / (float)scale);
    subpixelShift = (int)(*rndTopLeftY*scale-topLeftY);
    shiftMajor = subpixelShift/average;
    shiftMinor = subpixelShift % average;

    /*
     * Find the top-most and bottom-most "on" source image pixels.
     * Add subpixelshift to these to reflect where we will place these in
     * the output image.
     * Rows are added to the bottom sufficient to accommodate this.
     * The image needs to be filtered to reduce colour fringing, therefore
     * it must be padded to absorb energy distributed by this filter to
     * adjacent subpixels.
     * The test sminy<average implicitly hard codes a requirement that the
     * filter be 3 RGB subpixels wide - ie energy is not distributed beyond
     * the adjacent subpixels.
     * So average tells us how many of the source image bits to consider
     * and hence the number of bits of padding needed.
     */
    getTBFromBW2(t2k, &miny, &maxy);
    sminy = miny+subpixelShift;
    smaxy = maxy+subpixelShift;
    if (sminy<average) {
	padtop = 1; // will need to allocate an extra row at the top.
	*rndTopLeftY+=1.0;
    }

    /* The height required including padding on the bottom is paddedHeight
     * In the scaled space we see how much padding is needed on the bottom
     * and then we calculate the final height in rows by dividing by scale
     * and adding on any top padding.
     */
    paddedHeight = smaxy+average+1;
    if (paddedHeight % scale != 0) {
	paddedHeight+= (scale-(paddedHeight % scale));
    }
    paddedHeight = (paddedHeight / scale) + padtop;
    dstRowBytes = width*3;
    image = malloc(dstRowBytes*paddedHeight);
    /* Copy the above values to the return addresses */
    *dstHeight = paddedHeight;
    *dstBytesWidth = dstRowBytes;
    *dstImage = image;

    memset(image, 0, dstRowBytes*paddedHeight);
    dstRow = image + padtop*dstRowBytes;

    /* Process a column at a time as we need to average within a column.
     * Although this could be optimised, its preferred to keep the code
     * more straightforward and maintainable.
     */
    for (x=0; x < width ; x++) {
	/* set the x offset into the source and destination images.
	 * The start row of the destination may skip past a top row
	 * of padding used to absorb filter energy.
	 * The source is 1 bit pp, the dest is 3 bytes pp
	 * These will be updated to point to the right row.
	 */
	UInt8* srcBytePtr = (UInt8*)srcImage+(x>>3);
	UInt8* dstBytePtr = dstRow+(x*3);
	int bitmask = 0x80 >> (x & 7);
	int subpixelCount = shiftMajor;
	int count = shiftMinor;
	int sum = round_average;
	for (y=0; y <= maxy; y++) {
	    sum += (*srcBytePtr & bitmask) ? 0xFF : 0;
	    srcBytePtr += srcRowBytes;
	    count++;
	    if (count == average) {
		dstBytePtr[subpixelCount++] = (sum/average);
		sum = round_average;
		count = 0;
		if (subpixelCount == 3) {
		    subpixelCount = 0;
		    dstBytePtr += dstRowBytes;
		}
	    }
	}
	if (count > 0) {
	    dstBytePtr[subpixelCount] = sum/average;
	}
    }
}


/*
 * Simple filter centred on a sub-pixel. The filter will span pixel
 * boundaries.
 * Approx 1/3 of each of the 3 adjacent sub-pixels contributes to the
 * output subpixel. We approximate 1/3 as (21931*SUBPIX/65536)
 * We arrive at 21931 from ((1<<24) - 1) / (255*3)
 * This gives a good statistical distribion of the values into the 256
 * possible value "buckets".
 */
#define DIV3(x) (((x) * 21931) >> 16)
static void filterPixelsH(const UInt8* srcImage, const UInt8* dstImage,
			  int width, int height) {
    int x, y;
    const UInt8 *srcRow=srcImage;
    UInt8 *dstRow=(UInt8*)dstImage;

    for (y=0;y<height;y++) {
	dstRow[0] = (UInt8)(DIV3(srcRow[0] + srcRow[1]));
	for (x = 1; x < width-1; x++) {
	    dstRow[x] = (UInt8)(DIV3(srcRow[x-1] + srcRow[x] + srcRow[x+1]));
	}
	dstRow[width-1] = (UInt8)(DIV3(srcRow[width-2] + srcRow[width-1]));
	
        srcRow+=width;
	dstRow+=width;	
    }
}
/*
 * Simple filter centred on a sub-pixel. The filter will span pixel
 * boundaries. The in-memory organisation is what complicates this filter
 * Vertically adjacent subpixels may be stored a scanrow apart.
 */
static void filterPixelsV(const UInt8* srcImage, const UInt8* dstImage,
			  int width, int height) {
    int x, y;
    const UInt8 *src=srcImage;
    UInt8 *dst=(UInt8*)dstImage;

    for (x=0;x<width;x+=3) {
	dst[x]   = (UInt8)(DIV3(src[x] + src[x+1]));
	dst[x+1] = (UInt8)(DIV3(src[x] + src[x+1] + src[x+2]));
	if (height == 1) {
	    dst[x+2] = (UInt8)(DIV3(src[x+1] + src[x+2]));
	} else {
	    dst[x+2] = (UInt8)(DIV3(src[x+1] + src[x+2] + src[x+width]));
	}
    }

    for (x=0;x<width;x+=3) {
	for (y=1;y<height-1;y++) {
	    src = srcImage+(y*width);
	    dst = (UInt8*)dstImage+(y*width);
	    /* filter centred on Red (top subpixel) - row spanned */
	    dst[x]   = (UInt8)(DIV3(src[x-width+2] + src[x] + src[x+1]));
	    /* filter centred on Green (centre subpixel) - all in same row */
	    dst[x+1] = (UInt8)(DIV3(src[x]+src[x+1]+src[x+2]));
	    /* filter centred on Blue (bottom subpixel) - row spanned */
	    dst[x+2] = (UInt8)(DIV3(src[x+1] + src[x+2] + src[x+width]));
	}
    }
    
    if (height > 1) {
	src = srcImage+((height-1)*width);
	dst = (UInt8*)dstImage+((height-1)*width);
	for (x=0;x<width;x+=3) {
	    dst[x]   = (UInt8)(DIV3(src[x-width+2] + src[x] + src[x+1]));
	    dst[x+1] = (UInt8)(DIV3(src[x] + src[x+1] + src[x+2]));
	    dst[x+2] = (UInt8)(DIV3(src[x+1] + src[x+2]));
	}
    }
}

int setupT2KContext(JNIEnv *env, jobject font2D, 
		    T2KScalerInfo *scalerInfo, T2KScalerContext *context,
		    jboolean sbits, int renderFlags) {
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
    /* set up grid fitting only if scan conversion is performed */ 
    T2K_NewTransformation(t2k, renderFlags & T2K_SCAN_CONVERT, 72, 72, 
        &t2kMatrix, sbits, &errCode);

    return errCode;
}

JNIEXPORT void JNICALL
Java_sun_font_FileFont_setNullScaler
    (JNIEnv *env, jobject font2D, jlong pScalerContext) {

    T2KScalerContext *context = (T2KScalerContext*)pScalerContext;
    context->scalerInfo = getNullScaler();
}

JNIEXPORT jfloat JNICALL
Java_sun_font_FileFont_getGlyphAdvance
    (JNIEnv *env, jobject font2D, jlong pScalerContext, jint glyphCode) {
  
    int errCode = 0;
    T2KScalerContext *context = (T2KScalerContext*)pScalerContext;
    T2KScalerInfo *scalerInfo = context->scalerInfo;
    T2K *t2k = scalerInfo->t2k;
    UInt32 renderFlags = context->t2kFlags |T2K_SCAN_CONVERT |T2K_SKIP_SCAN_BM;
    int fAdvanceX;

    if (isNullScaler(scalerInfo) || context == theNullScalerContext) {
	return (jfloat)0;
    }

    if (glyphCode >= INVISIBLE_GLYPHS) {
	return (jfloat)0;
    }
    errCode = setupT2KContext(env, font2D, scalerInfo, context, 
                 context->sbits, renderFlags);
    if (errCode) {
        freeScalerInfoAfterError(env, context);
	return (jfloat)0;
    }

    T2K_RenderGlyph(t2k, glyphCode, 0, 0, context->greyLevel,
                    renderFlags, &errCode);
 
   if (errCode) {
       freeScalerInfoAfterError(env, context);
        return (jfloat)0;
    }

    if (context->doFM) {
        /* Fractional metrics requested.
	 * Use the linearly scaled advance which is from the precomputed
	 * hmtx table in a truetype font. xAdvanceWidth16Dot16 is derived
	 * from the points after hinting which snaps those points to the
	 * pixel grid. The linearly scaled advance is what the advance would
	 * be if hinting effects weren't present. This assumes that hinting
	 * doesn't also have some "linear" consequence that isn't accounted
	 * for in the precomputed metrics.
	 * So since xAdvanceWidth16Dot16 is always integral values, then
	 * xLinearAdvanceWidth16Dot16 is what we need to use here, unless
	 * there is some way to request the rasteriser
	 */
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
    if (errCode) {
        t2kIfDebugMessage(errCode, "T2K_PurgeMemory failed", errCode);
        freeScalerInfoAfterError(env, context);
    }
 
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

    UInt32 renderFlags = context->t2kFlags |T2K_SCAN_CONVERT |T2K_SKIP_SCAN_BM;
    int fAdvanceX, fAdvanceY;

    if (metricsPt == NULL) {
	return;
    }

    if (isNullScaler(scalerInfo) || context == theNullScalerContext ||
	glyphCode >= INVISIBLE_GLYPHS ||
	setupT2KContext(env, font2D, scalerInfo, context, 
            context->sbits, renderFlags) != 0) {
	(*env)->SetFloatField(env, metricsPt, sunFontIDs.xFID, (jfloat)0);
	(*env)->SetFloatField(env, metricsPt, sunFontIDs.yFID, (jfloat)0);
	return;
    }


    T2K_RenderGlyph(t2k, glyphCode, 0, 0, context->greyLevel,
                    renderFlags, &errCode);
    if (errCode) {
        freeScalerInfoAfterError(env, context);
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
    if (errCode) {
        t2kIfDebugMessage(errCode, "T2K_PurgeMemory failed", errCode);
        freeScalerInfoAfterError(env, context);
    }

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
    int aaType = context->aaType;
    int greyLevel = context->greyLevel;
    UInt32 renderFlags = context->t2kFlags | T2K_SCAN_CONVERT;
    short width, height;
    int imageRowBytes, imageHeight, imageSize;
    float rndTopLeftX, rndTopLeftY;
    int errCode;
    void* paddedImage = NULL;
    
    if (isNullScaler(scalerInfo) || context == theNullScalerContext) {
	return getNullGlyphImage();
    }

    errCode = setupT2KContext(env, font2D, scalerInfo, context, 
                  context->sbits, renderFlags);

    if (errCode) {
        freeScalerInfoAfterError(env, context);
	return getNullGlyphImage();
    }

    /* For some glyphs override requested behaviour to force a B&W glyph */
    if (scalerInfo->bwGlyphs != NULL && aaType == TEXT_AA_LCD_HRGB &&
        context->t2kMatrix.t00 < t2kFloatToFixed(14.0) &&
        context->t2kMatrix.t00 == context->t2kMatrix.t11 &&
        context->t2kMatrix.t01 == 0 && context->t2kMatrix.t10 == 0) {
        int matched = 0, i;
        for (i=0; i<scalerInfo->bwGlyphCnt;i++) {
            if (scalerInfo->bwGlyphs[i] == glyphCode) {
                matched = 1;
                break;
            }
        }
        if (matched) {
            aaType = TEXT_AA_OFF;
            greyLevel = BLACK_AND_WHITE_BITMAP;
            renderFlags = T2K_GRID_FIT | T2K_CODE_IS_GINDEX | T2K_SCAN_CONVERT;
            if (isHintingDisabled) { context->t2kFlags &= ~T2K_GRID_FIT; }
        }
    }    

    T2K_RenderGlyph(t2k, glyphCode, 0, 0, greyLevel,
                    renderFlags, &errCode);
    if (errCode) {
        freeScalerInfoAfterError(env, context);
        return getNullGlyphImage();
    }

    /* LCD text isn't used if an embedded bitmap is requested and available.
     * When "sbits" is true, then T2K will return embedded bitmaps if present.
     * This happens before even entering the internal T2K code that handles
     * the LCD case. Thus the returned bitmaps are not scaled. They are the
     * same as in the B&W case. We must therefore check the T2k flag
     * "embeddedBitmapWasUsed" on return to know how to handle the glyph.
     * If algorithmic bolding (but not obliquing) is specified then the
     * bitmaps may also be used (ie the bitmap is obtained and widened)
     * so the same applies in that case too.
     * Since that case also sets the embeddedBitmapWasUsed flag then that
     * flag is sufficient for us to know how to handle both these cases.
     *
     * However if there is no embedded bitmap then "widening" would still
     * happen. The widening will be applied to the 3X wide bitmap but it
     * will not sufficiently widen or bolden it because the code assumes
     * that the glyph bitmap is at pixel (1X) resolution. Eg it will add an
     * extra column of subpixels rather than the 3 extra columns needed.
     * That is the principal problem as it will affect metrics. Also probably
     * noticeable is that it will allow "gaps" of 1 subpixel rather than
     * 1 whole pixel. Until such time as we can resolve these issues we
     * need to disable bitmap bolding except when embedded bitmaps are
     * retrieved. But the increased horizontal resolution mitigates this
     * substantially
     */
    if (t2k->embeddedBitmapWasUsed &&
	(aaType == TEXT_AA_LCD_HRGB || aaType == TEXT_AA_LCD_VRGB)) {
        aaType = TEXT_AA_OFF;
    }
    width = (UInt16)t2k->width;
    height = (UInt16)t2k->height;
 
    imageRowBytes = width;
    imageHeight = height;
    if (aaType >= TEXT_AA_LCD_HRGB && width > 0) {
	if (aaType == TEXT_AA_LCD_HRGB) {
	    CopyBW2LCDH(t2k, lcdscale,
			&paddedImage, &imageRowBytes, &rndTopLeftX);
	} else {
	    CopyBW2LCDV(t2k, lcdscale,
			&paddedImage, &imageRowBytes, &imageHeight,
			&rndTopLeftY);
	}
    }

    imageSize = imageRowBytes*imageHeight;
    glyphInfo = (GlyphInfo*)malloc(sizeof(GlyphInfo)+imageSize);
    glyphInfo->cellInfo = NULL;
    glyphInfo->rowBytes	 = imageRowBytes;

    if (aaType == TEXT_AA_LCD_HRGB) {
	/* width adjusted by 2 because of FM subpixel padding */
	glyphInfo->width = (imageRowBytes-PADRIGHT)/3;
    } else if (aaType == TEXT_AA_LCD_VRGB) {
	/* NB no subpixel positioning for VRGB/VBGR */
	glyphInfo->width = (imageRowBytes/3);
    } else {
	glyphInfo->width = imageRowBytes;
    }
    glyphInfo->height = imageHeight;


    /* The T2K rasteriser always reports an integer value for topLeftX even
     * though we store it in a float. Perhaps it makes sense to store these
     * values in 16 bit integers as that will save 4 bytes per glyph
     * at the cost of being marginally slower to process at rendering time.
     */
    if (aaType == TEXT_AA_LCD_HRGB && width != 0) {
	glyphInfo->topLeftX = rndTopLeftX;
    } else {
	glyphInfo->topLeftX= t2kFixedToFloat(F26Dot6ToFixed(t2k->fLeft26Dot6));
    }

    if (aaType == TEXT_AA_LCD_VRGB && width != 0) {
	/* NB no subpixel positioning for VRGB/VBGR. */
	glyphInfo->topLeftY = -rndTopLeftY;
    } else {
	glyphInfo->topLeftY= -t2kFixedToFloat(F26Dot6ToFixed(t2k->fTop26Dot6));
    }

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
	memset(glyphInfo->image, 0, imageSize);
	if (aaType == TEXT_AA_OFF) {
	    CopyBW2Grey8(t2k->baseAddr, t2k->rowBytes,
			 (void *)glyphInfo->image, width, width, height);
	} else if (aaType == TEXT_AA_LCD_HRGB) {
	    filterPixelsH(paddedImage, (UInt8*)glyphInfo->image,
			  imageRowBytes, glyphInfo->height);
	    free(paddedImage);
	} else if (aaType == TEXT_AA_LCD_VRGB) {
		filterPixelsV(paddedImage, (UInt8*)glyphInfo->image,
			      imageRowBytes, glyphInfo->height); 
	    free(paddedImage);
	} else /* must be (aaType == TEXT_AA_ON) */ {
	    int x,y;
	    const UInt8* srcRow = (UInt8*)t2k->baseAddr;
	    UInt8* dstRow = (UInt8*)glyphInfo->image; 
	    for (y = 0; y < glyphInfo->height; y++) {
		for (x = 0; x < width; x++) {
		    dstRow[x] = T2KByteToAlpha255(srcRow[x]);
		}
		dstRow += imageRowBytes;
		srcRow += t2k->rowBytes;
	    }
	}
    }

    T2K_PurgeMemory(t2k, 1, &errCode);  // to release the bitmap/outline
    if (errCode) {
        t2kIfDebugMessage(errCode, "T2K_PurgeMemory failed", errCode);
        freeScalerInfoAfterError(env, context);
    }
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
    UInt32 renderFlags = context->t2kFlags | 
	T2K_SCAN_CONVERT | T2K_RETURN_OUTLINES;
    int errCode;

    if (isNullScaler(scalerInfo) || context == theNullScalerContext) {
	return NULL;
    }

    errCode = setupT2KContext(env, font2D, scalerInfo, context, 
                 false, renderFlags);

    if (errCode) {
        freeScalerInfoAfterError(env, context);
	return (*env)->NewObject(env, sunFontIDs.pt2DFloatClass,
				 sunFontIDs.pt2DFloatCtr, 0, 0);
    }

    T2K_RenderGlyph(t2k, glyphCode, 0, 0, context->greyLevel, renderFlags, 
		    &errCode);
    if (errCode) {
        t2kIfDebugMessage(errCode, "T2K_RenderGlyph failed", errCode);
        freeScalerInfoAfterError(env, context);
        return NULL; // seems to be allowed.
    }

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
    if (errCode) {
        t2kIfDebugMessage(errCode, "T2K_PurgeMemory failed", errCode);
        freeScalerInfoAfterError(env, context);
    }
    return point;
}

/* This function is called by the OpenType layout engine.
 */
int getUnitsPerEmForLayout(T2KScalerInfo *scalerInfo) {

    int upem = 2048;
    T2K *t2k;

    if (!isNullScaler(scalerInfo)) {
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
    hsFixed mag, fX, fY;
    hsFract caretX, caretY, baseX, baseY;
    jobject metrics;
    jfloat ax, ay, dx, dy, bx, by, lx, ly, mx, my;
    jfloat f0 = 0.0;

    int errCode;

    if (isNullScaler(scalerInfo) || context == theNullScalerContext) {
	return (*env)->NewObject(env, sunFontIDs.strikeMetricsClass,
				sunFontIDs.strikeMetricsCtr,
				f0,f0,f0,f0,f0,f0,f0,f0,f0,f0);
    }

    errCode = setupT2KContext(env, font2D, scalerInfo, context, 
                 context->sbits, 0);

    if (errCode) {
        freeScalerInfoAfterError(env, context);
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
/*     printf("mx=%f my=%f\n", mx, my); */

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
    nresults[6] = (size_t)&(info->rowBytes)-baseAddr;
    nresults[7] = (size_t)&(info->topLeftX)-baseAddr;
    nresults[8] = (size_t)&(info->topLeftY)-baseAddr;
    nresults[9] = (size_t)&(info->image)-baseAddr;
    nresults[10] = (jlong)(uintptr_t)info; /* invisible glyph */
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
                    AccelGlyphCache_RemoveAllCellInfos(ginfo);
                }
		free((void*)ginfo);
	    }
	}
	(*env)->ReleasePrimitiveArrayCritical(env, jmemArray, ptrs, JNI_ABORT);
    }
    if (pContext != 0
        && ((T2KScalerContext*)pContext) != theNullScalerContext) {

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
                    AccelGlyphCache_RemoveAllCellInfos(ginfo);
                }
		free((void*)ginfo);
	    }
	}
	(*env)->ReleasePrimitiveArrayCritical(env, jmemArray, ptrs, JNI_ABORT);
    }
    if (pContext != 0
        && ((T2KScalerContext*)pContext) != theNullScalerContext) {
	free((void*)pContext);
    }
}

JNIEXPORT jint JNICALL
Java_sun_font_Type1Font_getNumGlyphs
(JNIEnv *env, jobject t1font, jlong pScaler) {

    T2KScalerInfo *scalerInfo = (T2KScalerInfo*)pScaler;
    T2K *t2k = scalerInfo->t2k;

    if (t2k == NULL) { /* bad/null scaler */
       /* null scaler can render 1 glyph - "missing glyph" with code 0
          (all glyph codes requested by user are mapped to code 0 at 
           validation step) */ 
       return (jint) 1;
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
