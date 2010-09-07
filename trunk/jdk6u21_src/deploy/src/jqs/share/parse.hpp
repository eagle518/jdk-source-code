/*
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef PARSE_HPP
#define PARSE_HPP

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include <string>
#include "jqs.hpp"


/*
 * A set of commands and options supported by JQS.
 */ 
#define CMD_LOAD                    "load"
#define CMD_REFRESH                 "refresh"
#define CMD_LOADLIB                 "loadlib"
#define CMD_REFRESHLIB              "refreshlib"
#define CMD_SET                     "set"
#define CMD_UNSET                   "unset"
#define CMD_REFRESHDIR              "refreshdir"
#define CMD_PROFILE                 "profile"
#define CMD_OPTION                  "option"

#define OPTION_LOCKED               "locked"
#define OPTION_MAPPED               "mapped"
#define OPTION_PAGESIZE             "pagesize"
#define OPTION_BUFSIZE              "bufsize"
#define OPTION_LIBPATH              "libpath"
#define OPTION_LIBDEPEND            "libdepend"
#define OPTION_RECURSIVE            "recursive"

#define OPTION_INTERVAL                             "interval"
#define OPTION_MEMORY_LIMIT                         "memory_limit"
#define OPTION_BOOT_DISK_TIME_THRESHOLD             "boot_idle_disk_io_usage"
#define OPTION_BOOT_PROCESSOR_TIME_THRESHOLD        "boot_idle_cpu_usage"
#define OPTION_REFRESH_DISK_TIME_THRESHOLD          "idle_disk_io_usage"
#define OPTION_REFRESH_PROCESSOR_TIME_THRESHOLD     "idle_cpu_usage"


/*
 * A set of commands line arguments supported by JQS.
 */ 
#define CMDLINE_ARG_HELP             "-help"
#define CMDLINE_ARG_REGISTER         "-register"
#define CMDLINE_ARG_UNREGISTER       "-unregister"
#define CMDLINE_ARG_SERVICE          "-service"
#define CMDLINE_ARG_RUN              "-run"
#define CMDLINE_ARG_PAUSE            "-pause"
#define CMDLINE_ARG_RESUME           "-resume"
#define CMDLINE_ARG_ENABLE           "-enable"
#define CMDLINE_ARG_DISABLE          "-disable"
#define CMDLINE_ARG_VERSION          "-version"
#define CMDLINE_ARG_CONFIG           "-config"
#define CMDLINE_ARG_PROFILE          "-profile"
#define CMDLINE_ARG_VERBOSE          "-verbose"
#define CMDLINE_ARG_TIMING           "-timing"
#define CMDLINE_ARG_LOGFILE          "-logfile"
#define CMDLINE_ARG_DUMPCONFIG       "-dumpconfig"

/*
 * Terminators used for identifying variables in file names.
 */
#define OPEN_BRACE_C '{'
#define CLOSE_BRACE_C '}'
#define DOLLAR_SIGN_C '$'
#define OPEN_BRACE_S "{"
#define CLOSE_BRACE_S "}"
#define DOLLAR_SIGN_S "$"

/*
 * Comment char.
 */
#define COMMENT_C '#'
#define COMMENT_S "#"

/*
 * File name suffix which is appended to a filtered JQS control file.
 */
#define FILTERED_FILE_NAME_SUFFIX   ".filtered"


/*
 * Sets default value for a configuration file variable.
 */
extern void setConfigVariable(const std::string& name, const std::string& value);

/*
 * Parses given JQS configuration file and fills g_QSEntries with quick starter
 * entry objects.
 * If the doFilter flag is set, the function generates a filtered configuration 
 * file leaving entries only for those files which are present on current system.
 */
extern void parseConfigFile(const std::string& filename, bool doFilter = false);

/*
 * Parses given JQS profile file and attaches profile information to proper 
 * quick starter entry object added to g_QSEntries.
 * If the doFilter flag is set, the function generates a filtered profile file 
 * leaving entries only for those files which are present in both JQS 
 * configuration file and on current system.
 */
extern void parseProfile(const std::string& profileFileName, bool doFilter = false);

#endif
