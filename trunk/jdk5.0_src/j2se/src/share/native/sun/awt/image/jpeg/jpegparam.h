/*
 * @(#)jpegparam.h	1.9 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
	File: JPEGParam.h

*/

#ifdef __cplusplus
extern "C" {
#endif
#include "jpeglib.h"
#include "jerror.h"
#include "jni.h"
#include "jni_util.h"

#include <setjmp.h>


#define ILLEGAL_ARG            "java/lang/IllegalArgumentException"
#define NULL_POINTER           "java/lang/NullPointerException"
#define ARRAY_BOUNDS_ERROR     "java/lang/ArrayIndexOutOfBoundsException"
#define OUT_OF_MEMORY          "java/lang/OutOfMemoryError"
#define IMAGE_FORMAT_EXCEPTION "com/sun/image/codec/jpeg/ImageFormatException"
#define IO_EXCEPTION           "java/io/IOException"
#define UNKNOWN	               "java/lang/UnknownError"
#define BITS_TABLE_LEN 17
#define SYMBOLS_TABLE_LEN 256

#define DEFAULT_DATA_PRECISION 8;
/* 
* ifdef taken from the NIFTY additions to the lib.  We want to be in sync 
* with these as well
*/
#ifdef NIFTY
#define STREAMBUF_SIZE 65536	/* choose a much larger in memory buffer for NIFTY */

#else
#define STREAMBUF_SIZE  4096	/* choose an efficiently fwrite'able size */
#endif

struct error_mgr {
		struct jpeg_error_mgr pub;	//public fields
		jmp_buf setjmp_buffer;		//for return to caller
	};

/*  define some of the error related routines for our use */
GLOBAL(void) jpeg_output_message (j_common_ptr cinfo);
GLOBAL(void) error_exit (j_common_ptr cinfo);

/* This struct holds all info necessary to access JPEGParam from compressors and decompressors */
typedef struct {
	/* The Java Native environment struct pointer */
	JNIEnv * env; 
	/* pointer the the jpeg compress struct */
	j_compress_ptr compress ;
	/* pointer the the jpeg compress struct */
	j_decompress_ptr decompress; 
	/* The object pointer for the JPEGParam object, passed as part of any native call
	 * from java.
	 */
	jobject	JPPObj; 
	 /* a simple flag so that we can easily tell the mode in which we are called */
	 boolean  is_decompressor ;
	 boolean  tables_present;
	 boolean  image_data_present;
	 boolean  packed_data;
	 boolean  force_color_space;
	 boolean  headerProcessed;
} jpeg_param;

EXTERN(boolean)
CheckNThrow(JNIEnv *env, const char *name, const char *msg); 
EXTERN(void)
EKThrowByName(JNIEnv *env, const char *name, const char *msg);

EXTERN(void)
CInfoFromJava(jpeg_param *jpp, 
	      jobject     jpegImageEncoder, 
	      jobject     colorModel);

EXTERN(void)
CInfoToJava(jpeg_param * jpp, 
	    jobject      jpegImageDecoder, 
	    jboolean     convert );

EXTERN(void)
ImageInfoFromJava(jpeg_param *jpp, 
		  jobject     jpegImageEncoder, 
		  jobject     colorModel);

EXTERN(jobject)
createJPPFromCInfo(jpeg_param * jpp, 
		   jobject      jpegImageDecoder, 
		   jboolean     convert);

EXTERN(void)
CopyTablesFromJava(jpeg_param * jpp);

EXTERN(void)
CopyTablesToJava(jpeg_param * jpp);

EXTERN(void)
CopyCompInfoFromJava ( jpeg_param *jpp );

EXTERN(void)
CopyCompInfoToJava ( jpeg_param *jpp );

EXTERN(void)
writeMarkersFromJava ( jpeg_param *jpp );


EXTERN(boolean)
CheckExcept(JNIEnv *env);

EXTERN(boolean)
CheckPtrs( jpeg_param * jpp );

EXTERN(void) 
initJPEGParam( JNIEnv *env, jobject jpp );

EXTERN(int)
getOutCSFromJPP(jpeg_param * jpp);

EXTERN(int)
getEncodedCSFromJPP(jpeg_param * jpp );
#ifdef __cplusplus
}
#endif

