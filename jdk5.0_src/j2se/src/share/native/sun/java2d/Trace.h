/*
 * @(#)Trace.h	1.3 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef _Included_Trace
#define _Included_Trace

#include <jni.h>
#include "debug_trace.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * J2dTrace
 * Trace utility used throughout Java 2D code.  Uses a "level"
 * parameter that allows user to specify how much detail
 * they want traced at runtime.  Tracing is only enabled
 * in debug mode, to avoid overhead running release build.
 */

#define J2D_TRACE_INVALID 	-1
#define J2D_TRACE_OFF 		0
#define J2D_TRACE_ERROR 	1
#define J2D_TRACE_WARNING 	2
#define J2D_TRACE_INFO 		3
#define J2D_TRACE_VERBOSE 	4

JNIEXPORT void JNICALL
J2dTraceImpl(int level, jboolean cr, const char *string, ...);
JNIEXPORT void JNICALL
J2dTraceInit();

#ifndef DEBUG
#define J2dTrace(level, string)
#define J2dTrace1(level, string, arg1)
#define J2dTrace2(level, string, arg1, arg2)
#define J2dTrace3(level, string, arg1, arg2, arg3)
#define J2dTrace4(level, string, arg1, arg2, arg3, arg4)
#define J2dTrace5(level, string, arg1, arg2, arg3, arg4, arg5)
#define J2dTraceLn(level, string)
#define J2dTraceLn1(level, string, arg1)
#define J2dTraceLn2(level, string, arg1, arg2)
#define J2dTraceLn3(level, string, arg1, arg2, arg3)
#define J2dTraceLn4(level, string, arg1, arg2, arg3, arg4)
#define J2dTraceLn5(level, string, arg1, arg2, arg3, arg4, arg5)
#else /* DEBUG */
#define J2dTrace(level, string) { \
	    J2dTraceImpl(level, JNI_FALSE, string); \
	    DTRACE_PRINT(string); \
	}
#define J2dTrace1(level, string, arg1) { \
	    J2dTraceImpl(level, JNI_FALSE, string, arg1); \
	    DTRACE_PRINT1(string, arg1); \
	}
#define J2dTrace2(level, string, arg1, arg2) { \
	    J2dTraceImpl(level, JNI_FALSE, string, arg1, arg2); \
	    DTRACE_PRINT2(string, arg1, arg2); \
	}
#define J2dTrace3(level, string, arg1, arg2, arg3) { \
	    J2dTraceImpl(level, JNI_FALSE, string, arg1, arg2, arg3); \
	    DTRACE_PRINT3(string, arg1, arg2, arg3); \
	}
#define J2dTrace4(level, string, arg1, arg2, arg3, arg4) { \
	    J2dTraceImpl(level, JNI_FALSE, string, arg1, arg2, arg3, arg4); \
	    DTRACE_PRINT4(string, arg1, arg2, arg3, arg4); \
	}
#define J2dTrace5(level, string, arg1, arg2, arg3, arg4, arg5) { \
	    J2dTraceImpl(level, JNI_FALSE, string, arg1, arg2, arg3, arg4, arg5); \
	    DTRACE_PRINT5(string, arg1, arg2, arg3, arg4, arg5); \
	}
#define J2dTraceLn(level, string) { \
	    J2dTraceImpl(level, JNI_TRUE, string); \
	    DTRACE_PRINTLN(string); \
	}
#define J2dTraceLn1(level, string, arg1) { \
	    J2dTraceImpl(level, JNI_TRUE, string, arg1); \
	    DTRACE_PRINTLN1(string, arg1); \
	}
#define J2dTraceLn2(level, string, arg1, arg2) { \
	    J2dTraceImpl(level, JNI_TRUE, string, arg1, arg2); \
	    DTRACE_PRINTLN2(string, arg1, arg2); \
	}
#define J2dTraceLn3(level, string, arg1, arg2, arg3) { \
	    J2dTraceImpl(level, JNI_TRUE, string, arg1, arg2, arg3); \
	    DTRACE_PRINTLN3(string, arg1, arg2, arg3); \
	}
#define J2dTraceLn4(level, string, arg1, arg2, arg3, arg4) { \
	    J2dTraceImpl(level, JNI_TRUE, string, arg1, arg2, arg3, arg4); \
	    DTRACE_PRINTLN4(string, arg1, arg2, arg3, arg4); \
	}
#define J2dTraceLn5(level, string, arg1, arg2, arg3, arg4, arg5) { \
	    J2dTraceImpl(level, JNI_TRUE, string, arg1, arg2, arg3, arg4, arg5); \
	    DTRACE_PRINTLN5(string, arg1, arg2, arg3, arg4, arg5); \
	}
#endif /* DEBUG */

#ifdef __cplusplus
};
#endif /* __cplusplus */

#endif /* _Included_Trace */
