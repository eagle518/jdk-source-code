/*
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef JQS_HPP
#define JQS_HPP

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string>

#include "os_defs.hpp"

/*
 * Definitions for 1Kb, 1Mb and 1Gb.
 */
#define K (1024)
#define M (K*1024)
#define G (M*1024)

/*
 * The name of the configuration file variable used to define the path to JRE.
 */
#define JAVA_HOME                           "JAVA_HOME"
/*
 * The relative path to JRE starting from the JQS binary.
 */
#define JAVA_HOME_RELATIVE_TO_JQS           ".."

/*
 * The default name of the JQS configuration file.
 */
#define DEFAULT_CONFIG_NAME                 JAVA_HOME_RELATIVE_TO_JQS "/lib/deploy/jqs/jqs.conf"

/*
 * Default values for JQS options.
 */
#define DEFAULT_INTERVAL                    30
#define DEFAULT_MEMORY_LIMIT                (32 * M)
#define DEFAULT_DISK_TIME_THRESHOLD         10.0
#define DEFAULT_PROCESSOR_TIME_THRESHOLD    10.0

/*
 * JQS service information.
 */
#define JQS_SERVICE_NAME                "JavaQuickStarterService"
#define JQS_SERVICE_DISPLAY_NAME        "Java Quick Starter"
#define JQS_SERVICE_DESCRIPTION         "Prefetches JRE files for faster startup of Java applets and applications"
#define JQS_SERVICE_VERSION             "1.02"

#ifndef J2SE_BUILD_ID
#  error J2SE_BUILD_ID not defined
#endif

#define JQS_JAVA_VERSION                "Java(TM) SE Runtime Environment (build " J2SE_BUILD_ID ")"


/*
 * Checks if any asynchronous events occurred (such as terminate or pause), 
 * and throws proper AsyncEventException successor.
 * This macro is intended to be used inside time consuming tasks.
 */
#define CHECK_ASYNC_EVENTS()           \
            if (asyncEventOccured) {   \
                processAsyncEvents();  \
            }

/*
 * Asynchronous events exceptions.
 */
class AsyncEventException {};
class TerminationEventException : public AsyncEventException {};
class PauseEventException : public AsyncEventException {};

/*
 * Defines whether the asynchronous event occured.
 */
extern volatile bool asyncEventOccured;

/*
 * Determines kind of asynchronous event and throws proper exception.
 */
extern void processAsyncEvents();


/*
 * The structure for holding performance counter thresholds for disk I/O usage
 * counter and CPU usage counters respectively.
 */
struct PerfCountersThreshold {
    double diskTime;
    double processorTime;
};


/*
 * These macros define whether the "libpath" or "libdepend" options are 
 * supported by JQS.
 */
#define OPTION_LIBPATH_SUPPORTED        false
#define OPTION_LIBDEPEND_SUPPORTED      false

#endif
