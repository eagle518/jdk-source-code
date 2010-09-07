/*
 * @(#)jpegimagedecoderimpl.c	1.18 04/01/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/* ********************************************************************
 **********************************************************************
 *******************************************************************
 *** COPYRIGHT (c) Eastman Kodak Company, 1997                      ***
 *** As  an unpublished  work pursuant to Title 17 of the United    ***
 *** States Code.  All rights reserved.                             ***
 **********************************************************************
 **********************************************************************
 **********************************************************************/

/*
 *	File which link the code of JPEGImageDecoder.java and 
 *	the jpeglibrary
 *	version 1.0
 *	Friday 28th August 1997
 *
 */


#include <stdlib.h>
#include <setjmp.h>
#include <assert.h>
#include <string.h>
#include <stddef.h>

// ek java native interface headers
#include "sun_awt_image_codec_JPEGImageDecoderImpl.h"
#include "jpegparam.h"

//headers from the JPEG library
#include "jpeglib.h"
#include "jerror.h"
#include "jinclude.h" // Fix for 4411325 requires definition of SIZEOF

#if (defined EK_DEBUG || defined  EK_REG_TEST )
void displayDecompressStruct( j_decompress_ptr dcomp )
{
  printf( "JPEG decompress struct State ... \n ");
  printf( "Width = %d \n", dcomp->image_width );
  printf( "Height = %d\n", dcomp->image_height );
  printf( "out_color_space = %d\n", dcomp->out_color_space );
  printf( "output_components = %d\n", dcomp->output_components );
  printf( "jpeg_color_space= %d\n", dcomp->jpeg_color_space );
  printf( "num components = %d\n", dcomp->num_components );
  printf( "dct_method = %d\n", dcomp->dct_method );
  printf( "CCIR601_sampling = %d\n", dcomp->CCIR601_sampling );
  printf( "density_unit = %d\n", dcomp->density_unit );
  printf( "X_density = %d\n", dcomp->X_density );
  printf( "Y_density = %d\n", dcomp->Y_density );
  printf( "output_gamma = %d\n", dcomp->output_gamma );
  printf( "restart_interval = %d\n", dcomp->restart_interval );
  printf( "data precision = %d\n", dcomp->data_precision );
  printf( "global_state  = %d\n", dcomp->global_state );

  printf( "Q-Tables: %p %p %p %p\n", 
	  dcomp->quant_tbl_ptrs[0],
	  dcomp->quant_tbl_ptrs[1],
	  dcomp->quant_tbl_ptrs[2],
	  dcomp->quant_tbl_ptrs[3]);

  printf( "DC Tables: %p %p %p %p\n", 
	  dcomp->dc_huff_tbl_ptrs[0],
	  dcomp->dc_huff_tbl_ptrs[1],
	  dcomp->dc_huff_tbl_ptrs[2],
	  dcomp->dc_huff_tbl_ptrs[3]);
  printf( "AC Tables: %p %p %p %p\n", 
	  dcomp->ac_huff_tbl_ptrs[0],
	  dcomp->ac_huff_tbl_ptrs[1],
	  dcomp->ac_huff_tbl_ptrs[2],
	  dcomp->ac_huff_tbl_ptrs[3]);
}
#endif

static jmethodID allocateDataBufferID;
static jmethodID InputStream_readID;
static jmethodID InputStream_availableID;
static jmethodID InputStream_markSupportedID;
static jmethodID InputStream_markID;
static jmethodID InputStream_resetID;
static jmethodID InputStream_skipID;

static jfieldID  unpackID;
static jfieldID  flipID;

static jfieldID  rasID;
static jfieldID  biID;

/**
 *	Input Handling 
 *	//sun jpegdecoder explanations

 *	The jpeg library's input management is defined by the
 *	"jpeg_source_mgr" structure which contains two fields to
 *	convey the information in the buffer and 5 methods which
 *	perform all buffer management The library defines a standard
 *	input manager that uses stdio for obtaining compressed jpeg
 *	data, but here we need to use Java to get our data.  - we need
 *	to make the Java Class information accessible to the
 *	source_mgr input routines. We also need to store a pointer to
 *	the start of Java array being used as an input buffer so that
 *	it is not moved or garbage collected while the JPEGLibrary is
 *	using it.  To store these things, we make a private extension
 *	of the standard JPEG jpeg_source_mgr object 
 */
struct kodak_jpeg_source_mgr {
  struct	jpeg_source_mgr pub;	/* "public" fields */
  jobject	hInputStream;	// InputStream an input parameter
  jboolean	markSupported;	// does InputStream support mark/reset?
  int		suspendable;	// suspend JPEG decompression?	
  long		remaining_skip;	//

  JOCTET       *inbuf;		// Buffer to store data from the InputStream
  jbyteArray    hInputBuffer;	// Buffer to receive data from the InputStream
  ptrdiff_t	inbufoffset;	// current Offset into buffer
  int		buflen;		// The length of the buffer

  /* More stuff */
  union pixptr {
    long	   *ip; //for the buffer which data decompress from the buffer
    unsigned char  *bp;			//
  } outbuf;			//buffer which welcomes decompress data
  
  jobject		hOutputBuffer;

  jobjectArray          appMarkers;
  jobject               comMarker;

  jobject               buffImg;
  jobject               ras;

  jobject               err;

  JNIEnv		*env;
};

typedef struct kodak_jpeg_source_mgr * kodak_jpeg_source_ptr;

/* We use Get/ReleasePrimitiveArrayCritical functions to avoid
 * the need to copy buffer elements.
 *
 * MAKE SURE TO:
 *
 * - carefully insert pairs of RELEASE_ARRAYS and GET_ARRAYS around
 *   callbacks to Java.
 * - call RELEASE_ARRAYS before returning to Java.
 *
 * Otherwise things will go horribly wrong. There may be memory leaks,
 * excessive pinning, or even VM crashes!
 *
 * Note that GetPrimitiveArrayCritical may fail!
 */

/*******************************************************************************
*	Release the arrays that have been previously pinned...
*/
static void RELEASE_ARRAYS(JNIEnv *env, kodak_jpeg_source_ptr src)
{
  
  /* release the input buffer */
  if (src->inbuf) {
    if (src->pub.next_input_byte == 0) {
      src->inbufoffset = -1;
    } else {
      src->inbufoffset = src->pub.next_input_byte - src->inbuf;
    }
    (*env)->ReleasePrimitiveArrayCritical(env, src->hInputBuffer,
					  src->inbuf, 0);
    src->inbuf = 0;
  }

  /* release the output buffer */
  if (src->outbuf.ip) {
    (*env)->ReleasePrimitiveArrayCritical(env, src->hOutputBuffer,
					  src->outbuf.ip, 0);
    src->outbuf.ip = 0;
  }
}

/******************************************************************************
*	Get the arrays that have been previously unpinned
*/
static int GET_ARRAYS(JNIEnv *env, kodak_jpeg_source_ptr src)
{
  if (src->hInputBuffer) {
    assert(src->inbuf == 0);
    src->inbuf = (JOCTET *)(*env)->GetPrimitiveArrayCritical
      (env, src->hInputBuffer, 0);
    if (src->inbuf == 0) {
      return 0;
    }
    if (src->inbufoffset >= 0) {
      src->pub.next_input_byte = src->inbuf + src->inbufoffset;
    }
  }
  
  if (src->hOutputBuffer) {
    assert(src->outbuf.ip == 0);
    src->outbuf.ip = (long *)(*env)->GetPrimitiveArrayCritical
      (env, src->hOutputBuffer, 0);
    if (src->outbuf.ip == 0) {
      RELEASE_ARRAYS(env, src);
      return 0;
    }
  }
  
  return 1;
}

static void
unpackBuffer4(unsigned char *out, unsigned char *in, 
	      int width, jboolean flip)
{
  unsigned char *end = in+width*4;

  if (flip)
    while(in < end) {
      /* Build val... */
      register unsigned int val = ((in[0]<<16) |  /* Red */
				   (in[1]<<8 ) |  /* Grn */
				   (in[2]    ) |  /* Blu */
				   (in[3]<<24));  /* Alpha */
      
      /* XOr with FF is the same as flipping a two's compliment byte... */
      *(unsigned int *)out = val^0xFFFFFF;
      in+=4; out+=4;
    }
  else
    while(in < end) {
      register unsigned int val = ((in[0]<<16) |  /* Red */
				   (in[1]<<8 ) |  /* Grn */
				   (in[2]    ) |  /* Blu */
				   (in[3]<<24));  /* Alpha */
      
      *(unsigned int *)out = val;
      in+=4; out+=4;
    }
}

static void
unpackBuffer3(unsigned char *out, unsigned char *in, int width)
{
  /* point at the end of the input */
  unsigned char *inLine = in+(width*3)-1;

  /* point at the end of the unpacked data */
  unsigned int *outLine = (unsigned int *)(out+(width*4));
  outLine--;

  while (inLine > in)
    {
      register unsigned int val;
      val = *(inLine--);	/* Blu */
      val |= (*(inLine--))<<8;  /* Grn */
      val |= (*(inLine--))<<16; /* Red */
      *(outLine--) = val;
    }
}


/*******************************************************************************
 * Initialize source.  This is called by jpeg_read_header() before any
 * data is actually read. 
 */

GLOBAL(void)
kodak_jpeg_init_source(j_decompress_ptr cinfo)
{
  // src->jpeg_source_mgr
  kodak_jpeg_source_ptr src = (kodak_jpeg_source_ptr)cinfo->src;
  src->pub.next_input_byte = 0;	//next byte to read from buffer
  src->pub.bytes_in_buffer = 0;	//bytes remaining in buffer
}



/******************************************************************************
 * This is called whenever bytes_in_buffer has reached zero and more
 * data is wanted.  In typical applications, it should read fresh data
 * into the buffer (ignoring the current state of next_input_byte and
 * bytes_in_buffer), reset the pointer & count to the start of the
 * buffer, and return TRUE indicating that the buffer has been reloaded.
 * It is not necessary to fill the buffer entirely, only to obtain at
 * least one more byte.  bytes_in_buffer MUST be set to a positive value
 * if TRUE is returned.  A FALSE return should only be used when I/O
 * suspension is desired (this mode is discussed in the next section).
 
 * Note that with I/O suspension turned on, this procedure should not
 * do any work since the JPEG library has a very simple backtracking
 * mechanism which relies on the fact that the buffer will be filled
 * only when it has backed out to the top application level.  When
 * suspendable is turned on, the sun_jpeg_fill_suspended_buffer will
 * do the actual work of filling the buffer.
 */

GLOBAL(boolean)
kodak_jpeg_fill_input_buffer(j_decompress_ptr cinfo)
{
  //initialisation for the JNI environment
  int ret;
  int buflen;
  
  kodak_jpeg_source_ptr src = (kodak_jpeg_source_ptr) cinfo->src;
  
  if (src->suspendable) return FALSE;

  
  if (src->remaining_skip)
    src->pub.skip_input_data(cinfo, 0);

  // release for JNI rules
  RELEASE_ARRAYS(src->env, src);
  
  buflen = (*src->env)->GetArrayLength(src->env, src->hInputBuffer);

  if (src->markSupported) {
    (*src->env)->CallVoidMethod(src->env, src->hInputStream,
                                InputStream_markID, buflen + 1);
  }
  
  ret = (*src->env)->CallIntMethod(src->env, src->hInputStream,
				   InputStream_readID, 
				   src->hInputBuffer, 0, buflen);

  if (CheckExcept(src->env)) error_exit((j_common_ptr)cinfo);

  if (ret <= 0) {
    /* Throw an error for truncated JPEG files */
    if (src->buffImg)
      src->err = JNU_NewObjectByName
        (src->env, "com/sun/image/codec/jpeg/TruncatedFileException",
         "(Ljava/awt/image/BufferedImage;)V", src->buffImg);
    else
      src->err = JNU_NewObjectByName
        (src->env, "com/sun/image/codec/jpeg/TruncatedFileException",
         "(Ljava/awt/image/Raster;)V", src->ras);

    GET_ARRAYS(src->env, src);
    src->inbuf[0] = (JOCTET) 0xFF;
    src->inbuf[1] = (JOCTET) JPEG_EOI;
    ret = 2;
  } else
    GET_ARRAYS(src->env, src);
  


  src->pub.next_input_byte = src->inbuf;
  src->pub.bytes_in_buffer = ret;
  return TRUE;
}

	
/******************************************************************************
 * Skip num_bytes worth of data.  The buffer pointer and count should
 * be advanced over num_bytes input bytes, refilling the buffer as
 * needed.  This is used to skip over a potentially large amount of
 * uninteresting data (such as an APPn marker).  In some applications
 * it may be possible to optimize away the reading of the skipped
 * data, but it's not clear that being smart is worth much trouble;
 * large skips are uncommon.  bytes_in_buffer may be zero on return.
 * A zero or negative skip count should be treated as a no-op.  */

/*
 * Note that with I/O suspension turned on, this procedure should not
 * do any I/O since the JPEG library has a very simple backtracking
 * mechanism which relies on the fact that the buffer will be filled
 * only when it has backed out to the top application level.
 */

GLOBAL(void)
kodak_jpeg_skip_input_data(j_decompress_ptr cinfo, long num_bytes)
{
  int ret;
  int buflen;
  
  kodak_jpeg_source_ptr src = (kodak_jpeg_source_ptr) cinfo->src;
  if (num_bytes < 0) {
    return;
  }
  num_bytes += src->remaining_skip;
  src->remaining_skip = 0;
  ret = (int)src->pub.bytes_in_buffer;
  if (ret >= (int)num_bytes) {
    src->pub.next_input_byte += num_bytes;
    src->pub.bytes_in_buffer -= num_bytes;
    return;
  }
  num_bytes -= ret;
  if (src->suspendable) {
    src->remaining_skip = num_bytes;
    src->pub.bytes_in_buffer = 0;
    src->pub.next_input_byte = src->inbuf;
    return;
  }
  
  /* Note that the signature for the method indicates that it takes
   * and returns a long.  Casting the int num_bytes to a long on
   * the input should work well enough, and if we assume that the
   * return value for this particular method should always be less
   * than the argument value (or -1), then the return value coerced
   * to an int should return us the information we need...
   */
  // release for JNI rules
  RELEASE_ARRAYS(src->env, src);
  buflen =  (*src->env)->GetArrayLength(src->env, src->hInputBuffer);
  
  while (num_bytes > 0) {
    
    if (src->markSupported) {
        (*src->env)->CallVoidMethod(src->env, src->hInputStream,
                                    InputStream_markID, buflen + 1);
    }
  
    ret = (*src->env)->CallIntMethod(src->env, src->hInputStream,
				     InputStream_readID, 
				     src->hInputBuffer, 0, buflen);
    
    if( CheckNThrow(src->env, IO_EXCEPTION,
		    "reading encoded JPEG Stream "))
      error_exit((j_common_ptr)cinfo);
    
    if (ret < 0) break;

    num_bytes -= ret;
  }
  
  if (num_bytes > 0) {
    /* Throw an error for truncated JPEG files */
    if (src->buffImg)
      src->err = JNU_NewObjectByName
        (src->env, "com/sun/image/codec/jpeg/TruncatedFileException",
         "(Ljava/awt/image/BufferedImage;)V", src->buffImg);
    else
      src->err = JNU_NewObjectByName
        (src->env, "com/sun/image/codec/jpeg/TruncatedFileException",
         "(Ljava/awt/image/Raster;)V", src->ras);

    GET_ARRAYS(src->env, src);
    src->inbuf[0] = (JOCTET) 0xFF;
    src->inbuf[1] = (JOCTET) JPEG_EOI;
    src->pub.bytes_in_buffer = 2;
    src->pub.next_input_byte = src->inbuf;
  } else {
    GET_ARRAYS(src->env, src);

    src->pub.bytes_in_buffer = -num_bytes;
    src->pub.next_input_byte = src->inbuf + ret + num_bytes;
  }
}


/******************************************************************************
 * Terminate source --- called by jpeg_finish_decompress() after all
 * data has been read.  Often a no-op.
 */

GLOBAL(void)
kodak_jpeg_term_source(j_decompress_ptr cinfo)
{
}


METHODDEF(boolean)
kodak_jpeg_read_tag(j_decompress_ptr cinfo)
{
  int i;
  INT32 length=0;
  jobject vector;
  jbyteArray  markerData;
  jbyte      *markerDataPtr, *saveDataPtr=NULL;
  jboolean    hasExceptions=FALSE;

  int marker = cinfo->unread_marker;
  kodak_jpeg_source_ptr src = (kodak_jpeg_source_ptr) cinfo->src;

  
  if ((src->pub.bytes_in_buffer == 0) &&
      (!src->pub.fill_input_buffer(cinfo))) return FALSE;
  length = (*(src->pub.next_input_byte++));
  src->pub.bytes_in_buffer--;

  if ((src->pub.bytes_in_buffer == 0) &&
      (!src->pub.fill_input_buffer(cinfo))) return FALSE;
  length = (length<<8) | (*(src->pub.next_input_byte++));
  src->pub.bytes_in_buffer--;

  // printf("Marker: %d Length: %d\n", marker, length);

  length -= 2;  // kill the length bytes which we just read...

  RELEASE_ARRAYS(src->env, src);

  // Stick it into the param object...
  if (marker == JPEG_COM) 
    {
      if (src->comMarker == NULL)
	src->comMarker = 
	  JNU_NewObjectByName(src->env, "java/util/Vector", "(I)V", 1);
      if (CheckExcept(src->env)) goto fail;
      vector = src->comMarker;
    } 
  else if ((marker >= 0xE0) && (marker <= 0xEF))
    {
      
      jclass vectorC = (*(src->env))->FindClass(src->env, "java/util/Vector");
      if (src->appMarkers == NULL) {
	src->appMarkers = (*(src->env))->NewObjectArray(src->env, 16, 
							vectorC, NULL);
	if (CheckExcept(src->env)) goto fail;
      }

      vector = (*(src->env))->GetObjectArrayElement
	(src->env, src->appMarkers, (marker-JPEG_APP0));

      if (vector == NULL)
	{
	  vector = JNU_NewObjectByName(src->env, 
				       "java/util/Vector", "(I)V", 1);
	  if (CheckExcept(src->env)) goto fail;

	  (*(src->env))->SetObjectArrayElement
	    (src->env, src->appMarkers, (marker-JPEG_APP0), vector);
	  if (CheckExcept(src->env)) goto fail;
	}
    }
  else goto fail;
  
  if (length > 0) {
      // We do this a little backwards.  We create the array and stuff it into
      // Java and then we populate it with data.  This prevents an extra
      // Release/get cycle.

      // create the byte array that will hold the data.
      markerData    = (*(src->env))->NewByteArray(src->env, length);
      markerDataPtr = (*(src->env))->GetByteArrayElements(src->env, markerData, 0);
      saveDataPtr   = markerDataPtr;
      
      if (CheckExcept(src->env)) goto fail;

      JNU_CallMethodByName(src->env, &hasExceptions, vector,
                           "addElement", "(Ljava/lang/Object;)V",
                           markerData);
      GET_ARRAYS(src->env, src);

      // Fill the ARRAY with data...
      i=0;
      while(i<length)
      {
          size_t len;
          if ((src->pub.bytes_in_buffer == 0) &&
              (!src->pub.fill_input_buffer(cinfo))) {
              RELEASE_ARRAYS(src->env, src);
              (*(src->env))->ReleaseByteArrayElements(src->env, markerData, 
                                                      saveDataPtr, 0);
              GET_ARRAYS(src->env, src);
              return FALSE;
          }
          
          len = src->pub.bytes_in_buffer;
          if (len > (size_t)(length-i)) {
              len = length-i;
          }
          
          memcpy(markerDataPtr, src->pub.next_input_byte, len);
          
          // check for reading errors
          if (src->err != NULL) {
              // there's an error, stop reading input.
              break;
          }

          markerDataPtr            += len;
          src->pub.next_input_byte += len;
          src->pub.bytes_in_buffer -= len;
          i                        += len;
      }
      
      RELEASE_ARRAYS(src->env, src);
      (*(src->env))->ReleaseByteArrayElements(src->env, markerData, 
                                              saveDataPtr, 0);
  }
  GET_ARRAYS(src->env, src);
  
  return TRUE;

fail:
  if (saveDataPtr)
    (*(src->env))->ReleaseByteArrayElements(src->env, markerData, 
					    saveDataPtr, 0);
  GET_ARRAYS(src->env, src);

  return FALSE;
}

LOCAL(void)
CopyMarkersToJava(jpeg_param *jpp)
{
  int i;
  jvalue tmp;
  jboolean hasExceptions=FALSE;
  kodak_jpeg_source_ptr src = (kodak_jpeg_source_ptr) jpp->decompress->src;
  
  jobject vec = src->comMarker;
  tmp = JNU_CallStaticMethodByName(jpp->env, &hasExceptions, 
				   "sun/awt/image/codec/JPEGParam",
				   "buildArray","(Ljava/util/Vector;)[[B", 
				   vec);
  JNU_CallMethodByName(jpp->env, &hasExceptions, jpp->JPPObj,
		       "setMarkerData","(I[[B)V",
		       JPEG_COM, tmp.l);
  if (CheckExcept(jpp->env)) return; /* Exception was thrown. */

  // handle the sixteen APP markers
  for (i=0; i<16; i++)
    {
      vec = NULL;
      if (src->appMarkers != NULL)
	vec = (*(src->env))->GetObjectArrayElement
	  (src->env, src->appMarkers, i);

      tmp = JNU_CallStaticMethodByName(jpp->env, &hasExceptions, 
				       "sun/awt/image/codec/JPEGParam",
				       "buildArray","(Ljava/util/Vector;)[[B", 
				       vec);
      JNU_CallMethodByName(jpp->env, &hasExceptions, jpp->JPPObj,
			   "setMarkerData","(I[[B)V",
			   i+JPEG_APP0, tmp.l);
      if (CheckExcept(jpp->env)) return; /* Exception was thrown. */
    }
}

/******************************************************************************
 * initDecoder - this method will initialize some static variables that hold
 * the method ids on objects accessed from native code.
 */
JNIEXPORT void JNICALL 
Java_sun_awt_image_codec_JPEGImageDecoderImpl_initDecoder
(JNIEnv * env, jobject dec, jclass istr )
{
   jclass decoderCls = (*env)->GetObjectClass( env, dec );
   if( decoderCls == NULL ){
      JNU_ThrowByName( env, NULL_POINTER, "Initializing Decoder");
      return ;
   }

   unpackID     = (*env)->GetFieldID(env, decoderCls, "unpack", "Z");
   flipID       = (*env)->GetFieldID(env, decoderCls, "flip",   "Z");

   rasID        = (*env)->GetFieldID(env, decoderCls, "aRas",
                                     "Ljava/awt/image/WritableRaster;");
   biID         = (*env)->GetFieldID(env, decoderCls, "aBufImg",
                                     "Ljava/awt/image/BufferedImage;");

   allocateDataBufferID    = (*env)->GetMethodID(env, decoderCls, 
						 "allocateDataBuffer", 
						 "(III)Ljava/lang/Object;");
   InputStream_readID      = (*env)->GetMethodID(env, istr, "read", "([BII)I");
   InputStream_availableID = (*env)->GetMethodID(env, istr, 
						 "available", "()I");
   InputStream_markSupportedID = (*env)->GetMethodID(env, istr, "markSupported",
                                                     "()Z");
   InputStream_markID = (*env)->GetMethodID(env, istr, "mark", "(I)V");
   InputStream_resetID = (*env)->GetMethodID(env, istr, "reset", "()V");
   InputStream_skipID = (*env)->GetMethodID(env, istr, "skip", "(J)J");

   CheckNThrow(env, ILLEGAL_ARG, "Getting method ID's on Decoder init");
}



static int cleanup(j_decompress_ptr cinfo) {
    kodak_jpeg_source_ptr src = (kodak_jpeg_source_ptr) cinfo->src;

    if (!src->markSupported) {
        jpeg_destroy_decompress(cinfo);
        return 0;
    } else {
        jlong toSkip, skipped;

        if (src->inbufoffset == -1) {
            toSkip = 0;
        } else {
            toSkip = src->inbufoffset;
        }

        jpeg_destroy_decompress(cinfo);

        (*src->env)->CallVoidMethod(src->env, src->hInputStream,
                                    InputStream_resetID);
        if (CheckExcept(src->env)) return -1; /* Exception was thrown. */
        skipped = 0;
        while (skipped != toSkip) {
            jlong justSkipped = (*src->env)->CallLongMethod(src->env, 
                                                          src->hInputStream,
                                                          InputStream_skipID,
                                                          toSkip - skipped);
            if (CheckExcept(src->env)) return -1; /* Exception was thrown. */
            if (justSkipped == 0) break;  /* Probably EOF */
            skipped += justSkipped;
        }
        return 0;
    }
}


/******************************************************************************
 *
 *
 *   Java_sun_awt_image_codec_JPEGImageDecoderImpl_readJPEGStream
 * 
 *
 */
JNIEXPORT jobject JNICALL 
Java_sun_awt_image_codec_JPEGImageDecoderImpl_readJPEGStream(
   JNIEnv	*env,
   jobject	javaDecoderObj, 
   jobject	hInputStream, 
   jobject	jpegParamObj,
   jboolean	colCvt) 
{
  int        i;
  int        retVal;
  int        lineStride;
  jboolean   flip, unpack;
  JSAMPROW   scanLinePtr;  
  jbyteArray hInputBuffer;
  struct error_mgr ek_err;

  jpeg_param	jpp_info;		
  struct jpeg_decompress_struct cinfo;
  struct kodak_jpeg_source_mgr ksrc;

  /* check for NULL pointers */
  if(( javaDecoderObj == NULL ) || (hInputStream == NULL) ){
    JNU_ThrowByName( env, NULL_POINTER, "Writing JPEG Stream");
    return NULL;
  }
  /* set up the normal JPEGError routines, then override error_exit */
  
  cinfo.err = jpeg_std_error(&ek_err.pub);
  ek_err.pub.error_exit     = error_exit;
  ek_err.pub.output_message = jpeg_output_message;
  
  /* Establish the setjmp return context for my_error_exit to use. */
  if (setjmp(ek_err.setjmp_buffer))
    {
      jpeg_destroy_decompress(&cinfo);
      RELEASE_ARRAYS(env, &ksrc);
      if (!CheckExcept(env))
        {
          char buffer[JMSG_LENGTH_MAX];
          (*cinfo.err->format_message) ((struct jpeg_common_struct *) &cinfo,
                                        buffer);
          JNU_ThrowByName( env, IMAGE_FORMAT_EXCEPTION, buffer);
        }
      return NULL;
    }

  /* Now we can initialize the JPEG decompression object. */
  jpeg_create_decompress(&cinfo);

  /* Fix 4411325 - allocate space for comp_info */
  cinfo.comp_info =
    (jpeg_component_info *)
    (cinfo.mem->alloc_small) ((j_common_ptr) &cinfo, JPOOL_PERMANENT,
                              MAX_COMPONENTS * SIZEOF(jpeg_component_info));

  jpeg_set_marker_processor(&cinfo, JPEG_COM, kodak_jpeg_read_tag);
  for (i=0; i<16; i++)
    jpeg_set_marker_processor(&cinfo, i+JPEG_APP0, kodak_jpeg_read_tag);
    
  /* Set up the jpp_info struct and copy down info from Java. */
  jpp_info.env = env;
  jpp_info.decompress = &cinfo;
  jpp_info.compress = NULL;
  jpp_info.JPPObj = jpegParamObj;
  jpp_info.is_decompressor = TRUE;
  
  /* Copy the tables from the JPEGParam object passed in (if any)
   * If there are tables in the stream, they will override 
   */
  if(jpegParamObj != NULL)
    {
      CopyTablesFromJava(&jpp_info);
      if (CheckExcept(env)) return NULL;
      
      CopyCompInfoFromJava(&jpp_info);
      if (CheckExcept(env)) return NULL;
    }

  /* initialisation of the source manager	*/
  ksrc.env           = env;
  ksrc.outbuf.ip     = 0;
  ksrc.inbuf         = 0;
  ksrc.hOutputBuffer = 0;				
  ksrc.hInputStream  = hInputStream; /* InputStream passed as a parameter */
  ksrc.appMarkers    = NULL;
  ksrc.comMarker     = NULL;

  ksrc.markSupported = (*env)->CallBooleanMethod(env, hInputStream,
                       InputStream_markSupportedID);

  /* create and pin the buffer that will be used by the source 
   *  manager to get data from the InputStream 
   */
  hInputBuffer = (*env)->NewByteArray(env, STREAMBUF_SIZE);
  if (CheckExcept(env)) {
    jpeg_destroy_decompress(&cinfo);
    return NULL;
  }

  if ( hInputBuffer == NULL )
    {
      JNU_ThrowByName( env, OUT_OF_MEMORY, "Reading JPEG Stream");
      jpeg_destroy_decompress(&cinfo);
      return NULL;
    }

  ksrc.hInputBuffer = hInputBuffer;
  ksrc.buffImg      = (*env)->GetObjectField(env, javaDecoderObj, biID);
  ksrc.ras          = (*env)->GetObjectField(env, javaDecoderObj, rasID);
  ksrc.err          = NULL;
  
  GET_ARRAYS(env,&ksrc);
  /* complete CInfo initialization */
  cinfo.src = &ksrc.pub;	/* Source of compressed data */
  ksrc.suspendable = FALSE;
  ksrc.remaining_skip = 0;
  ksrc.inbufoffset = -1;
  ksrc.pub.init_source = kodak_jpeg_init_source;

  /* Initialize source, this is called by jpeg_read_header() */
  /* fill out the buffer */
  ksrc.pub.fill_input_buffer = kodak_jpeg_fill_input_buffer;
  ksrc.pub.skip_input_data   = kodak_jpeg_skip_input_data;
  ksrc.pub.resync_to_restart = jpeg_resync_to_restart; /* use default method */
  ksrc.pub.term_source       = kodak_jpeg_term_source;

  /* Initialize the struct used to communication with JPEGParam */
  
  /* read the jpeg header decide what to do based upon the value
   * returned */
  retVal =  jpeg_read_header(&cinfo, FALSE );
  /* printf("RetVal: %d\n", retVal); */

#if (defined EK_DEBUG || defined  EK_REG_TEST )
  displayDecompressStruct(&cinfo);
#endif

  RELEASE_ARRAYS(env, &ksrc);

  /* if we got an error while reading the header clean up and
   * throw it now... */
  if (ksrc.err) {
    jpeg_destroy_decompress(&cinfo);
    (*env)->Throw(env, ksrc.err);
    return NULL;
  }

  if(retVal == JPEG_HEADER_TABLES_ONLY){
    jboolean hasExceptions = FALSE;

    /* there is only header data to process no image data */
    jpp_info.image_data_present = FALSE;
    jpp_info.tables_present = TRUE;

    /* create a new JPEGParam object */
    jpp_info.JPPObj = JNU_NewObjectByName
      (env, "sun/awt/image/codec/JPEGParam", "(II)V",
       cinfo.jpeg_color_space, cinfo.num_components);
    if (CheckExcept(env)) return NULL;

    JNU_CallMethodByName(env, &hasExceptions, jpp_info.JPPObj,
			 "setTableInfoValid", "(Z)V", TRUE);
    if (CheckExcept(env)) return NULL; /* Exception was thrown. */

    JNU_CallMethodByName(env, &hasExceptions, jpp_info.JPPObj,
			 "setImageInfoValid", "(Z)V", FALSE);
    if (CheckExcept(env)) return NULL; /* Exception was thrown. */

    CopyTablesToJava  (&jpp_info);
    CopyCompInfoToJava(&jpp_info);
    CopyMarkersToJava (&jpp_info);

    if (cleanup(&cinfo) < 0)
        return NULL;

    return jpp_info.JPPObj;
  }
  
  // Create a new JPEGParam object a side effect a ColorModel
  // will be constructed if necessary as a private member of the
  // Java decoder.
  CInfoToJava(&jpp_info, javaDecoderObj, colCvt);
  if (CheckExcept(env)) return NULL;

  // if we are returning a scanline of data at a time don't need to call back 
  // into the decoder to allocate memory because our data does not have to be
  // embedded into a higher level object ( ex. a Raster or a BufferedImage )
  
  ksrc.hOutputBuffer = (*env)->CallObjectMethod(env, javaDecoderObj,
						allocateDataBufferID, 
						cinfo.image_width, 
						cinfo.image_height, 
						cinfo.num_components );
  if(CheckExcept(env)) return NULL;

  if(ksrc.hOutputBuffer == NULL ){
    JNU_ThrowByName( env, OUT_OF_MEMORY, "Allocating decoder output buffer");
    error_exit((j_common_ptr)&cinfo);
  }
  
  ksrc.buffImg = (*env)->GetObjectField(env, javaDecoderObj, biID);
  if (CheckExcept(env)) return NULL;

  ksrc.ras     = (*env)->GetObjectField(env, javaDecoderObj, rasID);
  if (CheckExcept(env)) return NULL;

  unpack       = (*env)->GetBooleanField(env, javaDecoderObj, unpackID);
  if (CheckExcept(env)) return NULL;

  flip         = (*env)->GetBooleanField(env, javaDecoderObj, flipID);
  if (CheckExcept(env)) return NULL;

  // calculate to lineStride for various and sundry purposes
  if (unpack)
    lineStride = cinfo.image_width * 4;
  else 
    lineStride = cinfo.image_width * cinfo.num_components;

  GET_ARRAYS(env, &ksrc);
  
  /* start decompressor */
  jpeg_start_decompress(&cinfo);
  
  scanLinePtr = (JSAMPROW)malloc(cinfo.image_width*cinfo.num_components);
  if (scanLinePtr == NULL) {
    jpeg_finish_decompress(&cinfo);
    RELEASE_ARRAYS(env, &ksrc);
    jpeg_destroy_decompress(&cinfo);
    JNU_ThrowByName( env, OUT_OF_MEMORY, "Reading JPEG Stream");
    return NULL;
  }
  
  /* We may need to do some setup of our own at this point before reading
   * the data.  After jpeg_start_decompress() we have the correct scaled
   * output image dimensions available, as well as the output colormap
   * if we asked for color quantization.
   */ 
    /* Here we use the library's state variable cinfo.output_scanline as the
     * loop counter, so that we don't have to keep track ourselves.
     */
    while(cinfo.output_scanline < cinfo.output_height) {
      unsigned char *ptr;
      jpeg_read_scanlines(&cinfo, &scanLinePtr, 1);

      /* we must use a temp buffer because the reading of data may
       * cause the Java array to move.  Since we can't update IJG
       * to point at the new stuff we need to give it an array that
       * can't move
       */
      ptr = ksrc.outbuf.bp + lineStride*(cinfo.output_scanline-1);
      if (unpack) {
	if (cinfo.num_components == 3)
	  unpackBuffer3(ptr, scanLinePtr, cinfo.image_width);
	else
	  unpackBuffer4(ptr, scanLinePtr, cinfo.image_width, flip);
      } else
	memcpy(ptr, scanLinePtr, cinfo.image_width * cinfo.num_components);

      /* if we got an error while decoding clean up and throw it now... */
      if (ksrc.err) {
        free(scanLinePtr);
        jpeg_destroy_decompress(&cinfo);
        RELEASE_ARRAYS(env, &ksrc);
        (*env)->Throw(env, ksrc.err);
        return NULL;
      }
    }// end while

    free(scanLinePtr);

    (void) jpeg_finish_decompress(&cinfo);

    RELEASE_ARRAYS(env, &ksrc);

    // put the markers we've read into JPEGParam object...
    CopyMarkersToJava(&jpp_info);

    if (cleanup(&cinfo) < 0)
        return NULL;

    return jpp_info.JPPObj;
}

