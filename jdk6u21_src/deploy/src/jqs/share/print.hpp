/*
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef PRINT_HPP
#define PRINT_HPP

#include <string>

/*
 * Custom assert implementation, reports failure and exits.
 */
#define assert(cond)    \
    if (!(cond)) { \
        jqs_error("Assertion failed at %s:%d\n", __FILE__, __LINE__); \
        jqs_exit(1); \
    }

/*
 * Opens given file for messages logging.
 */
extern void openLogFile (const std::string& name);

/*
 * Reports error to log file and to system event log. 
 * The format is the same as for printf.
 */
extern void jqs_error(const char* format, ...);
/*
 * Reports warning to system event log if current verbosity level is non-zero
 * and to log file. 
 * The format is the same as for printf.
 */
extern void jqs_warn(const char* format, ...);
/*
 * If current verbosity is equal or greater than given level,
 * reports info message to log file. 
 * The format is the same as for printf.
 */
extern void jqs_info (unsigned int level, const char* format, ...);

/*
 * Initializes facility to print to system event log.
 */
extern void addEventSource();
/*
 * Uninitializes facility to print to system event log.
 */
extern void removeEventSource();

/*
 * Current verbosity level.
 */
extern unsigned int verbose;
/*
 * The service mode flag defines whether to report messages to system event
 * log or to console window.
 */
extern bool service_mode;

#endif
