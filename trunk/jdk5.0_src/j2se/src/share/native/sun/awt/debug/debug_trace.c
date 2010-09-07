/*
 * @(#)debug_trace.c	1.13 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "debug_util.h"
#include "sun_awt_DebugHelperImpl.h"

static void DTrace_PrintStdErr(const char *msg);

#if defined(DEBUG)
enum {
    MAX_TRACES = 200, 		/* max number of defined trace points allowed */
    MAX_TRACE_BUFFER = 512,    	/* maximum size of a given trace output */
    MAX_LINE = 100000,		/* reasonable upper limit on line number in source file */
    MAX_ARGC = 8		/* maximum number of arguments to print functions */
};

typedef enum dtrace_scope {
    DTRACE_FILE,
    DTRACE_LINE
} dtrace_scope;
    
typedef struct dtrace_info {
    char 		file[FILENAME_MAX+1];
    int			line;
    int			enabled;
    dtrace_scope	scope;
} dtrace_info, * p_dtrace_info;

static dtrace_info	DTraceInfo[MAX_TRACES];
static char		DTraceBuffer[MAX_TRACE_BUFFER*2+1]; /* double the buffer size to catch overruns */
static dmutex_t 	DTraceMutex = NULL;
static dbool_t		GlobalTracingEnabled = FALSE;
static int 		NumTraces = 0;

static DTRACE_OUTPUT_CALLBACK	PfnTraceCallback = DTrace_PrintStdErr;

static p_dtrace_info DTrace_GetInfo(dtrace_id tid) {
    DASSERT(tid < MAX_TRACES);
    return &DTraceInfo[tid];
}

static dtrace_id DTrace_CreateTraceId(const char * file, int line, dtrace_scope scope) {
    dtrace_id		tid = NumTraces++;
    p_dtrace_info	info = &DTraceInfo[tid];
    DASSERT(NumTraces < MAX_TRACES);
    
    strcpy(info->file, file);
    info->line = line;
    info->enabled = FALSE;
    info->scope = scope;
    return tid;
}

/*
 * Compares the trailing characters in a filename to see if they match
 * e.g. "src\win32\foobar.c" and "foobar.c" would be considered equal
 * but "src\win32\foo.c" and "src\win32\bar.c" would not.
 */
static dbool_t FileNamesSame(const char * fileOne, const char * fileTwo) {
    size_t	lengthOne = strlen(fileOne);
    size_t	lengthTwo = strlen(fileTwo);
    size_t	numCompareChars;
    dbool_t	tailsEqual;

    if (fileOne == fileTwo) {
	return TRUE;
    } else if (fileOne == NULL || fileTwo == NULL) {
	return FALSE;
    }
    /* compare the tail ends of the strings for equality */
    numCompareChars = lengthOne < lengthTwo ? lengthOne : lengthTwo;
    tailsEqual = strcmp(fileOne + lengthOne - numCompareChars,
			fileTwo + lengthTwo - numCompareChars) == 0;
    return tailsEqual;
}

/*
 * Finds the trace id for a given file/line location or creates one
 * if it doesn't exist
 */
static dtrace_id DTrace_GetTraceId(const char * file, int line, dtrace_scope scope) {
    dtrace_id		tid;
    p_dtrace_info	info;
    
    /* check to see if the trace point has already been created */
    for ( tid = 0; tid < NumTraces; tid++ ) {
	info = DTrace_GetInfo(tid);
	if ( info->scope == scope ) {
	    dbool_t	sameFile = FileNamesSame(file, info->file);
	    dbool_t	sameLine = info->line == line;
	    
	    if ( (info->scope == DTRACE_FILE && sameFile) ||
		 (info->scope == DTRACE_LINE && sameFile && sameLine) ) {
		goto Exit;
	    }
	}
    }
    
    /* trace point wasn't created, so force it's creation */
    tid = DTrace_CreateTraceId(file, line, scope);
Exit:
    return tid;
}


static dbool_t DTrace_IsEnabledAt(dtrace_id * pfileid, dtrace_id * plineid, const char * file, int line) {
    DASSERT(pfileid != NULL && plineid != NULL);

    if ( *pfileid == UNDEFINED_TRACE_ID ) {
    /* first time calling the trace for this file, so obtain a trace id */
	 *pfileid = DTrace_GetTraceId(file, -1, DTRACE_FILE);
    }
    if ( *plineid == UNDEFINED_TRACE_ID ) {
    /* first time calling the trace for this line, so obtain a trace id */
	 *plineid = DTrace_GetTraceId(file, line, DTRACE_LINE);
    }
    
    return GlobalTracingEnabled || DTraceInfo[*pfileid].enabled || DTraceInfo[*plineid].enabled;
}

/*
 * Initialize trace functionality. This MUST BE CALLED before any
 * tracing function is called.
 */
void DTrace_Initialize() {
    DTraceMutex = DMutex_Create();
}

/*
 * Cleans up tracing system. Should be called when tracing functionality
 * is no longer needed.
 */
void DTrace_Shutdown() {
    DMutex_Destroy(DTraceMutex);
}

void DTrace_DisableMutex() {
    DTraceMutex = NULL;
}

/*
 * Enable tracing for all modules.
 */
void DTrace_EnableAll(dbool_t enabled) {
    DMutex_Enter(DTraceMutex);
    GlobalTracingEnabled = enabled;
    DMutex_Exit(DTraceMutex);
}

/*
 * Enable tracing for a specific module. Filename may
 * be fully or partially qualified.
 * e.g. awt_Component.cpp 
 *		or 
 *	src\win32\native\sun\windows\awt_Component.cpp
 */
void DTrace_EnableFile(const char * file, dbool_t enabled) {
    dtrace_id tid;
    p_dtrace_info info;
    
    DASSERT(file != NULL);
    DMutex_Enter(DTraceMutex);
    tid = DTrace_GetTraceId(file, -1, DTRACE_FILE);
    info = DTrace_GetInfo(tid);
    info->enabled = enabled;
    DMutex_Exit(DTraceMutex);
}

/*
 * Enable tracing for a specific line in a specific module.
 * See comments above regarding filename argument.
 */
void DTrace_EnableLine(const char * file, int line, dbool_t enabled) {
    dtrace_id tid;
    p_dtrace_info info;
    
    DASSERT(file != NULL && (line > 0 && line < MAX_LINE));
    DMutex_Enter(DTraceMutex);
    tid = DTrace_GetTraceId(file, line, DTRACE_LINE);
    info = DTrace_GetInfo(tid);
    info->enabled = enabled;
    DMutex_Exit(DTraceMutex);
}

static void DTrace_ClientPrint(const char * msg) {
    DASSERT(msg != NULL && PfnTraceCallback != NULL);
    (*PfnTraceCallback)(msg);
}

/*
 * Print implementation for the use of client defined trace macros. Unsynchronized so it must
 * be used from within a DTRACE_PRINT_CALLBACK function.
 */
void DTrace_VPrintImpl(const char * fmt, va_list arglist) {
    DASSERT(fmt != NULL);

    /* format the trace message */
    vsprintf(DTraceBuffer, fmt, arglist);
    /* not a real great overflow check (memory would already be hammered) but better than nothing */
    DASSERT(strlen(DTraceBuffer) < MAX_TRACE_BUFFER); 
    /* output the trace message */
    DTrace_ClientPrint(DTraceBuffer);
}

/*
 * Print implementation for the use of client defined trace macros. Unsynchronized so it must
 * be used from within a DTRACE_PRINT_CALLBACK function.
 */
void DTrace_PrintImpl(const char * fmt, ...) {
    va_list	arglist;

    va_start(arglist, fmt);
    DTrace_VPrintImpl(fmt, arglist);
    va_end(arglist);
}

/*
 * Called via DTRACE_PRINT macro. Outputs printf style formatted text.
 */
void DTrace_VPrint( const char * file, int line, int argc, const char * fmt, va_list arglist ) {
    DASSERT(fmt != NULL);
    DTrace_VPrintImpl(fmt, arglist);
}

/*
 * Called via DTRACE_PRINTLN macro. Outputs printf style formatted text with an automatic newline.
 */
void DTrace_VPrintln( const char * file, int line, int argc, const char * fmt, va_list arglist ) {
    DTrace_VPrintImpl(fmt, arglist);
    DTrace_PrintImpl("\n");
}

/*
 * Called via DTRACE_ macros. If tracing is enabled at the given location, it enters 
 * the trace mutex and invokes the callback function to output the trace.
 */
void DTrace_PrintFunction( DTRACE_PRINT_CALLBACK pfn, dtrace_id * pFileTraceId, dtrace_id * pLineTraceId, 
			   const char * file, int line,
			   int argc, const char * fmt, ... ) {
    va_list	arglist;
   
    DASSERT(file != NULL);
    DASSERT(line > 0 && line < MAX_LINE); 
    DASSERT(argc <= MAX_ARGC);
    DASSERT(fmt != NULL);
    
    DMutex_Enter(DTraceMutex);
    if ( DTrace_IsEnabledAt(pFileTraceId, pLineTraceId, file, line) ) {
	va_start(arglist, fmt);
	(*pfn)(file, line, argc, fmt, arglist);
	va_end(arglist);
    }
    DMutex_Exit(DTraceMutex);
}

/*
 * Sets a callback function to be used to output 
 * trace statements.
 */
void DTrace_SetOutputCallback(DTRACE_OUTPUT_CALLBACK pfn) {
    DASSERT(pfn != NULL);

    DMutex_Enter(DTraceMutex);
    PfnTraceCallback = pfn;
    DMutex_Exit(DTraceMutex);
}

#endif /* DEBUG */

/**********************************************************************************
 * Support for Java tracing in release or debug mode builds
 */

static void DTrace_PrintStdErr(const char *msg) {	
    fprintf(stderr, "%s", msg);
    fflush(stderr);
}

static void DTrace_JavaPrint(const char * msg) {
#if defined(DEBUG)    
    DMutex_Enter(DTraceMutex);
    DTrace_ClientPrint(msg);
    DMutex_Exit(DTraceMutex);
#else
    DTrace_PrintStdErr(msg);
#endif
}

static void DTrace_JavaPrintln(const char * msg) {
#if defined(DEBUG)
    DMutex_Enter(DTraceMutex);
    DTrace_ClientPrint(msg);
    DTrace_ClientPrint("\n");
    DMutex_Exit(DTraceMutex);
#else
    DTrace_PrintStdErr(msg);
    DTrace_PrintStdErr("\n");
#endif
}

/*********************************************************************************
 * Native method implementations. Java print trace calls are functional in 
 * release builds, but functions to enable/disable native tracing are not.
 */

/* Implementation of DebugHelperImpl.printImpl */
JNIEXPORT void JNICALL
Java_sun_awt_DebugHelperImpl_printImpl(JNIEnv *env, jclass cls, jstring msg) {
    const char *	cstr;
    
    cstr = JNU_GetStringPlatformChars(env, msg, NULL);
    if ( cstr == NULL ) {
	return;
    }
    DTrace_JavaPrint(cstr);
    JNU_ReleaseStringPlatformChars(env, msg, cstr);
}

/* Implementation of DebugHelperImpl.printlnImpl */
JNIEXPORT void JNICALL
Java_sun_awt_DebugHelperImpl_printlnImpl(JNIEnv *env, jclass cls, jstring msg) {
    const char *	cstr;
    cstr = JNU_GetStringPlatformChars(env, msg, NULL);
    if ( cstr == NULL ) {
	return;
    }
    DTrace_JavaPrintln(cstr);
    JNU_ReleaseStringPlatformChars(env, msg, cstr);
}

/* Implementation of DebugHelperImpl.setCTracingOn*/
JNIEXPORT void JNICALL
Java_sun_awt_DebugHelperImpl_setCTracingOn__Z(JNIEnv *env, jobject self, jboolean enabled) {
#if defined(DEBUG)    
    DTrace_EnableAll(enabled == JNI_TRUE);
#endif
}

/* Implementation of DebugHelperImpl.setCTracingOn*/
JNIEXPORT void JNICALL
Java_sun_awt_DebugHelperImpl_setCTracingOn__ZLjava_lang_String_2(
    JNIEnv *env,
    jobject self,
    jboolean enabled,
    jstring file ) {
#if defined(DEBUG)    
    const char *	cfile;
    cfile = JNU_GetStringPlatformChars(env, file, NULL);
    if ( cfile == NULL ) {
	return;
    }
    DTrace_EnableFile(cfile, enabled == JNI_TRUE);
    JNU_ReleaseStringPlatformChars(env, file, cfile);
#endif
}

/* Implementation of DebugHelperImpl.setCTracingOn*/
JNIEXPORT void JNICALL
Java_sun_awt_DebugHelperImpl_setCTracingOn__ZLjava_lang_String_2I(
    JNIEnv *env,
    jobject self, 
    jboolean enabled,
    jstring file,
    jint line ) {
#if defined(DEBUG)    
    const char *	cfile;
    cfile = JNU_GetStringPlatformChars(env, file, NULL);
    if ( cfile == NULL ) {
	return;
    }
    DTrace_EnableLine(cfile, line, enabled == JNI_TRUE);
    JNU_ReleaseStringPlatformChars(env, file, cfile);
#endif
}
