/*
 * @(#)error_messages.h	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef JDWP_ERROR_MESSAGES_H
#define JDWP_ERROR_MESSAGES_H

#define TTY_MESSAGE(args) ( tty_message args )
#define ERROR_MESSAGE(args) ( \
		LOG_ERROR(args), \
		error_message_begin(__FILE__, __LINE__), \
	        error_message_end args )

void error_message_begin(const char *, int);
void error_message_end(const char *, ...);
void tty_message(const char *, ...);
void jdiAssertionFailed(char *fileName, int lineNumber, char *msg);

const char * jvmtiErrorText(jvmtiError);
const char * eventText(int);
const char * jdwpErrorText(jdwpError);

#define EXIT_ERROR(error,msg) \
	{ \
		(void)fprintf(stderr, "JDWP exit error %s(%d): %s", \
			jvmtiErrorText(error),error,(msg==NULL?"":msg)); \
		debugInit_exit(error,msg); \
	}

#define JDI_ASSERT(expression)  \
do {                            \
    if (gdata && gdata->assertOn && !(expression)) {            \
        jdiAssertionFailed(__FILE__, __LINE__, #expression); \
    }                                           \
} while (0)

#define JDI_ASSERT_MSG(expression, msg)  \
do {                            \
    if (gdata && gdata->assertOn && !(expression)) {            \
        jdiAssertionFailed(__FILE__, __LINE__, msg); \
    }                                           \
} while (0)

#define JDI_ASSERT_FAILED(msg)  \
   jdiAssertionFailed(__FILE__, __LINE__, msg)

void do_pause(void);

#endif

