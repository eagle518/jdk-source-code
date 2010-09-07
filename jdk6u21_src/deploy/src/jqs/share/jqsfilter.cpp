/*
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>

#include "jqs.hpp"
#include "print.hpp"
#include "parse.hpp"
#include "timer.hpp"
#include "utils.hpp"
#include "os_utils.hpp"

using namespace std;

/*
 * Helper class, responsible for command line parsing and executing filtering.
 */
class Main {
    int             argc;
    const char**    argv;
    int             curParam;

    void printUsage ();
    void printVersion ();
    void parseCommandLine ();

    const char* stringParam ();
    unsigned int intParam ();

public:
    Main (int argc, const char** argv);
    void execute ();

};

Main::Main (int argc_, const char** argv_)
    : argc (argc_)
    , argv (argv_)
    , curParam (0)
{
    parseCommandLine ();
    execute ();
}

/*
 * Prints usage information to stdout and exits the process.
 */
void Main::printUsage () {
    printf(
        "Usage: jqsfilter " CMDLINE_ARG_CONFIG " <config>"
                          " [" CMDLINE_ARG_PROFILE " <profile>]\n"
        "\n"
        "Options:\n"
        "  " CMDLINE_ARG_CONFIG " <config>\tset JQS configuration file\n"
        "  " CMDLINE_ARG_PROFILE " <profile>\tset JQS profile file\n"
        "\n"
    );
    jqs_exit (1);
}

/*
 * Prints version information to stdout and exits the process.
 */
void Main::printVersion () {
    printf(JQS_JAVA_VERSION "\n"
           JQS_SERVICE_DISPLAY_NAME " " JQS_SERVICE_VERSION "\n");
    jqs_exit (0);
}

/*
 * Retrieves next parameter as a string value.
 */
const char* Main::stringParam () {
    ++curParam;
    if (curParam >= argc) {
        jqs_error("%s: string argument expected\n", argv[curParam - 1]);
        printUsage ();
    }
    return argv[curParam];
}

/*
 * Retrieves next parameter as positive integer value.
 */
unsigned int Main::intParam () {
    ++curParam;
    if (curParam >= argc) {
        jqs_error("%s: integer argument expected\n", argv[curParam - 1]);
        printUsage ();
    }
    errno = 0;
    long value = strtol(argv[curParam], (char**)NULL, 10);
    if (value == 0 && errno != 0) {
        jqs_error("%s: integer argument expected\n", argv[curParam - 1]);
        printUsage ();
    }
    if (value < 0) {
        jqs_error("%s: positive integer argument expected\n", argv[curParam - 1]);
        printUsage ();
    }
    if (value > UINT_MAX) {
        jqs_error("%s: positive integer argument out of range\n", argv[curParam - 1]);
        printUsage ();
    }
    return value;
}

/*
 * Parses command line.
 */
void Main::parseCommandLine () {
    for (curParam = 1; curParam < argc; ++curParam) {

        string opt = argv[curParam];

        if (opt == CMDLINE_ARG_CONFIG) {
            ::configFileName = stringParam ();

        } else if (opt == CMDLINE_ARG_PROFILE) {
            ::profileFileName = stringParam ();

        } else if (opt == CMDLINE_ARG_VERBOSE) {
            ::verbose = intParam ();

        } else if (opt == CMDLINE_ARG_LOGFILE) {
            ::logFileName = stringParam ();

        } else {
            jqs_error("Unknown command line option: %s\n", argv[curParam]);
            printUsage ();
        }
    }
    if (::configFileName.empty ()) {
        printUsage();
    }
}

/*
 * Executes filtering.
 */
void Main::execute () {
    openLogFile(logFileName);
    initOSLayer();
    
    parseConfigFile(::configFileName, true);

    if(!profileFileName.empty()) {
        parseProfile(profileFileName, true);
    }
    jqs_exit (0);
}

/*
 * The entry point.
 */
int main(int argc, const char** argv) {
    TRY {
        Main (argc, argv);
    } CATCH_SYSTEM_EXCEPTIONS {
        jqs_exit(1);
    }
}
