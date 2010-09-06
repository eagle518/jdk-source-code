/*
 * @(#)JdbcOdbc.c	@(#)JdbcOdbc.c	1.61 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//----------------------------------------------------------------------------
//
// Module:      jdbcodbc.c
//
// Description:	Implements the ODBC API routines for native java calls.
//              It is the intent to do as little processing as possible in
//              this implementation.  All of the work is being done in Java,
//              all that is being done here is to simply call the ODBC
//              API and return the results.  All buffers are provided from
//              Java, so we need not allocate any memory here (or worry about
//              freeing it)
//
// Product:     JDBCODBC (Java DataBase Connectivity using
//              Open DataBase Connectivity)
//
// Author:      Karl Moss
//
// Date:        March, 1996
//
//----------------------------------------------------------------------------

#ifdef macintosh
#define sys/types.h Types.h
#endif /* macintosh */

#include <sys/types.h>

#ifdef WIN32
#include <windows.h>
#include <stdio.h>
#endif /* WIN32 */

#ifdef UNIX
#include <stddef.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <unistd.h>
#endif /* UNIX */

//#include "javaString.h"
#include <fcntl.h>

// This is an ODBC 2.1 application

//#define ODBCVER 0x210
#define ODBCVER 0x300

#ifdef UNIX
#include "sqlext.h"
#include "odbcinst.h"
#endif
#ifdef WIN32
#include "wsqlext.h"
#include "wodbcinst.h"
#endif

#ifdef WIN32
#define SQLLEN SDWORD
#define SQLULEN DWORD
#define LONG_IA64 long
#endif

#ifdef UNIX
#define SQLLEN SDWORD
#define SQLULEN DWORD
#define LONG_IA64 long
#endif

#ifdef _WIN64
#define SQLLEN INT64
#define SQLULEN UINT64
#define LONG_IA64 ULONG_PTR
#define SQLSETPOSIROW UINT64
#else
#define SQLSETPOSIROW SQLUSMALLINT
#endif

#include <jni.h>
#include "stdio.h"
#include "sun_jdbc_odbc_JdbcOdbc.h"


static double buf = 0;
//SDWORD	lenValue = SQL_NULL_DATA;
static UCHAR* gPBuf = NULL;
static SDWORD gLBuf = 0;

// 4691886
JNIEXPORT jint JNICALL Java_sun_jdbc_odbc_JdbcOdbc_getSQLLENSize
	(JNIEnv *env, jobject callingObject)
{
	return(sizeof(SQLLEN));
}

// 4691886
JNIEXPORT void JNICALL Java_sun_jdbc_odbc_JdbcOdbc_intToBytes
	(JNIEnv *env, jobject callingObject, jint i, jbyteArray buf)
{
	UCHAR*	pBuf = (UCHAR*) (*env)->GetByteArrayElements(env, buf, 0);
	SQLLEN	n = (SQLLEN)i;
	
	memcpy (pBuf, &n, sizeof(SQLLEN));
	(*env)->ReleaseByteArrayElements(env,buf,pBuf,0);
}

// 4641016
JNIEXPORT void JNICALL Java_sun_jdbc_odbc_JdbcOdbc_intTo4Bytes
	(JNIEnv *env, jobject callingObject, jint i, jbyteArray buf)
{
	UCHAR*	pBuf = (UCHAR*) (*env)->GetByteArrayElements(env, buf, 0);
	int	n = i;
	
	memcpy (pBuf, &n, sizeof(int));
	(*env)->ReleaseByteArrayElements(env,buf,pBuf,0);
}

// 4691886
JNIEXPORT void JNICALL Java_sun_jdbc_odbc_JdbcOdbc_longToBytes
	(JNIEnv *env, jobject callingObject, jlong l, jbyteArray buf)
{
	UCHAR*	pBuf = (UCHAR*) (*env)->GetByteArrayElements(env, buf, 0);
	SQLLEN	n = (SQLLEN)l;

	memcpy (pBuf, &n, sizeof(SQLLEN));
	(*env)->ReleaseByteArrayElements(env,buf,pBuf,0);
}

//----------------------------------------------------------------------------
// bufferToInt
// Converts the given buffer to a native int (long)
//----------------------------------------------------------------------------

JNIEXPORT jint JNICALL Java_sun_jdbc_odbc_JdbcOdbc_bufferToInt
	(JNIEnv *env, jobject callingObject, jbyteArray buf)
{
// Get the data structure portion of the buffer
	UCHAR*	pBuf = (UCHAR*) (*env)->GetByteArrayElements(env, buf, 0);
	DWORD	n = 0;

	memcpy (&n, pBuf, sizeof (n));
	(*env)->ReleaseByteArrayElements(env,buf,pBuf,0);
	return (jint)n;
}

// bug 4412437
//----------------------------------------------------------------------------
// bufferToFloat
// Converts the given buffer to a native float
//----------------------------------------------------------------------------

JNIEXPORT jfloat JNICALL Java_sun_jdbc_odbc_JdbcOdbc_bufferToFloat
	(JNIEnv *env, jobject callingObject, jbyteArray buf)
{
// Get the data structure portion of the buffer
	UCHAR*	pBuf = (UCHAR*) (*env)->GetByteArrayElements(env, buf, 0);
	jfloat	n = 0;

	memcpy (&n, pBuf, sizeof (n));
	(*env)->ReleaseByteArrayElements(env,buf,pBuf,0);
	return n;
}

// bug 4412437
//----------------------------------------------------------------------------
// bufferToDouble
// Converts the given buffer to a native double
//----------------------------------------------------------------------------

JNIEXPORT jdouble JNICALL Java_sun_jdbc_odbc_JdbcOdbc_bufferToDouble
	(JNIEnv *env, jobject callingObject, jbyteArray buf)
{
// Get the data structure portion of the buffer
	UCHAR*	pBuf = (UCHAR*) (*env)->GetByteArrayElements(env, buf, 0);
	jdouble	n = 0;

	memcpy (&n, pBuf, sizeof (n));


	(*env)->ReleaseByteArrayElements(env,buf,pBuf,0);
	return n;
}

//4532162
//----------------------------------------------------------------------------
// bufferToLong
// Converts the given buffer to a native long
//----------------------------------------------------------------------------

JNIEXPORT jlong JNICALL Java_sun_jdbc_odbc_JdbcOdbc_bufferToLong
	(JNIEnv *env, jobject callingObject, jbyteArray buf)
{
// Get the data structure portion of the buffer
	UCHAR*	pBuf = (UCHAR*) (*env)->GetByteArrayElements(env, buf, 0);
	jlong	n = 0;

	memcpy (&n, pBuf, sizeof (n));
	
	(*env)->ReleaseByteArrayElements(env,buf,pBuf,0);
	return n;
}

//----------------------------------------------------------------------------
// allocConnect
//----------------------------------------------------------------------------

JNIEXPORT jlong JNICALL Java_sun_jdbc_odbc_JdbcOdbc_allocConnect
	(JNIEnv *env, jobject callingObject, 
	 jlong hEnv, jbyteArray errorCode)
{
	// Get the data structure portion of the error code
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	HDBC	hDbc = SQL_NULL_HDBC;
	RETCODE	rc;

// Allocate a new connection handle
	
	rc = SQLAllocConnect (
		(HENV) hEnv,		// Environment handle
		&hDbc);			// Pointer to connection handle

	// Set the error code
	
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
	// Return the connection handle
	return (jlong) hDbc;
}


//----------------------------------------------------------------------------
// allocEnv
//----------------------------------------------------------------------------

JNIEXPORT jlong JNICALL Java_sun_jdbc_odbc_JdbcOdbc_allocEnv
	(JNIEnv *env, jobject callingObject, jbyteArray errorCode)
{
// Get the data structure portion of the error code
	UCHAR*	 errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	HENV	hEnv = SQL_NULL_HENV;
	RETCODE rc;

// Allocate a new environment handle
	rc = SQLAllocEnv (
		&hEnv);		    // Pointer to environment handle

// Set the error code
	
	errCode[0] = (char) rc;


	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
// Return the environment handle
	return (jlong) hEnv;
}

//----------------------------------------------------------------------------
// allocStmt
//----------------------------------------------------------------------------

JNIEXPORT jlong JNICALL Java_sun_jdbc_odbc_JdbcOdbc_allocStmt
	(JNIEnv *env, jobject callingObject, jlong hDbc,
	 jbyteArray errorCode)
{
	//Get the data structure portion of the error code
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	HSTMT	hStmt = SQL_NULL_HSTMT;
	RETCODE rc;

// Allocate a new statement handle
	
	rc = SQLAllocStmt (
		(HDBC) hDbc,		// Connection handle
		&hStmt);		// Pointer to statement handle

	// Set the error code
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
	// Return the statement handle
	return (jlong) hStmt;
}

//----------------------------------------------------------------------------
// bindColAtExec
// Used for binding
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_sun_jdbc_odbc_JdbcOdbc_bindColAtExec
	(JNIEnv *env, jobject callingObject, 
	 jlong hStmt, 
	 jint icol,
	 jint SQLtype,
	 //4691886
	 jbyteArray lenInd,
	 jbyteArray dataBuf,
	 jlongArray buffers,
	 jbyteArray errorCode)
{

	// Get the data structure portion of the error code
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	RETCODE	rc;
	SWORD	Ctype = SQL_C_CHAR;
	UCHAR*	pBuf = NULL;
	SQLLEN  lBuf = 0;
	//4691886
	char* lenBuf = 0;
	int sarray = 0;		
	int i = 0;
	//4691886
	int sizeofSQLLEN = sizeof(SQLLEN);
	SQLLEN lenBufElement = 0;

	jlong* pBuffers=(jlong*)(*env)->GetLongArrayElements(env, buffers, 0);

	jobject gDataBuf = (*env)->NewGlobalRef(env, dataBuf);
	jobject glenInd  = (*env)->NewGlobalRef(env, lenInd);


	// Get the buffer for the column's data.  In this case, we'll set it
	// to the column number.  This number will be returned by SQLParamData.
	// It is assumed that this buffer is large enough to fit a 4-byte integer.

	if (dataBuf != NULL) {
		//4691886
		pBuf = (UCHAR*) (*env)->GetByteArrayElements(env, gDataBuf, 0);
		lBuf = (SQLLEN) (*env)->GetArrayLength(env, gDataBuf);
		
		//4691886
		memset (pBuf, 0x00, sizeof(jint));
		
		memcpy (pBuf, &icol, sizeof(int));

		pBuffers[0]=(LONG_IA64)pBuf;	//Store the pointers
		pBuffers[1]=(LONG_IA64)gDataBuf;
	}
	// Get the buffer used for the parameter's length.  It is assumed that
	// this buffer is large enough to fit a 4-byte integer
	
	if (lenInd != NULL) {

		//4691886
		lenBuf = (char*)(*env)->GetByteArrayElements(env, glenInd, 0);

		// what is the size and length of the array.
		//4691886
		if (lenBuf) sarray = (int)(((*env)->GetArrayLength(env, glenInd))/sizeofSQLLEN);

		for (i = 0; i < (sizeofSQLLEN * sarray); i=i+sizeofSQLLEN)
		{
			memcpy(&lenBufElement, &lenBuf[i], sizeofSQLLEN);
			if (lenBufElement > 0)
			{
				lenBufElement = (SQLLEN)(SQL_LEN_DATA_AT_EXEC (((int)(lenBufElement))));
			}
			memcpy(&lenBuf[i], &lenBufElement, sizeofSQLLEN);
		}

		//Store the pointer
		pBuffers[2]=(LONG_IA64)lenBuf;
		pBuffers[3]=(LONG_IA64)glenInd;

	}
	(*env)->ReleaseLongArrayElements(env, buffers, pBuffers, 0);


	
	// The SQL Type was given, make the C-Type match
	// Only SQL_CHAR and SQL_BINARY are currently supported.
	
	if ((SQLtype == SQL_BINARY) ||
	    (SQLtype == SQL_VARBINARY) ||
	    (SQLtype == SQL_LONGVARBINARY)) {
		Ctype = SQL_C_BINARY;
	}

	// Bind the parameter

	rc = SQLBindCol (
		(HSTMT) hStmt,			// Statement handle
		(UWORD) icol,			// Parameter number
		Ctype,				// The C data type
		pBuf,				// Pointer to parameter's data
		lBuf,				// Maximum length of parameter buffer
		//4691886
		(SQLLEN*)lenBuf);		// Pointer to parameter's length

	// Set the error code
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);

}

//----------------------------------------------------------------------------
// bindColBinary
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_sun_jdbc_odbc_JdbcOdbc_bindColBinary
	(JNIEnv *env, jobject callingObject, 
	 jlong hStmt, 
	 jint icol, 
	 jobjectArray values, 
	 //4691886
	 jbyteArray lenInd, 
	 jint descLen,
	 jbyteArray dataBuf,
	 jlongArray buffers,
	 jbyteArray errorCode)
{

	// Get the data structure portion of the error code
	RETCODE	rc;
	UCHAR*	errCode = (UCHAR*)(*env)->GetByteArrayElements(env, errorCode, 0);
	UCHAR*  pBuf = NULL;
	UCHAR* cpyData = NULL;
	SDWORD	lBuf = 	0;
	int i;
	int sarray;

	jlong* pBuffers=(jlong*)(*env)->GetLongArrayElements(env, buffers, 0);

	jobject gDataBuf = (*env)->NewGlobalRef(env, dataBuf);
	jobject glenInd  = (*env)->NewGlobalRef(env, lenInd);
	
	// Fix 4531124
	//jobjectArray gValue = (*env)->NewGlobalRef(env, values);

	//4691886
	char* lenBuf = (char*) (*env)->GetByteArrayElements(env, glenInd, 0);				

	if (dataBuf != NULL) 
	{
		pBuf = (UCHAR*) (*env)->GetByteArrayElements(env, gDataBuf, 0);
		lBuf = (SDWORD) (*env)->GetArrayLength(env, gDataBuf);
		
		//store the pointers 
		pBuffers[0]=(LONG_IA64)pBuf;
		pBuffers[1]=(LONG_IA64)gDataBuf;
		pBuffers[2]=(LONG_IA64)lenBuf;
		pBuffers[3]=(LONG_IA64)glenInd;

		// what is the size and length of the array.
		sarray = (SDWORD) (*env)->GetArrayLength(env, values);
	
		// Init the string buffers to an empty string
		memset (pBuf, 0x00, lBuf);
		
		// this loop populates the byte Data buffer.
		for (i = 0; i < sarray; i++)
		{
			jbyteArray jbyteData = (*env)->GetObjectArrayElement(env, values, i);

			if (jbyteData != NULL)
			{
				cpyData = (char *)((*env)->GetByteArrayElements(env, jbyteData, 0));

				if (cpyData != NULL)
				{
					memcpy ( (pBuf + (descLen * i)), cpyData, descLen);
				}

				(*env)->ReleaseByteArrayElements(env, jbyteData, cpyData, 0);
			}			

		}

		//(*env)->ReleaseByteArrayElements(env, gDataBuf, pBuf, 0);	
	}

	(*env)->ReleaseLongArrayElements(env, buffers, pBuffers, 0);

	rc = SQLBindCol (
		(HSTMT) hStmt,			// Statement handle
		(UWORD) icol,			// Parameter number
		SQL_C_BINARY,			// The C data type
		pBuf,				// Pointer to parameter's data
		(SQLLEN)descLen,		// Maximum length of parameter buffer
		//4691886
		(SQLLEN*)lenBuf);		// Pointer to parameter's length
		
	// Set the error code
	
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);

	// 4486195 -> resources now released at a later stage 
	// (when ResultSet closed)
	//(*env)->ReleaseIntArrayElements(env, glenInd, lenBuf, 0);
	//(*env)->ReleaseByteArrayElements(env, gDataBuf, pBuf, 0);	
	

}

//----------------------------------------------------------------------------
// bindColDate
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_sun_jdbc_odbc_JdbcOdbc_bindColDate
	(JNIEnv *env, jobject callingObject, 
	 jlong hStmt, 
	 jint icol, 
	 jintArray year, 
	 jintArray month, 
	 jintArray day, 
	 //4691886
	 jbyteArray lenInd,
	 jbyteArray dataBuf,
	 jlongArray buffers,
	 jbyteArray errorCode)
{

	// Get the data structure portion of the error code
	RETCODE	rc;
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	UCHAR*	pBuf = NULL;
	
	DATE_STRUCT dt;

	SQLLEN	lBuf = 0; // 4638528

	int i;

	jlong* pBuffers=(jlong*)(*env)->GetLongArrayElements(env, buffers, 0);

	jobject gDataBuf = (*env)->NewGlobalRef(env, dataBuf);
	jobject glenInd  = (*env)->NewGlobalRef(env, lenInd);

	//4691886
	char* lenBuf = NULL;				
	
	int arraySize = 0;
	
	//4691886
	int sizeofSQLLEN = sizeof(SQLLEN);

	jint* pyears	=NULL;
	jint* pmonths	=NULL;
	jint* pdays  	=NULL;

	//4691886
	if (glenInd) lenBuf = (char*) (*env)->GetByteArrayElements(env, glenInd, 0);
	
	//4691886
	if (lenInd) arraySize = (int)(((*env)->GetArrayLength(env, lenInd))/sizeofSQLLEN);

	if (year) pyears	=(jint*)(*env)->GetIntArrayElements(env, year, 0);
	if (month) pmonths	=(jint*)(*env)->GetIntArrayElements(env, month, 0);
	if (day) pdays		=(jint*)(*env)->GetIntArrayElements(env, day, 0);

	// Construct our Date structure
	memset (&dt, 0x00, sizeof (dt));

	if (gDataBuf) pBuf = (UCHAR*) (*env)->GetByteArrayElements(env, gDataBuf, 0);
	if (gDataBuf) lBuf = (SQLLEN) (*env)->GetArrayLength(env, gDataBuf); // 4638528
	memset (pBuf, 0x00, lBuf);	// 4638528
	
	if (dataBuf != NULL)
	{
		for (i = 0; i < arraySize; i++)
		{
			dt.year		= (short) pyears[i];
			dt.month	= (short) pmonths[i];
			dt.day		= (short) pdays[i];

		// Make a copy of the data coming in into the permanent data buffer.  This
		// data is in native format.  If no buffer is provided, the data buffer
		// will be NULL.  If a buffer is provided, it is assumed to be large enough
		// to fit the DATE structure

			memcpy (pBuf + (sizeof(dt) * i) , &dt, sizeof(dt)); // 4638528

		}	

	}

	// 4638528
	pBuffers[0]=(LONG_IA64)pBuf;
	pBuffers[1]=(LONG_IA64)gDataBuf;
	pBuffers[2]=(LONG_IA64)lenBuf;
	pBuffers[3]=(LONG_IA64)glenInd;

	(*env)->ReleaseLongArrayElements(env, buffers, pBuffers, 0);

	(*env)->ReleaseIntArrayElements(env, year, pyears, 0);
	(*env)->ReleaseIntArrayElements(env, month, pmonths, 0);
	(*env)->ReleaseIntArrayElements(env, day, pdays, 0);
	
	
	rc = SQLBindCol (
		(HSTMT) hStmt,			// Statement handle
		(UWORD) icol,			// Parameter number
		SQL_C_DATE,			// The C data type
		pBuf,				// Pointer to parameter's data
		lBuf,				// Maximum length of parameter buffer
		(SQLLEN*)lenBuf);		// Pointer to parameter's length


	// Set the error code
	
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
}


//----------------------------------------------------------------------------
// bindColInteger
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_sun_jdbc_odbc_JdbcOdbc_bindColInteger
	(JNIEnv *env, jobject callingObject,
	 jlong hStmt,
	 jint icol,
	 jintArray values,
	 //4691886
	 jbyteArray lenInd,
	 jbyteArray dataBuf,
	 jlongArray buffers,
	 jbyteArray errorCode)
{
	// Get the data structure portion of the error code
	RETCODE	rc;
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);

	UCHAR*	pBuf = NULL;
	SQLLEN	lBuf = 0;
	int i, sarray;
	
	//4691886
	int sizeofSQLLEN = sizeof(SQLLEN);

	jlong* pBuffers=(jlong*)(*env)->GetLongArrayElements(env, buffers, 0);

	jobject gintDataBuf = (*env)->NewGlobalRef(env, dataBuf);
	jobject gintlenInd  = (*env)->NewGlobalRef(env, lenInd);


	jint* pValues = (jint*)(*env)->GetIntArrayElements(env, values, 0); // 4638528

	//4691886
	char* lenBuf = (char*) (*env)->GetByteArrayElements(env, gintlenInd, 0);

	//4691886
	sarray = (SDWORD) (((*env)->GetArrayLength(env, gintlenInd))/sizeofSQLLEN);
	
	if (dataBuf != NULL)
	{
		pBuf = (UCHAR*)	(*env)->GetByteArrayElements(env, gintDataBuf, 0);
		lBuf = (SQLLEN) (*env)->GetArrayLength(env, gintDataBuf); // 4638528
		
		memset(pBuf, 0x00, lBuf);

		for (i = 0; i < sarray; i++)
		{
			// Make a copy of the incoming value
			memcpy (pBuf + (sizeof(int) * i), &pValues[i], sizeof(int)); // 4638528
		}
		pBuffers[0] = (LONG_IA64)pBuf;
		pBuffers[1] = (LONG_IA64)gintDataBuf;
		pBuffers[2] = (LONG_IA64)lenBuf;   	// 4638528
		pBuffers[3] = (LONG_IA64)gintlenInd;    // 4638528		
        } 

	(*env)->ReleaseLongArrayElements(env, buffers, pBuffers, 0);

	// Bind the Column
	rc = SQLBindCol (
		(HSTMT) hStmt,			// Statement handle
		(SQLUSMALLINT) icol,		// column number
		SQL_C_SLONG,			// The C data type
		pBuf,				// Pointer to parameter's data
		lBuf,				// Maximum length of parameter buffer
		//4691886
		(SQLLEN*)lenBuf);		// Pointer to parameter's length


	// Set the error code
	
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
	(*env)->ReleaseIntArrayElements(env, values, pValues, 0); // 4638528

}

//----------------------------------------------------------------------------
// bindColFloat
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_sun_jdbc_odbc_JdbcOdbc_bindColFloat
	(JNIEnv *env, jobject callingObject,
	 jlong hStmt,
	 jint icol,
	 jfloatArray values,
	 //4691886
	 jbyteArray lenInd,
	 jbyteArray dataBuf,
	 jlongArray buffers,
	 jbyteArray errorCode)
{
  
	RETCODE	rc;
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);

	UCHAR*	pBuf = NULL;
	SQLLEN	lBuf = 0;
	int i, sarray;
	
	//4691886
	int sizeofSQLLEN = sizeof(SQLLEN);

	jlong* pBuffers=(jlong*)(*env)->GetLongArrayElements(env, buffers, 0);

	jobject gfDataBuf = (*env)->NewGlobalRef(env, dataBuf);
	jobject gflenInd  = (*env)->NewGlobalRef(env, lenInd);

	jfloat* pValues = (jfloat*)(*env)->GetFloatArrayElements(env, values, 0);

	//4691886
	char* lenBuf = (char*) (*env)->GetByteArrayElements(env, gflenInd, 0);

	//4691886
	sarray = (SDWORD) (((*env)->GetArrayLength(env, gflenInd))/sizeofSQLLEN);
	
	if (dataBuf != NULL)
	{
		pBuf = (UCHAR*)	(*env)->GetByteArrayElements(env, gfDataBuf, 0);
		lBuf = (SQLLEN) (*env)->GetArrayLength(env, gfDataBuf); // 4638528

		memset(pBuf, 0x00, lBuf);

		for (i = 0; i < sarray; i++)
		{
			// Make a copy of the incoming value
			memcpy (pBuf + (sizeof(float) * i), &pValues[i], sizeof(float)); // 4638528
		}
		pBuffers[0] = (LONG_IA64)pBuf;
		pBuffers[1] = (LONG_IA64)gfDataBuf;
		pBuffers[2] = (LONG_IA64)lenBuf;   // 4638528
		pBuffers[3] = (LONG_IA64)gflenInd; // 4638528		
	}
	(*env)->ReleaseLongArrayElements(env, buffers, pBuffers, 0);

	// Bind the Column

	rc = SQLBindCol (
		(HSTMT) hStmt,			// Statement handle
		(SQLUSMALLINT) icol,		// column number
		SQL_C_FLOAT,			// The C data type
		pBuf,				// Pointer to parameter's data
		lBuf,				// Maximum length of parameter buffer
		//4691886
		(SQLLEN*)lenBuf);		// Pointer to parameter's length


	// Set the error code
	
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
	(*env)->ReleaseFloatArrayElements(env, values, pValues, 0); // 4638528

}

//----------------------------------------------------------------------------
// bindColDouble
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_sun_jdbc_odbc_JdbcOdbc_bindColDouble
	(JNIEnv *env, jobject callingObject,
	 jlong hStmt,
	 jint icol,
	 jdoubleArray values,
	 //4691886
	 jbyteArray lenInd,
	 jbyteArray dataBuf,
	 jlongArray buffers,
	 jbyteArray errorCode)
{
	RETCODE	rc;
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);

	UCHAR*	pBuf = NULL;
	SQLLEN	lBuf = 0;
	int i, sarray;
	
	//4691886
	int sizeofSQLLEN = sizeof(SQLLEN);

	jlong* pBuffers=(jlong*)(*env)->GetLongArrayElements(env, buffers, 0);

	jobject gdDataBuf = (*env)->NewGlobalRef(env, dataBuf);
	jobject gdlenInd  = (*env)->NewGlobalRef(env, lenInd);

	jdouble* pValues = (jdouble*)(*env)->GetDoubleArrayElements(env, values, 0); // 4638528

	//4691886
	char* lenBuf = (char*) (*env)->GetByteArrayElements(env, gdlenInd, 0);

	//4691886
	sarray = (SDWORD) (((*env)->GetArrayLength(env, gdlenInd))/sizeofSQLLEN);
	
	if (dataBuf != NULL)
	{
		pBuf = (UCHAR*)	(*env)->GetByteArrayElements(env, gdDataBuf, 0);
		lBuf = (SQLLEN) (*env)->GetArrayLength(env, gdDataBuf);	// 4638528	
		
		memset(pBuf, 0x00, lBuf);

		for (i = 0; i < sarray; i++)
		{
			memcpy (pBuf + (sizeof(double) * i), &pValues[i], sizeof(double)); // 4638528
		}
		pBuffers[0] = (LONG_IA64)pBuf;
		pBuffers[1] = (LONG_IA64)gdDataBuf;
		pBuffers[2] = (LONG_IA64)lenBuf;   // 4638528
		pBuffers[3] = (LONG_IA64)gdlenInd; // 4638528
	}
	(*env)->ReleaseLongArrayElements(env, buffers, pBuffers, 0);
	
	// Bind the Column

	rc = SQLBindCol (
		(HSTMT) hStmt,		// Statement handle
		(SQLUSMALLINT) icol,	// row number
		SQL_C_DOUBLE,		// The C data type
		pBuf,			// Pointer to parameter's data
		lBuf,			// Maximum length of parameter buffer
		//4691886
		(SQLLEN*)lenBuf);	// Pointer to parameter's length

	// Set the error code
	
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
	(*env)->ReleaseDoubleArrayElements(env, values, pValues, 0); // 4638528
}



//----------------------------------------------------------------------------
// bindColDefault
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_sun_jdbc_odbc_JdbcOdbc_bindColDefault
	(JNIEnv *env, jobject callingObject, jlong hStmt, jint icol,
	 jbyteArray rgb, jbyteArray pcb, jbyteArray errorCode)
{
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	UCHAR*	rgbValue = NULL;
	SQLLEN	cbValueMax = 0;
	UCHAR*	pcbValue = NULL;
	RETCODE	rc;

	if (rgb) rgbValue = (UCHAR*) (*env)->GetByteArrayElements(env, rgb, 0);
	if (rgb) cbValueMax = (SQLLEN) (*env)->GetArrayLength(env, rgb);
	if (pcb) pcbValue = (UCHAR*) (*env)->GetByteArrayElements(env, pcb, 0);

	// Bind the column
		
	rc = SQLBindCol (
		(HSTMT) hStmt,		// Statement handle
		(UWORD) icol,		// Column number of result data
		SQL_C_DEFAULT,		// C data type of the result data
		rgbValue,		// Pointer to storage for the data
		cbValueMax,		// Maximum length of rgbValue buffer
		(SQLLEN*) pcbValue);	// Number of bytes available
	
	// Set the error code
	
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
	(*env)->ReleaseByteArrayElements(env, rgb, rgbValue, 0);
	(*env)->ReleaseByteArrayElements(env, pcb, pcbValue, 0);
}

//----------------------------------------------------------------------------
// bindColString
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_sun_jdbc_odbc_JdbcOdbc_bindColString
	(JNIEnv *env, jobject callingObject, 
	 jlong hStmt, 
	 jint icol, 
	 jint SQLtype, 
	 jobjectArray strVal,
	 jint descLen,
	 //4691886
	 jbyteArray lenInd,
	 jbyteArray dataBuf,
	 jlongArray buffers,
	 jbyteArray errorCode)
{

	RETCODE	rc;
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	UCHAR*  pBuf = NULL;
	SDWORD	lBuf = 0; 	// 4638528
	int i, sarray;
	
	//4691886
	int sizeofSQLLEN = sizeof(SQLLEN);

	jlong* pBuffers=(jlong*)(*env)->GetLongArrayElements(env, buffers, 0);

	jobject gDataBuf = (*env)->NewGlobalRef(env, dataBuf);
	jobject glenInd  = (*env)->NewGlobalRef(env, lenInd);

	//4691886
	char* lenBuf = NULL;

	//4691886
	if (glenInd) lenBuf=(char*) (*env)->GetByteArrayElements(env, glenInd, 0);				
	
	if (dataBuf != NULL) 
	{
		pBuf = (UCHAR*) (*env)->GetByteArrayElements(env, gDataBuf, 0);
		lBuf = (SDWORD) (*env)->GetArrayLength(env, gDataBuf); // 4638528
			
		sarray = (SDWORD) (*env)->GetArrayLength(env, strVal); // 4638528

		//store the pointers 
		// 4638528
		pBuffers[0]=(LONG_IA64)pBuf;
		pBuffers[1]=(LONG_IA64)gDataBuf;
		pBuffers[2]=(LONG_IA64)lenBuf;
		pBuffers[3]=(LONG_IA64)glenInd;

		memset (pBuf, 0x00, lBuf);

		for (i = 0; i < sarray; i++)
		{
			// Get the String, then copy to data buffer and release it.
			jstring jstr = (*env)->GetObjectArrayElement(env, strVal, i); // 4638528

			if (jstr != NULL)
			{
				UCHAR* cpyData = (UCHAR*) ((*env)->GetStringUTFChars(env, jstr, NULL));
				memcpy ( (pBuf + (descLen * i)), cpyData, descLen); // 4638528
				(*env)->ReleaseStringUTFChars (env, jstr, cpyData);
			}
			
		}		

	}	
	(*env)->ReleaseLongArrayElements(env, buffers, pBuffers, 0); // 4638528
	
	// Bind the string parameter

	rc = SQLBindCol (
		(HSTMT) hStmt,			// Statement handle
		(UWORD) icol,			// Parameter number
		SQL_C_CHAR,			// The C data type
		pBuf,				// Pointer to parameter's data
		(SQLLEN) descLen + 1,		// Maximum length of parameter buffer
		//4691886
		(SQLLEN*)lenBuf);			// Pointer to parameter's length

	// Set the error code
	
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
}

//----------------------------------------------------------------------------
// bindColTime
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_sun_jdbc_odbc_JdbcOdbc_bindColTime
	(JNIEnv *env, jobject callingObject, 
	 jlong hStmt, 
	 jint icol, 
	 jintArray hour, 
	 jintArray minutes, 
	 jintArray seconds,
	 //4691886
	 jbyteArray lenInd,
	 jbyteArray dataBuf,
	 jlongArray buffers,
	 jbyteArray errorCode)
{

	// Get the data structure portion of the error code
	RETCODE	rc;
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	UCHAR*	pBuf = NULL;
	
	TIME_STRUCT tm;

	SQLLEN	lBuf = 0; // 4638528

	int i;

	jlong* pBuffers=(jlong*)(*env)->GetLongArrayElements(env, buffers, 0);

	jobject gDataBuf = (*env)->NewGlobalRef(env, dataBuf);
	jobject glenInd  = (*env)->NewGlobalRef(env, lenInd);
	
	//4691886
	char* lenBuf = NULL;				
	
	int arraySize = 0;
	
	//4691886
	int sizeofSQLLEN = sizeof(SQLLEN);

	jint* phours	 = NULL;
	jint* pminutes	 = NULL;
	jint* pseconds	 = NULL;

	//4691886
	if (glenInd) lenBuf = (char*) (*env)->GetByteArrayElements(env, glenInd, 0);
	
	//4691886
	if (lenInd) arraySize = (int)(((*env)->GetArrayLength(env, lenInd))/sizeofSQLLEN);

	if (hour) phours=(jint*)(*env)->GetIntArrayElements(env, hour, 0);
	if (minutes) pminutes=(jint*)(*env)->GetIntArrayElements(env, minutes, 0);
	if (seconds) pseconds=(jint*)(*env)->GetIntArrayElements(env, seconds, 0);

	memset (&tm, 0x00, sizeof (tm));

	if (gDataBuf) pBuf = (UCHAR*) (*env)->GetByteArrayElements(env, gDataBuf, 0);
	if (gDataBuf) lBuf = (SQLLEN) (*env)->GetArrayLength(env, gDataBuf); // 4638528
	memset (pBuf, 0x00, lBuf);	// 4638528
		
	for (i = 0; i < arraySize; i++)
	{
		tm.hour		= (short) phours[i];
		tm.minute	= (short) pminutes[i];
		tm.second	= (short) pseconds[i];
		
	// Make a copy of the data coming in into the permanent data buffer.  This
	// data is in native format.  If no buffer is provided, the data buffer
	// will be NULL.  If a buffer is provided, it is assumed to be large enough
	// to fit the Time structure

		memcpy (pBuf + (sizeof(tm) * i) , &tm, sizeof (tm)); // 4638528

	}
	
	// 4638528
	pBuffers[0]=(LONG_IA64)pBuf;
	pBuffers[1]=(LONG_IA64)gDataBuf;
	pBuffers[2]=(LONG_IA64)lenBuf;
	pBuffers[3]=(LONG_IA64)glenInd;
	
	(*env)->ReleaseLongArrayElements(env, buffers, pBuffers, 0);

	(*env)->ReleaseIntArrayElements(env, hour, phours, 0);
	(*env)->ReleaseIntArrayElements(env, minutes, pminutes, 0);
	(*env)->ReleaseIntArrayElements(env, seconds, pseconds, 0);
	
	rc = SQLBindCol (
		(HSTMT) hStmt,			// Statement handle
		(UWORD) icol,			// Parameter number
		SQL_C_TIME,			// The C data type
		pBuf,				// Pointer to parameter's data
		lBuf,				// Maximum length of parameter buffer
		//4691886
		(SQLLEN*)lenBuf); 			// pointer to parameter's length


	// Set the error code
	
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
}

//----------------------------------------------------------------------------
// bindColTimestamp
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_sun_jdbc_odbc_JdbcOdbc_bindColTimestamp (
	JNIEnv *env, jobject callingObject, 
	jlong hStmt, 
	jint icol, 
	jintArray year, 
	jintArray month, 
	jintArray day, 
	jintArray hour, 
	jintArray minutes, 
	jintArray seconds, 
	jintArray nanos,
	//4691886
	jbyteArray lenInd,
	jbyteArray dataBuf,
	jlongArray buffers,
	jbyteArray errorCode)
{

	// Get the data structure portion of the error code
	RETCODE	rc;
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	UCHAR*	pBuf = NULL;
	
	TIMESTAMP_STRUCT ts;

	SQLLEN	lBuf = 0; // 4638528

	int i;

	jlong* pBuffers=(jlong*)(*env)->GetLongArrayElements(env, buffers, 0);

	jobject gDataBuf = (*env)->NewGlobalRef(env, dataBuf);
	jobject glenInd  = (*env)->NewGlobalRef(env, lenInd);

	//4691886
	char* lenBuf = NULL;
	int arraySize = 0;
	
	//4691886
	int sizeofSQLLEN = sizeof(SQLLEN);
	

	jint* pyears		=NULL;
	jint* pmonths		=NULL;
	jint* pdays 		=NULL;
	jint* phours		=NULL;
	jint* pminutes		=NULL;
	jint* pseconds		=NULL;
	jint* pnanos		=NULL;

	//4691886
	if (glenInd) lenBuf = (char*) (*env)->GetByteArrayElements(env, glenInd, 0);				
	
	//4691886
	if (lenInd) arraySize = (int)(((*env)->GetArrayLength(env, lenInd))/sizeofSQLLEN);
	
	if (year) pyears=(jint*)(*env)->GetIntArrayElements(env, year, 0);
	if (month) pmonths=(jint*)(*env)->GetIntArrayElements(env, month, 0);
	if (day) pdays=(jint*)(*env)->GetIntArrayElements(env, day, 0);
	if (hour) phours=(jint*)(*env)->GetIntArrayElements(env, hour, 0);
	if (minutes) pminutes=(jint*)(*env)->GetIntArrayElements(env, minutes, 0);
	if (seconds) pseconds=(jint*)(*env)->GetIntArrayElements(env, seconds, 0);
	if (nanos) pnanos=(jint*)(*env)->GetIntArrayElements(env, nanos, 0);

	// Construct our timestamp structure
	memset (&ts, 0x00, sizeof (ts));

	if (dataBuf != NULL)
	{

		pBuf = (UCHAR*) (*env)->GetByteArrayElements(env, gDataBuf, 0);
		if (gDataBuf) lBuf = (SQLLEN) (*env)->GetArrayLength(env, gDataBuf); // 4638528
		memset (pBuf, 0x00, lBuf);	// 4638528

		for (i = 0; i < arraySize; i++)
		{

			ts.year		= (short) pyears[i];
			ts.month	= (short) pmonths[i];
			ts.day		= (short) pdays[i];
			ts.hour		= (short) phours[i];
			ts.minute	= (short) pminutes[i];
			ts.second	= (short) pseconds[i];
			ts.fraction 	= pnanos[i];
			
		// Make a copy of the data coming in into the permanent data buffer.  This
		// data is in native format.  If no buffer is provided, the data buffer
		// will be NULL.  If a buffer is provided, it is assumed to be large enough
		// to fit the timestamp structure

			memcpy (pBuf + (sizeof(ts) * i) , &ts, sizeof(ts)); // 4638528

		}
		
		// 4638528
		pBuffers[0]=(LONG_IA64)pBuf;
		pBuffers[1]=(LONG_IA64)gDataBuf;
		pBuffers[2]=(LONG_IA64)lenBuf;
		pBuffers[3]=(LONG_IA64)glenInd;

	}
	(*env)->ReleaseLongArrayElements(env, buffers, pBuffers, 0);

	(*env)->ReleaseIntArrayElements(env, year, pyears, 0);
	(*env)->ReleaseIntArrayElements(env, month, pmonths, 0);
	(*env)->ReleaseIntArrayElements(env, day, pdays, 0);
	(*env)->ReleaseIntArrayElements(env, hour, phours, 0);
	(*env)->ReleaseIntArrayElements(env, minutes, pminutes, 0);
	(*env)->ReleaseIntArrayElements(env, seconds, pseconds, 0);
	(*env)->ReleaseIntArrayElements(env, nanos, pnanos, 0);
	
// Bind the parameter

	rc = SQLBindCol (
		(HSTMT) hStmt,			// Statement handle
		(UWORD) icol,			// Parameter number
		SQL_C_TIMESTAMP,		// The C data type
		pBuf,				// Pointer to parameter's data
		lBuf,				// Maximum length of parameter buffer
		//4691886
		(SQLLEN*)lenBuf);			// Pointer to parameter's length

	// Set the error code
	
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);		
}



//----------------------------------------------------------------------------
// bindInParameterAtExec
// Used for binding
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_sun_jdbc_odbc_JdbcOdbc_bindInParameterAtExec
	(JNIEnv *env, jobject callingObject, jlong hStmt, jint ipar,
	 jint SQLtype, jint len, jbyteArray dataBuf, jbyteArray lenBuf, 
	 jbyteArray errorCode, jlongArray buffers)


{
	// Get the data structure portion of the error code
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	RETCODE	rc;
	SWORD	Ctype = SQL_C_CHAR;
	SQLULEN	atExec = SQL_LEN_DATA_AT_EXEC (len);
	UCHAR*	pBuf = NULL;
	UCHAR*	pLen = NULL;
	jobject gDataBuf = (*env)->NewGlobalRef(env, dataBuf);
	jobject gLenBuf = (*env)->NewGlobalRef(env, lenBuf);

	jlong* pBuffers=(jlong*)(*env)->GetLongArrayElements(env, buffers, 0);

// Get the buffer for the parameter's data.  In this case, we'll set it
// to the parameter number.  This number will be returned by SQLParamData.
// It is assumed that this buffer is large enough to fit a 4-byte integer.

	if (dataBuf != NULL) {

		pBuf = (UCHAR*) (*env)->GetByteArrayElements(env, gDataBuf, 0);
		memcpy (pBuf, &ipar, sizeof (ipar));
		pBuffers[0]=(LONG_IA64)pBuf;	//Store the pointers
		pBuffers[1]=(LONG_IA64)gDataBuf;
	}
	
	// Get the buffer used for the parameter's length.  It is assumed that
	// this buffer is large enough to fit a 4-byte integer
	
	if (lenBuf != NULL) {

		pLen = (UCHAR*) (*env)->GetByteArrayElements(env, gLenBuf, 0);
		memcpy (pLen, &atExec, sizeof (atExec));
		//Store the pointer
		pBuffers[2]=(LONG_IA64)pLen;
		pBuffers[3]=(LONG_IA64)gLenBuf;
	}
	(*env)->ReleaseLongArrayElements(env, buffers, pBuffers, 0);
	
// The SQL Type was given, make the C-Type match
// Only SQL_CHAR and SQL_BINARY are currently supported.
	
	if ((SQLtype == SQL_BINARY) ||
	    (SQLtype == SQL_VARBINARY) ||
	    (SQLtype == SQL_LONGVARBINARY)) {
		Ctype = SQL_C_BINARY;
	}

	// Bind the parameter

	rc = SQLBindParameter (
		(HSTMT) hStmt,		// Statement handle
		(UWORD) ipar,		// Parameter number
		SQL_PARAM_INPUT,	// Type of parameter
		Ctype,			// The C data type
		(SWORD) SQLtype,	// The SQL data type
		(SQLULEN) len,		// Precision
		0,			// Scale
		pBuf,			// Pointer to parameter's data
		(SQLLEN)sizeof (ipar),		// Maximum length of parameter buffer
		(SQLLEN*) pLen);	// Pointer to parameter's length

	// Set the error code
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
}

// 4641016
//----------------------------------------------------------------------------
// bindInOutParameterAtExec
// Used for bindingIN OUT parameters for which data is passed at run time
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_sun_jdbc_odbc_JdbcOdbc_bindInOutParameterAtExec(
	JNIEnv *env, 
	jobject callingObject, 
	jlong hStmt, 
	jint ipar, 
	jint CType,
	jint SQLtype, 
	jint dataBufLen, 
	jbyteArray dataBuf, 
	jint streamLength, 
	jbyteArray lenBuf, 
	jbyteArray errorCode, 
	jlongArray buffers)


{
	// Get the data structure portion of the error code
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	RETCODE	rc;
	SWORD	Ctype = (SWORD)CType;
	SQLULEN	atExec = SQL_LEN_DATA_AT_EXEC (streamLength);
	UCHAR*	pBuf = NULL;
	UCHAR*	pLen = NULL;
	jobject gDataBuf = (*env)->NewGlobalRef(env, dataBuf);
	jobject gLenBuf = (*env)->NewGlobalRef(env, lenBuf);

	jlong* pBuffers=(jlong*)(*env)->GetLongArrayElements(env, buffers, 0);

	// Get the buffer for the parameter's data.  In this case, we'll set it
	// to the parameter number.  This number will be returned by SQLParamData.
	// It is assumed that this buffer is large enough to fit a 4-byte integer.

	if (dataBuf != NULL) {

		pBuf = (UCHAR*) (*env)->GetByteArrayElements(env, gDataBuf, 0);
		memcpy (pBuf, &ipar, sizeof (ipar));
		pBuffers[0]=(LONG_IA64)pBuf;	//Store the pointers
		pBuffers[1]=(LONG_IA64)gDataBuf;
	}
	
	// Get the buffer used for the parameter's length.  It is assumed that
	// this buffer is large enough to fit a 4-byte integer
	
	if (lenBuf != NULL) {

		pLen = (UCHAR*) (*env)->GetByteArrayElements(env, gLenBuf, 0);
		memcpy (pLen, &atExec, sizeof (atExec));
		//Store the pointer
		pBuffers[2]=(LONG_IA64)pLen;
		pBuffers[3]=(LONG_IA64)gLenBuf;
	}
	(*env)->ReleaseLongArrayElements(env, buffers, pBuffers, 0);

	// Bind the parameter

	rc = SQLBindParameter (
		(HSTMT) hStmt,			// Statement handle
		(UWORD) ipar,			// Parameter number
		SQL_PARAM_INPUT_OUTPUT,		// Type of parameter
		Ctype,				// The C data type
		(SWORD) SQLtype,		// The SQL data type
		(SQLULEN) streamLength,		// Precision
		0,				// Scale
		pBuf,				// Pointer to parameter's data
		(SQLLEN)dataBufLen,		// Maximum length of parameter buffer
		(SQLLEN*) pLen);		// Pointer to parameter's length

	// Set the error code
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
}

//----------------------------------------------------------------------------
// bindInParameterBinary
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_sun_jdbc_odbc_JdbcOdbc_bindInParameterBinary
	(JNIEnv *env, jobject callingObject, jlong hStmt, jint ipar, 
	 jint SQLtype, jbyteArray value, jint cbColDef, jbyteArray dataBuf, 
	 jbyteArray lenBuf, jbyteArray errorCode, jlongArray buffers)
{
	// Get the data structure portion of the error code
	RETCODE	rc;
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	jobject	gDataBuf = (*env)->NewGlobalRef(env, dataBuf);
	jobject	gValue = (*env)->NewGlobalRef(env, value);
	jobject	gLenBuf = (*env)->NewGlobalRef(env, lenBuf);
	UCHAR*	rgbValue = NULL;
	SDWORD	lValue = 0;
	UCHAR*	pBuf = NULL;
	SQLLEN	lBuf = 0;
	UCHAR*	pLen = NULL;

	jlong* pBuffers=(jlong*)(*env)->GetLongArrayElements(env, buffers, 0);

	if (gValue) rgbValue = (UCHAR*) (*env)->GetByteArrayElements(env, gValue, 0);
	if (value) lValue = (*env)->GetArrayLength(env, value);

	pBuffers[4]=(LONG_IA64)rgbValue;  //Store the pointers
	pBuffers[5]=(LONG_IA64)gValue;

// Make a copy of the data coming in into the permanent data buffer.  This
// data is in native format.  If no buffer is provided, the data buffer
// will be NULL.

	if (dataBuf != NULL) {
		pBuf = (UCHAR*) (*env)->GetByteArrayElements(env, gDataBuf, 0);

		pBuffers[0]=(LONG_IA64)pBuf;	//Store the pointers
		pBuffers[1]=(LONG_IA64)gDataBuf;

		lBuf = (*env)->GetArrayLength(env, dataBuf);
  
		// Sanity check the length
		if (lBuf > lValue) {
			lBuf = lValue;
		}

		// Make a copy of the data
		memcpy (pBuf, rgbValue, lBuf);
	}
	
	// Get the buffer used for the parameter's length.  It is assumed that
	// this buffer is large enough to fit a 4-byte integer
	
	if (lenBuf != NULL) {
		pLen = (UCHAR*) (*env)->GetByteArrayElements(env, gLenBuf, 0);
		memcpy (pLen, &lBuf, sizeof(lBuf));
		//Store the pointer
		pBuffers[2]=(LONG_IA64)pLen;
		pBuffers[3]=(LONG_IA64)gLenBuf;
	}
	(*env)->ReleaseLongArrayElements(env, buffers, pBuffers, 0);
	
// Bind the binary parameter

	rc = SQLBindParameter (
		(HSTMT) hStmt,		// Statement handle
		(UWORD) ipar,		// Parameter number
		SQL_PARAM_INPUT,	// Type of parameter
		SQL_C_BINARY,		// The C data type
		(SWORD) SQLtype,	// The SQL data type
		(SQLULEN) cbColDef,	// Precision
		0,			// Scale
		pBuf,			// Pointer to parameter's data
		lBuf,			// Maximum length of parameter buffer
		(SQLLEN*) pLen);	// Pointer to parameter's length

	// Set the error code
	
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
}

//----------------------------------------------------------------------------
// bindInParameterDate
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_sun_jdbc_odbc_JdbcOdbc_bindInParameterDate
	(JNIEnv *env, jobject callingObject, jlong hStmt, jint ipar, 
	 jint year, jint month, jint day, jbyteArray dataBuf, jbyteArray errorCode,
	 jlongArray buffers)
{
	// Get the data structure portion of the error code
	RETCODE	rc;
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	jobject gDataBuf = (*env)->NewGlobalRef(env, dataBuf);
	UCHAR*	pBuf = NULL;
	DATE_STRUCT dt;
	SQLLEN	lBuf = sizeof (dt);
	
	jlong* pBuffers=(jlong*)(*env)->GetLongArrayElements(env, buffers, 0);

// Construct our date structure
	memset (&dt, 0x00, sizeof (dt));
	dt.year = (short) year;
	dt.month = (short) month;
	dt.day = (short) day;
	
// Make a copy of the data coming in into the permanent data buffer.  This
// data is in native format.  If no buffer is provided, the data buffer
// will be NULL.  If a buffer is provided, it is assumed to be large enough
// to fit the date structure

	if (dataBuf != NULL) {
		pBuf = (UCHAR*) (*env)->GetByteArrayElements(env, gDataBuf, 0);
		// Make a copy of the incoming value
		memcpy (pBuf, &dt, lBuf);
		pBuffers[0]=(LONG_IA64)pBuf;
		pBuffers[1]=(LONG_IA64)gDataBuf;
	}

	(*env)->ReleaseLongArrayElements(env, buffers, pBuffers, 0);
	
// Bind the parameter

	rc = SQLBindParameter (
		(HSTMT) hStmt,		// Statement handle
		(UWORD) ipar,		// Parameter number
		SQL_PARAM_INPUT,	// Type of parameter
		SQL_C_DATE,		// The C data type
		SQL_DATE,		// The SQL data type
		(SQLULEN)10,		// Precision
		0,			// Scale
		pBuf,			// Pointer to parameter's data
		lBuf,			// Maximum length of parameter buffer
		NULL);			// Pointer to parameter's length

	// Set the error code
	
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
}

//----------------------------------------------------------------------------
// bindInParameterDouble
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_sun_jdbc_odbc_JdbcOdbc_bindInParameterDouble
	(JNIEnv *env, jobject callingObject, jlong hStmt, jint ipar, 
	 jint SQLtype, jint scale, jdouble value, jbyteArray dataBuf, 
	 jbyteArray errorCode, jlongArray buffers)
{
	// Get the data structure portion of the error code
	RETCODE rc;
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	UCHAR*	pBuf = NULL;
	jobject gDataBuf = (*env)->NewGlobalRef(env, dataBuf);
	SQLLEN	lBuf = sizeof (value);
	jlong* pBuffers=(jlong*)(*env)->GetLongArrayElements(env, buffers, 0);
	// Make a copy of the data coming in into the permanent data buffer.  This
	// data is in native format.  If no buffer is provided, the data buffer
	// will be NULL.  If a buffer is provided, it is assumed to be large enough
	// to fit the float value

	if (dataBuf != NULL) {
		pBuf = (UCHAR*) (*env)->GetByteArrayElements(env, gDataBuf, 0);
		// Make a copy of the incoming value
		memcpy (pBuf, &value, lBuf);
		pBuffers[0]=(LONG_IA64)pBuf;
		pBuffers[1]=(LONG_IA64)gDataBuf;
	}

	(*env)->ReleaseLongArrayElements(env, buffers, pBuffers, 0);
	
// Bind the parameter

	rc = SQLBindParameter (
		(HSTMT) hStmt,		// Statement handle
		(UWORD) ipar,		// Parameter number
		SQL_PARAM_INPUT,	// Type of parameter
		SQL_C_DOUBLE,		// The C data type
		(SWORD) SQLtype,	// The SQL data type
		(SQLULEN)18,		// Precision
		(SWORD) scale,		// Scale
		pBuf,			// Pointer to parameter's data
		lBuf,			// Maximum length of parameter buffer
		NULL);			// Pointer to parameter's length

	// Set the error code
	
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
}

//Bug 4495452
//----------------------------------------------------------------------------
// bindInParameterBigint
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_sun_jdbc_odbc_JdbcOdbc_bindInParameterBigint
	(JNIEnv *env, jobject callingObject, jlong hStmt, jint ipar, 
	 jint SQLtype, jint scale, jlong value, jbyteArray dataBuf, 
	 jbyteArray errorCode, jlongArray buffers)
{
	// Get the data structure portion of the error code
	RETCODE	rc;
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	jobject gDataBuf = (*env)->NewGlobalRef(env, dataBuf);
	UCHAR*	pBuf = NULL;
	SQLLEN	lBuf = sizeof (value);
	jlong* pBuffers=(jlong*)(*env)->GetLongArrayElements(env, buffers, 0);
	
	// Make a copy of the data coming in into the permanent data buffer.  This
	// data is in native format.  If no buffer is provided, the data buffer
	// will be NULL.  If a buffer is provided, it is assumed to be large enough
	// to fit the float value
	
	if (dataBuf != NULL) {
		pBuf = (UCHAR*) (*env)->GetByteArrayElements(env, gDataBuf, 0);
		// Make a copy of the incoming value
		memcpy (pBuf, &value, lBuf);
		pBuffers[0]=(LONG_IA64)pBuf;
		pBuffers[1]=(LONG_IA64)gDataBuf;
	}

	(*env)->ReleaseLongArrayElements(env, buffers, pBuffers, 0);
	// Bind the parameter
	
	rc = SQLBindParameter (
		(HSTMT) hStmt,		// Statement handle
		(UWORD) ipar,		// Parameter number
		SQL_PARAM_INPUT,	// Type of parameter
		SQL_C_SBIGINT,		// The C data type ODBC 3.x datatype
		(SWORD) SQLtype,	// The SQL data type
		(SQLULEN)19,		// Precision
		(SWORD) scale,		// Scale
		pBuf,			// Pointer to parameter's data
		lBuf,			// Maximum length of parameter buffer
		NULL);			// Pointer to parameter's length

	// Set the error code
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
}


//----------------------------------------------------------------------------
// bindInParameterFloat
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_sun_jdbc_odbc_JdbcOdbc_bindInParameterFloat
	(JNIEnv *env, jobject callingObject, jlong hStmt, jint ipar, 
	 jint SQLtype, jint scale, jdouble value, jbyteArray dataBuf, 
	 jbyteArray errorCode, jlongArray buffers)
{
	// Get the data structure portion of the error code
	RETCODE	rc;
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	jobject gDataBuf = (*env)->NewGlobalRef(env, dataBuf);
	UCHAR*	pBuf = NULL;
	SQLLEN	lBuf = sizeof (value);
	jlong* pBuffers=(jlong*)(*env)->GetLongArrayElements(env, buffers, 0);


	// Make a copy of the data coming in into the permanent data buffer.  This
	// data is in native format.  If no buffer is provided, the data buffer
	// will be NULL.  If a buffer is provided, it is assumed to be large enough
	// to fit the float value

	if (dataBuf != NULL) {
		pBuf = (UCHAR*) (*env)->GetByteArrayElements(env, gDataBuf, 0);
		// Make a copy of the incoming value
		memcpy (pBuf, &value, lBuf);
		pBuffers[0]=(LONG_IA64)pBuf;
		pBuffers[1]=(LONG_IA64)gDataBuf;
	}

	(*env)->ReleaseLongArrayElements(env, buffers, pBuffers, 0);
	// Bind the parameter

	rc = SQLBindParameter (
		(HSTMT) hStmt,		// Statement handle
		(UWORD) ipar,		// Parameter number
		SQL_PARAM_INPUT,	// Type of parameter
		// Fix 4532167
		// Using Double instead of real to accommodate maximum value of java float
		SQL_C_DOUBLE,           // The C data type
		(SWORD) SQL_DOUBLE,     // The SQL data type
		(SQLULEN)18,		// Precision
		(SWORD) scale,		// Scale
		pBuf,			// Pointer to parameter's data
		lBuf,			// Maximum length of parameter buffer
		NULL);			// Pointer to parameter's length

	// Set the error code
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
}


//----------------------------------------------------------------------------
// bindInParameterInteger
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_sun_jdbc_odbc_JdbcOdbc_bindInParameterInteger
	(JNIEnv *env, jobject callingObject, jlong hStmt, jint ipar,
	 jint SQLtype, jint value, jbyteArray dataBuf, jbyteArray errorCode, 
	 jlongArray buffers)
{

	// Get the data structure portion of the error code
	RETCODE	rc;
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	UCHAR*	pBuf = NULL;
	jobject gDataBuf = (*env)->NewGlobalRef(env, dataBuf);
	SQLULEN	lBuf = sizeof (value);

	jlong* pBuffers=(jlong*)(*env)->GetLongArrayElements(env, buffers, 0);

	// Make a copy of the data coming in into the permanent data buffer.  This
	// data is in native format.  If no buffer is provided, the data buffer
	// will be NULL.  If a buffer is provided, it is assumed to be large enough
	// to fit the value

	if (dataBuf != NULL) {
		pBuf = (UCHAR*) (*env)->GetByteArrayElements(env, gDataBuf, 0);

		// Make a copy of the incoming value
		memcpy (pBuf, &value, lBuf);
		pBuffers[0]= (LONG_IA64) pBuf;
		pBuffers[1]= (LONG_IA64) gDataBuf;				
	}
	(*env)->ReleaseLongArrayElements(env, buffers, pBuffers, 0);

	// Bind the parameter

	rc = SQLBindParameter (
		(HSTMT) hStmt,		// Statement handle
		(UWORD) ipar,		// Parameter number
		SQL_PARAM_INPUT,	// Type of parameter
		SQL_C_SLONG,		// The C data type
		(SWORD) SQLtype,	// The SQL data type
		(SQLULEN)lBuf,		// Precision
		0,			// Scale
		pBuf,			// Pointer to parameter's data
		(SQLLEN)lBuf,		// Maximum length of parameter buffer
		NULL);			// Pointer to parameter's length
		

	// Set the error code
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
}


//----------------------------------------------------------------------------
// bindInParameterNull
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_sun_jdbc_odbc_JdbcOdbc_bindInParameterNull
	(JNIEnv *env, jobject callingObject, jlong hStmt, jint ipar, 
	 jint SQLtype, jint jprec, jint jscale, jbyteArray lenBuf, 
	 jbyteArray errorCode, jlongArray buffers)
{
	// Get the data structure portion of the error code
	RETCODE	rc;
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	jobject gLenBuf = (*env)->NewGlobalRef(env, lenBuf);
	UCHAR*	pLen = NULL;
	SQLLEN	lenValue = SQL_NULL_DATA;
	SQLULEN	prec = 0;
	SWORD  scale = 0;
	
	jlong* pBuffers=(jlong*)(*env)->GetLongArrayElements(env, buffers, 0);
	
	if (gLenBuf) pLen = (UCHAR*) (*env)->GetByteArrayElements(env, gLenBuf, 0);

	pBuffers[0]=(LONG_IA64)pLen;
	pBuffers[1]=(LONG_IA64)gLenBuf;

	(*env)->ReleaseLongArrayElements(env, buffers, pBuffers, 0);

	// Set the length parameter to indicate a null value
	
	if (pLen) memcpy (pLen, &lenValue, sizeof (lenValue));

	// If the SQL type is CHAR, set the precision.	This is a workaround
	// for Oracle prec = 1;
	// for DRDA and Dbase prec = 254;
	// for Sybase the precision = 255;

	switch (SQLtype)
	{
		case SQL_CHAR:
		case SQL_BINARY: // 4532171
		case SQL_VARBINARY:
		case SQL_LONGVARBINARY:
				prec = jprec;
				break;
		
		case SQL_NUMERIC:
		case SQL_DECIMAL:		
				prec = jprec;
				scale = (short)jscale;
				break;
		
		case SQL_DATE:		
				prec = 10;
				break;

		case SQL_TIME:			
				prec = 8;
				break;

		case SQL_TIMESTAMP:		
				prec = 29;
				scale = 9;
				break;

	}

	// Bind the parameter to NULL
	
	rc = SQLBindParameter (
		(HSTMT) hStmt,			// Statement handle
		(UWORD) ipar,			// Parameter number
		SQL_PARAM_INPUT,		// Type of parameter
		SQL_C_DEFAULT,			// The C data type
		(SWORD) SQLtype,		// The SQL data type
		(SQLULEN)prec,			// Precision
		scale,				// Scale
		NULL,				// Pointer to parameter's data
		(SQLLEN) 0,			// Maximum length of parameter buffer
		(SQLLEN*) pLen);		// Pointer to parameter's length

	// Set the error code
	
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);

}

//----------------------------------------------------------------------------
// bindInParameterString
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_sun_jdbc_odbc_JdbcOdbc_bindInParameterString
	(JNIEnv *env, jobject callingObject, jlong hStmt, jint ipar, 
	 jint SQLtype, jbyteArray value, jint cbColDef, jint ibScale, 
	 jbyteArray dataBuf, jbyteArray errorCode, jlongArray buffers)
{
	// Get the data structure portion of the error code
	RETCODE	rc;
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	jobject gDataBuf = (*env)->NewGlobalRef(env, dataBuf);
	jobject gValue = (*env)->NewGlobalRef(env, value);
	UCHAR*	rgbValue = NULL;
	UCHAR*	pBuf = NULL;
	SQLLEN	lBuf = 0;

	jlong* pBuffers = (jlong*)(*env)->GetLongArrayElements(env, buffers, 0);

	if (gValue) rgbValue = (UCHAR*) (*env)->GetByteArrayElements(env, gValue, 0);

	pBuffers[2] = (LONG_IA64)rgbValue;
	pBuffers[3] = (LONG_IA64)gValue;
	// Make a copy of the data coming in into the permanent data buffer.  This
	// data is in native format.  If no buffer is provided, the data buffer
	// will be NULL.

	if (dataBuf != NULL) {
		pBuf = (UCHAR*) (*env)->GetByteArrayElements(env, gDataBuf, 0);
		lBuf = (*env)->GetArrayLength(env, dataBuf);
		pBuffers[0]=(LONG_IA64)pBuf;
		pBuffers[1]=(LONG_IA64)gDataBuf;
		// Sanity check the length
		if (lBuf > (SQLLEN) strlen ((LPCSTR) rgbValue)) {
			lBuf = strlen ((LPCSTR) rgbValue);
		}

		// Make a copy of the data
		memcpy (pBuf, rgbValue, lBuf);
	}
			
	(*env)->ReleaseLongArrayElements(env, buffers, pBuffers, 0);	

// Bind the string parameter

	rc = SQLBindParameter (
		(HSTMT) hStmt,		// Statement handle
		(UWORD) ipar,		// Parameter number
		SQL_PARAM_INPUT,	// Type of parameter
		SQL_C_CHAR,		// The C data type
		(SWORD) SQLtype,	// The SQL data type
		(SQLULEN) cbColDef,	// Precision
		(SWORD) ibScale,	// Scale
		pBuf,			// Pointer to parameter's data
		lBuf,			// Maximum length of parameter buffer
		NULL);			// Pointer to parameter's length

	// Set the error code
	
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
}

//----------------------------------------------------------------------------
// bindInParameterTime
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_sun_jdbc_odbc_JdbcOdbc_bindInParameterTime
	(JNIEnv *env, jobject callingObject, jlong hStmt, jint ipar, 
	 jint hour, jint minutes, jint seconds, jbyteArray dataBuf, 
	 jbyteArray errorCode, jlongArray buffers)
{
	// Get the data structure portion of the error code
	RETCODE	rc;
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	UCHAR*	pBuf = NULL;
	jobject gDataBuf = (*env)->NewGlobalRef(env, dataBuf);
	TIME_STRUCT tm;
	jlong* pBuffers=(jlong*)(*env)->GetLongArrayElements(env, buffers, 0);
	SQLLEN	lBuf = sizeof (tm);
	


// Construct our time structure
	memset (&tm, 0x00, sizeof (tm));
	tm.hour = (short) hour;
	tm.minute = (short) minutes;
	tm.second = (short) seconds;
	
// Make a copy of the data coming in into the permanent data buffer.  This
// data is in native format.  If no buffer is provided, the data buffer
// will be NULL.  If a buffer is provided, it is assumed to be large enough
// to fit the time structure

	if (dataBuf != NULL) {
		pBuf = (UCHAR*) (*env)->GetByteArrayElements(env, gDataBuf, 0);

		// Make a copy of the incoming value
		memcpy (pBuf, &tm, lBuf);
		pBuffers[0]=(LONG_IA64)pBuf;
		pBuffers[1]=(LONG_IA64)gDataBuf;
	}
	(*env)->ReleaseLongArrayElements(env, buffers, pBuffers, 0);
	
// Bind the parameter

	rc = SQLBindParameter (
		(HSTMT) hStmt,		// Statement handle
		(UWORD) ipar,		// Parameter number
		SQL_PARAM_INPUT,	// Type of parameter
		SQL_C_TIME,		// The C data type
		SQL_TIME,		// The SQL data type
		(SQLULEN)8,		// Precision
		0,			// Scale
		pBuf,			// Pointer to parameter's data
		lBuf,			// Maximum length of parameter buffer
		NULL);			// Pointer to parameter's length

	// Set the error code
	
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
}

//----------------------------------------------------------------------------
// bindInParameterTimestamp
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_sun_jdbc_odbc_JdbcOdbc_bindInParameterTimestamp (
	JNIEnv *env, jobject callingObject, 
	jlong hStmt, 
	jint ipar, 
	jint year, 
	jint month, 
	jint day, 
	jint hour, 
	jint minutes, 
	jint seconds, 
	jint nanos,
	jbyteArray dataBuf, 
	jbyteArray errorCode,
	jlongArray buffers)
{
	// Get the data structure portion of the error code
	RETCODE	rc;
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	UCHAR*	pBuf = NULL;
	char  cNanos[10];
	TIMESTAMP_STRUCT ts;
	jobject gDataBuf = (*env)->NewGlobalRef(env, dataBuf);
	jlong* pBuffers=(jlong*)(*env)->GetLongArrayElements(env, buffers, 0);

	SQLLEN	lBuf = sizeof (ts);
	
	SQLULEN prec=0;
	int scale=0;

// Construct our timestamp structure
	memset (&ts, 0x00, sizeof (ts));
	ts.year = (short) year;
	ts.month = (short) month;
	ts.day = (short) day;
	ts.hour = (short) hour;
	ts.minute = (short) minutes;
	ts.second = (short) seconds;
	ts.fraction = nanos;
	
// Make a copy of the data coming in into the permanent data buffer.  This
// data is in native format.  If no buffer is provided, the data buffer
// will be NULL.  If a buffer is provided, it is assumed to be large enough
// to fit the timestamp structure

	if (dataBuf != NULL) {
		pBuf = (UCHAR*) (*env)->GetByteArrayElements(env, gDataBuf, 0);

		// Make a copy of the incoming value
		memcpy (pBuf, &ts, lBuf);
		pBuffers[0]=(LONG_IA64)pBuf;
		pBuffers[1]=(LONG_IA64)gDataBuf;
	}

	(*env)->ReleaseLongArrayElements(env, buffers, pBuffers, 0);
	
// Fix 4238983.
	sprintf(cNanos,"%d",ts.fraction);
	for (scale = strlen(cNanos); scale > 0; scale--){
		if (cNanos[scale-1] != '0'){
	    		break;
		}
	}
	
	if (ts.fraction==0) {
		scale = 1;
	}
	
	prec = 20 + scale;	
	
// Bind the parameter
	
	rc = SQLBindParameter (
		(HSTMT) hStmt,		// Statement handle
		(UWORD) ipar,		// Parameter number
		SQL_PARAM_INPUT,	// Type of parameter
		SQL_C_TIMESTAMP,	// The C data type
		SQL_TIMESTAMP,		// The SQL data type
		(SQLULEN)prec,		// Precision
		scale,			// Scale
		pBuf,			// Pointer to parameter's data
		lBuf,			// Maximum length of parameter buffer
		NULL);			// Pointer to parameter's length

	// Set the error code
	
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
}

//----------------------------------------------------------------------------
// bindInParameterStringArray
//----------------------------------------------------------------------------
JNIEXPORT void JNICALL Java_sun_jdbc_odbc_JdbcOdbc_bindInParameterStringArray (
	 JNIEnv *env, jobject callingObject,
	 jlong hStmt,
	 jint ipar,
	 jint SQLtype,
	 jobjectArray pBuf,
	 jbyteArray strBuf,
	 jint descLen,
	 jint scale,
	 jintArray indexbuffer,
 	 jbyteArray errorCode)

{
	
	// Get the data structure portion of the error code
	RETCODE	rc;
	UCHAR*	errCode = (UCHAR*)(*env)->GetByteArrayElements(env, errorCode, 0);
	SQLLEN* lenInd = NULL;
	int i;
	
	// what is the size and length of the array.
	int sarray = 0;		
	int maxLen = 0;
	
	UCHAR* strData = NULL;
	UCHAR* cpyData = NULL;

	if (indexbuffer) lenInd = (jint*)(*env)->GetIntArrayElements(env, indexbuffer, 0);
	if (pBuf) sarray = (*env)->GetArrayLength(env, pBuf);		
	maxLen = (descLen + 1) * sarray;
	if (strBuf) strData = (UCHAR*)(*env)->GetByteArrayElements(env, strBuf, 0);

	// Init the string buffers to an empty string
	if (strData) memset (strData, '\0', maxLen);
		
	// this loop populates the strData with null terminated strings 
	for (i = 0; i < sarray; i++)
	{
		// Get the String, then copy to strData buffer and release it.
		jstring jstr = (*env)->GetObjectArrayElement(env, pBuf, i);

		if (jstr != NULL)
		{
			cpyData = (char *)((*env)->GetStringUTFChars(env, jstr, NULL));
			strcpy ( (strData + ((descLen + 1) * i)), cpyData);
			(*env)->ReleaseStringUTFChars (env, jstr, cpyData);
		}
		
	}


	rc = SQLBindParameter (
		(HSTMT)hStmt,			// Statement handle
		(UWORD)ipar,			// Parameter number
		SQL_PARAM_INPUT,		// Type of parameter
		SQL_C_CHAR,			// The C data type
	    	(SWORD) SQLtype,		// The SQL data type
		(SQLULEN)descLen,		// Precision
		(SWORD) scale,			// Scale
		strData,			// Pointer to parameter data buffer
		(SQLLEN) descLen + 1,		// Maximum length of parameter
		(SQLLEN*)lenInd);		// Pointer to parameter's length/Index.
		
	// Set the error code
	
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
	(*env)->ReleaseIntArrayElements(env, indexbuffer, lenInd, 0);
	(*env)->ReleaseByteArrayElements(env, strBuf, strData, 0);	
	
}

//----------------------------------------------------------------------------
// bindInParameterIntegerArray
//----------------------------------------------------------------------------
JNIEXPORT void JNICALL Java_sun_jdbc_odbc_JdbcOdbc_bindInParameterIntegerArray (
	 JNIEnv *env, jobject callingObject,
	 jlong hStmt,
	 jint ipar,
	 jint SQLtype,
	 jintArray pBuf, 
	 jintArray buffers,
 	 jbyteArray errorCode)

{
	// Get the data structure portion of the error code
	RETCODE	rc;
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);

	jint* pData = NULL;
	jint* lenInd = NULL;
		
	if (pBuf) pData = (jint*)(*env)->GetIntArrayElements(env, pBuf, 0);
	if (buffers) lenInd = (jint*)(*env)->GetIntArrayElements(env, buffers, 0);

	rc = SQLBindParameter (
		(HSTMT)hStmt,
		(UWORD)ipar,
		SQL_PARAM_INPUT,	// Type of parameter
		SQL_C_SLONG,		// The C data type
		(SWORD) SQLtype,	// The SQL data type
		(SQLULEN)0,		// Precision
		0,			// Scale
		pData,
		0, 
		(SQLLEN*)lenInd);


	// Set the error code
	
	errCode[0] = (char) rc;
	
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
	(*env)->ReleaseIntArrayElements(env, buffers, lenInd, 0);
	(*env)->ReleaseIntArrayElements(env, pBuf, pData, 0);
	
}


//----------------------------------------------------------------------------
// bindInParameterFloatArray
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_sun_jdbc_odbc_JdbcOdbc_bindInParameterFloatArray (
	 JNIEnv *env, jobject callingObject,
	 jlong hStmt,
	 jint ipar,	 
	 jint SQLtype,
	 jint scale,
	 jfloatArray pBuf, 
	 jintArray buffers,
 	 jbyteArray errorCode)
{
	// Get the data structure portion of the error code
	RETCODE	rc;
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);

	float* pData = NULL;
	jint* lenInd = NULL;
	SQLLEN	lBuf = sizeof (pData);

	if (pBuf) pData=(float*)(*env)->GetFloatArrayElements(env, pBuf, 0);
	if (buffers) lenInd = (jint*)(*env)->GetIntArrayElements(env, buffers, 0);

	// Bind the parameter

	rc = SQLBindParameter (
		(HSTMT) hStmt,		// Statement handle
		(UWORD) ipar,		// Parameter number
		SQL_PARAM_INPUT,	// Type of parameter
		SQL_C_FLOAT,		// The C data type
		(SWORD) SQLtype,	// The SQL data type
		(SQLULEN)18,		// Precision
		(SWORD) scale,		// Scale
		pData,			// Pointer to parameter's data
		lBuf,			// Maximum length of parameter buffer
		(SQLLEN*)lenInd);	// Pointer to parameter's length

	// Set the error code
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
	(*env)->ReleaseFloatArrayElements(env, pBuf, pData, 0);
	(*env)->ReleaseIntArrayElements(env, buffers, lenInd, 0);

}

//----------------------------------------------------------------------------
// bindInParameterDoubleArray
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_sun_jdbc_odbc_JdbcOdbc_bindInParameterDoubleArray (
	 JNIEnv *env, jobject callingObject,
	 jlong hStmt,
	 jint ipar,	 
	 jint SQLtype,
	 jint scale,
	 jdoubleArray pBuf, 
	 jintArray buffers,
 	 jbyteArray errorCode)
{
	// Get the data structure portion of the error code
	RETCODE	rc;
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);

	double* pData= NULL;
	jint* lenInd = NULL;
	SQLLEN	lBuf = sizeof (pData);

	if (pBuf) pData=(double*)(*env)->GetDoubleArrayElements(env, pBuf, 0);
	if (buffers) lenInd = (jint*)(*env)->GetIntArrayElements(env, buffers, 0);

	// Bind the parameter

	rc = SQLBindParameter (
		(HSTMT) hStmt,		// Statement handle
		(UWORD) ipar,		// Parameter number
		SQL_PARAM_INPUT,	// Type of parameter
		SQL_C_DOUBLE,		// The C data type
		(SWORD) SQLtype,	// The SQL data type
		(SQLULEN)18,		// Precision
		(SWORD) scale,		// Scale
		pData,			// Pointer to parameter's data
		lBuf,			// Maximum length of parameter buffer
		(SQLLEN*)lenInd);	// Pointer to parameter's length

	// Set the error code
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
	(*env)->ReleaseDoubleArrayElements(env, pBuf, pData, 0);
	(*env)->ReleaseIntArrayElements(env, buffers, lenInd, 0);

}

//----------------------------------------------------------------------------
// bindInParameterDateArray
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_sun_jdbc_odbc_JdbcOdbc_bindInParameterDateArray (
	JNIEnv *env, jobject callingObject, 
	jlong hStmt, 
	jint ipar, 
	jintArray year, 
	jintArray month, 
	jintArray day, 
	jbyteArray dataBuf, 
	jbyteArray errorCode,
	jintArray buffers)
{
	// Get the data structure portion of the error code
	RETCODE	rc;
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	UCHAR*	pBuf = NULL;
	
	DATE_STRUCT dt;

	SQLLEN	lBuf = sizeof (dt);

	int i;

	int arraySize = 0;

	SQLLEN* pBuffers= NULL;
	jint* pyears    = NULL;
	jint* pmonths   = NULL;
	jint* pdays     = NULL;

	if (buffers) arraySize = (*env)->GetArrayLength(env, buffers);

	if (buffers) pBuffers	=(jint*)(*env)->GetIntArrayElements(env, buffers, 0);
	if (year) pyears	=(jint*)(*env)->GetIntArrayElements(env, year, 0);
	if (month) pmonths	=(jint*)(*env)->GetIntArrayElements(env, month, 0);
	if (day) pdays		=(jint*)(*env)->GetIntArrayElements(env, day, 0);

	// Construct our Date structure
	memset (&dt, 0x00, sizeof (dt));

	if (dataBuf) pBuf = (UCHAR*) (*env)->GetByteArrayElements(env, dataBuf, 0);

	for (i = 0; i < arraySize; i++)
	{

		dt.year		= (short) pyears[i];
		dt.month	= (short) pmonths[i];
		dt.day		= (short) pdays[i];

		// Make a copy of the data coming in into the permanent data buffer.  This
	// data is in native format.  If no buffer is provided, the data buffer
	// will be NULL.  If a buffer is provided, it is assumed to be large enough
	// to fit the DATE structure

		memcpy (pBuf + (sizeof(dt) * i) , &dt, lBuf);

	}
	
	(*env)->ReleaseIntArrayElements(env, buffers, pBuffers, 0);
	(*env)->ReleaseIntArrayElements(env, year, pyears, 0);
	(*env)->ReleaseIntArrayElements(env, month, pmonths, 0);
	(*env)->ReleaseIntArrayElements(env, day, pdays, 0);
	
// Bind the parameter

	rc = SQLBindParameter (
		(HSTMT) hStmt,		// Statement handle
		(UWORD) ipar,		// Parameter number
		SQL_PARAM_INPUT,	// Type of parameter
		SQL_C_DATE,		// The C data type
		SQL_DATE,		// The SQL data type
		(SQLULEN)10,		// Precision
		0,			// Scale
		pBuf,			// Pointer to parameter's data
		lBuf,			// Maximum length of parameter buffer
		pBuffers);		// Pointer to parameter's length


	// Set the error code
	
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
	(*env)->ReleaseByteArrayElements(env, dataBuf, pBuf, 0);	
}


//----------------------------------------------------------------------------
// bindInParameterTimeArray
//----------------------------------------------------------------------------
JNIEXPORT void JNICALL Java_sun_jdbc_odbc_JdbcOdbc_bindInParameterTimeArray (
	JNIEnv *env, jobject callingObject, 
	jlong hStmt, 
	jint ipar, 
	jintArray hour, 
	jintArray minutes, 
	jintArray seconds, 
	jbyteArray dataBuf, 
	jbyteArray errorCode,
	jintArray buffers)
{
	// Get the data structure portion of the error code
	RETCODE	rc;
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	UCHAR*	pBuf = NULL;
	
	TIME_STRUCT tm;

	SQLLEN	lBuf = sizeof (tm);

	int i;

	int arraySize = 0;

	SQLLEN* pBuffers =NULL;
	jint* phours  	 =NULL;
	jint* pminutes	 =NULL;
	jint* pseconds	 =NULL;

	if (buffers) arraySize = (*env)->GetArrayLength(env, buffers);

	if (buffers) pBuffers=(jint*)(*env)->GetIntArrayElements(env, buffers, 0);
	if (hour)    phours=(jint*)(*env)->GetIntArrayElements(env, hour, 0);
	if (minutes) pminutes=(jint*)(*env)->GetIntArrayElements(env, minutes, 0);
	if (seconds) pseconds=(jint*)(*env)->GetIntArrayElements(env, seconds, 0);

	// Construct our Time structure
	memset (&tm, 0x00, sizeof (tm));

	if (dataBuf) pBuf = (UCHAR*) (*env)->GetByteArrayElements(env, dataBuf, 0);
	
	for (i = 0; i < arraySize; i++)
	{
		tm.hour		= (short) phours[i];
		tm.minute	= (short) pminutes[i];
		tm.second	= (short) pseconds[i];
		
	// Make a copy of the data coming in into the permanent data buffer.  This
	// data is in native format.  If no buffer is provided, the data buffer
	// will be NULL.  If a buffer is provided, it is assumed to be large enough
	// to fit the Time structure

		memcpy (pBuf + (sizeof(tm) * i) , &tm, lBuf);

	}
	
	(*env)->ReleaseIntArrayElements(env, buffers, pBuffers, 0);
	(*env)->ReleaseIntArrayElements(env, hour, phours, 0);
	(*env)->ReleaseIntArrayElements(env, minutes, pminutes, 0);
	(*env)->ReleaseIntArrayElements(env, seconds, pseconds, 0);
	
// Bind the parameter
	rc = SQLBindParameter (
		(HSTMT) hStmt,			// Statement handle
		(UWORD) ipar,			// Parameter number
		SQL_PARAM_INPUT,		// Type of parameter
		SQL_C_TIME,			// The C data type
		SQL_TIME,			// The SQL data type
		(SQLULEN)8,			// Precision
		0,				// Scale
		pBuf,				// Pointer to parameter's data
		lBuf,				// Maximum length of parameter buffer
		pBuffers);			// Pointer to parameter's length

	// Set the error code
	
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
	(*env)->ReleaseByteArrayElements(env, dataBuf, pBuf, 0);	
}


//----------------------------------------------------------------------------
// bindInParameterTimestampArray
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_sun_jdbc_odbc_JdbcOdbc_bindInParameterTimestampArray (
	JNIEnv *env, jobject callingObject, 
	jlong hStmt, 
	jint ipar, 
	jintArray year, 
	jintArray month, 
	jintArray day, 
	jintArray hour, 
	jintArray minutes, 
	jintArray seconds, 
	jintArray nanos,
	jbyteArray dataBuf, 
	jbyteArray errorCode,
	jintArray buffers)
{
	// Get the data structure portion of the error code
	RETCODE	rc;
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	UCHAR*	pBuf = NULL;
	
	TIMESTAMP_STRUCT ts;

	SQLLEN	lBuf = sizeof (ts);

	int i;

	int arraySize = 0;

	SQLLEN* pBuffers=NULL;
	jint* pyears  	=NULL;
	jint* pmonths	=NULL;
	jint* pdays   	=NULL;
	jint* phours  	=NULL;
	jint* pminutes	=NULL;
	jint* pseconds	=NULL;
	jint* pnanos  	=NULL;

	if (buffers) arraySize = (*env)->GetArrayLength(env, buffers);

	if (buffers) pBuffers	=(jint*)(*env)->GetIntArrayElements(env, buffers, 0);
	if (year) pyears  	=(jint*)(*env)->GetIntArrayElements(env, year, 0);
	if (month) pmonths	=(jint*)(*env)->GetIntArrayElements(env, month, 0);
	if (day) pdays		=(jint*)(*env)->GetIntArrayElements(env, day, 0);
	if (hour) phours   	=(jint*)(*env)->GetIntArrayElements(env, hour, 0);
	if (minutes) pminutes	=(jint*)(*env)->GetIntArrayElements(env, minutes, 0);
	if (seconds) pseconds	=(jint*)(*env)->GetIntArrayElements(env, seconds, 0);
	if (nanos) pnanos   	=(jint*)(*env)->GetIntArrayElements(env, nanos, 0);

	// Construct our timestamp structure
	memset (&ts, 0x00, sizeof (ts));

	if (dataBuf) pBuf = (UCHAR*) (*env)->GetByteArrayElements(env, dataBuf, 0);


	for (i = 0; i < arraySize; i++)
	{

		ts.year		= (short) pyears[i];
		ts.month	= (short) pmonths[i];
		ts.day		= (short) pdays[i];
		ts.hour		= (short) phours[i];
		ts.minute	= (short) pminutes[i];
		ts.second	= (short) pseconds[i];
		ts.fraction = pnanos[i];
		
	// Make a copy of the data coming in into the permanent data buffer.  This
	// data is in native format.  If no buffer is provided, the data buffer
	// will be NULL.  If a buffer is provided, it is assumed to be large enough
	// to fit the timestamp structure

		memcpy (pBuf + (sizeof(ts) * i) , &ts, lBuf);

	}
	
	(*env)->ReleaseIntArrayElements(env, buffers, pBuffers, 0);
	(*env)->ReleaseIntArrayElements(env, year, pyears, 0);
	(*env)->ReleaseIntArrayElements(env, month, pmonths, 0);
	(*env)->ReleaseIntArrayElements(env, day, pdays, 0);
	(*env)->ReleaseIntArrayElements(env, hour, phours, 0);
	(*env)->ReleaseIntArrayElements(env, minutes, pminutes, 0);
	(*env)->ReleaseIntArrayElements(env, seconds, pseconds, 0);
	(*env)->ReleaseIntArrayElements(env, nanos, pnanos, 0);
	
// Bind the parameter

	rc = SQLBindParameter (
		(HSTMT) hStmt,		// Statement handle
		(UWORD) ipar,		// Parameter number
		SQL_PARAM_INPUT,	// Type of parameter
		SQL_C_TIMESTAMP,	// The C data type
		SQL_TIMESTAMP,		// The SQL data type
		(SQLULEN)29,		// Precision
		9,			// Scale
		pBuf,			// Pointer to parameter's data
		lBuf,			// Maximum length of parameter buffer
		pBuffers);		// Pointer to parameter's length

	// Set the error code
	
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
	(*env)->ReleaseByteArrayElements(env, dataBuf, pBuf, 0);	
}

//----------------------------------------------------------------------------
// bindInParameterBinaryArray
//----------------------------------------------------------------------------
JNIEXPORT void JNICALL Java_sun_jdbc_odbc_JdbcOdbc_bindInParameterBinaryArray (
	 JNIEnv *env, jobject callingObject,
	 jlong hStmt,
	 jint ipar,
	 jint SQLtype,
	 jobjectArray pBuf,
	 jint descLen,
	 jbyteArray byteBuf,
	 jintArray indexbuffer,
 	 jbyteArray errorCode)

{
	
	// Get the data structure portion of the error code
	RETCODE	rc;
	UCHAR*	errCode = (UCHAR*)(*env)->GetByteArrayElements(env, errorCode, 0);
	SQLLEN* lenInd = NULL;

	int i;
	
	// what is the size and length of the array.
	int sarray = 0;		
	int maxLen = 0;
	
	UCHAR* byteData = NULL;
	UCHAR* cpyData = NULL;

	if (indexbuffer) lenInd = (jint*)(*env)->GetIntArrayElements(env, indexbuffer, 0);
	if (pBuf) sarray = (*env)->GetArrayLength(env, pBuf);		
	if (byteBuf) maxLen = (*env)->GetArrayLength(env, byteBuf);
	
	if (byteBuf) byteData = (UCHAR*)(*env)->GetByteArrayElements(env, byteBuf, 0);

	// Init the string buffers to an empty string
	if (byteData) memset (byteData, 0x00, maxLen);
		
	// this loop populates the byteData buffer.
	for (i = 0; i < sarray; i++)
	{
		jbyteArray jbyteData = (*env)->GetObjectArrayElement(env, pBuf, i);

		if (jbyteData != NULL)
		{
			cpyData = (char *)((*env)->GetByteArrayElements(env, jbyteData, 0));

			if (cpyData != NULL)
			{
				lenInd[i] = (*env)->GetArrayLength(env, jbyteData);
				memcpy ( (byteData + (descLen * i)), cpyData, descLen);
			}
		}			

		(*env)->ReleaseByteArrayElements(env, jbyteData, cpyData, 0);
	}

	rc = SQLBindParameter (
		(HSTMT)hStmt,			// Statement handle
		(UWORD)ipar,			// Parameter number
		SQL_PARAM_INPUT,		// Type of parameter
		SQL_C_BINARY,			// The C data type
	    	(SWORD) SQLtype,		// The SQL data type
		(SQLULEN)descLen,		// Precision
		0,				// Scale
		byteData,			// Pointer to parameter data buffer
		(SQLLEN)descLen,		// Maximum length of parameter
		lenInd);			// Pointer to parameter's length/Index.
		
	// Set the error code
	
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
	(*env)->ReleaseIntArrayElements(env, indexbuffer, lenInd, 0);
	(*env)->ReleaseByteArrayElements(env, byteBuf, byteData, 0);	
	
}


//----------------------------------------------------------------------------
// bindInParameterAtExecArray
// Used for binding array of Streams.
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_sun_jdbc_odbc_JdbcOdbc_bindInParameterAtExecArray
	(JNIEnv *env, jobject callingObject,
	 jlong hStmt,
	 jint ipar,
	 jint SQLtype,
	 jint len,
	 jbyteArray dataBuf,
	 jintArray lenBuf,
	 jbyteArray errorCode)

{
	// Get the data structure portion of the error code
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	RETCODE	rc;
	SWORD	Ctype = SQL_C_CHAR;
	UCHAR*	pBuf = NULL;

	// what is the size and length of the array.
	int sarray = 0;		
	int i = 0;

	jint* lenInd = NULL;
	
	if (lenBuf) sarray = (*env)->GetArrayLength(env, lenBuf);		

	if (lenBuf) lenInd = (jint*)(*env)->GetIntArrayElements(env, lenBuf, 0);

	if (dataBuf) pBuf = (UCHAR*) (*env)->GetByteArrayElements(env, dataBuf, 0);

	if (pBuf) memcpy (pBuf, &ipar, sizeof (ipar));

	for (i = 0; i < sarray; i++)
	{
		if (lenInd[i] > 0)
			lenInd[i] = SQL_LEN_DATA_AT_EXEC (lenInd[i]);				
	}		

	
// The SQL Type was given, make the C-Type match
// Only SQL_CHAR and SQL_BINARY are currently supported.
	
	if ((SQLtype == SQL_BINARY) ||
	    (SQLtype == SQL_VARBINARY) ||
	    (SQLtype == SQL_LONGVARBINARY)) {
		Ctype = SQL_C_BINARY;
	}

	// Bind the parameter

	rc = SQLBindParameter (
		(HSTMT) hStmt,		// Statement handle
		(UWORD) ipar,		// Parameter number
		SQL_PARAM_INPUT,	// Type of parameter
		Ctype,			// The C data type
		(SWORD) SQLtype,	// The SQL data type
		(SQLULEN) len,		// Precision
		0,			// Scale
		pBuf,			// Pointer to parameter's data
		(SQLLEN) sizeof (ipar),	// Maximum length of parameter buffer
		(SQLLEN*) lenInd);	// Pointer to parameter's length

	// Set the error code
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
	(*env)->ReleaseByteArrayElements(env, dataBuf, pBuf, 0);
	(*env)->ReleaseIntArrayElements(env, lenBuf, lenInd, 0);

}

//----------------------------------------------------------------------------
// bindOutParameterString
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL	Java_sun_jdbc_odbc_JdbcOdbc_bindOutParameterString (
	JNIEnv *env, jobject callingObject,
	jlong hStmt,
	jint ipar,
	jint SQLtype,
	jint scale,
	jbyteArray dataBuf,
	jbyteArray lenBuf,
	jbyteArray errorCode,
	jlongArray buffers)
{
	// Get the data structure portion of the error code
	jobject gDataBuf=(*env)->NewGlobalRef(env, dataBuf); 
	jobject gLenBuf=(*env)->NewGlobalRef(env, lenBuf);
	RETCODE	rc;
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	UCHAR*	pBuf = NULL;
	UCHAR*	pLen = NULL;
	SQLLEN	lBuf = 0;
	jlong* pBuffers=(jlong*)(*env)->GetLongArrayElements(env, buffers, 0);

	
	if (gDataBuf) pBuf = (UCHAR*) (*env)->GetByteArrayElements(env, gDataBuf, 0);
	if (gLenBuf) pLen = (UCHAR*) (*env)->GetByteArrayElements(env, gLenBuf, 0);
	if (dataBuf) lBuf = (*env)->GetArrayLength(env, dataBuf);

	pBuffers[0]=(LONG_IA64)pBuf;
	pBuffers[1]=(LONG_IA64)gDataBuf;
	pBuffers[2]=(LONG_IA64)pLen;
	pBuffers[3]=(LONG_IA64)gLenBuf;


	(*env)->ReleaseLongArrayElements(env, buffers, pBuffers, 0);

// Bind the output string parameter

	rc = SQLBindParameter (
		(HSTMT) hStmt,		// Statement handle
		(SWORD) ipar,		// Parameter number
		SQL_PARAM_OUTPUT,	// Type of parameter
		SQL_C_CHAR,		// The C data type
		(SWORD) SQLtype,	// The SQL data type
		(SQLULEN) lBuf - 1,	// Precision
		(SWORD) scale,		// Scale
		pBuf,			// Pointer to parameter data buffer
		lBuf,			// Maximum length of parameter
		(SQLLEN*) pLen);	// Pointer to parameter's length

	// Set the error code
	
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
}

// bug 4412437
//----------------------------------------------------------------------------
// bindInOutParameterDate
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL	Java_sun_jdbc_odbc_JdbcOdbc_bindInOutParameterDate (
	JNIEnv *env, jobject callingObject,
	jlong hStmt,
	jint ipar,
	jint scale,
	jbyteArray dataBuf,
	jbyteArray lenBuf,
	jbyteArray errorCode,
	jlongArray buffers)
{
	// Get the data structure portion of the error code
	jobject gDataBuf=(*env)->NewGlobalRef(env, dataBuf); 
	jobject gLenBuf=(*env)->NewGlobalRef(env, lenBuf);
	RETCODE	rc;
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	UCHAR*	pBuf = NULL;
	UCHAR*	pLen = NULL;
	SDWORD	lBuf = 0;
	jlong* pBuffers=(jlong*)(*env)->GetLongArrayElements(env, buffers, 0);
	
	if (gDataBuf) pBuf = (UCHAR*) (*env)->GetByteArrayElements(env, gDataBuf, 0);
	if (gLenBuf) pLen = (UCHAR*) (*env)->GetByteArrayElements(env, gLenBuf, 0);
	if (dataBuf) lBuf = (*env)->GetArrayLength(env, dataBuf);

	pBuffers[0]=(LONG_IA64)pBuf;
	pBuffers[1]=(LONG_IA64)gDataBuf;
	pBuffers[2]=(LONG_IA64)pLen;
	pBuffers[3]=(LONG_IA64)gLenBuf;

	(*env)->ReleaseLongArrayElements(env, buffers, pBuffers, 0);

	rc = SQLBindParameter (
		(HSTMT) hStmt,		// Statement handle
		(SWORD) ipar,		// Parameter number
		SQL_PARAM_INPUT_OUTPUT,	// Type of parameter
		SQL_C_DATE,		// The C data type
		SQL_DATE,		// The SQL data type
		(SQLULEN)0,		// Precision
		(SWORD) scale,		// Scale
		pBuf,			// Pointer to parameter data buffer
		0,			// Maximum length of parameter
		(SQLLEN*) pLen);	// Pointer to parameter's length

	// Set the error code
	
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
}

// 4412437
//----------------------------------------------------------------------------
// bindInOutParameterTime
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL	Java_sun_jdbc_odbc_JdbcOdbc_bindInOutParameterTime (
	JNIEnv *env, jobject callingObject,
	jlong hStmt,
	jint ipar,
	jint scale,
	jbyteArray dataBuf,
	jbyteArray lenBuf,
	jbyteArray errorCode,
	jlongArray buffers)
{
	// Get the data structure portion of the error code
	jobject gDataBuf=(*env)->NewGlobalRef(env, dataBuf); 
	jobject gLenBuf=(*env)->NewGlobalRef(env, lenBuf);
	RETCODE	rc;
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	UCHAR*	pBuf = NULL;
	UCHAR*	pLen = NULL;
	SDWORD	lBuf = 0;
	jlong* pBuffers=(jlong*)(*env)->GetLongArrayElements(env, buffers, 0);
	

	
	if (gDataBuf) pBuf = (UCHAR*) (*env)->GetByteArrayElements(env, gDataBuf, 0);
	if (gLenBuf) pLen = (UCHAR*) (*env)->GetByteArrayElements(env, gLenBuf, 0);
	if (dataBuf) lBuf = (*env)->GetArrayLength(env, dataBuf);


	pBuffers[0]=(LONG_IA64)pBuf;
	pBuffers[1]=(LONG_IA64)gDataBuf;
	pBuffers[2]=(LONG_IA64)pLen;
	pBuffers[3]=(LONG_IA64)gLenBuf;


	(*env)->ReleaseLongArrayElements(env, buffers, pBuffers, 0);

	rc = SQLBindParameter (
		(HSTMT) hStmt,		// Statement handle
		(SWORD) ipar,		// Parameter number
		SQL_PARAM_INPUT_OUTPUT,	// Type of parameter
		SQL_C_TIME,		// The C data type
		SQL_TIME,		// The SQL data type
		(SQLULEN)0,		// Precision
		(SWORD) scale,		// Scale
		pBuf,			// Pointer to parameter data buffer
		0,			// Maximum length of parameter
		(SQLLEN*) pLen);	// Pointer to parameter's length

	// Set the error code
	
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
}

// bug 4412437
//----------------------------------------------------------------------------
// bindInOutParameterStr
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL  Java_sun_jdbc_odbc_JdbcOdbc_bindInOutParameterStr (
        JNIEnv *env, jobject callingObject,
        jlong hStmt,
        jint ipar,
        jint SQLtype,
	jint precision,
        jbyteArray dataBuf,
        jbyteArray lenBuf,
        jbyteArray errorCode,
        jlongArray buffers,
	jint strLenInd)
{
        // Get the data structure portion of the error code
        jobject gDataBuf=(*env)->NewGlobalRef(env, dataBuf);
        jobject gLenBuf=(*env)->NewGlobalRef(env, lenBuf);
        RETCODE rc;
        UCHAR*  errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
        UCHAR*  pBuf = NULL;
        UCHAR*  pLen = NULL;
        SQLLEN  lBuf = 0;
        jlong* pBuffers=(jlong*)(*env)->GetLongArrayElements(env, buffers, 0);
	SQLLEN indBuf = strLenInd;


        if (gDataBuf) pBuf = (UCHAR*) (*env)->GetByteArrayElements(env, gDataBuf, 0);
        if (dataBuf) lBuf = (*env)->GetArrayLength(env, dataBuf);
        if (gLenBuf)
	{ 
		pLen = (UCHAR*) (*env)->GetByteArrayElements(env, gLenBuf, 0);
		memcpy(pLen, &indBuf, sizeof(indBuf));
	}

        pBuffers[0]=(LONG_IA64)pBuf;
        pBuffers[1]=(LONG_IA64)gDataBuf;
        pBuffers[2]=(LONG_IA64)pLen;
        pBuffers[3]=(LONG_IA64)gLenBuf;
	
        (*env)->ReleaseLongArrayElements(env, buffers, pBuffers, 0);

// Bind the output string parameter

        rc = SQLBindParameter (
                (HSTMT) hStmt,          // Statement handle
                (SWORD) ipar,           // Parameter number
                SQL_PARAM_INPUT_OUTPUT, // Type of parameter
                SQL_C_CHAR,             // The C data type
                (SWORD) SQLtype,        // The SQL data type
                (SQLULEN) lBuf - 1,	// Precision
                0,          		// Scale
                pBuf,                   // Pointer to parameter data buffer
                (SQLLEN)lBuf,           // Maximum length of parameter
                (SQLLEN*) pLen);        // Pointer to parameter's length

        // Set the error code

        errCode[0] = (char) rc;
        (*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
}

//----------------------------------------------------------------------------
// bindInOutParameterNull
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_sun_jdbc_odbc_JdbcOdbc_bindInOutParameterNull
	(JNIEnv *env, jobject callingObject, jlong hStmt, jint ipar, 
	 jint SQLtype, jint jprec, jint jscale, jbyteArray lenBuf, 
	 jbyteArray errorCode, jlongArray buffers)
{
	// Get the data structure portion of the error code
	RETCODE	rc;
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	jobject gLenBuf = (*env)->NewGlobalRef(env, lenBuf);
	UCHAR*	pLen = NULL;
	SQLLEN	lenValue = SQL_NULL_DATA;
	//UDWORD	prec = 0;
	//SWORD  scale = 0;
	
	jlong* pBuffers=(jlong*)(*env)->GetLongArrayElements(env, buffers, 0);
	
	if (gLenBuf) pLen = (UCHAR*) (*env)->GetByteArrayElements(env, gLenBuf, 0);

	pBuffers[0]=(LONG_IA64)pLen;
	pBuffers[1]=(LONG_IA64)gLenBuf;
	
	(*env)->ReleaseLongArrayElements(env, buffers, pBuffers, 0);

	// Set the length parameter to indicate a null value
	
	if (pLen) memcpy (pLen, &lenValue, sizeof (lenValue));

	// If the SQL type is CHAR, set the precision.	This is a workaround
	// for Oracle prec = 1;
	// for DRDA and Dbase prec = 254;
	// for Sybase the precision = 255;	

//printf("The length buffer data is %d\n", *pLen);
//printf("value of SQL_NULL_DATA is %d\n", lenValue);
//printf("address of lenValue is %x\n", &lenValue);
	// Bind the parameter to NULL
	
	rc = SQLBindParameter (
		(HSTMT) hStmt,		// Statement handle
		(UWORD) ipar,		// Parameter number
		SQL_PARAM_INPUT_OUTPUT,	// Type of parameter
		(SWORD) SQLtype,	// The C data type
		(SWORD) SQLtype,	// The SQL data type
		(SQLULEN)jprec,		// Precision
		(SWORD)jscale,		// Scale
		NULL,			// Pointer to parameter's data
		(SQLLEN) 0,		// Maximum length of parameter buffer
		(SQLLEN*) pLen);	// Pointer to parameter's length
		//&lenValue);

	// Set the error code

	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);

}

//----------------------------------------------------------------------------
// bindInOutParameterString
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_sun_jdbc_odbc_JdbcOdbc_bindInOutParameterString
	(JNIEnv *env, jobject callingObject, jlong hStmt, jint ipar, 
	 jint SQLtype, jint cbColDef, jint ibScale, 
	 jbyteArray dataBuf, jbyteArray lenBuf, jbyteArray errorCode, jlongArray buffers)
{
	// Get the data structure portion of the error code
	RETCODE	rc;
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	jobject gDataBuf = (*env)->NewGlobalRef(env, dataBuf);
	jobject gLenBuf=(*env)->NewGlobalRef(env, lenBuf);
	UCHAR*	pBuf = NULL;
	UCHAR*	pLen = NULL;
	SQLLEN	lBuf = 0;
	int i=0;
	
	jlong* pBuffers = (jlong*)(*env)->GetLongArrayElements(env, buffers, 0);	
	
	// Make a copy of the data coming in into the permanent data buffer.  This
	// data is in native format.  If no buffer is provided, the data buffer
	// will be NULL.

	if (dataBuf != NULL) {
		pBuf = (UCHAR*) (*env)->GetByteArrayElements(env, gDataBuf, 0);
		lBuf = (*env)->GetArrayLength(env, dataBuf);		
		pBuffers[0]=(LONG_IA64)pBuf;
		pBuffers[1]=(LONG_IA64)gDataBuf;
	}	
	/****
	// Sanity check the length
	if (lBuf > (SDWORD) strlen (*pBuf))
	{
			lBuf = strlen (*pBuf);
			printf("lBuf is changed to %d\n", lBuf);
	}
	*****/
	if (gLenBuf) 
	{
		pLen = (UCHAR*) (*env)->GetByteArrayElements(env, gLenBuf, 0);
		pBuffers[2]=(LONG_IA64)pLen;
		pBuffers[3]=(LONG_IA64)gLenBuf;
	}
			
	(*env)->ReleaseLongArrayElements(env, buffers, pBuffers, 0);
	/****
	gPBuf = pBuf;
	gLBuf = lBuf;
	for (i=0; gPBuf != NULL && i<gLBuf ; i++)
		printf("%c", gPBuf[i]);
	printf("\n");
	*****/
	// Bind the string parameter

	rc = SQLBindParameter (
		(HSTMT) hStmt,		// Statement handle
		(UWORD) ipar,		// Parameter number
		SQL_PARAM_INPUT_OUTPUT,	// Type of parameter
		SQL_C_CHAR,		// The C data type
		//SQL_C_BINARY,
		(SWORD) SQLtype,	// The SQL data type
		(SQLULEN) cbColDef,	// Precision		
		(SWORD) ibScale,	// Scale
		pBuf,			// Pointer to parameter's data
		lBuf,  			// Maximum length of parameter buffer
		NULL);			// Pointer to parameter's length
		//(SDWORD*) pLen); 	// Pointer to parameter's length

	// Set the error code
	
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
}

//----------------------------------------------------------------------------
// bindInOutParameterBinary
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_sun_jdbc_odbc_JdbcOdbc_bindInOutParameterBinary
	(JNIEnv *env, jobject callingObject, jlong hStmt, jint ipar, 
	 jint SQLtype, jint cbColDef, jint ibScale, 
	 jbyteArray dataBuf, jbyteArray lenBuf, jbyteArray errorCode, jlongArray buffers)
{
	// Get the data structure portion of the error code
	RETCODE	rc;
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	jobject gDataBuf = (*env)->NewGlobalRef(env, dataBuf);
	jobject gLenBuf=(*env)->NewGlobalRef(env, lenBuf);
	UCHAR*	pBuf = NULL;
	UCHAR*	pLen = NULL;
	SQLLEN	lBuf = 0;
	int i=0;
	
	jlong* pBuffers = (jlong*)(*env)->GetLongArrayElements(env, buffers, 0);
	
	// Make a copy of the data coming in into the permanent data buffer.  This
	// data is in native format.  If no buffer is provided, the data buffer
	// will be NULL.

	if (dataBuf != NULL) {
		pBuf = (UCHAR*) (*env)->GetByteArrayElements(env, gDataBuf, 0);
		lBuf = (*env)->GetArrayLength(env, dataBuf);		
		pBuffers[0]=(LONG_IA64)pBuf;
		pBuffers[1]=(LONG_IA64)gDataBuf;
	}
	/****
	// Sanity check the length
	if (lBuf > (SDWORD) strlen (*pBuf))
	{
			lBuf = strlen (*pBuf);
			printf("lBuf is changed to %d\n", lBuf);
	}
	*****/
	if (gLenBuf) 
	{
		pLen = (UCHAR*) (*env)->GetByteArrayElements(env, gLenBuf, 0);
		pBuffers[2]=(LONG_IA64)pLen;
		pBuffers[3]=(LONG_IA64)gLenBuf;
	}
			
	(*env)->ReleaseLongArrayElements(env, buffers, pBuffers, 0);
	/***
	gPBuf = pBuf;
	gLBuf = lBuf;
	for (i=0; gPBuf != NULL && i<gLBuf ; i++)
		printf("%c", gPBuf[i]);
	printf("\n");
	*****/
	// Bind the string parameter

	rc = SQLBindParameter (
		(HSTMT) hStmt,		// Statement handle
		(UWORD) ipar,		// Parameter number
		SQL_PARAM_INPUT_OUTPUT,	// Type of parameter
		//SQL_C_CHAR,		// The C data type
		SQL_C_BINARY,
		(SWORD) SQLtype,	// The SQL data type
		(SQLULEN) cbColDef,	// Precision
		(SWORD) ibScale,	// Scale
		pBuf,			// Pointer to parameter's data
		lBuf,  			// Maximum length of parameter buffer
		//NULL);		// Pointer to parameter's length
		(SQLLEN*) pLen); 	// Pointer to parameter's length

	// Set the error code
	
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
}

// bug 4412437
//----------------------------------------------------------------------------
// bindInOutParameterBin
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_sun_jdbc_odbc_JdbcOdbc_bindInOutParameterBin
        (JNIEnv *env, jobject callingObject, jlong hStmt, jint ipar,
         jint SQLtype, jint cbColDef,
         jbyteArray dataBuf, jbyteArray lenBuf, jbyteArray errorCode, jlongArray buffers, jint strLenInd)
{
        // Get the data structure portion of the error code
        RETCODE rc;
        UCHAR*  errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
        jobject gDataBuf = (*env)->NewGlobalRef(env, dataBuf);
        jobject gLenBuf=(*env)->NewGlobalRef(env, lenBuf);
        UCHAR*  pBuf = NULL;
        UCHAR*  pLen = NULL;
        SQLLEN  lBuf = 0;
	SQLLEN indBuf = strLenInd;

        jlong* pBuffers = (jlong*)(*env)->GetLongArrayElements(env, buffers, 0);

        // Make a copy of the data coming in into the permanent data buffer.  This
        // data is in native format.  If no buffer is provided, the data buffer
        // will be NULL.

        if (dataBuf != NULL) {
                pBuf = (UCHAR*) (*env)->GetByteArrayElements(env, gDataBuf, 0);
                lBuf = (*env)->GetArrayLength(env, dataBuf);
                pBuffers[0]=(LONG_IA64)pBuf;
                pBuffers[1]=(LONG_IA64)gDataBuf;
        }
        
	if (gLenBuf)
        {
                pLen = (UCHAR*) (*env)->GetByteArrayElements(env, gLenBuf, 0);
		memcpy(pLen, &indBuf, sizeof(indBuf));
                pBuffers[2]=(LONG_IA64)pLen;
                pBuffers[3]=(LONG_IA64)gLenBuf;
        }

        (*env)->ReleaseLongArrayElements(env, buffers, pBuffers, 0);

        rc = SQLBindParameter (
                (HSTMT) hStmt,          // Statement handle
                (UWORD) ipar,           // Parameter number
                SQL_PARAM_INPUT_OUTPUT, // Type of parameter
                SQL_C_BINARY,           // C Type
                (SWORD) SQLtype,        // The SQL data type
                (SQLULEN) cbColDef,     // Precision
                0,                      // Scale
                pBuf,                   // Pointer to parameter's data
                lBuf,                   // Maximum length of parameter buffer
                (SQLLEN*) pLen); 	// Pointer to parameter's length

        // Set the error code

        errCode[0] = (char) rc;
        (*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
}



// bug 4412437
//----------------------------------------------------------------------------
// bindInOutParameterFixed
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_sun_jdbc_odbc_JdbcOdbc_bindInOutParameterFixed
	(JNIEnv *env, jobject callingObject, jlong hStmt, jint ipar, jint CType,
	 jint SQLtype, jint maxLen, jbyteArray dataBuf, jbyteArray lenBuf, jbyteArray errorCode, 
	 jlongArray buffers)
{

	// Get the data structure portion of the error code
	RETCODE	rc;
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	UCHAR*	pBuf = NULL;
	UCHAR*  pLen = NULL;
	jobject gDataBuf = (*env)->NewGlobalRef(env, dataBuf);
	jobject gLenBuf=(*env)->NewGlobalRef(env, lenBuf);
	SQLLEN	lBuf = 0;

	jlong* pBuffers=(jlong*)(*env)->GetLongArrayElements(env, buffers, 0);

	// The data buffer is assumed to contain the value
	
	if (dataBuf != NULL)
	{
		pBuf = (UCHAR*) (*env)->GetByteArrayElements(env, gDataBuf, 0);
		pBuffers[0]=(LONG_IA64)pBuf;
		pBuffers[1]=(LONG_IA64)gDataBuf;
	}
	
        if (gLenBuf != NULL)
        {
                pLen = (UCHAR*) (*env)->GetByteArrayElements(env, gLenBuf, 0);
                pBuffers[2]=(LONG_IA64)pLen;
                pBuffers[3]=(LONG_IA64)gLenBuf;
        }
	
	
	(*env)->ReleaseLongArrayElements(env, buffers, pBuffers, 0);

	// Bind the parameter


	rc = SQLBindParameter (
		(HSTMT) hStmt,		// Statement handle
		(UWORD) ipar,		// Parameter number
		SQL_PARAM_INPUT_OUTPUT,	// Type of parameter
		(SWORD) CType,		// The C data type
		(SWORD) SQLtype,	// The SQL data type
		(SQLULEN) maxLen,	// Precision
		0,			// Scale
		pBuf,			// Pointer to parameter's data
		lBuf,			// Maximum length of parameter buffer
		(SQLLEN*)pLen);		// Pointer to parameter's length

	// Set the error code
	
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
}

//4532162
//----------------------------------------------------------------------------
// bindInOutParameterTimestamp
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL	Java_sun_jdbc_odbc_JdbcOdbc_bindInOutParameterTimestamp (
	JNIEnv *env, jobject callingObject,
	jlong hStmt,
	jint ipar,
	jint precision,
	jint scale,
	jbyteArray dataBuf,
	jbyteArray lenBuf,
	jbyteArray errorCode,
	jlongArray buffers)
{
	// Get the data structure portion of the error code
	jobject gDataBuf=(*env)->NewGlobalRef(env, dataBuf); 
	jobject gLenBuf=(*env)->NewGlobalRef(env, lenBuf);
	RETCODE	rc;
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	UCHAR*	pBuf = NULL;
	UCHAR*	pLen = NULL;
	SQLLEN	lBuf = 0;
	jlong* pBuffers=(jlong*)(*env)->GetLongArrayElements(env, buffers, 0);
	

	
	if (gDataBuf) pBuf = (UCHAR*) (*env)->GetByteArrayElements(env, gDataBuf, 0);
	if (gLenBuf) pLen = (UCHAR*) (*env)->GetByteArrayElements(env, gLenBuf, 0);
	if (dataBuf) lBuf = (*env)->GetArrayLength(env, dataBuf);


	pBuffers[0]=(LONG_IA64)pBuf;
	pBuffers[1]=(LONG_IA64)gDataBuf;
	pBuffers[2]=(LONG_IA64)pLen;
	pBuffers[3]=(LONG_IA64)gLenBuf;

	(*env)->ReleaseLongArrayElements(env, buffers, pBuffers, 0);

	rc = SQLBindParameter (
		(HSTMT) hStmt,		// Statement handle
		(SWORD) ipar,		// Parameter number
		SQL_PARAM_INPUT_OUTPUT,	// Type of parameter
		SQL_C_TIMESTAMP,	// The C data type
		SQL_TIMESTAMP,		// The SQL data type
		(SQLULEN) precision,	// Precision
		scale,			// Scale
		pBuf,			// Pointer to parameter data buffer
		lBuf,			// Maximum length of parameter
		(SQLLEN *) pLen);	// Pointer to parameter's length

	// Set the error code
	
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
	
}  

//----------------------------------------------------------------------------
// bindInOutParameter
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_sun_jdbc_odbc_JdbcOdbc_bindInOutParameter
	(JNIEnv *env, jobject callingObject, jlong hStmt, jint ipar, 
	 jint SQLtype, jint cbColDef, jint ibScale, 
	 jdouble value, jbyteArray dataBuf, jbyteArray errorCode, jlongArray buffers)
{
	// Get the data structure portion of the error code
	RETCODE	rc;
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	jobject gDataBuf = (*env)->NewGlobalRef(env, dataBuf);
	//UCHAR*	pBuf = NULL;
	//SDWORD	lBuf = 0;
	//double *pBuf ;
	jlong* pBuffers = (jlong*)(*env)->GetLongArrayElements(env, buffers, 0);	
	
	// Make a copy of the data coming in into the permanent data buffer.  This
	// data is in native format.  If no buffer is provided, the data buffer
	// will be NULL.

	if (dataBuf != NULL) {
		//pBuf = (double*) (*env)->GetByteArrayElements(env, gDataBuf, 0);
		//lBuf = (*env)->GetArrayLength(env, dataBuf);
		pBuffers[0]=(LONG_IA64)&buf;
		pBuffers[1]=(LONG_IA64)gDataBuf;			
	}
	
	//printf("the address stored was %d\n", pBuffers[0]);
	(*env)->ReleaseLongArrayElements(env, buffers, pBuffers, 0);	
	buf = value;
	// Bind the string parameter
	//printf("Before binding pBuf is %e\n", buf);
	//printf("Before binding value is %e\n", value);

	rc = SQLBindParameter (
		(HSTMT) hStmt,		// Statement handle
		(UWORD) ipar,		// Parameter number
		SQL_PARAM_INPUT_OUTPUT,	// Type of parameter
		//SQL_C_CHAR,		// The C data type
		(SWORD) SQLtype,
		(SWORD) SQLtype,	// The SQL data type
		(SQLULEN) cbColDef,	// Precision
		(SWORD) ibScale,	// Scale
		&buf,			// Pointer to parameter's data		
		//lBuf,			// Maximum length of parameter buffer
		(SQLLEN)sizeof(value),
		NULL);			// Pointer to parameter's length

	// Set the error code
	
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
}

//4532162
//----------------------------------------------------------------------------
// bindOutParameterNull
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_sun_jdbc_odbc_JdbcOdbc_bindOutParameterNull
	(JNIEnv *env, jobject callingObject, jlong hStmt, jint ipar, 
	 jint SQLtype, jint jprec, jint jscale, jbyteArray lenBuf, 
	 jbyteArray errorCode, jlongArray buffers)
{
	// Get the data structure portion of the error code
	RETCODE	rc;
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	jobject gLenBuf = (*env)->NewGlobalRef(env, lenBuf);
	UCHAR*	pLen = NULL;
	SQLLEN	lenValue = SQL_NULL_DATA;
	//UDWORD	prec = 0;
	//SWORD  scale = 0;
	
	jlong* pBuffers=(jlong*)(*env)->GetLongArrayElements(env, buffers, 0);
	
	if (gLenBuf) pLen = (UCHAR*) (*env)->GetByteArrayElements(env, gLenBuf, 0);

	pBuffers[0]=(LONG_IA64)pLen;
	pBuffers[1]=(LONG_IA64)gLenBuf;

	(*env)->ReleaseLongArrayElements(env, buffers, pBuffers, 0);

	// Set the length parameter to indicate a null value
	
	if (pLen) memcpy (pLen, &lenValue, sizeof (lenValue));

	// Bind the parameter to NULL
	
	rc = SQLBindParameter (
		(HSTMT) hStmt,		// Statement handle
		(UWORD) ipar,		// Parameter number
		SQL_PARAM_OUTPUT,	// Type of parameter
		(SWORD) SQLtype,	// The C data type
		(SWORD) SQLtype,	// The SQL data type
		(SQLULEN)jprec,		// Precision
		(SWORD)jscale,		// Scale
		NULL,			// Pointer to parameter's data
		(SQLLEN) 0,			// Maximum length of parameter buffer
		(SQLLEN*) pLen);	// Pointer to parameter's length
		//&lenValue);

	// Set the error code
	
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);

}

//4532162
//----------------------------------------------------------------------------
// bindOutParameterFixed
//---------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_sun_jdbc_odbc_JdbcOdbc_bindOutParameterFixed
	(JNIEnv *env, jobject callingObject, jlong hStmt, jint ipar, jint CType,
	 jint SQLtype, jint maxLen, jbyteArray dataBuf, jbyteArray lenBuf, jbyteArray errorCode, 
	 jlongArray buffers)
{

	// Get the data structure portion of the error code
	jobject gDataBuf=(*env)->NewGlobalRef(env, dataBuf); 
	jobject gLenBuf=(*env)->NewGlobalRef(env, lenBuf);
	RETCODE	rc;
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	UCHAR*	pBuf = NULL;
	UCHAR*	pLen = NULL;
	SQLLEN	lBuf = 0;
	jlong* pBuffers=(jlong*)(*env)->GetLongArrayElements(env, buffers, 0);

	
	if (gDataBuf) pBuf = (UCHAR*) (*env)->GetByteArrayElements(env, gDataBuf, 0);
	if (gLenBuf) pLen = (UCHAR*) (*env)->GetByteArrayElements(env, gLenBuf, 0);
	if (dataBuf) lBuf = (*env)->GetArrayLength(env, dataBuf);

	pBuffers[0]=(LONG_IA64)pBuf;
	pBuffers[1]=(LONG_IA64)gDataBuf;
	pBuffers[2]=(LONG_IA64)pLen;
	pBuffers[3]=(LONG_IA64)gLenBuf;


	(*env)->ReleaseLongArrayElements(env, buffers, pBuffers, 0);
	
	// Bind the output string parameter

	rc = SQLBindParameter (
		(HSTMT) hStmt,		// Statement handle
		(SWORD) ipar,		// Parameter number
		SQL_PARAM_OUTPUT,	// Type of parameter
		CType,			// The C data type
		SQLtype,		// The SQL data type
		(SQLULEN)maxLen,	// Precision
		0,			// Scale
		pBuf,			// Pointer to parameter data buffer
		lBuf,			// Maximum length of parameter
		(SQLLEN *)pLen);	// Pointer to parameter's length

	// Set the error code
	
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
}

//4532162
//----------------------------------------------------------------------------
// bindOutParameterBinary
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_sun_jdbc_odbc_JdbcOdbc_bindOutParameterBinary
	(JNIEnv *env, jobject callingObject, jlong hStmt, jint ipar, 
	 jint SQLtype, jint cbColDef, jint ibScale, 
	 jbyteArray dataBuf, jbyteArray lenBuf, jbyteArray errorCode, jlongArray buffers)
{
	// Get the data structure portion of the error code
	RETCODE	rc;
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	jobject gDataBuf = (*env)->NewGlobalRef(env, dataBuf);
	jobject gLenBuf=(*env)->NewGlobalRef(env, lenBuf);
	UCHAR*	pBuf = NULL;
	UCHAR*	pLen = NULL;
	SQLLEN	lBuf = 0;
	int i=0;
	
	jlong* pBuffers = (jlong*)(*env)->GetLongArrayElements(env, buffers, 0);
	
	// Make a copy of the data coming in into the permanent data buffer.  This
	// data is in native format.  If no buffer is provided, the data buffer
	// will be NULL.

	if (dataBuf != NULL) {
		pBuf = (UCHAR*) (*env)->GetByteArrayElements(env, gDataBuf, 0);
		lBuf = (*env)->GetArrayLength(env, dataBuf);		
		pBuffers[0]=(LONG_IA64)pBuf;
		pBuffers[1]=(LONG_IA64)gDataBuf;
	}
	
	if (gLenBuf){
		pLen = (UCHAR*) (*env)->GetByteArrayElements(env, gLenBuf, 0);
		pBuffers[2]=(LONG_IA64)pLen;
		pBuffers[3]=(LONG_IA64)gLenBuf;
	}
		
	(*env)->ReleaseLongArrayElements(env, buffers, pBuffers, 0);
	
	// Bind the string parameter

	rc = SQLBindParameter (
		(HSTMT) hStmt,		// Statement handle
		(UWORD) ipar,		// Parameter number
		SQL_PARAM_OUTPUT,	// Type of parameter
		SQL_C_BINARY,		// The C data type
		(SWORD) SQLtype,	// The SQL data type
		(SQLULEN) cbColDef,	// Precision
		(SWORD) ibScale,	// Scale
		pBuf,			// Pointer to parameter's data
		lBuf,  			// Maximum length of parameter buffer
		//NULL);		// Pointer to parameter's length
		(SQLLEN*) pLen); 	// Pointer to parameter's length

	// Set the error code
	
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
}

//4532162
//----------------------------------------------------------------------------
// bindOutParameterDate
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL	Java_sun_jdbc_odbc_JdbcOdbc_bindOutParameterDate (
	JNIEnv *env, jobject callingObject,
	jlong hStmt,
	jint ipar,
	jint scale,
	jbyteArray dataBuf,
	jbyteArray lenBuf,
	jbyteArray errorCode,
	jlongArray buffers)
{
	// Get the data structure portion of the error code
	jobject gDataBuf=(*env)->NewGlobalRef(env, dataBuf); 
	jobject gLenBuf=(*env)->NewGlobalRef(env, lenBuf);
	RETCODE	rc;
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	UCHAR*	pBuf = NULL;
	UCHAR*	pLen = NULL;
	SQLLEN	lBuf = 0;
	jlong* pBuffers=(jlong*)(*env)->GetLongArrayElements(env, buffers, 0);
	

	
	if (gDataBuf) pBuf = (UCHAR*) (*env)->GetByteArrayElements(env, gDataBuf, 0);
	if (gLenBuf) pLen = (UCHAR*) (*env)->GetByteArrayElements(env, gLenBuf, 0);
	if (dataBuf) lBuf = (*env)->GetArrayLength(env, dataBuf);


	pBuffers[0]=(LONG_IA64)pBuf;
	pBuffers[1]=(LONG_IA64)gDataBuf;
	pBuffers[2]=(LONG_IA64)pLen;
	pBuffers[3]=(LONG_IA64)gLenBuf;


	(*env)->ReleaseLongArrayElements(env, buffers, pBuffers, 0);

	rc = SQLBindParameter (
		(HSTMT) hStmt,		// Statement handle
		(SWORD) ipar,		// Parameter number
		SQL_PARAM_OUTPUT,	// Type of parameter
		SQL_C_DATE,		// The C data type
		SQL_DATE,		// The SQL data type
		(SQLULEN) 0,		// Precision
		(SWORD) scale,		// Scale
		pBuf,			// Pointer to parameter data buffer
 		0,			// 	
		(SQLLEN*) pLen);	// Pointer to parameter's length

	// Set the error code
	
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
}

//4532162
//----------------------------------------------------------------------------
// bindOutParameterTime
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL	Java_sun_jdbc_odbc_JdbcOdbc_bindOutParameterTime (
	JNIEnv *env, jobject callingObject,
	jlong hStmt,
	jint ipar,
	jint scale,
	jbyteArray dataBuf,
	jbyteArray lenBuf,
	jbyteArray errorCode,
	jlongArray buffers)
{
	// Get the data structure portion of the error code
	jobject gDataBuf=(*env)->NewGlobalRef(env, dataBuf); 
	jobject gLenBuf=(*env)->NewGlobalRef(env, lenBuf);
	RETCODE	rc;
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	UCHAR*	pBuf = NULL;
	UCHAR*	pLen = NULL;
	SQLLEN	lBuf = 0;
	jlong* pBuffers=(jlong*)(*env)->GetLongArrayElements(env, buffers, 0);
	

	
	if (gDataBuf) pBuf = (UCHAR*) (*env)->GetByteArrayElements(env, gDataBuf, 0);
	if (gLenBuf) pLen = (UCHAR*) (*env)->GetByteArrayElements(env, gLenBuf, 0);
	if (dataBuf) lBuf = (*env)->GetArrayLength(env, dataBuf);


	pBuffers[0]=(LONG_IA64)pBuf;
	pBuffers[1]=(LONG_IA64)gDataBuf;
	pBuffers[2]=(LONG_IA64)pLen;
	pBuffers[3]=(LONG_IA64)gLenBuf;


	(*env)->ReleaseLongArrayElements(env, buffers, pBuffers, 0);

	rc = SQLBindParameter (
		(HSTMT) hStmt,		// Statement handle
		(SWORD) ipar,		// Parameter number
		SQL_PARAM_OUTPUT,	// Type of parameter
		SQL_C_TIME,		// The C data type
		SQL_TIME,		// The SQL data type
		(SQLULEN) 0,		// Precision
		(SWORD) scale,		// Scale
		pBuf,			// Pointer to parameter data buffer
		0,			// Maximum length of parameter
		(SQLLEN*) pLen);	// Pointer to parameter's length

	// Set the error code
	
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
}

//4532162
//----------------------------------------------------------------------------
// bindOutParameterTimestamp
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL	Java_sun_jdbc_odbc_JdbcOdbc_bindOutParameterTimestamp (
	JNIEnv *env, jobject callingObject,
	jlong hStmt,
	jint ipar,
	jint precision,
	jbyteArray dataBuf,
	jbyteArray lenBuf,
	jbyteArray errorCode,
	jlongArray buffers)
{
	// Get the data structure portion of the error code
	jobject gDataBuf=(*env)->NewGlobalRef(env, dataBuf); 
	jobject gLenBuf=(*env)->NewGlobalRef(env, lenBuf);
	RETCODE	rc;
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	UCHAR*	pBuf = NULL;
	UCHAR*	pLen = NULL;
	SQLLEN	lBuf = 0;
	jlong* pBuffers=(jlong*)(*env)->GetLongArrayElements(env, buffers, 0);
	
	if (gDataBuf) pBuf = (UCHAR*) (*env)->GetByteArrayElements(env, gDataBuf, 0);
	if (gLenBuf) pLen = (UCHAR*) (*env)->GetByteArrayElements(env, gLenBuf, 0);
	if (dataBuf) lBuf = (*env)->GetArrayLength(env, dataBuf);


	pBuffers[0]=(LONG_IA64)pBuf;
	pBuffers[1]=(LONG_IA64)gDataBuf;
	pBuffers[2]=(LONG_IA64)pLen;
	pBuffers[3]=(LONG_IA64)gLenBuf;

	(*env)->ReleaseLongArrayElements(env, buffers, pBuffers, 0);

	rc = SQLBindParameter (
		(HSTMT) hStmt,		// Statement handle
		(SWORD) ipar,		// Parameter number
		SQL_PARAM_OUTPUT,	// Type of parameter
		SQL_C_TIMESTAMP,	// The C data type
		SQL_TIMESTAMP,		// The SQL data type
		(SQLULEN) 29,		// Precision
		9,			// Scale
		pBuf,			// Pointer to parameter data buffer
		lBuf,			// Maximum length of parameter
		(SQLLEN*) pLen);	// Pointer to parameter's length

	// Set the error code
	
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
	
}  

//----------------------------------------------------------------------------
// browseConnect
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL	Java_sun_jdbc_odbc_JdbcOdbc_browseConnect (
	JNIEnv *env, jobject callingObject,
	jlong hDbc,
	jbyteArray connStrIn,
	jbyteArray connStrOut,
	jbyteArray errorCode)
{
	// Get the data structure portion of the error code
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	UCHAR*	szConnStrOut = NULL;
	SWORD	cbConnStrOutMax = 0;
	UCHAR*	pConnectString = NULL;
	
	RETCODE	rc;
	SWORD	cbConnStrOut = 0;

	if (connStrOut) szConnStrOut = (UCHAR*) (*env)->GetByteArrayElements(env, connStrOut, 0);
	if (connStrOut) cbConnStrOutMax = (SWORD) (*env)->GetArrayLength(env, connStrOut);
	if (connStrIn)  pConnectString = (UCHAR*) (*env)->GetByteArrayElements(env, connStrIn, 0);

	rc = SQLBrowseConnect (
		(HDBC) hDbc,		// Connection handle
		pConnectString,		// Full connection string
		SQL_NTS,		// Length of connection string
		szConnStrOut,		// Pointer to completed connection string
		cbConnStrOutMax,	// Maximum length of completed connection string
		&cbConnStrOut);		// Pointer to completed connection string length

	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
	(*env)->ReleaseByteArrayElements(env, connStrOut, szConnStrOut, 0);
	(*env)->ReleaseByteArrayElements(env, connStrIn, pConnectString, 0);
}

//----------------------------------------------------------------------------
// cancel
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_sun_jdbc_odbc_JdbcOdbc_cancel
	(JNIEnv *env, jobject callingObject, jlong hStmt, 
	 jbyteArray errorCode)

{
	// Get the data structure portion of the error code
	UCHAR*  errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	RETCODE	rc;

	// Cancel the statement
	
	rc = SQLCancel (
		(HSTMT) hStmt);		// Statement handle

	// Set the error code
	
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
}

//----------------------------------------------------------------------------
// colAttributes
//----------------------------------------------------------------------------

JNIEXPORT jint JNICALL Java_sun_jdbc_odbc_JdbcOdbc_colAttributes (
	JNIEnv *env, jobject callingObject,
	jlong hStmt,
	jint column,
	jint type,
	jbyteArray errorCode)
{
	// Get the data structure portion of the error code
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	SQLLEN value = 0;
	SWORD	lValue = 0;
	
	RETCODE	rc;
	// Get the column attribute
	
	rc = SQLColAttributes (
		(HSTMT) hStmt,		// Statement handle
		(UWORD) column,		// Column number
		(UWORD) type,		// Descriptor type
		NULL,			// Pointer to descriptor storage
		0,			// Maximum length of descriptor
		&lValue,		// Length of descriptor
		&value);		// Pointer to integer value for
					//  descriptor
	
// Set the error code
	
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
	return value;
}

//----------------------------------------------------------------------------
// colAttributesString
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL	Java_sun_jdbc_odbc_JdbcOdbc_colAttributesString (
	JNIEnv *env, jobject callingObject,
	jlong hStmt,
	jint column,
	jint type,
	jbyteArray rgbDesc,
	jbyteArray errorCode)
{
	// Get the data structure portion of the error code
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	UCHAR*	pDesc = NULL;
	SWORD	lDesc = 0;
	SWORD	lValue = 0;
	
	RETCODE	rc;
	
	if (rgbDesc) pDesc = (UCHAR*) (*env)->GetByteArrayElements(env, rgbDesc, 0);
	if (rgbDesc) lDesc = (SWORD) (*env)->GetArrayLength(env, rgbDesc);

	// Get the column attribute
	
	rc = SQLColAttributes (
		(HSTMT) hStmt,		// Statement handle
		(UWORD) column,		// Column number
		(UWORD) type,		// Descriptor type
		pDesc,			// Pointer to descriptor buffer
		lDesc,			// Maximum length of descriptor
		&lValue,		// Pointer to descriptor length
		NULL);			// Pointer to integer value for
					//  descriptor
	
// Set the error code
	
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
	(*env)->ReleaseByteArrayElements(env, rgbDesc, pDesc, 0);
}

//----------------------------------------------------------------------------
// columns
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_sun_jdbc_odbc_JdbcOdbc_columns (
	JNIEnv *env, jobject callingObject,
	jlong hStmt,
	jbyteArray catalog, jboolean catalogNull,
	jbyteArray schema, jboolean schemaNull,
	jbyteArray table, jboolean tableNull,	     
	jbyteArray column, jboolean columnNull,	     
	jbyteArray errorCode)
{
	// Get the data structure portion of the error code
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	
	RETCODE	rc;
	UCHAR*	pCatalog = NULL;
	UCHAR*	pSchema = NULL;
	UCHAR*	pTable = NULL;
	UCHAR*	pColumn = NULL;

	// Check if the parameter is null.  If it is, we need to pass
	// a NULL to ODBC.  Unfortuneatly, Java converts a null string
	// to an empty string.
	
	if (!catalogNull) {
		pCatalog = (UCHAR*) (*env)->GetByteArrayElements(env, catalog, 0);
	}
	
	if (!schemaNull) {
		pSchema = (UCHAR*) (*env)->GetByteArrayElements(env, schema, 0);
	}
	
	if (!tableNull) {
		pTable = (UCHAR*) (*env)->GetByteArrayElements(env, table, 0);
	}
	
	if (!columnNull) {
		pColumn = (UCHAR*) (*env)->GetByteArrayElements(env, column, 0);
	}

	rc = SQLColumns (
		(HSTMT) hStmt,		// Statement handle
		pCatalog,		// Table catalog (qualifier)
		SQL_NTS,		// Length of catalog
		pSchema,		// Table schema name (owner)
		SQL_NTS,		// Length of schema name
		pTable,			// Table name
		SQL_NTS,		// Length of table name
		pColumn,		// Search pattern for column names
		SQL_NTS);		// Length of search pattern

	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
	if (!catalogNull) 
		(*env)->ReleaseByteArrayElements(env, catalog, pCatalog, 0);
	if (!schemaNull) 
		(*env)->ReleaseByteArrayElements(env, schema, pSchema, 0);
	if (!tableNull) 
		(*env)->ReleaseByteArrayElements(env, table, pTable, 0);
	if (!columnNull) 
		(*env)->ReleaseByteArrayElements(env, column, pColumn, 0);
}

//----------------------------------------------------------------------------
// columnPrivileges
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_sun_jdbc_odbc_JdbcOdbc_columnPrivileges (
	JNIEnv *env, jobject callingObject,
	jlong hStmt,
	jbyteArray catalog, jboolean catalogNull,
	jbyteArray schema, jboolean schemaNull,
	jbyteArray table, jboolean tableNull,	     
	jbyteArray column, jboolean columnNull,	     
	jbyteArray errorCode)

{
	// Get the data structure portion of the error code
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	
	RETCODE	rc;
	UCHAR*	pCatalog = NULL;
	UCHAR*	pSchema = NULL;
	UCHAR*	pTable = NULL;
	UCHAR*	pColumn = NULL;

	// Check if the parameter is null.  If it is, we need to pass
	// a NULL to ODBC.  Unfortuneatly, Java converts a null string
	// to an empty string.
	
	if (!catalogNull) {
		pCatalog = (UCHAR*) (*env)->GetByteArrayElements(env, catalog, 0);
	}
	
	if (!schemaNull) {
		pSchema = (UCHAR*) (*env)->GetByteArrayElements(env, schema, 0);
	}
	
	if (!tableNull) {
		pTable = (UCHAR*) (*env)->GetByteArrayElements(env, table, 0);
	}
	
	if (!columnNull) {
		pColumn = (UCHAR*) (*env)->GetByteArrayElements(env, column, 0);
	}


	rc = SQLColumnPrivileges (
		(HSTMT) hStmt,		// Statement handle
		pCatalog,		// Table catalog (qualifier)
		SQL_NTS,		// Length of table catalog
		pSchema,		// Table schema name (owner)
		SQL_NTS,		// Length of schema name
		pTable,			// Table name
		SQL_NTS,		// Length of table name
		pColumn,		// Search pattern for column names
		SQL_NTS);		// Length of search pattern

	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
	if (!catalogNull) 
		(*env)->ReleaseByteArrayElements (env, catalog, pCatalog, 0);
	if (!schemaNull) 
		(*env)->ReleaseByteArrayElements (env, schema, pSchema, 0);
	if (!tableNull) 
		(*env)->ReleaseByteArrayElements (env, table, pTable, 0);
	if (!columnNull) 
		(*env)->ReleaseByteArrayElements (env, column, pColumn, 0);

}

//----------------------------------------------------------------------------
// describeParam
//----------------------------------------------------------------------------

JNIEXPORT jint JNICALL Java_sun_jdbc_odbc_JdbcOdbc_describeParam (
	JNIEnv *env, jobject callingObject,
	jlong hStmt,
	jint param,
	jint returnParam,
	jbyteArray errorCode)
{
	// Get the data structure portion of the error code
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	
	RETCODE	rc;
	SWORD	sqlType = 0;
	SQLULEN	precision = 0;
	SWORD	scale = 0;
	SWORD	nullable = 0;
	DWORD	returnValue = 0;
	
// SQLDescribeParam
	
	rc = SQLDescribeParam (
		(HSTMT) hStmt,		// Statement handle
		(UWORD) param,		// Parameter marker number
		&sqlType,		// The SQL data type
		&precision,		// Precision
		&scale,			// Scale
		&nullable);		// Parameter allow NULL values?

	// Set the error code
	
	errCode[0] = (char) rc;

	// Determine which piece of information to return
	if (returnParam == 1) {
		returnValue = sqlType;
	}
	else if (returnParam == 2) {
		returnValue = precision;
	}
	else if (returnParam == 3) {
		returnValue = scale;
	}
	else if (returnParam == 4) {
		returnValue = nullable;
	}
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
	return (jint)returnValue;
}

//----------------------------------------------------------------------------
// disconnect
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL	Java_sun_jdbc_odbc_JdbcOdbc_disconnect(
	JNIEnv *env, jobject callingObject,
	jlong hDbc,
	jbyteArray errorCode)
{
	// Get the data structure portion of the error code
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	RETCODE	rc;
	
	// Disconnect
	
	rc = SQLDisconnect (
		(HDBC) hDbc);		// Connection handle

	// Set the error code
	
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
}

//----------------------------------------------------------------------------
// driverConnect
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL	Java_sun_jdbc_odbc_JdbcOdbc_driverConnect(
	JNIEnv *env, jobject callingObject,
	jlong hDbc,
	jbyteArray connectString,
	jbyteArray errorCode)
{
	// Get the data structure portion of the error code
	UCHAR*	errCode = (UCHAR*)(*env)->GetByteArrayElements(env, errorCode, 0);
	
	RETCODE	rc;
	UCHAR*	pConnectString = (UCHAR*)(*env)->GetByteArrayElements(env, connectString, 0);
	UCHAR	szConnStrOut[256];
	SWORD	cbConnStrOut = 0;

	rc = SQLDriverConnect (
		(HDBC) hDbc,			// Connection handle
		0,				// Window handle
		pConnectString,			// Full connection string
		SQL_NTS,			// Length of connection string
		szConnStrOut,			// Pointer to completed connection string
		sizeof (szConnStrOut),		// Maximum length of completed connection string
		&cbConnStrOut,			// Pointer to completed connection string length
		SQL_DRIVER_NOPROMPT);		// Prompt flag

	errCode[0] = (char) rc;

	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
	(*env)->ReleaseByteArrayElements(env,  connectString, pConnectString, 0);
}

//----------------------------------------------------------------------------
// error
//----------------------------------------------------------------------------

JNIEXPORT jint JNICALL Java_sun_jdbc_odbc_JdbcOdbc_error(
	JNIEnv *env, jobject callingObject,
	jlong hEnv,
	jlong hDbc,
	jlong hStmt,
	jbyteArray sqlState,
	jbyteArray errorMsg,
	jbyteArray errorCode)
{
	// Get the data structure portion of the error code
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	RETCODE	rc;
	UCHAR*	pSqlState = NULL;
	UCHAR*	pErrorMsg = NULL;
	SWORD	lErrorMsg = 0;
	SWORD	cbErrorMsg = 0;
	DWORD	nativeError = 0;
	
	if (sqlState) pSqlState = (UCHAR*) (*env)->GetByteArrayElements(env, sqlState, 0);
	if (errorMsg) pErrorMsg = (UCHAR*) (*env)->GetByteArrayElements(env, errorMsg, 0);
	if (errorMsg) lErrorMsg = (SWORD)  (*env)->GetArrayLength(env, errorMsg);

	rc = SQLError (
		(HENV) hEnv,		// Environment handle
		(HDBC) hDbc,		// Connection handle
		(HSTMT) hStmt,		// Statement handle
		pSqlState,		// Pointer to SQLSTATE
		&nativeError,		// Pointer to native error code
		pErrorMsg,		// Pointer to error message buffer
		lErrorMsg,		// Maximum length of error message
		&cbErrorMsg);		// Length of error message buffer

	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
	(*env)->ReleaseByteArrayElements(env, sqlState, pSqlState, 0);
	(*env)->ReleaseByteArrayElements(env, errorMsg, pErrorMsg, 0);
	return (jint)nativeError;
}

//----------------------------------------------------------------------------
// execDirect
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL	Java_sun_jdbc_odbc_JdbcOdbc_execDirect (
	JNIEnv *env, jobject callingObject,
	jlong hStmt,
	jbyteArray SQLString,
	jbyteArray errorCode)
{
	// Get the data structure portion of the error code
	UCHAR*	errCode = (UCHAR*)(*env)->GetByteArrayElements(env, errorCode, 0);
	RETCODE	rc;
	UCHAR*	pSQL = NULL;

	if (SQLString) pSQL = (UCHAR*)(*env)->GetByteArrayElements(env, SQLString, 0);

	rc = SQLExecDirect (
		(HSTMT) hStmt,		// Statement handle
		pSQL,			// SQL Statement
		SQL_NTS);		// Length of SQL statement

	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
	(*env)->ReleaseByteArrayElements(env,  SQLString, pSQL, 0);
}

//----------------------------------------------------------------------------
// execute
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL	Java_sun_jdbc_odbc_JdbcOdbc_execute (
	JNIEnv *env, jobject callingObject,
	jlong hStmt,
	jbyteArray errorCode)
{
	// Get the data structure portion of the error code
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	
	RETCODE	rc;
	//LONG_IA64 i=0;
	
	rc = SQLExecute (
		(HSTMT) hStmt);		// Statement handle

	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
	/****
	for (i=0; gPBuf != NULL && i<gLBuf ; i++)
		printf("%c", gPBuf[i]);
	printf("\n");
	****/
	
}

//----------------------------------------------------------------------------
// fetch
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL	Java_sun_jdbc_odbc_JdbcOdbc_fetch (
	JNIEnv *env, jobject callingObject,
	jlong hStmt,
	jbyteArray errorCode)
{
	// Get the data structure portion of the error code
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);

	RETCODE	rc;

	// Fetch the next row from the statement
	rc = SQLFetch (
		(HSTMT) hStmt);		// Statement handle

	// Set the error code
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
}

//----------------------------------------------------------------------------
// fetchScroll
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL	Java_sun_jdbc_odbc_JdbcOdbc_fetchScroll (
	JNIEnv *env, jobject callingObject,
	jlong hStmt,
	jshort orientation,
	jint offset,
	jbyteArray errorCode)
{
	// Get the data structure portion of the error code
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);

	RETCODE	rc;

	// Fetch a row with scrollabe cursor
	
	rc = SQLFetchScroll (
		(SQLHSTMT) hStmt,
		(SQLSMALLINT) orientation,
		(SQLLEN) offset);

	// Set the error code
	
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
}

//----------------------------------------------------------------------------
// foreignKeys
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_sun_jdbc_odbc_JdbcOdbc_foreignKeys (
	JNIEnv *env, jobject callingObject,
	jlong hStmt,
	jbyteArray PKcatalog, jboolean PKcatalogNull,
	jbyteArray PKschema, jboolean PKschemaNull,
	jbyteArray PKtable, jboolean PKtableNull,
	jbyteArray FKcatalog, jboolean FKcatalogNull,
	jbyteArray FKschema, jboolean FKschemaNull,
	jbyteArray FKtable, jboolean FKtableNull,
	jbyteArray errorCode)
{
	// Get the data structure portion of the error code
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	
	RETCODE	rc;
	UCHAR*	pPKCatalog = NULL;
	UCHAR*	pPKSchema = NULL;
	UCHAR*	pPKTable = NULL;
	UCHAR*	pFKCatalog = NULL;
	UCHAR*	pFKSchema = NULL;
	UCHAR*	pFKTable = NULL;

	// Check if the parameter is null.  If it is, we need to pass
	// a NULL to ODBC.  Unfortuneatly, Java converts a null string
	// to an empty string.
	
	if (!PKcatalogNull) {
		pPKCatalog = (UCHAR*) (*env)->GetByteArrayElements(env, PKcatalog, 0);
	}
	
	if (!PKschemaNull) {
		pPKSchema = (UCHAR*) (*env)->GetByteArrayElements(env, PKschema,0);
	}
	
	if (!PKtableNull) {
		pPKTable = (UCHAR*) (*env)->GetByteArrayElements(env, PKtable, 0);
	}
	if (!FKcatalogNull) {
		pFKCatalog = (UCHAR*) (*env)->GetByteArrayElements(env, FKcatalog, 0);
	}
	
	if (!FKschemaNull) {
		pFKSchema = (UCHAR*) (*env)->GetByteArrayElements(env, FKschema, 0);
	}
	
	if (!FKtableNull) {
		pFKTable = (UCHAR*) (*env)->GetByteArrayElements(env, FKtable, 0);
	}
	
	rc = SQLForeignKeys (
		(HSTMT) hStmt,		// Statement handle
		pPKCatalog,		// Primary key catalog (qualifier)
		SQL_NTS,		// Length of primary key catalog
		pPKSchema,		// Primary key schema name (owner)
		SQL_NTS,		// Length of primary key schema
		pPKTable,		// Primary key table name
		SQL_NTS,		// Length of primary key table name
		pFKCatalog,		// Foreign key catalog (qualifier)
		SQL_NTS,		// Length of foreign key catalog
		pFKSchema,		// Foreign key schema name (owner)
		SQL_NTS,		// Length of foreign key schema
		pFKTable,		// Foreign key table name
		SQL_NTS);		// Length of foreign key table name

	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
	if (!PKcatalogNull) 
		(*env)->ReleaseByteArrayElements(env, PKcatalog, pPKCatalog, 0);
	if (!PKschemaNull) 
		(*env)->ReleaseByteArrayElements(env, PKschema, pPKSchema, 0);
	if (!PKtableNull) 
		(*env)->ReleaseByteArrayElements(env, PKtable, pPKTable, 0);
	if (!FKcatalogNull) 
		(*env)->ReleaseByteArrayElements(env, FKcatalog, pFKCatalog, 0);
	if (!FKschemaNull) 
		(*env)->ReleaseByteArrayElements(env, FKschema, pFKSchema, 0);
	if (!FKtableNull) 
		(*env)->ReleaseByteArrayElements(env, FKtable, pFKTable, 0);
}


//----------------------------------------------------------------------------
// freeConnect
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL	Java_sun_jdbc_odbc_JdbcOdbc_freeConnect(
	JNIEnv *env, jobject callingObject,
	jlong hDbc,
	jbyteArray errorCode)
{
	// Get the data structure portion of the error code
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	
	RETCODE	rc;

	// Free the connection handle
	
	rc = SQLFreeConnect (
		(HDBC) hDbc);		// Connection handle

	// Set the error code
	
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
}

//----------------------------------------------------------------------------
// freeEnv
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL	Java_sun_jdbc_odbc_JdbcOdbc_freeEnv(
	JNIEnv *env, jobject callingObject,
	jlong hEnv,
	jbyteArray errorCode)
{
	// Get the data structure portion of the error code
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	RETCODE	rc;
	
	// Free the environment handle
	
	rc = SQLFreeEnv (
		(HENV) hEnv);		// Environment handle

	// Set the error code
	
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
}

//----------------------------------------------------------------------------
// freeStmt
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL	  Java_sun_jdbc_odbc_JdbcOdbc_freeStmt (
	JNIEnv *env, jobject callingObject,
	jlong hStmt,
	jint fOption,
	jbyteArray errorCode)
{
	// Get the data structure portion of the error code
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	
	RETCODE	rc;
	
	// Free the statement handle

	rc = SQLFreeStmt (
		(HSTMT) hStmt,		// Statement handle
		(UWORD) fOption);	// Option


	// Set the error code
	
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
}

//----------------------------------------------------------------------------
// getConnectOption
//----------------------------------------------------------------------------

JNIEXPORT jlong JNICALL Java_sun_jdbc_odbc_JdbcOdbc_getConnectOption(
	JNIEnv *env, jobject callingObject,
	jlong hDbc,
	jshort fOption,
	jbyteArray errorCode)
{
	// Get the data structure portion of the error code
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	
	RETCODE	rc;
	/* SQL_ATTR_TRACEFILE, SQL_ATTR_TRANSLATE_LIB
	 * are pointers (ODBC1.0)
	 */
	long	vParam = 0;

// Get the connection option
	
	rc = SQLGetConnectOption (
		(HDBC) hDbc,		// Connection handle
		fOption,		// Option to retrieve
		&vParam);		// Pointer to value

	// Set the error code
	
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
	return (jlong)vParam;
}

//----------------------------------------------------------------------------
// getConnectOptionString
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL	Java_sun_jdbc_odbc_JdbcOdbc_getConnectOptionString(
	JNIEnv *env, jobject callingObject,
	jlong hDbc,
	jshort fOption,
	jbyteArray szParam,
	jbyteArray errorCode)
{
	// Get the data structure portion of the error code
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	UCHAR*	pParam = NULL;
	
	RETCODE	rc;

	if (szParam) pParam = (UCHAR*) (*env)->GetByteArrayElements(env, szParam, 0);

// Get the connection option
	
	rc = SQLGetConnectOption (
		(HDBC) hDbc,		// Connection handle
		fOption,		// Option to retrieve
		pParam);		// Pointer to value

	// Set the error code
	
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
	(*env)->ReleaseByteArrayElements(env, szParam, pParam, 0);
}

//----------------------------------------------------------------------------
// getCursorName
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL	Java_sun_jdbc_odbc_JdbcOdbc_getCursorName (
	JNIEnv *env, jobject callingObject,
	jlong hStmt,
	jbyteArray szCursor,
	jbyteArray errorCode)
{
	// Get the data structure portion of the error code
	UCHAR*  errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	UCHAR*  pCursor = NULL;
	SWORD	maxLen = 0;
	SWORD	lValue = 0;
	
	RETCODE	rc;
	
	if (szCursor) pCursor = (UCHAR*) (*env)->GetByteArrayElements(env, szCursor, 0);
	if (szCursor) maxLen = (SWORD) (*env)->GetArrayLength(env, szCursor);

	// Get the column data
	
	rc = SQLGetCursorName (
		(HSTMT) hStmt,		// Statement handle
		pCursor,		// Pointer to storage for cursor name
		maxLen,			// Maximum length of cursor name
		&lValue);		// Pointer to cursor name length

	// Set the error code
	
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
	(*env)->ReleaseByteArrayElements(env, szCursor, pCursor, 0);
}

//----------------------------------------------------------------------------
// getDataBinary
// Returns number of bytes read, -1 if eof
//----------------------------------------------------------------------------

JNIEXPORT jint JNICALL Java_sun_jdbc_odbc_JdbcOdbc_getDataBinary (
	JNIEnv *env, jobject callingObject,
	jlong hStmt,
	jint column,
	jint cType,
	jbyteArray rgbValue,
	jint maxLen,
	jbyteArray errorCode)
{
	// Get the data structure portion of the error code
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	UCHAR*	pValue = NULL;
	SQLLEN	lValue = 0;
	
	RETCODE	rc;
	if (rgbValue) pValue = (UCHAR*) (*env)->GetByteArrayElements(env, rgbValue, 0);

	// Get the column data
	
	rc = SQLGetData (
		(HSTMT) hStmt,		// Statement handle
		(UWORD) column,		// Column number
		(SWORD) cType,		// The C data type
		pValue,			// Pointer to storage for the data
		(SQLLEN)maxLen,		// Maximum length of data
		&lValue);		// Pointer to data length

	// If the number of bytes available is greater than our buffer length,
	// return the buffer length (data was truncated)
	
	if ((lValue > maxLen) ||
	    (lValue == SQL_NO_TOTAL)) {
		lValue = maxLen;
	}
	
	// Set the null indicator
	
	errCode[1] = 0;
	if (lValue == SQL_NULL_DATA) {
		errCode[1] = 1;
		lValue = -1;
	}

	// Set the error code
	
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
	(*env)->ReleaseByteArrayElements(env, rgbValue, pValue, 0);
	return (jint)lValue;
}		


//----------------------------------------------------------------------------
// getDataDouble
//----------------------------------------------------------------------------

JNIEXPORT jdouble JNICALL Java_sun_jdbc_odbc_JdbcOdbc_getDataDouble (
	JNIEnv *env, jobject callingObject,
	jlong hStmt,
	jint column,
	jbyteArray errorCode)
{
	// Get the data structure portion of the error code
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	double	value = 0;
	SQLLEN	lValue = 0;
	
	RETCODE	rc;
	// Get the column data
	
	rc = SQLGetData (
		(HSTMT) hStmt,		// Statement handle
		(UWORD) column,		// Column number
		SQL_C_DOUBLE,		// The C data type
		&value,			// Pointer to storage for the data
		(SQLLEN)sizeof (value),	// Maximum length of the data
		&lValue);		// Pointer to data length

	// Set the error code
	
	errCode[0] = (char) rc;

	// Set the null indicator
	errCode[1] = 0;
	if (lValue == SQL_NULL_DATA) {
		errCode[1] = 1;
	}
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
	return (jdouble)value;
}

//----------------------------------------------------------------------------
// getDataFloat
//----------------------------------------------------------------------------

JNIEXPORT jdouble JNICALL Java_sun_jdbc_odbc_JdbcOdbc_getDataFloat (
	JNIEnv *env, jobject callingObject,
	jlong hStmt,
	jint column,
	jbyteArray errorCode)
{
	// Get the data structure portion of the error code
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	double  value = (double) 0;
	SQLLEN	lValue = 0;
	
	RETCODE	rc;
	// Get the column data
	
	rc = SQLGetData (
		(HSTMT) hStmt,		// Statement handle
		(UWORD) column,		// Column number
		SQL_C_DOUBLE,           // The C data type
		&value,			// Pointer to storage for the data
		(SQLLEN)sizeof (value),	// Maximum length of the data
		&lValue);		// Pointer to data length

	// Set the error code
	
	errCode[0] = (char) rc;

	// Set the null indicator
	errCode[1] = 0;
	if (lValue == SQL_NULL_DATA) {
		errCode[1] = 1;
	}
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
	return (jfloat)value;
}

//----------------------------------------------------------------------------
// getDataInteger
//----------------------------------------------------------------------------

JNIEXPORT jint JNICALL Java_sun_jdbc_odbc_JdbcOdbc_getDataInteger (
	JNIEnv *env, jobject callingObject,
	jlong hStmt,
	jint column,
	jbyteArray errorCode)
{
	// Get the data structure portion of the error code
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	SQLLEN value = 0;
	SQLLEN lValue = 0;
	
	RETCODE	rc;
	
	// Get the column data
	
	rc = SQLGetData (
		(HSTMT) hStmt,		// Statement handle
		(UWORD) column,		// Column number
		SQL_C_LONG,		// The C data type
		&value,			// Pointer to storage for the data
		(SQLLEN)sizeof (value),	// Maximum length of the data
		&lValue);		// Pointer to data length

	// Set the error code
	
	errCode[0] = (char) rc;
	// Set the null indicator
	errCode[1] = 0;
	if (lValue == SQL_NULL_DATA) {
		errCode[1] = 1;
	}
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
	return (jint)value;
}		

//----------------------------------------------------------------------------
// getDataString
//----------------------------------------------------------------------------

JNIEXPORT jint JNICALL Java_sun_jdbc_odbc_JdbcOdbc_getDataString (
	JNIEnv *env, jobject callingObject,
	jlong hStmt,
	jint column,
	jbyteArray rgbValue,
	jbyteArray errorCode)
{
	// Get the data structure portion of the error code
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	UCHAR*	pValue = NULL;
	SQLLEN 	maxLen = 0;
	SQLLEN	lValue = 0;
	
	RETCODE	rc;
	if (rgbValue) pValue = (UCHAR*) (*env)->GetByteArrayElements(env, rgbValue, 0);
	if (rgbValue) maxLen = (SDWORD) (*env)->GetArrayLength(env, rgbValue);

	// Init the data buffer to an empty string

	if (pValue) memset (pValue, 0x00, maxLen);
	
	// Get the column data
	
	rc = SQLGetData (
		(HSTMT) hStmt,		// Statement handle
		(UWORD) column,		// Column number
		SQL_C_CHAR,		// The C data type
		pValue,			// Pointer to storage for the data
		maxLen,			// Maximum length of the data
		&lValue);		// Pointer to data length

	// Set the error code
	errCode[0] = (char) rc;

	// Set the null indicator
	errCode[1] = 0;
	if (lValue == SQL_NULL_DATA) {
		errCode[1] = 1;
	}

	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
	(*env)->ReleaseByteArrayElements(env, rgbValue, pValue, 0);
	return (jint) lValue;
}

//----------------------------------------------------------------------------
// getDataStringDate
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL	Java_sun_jdbc_odbc_JdbcOdbc_getDataStringDate (
	JNIEnv *env, jobject callingObject,
	jlong hStmt,
	jint column,
	jbyteArray rgbValue,
	jbyteArray errorCode)
{
	// Get the data structure portion of the error code
	UCHAR*  errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	UCHAR*	pValue = NULL;
	SQLLEN	lValue = 0;
	DATE_STRUCT dt;
	
	RETCODE	rc;
	if (rgbValue) pValue = (UCHAR*) (*env)->GetByteArrayElements(env, rgbValue, 0);

	// Construct our date structure
	
	memset (&dt, 0x00, sizeof (dt));

	// Init the data buffer to an empty string
	
	if (pValue) pValue[0] = 0x00;
	// Get the column data as a date, then convert to a string
	
	rc = SQLGetData (
		(HSTMT) hStmt,		// Statement handle
		(UWORD) column,		// Column number
		SQL_C_DATE,		// The C data type
		&dt,			// Pointer to storage for the data
		(SQLLEN)sizeof (dt),	// Maximum length of the data
		&lValue);		// Pointer to data length

	// Set the error code
	
	errCode[0] = (char) rc;

	// Set the null indicator
	errCode[1] = 0;
	if (lValue == SQL_NULL_DATA) {
		errCode[1] = 1;
	}

	// Not a null column.  Create the string date
	
	else {
		if (pValue) sprintf (pValue, "%04i-%02i-%02i", dt.year, dt.month,
			 dt.day);
	}
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
	(*env)->ReleaseByteArrayElements(env, rgbValue, pValue, 0);
}

// 4412437
//----------------------------------------------------------------------------
// convertDateString
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_sun_jdbc_odbc_JdbcOdbc_convertDateString (
	JNIEnv *env, jobject callingObject,
	jbyteArray dataBuf,
	jbyteArray dateString)
{
	DATE_STRUCT dt;
	UCHAR* pValue = NULL;
	UCHAR* dataBufArr = NULL;
	
	if (dataBuf) dataBufArr = (UCHAR*) (*env)->GetByteArrayElements(env, dataBuf, 0);
	if (dateString) pValue = (UCHAR*) (*env)->GetByteArrayElements(env, dateString, 0);
	
	memset (&dt, 0x00, sizeof (dt));
	
	memcpy(&dt, dataBufArr, sizeof(dt)); 
	
	
	if (dataBufArr) sprintf (pValue, "%04i-%02i-%02i", dt.year, dt.month,
			 dt.day);
	
	(*env)->ReleaseByteArrayElements(env, dataBuf, dataBufArr, 0);
	(*env)->ReleaseByteArrayElements(env, dateString, pValue, 0);
}

// 4412437
//----------------------------------------------------------------------------
// convertTimeString
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_sun_jdbc_odbc_JdbcOdbc_convertTimeString (
	JNIEnv *env, jobject callingObject,
	jbyteArray dataBuf,
	jbyteArray timeString)
{
	TIME_STRUCT dt;
	UCHAR* pValue = NULL;
	UCHAR* dataBufArr = NULL;
	
	if (dataBuf) dataBufArr = (UCHAR*) (*env)->GetByteArrayElements(env, dataBuf, 0);
	if (timeString) pValue = (UCHAR*) (*env)->GetByteArrayElements(env, timeString, 0);
	
	memset (&dt, 0x00, sizeof (dt));
	
	memcpy(&dt, dataBufArr, sizeof(dt)); 
	
	
	if (dataBufArr) sprintf (pValue, "%02i:%02i:%02i", dt.hour, dt.minute,
			 dt.second);
	
	(*env)->ReleaseByteArrayElements(env, dataBuf, dataBufArr, 0);
	(*env)->ReleaseByteArrayElements(env, timeString, pValue, 0);
}


//----------------------------------------------------------------------------
// convertTimestampString
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_sun_jdbc_odbc_JdbcOdbc_convertTimestampString (
	JNIEnv *env, jobject callingObject,
	jbyteArray dataBuf,
	jbyteArray timestampString)
{
	TIMESTAMP_STRUCT dt;
	UCHAR* pValue = NULL;
	UCHAR* dataBufArr = NULL;
	
	if (dataBuf) dataBufArr = (UCHAR*) (*env)->GetByteArrayElements(env, dataBuf, 0);
	if (timestampString) pValue = (UCHAR*) (*env)->GetByteArrayElements(env, timestampString, 0);
	
	memset (&dt, 0x00, sizeof (dt));
	
	memcpy(&dt, dataBufArr, sizeof(dt)); 
	
	if (dataBufArr) sprintf (pValue, "%04i-%02i-%02i %02i:%02i:%02i.%09li",
			     dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second, dt.fraction);
			
	(*env)->ReleaseByteArrayElements(env, dataBuf, dataBufArr, 0);
	(*env)->ReleaseByteArrayElements(env, timestampString, pValue, 0);

}


// 4412437
//----------------------------------------------------------------------------
// getDateStruct
//----------------------------------------------------------------------------
JNIEXPORT void JNICALL Java_sun_jdbc_odbc_JdbcOdbc_getDateStruct (
	JNIEnv *env, jobject callingObject,
	jbyteArray dataBuf,
	jint year,
	jint month,
	jint day)
{
	DATE_STRUCT dt;
	UCHAR* dataBufArr = NULL;
	
	if (dataBuf) dataBufArr = (UCHAR*) (*env)->GetByteArrayElements(env, dataBuf, 0);
	
	dt.year = year;
	dt.month = month;
	dt.day = day;
	
	memset(dataBufArr, 0x00, sizeof(dataBufArr));
	memcpy(dataBufArr, &dt, sizeof(dt));
	
	(*env)->ReleaseByteArrayElements(env, dataBuf, dataBufArr, 0);
	
}

// 4412437
//----------------------------------------------------------------------------
// getTimeStruct
//----------------------------------------------------------------------------
JNIEXPORT void JNICALL Java_sun_jdbc_odbc_JdbcOdbc_getTimeStruct (
	JNIEnv *env, jobject callingObject,
	jbyteArray dataBuf,
	jint hour,
	jint minute,
	jint second)
{
	TIME_STRUCT dt;
	UCHAR* dataBufArr = NULL;
	
	if (dataBuf) dataBufArr = (UCHAR*) (*env)->GetByteArrayElements(env, dataBuf, 0);
	
	dt.hour = hour;
	dt.minute = minute;
	dt.second = second;
	
	memset(dataBufArr, 0x00, sizeof(dataBufArr));
	memcpy(dataBufArr, &dt, sizeof(dt));
	
	(*env)->ReleaseByteArrayElements(env, dataBuf, dataBufArr, 0);
	
}

//4532162
//----------------------------------------------------------------------------
// getTimestampStruct
//----------------------------------------------------------------------------
JNIEXPORT void JNICALL Java_sun_jdbc_odbc_JdbcOdbc_getTimestampStruct (
	JNIEnv *env, jobject callingObject,
	jbyteArray dataBuf,
	jint year,
	jint month,
	jint day,
	jint hour,
	jint minute,
	jint second,
	jlong fraction)
{
	TIMESTAMP_STRUCT dt;
	UCHAR* dataBufArr = NULL;
	
	if (dataBuf) dataBufArr = (UCHAR*) (*env)->GetByteArrayElements(env, dataBuf, 0);
	
	dt.year = year;
	dt.month = month;
	dt.day = day;
	dt.hour = hour;
	dt.minute = minute;
	dt.second = second;
	dt.fraction = fraction;
	
	memset(dataBufArr, 0x00, sizeof(dataBufArr));
	memcpy(dataBufArr, &dt, sizeof(dt));
	
	(*env)->ReleaseByteArrayElements(env, dataBuf, dataBufArr, 0);
	
}

//----------------------------------------------------------------------------
// getDataStringTime
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL	Java_sun_jdbc_odbc_JdbcOdbc_getDataStringTime (
	JNIEnv *env, jobject callingObject,
	jlong hStmt,
	jint column,
	jbyteArray rgbValue,
	jbyteArray errorCode)
{
	// Get the data structure portion of the error code
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	UCHAR*	pValue = NULL;
	SQLLEN	lValue = 0;
	TIME_STRUCT tm;
	
	RETCODE	rc;

	if (rgbValue) pValue = (UCHAR*) (*env)->GetByteArrayElements(env, rgbValue, 0);

	// Construct our date structure
	
	memset (&tm, 0x00, sizeof (tm));

	// Init the data buffer to an empty string
	
	if (pValue) pValue[0] = 0x00;

	// Get the column data as a date, then convert to a string
	
	rc = SQLGetData (
		(HSTMT) hStmt,		// Statement handle
		(UWORD) column,		// Column number
		SQL_C_TIME,		// The C data type
		&tm,			// Pointer to storage for the data
		(SQLLEN) sizeof (tm),	// Maximum length of the data
		&lValue);		// Pointer to data length

	// Set the error code
	
	errCode[0] = (char) rc;

	// Set the null indicator
	errCode[1] = 0;
	if (lValue == SQL_NULL_DATA) {
		errCode[1] = 1;
	}

	// Not a null column.  Create the string time
	
	else {
		sprintf (pValue, "%02i:%02i:%02i", tm.hour, tm.minute,
			 tm.second);
	}
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
	(*env)->ReleaseByteArrayElements(env, rgbValue, pValue, 0);
}

//----------------------------------------------------------------------------
// getDataStringTimestamp
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL	Java_sun_jdbc_odbc_JdbcOdbc_getDataStringTimestamp (
	JNIEnv *env, jobject callingObject,
	jlong hStmt,
	jint column,
	jbyteArray rgbValue,
	jbyteArray errorCode)
{
	// Get the data structure portion of the error code
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	UCHAR*	pValue = NULL;
	SQLLEN	lValue = 0;
	TIMESTAMP_STRUCT tm;
	
	RETCODE	rc;

	// rgbValue always set - unnecessary ?
	if (rgbValue) pValue = (UCHAR*) (*env)->GetByteArrayElements(env, rgbValue, 0);

// Construct our date structure
	
	memset (&tm, 0x00, sizeof (tm));

	// Init the data buffer to an empty string
	
	if (pValue) pValue[0] = 0x00;

	// Get the column data as a date, then convert to a string
	
	rc = SQLGetData (
		(HSTMT) hStmt,		// Statement handle
		(UWORD) column,		// Column number
		SQL_C_TIMESTAMP,	// The C data type
		&tm,			// Pointer to storage for the data
		(SQLLEN) sizeof (tm),	// Maximum length of the data
		&lValue);		// Pointer to data length

	// Set the error code
	
	errCode[0] = (char) rc;

	// Set the null indicator
	errCode[1] = 0;
	if (lValue == SQL_NULL_DATA) {
		errCode[1] = 1;
	}

	// Not a null column.  Create the string time
	
	else {
		sprintf (pValue, "%04i-%02i-%02i %02i:%02i:%02i",
			 tm.year, tm.month, tm.day, tm.hour,
			 tm.minute, tm.second);
		if (tm.fraction != 0) {
			sprintf (&pValue[strlen ((LPCSTR) pValue)], ".%09li",
				 tm.fraction);
		}
	}
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
	(*env)->ReleaseByteArrayElements(env, rgbValue, pValue, 0);
}		

//----------------------------------------------------------------------------
// getInfo
//----------------------------------------------------------------------------

JNIEXPORT jint JNICALL Java_sun_jdbc_odbc_JdbcOdbc_getInfo(
	JNIEnv *env, jobject callingObject,
	jlong hDbc,
	jshort fInfoType,
	jbyteArray errorCode)
{
	// Get the data structure portion of the error code
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	RETCODE	rc;
	SQLULEN	infoValue = 0;
	SWORD	lInfoValue = 0; 

	// Get the requested info
	rc = SQLGetInfo (
		(HDBC) hDbc,			// Connection handle
		fInfoType,			// Type of information
		&infoValue,			// Pointer to storage for the info
		(SWORD) sizeof (infoValue),	// Maximum length of the data
		&lInfoValue);			// Pointer to the information length

	// Set the error code
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
	return (jint)infoValue;
}

//----------------------------------------------------------------------------
// getInfoShort
//----------------------------------------------------------------------------

JNIEXPORT jint JNICALL Java_sun_jdbc_odbc_JdbcOdbc_getInfoShort(
	JNIEnv *env, jobject callingObject,
	jlong hDbc,
	jshort fInfoType,
	jbyteArray errorCode)
{
	// Get the data structure portion of the error code
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	
	RETCODE	rc;
	SQLULEN	infoValue = 0;
	SWORD	lInfoValue = 0; 

	// Get the requested info
	
	rc = SQLGetInfo (
		(HDBC) hDbc,			// Connection handle
		fInfoType,			// Type of information
		&infoValue,			// Pointer to storage for the info
		(SWORD) sizeof (infoValue),	// Maximum length of the data
		&lInfoValue);			// Pointer to the information length

	// Set the error code
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
	return infoValue;
}

//----------------------------------------------------------------------------
// getInfoString
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL	Java_sun_jdbc_odbc_JdbcOdbc_getInfoString(
	JNIEnv *env, jobject callingObject,
	jlong hDbc,
	jshort fInfoType,
	jbyteArray szParam,
	jbyteArray errorCode)
{
	// Get the data structure portion of the error code
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	UCHAR* pParam = NULL;
	SWORD	lParam = 0;
	SWORD   lInfoValue = 0; 
	
	RETCODE	rc;

	if (szParam) pParam = (UCHAR*) (*env)->GetByteArrayElements(env, szParam, 0);
	if (szParam) lParam = (SWORD) (*env)->GetArrayLength(env, szParam);

	// Get the requested info
	
	rc = SQLGetInfo (
		(HDBC) hDbc,		// Connection handle
		fInfoType,		// Type of information
		pParam,			// Pointer to storage for the info
		lParam,			// Maximum length of the data
		&lInfoValue);		// Pointer to the information length

	// Set the error code
	
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
	(*env)->ReleaseByteArrayElements(env, szParam, pParam, 0);
}

//----------------------------------------------------------------------------
// getStmtOption
//----------------------------------------------------------------------------

JNIEXPORT jlong JNICALL Java_sun_jdbc_odbc_JdbcOdbc_getStmtOption (
	JNIEnv *env, jobject callingObject,
	jlong hStmt,
	jshort fOption,
	jbyteArray errorCode)
{
	// Get the data structure portion of the error code
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	
	RETCODE	rc;
	/* Can be pointer value V3.0
	 */
	long	vParam = 0;

// Get the connection option
	
	rc = SQLGetStmtOption (
		(HSTMT) hStmt,		// Statement handle
		fOption,		// Option to retrieve
		&vParam);		// Value

	// Set the error code
	
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
	return (jlong)vParam;
}

//----------------------------------------------------------------------------
// getStmtAttr
//----------------------------------------------------------------------------

JNIEXPORT jint JNICALL Java_sun_jdbc_odbc_JdbcOdbc_getStmtAttr(
	JNIEnv *env, jobject callingObject,
	jlong hStmt,
	jint fOptionType,
	jbyteArray errorCode)
{
	// Get the data structure portion of the error code
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);

	RETCODE	rc;
	

	int	optionValue = 0;
	SDWORD	loptionValue = 0; 
	
	rc = SQLGetStmtAttr (
		(SQLHSTMT) hStmt,			// Connection handle
		fOptionType,				// Type of information
		&optionValue,				// Pointer to storage for the info
		(SDWORD) sizeof (optionValue),		// Maximum length of the data
		&loptionValue);				// Pointer to the information length

	// Set the error code
	
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
	return optionValue;
}

//----------------------------------------------------------------------------
// getTypeInfo
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL	Java_sun_jdbc_odbc_JdbcOdbc_getTypeInfo (
	JNIEnv *env, jobject callingObject,
	jlong hStmt,
	jshort fSqlType,
	jbyteArray errorCode)
{
	// Get the data structure portion of the error code
	UCHAR*	errCode = (UCHAR*)(*env)->GetByteArrayElements(env, errorCode, 0);	
	RETCODE	rc;

// Get the type infos
	
	rc = SQLGetTypeInfo (
		(HSTMT) hStmt,		// Statement handle
		fSqlType);		// The SQL data type

	// Set the error code
	
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
}

//----------------------------------------------------------------------------
// moreResults
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL	Java_sun_jdbc_odbc_JdbcOdbc_moreResults (
	JNIEnv *env, jobject callingObject,
	jlong hStmt,
	jbyteArray errorCode)
{
	// Get the data structure portion of the error code
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	
	RETCODE	rc;

	// Get the next result set
	
	rc = SQLMoreResults (
		(HSTMT) hStmt);		// Statement handle

	// Set the error code
	
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
}

//----------------------------------------------------------------------------
// nativeSql
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL	Java_sun_jdbc_odbc_JdbcOdbc_nativeSql (
	JNIEnv *env, jobject callingObject,
	jlong hDbc,
	jbyteArray query,
	jbyteArray nativeQuery,
	jbyteArray errorCode)
{
	// Get the data structure portion of the error code
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	UCHAR*	pQuery = NULL;
	UCHAR*	pNativeQuery = NULL;
	SDWORD	lNativeQuery = 0;
	SDWORD	lOut = 0;

	RETCODE rc;

	if (query) pQuery = (UCHAR*) (*env)->GetByteArrayElements(env, query, 0);
	if (nativeQuery) pNativeQuery = (UCHAR*) (*env)->GetByteArrayElements(env, nativeQuery, 0);
	if (nativeQuery) lNativeQuery = (*env)->GetArrayLength(env, nativeQuery);

	// Call SQLNativeSql

	rc = SQLNativeSql (
		(HDBC) hDbc,		// Connection handle
		pQuery,			// SQL text string to be translated
		SQL_NTS,		// Length of text string
		pNativeQuery,		// Pointer to storage for translated
					//  string
		lNativeQuery,		// Maximum length of translated string
		&lOut);			// Pointer to translated string length

	// Set the error code
	
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
	(*env)->ReleaseByteArrayElements(env, query, pQuery, 0);
	(*env)->ReleaseByteArrayElements(env, nativeQuery, pNativeQuery, 0);
}


//----------------------------------------------------------------------------
// numParams
//----------------------------------------------------------------------------

JNIEXPORT jint JNICALL Java_sun_jdbc_odbc_JdbcOdbc_numParams (
	JNIEnv *env, jobject callingObject,
	jlong hStmt,
	jbyteArray errorCode)
{
	// Get the data structure portion of the error code
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	
	RETCODE	rc;
	short	numParams = 0;

// Get the number of parameters in the prepared statement
	
	rc = SQLNumParams (
		(HSTMT) hStmt,		// Statement handle
		&numParams);		// Number of parameters in the
					//  statement

// Set the error code
	
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
	return numParams;
}

//----------------------------------------------------------------------------
// numResultCols
//----------------------------------------------------------------------------

JNIEXPORT jint JNICALL Java_sun_jdbc_odbc_JdbcOdbc_numResultCols (
	JNIEnv *env, jobject callingObject,
	jlong hStmt,
	jbyteArray errorCode)
{
	// Get the data structure portion of the error code
	UCHAR*	errCode = (UCHAR*)(*env)->GetByteArrayElements(env, errorCode, 0);	
	
	RETCODE	rc;
	short	numCols = 0;

// Get the number of columns in the result set
	
	rc = SQLNumResultCols (
		(HSTMT) hStmt,		// Statement handle
		&numCols);		// Number of columns in the result set

	// Set the error code
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
	return numCols;
}

//----------------------------------------------------------------------------
// paramData
//----------------------------------------------------------------------------

JNIEXPORT jint JNICALL Java_sun_jdbc_odbc_JdbcOdbc_paramData (
	JNIEnv *env, jobject callingObject,
	jlong hStmt,
	jbyteArray errorCode)
{
	// Get the data structure portion of the error code
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	
	RETCODE	rc;
	//4691886
	LONG_IA64 pBuf;
	/* It appears that nothing but 4 byte data
	 * is ever set with BindColData
	 */
	jint	param = -1;

	// Get the parameter data.  This is simply the parameter number, as
	// supplied to SQLBindParameter
	
	rc = SQLParamData (
		(SQLHSTMT) hStmt,		// Statement handle
		(SQLPOINTER) &pBuf);		// Pointer to storage for the value

	// If we need data, get the parameter number that needs is
	
	if (rc == SQL_NEED_DATA) {
		memcpy (&param, (SQLPOINTER) pBuf, sizeof (param));
	}
	// Set the error code
	
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
	return (jint)param;
}

//----------------------------------------------------------------------------
// paramDataInBlock
//----------------------------------------------------------------------------

JNIEXPORT jint JNICALL Java_sun_jdbc_odbc_JdbcOdbc_paramDataInBlock (
	JNIEnv *env, jobject callingObject,
	jlong hStmt,
	jint rowPos,
	jbyteArray errorCode)
{
	// Get the data structure portion of the error code
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	
	RETCODE	rc;
	UDWORD	pBuf;
	jint	param = -1;
	int i=0;
	
	// offSet is the number of bytes to subtract in order to point to the
	// correct storage that contains the column value for a block-cursor.
	// this bug is a result of moving within a rowSet > 1. 
	// after a setPos() w/ SQL_POSITION call was made instead of a FetchScroll()
	int	offSet;
	
	// Get the parameter data.  This is simply the parameter number, as
	// supplied to SQLBindCol
	
	rc = SQLParamData (
		(SQLHSTMT) hStmt,		// Statement handle
		(SQLPOINTER) &pBuf);	// Pointer to storage for the value

	// If we need data, get the column number that needs is
	
	if (rc == SQL_NEED_DATA) 
	{
	
		if (rowPos >= 1)
			offSet = ( (rowPos) * sizeof(param) );
		else
			offSet = 0;

		memcpy (&param, (SQLPOINTER) (pBuf - offSet), sizeof (param));
	}
	
	// Set the error code
	
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
	return (jint)param;
}


//----------------------------------------------------------------------------
// prepare
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL	Java_sun_jdbc_odbc_JdbcOdbc_prepare (
	JNIEnv *env, jobject callingObject,
	jlong hStmt,
	jbyteArray SQLString,
	jbyteArray errorCode)
{
	// Get the data structure portion of the error code
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	RETCODE	rc;
	UCHAR*	pSQL = NULL;

	if (SQLString) pSQL = (UCHAR*) (*env)->GetByteArrayElements(env, SQLString, 0);

	rc = SQLPrepare (
		(HSTMT) hStmt,		// Statement handle
		pSQL,			// SQL text string
		SQL_NTS);		// Length of the SQL text string

	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
	// Prevent JVM GP Error by ignoring null Strings
	if (SQLString)
		(*env)->ReleaseByteArrayElements(env, SQLString, pSQL, 0);
}

//----------------------------------------------------------------------------
// primaryKeys
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_sun_jdbc_odbc_JdbcOdbc_primaryKeys (
	JNIEnv *env, jobject callingObject,
	jlong hStmt,
	jbyteArray catalog, jboolean catalogNull,
	jbyteArray schema, jboolean schemaNull,
	jbyteArray table, jboolean tableNull,
	jbyteArray errorCode)
{
	// Get the data structure portion of the error code
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	
	RETCODE	rc;
	UCHAR*	pCatalog = NULL;
	UCHAR*	pSchema = NULL;
	UCHAR*	pTable = NULL;

	// Check if the parameter is null.  If it is, we need to pass
	// a NULL to ODBC.  Unfortuneatly, Java converts a null string
	// to an empty string.
	
	if (!catalogNull) {
		pCatalog = (UCHAR*) (*env)->GetByteArrayElements(env, catalog, 0);
	}
	
	if (!schemaNull) {
		pSchema = (UCHAR*) (*env)->GetByteArrayElements(env, schema, 0);
	}
	
	if (!tableNull) {
		pTable = (UCHAR*) (*env)->GetByteArrayElements(env, table, 0);
	}
	
	rc = SQLPrimaryKeys (
		(HSTMT) hStmt,		// Statement handle
		pCatalog,		// Table catalog (qualifier)
		SQL_NTS,		// Length of catalog	
		pSchema,		// Table schema name (owner)
		SQL_NTS,		// Length of schema name
		pTable,			// Table name
		SQL_NTS);		// Length of table name

	errCode[0] = (char) rc;

	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
	if (!catalogNull) 
		(*env)->ReleaseByteArrayElements(env, catalog, pCatalog, 0);
	if (!schemaNull) 
		(*env)->ReleaseByteArrayElements(env, schema, pSchema, 0);
	if (!tableNull) 
		(*env)->ReleaseByteArrayElements(env, table, pTable, 0);
}

//----------------------------------------------------------------------------
// procedures
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_sun_jdbc_odbc_JdbcOdbc_procedures (
	JNIEnv *env, jobject callingObject,
	jlong hStmt,
	jbyteArray catalog, jboolean catalogNull,
	jbyteArray schema, jboolean schemaNull,
	jbyteArray procedure, jboolean procedureNull,
	jbyteArray errorCode)
{
	// Get the data structure portion of the error code
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	
	RETCODE	rc;
	UCHAR*	pCatalog = NULL;
	UCHAR*	pSchema = NULL;
	UCHAR*	pProcedure = NULL;

	// Check if the parameter is null.  If it is, we need to pass
	// a NULL to ODBC.  Unfortuneatly, Java converts a null string
	// to an empty string.

	if (!catalogNull) {
		pCatalog = (UCHAR*) (*env)->GetByteArrayElements(env, catalog, 0);
	}
	
	if (!schemaNull) {
		pSchema = (UCHAR*) (*env)->GetByteArrayElements(env, schema, 0);
	}
	
	if (!procedureNull) {
		pProcedure = (UCHAR*) (*env)->GetByteArrayElements(env, procedure, 0);
	}

	rc = SQLProcedures (
		(HSTMT) hStmt,		// Statement handle
		pCatalog,		// Procedure catalog (qualifier)
		SQL_NTS,		// Length of catalog
		pSchema,		// Procedure schema name (owner)
		SQL_NTS,		// Length of schema name
		pProcedure,		// Search pattern for procedure names
		SQL_NTS);		// Length of search pattern

	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
	if (!catalogNull) 
		(*env)->ReleaseByteArrayElements(env, catalog, pCatalog, 0);
	if (!schemaNull) 
		(*env)->ReleaseByteArrayElements(env, schema, pSchema, 0);
	if (!procedureNull) 
		(*env)->ReleaseByteArrayElements(env, procedure, pProcedure, 0);
}

//----------------------------------------------------------------------------
// procedureColumns
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_sun_jdbc_odbc_JdbcOdbc_procedureColumns (
	JNIEnv *env, jobject callingObject,
	jlong hStmt,
	jbyteArray catalog, jboolean catalogNull,
	jbyteArray schema, jboolean schemaNull,
	jbyteArray procedure, jboolean procedureNull,
	jbyteArray column, jboolean columnNull,
	jbyteArray errorCode)
{
	// Get the data structure portion of the error code
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	
	RETCODE	rc;
	UCHAR*	pCatalog = NULL;
	UCHAR*	pSchema = NULL;
	UCHAR*	pProcedure = NULL;
	UCHAR*	pColumn = NULL;

	// Check if the parameter is null.  If it is, we need to pass
	// a NULL to ODBC.  Unfortuneatly, Java converts a null string
	// to an empty string.
	
	
	if (!catalogNull) {
		pCatalog = (UCHAR*) (*env)->GetByteArrayElements(env, catalog, 0);
	}
	
	if (!schemaNull) {
		pSchema = (UCHAR*) (*env)->GetByteArrayElements(env, schema, 0);
	}

	if (!procedureNull) {
		pProcedure = (UCHAR*) (*env)->GetByteArrayElements(env, procedure, 0);
	}
	
	if (!columnNull) {
		pColumn = (UCHAR*) (*env)->GetByteArrayElements(env, column, 0);
	}

	rc = SQLProcedureColumns (
		(HSTMT) hStmt,		// Statement handle
		pCatalog,		// Procedure catalog (qualifier)
		SQL_NTS,		// Length of catalog
		pSchema,		// Procedure schema name (owner)
		SQL_NTS,		// Length of schema name
		pProcedure,		// Search string for procedure names
		SQL_NTS,		// Length of search string
		pColumn,		// Search string for procedure columns
		SQL_NTS);		// Length of search string

	errCode[0] = (char) rc;

	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
	if (!catalogNull) 
		(*env)->ReleaseByteArrayElements(env, catalog, pCatalog, 0);
	if (!schemaNull) 
		(*env)->ReleaseByteArrayElements(env, schema, pSchema, 0);
	if (!procedureNull) 
		(*env)->ReleaseByteArrayElements(env, procedure, pProcedure, 0);
	if (!columnNull) 
		(*env)->ReleaseByteArrayElements(env, column, pColumn, 0);
}

//----------------------------------------------------------------------------
// putData
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL	Java_sun_jdbc_odbc_JdbcOdbc_putData (
	JNIEnv *env, jobject callingObject,
	jlong hStmt,
	jbyteArray dataBuf,
	jint dataLen,
	jbyteArray errorCode)
{
	// Get the data structure portion of the error code
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	UCHAR*	pBuf = NULL;
	
	RETCODE	rc;

	if (dataBuf) pBuf = (UCHAR*) (*env)->GetByteArrayElements(env, dataBuf, 0);

	// Put the data

	rc = SQLPutData (
		(HSTMT) hStmt,		// Statement handle
		pBuf,			// Pointer to data
		(SQLLEN) dataLen);	// Length of data

	// Set the error code
	
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
	(*env)->ReleaseByteArrayElements(env, dataBuf, pBuf, 0);
}

//----------------------------------------------------------------------------
// rowCount
//----------------------------------------------------------------------------

JNIEXPORT jint JNICALL Java_sun_jdbc_odbc_JdbcOdbc_rowCount (
	JNIEnv *env, jobject callingObject,
	jlong hStmt,
	jbyteArray errorCode)
{
	// Get the data structure portion of the error code
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	
	RETCODE	rc;
	SQLLEN	numRows = 0;

// Get the number of rows affected
	
	rc = SQLRowCount (
		(HSTMT) hStmt,		// Statement handle
		&numRows);		// Number of rows affected

	// Set the error code
	
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
	return (jint)numRows;
}

//----------------------------------------------------------------------------
// setConnectOption
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL	Java_sun_jdbc_odbc_JdbcOdbc_setConnectOption(
	JNIEnv *env, jobject callingObject,
	jlong hDbc,
	jshort fOption,
	jint vParam,
	jbyteArray errorCode)
{
	// Get the data structure portion of the error code
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	
	RETCODE	rc;

	// Set the connection option
	rc = SQLSetConnectOption (
		(HDBC) hDbc,		// Connection handle
		fOption,		// Option to set
		(SQLULEN) vParam);	// Value

	// Set the error code
	
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
}

//----------------------------------------------------------------------------
// setConnectOptionString
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL	Java_sun_jdbc_odbc_JdbcOdbc_setConnectOptionString(
	JNIEnv *env, jobject callingObject,
	jlong hDbc,
	jshort fOption,
	jbyteArray szParam,
	jbyteArray errorCode)
{
	// Get the data structure portion of the error code
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	
	RETCODE	rc;
	UCHAR*	pParam = NULL;

	if (szParam) pParam = (UCHAR*) (*env)->GetByteArrayElements(env, szParam, 0);

	// Set the connection option
	rc = SQLSetConnectOption (
		(HDBC) hDbc,		// Connection handle
		fOption,		// Option to set
		(SQLULEN) pParam);	// Value

	// Set the error code
	
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
	(*env)->ReleaseByteArrayElements(env, szParam, pParam, 0);
}

//----------------------------------------------------------------------------
// setCursorName
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL	Java_sun_jdbc_odbc_JdbcOdbc_setCursorName (
	JNIEnv *env, jobject callingObject,
	jlong hStmt,
	jbyteArray szCursor,
	jbyteArray errorCode)
{
	// Get the data structure portion of the error code
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	
	RETCODE	rc;
	UCHAR*	pCursor = NULL;

	if (szCursor) pCursor = (UCHAR*) (*env)->GetByteArrayElements(env, szCursor, 0);

	// Set the cursor name
	
	rc = SQLSetCursorName (
		(HSTMT) hStmt,		// Statement handle
		pCursor,		// Cursor name
		SQL_NTS);		// Length of cursor name

	// Set the error code
	
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
	(*env)->ReleaseByteArrayElements(env, szCursor, pCursor, 0);
}

//----------------------------------------------------------------------------
// setStmtOption
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL	Java_sun_jdbc_odbc_JdbcOdbc_setStmtOption(
	JNIEnv *env, jobject callingObject,
	jlong hStmt,
	jshort fOption,
	jint vParam,
	jbyteArray errorCode)
{
	// Get the data structure portion of the error code
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	
	RETCODE	rc;

	// Set the statement option
	
	rc = SQLSetStmtOption (
		(HSTMT) hStmt,		// Statement handle
		fOption,		// Option to set
		(SQLULEN) vParam);	// Value

	// Set the error code
	
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
}


//----------------------------------------------------------------------------
// setStmtAttr
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL	Java_sun_jdbc_odbc_JdbcOdbc_setStmtAttr(
	JNIEnv *env, jobject callingObject,
	jlong hStmt,
	jint fOption,
	jint vParam,
	jint len,
	jbyteArray errorCode)
{
	// Get the data structure portion of the error code
	RETCODE	rc;
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);


	// Set the statement attribute
	
	rc = SQLSetStmtAttr (
		(SQLHSTMT) hStmt,		// Statement handle
		fOption,			// Option to set
  		(SQLPOINTER)vParam,		// Pointer to Value
  		(SQLINTEGER)len);		// String length

	// Set the error code
	
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
}



//----------------------------------------------------------------------------
// setStmtAttr Overloaded.
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL	Java_sun_jdbc_odbc_JdbcOdbc_setStmtAttrPtr(
	JNIEnv *env, jobject callingObject,
	jlong hStmt,
	jint fOption,
	jintArray vParam,
	jint len,
	jbyteArray errorCode,
	jlongArray buffers)
{
	// Get the data structure portion of the error code
	RETCODE	rc;

	UCHAR* errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);

	UDWORD* pvParam = NULL;
	
	//sets the Pointer to attribute as global
	//mainly used for execute Batch processed/status pointers. 
	
	jlong* pBuffers = (jlong*)(*env)->GetLongArrayElements(env, buffers, 0);
	jobject gvParam = (*env)->NewGlobalRef(env, vParam);

	if (gvParam) 
	{
	    pvParam = (jint*)(*env)->GetIntArrayElements(env, gvParam, 0);
	    pBuffers[0] = (LONG_IA64)pvParam;
	    pBuffers[1] = (LONG_IA64)gvParam;
	}
		
	// Set the statement attribute

	(*env)->ReleaseLongArrayElements(env, buffers, pBuffers, 0);
	
	rc = SQLSetStmtAttr (
		(SQLHSTMT) hStmt,		// Statement handle
		fOption,			// Option to set
		(SQLPOINTER) pvParam,		// Pointer to Value
		(SQLINTEGER)len);		// String length

	// Set the error code
	
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);

}

JNIEXPORT void JNICALL	Java_sun_jdbc_odbc_JdbcOdbc_setPos(
	JNIEnv *env, jobject callingObject,
	jlong hStmt,
	jint rowNumb,
	jint operation,
	jint locktype,
	jbyteArray errorCode)
{
	// Get the data structure portion of the error code
	RETCODE	rc;

	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
		
	// Set the statement attribute
	
	rc = SQLSetPos (
		(SQLHSTMT) hStmt,		// Statement handle
		(SQLUSMALLINT)rowNumb,		// row number
		(SQLUSMALLINT)operation,	// Operation to Perform
		(SQLUSMALLINT)locktype);	// lock type after operation is performed.

	// Set the error code
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);

}


//----------------------------------------------------------------------------
// specialColumns
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL	Java_sun_jdbc_odbc_JdbcOdbc_specialColumns (
	JNIEnv *env, jobject callingObject,
	jlong hStmt,
	jshort fOption,
	jbyteArray catalog, jboolean catalogNull,
	jbyteArray schema, jboolean schemaNull,
	jbyteArray table, jboolean tableNull,
	jint fScope,
	jboolean fNullable,
	jbyteArray errorCode)
{
	// Get the data structure portion of the error code
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	
	RETCODE	rc;
	UCHAR*	pCatalog = NULL;
	UCHAR*	pSchema = NULL;
	UCHAR*	pTable = NULL;
	UWORD	nullable = SQL_NO_NULLS;

	// Check if the parameter is null.  If it is, we need to pass
	// a NULL to ODBC.  Unfortuneatly, Java converts a null string
	// to an empty string.
	
	if (!catalogNull) {
		pCatalog = (UCHAR*) (*env)->GetByteArrayElements(env, catalog, 0);
	}
	
	if (!schemaNull) {
		pSchema = (UCHAR*) (*env)->GetByteArrayElements(env, schema, 0);
	}
	
	if (!tableNull) {
		pTable = (UCHAR*) (*env)->GetByteArrayElements(env, table, 0);
	}

	if (fNullable) {
		nullable = SQL_NULLABLE;
	}
	
	rc = SQLSpecialColumns (
		(HSTMT) hStmt,		// Statement handle
		(UWORD) fOption,	// Type of column to return
		pCatalog,		// Table catalog (qualifier)
		SQL_NTS,		// Length of catalog
		pSchema,		// Table schema name (owner)	
		SQL_NTS,		// Length of schema name
		pTable,			// Table name
		SQL_NTS,		// Length of table name
		(UWORD) fScope,		// Minimum scope
		nullable);		// Return column that can have nulls?

	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
	if (!catalogNull) 
		(*env)->ReleaseByteArrayElements(env, catalog, pCatalog, 0);
	if (!schemaNull) 
		(*env)->ReleaseByteArrayElements(env, schema, pSchema, 0);
	if (!tableNull) 
		(*env)->ReleaseByteArrayElements(env, table, pTable, 0);
}

//----------------------------------------------------------------------------
// statistics
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL	Java_sun_jdbc_odbc_JdbcOdbc_statistics (
	JNIEnv *env, jobject callingObject,
	jlong hStmt,
	jbyteArray catalog, jboolean catalogNull,
	jbyteArray schema, jboolean schemaNull,
	jbyteArray table, jboolean tableNull,
	jboolean unique,
	jboolean approximate,
	jbyteArray errorCode)
{
	// Get the data structure portion of the error code
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	
	RETCODE	rc;
	UCHAR*	pCatalog = NULL;
	UCHAR*	pSchema = NULL;
	UCHAR*	 pTable = NULL;
	UWORD	fUnique = SQL_INDEX_ALL;
	UWORD	fAccuracy = SQL_ENSURE;

	// Check if the parameter is null.  If it is, we need to pass
	// a NULL to ODBC.  Unfortuneatly, Java converts a null string
	// to an empty string.
	if (!catalogNull) {
		pCatalog = (UCHAR*) (*env)->GetByteArrayElements(env, catalog, 0);
	}
	
	if (!schemaNull) {
		pSchema = (UCHAR*) (*env)->GetByteArrayElements(env, schema, 0);
	}
	
	if (!tableNull) {
		pTable = (UCHAR*) (*env)->GetByteArrayElements(env, table, 0);
	}
	

	// If only unique indexes were specified, set the proper flag
	
	if (unique) {
		fUnique = SQL_INDEX_UNIQUE;
	}

	// If only approximate indexes are required, set the proper flag

	if (approximate) {
		fAccuracy = SQL_QUICK;
	}
	
	rc = SQLStatistics (
		(HSTMT) hStmt,		// Statement handle
		pCatalog,		// Table catalog (qualifer)
		SQL_NTS,		// Length of catalog
		pSchema,		// Table schema name (owner)
		SQL_NTS,		// Length of schema name
		pTable,			// Table name
		SQL_NTS,		// Length of table name
		fUnique,		// Type of index
		fAccuracy);		// Importance of cardinality and pages

	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
	if (!catalogNull) 
		(*env)->ReleaseByteArrayElements(env, catalog, pCatalog, 0);
	if (!schemaNull) 
		(*env)->ReleaseByteArrayElements(env, schema, pSchema, 0);
	if (!tableNull) 
		(*env)->ReleaseByteArrayElements(env, table, pTable, 0);
}

//----------------------------------------------------------------------------
// tables
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_sun_jdbc_odbc_JdbcOdbc_tables (
	JNIEnv *env, jobject callingObject,
	jlong hStmt,
	jbyteArray catalog, jboolean catalogNull,
	jbyteArray schema, jboolean schemaNull,
	jbyteArray table, jboolean tableNull,
	jbyteArray types, jboolean typesNull,
	jbyteArray errorCode)
{
	// Get the data structure portion of the error code
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	
	RETCODE	rc;
	UCHAR*	pCatalog = NULL;
	UCHAR*	pSchema = NULL;
	UCHAR*	 pTable = NULL;
	UCHAR*	 pTypes = NULL;

	// Check if the parameter is null.  If it is, we need to pass
	// a NULL to ODBC.  Unfortuneatly, Java converts a null string
	// to an empty string.
	
	if (!catalogNull) {
		pCatalog = (UCHAR*) (*env)->GetByteArrayElements(env, catalog, 0);
	}
	
	if (!schemaNull) {
		pSchema = (UCHAR*) (*env)->GetByteArrayElements(env, schema, 0);
	}
	
	if (!tableNull) {
		pTable = (UCHAR*) (*env)->GetByteArrayElements(env, table, 0);
	}

	if (!typesNull) {
		pTypes = (UCHAR*) (*env)->GetByteArrayElements(env, types, 0);
	}

	rc = SQLTables (
		(HSTMT) hStmt,		// Statement handle
		pCatalog,		// Table catalog (qualifer)
		SQL_NTS,		// Length of catalog
		pSchema,		// Table schema name (owner)
		SQL_NTS,		// Length of schema name
		pTable,			// Search pattern for table names
		SQL_NTS,		// Length of search pattern
		pTypes,			// List of table types to match
		SQL_NTS);		// Length of list

	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
	if (!catalogNull) 
		(*env)->ReleaseByteArrayElements(env, catalog, pCatalog, 0);
	if (!schemaNull) 
		(*env)->ReleaseByteArrayElements(env, schema, pSchema, 0);
	if (!tableNull) 
		(*env)->ReleaseByteArrayElements(env, table, pTable, 0);
	if (!typesNull) 
		(*env)->ReleaseByteArrayElements(env, types, pTypes, 0);
}

//----------------------------------------------------------------------------
// tablePrivileges
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_sun_jdbc_odbc_JdbcOdbc_tablePrivileges (
	JNIEnv *env, jobject callingObject,
	jlong hStmt,
	jbyteArray catalog, jboolean catalogNull,
	jbyteArray schema, jboolean schemaNull,
	jbyteArray table, jboolean tableNull,
	jbyteArray errorCode)
{
	// Get the data structure portion of the error code
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);

	RETCODE	rc;
	UCHAR*	pCatalog = NULL;
	UCHAR*	pSchema = NULL;
	UCHAR*	pTable = NULL;

	// Check if the parameter is null.  If it is, we need to pass
	// a NULL to ODBC.  Unfortuneatly, Java converts a null string
	// to an empty string.
	
	if (!catalogNull) {
		pCatalog = (UCHAR*) (*env)->GetByteArrayElements(env, catalog, 0);
	}
	
	if (!schemaNull) {
		pSchema = (UCHAR*) (*env)->GetByteArrayElements(env, schema, 0);
	}
	
	if (!tableNull) {
		pTable = (UCHAR*) (*env)->GetByteArrayElements(env, table, 0);
	}
	
	rc = SQLTablePrivileges (
		(HSTMT) hStmt,		// Statement handle
		pCatalog,		// Table catalog (qualifier)
		SQL_NTS,		// Length of catalog
		pSchema,		// Table schema name (owner)
		SQL_NTS,		// Length of schema name
		pTable,			// Search pattern for table names
		SQL_NTS);		// Length of search pattern

	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
	if (!catalogNull) 
		(*env)->ReleaseByteArrayElements(env, catalog, pCatalog, 0);
	if (!schemaNull) 
		(*env)->ReleaseByteArrayElements(env, schema, pSchema, 0);
	if (!tableNull) 
		(*env)->ReleaseByteArrayElements(env, table, pTable, 0);
}

//----------------------------------------------------------------------------
// transact
//----------------------------------------------------------------------------

JNIEXPORT void JNICALL	Java_sun_jdbc_odbc_JdbcOdbc_transact (
	JNIEnv *env, jobject callingObject,
	jlong hEnv,
	jlong hDbc,
	jshort fType,
	jbyteArray errorCode)
{
	// Get the data structure portion of the error code
	UCHAR*	errCode = (UCHAR*) (*env)->GetByteArrayElements(env, errorCode, 0);
	
	RETCODE	rc;

	// Call SQLTransact
	
	rc = SQLTransact (
		(HENV) hEnv,		// Environment handle
		(HDBC) hDbc,		// Connection handle
		fType);			// Type (commit, rollback)

	// Set the error code
	
	errCode[0] = (char) rc;
	(*env)->ReleaseByteArrayElements(env, errorCode, errCode, 0);
}



// These functions release the stored buffers (Java's garbage collector
// will no longer maintain, or pin, the buffers). 
JNIEXPORT void JNICALL Java_sun_jdbc_odbc_JdbcOdbc_ReleaseStoredBytes (
	JNIEnv *env, jobject callingObject,
	jlong x1,
	jlong x2)
{
	(*env)->ReleaseByteArrayElements(env, (jbyteArray)x2, (UCHAR *)x1, 0);
	(*env)->DeleteGlobalRef(env, (jobject)x2);
}

JNIEXPORT void JNICALL Java_sun_jdbc_odbc_JdbcOdbc_ReleaseStoredChars (
	JNIEnv *env, jobject callingObject,
	jlong s1,
	jlong s2)
{
	(*env)->ReleaseStringUTFChars(env, (jstring)s2, (jbyte *)s1);
	(*env)->DeleteGlobalRef(env, (jobject)s2);
}

JNIEXPORT void JNICALL Java_sun_jdbc_odbc_JdbcOdbc_ReleaseStoredIntegers ( //4486684
	JNIEnv *env, jobject callingObject,
	jlong x1,
	jlong x2)
{
	(*env)->ReleaseIntArrayElements(env, (jintArray)x2, (UDWORD*)x1, 0);
	(*env)->DeleteGlobalRef(env, (jobject)x2);
}

JNIEXPORT jdouble JNICALL Java_sun_jdbc_odbc_JdbcOdbcCallableStatement_getTheOutValue
  (JNIEnv *env, jobject callingObject, jint address)
{
	jdouble *pjDouble = (jdouble *) address;
	jdouble value = *pjDouble;
	return (jdouble)value; 
}
