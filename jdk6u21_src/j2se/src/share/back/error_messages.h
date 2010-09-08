/*
 * @(#)error_messages.h	1.14 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef JDWP_ERROR_MESSAGES_H
#define JDWP_ERROR_MESSAGES_H

/* It is assumed that ALL strings are UTF-8 safe on entry */
#define TTY_MESSAGE(args) ( tty_message args )
#define ERROR_MESSAGE(args) ( \
                LOG_ERROR(args), \
                error_message args )

void print_message(FILE *fp, const char *prefix,  const char *suffix,
                   const char *format, ...);
void error_message(const char *, ...);
void tty_message(const char *, ...);
void jdiAssertionFailed(char *fileName, int lineNumber, char *msg);

const char * jvmtiErrorText(jvmtiError);
const char * eventText(int);
const char * jdwpErrorText(jdwpError);

#define EXIT_ERROR(error,msg) \
        { \
                print_message(stderr, "JDWP exit error ", "\n", \
                        "%s(%d): %s [%s:%d]", \
                        jvmtiErrorText((jvmtiError)error), error, (msg==NULL?"":msg), \
                        __FILE__, __LINE__); \
                debugInit_exit((jvmtiError)error, msg); \
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


