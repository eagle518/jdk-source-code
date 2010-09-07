/*
 * @(#)log_messages.h	1.1 03/04/07
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef JDWP_LOG_MESSAGES_H
#define JDWP_LOG_MESSAGES_H

/* LOG: Must be called like:  LOG_category(("anything")) or LOG_category((format,args)) */

void setup_logging(const char *, unsigned, const char*, ... );
void finish_logging(int);

#define LOG_NULL ((void)0)

#ifdef JDWP_LOGGING
   
    
    #define _LOG(flavor,args) \
		(log_message_begin(flavor,__FILE__,__LINE__), \
		 log_message_end args)
    
    #define LOG_TEST(flag)  (gdata->log_flags & (flag))
    
    #define LOG_JVM(args)   \
	(LOG_TEST(JDWP_LOG_JVM)  ?_LOG("JVM",  args):LOG_NULL)
    #define LOG_JNI(args)   \
	(LOG_TEST(JDWP_LOG_JNI)  ?_LOG("JNI",  args):LOG_NULL)
    #define LOG_JVMTI(args) \
	(LOG_TEST(JDWP_LOG_JVMTI)?_LOG("JVMTI",args):LOG_NULL)
    #define LOG_MISC(args)  \
	(LOG_TEST(JDWP_LOG_MISC) ?_LOG("MISC", args):LOG_NULL)
    #define LOG_STEP(args)  \
	(LOG_TEST(JDWP_LOG_STEP) ?_LOG("STEP", args):LOG_NULL)
    #define LOG_LOC(args)   \
	(LOG_TEST(JDWP_LOG_LOC)  ?_LOG("LOC",  args):LOG_NULL)
    #define LOG_CB(args) \
	(LOG_TEST(JDWP_LOG_CB)?_LOG("CB",args):LOG_NULL)
    #define LOG_ERROR(args) \
	(LOG_TEST(JDWP_LOG_ERROR)?_LOG("ERROR",args):LOG_NULL)
    
    
    /* DO NOT USE THESE DIRECTLY */
    void log_message_begin(const char *, const char *, int);
    void log_message_end(const char *, ...);

#else
    
    #define LOG_TEST(flag)	0    
    
    #define LOG_JVM(args) 	LOG_NULL
    #define LOG_JNI(args) 	LOG_NULL
    #define LOG_JVMTI(args) 	LOG_NULL
    #define LOG_MISC(args) 	LOG_NULL
    #define LOG_STEP(args) 	LOG_NULL
    #define LOG_LOC(args) 	LOG_NULL
    #define LOG_CB(args) 	LOG_NULL
    #define LOG_ERROR(args) 	LOG_NULL

#endif

#define    JDWP_LOG_JVM 	0x00000001
#define    JDWP_LOG_JNI 	0x00000002
#define    JDWP_LOG_JVMTI 	0x00000004
#define    JDWP_LOG_MISC 	0x00000008
#define    JDWP_LOG_STEP 	0x00000010
#define    JDWP_LOG_LOC 	0x00000020
#define    JDWP_LOG_CB 	        0x00000040
#define    JDWP_LOG_ERROR       0x00000080
#define    JDWP_LOG_ALL		0xffffffff

#endif

