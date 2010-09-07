/*
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "os_layer.hpp"

#include <stdio.h> 
#include <stdlib.h>
#include <stdarg.h>

#include <string>
#include <vector>
#include <time.h>

#include "jqs.hpp"
#include "utils.hpp"
#include "service.hpp"
#include "serviceres.h"
#include "print.hpp"
#include "os_utils.hpp"


using namespace std;

/*
 * Current verbosity level.
 */
unsigned int verbose = 0;

/*
 * The service mode flag defines whether to report messages to system event
 * log or to console window.
 */
bool service_mode = false;


enum MessageKind {
    MK_Error,
    MK_Warning,
    MK_Info,
};


//////////////////////////////////////////////////////////////////////////
// EventLog reporting stuff
//////////////////////////////////////////////////////////////////////////

static bool eventLogAvailable = false;

#define EVENT_SOURCE_KEY    "SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application"

/*
 * Initializes facility to print to system event log.
 */
void addEventSource() {
    HKEY hk;
    DWORD dwData, dwDisp; 
    LONG err;

    // Create the event source registry key

    const char* key = EVENT_SOURCE_KEY "\\" JQS_SERVICE_NAME;

    err = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                         key,
                         0, NULL, REG_OPTION_NON_VOLATILE,
                         KEY_WRITE, NULL, &hk, &dwDisp);
    if (err != ERROR_SUCCESS) {
        jqs_warn("Unable to initialize event logging: Could not create the registry key HKEY_LOCAL_MACHINE\\%s. Error %d\n", key, err); 
        return;
    }

    RegKeyCloser keyCloser(hk);

    // Set the name of the message file. 

    string myFileName = getMyFileName ();

    err = RegSetValueEx(hk,
                        "EventMessageFile",
                        0,
                        REG_EXPAND_SZ,
                        (LPBYTE) myFileName.c_str(),
                        (DWORD) myFileName.length() + 1);
    
    if (err != ERROR_SUCCESS) {
        jqs_warn("Unable to initialize event logging: Could not set the event message file. Error %d\n", err); 
        return;
    }

    // Set the supported event types. 

    dwData = EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | 
        EVENTLOG_INFORMATION_TYPE; 

    err = RegSetValueEx(hk,
                        "TypesSupported",
                        0,
                        REG_DWORD,
                        (LPBYTE) &dwData,
                        sizeof(DWORD));
    
    if (err != ERROR_SUCCESS) {
        jqs_warn("Unable to initialize event logging: Could not set the supported types. Error %d\n", err); 
        return;
    }

    eventLogAvailable = true;
}

/*
 * Uninitializes facility to print to system event log.
 */
void removeEventSource() {
    HKEY hk;
    LONG err;

    err = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                       EVENT_SOURCE_KEY,
                       0,
                       KEY_WRITE, &hk);
    
    if (err != ERROR_SUCCESS) {
        if (err == ERROR_FILE_NOT_FOUND) {
            jqs_info (4, "Registry key HKEY_LOCAL_MACHINE\\%s does not exist\n", 
                EVENT_SOURCE_KEY);

        } else {
            jqs_warn("Could not open the registry key HKEY_LOCAL_MACHINE\\%s. Error %d\n", 
                EVENT_SOURCE_KEY, err);
        }
        return;
    }

    RegKeyCloser keyCloser(hk);

    err = RegDeleteKey(hk, JQS_SERVICE_NAME);
    if (err != ERROR_SUCCESS) {
        if (err == ERROR_FILE_NOT_FOUND) {
            jqs_info (4, "Registry value HKEY_LOCAL_MACHINE\\%s\\%s does not exist\n", 
                EVENT_SOURCE_KEY, JQS_SERVICE_NAME);

        } else {
            jqs_warn("Could not delete the registry value HKEY_LOCAL_MACHINE\\%s\\%s. Error %d\n", 
                EVENT_SOURCE_KEY, JQS_SERVICE_NAME, err);
        }
        return;
    }
}

/*
 * Reports event of given kind to system event log.
 */
static void reportEvent(MessageKind msgKind, const char* message) {

    WORD wType;
    switch (msgKind) {
        case MK_Error:
            wType = EVENTLOG_ERROR_TYPE;
            break;

        case MK_Warning:
            wType = EVENTLOG_WARNING_TYPE;
            break;

        case MK_Info:
            wType = EVENTLOG_INFORMATION_TYPE;
            break;
    }

    HANDLE hEventSource = RegisterEventSource (NULL, JQS_SERVICE_NAME);
    if (hEventSource == NULL) {
        eventLogAvailable = false;
        jqs_warn("Unable to report messages to system event log: RegisterEventSource failed (error %d)\n", GetLastError()); 
        return;
    }

    const char* strings [] = { message };

    if (!ReportEvent(hEventSource, 
        wType,
        0, // category
        EVENT_GENERIC_INFORMATION,
        NULL,
        1,
        0,
        strings,
        NULL))
    {
        DWORD lastError = GetLastError();
        eventLogAvailable = false;
        if (lastError == ERROR_LOG_FILE_FULL) {
            jqs_warn("Unable to report messages to system event log: event log is full\n"); 
        } else {
            jqs_warn("Unable to report messages to system event log: ReportEvent failed (error %d)\n", lastError); 
        }
    }

    if (!DeregisterEventSource (hEventSource)) {
        eventLogAvailable = false;
        jqs_warn("Unable to report messages to system event log: DeregisterEventSource failed (error %d)\n", GetLastError()); 
    }
}

//////////////////////////////////////////////////////////////////////////
// Log file reporting stuff
//////////////////////////////////////////////////////////////////////////

/*
 * Log file handle.
 */
static FILE* logfile = NULL;

/*
 * Opens given file for messages logging.
 */
void openLogFile (const string& name) {
    setlocale(LC_TIME, "");
    if (logfile) {
        fclose (logfile);
        logfile = NULL;
    }
    if (!name.empty ()) {
        logfile = fopen (name.c_str (), "a");
        if (!logfile) {
            int lastErrno = errno;
            jqs_warn ("Unable to open log file %s: %s\n", name.c_str (), strerror (lastErrno));
        }
    }
}

/*
 * Logs message of given kind.
 */
static void print (MessageKind msgKind, const char* msg) {
    const char* msgPrefix = "";
    bool reportToEventLog = false;

    switch (msgKind) {
        case MK_Error:
            msgPrefix = "ERROR: ";
            reportToEventLog = true;
            break;

        case MK_Warning:
            msgPrefix = "WARN: ";
            reportToEventLog = (verbose > 0);
            break;

        case MK_Info:
            msgPrefix = "INFO: ";
            break;
    }

    if (logfile) {
        time_t t = time (NULL);
        struct tm* today = localtime(&t);

        char buf[64] = {0};
        strftime(buf, sizeof(buf) - 1, "[%c] ", today);

        fprintf(logfile, "%s%s%s", buf, msgPrefix, msg);
        fflush (logfile);
    }
    
    if (service_mode && eventLogAvailable) {
        if (reportToEventLog) {
            reportEvent (msgKind, msg);
        }

    } else {
        fprintf(stderr, "%s%s", msgPrefix, msg);
        fflush (stderr);
    }
}

/*
 * Reports error to log file and to system event log. 
 * The format is the same as for printf.
 */
void jqs_error(const char* format, ...) {
    vector<char> buf (1024);

    va_list args;
    va_start(args, format);
    _vsnprintf(&buf[0], buf.size()-1, format, args);
    va_end(args);

    print (MK_Error, &buf[0]);
}

/*
 * Reports warning to system event log if current verbosity level is non-zero
 * and to log file. 
 * The format is the same as for printf.
 */
void jqs_warn(const char* format, ...) {
    vector<char> buf (1024);

    va_list args;
    va_start(args, format);
    _vsnprintf(&buf[0], buf.size()-1, format, args);
    va_end(args);

    print (MK_Warning, &buf[0]);
}

/*
 * If current verbosity is equal or greater than given level,
 * reports info message to log file. 
 * The format is the same as for printf.
 */
void jqs_info (unsigned int level, const char* format, ...) {
    if (verbose < level) {
        return;
    }

    vector<char> buf (1024);

    va_list args;
    va_start(args, format);
    _vsnprintf(&buf[0], buf.size()-1, format, args);
    va_end(args);

    print (MK_Info, &buf[0]);
}
