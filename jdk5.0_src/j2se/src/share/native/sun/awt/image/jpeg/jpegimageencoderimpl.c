/*
 * @(#)jpegimageencoderimpl.c	1.13 03/12/19
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

#include <stdio.h>
#include <assert.h>
#include <setjmp.h>
#include "jpeglib.h"
#include "jinclude.h"
#include "jpegparam.h"
#include "sun_awt_image_codec_JPEGImageEncoderImpl.h"

// forward declaration
LOCAL(void)
processJPEGStream(JNIEnv * env, jobject jpegEncoder,
                       jobject jpegParam, 
		       jobject colorModel, 
		       jobject outStream, 
		       jobject inBuf, jint start, jint lineStride);

#if (defined EK_REG_TEST || defined EK_DEBUG )
void displayCompressStruct( j_compress_ptr compress )
{
	printf( "JPEG compress struct State ... \n ");
	printf( "Width = %d \n", compress->image_width );
	printf( "Height = %d\n", compress->image_height );
	printf( "in color space = %d\n", compress->in_color_space );
	printf( "input components = %d\n", compress->input_components );
	printf( "jpeg colorspace  = %d\n", compress->jpeg_color_space );
	printf( "num components = %d\n", compress->num_components );
	printf( "data precision = %d\n", compress->data_precision );
	printf( "DCT Method = %d\n", compress->dct_method );
	printf( "raw_data_in = %d\n", compress->raw_data_in );
	printf( "optimize_coding = %d\n", compress->optimize_coding );
	printf( "CCIR601_sampling = %d\n", compress->CCIR601_sampling );
	printf( "write_JFIF_header = %d\n", compress->write_JFIF_header );
	printf( "density_unit = %d\n", compress->density_unit );
	printf( "X_density = %d\n", compress->X_density );
	printf( "Y_density = %d\n", compress->Y_density );
	printf( "input_gamma = %f\n", compress->input_gamma );
	printf( "restart_interval = %d\n", compress->restart_interval );
	printf( "restart_in_rows = %d\n", compress->restart_in_rows );
	printf( "sampling factor = %d\n", compress->smoothing_factor );
	printf( "global_state  = %d\n", compress->global_state );
}
#endif
/*
* Struct that defines the data items and buffers associated with 
* compressing using the lib.  This includes the usual public fields
* required by the library as well as the stuff needed to process data
* from an Java OutStream.  
*/
typedef struct {
  struct jpeg_destination_mgr pub; /* public fields */

  JNIEnv     *env;

  jbyteArray  hOutputBuffer;	/* The stream buffer of the output */
  JOCTET *    buffer;		/* start of buffer */
  ptrdiff_t   bufOffset;

  jobject     inBuf;
  jbyte      *inBufPtr;

  jobject     OutStream;	/* target stream */
} EK_destination_mgr;

typedef EK_destination_mgr *EK_dest_ptr;

static jmethodID OutputStream_writeID;
static jmethodID OutputStream_flushID;
static jfieldID  packID;

/****************************************************************************
* The arrays used to buffer for the JPEG lib and the Output Stream have
* been pinned.  If there is an error or if complete Release the arrays if
* they have been pinned
*/
void ReleaseArrays(j_compress_ptr cinfo){
  EK_dest_ptr dest = (EK_dest_ptr) cinfo->dest;
  
  /* Release the output buffer*/
  
  if (( dest->hOutputBuffer ) &&
      ( dest->buffer != 0 ))
    {
      if (dest->pub.next_output_byte == NULL)
	dest->bufOffset = -1;
      else
	dest->bufOffset = dest->pub.next_output_byte - dest->buffer;

      (*dest->env)->ReleasePrimitiveArrayCritical(dest->env, 
						  dest->hOutputBuffer,
						  (jbyte *)dest->buffer, 
						  0);
      dest->buffer = NULL;
    }

  if ((dest->inBuf) &&
      (dest->inBufPtr)) 
    {
      (*dest->env)->ReleasePrimitiveArrayCritical(dest->env, dest->inBuf,
						  dest->inBufPtr, JNI_ABORT);
      dest->inBufPtr = 0;
    }
}

static int GetArrays(j_compress_ptr cinfo){
  EK_dest_ptr dest = (EK_dest_ptr) cinfo->dest;

  /* Retrive native pointers to data */
  if (dest->hOutputBuffer) {
    assert(dest->buffer == NULL);

    dest->buffer = (JOCTET *)(*dest->env)->GetPrimitiveArrayCritical
      (dest->env, dest->hOutputBuffer, 0);

    if (dest->buffer == 0) return FALSE;

    if (dest->bufOffset >= 0)
      dest->pub.next_output_byte = dest->buffer + dest->bufOffset;
  }

  if (dest->inBuf) {
    assert(dest->inBufPtr == NULL);

    dest->inBufPtr = (jbyte *)(*dest->env)->GetPrimitiveArrayCritical
      (dest->env, dest->inBuf, 0);
    if (dest->inBufPtr == NULL) {
      ReleaseArrays(cinfo);
      return FALSE;
    }
  }

  return TRUE;
}

/****************************************************************************
 * Initialize destination --- called by jpeg_start_compress
 * before any data is actually written.
 */

METHODDEF(void)
init_mem_destination (j_compress_ptr cinfo)
{
  EK_dest_ptr dest = (EK_dest_ptr) cinfo->dest;

  dest->pub.next_output_byte = dest->buffer;
  dest->pub.free_in_buffer = STREAMBUF_SIZE;
}

/****************************************************************************
 * Empty the output buffer --- called whenever buffer fills up.
 *
 * In typical applications, this should write the entire output buffer
 * (ignoring the current state of next_output_byte & free_in_buffer),
 * reset the pointer & count to the start of the buffer, and return TRUE
 * indicating that the buffer has been dumped.
 *
 * In applications that need to be able to suspend compression due to output
 * overrun, a FALSE return indicates that the buffer cannot be emptied now.
 * In this situation, the compressor will return to its caller (possibly with
 * an indication that it has not accepted all the supplied scanlines).  The
 * application should resume compression after it has made more room in the
 * output buffer.  Note that there are substantial restrictions on the use of
 * suspension --- see the documentation.
 *
 * When suspending, the compressor will back up to a convenient restart point
 * (typically the start of the current MCU). next_output_byte & free_in_buffer
 * indicate where the restart point will be if the current call returns FALSE.
 * Data beyond this point will be regenerated after resumption, so do not
 * write it out when emptying the buffer externally.
 */


METHODDEF(boolean)
empty_mem_output_buffer (j_compress_ptr cinfo)
{
  EK_dest_ptr dest = (EK_dest_ptr) cinfo->dest;
  
  cinfo->bytes_in_buffer+=STREAMBUF_SIZE;

  // Push the data back into JAVA array...
  ReleaseArrays(cinfo);
  
  (*dest->env)->CallVoidMethod(dest->env, dest->OutStream,
			       OutputStream_writeID, 
			       dest->hOutputBuffer, 0, STREAMBUF_SIZE);
  
  if( CheckNThrow(dest->env, IO_EXCEPTION, 
		  "reading encoded JPEG Stream ")){
    error_exit((j_common_ptr)cinfo);
  }
  
  // make sure that all went OK
  if( CheckNThrow(dest->env, OUT_OF_MEMORY,
		  "No memory to initialize the JPEG encoder."))
    return FALSE; /* Error condition, and exception was thrown. */
  
  GetArrays(cinfo);
  
  dest->pub.next_output_byte = dest->buffer;
  dest->pub.free_in_buffer = STREAMBUF_SIZE;
  return TRUE;
}

/****************************************************************************
 *	After all of the data has been encoded there may still be some
 *	more left over in some of the working buffers.  Now is the
 *	time to clear them out.  */
METHODDEF(void)
term_mem_destination (j_compress_ptr cinfo)

{
  EK_dest_ptr dest = (EK_dest_ptr) cinfo->dest;

  /* find out how much needs to be written */
  jint datacount = STREAMBUF_SIZE - dest->pub.free_in_buffer;
  cinfo->bytes_in_buffer+=STREAMBUF_SIZE;
  
  ReleaseArrays(cinfo);
  
  (*dest->env)->CallVoidMethod(dest->env, dest->OutStream,
			       OutputStream_writeID, 
			       dest->hOutputBuffer, 0, datacount);
  
  if( CheckNThrow(dest->env, IO_EXCEPTION,	
		  "reading encoded JPEG Stream ")){
    error_exit((j_common_ptr)cinfo);
  }
  
  (*dest->env)->CallVoidMethod(dest->env, dest->OutStream,
			       OutputStream_flushID );
  
  if( CheckNThrow(dest->env, IO_EXCEPTION,
		  "reading encoded JPEG Stream ")){
    error_exit((j_common_ptr)cinfo);
  }
}

LOCAL(void)
writeMarker(jpeg_param *jpp, int marker, jobject datem, JNIEnv *env)
{
  int i;
  jint sz = (*env)->GetArrayLength(env, datem);
  if (CheckExcept(jpp->env)) return; /* Exception was thrown. */

  for (i=0; i<sz; i++) {
      jbyteArray  data;
      jint        dataLen;
      jbyte      *dataPtr;
      
      data = (*env)->GetObjectArrayElement(env, datem, i);
      if (CheckExcept(jpp->env)) return; /* Exception was thrown. */

      if (data != NULL) {
          dataPtr = (*env)->GetByteArrayElements(env, data, 0);
          dataLen = (*env)->GetArrayLength(env, data);
      
          GetArrays(jpp->compress);
          jpeg_write_marker(jpp->compress, marker,
			    (const JOCTET *)dataPtr, dataLen);
          ReleaseArrays(jpp->compress);
          (*env)->ReleaseByteArrayElements(env, data, dataPtr, JNI_ABORT);
      }
  }
}

GLOBAL(void)
writeMarkersFromJava(jpeg_param *jpp)
{
  int i;
  jvalue   tmp;
  jobject  vec;
  jboolean hasExceptions=FALSE;

  if (jpp->is_decompressor) return;

  ReleaseArrays(jpp->compress);

  for (i=0; i<16; i++)
    {
      tmp = JNU_CallMethodByName(jpp->env, &hasExceptions, jpp->JPPObj,
				 "getMarkerData", "(I)[[B",
				 i+JPEG_APP0);
      if (CheckExcept(jpp->env)) return; /* Exception was thrown. */
      vec = tmp.l;

      if (vec != NULL)
	writeMarker(jpp, i+JPEG_APP0, vec, jpp->env);
      if (CheckExcept(jpp->env)) return; /* Exception was thrown. */
    }

  tmp = JNU_CallMethodByName(jpp->env, &hasExceptions, jpp->JPPObj,
			     "getMarkerData", "(I)[[B",
			     JPEG_COM);
  if (CheckExcept(jpp->env)) return; /* Exception was thrown. */
  vec = tmp.l;
  
  if (vec != NULL)
    writeMarker(jpp, JPEG_COM, vec, jpp->env);

  GetArrays(jpp->compress);
}

static void
packBuffer3(unsigned char *out, 
	    unsigned char *in,
	    int width)
{
  unsigned char *end = in+width*4;
  while (in<end) {
    unsigned int val = *(unsigned int *)in;
    *(out++) = (val&0xFF0000) >> 16;
    *(out++) = (val&0xFF00)   >> 8;
    *(out++) = val&0xFF;
    in+=4;
  }
}

static void
packBuffer4(unsigned char *out, 
	    unsigned char *in,
	    int width)
{
  unsigned char *end = in+width*4;
  while (in<end) {
    unsigned int val = *(unsigned int *)in;
    *(out++) = (val&0xFF0000)   >> 16;
    *(out++) = (val&0xFF00)     >> 8;
    *(out++) = (val&0xFF);
    *(out++) = (val&0xFF000000) >> 24;
    in+=4;
  }
}


/****************************************************************************
 * Class:     Java_sun_awt_image_JPEGImageEncoder_writeJPEGStream
 * Method:    writeJPEGStream
 * Signature: (Ljava/io/OutputStream;[B)V
 */
JNIEXPORT void JNICALL 
Java_sun_awt_image_codec_JPEGImageEncoderImpl_writeJPEGStream
(JNIEnv * env, jobject jpegEncoder,
 jobject jpegParam, jobject colorModel, jobject outStream, 
 jbyteArray inBuf, jint start, jint lineStride)
{
  // this call implies that the data is not packed 
  processJPEGStream(env, jpegEncoder, jpegParam, colorModel, outStream, 
		    inBuf, start, lineStride);
}
/*******************************************************************************
	 * initEncoder - this method will initialize some static variables that hold
	 * the method ids on objects accessed from native code.
	 */

JNIEXPORT void JNICALL 
Java_sun_awt_image_codec_JPEGImageEncoderImpl_initEncoder
(JNIEnv * env, jobject enc, jclass istr )
{
   jclass encoderCls = (*env)->GetObjectClass( env, enc );
   if( encoderCls == NULL ){
      JNU_ThrowByName( env, NULL_POINTER, "Initializing Encoder");
      return ;
   }

  OutputStream_writeID = (*env)->GetMethodID(env, istr, "write", "([BII)V");
  OutputStream_flushID = (*env)->GetMethodID(env, istr, "flush", "()V");
  packID	       = (*env)->GetFieldID (env, encoderCls,  "pack", "Z");
}

/****************************************************************************
*/
void processJPEGStream(JNIEnv * env, jobject jpegEncoder, 
		       jobject jpegParam,
		       jobject colorModel,
                       jobject outStream,
		       jbyteArray inBuf, 
		       jint start, jint lineStride)
{
  JSAMPARRAY buffer;
  JSAMPROW scanLinePtr;
  jint  *intPtr;
  jboolean   pack;
  struct error_mgr ek_err;

  struct jpeg_compress_struct comp_info ;/* compression struct */
  EK_destination_mgr          dest;
  jpeg_param                  jpp_info;

  if( (jpegParam == NULL ) || (outStream == NULL) ){
    JNU_ThrowByName( env, NULL_POINTER, "Writing JPEG Stream");
    return;
  }

  /*
   *	Initialization
   */
  
  /* initialize the destination manager */
  dest.pub.init_destination    = init_mem_destination;
  dest.pub.empty_output_buffer = empty_mem_output_buffer;
  dest.pub.term_destination    = term_mem_destination;
  
  /* prepare for things to come */
  dest.env = env ; 
  
  dest.hOutputBuffer = 0;
  dest.buffer        = 0;
  dest.bufOffset     = -1;  
  
  dest.inBuf	     = inBuf;    
  dest.inBufPtr	     = NULL;    
  dest.OutStream     = outStream;

  dest.hOutputBuffer = (*env)->NewByteArray(env, STREAMBUF_SIZE);	 
  if( CheckNThrow(env, OUT_OF_MEMORY,
		  "No memory to initialize the JPEG encoder."))
    return; /* Error condition, and exception was thrown. */

  /* set up the normal JPEGError routines, then override error_exit */

  comp_info.err             = jpeg_std_error(&ek_err.pub);
  ek_err.pub.error_exit     = error_exit;
  ek_err.pub.output_message = jpeg_output_message;
  
  /* Establish the setjmp return context for my_error_exit to use. */
  if (setjmp(ek_err.setjmp_buffer)) {
    jpeg_destroy_compress(&comp_info);
    /*
     * now we need to clean up all the arrays that we prepared
     */
    ReleaseArrays(&comp_info);
    JNU_ThrowByName( env, IMAGE_FORMAT_EXCEPTION, "JPEG lib error");
    return;
  }
  
  jpeg_create_compress(&comp_info);
  
  comp_info.dest     = &(dest.pub); 

  /* initialize the struct used for accessing JPEGParam object */
  jpp_info.env = env;
  jpp_info.decompress = NULL;
  jpp_info.compress = &comp_info;
  jpp_info.JPPObj = jpegParam;
  jpp_info.is_decompressor = FALSE;
  
  /* copies the basic information about the data to be compressed to 
   *  the libraries compress info struct 
   */
  ImageInfoFromJava( &jpp_info, jpegEncoder, colorModel);
  if( CheckNThrow(env, IMAGE_FORMAT_EXCEPTION,
		  "Initializing CInfo for encodeing. "))
    return ; /* Error condition, and exception was thrown. */
  
  /* set the other library defaults */
  jpeg_set_defaults( &comp_info );
  
  /* 
   * let us override any of thier defaults because we know more about the
   * the data 
   */
  //JPEGParamToCInfo( &jpp_info );
  CInfoFromJava(&jpp_info, jpegEncoder, colorModel);
  if( CheckNThrow(env, IMAGE_FORMAT_EXCEPTION,
		  "Initializing CInfo for encoding. "))
    return ; /* Error condition, and exception was thrown. */

  pack = (*env)->GetBooleanField(env, jpegEncoder, packID);

#if (defined EK_DEBUG || defined EK_REG_TEST )
  displayCompressStruct((j_compress_ptr) &comp_info);
#endif

  GetArrays(&comp_info);

  //
  // if we don't want to write the data must mean we want to
  // write the tables.
  //
  if( jpp_info.image_data_present == FALSE){
    // write the tables 
    jpeg_write_tables(&comp_info);

    ReleaseArrays( &comp_info );
    jpeg_destroy_compress(&comp_info);
    return;
  }
  else{
    if(inBuf != NULL ){ // if we were given a non-null input buffer
      if(jpp_info.tables_present == TRUE ) {
	// write both tables and data
	jpeg_start_compress(&comp_info, TRUE);
      }
      else{// only write the image data
	jpeg_suppress_tables(&comp_info, TRUE);
	jpeg_start_compress(&comp_info, FALSE);
      }

      writeMarkersFromJava(&jpp_info);
    }
    else
      JNU_ThrowByName( env, NULL_POINTER, "Writing JPEG Stream");
  }

  scanLinePtr = (JSAMPROW)malloc(comp_info.image_width*
				 comp_info.num_components);
  if (scanLinePtr == NULL) {
    jpeg_finish_compress(&comp_info);
    ReleaseArrays(&comp_info);
    jpeg_destroy_compress(&comp_info);
    JNU_ThrowByName( env, OUT_OF_MEMORY, "Writing JPEG Stream");
    return;
  }
  
  /* Write the scanlines */
  while (comp_info.next_scanline < comp_info.image_height)
    {
      // We must copy because the process of writing the data out
      // may cause the image data buffer to move.. Since we can't
      // update all of the IJG's ptrs into that buffer we have
      // to copy the data into a buffer that won't move.
      if (pack) {
	unsigned char *ptr = (unsigned char *)dest.inBufPtr;
	ptr += (start+comp_info.next_scanline*lineStride)*4;

	if (comp_info.num_components == 3)
	  packBuffer3(scanLinePtr, ptr, comp_info.image_width);
	else
	  packBuffer4(scanLinePtr, ptr, comp_info.image_width);
      } else
	memcpy(scanLinePtr, 
	       dest.inBufPtr+start+comp_info.next_scanline*lineStride, 
	       comp_info.image_width*comp_info.num_components);

      jpeg_write_scanlines(&comp_info, (JSAMPARRAY)&scanLinePtr, 1);
    }

  free(scanLinePtr);


  /*
   * Clean up
   */
  jpeg_finish_compress(&comp_info);
  ReleaseArrays( &comp_info );
  jpeg_destroy_compress(&comp_info);
  return;		
}
