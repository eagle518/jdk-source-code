/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-*/
/*
 * @(#)plugin_defs.h	1.16 02/02/14
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


/* Maximum size of the env table */
#define MAX_ENVS 100

/* A grouping of some global definitions */
#define PLUGIN_INSTANCE_COUNT 100
/* These tracing files are activated by JAVA_PLUGIN_TRACE being a defined
   environment variable */
/* Parent tracing file */
#define PARENT_TRACE_FILE(VERSION) "/tmp/plugin_parent"VERSION"_"
/* File used after the fork and before the exec for tracing */
#define FORK_TRACE_FILE(VERSION) "/tmp/child_fork"VERSION".trace"
/* File used in java_vm.c i.e. after the exec, before java startup */
#define EXEC_TRACE_FILE(VERSION) "/tmp/child_java_vm"VERSION"_"


/* This is used for localization in the various calls to dggettext.
   Right now, none of these calls are particularly important (most are
   internal errors which we want to see in English when they are
   reported back as bugs) and none are localized. In the long run, it
   would be preferrable to move all these calls into Java so that the
   messages are common across platforms
   */
#define JAVA_PLUGIN_DOMAIN "sunw_java_plugin"

#define JAVA_PLUGIN_SOCKFILE "/tmp/jpsock"
#define INITIAL_ATHREAD_PORT 13000

/* Arrange these file descriptors so that the ones that are common to
   4.0 and 5.0 coincide i.e. command = 11 and worker = 12 */

#define MOZ5_STARTSAFEFD 10
#define MOZ5_ENDSAFEFD   16

/*
 * When we exec our child process we use the following file
 * descriptor number as the way of passing the communication
 * pipe through to the Java process. 
 *
 * NOTE: These must be kept consistent with the values in Plugin.java
 */
#define SPONT_FD   10
#define COMMAND_FD 11
#define WORK_FD    12
#define PRINT_FD   13

#define UNUSED(x) x=x

/*
 *	Bug 4290997: Solaris plug-in source uses non-Posix OS functions
 *		
 *	define DGETTEXT macro for plugin routines.
 *	
 *
 */

#ifdef __sun

#include <libintl.h>

#define DGETTEXT dgettext

#else
/*
 *	Following converts to asci messages if
 *	dgettext is not available.
 *	PORTER: Should define appropriate function if available.
 *
 */

#define DGETTEXT( _dom_ , _msgid_ ) _msgid_

#endif

