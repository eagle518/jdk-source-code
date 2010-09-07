/*
 * @(#)error_messages.c	1.13 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/* Error message handling functions. */

#include <stdarg.h>
#include <errno.h>

#include "util.h"
#include "proc_md.h"
#include "typedefs.h"

static MUTEX_T my_mutex = MUTEX_INIT;

static char location_stamp[256];

/* Get basename of file. */
static const char *
file_basename(const char *file)
{
    char *p1;
    char *p2;
    
    if ( file==NULL )
	return "unknown";
    p1 = strrchr(file, '\\');
    p2 = strrchr(file, '/');
    p1 = ((p1 > p2) ? p1 : p2);
    if (p1 != NULL) {
        file = p1 + 1;
    }
    return file;
}

/* Fill in location_stamp[] with location where error message came from. */
void
error_message_begin(const char *file, int line)
{
	
    MUTEX_LOCK(my_mutex); /* Unlocked in error_message_end() */
    
    location_stamp[0] = 0;
    if ( line <= 0 || file==NULL )
	return;
    (void)snprintf(location_stamp, sizeof(location_stamp), 
		" [\"%s\",L%d]", file_basename(file), line);
    location_stamp[sizeof(location_stamp)-1] = 0;
    LOG_MISC(("ERROR at %s", location_stamp));
}

/* Construct complete error message and send it to stderr */
void
error_message_end(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    (void)fprintf(stderr, "ERROR: ");
    (void)vfprintf(stderr, format, ap);
    (void)fprintf(stderr, "%s\n", location_stamp);
    va_end(ap);
    location_stamp[0] = 0;
    
    MUTEX_UNLOCK(my_mutex); /* Locked in error_message_begin() */

    if ( gdata->doerrorexit ) {
	EXIT_ERROR(JVMTI_ERROR_INTERNAL,"Requested errorexit=y exit()");
    }
}

/* Print plain message to stdout. */
void
tty_message(const char *format, ...)
{
    va_list ap;
    MUTEX_LOCK(my_mutex);
    va_start(ap, format);
    (void)vfprintf(stdout,format, ap);
    (void)fprintf(stdout, "\n");
    (void)fflush(stdout);
    va_end(ap);
    MUTEX_UNLOCK(my_mutex);
}

/* Print assertion error message to stderr. */
void 
jdiAssertionFailed(char *fileName, int lineNumber, char *msg)
{
    LOG_MISC(("ASSERT FAILED: %s : %d - %s\n", fileName, lineNumber, msg));
    (void)fprintf(stderr, "ASSERT FAILED: %s : %d - %s\n", fileName, lineNumber, msg);
    if (gdata && gdata->assertFatal) {
        EXIT_ERROR(JVMTI_ERROR_INTERNAL,"Assertion Failed");
    }
}

/* Macro for case on switch, returns string for name. */
#define CASE_RETURN_TEXT(name) case name: return #name;

/* Mapping of JVMTI errors to their name */
const char *
jvmtiErrorText(jvmtiError error)
{
    switch (error) {
	CASE_RETURN_TEXT(JVMTI_ERROR_NONE)
	CASE_RETURN_TEXT(JVMTI_ERROR_INVALID_THREAD)
	CASE_RETURN_TEXT(JVMTI_ERROR_INVALID_THREAD_GROUP)
	CASE_RETURN_TEXT(JVMTI_ERROR_INVALID_PRIORITY)
	CASE_RETURN_TEXT(JVMTI_ERROR_THREAD_NOT_SUSPENDED)
	CASE_RETURN_TEXT(JVMTI_ERROR_THREAD_SUSPENDED)
	CASE_RETURN_TEXT(JVMTI_ERROR_THREAD_NOT_ALIVE)
	CASE_RETURN_TEXT(JVMTI_ERROR_INVALID_OBJECT)
	CASE_RETURN_TEXT(JVMTI_ERROR_INVALID_CLASS)
	CASE_RETURN_TEXT(JVMTI_ERROR_CLASS_NOT_PREPARED)
	CASE_RETURN_TEXT(JVMTI_ERROR_INVALID_METHODID)
	CASE_RETURN_TEXT(JVMTI_ERROR_INVALID_LOCATION)
	CASE_RETURN_TEXT(JVMTI_ERROR_INVALID_FIELDID)
	CASE_RETURN_TEXT(JVMTI_ERROR_NO_MORE_FRAMES)
	CASE_RETURN_TEXT(JVMTI_ERROR_OPAQUE_FRAME)
	CASE_RETURN_TEXT(JVMTI_ERROR_TYPE_MISMATCH)
	CASE_RETURN_TEXT(JVMTI_ERROR_INVALID_SLOT)
	CASE_RETURN_TEXT(JVMTI_ERROR_DUPLICATE)
	CASE_RETURN_TEXT(JVMTI_ERROR_NOT_FOUND)
	CASE_RETURN_TEXT(JVMTI_ERROR_INVALID_MONITOR)
	CASE_RETURN_TEXT(JVMTI_ERROR_NOT_MONITOR_OWNER)
	CASE_RETURN_TEXT(JVMTI_ERROR_INTERRUPT)
        CASE_RETURN_TEXT(JVMTI_ERROR_INVALID_CLASS_FORMAT)
        CASE_RETURN_TEXT(JVMTI_ERROR_CIRCULAR_CLASS_DEFINITION)
        CASE_RETURN_TEXT(JVMTI_ERROR_FAILS_VERIFICATION)
        CASE_RETURN_TEXT(JVMTI_ERROR_UNSUPPORTED_REDEFINITION_METHOD_ADDED)
        CASE_RETURN_TEXT(JVMTI_ERROR_UNSUPPORTED_REDEFINITION_SCHEMA_CHANGED)
        CASE_RETURN_TEXT(JVMTI_ERROR_INVALID_TYPESTATE)
        CASE_RETURN_TEXT(JVMTI_ERROR_UNSUPPORTED_REDEFINITION_HIERARCHY_CHANGED)
        CASE_RETURN_TEXT(JVMTI_ERROR_UNSUPPORTED_REDEFINITION_METHOD_DELETED)
        CASE_RETURN_TEXT(JVMTI_ERROR_UNSUPPORTED_VERSION)
        CASE_RETURN_TEXT(JVMTI_ERROR_NAMES_DONT_MATCH)
        CASE_RETURN_TEXT(JVMTI_ERROR_UNSUPPORTED_REDEFINITION_CLASS_MODIFIERS_CHANGED)
        CASE_RETURN_TEXT(JVMTI_ERROR_UNSUPPORTED_REDEFINITION_METHOD_MODIFIERS_CHANGED)
        CASE_RETURN_TEXT(JVMTI_ERROR_NOT_AVAILABLE)
        CASE_RETURN_TEXT(JVMTI_ERROR_MUST_POSSESS_CAPABILITY)
        CASE_RETURN_TEXT(JVMTI_ERROR_NULL_POINTER)
        CASE_RETURN_TEXT(JVMTI_ERROR_ABSENT_INFORMATION)
        CASE_RETURN_TEXT(JVMTI_ERROR_INVALID_EVENT_TYPE)
	CASE_RETURN_TEXT(JVMTI_ERROR_ILLEGAL_ARGUMENT)
	CASE_RETURN_TEXT(JVMTI_ERROR_OUT_OF_MEMORY)
	CASE_RETURN_TEXT(JVMTI_ERROR_ACCESS_DENIED)
	CASE_RETURN_TEXT(JVMTI_ERROR_WRONG_PHASE)
	CASE_RETURN_TEXT(JVMTI_ERROR_INTERNAL)
	CASE_RETURN_TEXT(JVMTI_ERROR_UNATTACHED_THREAD)
	CASE_RETURN_TEXT(JVMTI_ERROR_INVALID_ENVIRONMENT)
	default: return  "JVMTI_ERROR_unknown";
    }
}

const char *
eventText(int i)
{
    switch ( i ) {
        CASE_RETURN_TEXT(EI_SINGLE_STEP)
        CASE_RETURN_TEXT(EI_BREAKPOINT)
        CASE_RETURN_TEXT(EI_FRAME_POP)
        CASE_RETURN_TEXT(EI_EXCEPTION)
        CASE_RETURN_TEXT(EI_THREAD_START)
        CASE_RETURN_TEXT(EI_THREAD_END)
        CASE_RETURN_TEXT(EI_CLASS_PREPARE)
        CASE_RETURN_TEXT(EI_CLASS_LOAD)
        CASE_RETURN_TEXT(EI_FIELD_ACCESS)
        CASE_RETURN_TEXT(EI_FIELD_MODIFICATION)
        CASE_RETURN_TEXT(EI_EXCEPTION_CATCH)
        CASE_RETURN_TEXT(EI_METHOD_ENTRY)
        CASE_RETURN_TEXT(EI_METHOD_EXIT)
        CASE_RETURN_TEXT(EI_VM_INIT)
        CASE_RETURN_TEXT(EI_VM_DEATH)
        CASE_RETURN_TEXT(EI_GC_FINISH)
        default: return "EVENT_unknown";
    }
}

/* Macro for case on switch, returns string for name. */
#define CASE_RETURN_JDWP_ERROR_TEXT(name) case JDWP_ERROR(name): return #name;

const char *
jdwpErrorText(jdwpError serror)
{
    switch ( serror ) {
	CASE_RETURN_JDWP_ERROR_TEXT(NONE)
	CASE_RETURN_JDWP_ERROR_TEXT(INVALID_THREAD)
        CASE_RETURN_JDWP_ERROR_TEXT(INVALID_THREAD_GROUP)
        CASE_RETURN_JDWP_ERROR_TEXT(INVALID_PRIORITY)
        CASE_RETURN_JDWP_ERROR_TEXT(THREAD_NOT_SUSPENDED)
        CASE_RETURN_JDWP_ERROR_TEXT(THREAD_SUSPENDED)
        CASE_RETURN_JDWP_ERROR_TEXT(INVALID_OBJECT)
        CASE_RETURN_JDWP_ERROR_TEXT(INVALID_CLASS)
        CASE_RETURN_JDWP_ERROR_TEXT(CLASS_NOT_PREPARED)
        CASE_RETURN_JDWP_ERROR_TEXT(INVALID_METHODID)
        CASE_RETURN_JDWP_ERROR_TEXT(INVALID_LOCATION)
        CASE_RETURN_JDWP_ERROR_TEXT(INVALID_FIELDID)
        CASE_RETURN_JDWP_ERROR_TEXT(INVALID_FRAMEID)
        CASE_RETURN_JDWP_ERROR_TEXT(NO_MORE_FRAMES)
        CASE_RETURN_JDWP_ERROR_TEXT(OPAQUE_FRAME)
        CASE_RETURN_JDWP_ERROR_TEXT(NOT_CURRENT_FRAME)
        CASE_RETURN_JDWP_ERROR_TEXT(TYPE_MISMATCH)
        CASE_RETURN_JDWP_ERROR_TEXT(INVALID_SLOT)
        CASE_RETURN_JDWP_ERROR_TEXT(DUPLICATE)
        CASE_RETURN_JDWP_ERROR_TEXT(NOT_FOUND)
        CASE_RETURN_JDWP_ERROR_TEXT(INVALID_MONITOR)
        CASE_RETURN_JDWP_ERROR_TEXT(NOT_MONITOR_OWNER)
        CASE_RETURN_JDWP_ERROR_TEXT(INTERRUPT)
        CASE_RETURN_JDWP_ERROR_TEXT(INVALID_CLASS_FORMAT)
        CASE_RETURN_JDWP_ERROR_TEXT(CIRCULAR_CLASS_DEFINITION)
        CASE_RETURN_JDWP_ERROR_TEXT(FAILS_VERIFICATION)
        CASE_RETURN_JDWP_ERROR_TEXT(ADD_METHOD_NOT_IMPLEMENTED)
        CASE_RETURN_JDWP_ERROR_TEXT(SCHEMA_CHANGE_NOT_IMPLEMENTED)
        CASE_RETURN_JDWP_ERROR_TEXT(INVALID_TYPESTATE)
        CASE_RETURN_JDWP_ERROR_TEXT(HIERARCHY_CHANGE_NOT_IMPLEMENTED)
        CASE_RETURN_JDWP_ERROR_TEXT(DELETE_METHOD_NOT_IMPLEMENTED)
        CASE_RETURN_JDWP_ERROR_TEXT(UNSUPPORTED_VERSION)
        CASE_RETURN_JDWP_ERROR_TEXT(NAMES_DONT_MATCH)
        CASE_RETURN_JDWP_ERROR_TEXT(CLASS_MODIFIERS_CHANGE_NOT_IMPLEMENTED)
        CASE_RETURN_JDWP_ERROR_TEXT(METHOD_MODIFIERS_CHANGE_NOT_IMPLEMENTED)
        CASE_RETURN_JDWP_ERROR_TEXT(NOT_IMPLEMENTED)
        CASE_RETURN_JDWP_ERROR_TEXT(NULL_POINTER)
        CASE_RETURN_JDWP_ERROR_TEXT(ABSENT_INFORMATION)
        CASE_RETURN_JDWP_ERROR_TEXT(INVALID_EVENT_TYPE)
        CASE_RETURN_JDWP_ERROR_TEXT(ILLEGAL_ARGUMENT)
        CASE_RETURN_JDWP_ERROR_TEXT(OUT_OF_MEMORY)
        CASE_RETURN_JDWP_ERROR_TEXT(ACCESS_DENIED)
        CASE_RETURN_JDWP_ERROR_TEXT(VM_DEAD)
        CASE_RETURN_JDWP_ERROR_TEXT(INTERNAL)
        CASE_RETURN_JDWP_ERROR_TEXT(UNATTACHED_THREAD)
        CASE_RETURN_JDWP_ERROR_TEXT(INVALID_TAG)
        CASE_RETURN_JDWP_ERROR_TEXT(ALREADY_INVOKING)
        CASE_RETURN_JDWP_ERROR_TEXT(INVALID_INDEX)
        CASE_RETURN_JDWP_ERROR_TEXT(INVALID_LENGTH)
        CASE_RETURN_JDWP_ERROR_TEXT(INVALID_STRING)
        CASE_RETURN_JDWP_ERROR_TEXT(INVALID_CLASS_LOADER)
        CASE_RETURN_JDWP_ERROR_TEXT(INVALID_ARRAY)
        CASE_RETURN_JDWP_ERROR_TEXT(TRANSPORT_LOAD)
        CASE_RETURN_JDWP_ERROR_TEXT(TRANSPORT_INIT)
        CASE_RETURN_JDWP_ERROR_TEXT(NATIVE_METHOD)
        CASE_RETURN_JDWP_ERROR_TEXT(INVALID_COUNT)
        default: return "JDWP_ERROR_unknown";
    }
}

static int p = 1;

void
do_pause(void)
{
    THREAD_T tid = GET_THREAD_ID();
    PID_T pid    = GETPID();
    int timeleft = 600; /* 10 minutes max */
    int interval = 10;	/* 10 second message check */

    /*LINTED*/
    tty_message("DEBUGGING: JDWP pause for PID %d, THREAD %d (0x%x)", 
                    /*LINTED*/
		    (int)(intptr_t)pid, (int)(intptr_t)tid, (int)(intptr_t)tid);
    while ( p && timeleft > 0 ) {
	(void)sleep(interval); /* 'assign p = 0;' to get out of loop */
	timeleft -= interval;
    }
    if ( timeleft <= 0 ) {
	tty_message("DEBUGGING: JDWP pause got tired of waiting and gave up.");
    }
}


