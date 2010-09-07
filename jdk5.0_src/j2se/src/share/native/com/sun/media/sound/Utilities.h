/*
 * @(#)Utilities.h	1.26 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


// JNI includes
#include <jni.h>

// ENGINE includes
#include "engine/X_API.h"
#include "engine/GenSnd.h"			// need for OpErr stuff

#include "Configure.h"				// put flags for debug msgs etc. here

// Global midi song count
XShortResourceID getMidiSongCount();

// return 1 if this platform is big endian, or 0 for little endian
int UTIL_IsBigEndianPlatform();

// ERROR PRINTS (from Jnc.h)

#ifdef USE_ERROR
#define ERROR0(string)                        fprintf(stdout, (string)); fflush(stdout);
#define ERROR1(string, p1)                    fprintf(stdout, (string), (p1)); fflush(stdout);
#define ERROR2(string, p1, p2)                fprintf(stdout, (string), (p1), (p2)); fflush(stdout);
#define ERROR3(string, p1, p2, p3)            fprintf(stdout, (string), (p1), (p2), (p3)); fflush(stdout);
#define ERROR4(string, p1, p2, p3, p4)        fprintf(stdout, (string), (p1), (p2), (p3), (p4)); fflush(stdout);
#else
#define ERROR0(string)
#define ERROR1(string, p1)
#define ERROR2(string, p1, p2)
#define ERROR3(string, p1, p2, p3)
#define ERROR4(string, p1, p2, p3, p4)
#endif


// TRACE PRINTS (from Jnc.h)

#ifdef USE_TRACE
#define TRACE0(string)                        fprintf(stdout, (string)); fflush(stdout);
#define TRACE1(string, p1)                    fprintf(stdout, (string), (p1)); fflush(stdout);
#define TRACE2(string, p1, p2)                fprintf(stdout, (string), (p1), (p2)); fflush(stdout);
#define TRACE3(string, p1, p2, p3)            fprintf(stdout, (string), (p1), (p2), (p3)); fflush(stdout);
#define TRACE4(string, p1, p2, p3, p4)        fprintf(stdout, (string), (p1), (p2), (p3), (p4)); fflush(stdout);
#define TRACE5(string, p1, p2, p3, p4, p5)    fprintf(stdout, (string), (p1), (p2), (p3), (p4), (p5)); fflush(stdout);
#else
#define TRACE0(string)
#define TRACE1(string, p1)
#define TRACE2(string, p1, p2)
#define TRACE3(string, p1, p2, p3)
#define TRACE4(string, p1, p2, p3, p4)
#define TRACE5(string, p1, p2, p3, p4, p5)
#endif


// VERBOSE TRACE PRINTS

#ifdef USE_VERBOSE_TRACE
#define VTRACE0(string)			fprintf(stdout, (string));
#define VTRACE1(string, p1)		fprintf(stdout, (string), (p1));
#define VTRACE2(string, p1, p2)		printf(stdout, (string), (p1), (p2));
#define VTRACE3(string, p1, p2, p3)	fprintf(stdout, (string), (p1), (p2), (p3));
#define VTRACE4(string, p1, p2, p3, p4)	fprintf(stdout, (string), (p1), (p2), (p3), (p4));
#else
#define VTRACE0(string)
#define VTRACE1(string, p1)
#define VTRACE2(string, p1, p2)
#define VTRACE3(string, p1, p2, p3)
#define VTRACE4(string, p1, p2, p3, p4)
#endif


void ThrowJavaMessageException(JNIEnv* e, char const* exception, char const* message);

void ThrowJavaOpErrException(JNIEnv* e, char const* exception, OPErr opErr);

char const* TranslateOPErr(OPErr opErr);

void SleepMillisInJava(JNIEnv* e, INT32 millis);

// from Hae_Impl.h
#define DOUBLE_TO_FIXED(x)  ((UINT32)((x) * 65536.0))
#define FIXED_TO_DOUBLE(x)  (((jdouble)(x)) / 65536.0)
//$$fb 2002-02-07: itanium port - remove compiler typecast warnings
#define FLOAT_TO_FIXED(x)  ((UINT32)(((jfloat)x) * 65536.0f))
#define FIXED_TO_FLOAT(x)  (((jfloat)(x)) / 65536.0f)
#define DOUBLE_TO_VOLUME(v) ((unsigned short)((v) * MAX_NOTE_VOLUME))
#define VOLUME_TO_DOUBLE(v) ((jdouble)(v) / MAX_NOTE_VOLUME)
#define FLOAT_TO_VOLUME(v) ((unsigned short)((v) * MAX_NOTE_VOLUME))
#define VOLUME_TO_FLOAT(v) ((jfloat)(v) / MAX_NOTE_VOLUME)
#define DOUBLE_TO_PAN(v)    ((short)((v) * 63))
#define PAN_TO_DOUBLE(v)    ((jdouble)(v) / 63)
#define FLOAT_TO_PAN(v)    ((short)((v) * 63))
#define PAN_TO_FLOAT(v)    ((jfloat)(v) / 63)
#define INT_TO_FIXED(v)		((UINT32)(((jint)v) * 65536))
#define FIXED_TO_INT(v)    (((jint)(v)) / 65536)

