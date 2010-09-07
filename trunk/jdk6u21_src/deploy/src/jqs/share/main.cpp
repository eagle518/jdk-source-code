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
#include "service.hpp"
#include "jqs_api_client.hpp"
#include "sockets.hpp"
#include "prefetch.hpp"
#include "os_utils.hpp"
#include "messages.hpp"

using namespace std;

static string configDumpFileName;

/*
 * JQS operation modes.
 */
enum Mode
{
    Mode_Unknown,
    Mode_InstallService,
    Mode_UninstallService,
    Mode_Service,
    Mode_Run,
    Mode_Version,
    Mode_Pause,
    Mode_Resume,
    Mode_Enable,
    Mode_Disable,
    Mode_DumpConfig,
};

/*
 * Helper class, responsible for command line parsing and executing JQS
 * the specified operation mode.
 */
class Main {
    Mode            mode;
    int             argc;
    const char**    argv;
    int             curParam;

    void printUsage ();
    void printVersion ();
    void setMode (Mode mode);
    void parseCommandLine ();

    const char* stringParam ();
    unsigned int intParam ();

public:
    Main (int argc, const char** argv);
    void execute ();

};

Main::Main (int argc_, const char** argv_)
    : mode (Mode_Unknown) 
    , argc (argc_)
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
    printf("%s", getMsgString(MSG_JQSUsage));
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
 * Sets JQS operation mode.
 */
void Main::setMode (Mode m) {
    if (mode != Mode_Unknown) {
        jqs_error("%s: mode already specified\n", argv[curParam]);
        printUsage ();
    }
    mode = m;
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
    configFileName = getSatelliteFileName (DEFAULT_CONFIG_NAME);
   
    for (curParam = 1; curParam < argc; ++curParam) {

        string opt = argv[curParam];

        if (opt == CMDLINE_ARG_HELP) {
            printUsage ();

        } else if (opt == CMDLINE_ARG_REGISTER) {
            setMode (Mode_InstallService);

        } else if (opt == CMDLINE_ARG_UNREGISTER) {
            setMode (Mode_UninstallService);

        } else if (opt == CMDLINE_ARG_SERVICE) {
            setMode (Mode_Service);

        } else if (opt == CMDLINE_ARG_RUN) {
            setMode (Mode_Run);

        } else if (opt == CMDLINE_ARG_VERSION) {
            setMode (Mode_Version);

        } else if (opt == CMDLINE_ARG_PAUSE) {
            setMode (Mode_Pause);

        } else if (opt == CMDLINE_ARG_RESUME) {
            setMode (Mode_Resume);

        } else if (opt == CMDLINE_ARG_ENABLE) {
            setMode (Mode_Enable);

        } else if (opt == CMDLINE_ARG_DISABLE) {
            setMode (Mode_Disable);

        } else if (opt == CMDLINE_ARG_CONFIG) {
            configFileName = stringParam ();

        } else if (opt == CMDLINE_ARG_PROFILE) {
            profileFileName = stringParam ();

        } else if (opt == CMDLINE_ARG_VERBOSE) {
            verbose = intParam ();

        } else if (opt == CMDLINE_ARG_TIMING) {
            print_times = intParam ();
        
        } else if (opt == CMDLINE_ARG_LOGFILE) {
            logFileName = stringParam ();

        } else if (opt == CMDLINE_ARG_DUMPCONFIG) {
            setMode (Mode_DumpConfig);
            configDumpFileName = stringParam ();

        } else {
            jqs_error("Unknown command line option: %s\n", argv[curParam]);
            printUsage ();
        }
    }
}

void dumpConfig(const std::string& outFileName);

/*
 * Checks if JQS is launched by user having administrator privileges.
 * If it isn't, issues error message and exits.
 */
void checkAdminPrivileges() {
    if (!isAdmin()) {
        printf("%s\n", getMsgString(MSG_JQSRequiresAdminPrivileges));
        jqs_exit(1);
    }
}

/*
 * Executes JQS according to operation mode and options specified 
 * in the command line.
 */
void Main::execute () {
    service_mode = (mode == Mode_Service);

    openLogFile(logFileName);
    initOSLayer();
    initSocketLibrary ();
    checkAdminPrivileges();

    setConfigVariable (JAVA_HOME, 
                       getFullPath (getSatelliteFileName (JAVA_HOME_RELATIVE_TO_JQS)));

    bool ok = true;

    switch (mode) {
        case Mode_InstallService:
            if (installService()) {
                printf("%s\n", getMsgString(MSG_JQSRegistered));

            } else {
                printf("%s\n", getMsgString(MSG_JQSRegisterFailed));
                ok = false;
            }
            break;

        case Mode_UninstallService:
            if (uninstallService()) {
                printf("%s\n", getMsgString(MSG_JQSUnregistered));

            } else {
                printf("%s\n", getMsgString(MSG_JQSUnregisterFailed));
                ok = false;
            }
            break;

        case Mode_Service:
            runService();
            break;

        case Mode_Run:
        {
            initialize();

            { 
                TraceTime t1("Parse time", 0, ::print_times >= 2);
                parseConfigFile(configFileName);
            }
            if (g_QSEntries.empty()) {
                jqs_error("No entries found in %s\n", configFileName.c_str ());
                jqs_exit(1);
            }
            if(!profileFileName.empty()) {
                parseProfile(profileFileName);
            }

            do_jqs(interval);
            cleanup();
            break;
        }

        case Mode_Version:
            printVersion();
            break;

        case Mode_Pause:
            if (sendJQSAPICommand(JMK_Pause)) {
                printf ("%s\n", getMsgString(MSG_JQSPaused));
            } else {
                printf ("%s\n", getMsgString(MSG_JQSPauseFailed));
                ok = false;
            }
            break;

        case Mode_Resume:
            if (sendJQSAPICommand(JMK_Resume)) {
                printf ("%s\n", getMsgString(MSG_JQSResumed));
            } else {
                printf ("%s\n", getMsgString(MSG_JQSResumeFailed));
                ok = false;
            }
            break;

        case Mode_Enable:
            if (enableService (true)) {
                printf ("%s\n", getMsgString(MSG_JQSEnabled));
            } else {
                printf ("%s\n", getMsgString(MSG_JQSEnableFailed));
                ok = false;
            }
            break;

        case Mode_Disable:
            if (enableService (false)) {
                printf ("%s\n", getMsgString(MSG_JQSDisabled));
            } else {
                printf ("%s\n", getMsgString(MSG_JQSDisableFailed));
                ok = false;
            }
            break;

        case Mode_DumpConfig:
            initialize();
            parseConfigFile(configFileName);
            dumpConfig(configDumpFileName);
            cleanup();
            break;

        default:
            printUsage ();
    }
    cleanupSocketLibrary ();
    jqs_exit (ok ? 0 : 1);
}

/*
 * Auxiliary function, obtains information about file system 
 * entry with specified name and ensures that the entry exists
 * and that it is of specified type.
 */
bool statFile(bool requireFile, const char* fileName, struct stat* statbuf) {
    bool exists;
    {
        TraceTime t2("Stat time", 1, print_times >= 2);
        exists = (stat(fileName, statbuf) == 0);
    }

    if (!exists) {
        jqs_warn("File %s does not exist: %s\n", fileName, strerror(errno));
        return false;
    }

    if (requireFile) {

        // only accept regular files 

        if ((statbuf->st_mode & S_IFMT) != S_IFREG) {
            jqs_warn ("Non-regular file ignored: %s\n", fileName);
            return false;
        }

        // don't accept zero length files

        if (statbuf->st_size == 0) {
            jqs_warn ("Zero length file ignored: %s\n", fileName);
            return false;
        }

    } else {

        // only accept directories

        if ((statbuf->st_mode & S_IFMT) != S_IFDIR) {
            jqs_warn ("Non-directory ignored: %s\n", fileName);
            return false;
        }
    }

    return true;
}

/*
 * Execute the command for the given quick starter entry.
 */
void do_preload(QSEntry* ent, bool powerStatus) {
    TraceTime t1("File load time", 1, print_times >= 1);

    CriticalSection::Lock lock(PrefetchLock);

    if (!powerStatus) {
        if (!checkForDeviceSuspend(ent)) {
            // device is suspended or going to suspend
            return;
        }
    }

    struct stat statbuf;
    bool preloadable = statFile((ent->getCmd() != QS_REFRESHDIR), ent->getFileName(), &statbuf);
    bool modified = preloadable &&
                    ((statbuf.st_mtime != ent->getLastModified()) ||
                     (statbuf.st_size  != ent->getFileSize()));

    // unload file if it does not exist or was modified

    if (ent->isLoaded() && (!preloadable || modified)) {
        if (modified) {
            jqs_warn ("File %s was modified, unloading it.\n", ent->getFileName());
        }
        do_unload(ent);
    }

    if (!preloadable) {
        // file unloaded, nothing else to do if file does not exist
        return;
    }

    ent->setFileSize(statbuf.st_size);
    ent->setLastModified(statbuf.st_mtime);

    if (ent->isLoaded()) {
        // refresh loaded file
        do_refresh(ent);

    } else {
        switch(ent->getCmd()) {
            case QS_LOAD:
                do_load(ent);
                break;

            case QS_LOADLIB:
                do_loadlib(ent);
                break;

            case QS_REFRESH:
            case QS_REFRESHLIB:
                do_refresh(ent);
                break;

            case QS_REFRESHDIR:
                do_refreshdir(ent);
                break;
        }
    }
}


/*
 * Unload all entries.
 */
void unload_all() {

    TraceTime t1("Unload time", 1, print_times >= 1);

    for (size_t i = 0; i < g_QSEntries.size(); i++) {
        QSEntry* entry = g_QSEntries[i];
        do_unload(entry);
    }
}

/*
 * Refresh all unlocked mapped entries and all refresh entries.
 */
void refresh_all() {
    jqs_info (3, "------- Refreshing -------\n");

    bool powerStatus = checkPowerStatus();

    TraceTime t2("Total Refresh Time", 0, (verbose >= 2) || (print_times >= 1));

    TotalBytesToTouch = 0;
    BytesActuallyTouched = 0;

    TotalBytesToRead = 0;
    BytesActuallyRead = 0;

    for (size_t i = 0; i < g_QSEntries.size(); i++) {
        CHECK_ASYNC_EVENTS();
        do_preload(g_QSEntries[i], powerStatus);
    }

    double touchedPercent = 0.0;
    if (TotalBytesToTouch != 0) {
        touchedPercent = (double)BytesActuallyTouched / (double)TotalBytesToTouch * 100;
    }
    double readPercent = 0.0;
    if (TotalBytesToRead != 0) {
        readPercent = (double)BytesActuallyRead / (double)TotalBytesToRead * 100;
    }
    jqs_info (2, "Refresh: " UINT64_FORMAT "Kb touched (%2.1f%%), " UINT64_FORMAT "Kb read (%2.1f%%)\n", 
        BytesActuallyTouched/K, touchedPercent, 
        BytesActuallyRead/K, readPercent);
    jqs_info (3, "---- Refresh finished ----\n");

    for (size_t i = 0; i < g_QSEntries.size(); i++) {
        prefetchingFinished(g_QSEntries[i]);
    }
}

/*
 * Main JQS cycle, load and refresh all entries specified in 
 * the configuration file.
 */
void do_jqs(unsigned int interval) {
    try {
        bool initialPrefetching = true;
        while (true) {
            try {
                CHECK_ASYNC_EVENTS();
                waitForSystemIdle(initialPrefetching);

                refresh_all();

                wait_for_high_mem_or_timeout(interval);
            } catch (const PauseEventException&) {
                jqs_info (3, "Pause event signaled: waiting for resume.\n");
                wait_for_resume();
                jqs_info (3, "Resuming JQS operation after pause.\n");
            }
            initialPrefetching = false;
        } 
    } catch (const TerminationEventException&) {
        jqs_info (3, "Termination event signaled: finishing JQS operation.\n");
    }
}

/*
 * Emits all options and entries to specified file in the format of JQS
 * configuration file. This operation mode is used for JQS testing purposes.
 */
void dumpConfig(const std::string& outFileName) {
    FILE* out = fopen (outFileName.c_str(), "w");
    if (!out) {
        jqs_error ("Unable to open %s for writing: %s\n", outFileName.c_str(), strerror(errno));
        return;
    }

    fprintf (out, CMD_OPTION " " OPTION_INTERVAL " = %u\n", interval);
    fprintf (out, CMD_OPTION " " OPTION_MEMORY_LIMIT " = " SIZET_FORMAT "\n", memoryLimit);
    fprintf (out, "\n");
    fprintf (out, CMD_OPTION " " OPTION_BOOT_DISK_TIME_THRESHOLD " = %3.1f\n", bootThresholds.diskTime);
    fprintf (out, CMD_OPTION " " OPTION_BOOT_PROCESSOR_TIME_THRESHOLD " = %3.1f\n", bootThresholds.processorTime);
    fprintf (out, CMD_OPTION " " OPTION_REFRESH_DISK_TIME_THRESHOLD " = %3.1f\n", refreshThresholds.diskTime);
    fprintf (out, CMD_OPTION " " OPTION_REFRESH_PROCESSOR_TIME_THRESHOLD " = %3.1f\n", refreshThresholds.processorTime);
    fprintf (out, "\n");
    for (size_t i = 0; i < g_QSEntries.size(); i++) {
        g_QSEntries[i]->dumpConfig(out);
    }
    fclose(out);
}

/*
 * The JQS entry point.
 */
int main(int argc, const char** argv) {
    TRY {
        Main (argc, argv);
    } CATCH_SYSTEM_EXCEPTIONS {
        jqs_exit(1);
    }
}
