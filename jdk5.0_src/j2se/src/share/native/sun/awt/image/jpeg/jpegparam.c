/*
 * @(#)jpegparam.c	1.13 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/* ********************************************************************
 **********************************************************************
 *******************************************************************
 *** COPYRIGHT (c) Eastman Kodak Company, 1997-1998                      ***
 *** As  an unpublished  work pursuant to Title 17 of the United    ***
 *** States Code.  All rights reserved.                             ***
 **********************************************************************
 **********************************************************************
 **********************************************************************/

/*
*	JPEGParam.c
*
*	This file will implement methods that will be responsible for
*	accessing information to and from the JPEGParam java object.
*	This file will contain two methods getJPEGParam() which will 
*	transfer information from the JPEGParam object into the CInfo
*	struct ( decompress or compress ) and setJPEGParam() which will
*	take information from the CInfo struct and set the appropriate
*	fields in the JPEGParam object.
*	
*
*/
#include "jni.h"
#include "jni_util.h"

#include "jpeglib.h"
#include "jpegparam.h"

typedef struct error_mgr * error_ptr;

/*
 * This routine that will replace the standard error_exit method:
 */

GLOBAL(void)
error_exit (j_common_ptr cinfo)
{
  /* cinfo->err really points to a error_mgr struct */
  error_ptr myerr = (error_ptr) cinfo->err;
  // (*cinfo->err->output_message)(cinfo);
  /* Return control to the setjmp point */
  longjmp(myerr->setjmp_buffer, 1);
}

/*
 * Error Message handling
 *
 * This overrides the output_message method to send JPEG messages
 *
 */

GLOBAL(void)
jpeg_output_message (j_common_ptr cinfo)
{
  char buffer[JMSG_LENGTH_MAX];
  
  /* Create the message */
  (*cinfo->err->format_message) (cinfo, buffer);
  
  /* Send it to stderr, adding a newline */
  fprintf(stderr, "%s\n", buffer);
}

GLOBAL(boolean)
CheckExcept(JNIEnv *env)
{
  jthrowable exc = (*env)->ExceptionOccurred(env);
  if (exc) return TRUE;

  return FALSE;
}

/*
*	check for the existence of an exception thrown by another part of the  
*   the native operation.  If one is found then clear it and throw another
*   exception that we advertise that we will throw.  The exception class name
*   is the second parameter passed in the call.  Return a boolean so that we
*   know when it is OK to exit.
*/
GLOBAL(boolean)
CheckNThrow(JNIEnv *env, const char *name, const char *msg)
{
  jthrowable exc = (*env)->ExceptionOccurred(env);

  if (exc) {
    /* we let the calling java methods catch all exceptions we return  */    
    (*env)->ExceptionClear(env);
    JNU_ThrowByName(env, name, msg );
    return TRUE;
  }

  return FALSE;
}

/************************************************************************************
 * Component information
 */

GLOBAL(void)
CopyCompInfoToJava ( jpeg_param *jpp )
{
  int i, numComp;
  int hMax=0, vMax=0;
  jpeg_component_info *comps;
  jboolean hasExceptions = FALSE;

  if( jpp->is_decompressor ) {
    numComp = jpp->decompress->num_components;
    comps   = jpp->decompress->comp_info;
  } else {
    numComp = jpp->compress->num_components;
    comps   = jpp->compress->comp_info;
  }    


  // Copy the Q and Huffman Table mappings over.
  // Determine the Max Horz&Vert sampling factor
  for (i=0; i<numComp; i++)
    {
      if (comps[i].h_samp_factor > hMax) hMax = comps[i].h_samp_factor;
      if (comps[i].v_samp_factor > vMax) vMax = comps[i].v_samp_factor;
    }

  if ((numComp!=0) &&
      ((hMax == 0) || (vMax == 0)))
    { 
      JNU_ThrowByName(jpp->env, IMAGE_FORMAT_EXCEPTION, 
		      "JPEGParam, zero sub-sample factors");
      return;
    }

  for (i=0; i<numComp; i++)
    {
      jint val;
      jint indx = i;

      val  = comps[i].quant_tbl_no;
      JNU_CallMethodByName(jpp->env, &hasExceptions, jpp->JPPObj,
			   "setQTableComponentMapping", "(II)V", 
			   indx, val);
      if (CheckExcept(jpp->env)) return; /* Exception was thrown. */
      
      val  = comps[i].dc_tbl_no;
      JNU_CallMethodByName(jpp->env, &hasExceptions, jpp->JPPObj,
			   "setDCHuffmanComponentMapping", "(II)V", 
			   indx, val);
      if (CheckExcept(jpp->env)) return; /* Exception was thrown. */
  
      val  = comps[i].ac_tbl_no;
      JNU_CallMethodByName(jpp->env, &hasExceptions, jpp->JPPObj,
			   "setACHuffmanComponentMapping", "(II)V", 
			   indx, val);
      if (CheckExcept(jpp->env)) return; /* Exception was thrown. */
  

      val = hMax/comps[i].h_samp_factor;
      JNU_CallMethodByName(jpp->env, &hasExceptions, jpp->JPPObj,
			   "setHorizontalSubsampling", "(II)V", 
			   indx,  val);
      if (CheckExcept(jpp->env)) return; /* Exception was thrown. */

      val = vMax/comps[i].v_samp_factor;
      JNU_CallMethodByName(jpp->env, &hasExceptions, jpp->JPPObj,
			   "setVerticalSubsampling", "(II)V", 
			   indx,  val);
      if (CheckExcept(jpp->env)) return; /* Exception was thrown. */
    }
}

GLOBAL(void)
CopyCompInfoFromJava(jpeg_param *jpp)
{
  int i, numComp, colorID;
  int hMax=0, vMax=0;
  jpeg_component_info *comps;
  jboolean hasExceptions = FALSE;
  jvalue tmp;

  tmp = JNU_CallMethodByName(jpp->env, &hasExceptions, jpp->JPPObj,
			     "getNumComponents", "()I");
  if (CheckExcept(jpp->env)) return; /* Exception was thrown. */
  numComp = tmp.i;

  tmp = JNU_CallMethodByName(jpp->env, &hasExceptions, jpp->JPPObj, 
			     "getEncodedColorID", "()I");
  if (CheckExcept(jpp->env)) return; /* Exception was thrown. */
  colorID = tmp.i;

  if( jpp->is_decompressor ) {
    jpp->decompress->num_components   = numComp;
    jpp->decompress->jpeg_color_space = colorID;
    comps   = jpp->decompress->comp_info;
  } else {
    jpp->compress->num_components   = numComp;
    jpp->compress->jpeg_color_space = colorID;
    comps   = jpp->compress->comp_info;
  }    
  
  for (i=0; i<numComp; i++)
    {
      jint indx = i;
      jvalue tmp;
      
      tmp = JNU_CallMethodByName(jpp->env, &hasExceptions, jpp->JPPObj,
				 "getQTableComponentMapping", "(I)I", indx);
      if (CheckExcept(jpp->env)) return; /* Exception was thrown. */
      comps[i].quant_tbl_no =  tmp.i;
      
      tmp = JNU_CallMethodByName(jpp->env, &hasExceptions, jpp->JPPObj,
				 "getDCHuffmanComponentMapping", "(I)I", indx);
      if (CheckExcept(jpp->env)) return; /* Exception was thrown. */
      comps[i].dc_tbl_no = tmp.i;
      
      tmp = JNU_CallMethodByName(jpp->env, &hasExceptions, jpp->JPPObj,
				 "getACHuffmanComponentMapping", "(I)I", indx);
      if (CheckExcept(jpp->env)) return; /* Exception was thrown. */
      comps[i].ac_tbl_no = tmp.i;
      
      tmp = JNU_CallMethodByName(jpp->env, &hasExceptions, jpp->JPPObj,
				 "getHorizontalSubsampling", "(I)I", indx);
      if (CheckExcept(jpp->env)) return; /* Exception was thrown. */
      
      comps[i].h_samp_factor = tmp.i;
      if (tmp.i > hMax) hMax = tmp.i;
      
      tmp = JNU_CallMethodByName(jpp->env, &hasExceptions, jpp->JPPObj,
				 "getVerticalSubsampling", "(I)I", indx);
      if (CheckExcept(jpp->env)) return; /* Exception was thrown. */
      
      comps[i].v_samp_factor = tmp.i;
      if (tmp.i > vMax) vMax = tmp.i;
    }
  
  if ((numComp != 0) &&
      ((hMax == 0) || (vMax == 0)))
    { 
      JNU_ThrowByName(jpp->env, IMAGE_FORMAT_EXCEPTION, 
		      "JPEGParam, zero sub-sample factors");
      /* clean up the h/v samp factors... */
      for (i=0; i<numComp; i++)
	{
	  comps[i].h_samp_factor = 1;
	  comps[i].v_samp_factor = 1;
	}
      return;
    }
  
  for (i=0; i<numComp; i++)
    {
      comps[i].h_samp_factor = hMax/comps[i].h_samp_factor;
      comps[i].v_samp_factor = vMax/comps[i].v_samp_factor;
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//
//				HuffmanTables
//
//
void
CopyHTablesToJava( jpeg_param *jpp,  int tableNum)
{
  int i;
  JHUFF_TBL  *ac, *dc;
  jobject     acTable, dcTable;
  jshortArray ac_bitsTable, dc_bitsTable, ac_symbolsTable, dc_symbolsTable;
  jshort      *acBitsPtr, *dcBitsPtr, *acValPtr, *dcValPtr ;
  jboolean hasExceptions = FALSE;

  if( jpp->is_decompressor ) {
    dc = jpp->decompress->dc_huff_tbl_ptrs[tableNum];
    ac = jpp->decompress->ac_huff_tbl_ptrs[tableNum];
  } else {
    dc = jpp->compress->dc_huff_tbl_ptrs[tableNum];
    ac = jpp->compress->ac_huff_tbl_ptrs[tableNum];
  }

  // nothing to do if there are no tables
  if( (ac == NULL ) || (dc == NULL )) return;
  
  // create new jshort arrays for each of the arrays in each of the
  // huffman table object.  create JPEGHuffmanTable objects for ac
  // and dc passing the arrays in the constructors invoke the
  // JPEGParam setHuffmanTables mehtod to set the tables
  
  // create the jshort objects that will hold the arrays
  ac_bitsTable =    (*(jpp->env))->NewShortArray(jpp->env, BITS_TABLE_LEN);
  dc_bitsTable =    (*(jpp->env))->NewShortArray(jpp->env, BITS_TABLE_LEN);
  ac_symbolsTable = (*(jpp->env))->NewShortArray(jpp->env, SYMBOLS_TABLE_LEN);
  dc_symbolsTable = (*(jpp->env))->NewShortArray(jpp->env, SYMBOLS_TABLE_LEN);
  
  // copy the array values from the cInfo to the newly allocated arrays
  acBitsPtr=(*(jpp->env))->GetShortArrayElements(jpp->env, ac_bitsTable, 0);
  dcBitsPtr=(*(jpp->env))->GetShortArrayElements(jpp->env, dc_bitsTable, 0);
  acValPtr =(*(jpp->env))->GetShortArrayElements(jpp->env, ac_symbolsTable, 0);
  dcValPtr =(*(jpp->env))->GetShortArrayElements(jpp->env, dc_symbolsTable, 0);

  // copy the tables
  for(i = 0 ; i < BITS_TABLE_LEN; i++){
    acBitsPtr[i] = ac->bits[i]; 
    dcBitsPtr[i] = dc->bits[i] ;
  }
  for(i = 0 ; i < SYMBOLS_TABLE_LEN; i++){
    acValPtr[i] = ac->huffval[i];
    dcValPtr[i] = dc->huffval[i];
  }
		
  (*(jpp->env))->ReleaseShortArrayElements(jpp->env,  ac_bitsTable, 
					   acBitsPtr, 0);
  (*(jpp->env))->ReleaseShortArrayElements(jpp->env, dc_bitsTable, 
					   dcBitsPtr, 0);

  (*(jpp->env))->ReleaseShortArrayElements(jpp->env, ac_symbolsTable, 
					   acValPtr, 0);
  (*(jpp->env))->ReleaseShortArrayElements(jpp->env, dc_symbolsTable, 
					   dcValPtr, 0);

  /* create the acHuffmanTable */
  acTable = JNU_NewObjectByName(jpp->env, 
                                "com/sun/image/codec/jpeg/JPEGHuffmanTable", 
                                "([S[S)V", ac_bitsTable, ac_symbolsTable );

  if( CheckNThrow(jpp->env, NULL_POINTER, 
		  "Could't create a JPEGHuffmanTable object"))
    return; /* Error condition, and exception was thrown. */
  
  /* create the dc huffman table object */
  dcTable = JNU_NewObjectByName(jpp->env, 
                                "com/sun/image/codec/jpeg/JPEGHuffmanTable", 
                                "([S[S)V", dc_bitsTable, dc_symbolsTable );

  if (CheckNThrow(jpp->env, NULL_POINTER, 
		  "Could't create a JPEGHuffmanTable object"))
    return; /* Error condition, and exception was thrown. */

  /* set the tables via call on the JPEGParam object */
  if ((dcTable == 0) || (acTable == 0))
    {
      JNU_ThrowByName(jpp->env, NULL_POINTER, 
		      "Error creating JPEGHuffmanTable objects");
      return;
    }

  JNU_CallMethodByName(jpp->env, &hasExceptions, jpp->JPPObj,
		       "setDCHuffmanTable",
		       "(ILcom/sun/image/codec/jpeg/JPEGHuffmanTable;)V", 
		       tableNum, dcTable );
  if (CheckExcept(jpp->env)) return; /* Exception was thrown. */
  
  JNU_CallMethodByName(jpp->env, &hasExceptions, jpp->JPPObj,
		       "setACHuffmanTable",
		       "(ILcom/sun/image/codec/jpeg/JPEGHuffmanTable;)V", 
		       tableNum, acTable );
  if (CheckExcept(jpp->env)) return; /* Exception was thrown. */
}

void
CopyHTablesFromJava( jpeg_param * jpp,  jint tableNum)
{
  int i;
  JHUFF_TBL  *ac, *dc;
  jvalue      tmp;
  jobject     acTable, dcTable;
  jshortArray ac_bitsTable, dc_bitsTable, ac_symbolsTable, dc_symbolsTable;
  jshort      *acBitsPtr, *dcBitsPtr, *acValPtr, *dcValPtr ;
  jint        acBitsLen, dcBitsLen, acSymsLen, dcSymsLen;
  jboolean    hasExceptions = FALSE;

  // get the ac and the dc JPEGHuffmanTable objects from JPEGParam 
  // dc 
  tmp = JNU_CallMethodByName(jpp->env, &hasExceptions, jpp->JPPObj,
                             "getDCHuffmanTable",
                             "(I)Lcom/sun/image/codec/jpeg/JPEGHuffmanTable;", 
                             tableNum);
  dcTable = tmp.l;

  if (CheckExcept(jpp->env)) return; /* Exception was thrown. */

  // ac
  tmp = JNU_CallMethodByName(jpp->env, &hasExceptions, jpp->JPPObj,
                             "getACHuffmanTable",
                             "(I)Lcom/sun/image/codec/jpeg/JPEGHuffmanTable;", 
                             tableNum);
  acTable = tmp.l;

  if (CheckExcept(jpp->env)) return; /* Exception was thrown. */

		
  /* if the dc or the ac table are null then return, no work to do here 
   * note: we only accept situations where ac and dc tables are provided 
   * together
   */
  if (( acTable == NULL ) || (dcTable == NULL ))
    return;

  /*  get the tables from each of the JPEGHuffmanTableObjects */

  // dc bits array
  tmp = JNU_CallMethodByName(jpp->env, &hasExceptions, 
                             dcTable,"getLengths","()[S");
  dc_bitsTable = (jshortArray)tmp.l;
  if (CheckExcept(jpp->env)) return; /* Exception was thrown. */

  // dc symbols array
  tmp = JNU_CallMethodByName(jpp->env, &hasExceptions, 
                             dcTable,"getSymbols","()[S");
  dc_symbolsTable = (jshortArray)tmp.l;
  if (CheckExcept(jpp->env)) return; /* Exception was thrown. */

  // ac bits array
  tmp = JNU_CallMethodByName(jpp->env, &hasExceptions, 
                             acTable,"getLengths","()[S");
  ac_bitsTable = (jshortArray)tmp.l;
  if (CheckExcept(jpp->env)) return; /* Exception was thrown. */

  // ac symbols array
  tmp = JNU_CallMethodByName(jpp->env, &hasExceptions, 
                             acTable,"getSymbols","()[S");
  ac_symbolsTable = (jshortArray)tmp.l;
  if (CheckExcept(jpp->env)) return; /* Exception was thrown. */

    // check the lengths of the bits tables retrieved for validity
  acBitsLen = (*(jpp->env))->GetArrayLength(jpp->env, ac_bitsTable);
  dcBitsLen = (*(jpp->env))->GetArrayLength(jpp->env, dc_bitsTable);
  acSymsLen = (*(jpp->env))->GetArrayLength(jpp->env, ac_symbolsTable);
  dcSymsLen = (*(jpp->env))->GetArrayLength(jpp->env, dc_symbolsTable);
  if ((acBitsLen > BITS_TABLE_LEN) || 
      (dcBitsLen > BITS_TABLE_LEN))
    { 
      JNU_ThrowByName(jpp->env, ARRAY_BOUNDS_ERROR, 
		      "Huffman bits Table is to long.");
      return;
    }
  
  if ((acSymsLen > SYMBOLS_TABLE_LEN) || 
      (dcSymsLen > SYMBOLS_TABLE_LEN))
    {
      JNU_ThrowByName(jpp->env, ARRAY_BOUNDS_ERROR,
		      "Huffman symbols Tables is to long.");
      return;
    }

  /* get pointers to the actual data in the jshortArray objects */
  acBitsPtr=(*(jpp->env))->GetShortArrayElements(jpp->env, ac_bitsTable, 0);
  dcBitsPtr=(*(jpp->env))->GetShortArrayElements(jpp->env, dc_bitsTable, 0);
  acValPtr =(*(jpp->env))->GetShortArrayElements(jpp->env, ac_symbolsTable,0);
  dcValPtr =(*(jpp->env))->GetShortArrayElements(jpp->env, dc_symbolsTable,0);

  if (jpp->is_decompressor) 
    {
      dc = jpp->decompress->dc_huff_tbl_ptrs[tableNum];
      ac = jpp->decompress->ac_huff_tbl_ptrs[tableNum];

      // We didn't have tables already allocated, so allocate new tables.
      if ( dc == NULL ) 
	{
	  dc = jpeg_alloc_huff_table((j_common_ptr)jpp->decompress);
	  ac = jpeg_alloc_huff_table((j_common_ptr)jpp->decompress);

	  jpp->decompress->dc_huff_tbl_ptrs[tableNum] = dc;
	  jpp->decompress->ac_huff_tbl_ptrs[tableNum] = ac;
	}
    } else {
      dc = jpp->compress->dc_huff_tbl_ptrs[tableNum];
      ac = jpp->compress->ac_huff_tbl_ptrs[tableNum];
    }

  // copy the tables...
  for( i = 0 ; i < BITS_TABLE_LEN; i++){
    if (i<acBitsLen) ac->bits[i] = (UINT8)acBitsPtr[i];
    else             ac->bits[i] = 0;

    if (i<dcBitsLen) dc->bits[i] = (UINT8)dcBitsPtr[i];
    else             dc->bits[i] = 0;
  }

  for( i = 0 ; i < SYMBOLS_TABLE_LEN; i++){
    if (i<acSymsLen) ac->huffval[i] = (UINT8)acValPtr[i];
    else             ac->huffval[i] = 0;
    if (i<dcSymsLen) dc->huffval[i] = (UINT8)dcValPtr[i];
    else             dc->huffval[i] = 0;
  }

  (*(jpp->env))->ReleaseShortArrayElements(jpp->env, ac_bitsTable, 
					   acBitsPtr, JNI_ABORT);
  (*(jpp->env))->ReleaseShortArrayElements(jpp->env, dc_bitsTable, 
					   dcBitsPtr, JNI_ABORT);

  (*(jpp->env))->ReleaseShortArrayElements(jpp->env, ac_symbolsTable, 
					   acValPtr, JNI_ABORT);
  (*(jpp->env))->ReleaseShortArrayElements(jpp->env, dc_symbolsTable, 
					   dcValPtr, JNI_ABORT);

}

//////////////////////////////////////////////////////////////////////////////
//
//			QTables
//

/*
 * jpeg_zigzag_order[i] is the zigzag-order position of the i'th element
 * of a DCT block read in natural order (left to right, top to bottom).
 */
static int jpeg_zigzag_order[DCTSIZE2] = {
   0,  1,  5,  6, 14, 15, 27, 28,
   2,  4,  7, 13, 16, 26, 29, 42,
   3,  8, 12, 17, 25, 30, 41, 43,
   9, 11, 18, 24, 31, 40, 44, 53,
  10, 19, 23, 32, 39, 45, 52, 54,
  20, 22, 33, 38, 46, 51, 55, 60,
  21, 34, 37, 47, 50, 56, 59, 61,
  35, 36, 48, 49, 57, 58, 62, 63
};

void
CopyQTablesToJava( jpeg_param *jpp, int tableNum)
{
  int i;
  JQUANT_TBL *jQTable;
  jintArray   qTable;
  jobject     qTableObj;
  jint	     *body;
  jboolean    hasExceptions = FALSE;

  if( jpp->is_decompressor)
    jQTable = jpp->decompress->quant_tbl_ptrs[tableNum];
  else
    jQTable = jpp->compress->quant_tbl_ptrs[tableNum];

  // don't need to copy a null table
  if (jQTable == NULL) return;
  
  // allocate a jint array and copy the current table into that array
  qTable = (*(jpp->env))->NewIntArray(jpp->env, DCTSIZE2 );

  // get a buffer to store the values
  body = (*(jpp->env))->GetIntArrayElements(jpp->env, qTable, 0);

  // Convert from natural to zig-zag order, for compatibility
  for( i = 0; i < DCTSIZE2 ; i++){
    body[jpeg_zigzag_order[i]] = jQTable->quantval[i] ;
  } 

  (*(jpp->env))->ReleaseIntArrayElements(jpp->env, qTable, body, 0);

  
  // create a new QTable object
  qTableObj = JNU_NewObjectByName(jpp->env, 
				  "com/sun/image/codec/jpeg/JPEGQTable", "([I)V", 
				  qTable );
  if( CheckNThrow(jpp->env, NULL_POINTER, "Could't create a JPEGQtable object"))
    return; /* Error condition, and exception was thrown. */
  
  // if the table was correctly created then set the table to the value of the
  // table that was copied from the cinfo struct
  if( qTableObj != NULL )
    JNU_CallMethodByName(jpp->env, &hasExceptions, jpp->JPPObj, 
			 "setQTable",
			 "(ILcom/sun/image/codec/jpeg/JPEGQTable;)V", 
			 tableNum, qTableObj);
}

void
CopyQTablesFromJava( jpeg_param *jpp, int tableNum)
{
  int i;
  jvalue     tmp;
  jobject    qTableObj;
  jintArray  qTable;
  jint      *body;
  jboolean   hasExceptions = FALSE;
  unsigned int temp[DCTSIZE2];

  // get the QTable from JPEGParam
  tmp = JNU_CallMethodByName(jpp->env, &hasExceptions, jpp->JPPObj, 
			     "getQTable",
			     "(I)Lcom/sun/image/codec/jpeg/JPEGQTable;",
			     tableNum);
  if (tmp.l == NULL ) return;  // if nothing just return...
  qTableObj = tmp.l;
  
  // get the actual table from the QTable object
  tmp = JNU_CallMethodByName(jpp->env, &hasExceptions, qTableObj, 
			     "getTable","()[I");
  if (tmp.l == NULL ) return;  // if nothing just return...
  qTable = (jintArray) tmp.l;
  
  // Copy over the 
  body = (*(jpp->env))->GetIntArrayElements(jpp->env, qTable, 0);
  if ( jpp->is_decompressor ) {
    UINT16     *ptr;
    JQUANT_TBL *jQTable    = jpp->decompress->quant_tbl_ptrs[tableNum];
    jsize      arraylength = (*(jpp->env))->GetArrayLength(jpp->env, qTable);
    
    if (jQTable == NULL) 
      {
	jQTable = jpeg_alloc_quant_table((j_common_ptr) jpp->decompress);
	jpp->decompress->quant_tbl_ptrs[tableNum] = jQTable;
      }
    
    ptr = jQTable->quantval;
    for( i = 0; i < arraylength; i++ )
      {
	ptr[i] =(UINT16) body[jpeg_zigzag_order[i]];
      }
  } else {
      // Convert from zig_zag order first
      for (i = 0; i < DCTSIZE2; i++)
          temp[i] = ((unsigned int *)body)[jpeg_zigzag_order[i]];
    // need to pass in the value of the forceBaseline var as well
    jpeg_add_quant_table (jpp->compress, tableNum, 
			  &temp[0], 100, TRUE );
  }
  (*(jpp->env))->ReleaseIntArrayElements(jpp->env, qTable, body, JNI_ABORT);
}

GLOBAL(void) 
CopyTablesToJava( jpeg_param * jpp)
{
  int i;
  if( CheckPtrs( jpp )  == FALSE ) return;  /* an exception has been thrown */

  /*	QTables */
  for (i = 0; i<NUM_QUANT_TBLS; i++ ) 
    {
      CopyQTablesToJava(jpp, i);
      if (CheckExcept(jpp->env)) return; /* Exception was thrown. */
    }
  
  /* HuffmanTables  */
  for (i = 0; i < NUM_HUFF_TBLS; i++ )
    {
      CopyHTablesToJava( jpp, i );
      if (CheckExcept(jpp->env)) return; /* Exception was thrown. */
    }
}

GLOBAL(void) 
CopyTablesFromJava( jpeg_param * jpp)
{
  int i;
  if( CheckPtrs( jpp )  == FALSE ) return;  /* an exception has been thrown */

  /*	QTables */
  for (i = 0; i<NUM_QUANT_TBLS; i++ ) 
    {
      CopyQTablesFromJava(jpp, i);
      if (CheckExcept(jpp->env)) return; /* Exception was thrown. */
    }
  
  /* HuffmanTables  */
  for (i = 0; i < NUM_HUFF_TBLS; i++ )
    {
      CopyHTablesFromJava( jpp, i );
      if (CheckExcept(jpp->env)) return; /* Exception was thrown. */
    }
}

/****************************************************************************
 *	External intended to be called from native jpeg decoder code
 ****************************************************************************/
GLOBAL(void)
CInfoToJava(jpeg_param * jpp, 
	    jobject      jpegImageDecoder, 
	    jboolean     convert)
{
  jobject newJPPObj;
  jboolean   hasExceptions=FALSE;

  /* make sure that we have no NULLs */
  if (jpp->env == NULL) 
    return;

  /* printf("In CInfoToJava\n"); */

  jpp->JPPObj = createJPPFromCInfo ( jpp, jpegImageDecoder, convert );
  if (CheckExcept(jpp->env)) return; /* Exception was thrown. */
  
  CopyTablesToJava  ( jpp ); /* set the tables that are found in JPEGParam */
  if (CheckExcept(jpp->env)) return; /* Exception was thrown. */

  CopyCompInfoToJava( jpp ); /* set the per/component info in JPEGParam */
  if (CheckExcept(jpp->env)) return; /* Exception was thrown. */
	
  if( jpp->is_decompressor ){
    jint tmpInt;
    jboolean flag;

    // restart_interval
    tmpInt = jpp->decompress->restart_interval;
    JNU_CallMethodByName(jpp->env, &hasExceptions, jpp->JPPObj,
			 "setRestartInterval", "(I)V", tmpInt);
    if (CheckExcept(jpp->env)) return; /* Exception was thrown. */
  }
}

//////////////////////////////////////////////////////////////////////////////
GLOBAL(void)
CInfoFromJava (jpeg_param *jpp, 
	       jobject     jpegImageEncoder,
	       jobject     colorModel)
{
  jvalue tmpVal;
  int	   dctMethod;
  jboolean hasExceptions=FALSE;

  /* make sure that we have no NULLs */
  if( CheckPtrs( jpp )  == FALSE )
    return;  /* an exception has been thrown */

  ImageInfoFromJava   ( jpp, jpegImageEncoder, colorModel );
  if (CheckExcept(jpp->env)) return; /* Exception was thrown. */

  CopyTablesFromJava  ( jpp ); /* set the tables from JPEGParam */
  if (CheckExcept(jpp->env)) return; /* Exception was thrown. */

  CopyCompInfoFromJava( jpp ); /* set the per component info from JPEGParam */
  if (CheckExcept(jpp->env)) return; /* Exception was thrown. */

  jpp->compress->data_precision  = DEFAULT_DATA_PRECISION;
  jpp->compress->optimize_coding = FALSE; 

  // We will write our own markers if needed.
  jpp->compress->write_JFIF_header  = FALSE;
  jpp->compress->write_Adobe_marker = FALSE;

  // restart_interval
  tmpVal = JNU_CallMethodByName(jpp->env, &hasExceptions, jpp->JPPObj,
				"getRestartInterval","()I");
  if (CheckExcept(jpp->env)) return; /* Exception was thrown. */
  jpp->compress->restart_interval = tmpVal.i; 
}

/*/////////////////////////////////////////////////////////////////////////////
*	Set the information held in the JPEGParam object into the
*	CInfo struct for compression.  This information includes the
*	image height, width, colorspace and number of components.  
*/
GLOBAL(void)
ImageInfoFromJava( jpeg_param * jpp, jobject comp, jobject cm )
{
  jvalue tmpVal;
  jclass paramCls = (*(jpp->env))->GetObjectClass( jpp->env, jpp->JPPObj );
  jboolean hasExceptions = FALSE;

  if( CheckPtrs( jpp )  == FALSE ) return;  /* an exception has been thrown */

  // height
  tmpVal = JNU_CallMethodByName(jpp->env, &hasExceptions, jpp->JPPObj,
				"getHeight","()I");
  if (CheckExcept(jpp->env)) return; /* Exception was thrown. */
  jpp->compress->image_height = tmpVal.i;

  // width
  tmpVal = JNU_CallMethodByName(jpp->env, &hasExceptions, jpp->JPPObj,
				"getWidth","()I");
  if (CheckExcept(jpp->env)) return; /* Exception was thrown. */
  jpp->compress->image_width  = tmpVal.i;

  tmpVal = JNU_CallMethodByName(jpp->env, &hasExceptions, jpp->JPPObj, 
				"getEncodedColorID", "()I");
  if (CheckExcept(jpp->env)) return; /* Exception was thrown. */
  jpp->compress->jpeg_color_space = tmpVal.i;

  jpp->compress->in_color_space   = tmpVal.i;

  if (cm != NULL) 
    {
      tmpVal = JNU_CallMethodByName(jpp->env, &hasExceptions, comp, 
				    "getNearestColorId", 
				    "(Ljava/awt/image/ColorModel;)I",
				    cm);
      if (CheckExcept(jpp->env)) return; /* Exception was thrown. */
      jpp->compress->in_color_space = tmpVal.i;
    }
  
  tmpVal = JNU_CallMethodByName(jpp->env, &hasExceptions, jpp->JPPObj,
				"getNumComponents","()I");
  if (CheckExcept(jpp->env)) return; /* Exception was thrown. */
  jpp->compress->input_components = tmpVal.i;
  jpp->compress->num_components   = tmpVal.i;

  // get flag indicating whether to write the tables
  tmpVal = JNU_CallMethodByName(jpp->env, &hasExceptions, jpp->JPPObj,
				"isTableInfoValid","()Z");
  if (CheckExcept(jpp->env)) return; /* Exception was thrown. */
  jpp->tables_present     = tmpVal.z;

  // get flag indicating whether to write the tables
  tmpVal = JNU_CallMethodByName(jpp->env, &hasExceptions, jpp->JPPObj,
				"isImageInfoValid","()Z");
  if (CheckExcept(jpp->env)) return; /* Exception was thrown. */
  jpp->image_data_present = tmpVal.z;
}

/*/////////////////////////////////////////////////////////////////////////////
 *	Set the information held in the CInfo object into the JPEGParam object 
 *	( both compression or decompression ).  This information includes the 
 *	image height, width, colorspace and number of components.
 */
GLOBAL(jobject)
createJPPFromCInfo( jpeg_param * jpp, 
		    jobject      jpegImageDecoder, 
		    jboolean     convert)
{
  jvalue  tmp;
  jobject ret;
  jboolean hasExceptions=FALSE;

  /* create a new JPEGParam object w/ proper encoded COLOR_ID */
  ret  = JNU_NewObjectByName(jpp->env, 
			     "sun/awt/image/codec/JPEGParam", "(II)V", 
			     jpp->decompress->jpeg_color_space,
			     jpp->decompress->num_components);
  if (CheckExcept(jpp->env)) return NULL;
  
  // width
  JNU_CallMethodByName(jpp->env, &hasExceptions, ret,"setWidth","(I)V", 
		       jpp->decompress->image_width);
  if (CheckExcept(jpp->env)) return NULL; /* Exception was thrown. */

  // height 
  JNU_CallMethodByName(jpp->env, &hasExceptions, ret, "setHeight","(I)V", 
		       jpp->decompress->image_height);
  if (CheckExcept(jpp->env)) return NULL; /* Exception was thrown. */

  tmp = JNU_CallMethodByName(jpp->env, &hasExceptions, jpegImageDecoder, 
			     "getDecodedColorModel","(IZ)I", 
			     jpp->decompress->jpeg_color_space, convert);
  if (CheckExcept(jpp->env)) return NULL; /* Exception was thrown. */
  jpp->decompress->out_color_space = tmp.i;
  
  return ret;
}

/*
 * This simply checks that the information passed in from the outside
 * is non-NULL.  If there is a null value throw and exception and
 * return false.  If all is well set the file local version of the
 * environment variables and return true.  All external calls to
 * access JPEGParam must perform this check
 */
GLOBAL(boolean)
CheckPtrs(jpeg_param * jpp) 
{
  if (( jpp->env != NULL ) && ( jpp->JPPObj != NULL )) {
    if (jpp->is_decompressor) 
      return (jpp->decompress != NULL);
    else
      return (jpp->compress != NULL);
  }

  JNU_ThrowByName(jpp->env, NULL_POINTER, 
                  "JPEGParam, recieved an unexpected null pointer");
  return FALSE;
}

