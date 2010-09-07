/*
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "os_layer.hpp"

#include <vector>

#include <pdhmsg.h>

#include "os_utils.hpp"
#include "print.hpp"
#include "service.hpp"
#include "utils.hpp"
#include "locale_str.h"

using namespace std;

#define N_DRIVE_LETTERS 26


static jqs_at_exit_callback_func_t at_exit_callback;

/*
 * Sets the callback to be called when jqs_exit() is executed.
 */
void jqs_at_exit(jqs_at_exit_callback_func_t at_exit) {
    assert (!at_exit_callback);
    at_exit_callback = at_exit;
}

/*
 * Notifies the registered at_exit callback and exits the process.
 */
void jqs_exit (int code) {
    TRY {
        if (at_exit_callback) {
            at_exit_callback(code);
        }
    } CATCH_SYSTEM_EXCEPTIONS {
        // ignore system exceptions raised during at_exit_callback
    }
    exit(code);
}

/*
 * A Structured Exception Handling (SEH) filter function. Logs the system 
 * exception occurred and causes the __except handle to execute.
 */
int jqs_exception_filter(struct _EXCEPTION_POINTERS *ep) {
    __try {
        if (ep->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION) {
            jqs_error ("Access violation at %#x, access to " PTR_FORMAT "\n", ep->ContextRecord->Eip, ep->ExceptionRecord->ExceptionInformation[1]);
        } else {
            jqs_error ("System exception %#x at %#x\n", ep->ExceptionRecord->ExceptionCode, ep->ContextRecord->Eip);
        }
    } __except(EXCEPTION_EXECUTE_HANDLER) {
    }
    return EXCEPTION_EXECUTE_HANDLER;
}
   

/*
 * Returns the full path to the specified module. If NULL is passed, the function
 * return the full path of the process' executable.
 */
std::string getMyFileName(HMODULE hModule) {
    vector<char> exePath (MAX_PATH);

    if (!GetModuleFileName(hModule, &exePath [0], MAX_PATH)) {
        jqs_warn("Unable to obtain module path: GetModuleFileName failed (error %d).\n", GetLastError());
        return "";
    }

    return string(&exePath[0]);
}

/*
 * Returns canonicalized name of the given path (expands . and ..).
 */
std::string getFullPath(const std::string& path) {
    vector<char> fullPath(MAX_PATH);
    LPTSTR lpFilePart;

    DWORD res = GetFullPathName (path.c_str(), MAX_PATH, &fullPath[0], &lpFilePart);
    if ((res == 0) || (res >= MAX_PATH)) {
        jqs_warn("Unable to get full path for \"%s\": GetFullPathName failed (error %d)\n", path.c_str(), GetLastError());
        return path;
    }

    vector<char> longFullPath(MAX_PATH);
    res = GetLongPathName(&fullPath[0], &longFullPath[0], (DWORD) longFullPath.size());
    if (!res || (res >= (DWORD) longFullPath.size())) {
        DWORD lastError = GetLastError();
        if (lastError != ERROR_FILE_NOT_FOUND) {
            jqs_warn("Unable to get long path name for \"%s\": GetLongPathName failed (error %d)\n", &fullPath[0], lastError);
        }
        return string(&fullPath[0]);
    }

    return string(&longFullPath[0]);
}

/*
 * Extracts file name from given path.
 */
std::string getBaseName(const std::string& path) {
    size_t pos = path.find_last_of(FILE_SEPARATOR_CHAR);
    if (pos == string::npos) {
        return path;
    }
    return path.substr(pos + 1);
}


/*
 * Constructs file name of the file located in the same directory as "my file".
 * "." and ".." are allowed here.
 */
std::string getSatelliteFileName (const string& name, HMODULE hModule) {
    string exePath = getMyFileName(hModule);

    size_t pos = exePath.find_last_of(FILE_SEPARATOR_CHAR);
    if (pos == string::npos) {
        jqs_warn("Unable to get JQS file %s: invalid JQS path %s\n", name.c_str(), exePath.c_str());
        return name;
    }
    exePath.erase (pos + 1);

    return exePath + name;
}

/*
 * Converts file name of the mapping (in "device" notation) to ordinary path.
 */
std::string convertMappedFileName (const std::string& mappedFileName) {
    DWORD logicalDrives = GetLogicalDrives();
    if (!logicalDrives) {
        jqs_warn("Unable to convert mapped file name %s: GetLogicalDrives failed (error %d)\n", mappedFileName.c_str(), GetLastError());
        return mappedFileName;
    }

    for (size_t i = 0; i < N_DRIVE_LETTERS; i++) {
        if ((logicalDrives & (1 << i)) == 0) {
            continue;
        }

        char drive[] = {(char)('A' + i), ':', 0};
        vector<char> deviceName(MAX_PATH);

        DWORD res = QueryDosDevice(drive, &deviceName[0], (DWORD)deviceName.size());
        if (!res || (res > (DWORD)deviceName.size())) {
            jqs_warn("QueryDosDevice failed for \"%s\" (error %d)\n", drive, GetLastError());
            continue;
        }

        size_t deviceNameLen = strlen(&deviceName[0]);
        if (!_strnicmp(mappedFileName.c_str(), &deviceName[0], deviceNameLen)) {
            // device name found
            return string(drive) + mappedFileName.substr(deviceNameLen);
        }
    }

    return mappedFileName;
}

/*
 * Opens physical device of the specified file.
 * Returns INVALID_HANDLE_VALUE if device cannot be opened.
 */
HANDLE openDeviceByFileName (const char* path) {
    vector<char> fullPath(MAX_PATH);
    LPTSTR lpFilePart;

    DWORD res = GetFullPathName (path, MAX_PATH, &fullPath[0], &lpFilePart);
    if ((res == 0) || (res >= MAX_PATH)) {
        jqs_warn("Unable to get full path for \"%s\": GetFullPathName failed (error %d)\n", path, GetLastError());
        return INVALID_HANDLE_VALUE;
    }

    if (fullPath[1] != ':') {
        jqs_info (5, "Unable to open device: full path for \"%s\" is not local\n", path);
        return INVALID_HANDLE_VALUE;
    }

    // Make "\\.\X:" device name
    char disk[] = {'\\', '\\', '.', '\\', fullPath[0], ':', 0};

    HANDLE hDevice = CreateFile (disk,
                                 0, 
                                 FILE_SHARE_READ | FILE_SHARE_WRITE,
                                 NULL,
                                 OPEN_EXISTING,
                                 FILE_ATTRIBUTE_NORMAL,
                                 NULL);

    if (hDevice == INVALID_HANDLE_VALUE) {
        jqs_warn("Unable to open device %s: CreateFile failed (error %d).\n", disk, GetLastError());
    }

    return hDevice;
 }

//////////////////////////////////////////////////////////////////////////

/*
 * Performance counters obtained by PerformanceCountersMonitor implementation.
 */
PDH_COUNTER_PATH_ELEMENTS MonitoredPerformanceCounters[N_PERF_COUNTERS] =
{
    // PERF_COUNTER_PROCESSOR_TIME
    { NULL, "Processor", "_Total", NULL, -1, "% Processor Time" },

    // PERF_COUNTER_DISK_TIME
    { NULL, "PhysicalDisk", "_Total", NULL, -1, "% Disk Time" },

};

PerformanceCountersMonitor::PerformanceCountersMonitor()
    : hQuery(NULL)
{
    for (size_t i = 0; i < N_PERF_COUNTERS; i++) {
        hCounters[i] = NULL;
        counterValues[i] = 0.0;
        counterFailures[i] = 0;
    }
} 

PerformanceCountersMonitor::~PerformanceCountersMonitor() {
    if (!_PdhRemoveCounter ||
        !_PdhCloseQuery)
    {
        return;
    }

    // Remove all the counters from the query.
    for (size_t i = 0; i < N_PERF_COUNTERS; i++) {
        if (hCounters[i]) {
            _PdhRemoveCounter(hCounters[i]);
        }
    }

    if (hQuery) {
        _PdhCloseQuery(hQuery);
    }
}

/*
 * Registers performance counters.
 */
bool PerformanceCountersMonitor::init() {

    if (!_PdhOpenQuery ||
        !_PdhAddCounter ||
        !_PdhRemoveCounter ||
        !_PdhCollectQueryData ||
        !_PdhCloseQuery ||
        !_PdhGetFormattedCounterValue ||
        !_PdhMakeCounterPath)
    {
        return false;
    }

    PDH_STATUS s = _PdhOpenQuery(NULL, 0, &hQuery);
    if (s != ERROR_SUCCESS) {
        jqs_warn("Unable to initialize performance counters monitor: PdhOpenQuery failed (status %08x)\n", s);
        return false;
    }

    vector<char> szFullPath(PDH_MAX_COUNTER_PATH);

    for (size_t i = 0; i < N_PERF_COUNTERS; i++) {
        jqs_info (5, "Registering performance counter monitor for [%s\\%s\\%s]\n",
                     MonitoredPerformanceCounters[i].szObjectName,
                     MonitoredPerformanceCounters[i].szCounterName,
                     MonitoredPerformanceCounters[i].szInstanceName);

        DWORD cbPathSize = (DWORD)szFullPath.size ();

        s = _PdhMakeCounterPath(&MonitoredPerformanceCounters[i], &szFullPath[0], &cbPathSize, 0);
        if (s != ERROR_SUCCESS) {
            jqs_warn("Unable to initialize performance counters monitor: PdhMakeCounterPath failed (status %08x)\n", s);
            return false;
        }

        s = _PdhAddCounter(hQuery, &szFullPath[0], 0, &hCounters[i]);
        if (s != ERROR_SUCCESS) {
            jqs_warn("Unable to initialize performance counters monitor: PdhAddCounter failed (status %08x)\n", s);
            return false;
        }

        jqs_info (4, "Performance counter monitor for [%s\\%s\\%s] registered\n",
                     MonitoredPerformanceCounters[i].szObjectName,
                     MonitoredPerformanceCounters[i].szCounterName,
                     MonitoredPerformanceCounters[i].szInstanceName);
}
    return true;
}

/*
 * Obtains performance counter values from the system.
 */
void PerformanceCountersMonitor::collect() {
    for (size_t i = 0; i < N_PERF_COUNTERS; i++) {
        counterValues[i] = 0.0;
    }

    // Collect data as often as you need to.
    PDH_STATUS s = _PdhCollectQueryData(hQuery);
    if (s != ERROR_SUCCESS) {
        jqs_warn("Unable to collect performance counters data: PdhCollectQueryData failed (status %08x)\n", s);
        return;
    }

    // Extract the calculated performance counter value for each counter or
    // instance.
    for (size_t i = 0; i < N_PERF_COUNTERS; i++) {
        PDH_FMT_COUNTERVALUE counterValue;
        s = _PdhGetFormattedCounterValue(hCounters[i], PDH_FMT_DOUBLE, NULL, &counterValue);
        if (s != ERROR_SUCCESS) {
            switch (s) {
                case PDH_CALC_NEGATIVE_DENOMINATOR:
                    jqs_info (5, "Unable to collect performance counters data: PdhGetFormattedCounterValue failed with PDH_CALC_NEGATIVE_DENOMINATOR\n");
                    break;

                case PDH_CALC_NEGATIVE_VALUE:
                    jqs_info (5, "Unable to collect performance counters data: PdhGetFormattedCounterValue failed with PDH_CALC_NEGATIVE_VALUE\n");
                    break;

                case PDH_INVALID_DATA:
                    // Performance counter does not exist or it is not yet created
                    jqs_info (5, "Unable to collect performance counters data: PdhGetFormattedCounterValue failed with PDH_INVALID_DATA\n");
                    counterFailures[i]++;
                    if (counterFailures[i] <= PERF_COUNTER_FAILURES_LIMIT) {
                        counterValues[i] = 100.0;
                    }
                    break;

                default:
                    jqs_warn("Unable to collect performance counters data: PdhGetFormattedCounterValue failed (status %08x)\n", s);
                    break;
            }
        } else {
            counterValues[i] = counterValue.doubleValue;
            counterFailures[i] = 0;
        }
        jqs_info (5, "Performance counter \"%s\\%s\\%s\": %3.1f\n",
                    MonitoredPerformanceCounters[i].szObjectName,
                    MonitoredPerformanceCounters[i].szCounterName,
                    MonitoredPerformanceCounters[i].szInstanceName,
                    counterValues[i]);
    }
}

/*
 * Returns the value of the requested performance counter in percent.
 */
double PerformanceCountersMonitor::getCounterValue(size_t index) {
    assert (index < N_PERF_COUNTERS);
    return counterValues[index];
}


//////////////////////////////////////////////////////////////////////////

/*
 * function to print the process working set size held in wss
 */
void print_process_working_set_size(const char* str, working_set_size_t* wss) {
    jqs_info (5, "%s process working set size: min = " SIZET_FORMAT
                 " max = " SIZET_FORMAT "\n",
                 str, wss->minimum, wss->maximum);
}

/*
 * this function gets the current process working set size
 */
void get_process_working_set_size(working_set_size_t* wss) {
    assert (CurProcessHandle);

    wss->minimum = 0;
    wss->maximum = 0;

    if (_GetProcessWorkingSetSize == NULL) {
        jqs_warn ("Could not get working set size: no operating system support\n");
        return;
    }

    if (!_GetProcessWorkingSetSize(CurProcessHandle, &wss->minimum, &wss->maximum)) {
        jqs_warn ("Could not get process working set size: GetProcessWorkingSetSize failed (error %d)\n",
                  GetLastError());
    }
}

/*
 * this function sets the current process working set size
 */
void set_process_working_set_size(working_set_size_t* wss) {
    assert (CurProcessHandle);

    if (_SetProcessWorkingSetSize == NULL) {
        jqs_warn ("Could not set working set size: no operating system support\n");
        return;
    }

    if (verbose >= 5) {
        working_set_size_t wssTmp;
        get_process_working_set_size(&wssTmp);
        print_process_working_set_size ("Before ", &wssTmp);
    }

    if (wss->minimum == (size_t)-1 && wss->maximum == (size_t)-1) {
        jqs_info (4, "Flushing process working set\n");
    
    } else {
        jqs_info (4, "Setting process working set size to: "
                    "min = " SIZET_FORMAT " max = " SIZET_FORMAT "\n",
                    wss->minimum, wss->maximum);
    }

    if (!_SetProcessWorkingSetSize(CurProcessHandle, wss->minimum, wss->maximum)) {
        jqs_warn ("Could not set process working set size: "
                  "requested min = %ld, requested max = %ld: "
                  " error %d\n", 
                  wss->minimum, wss->maximum, GetLastError());
    }
}

/*
 * Converts unicode string to multibyte (ANSI code page).
 */
std::string convUnicodeToString (const unicodechar_t* wcsString) {
    assert (wcsString != NULL);

    int len = WideCharToMultiByte (CP_ACP, 0, wcsString, -1, NULL,
                                   0, NULL, NULL);

    vector<char> mbsString (len + 1);

    if (!WideCharToMultiByte (CP_ACP, 0, wcsString, -1, &mbsString[0],
                              len + 1, NULL, NULL))
    {
        jqs_error("Failed to convert unicode string to ansi string: '%S' (error %d)\n", 
            wcsString, GetLastError ());
        return string();
    }

    return string(&mbsString[0]);
}

/*
 * Returns true if JQS is launched by user having administrator privileges.
 */
bool isAdmin() {
    // First we must open a handle to the access token for this thread.

    HANDLE hToken;
    if (!OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, FALSE, &hToken)) {
        if (GetLastError() != ERROR_NO_TOKEN)
            return false;

        // If the thread does not have an access token, we'll 
        // examine the access token associated with the process.
        if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
            return false;
    }

    HandleCloser hTokenCloser(hToken);

    // Now we allocate a buffer for the group information.
    vector<char> buf(1024);

    for (;;) {
        DWORD returnLength = 0;
        if (GetTokenInformation (hToken, TokenGroups,
                                 (TOKEN_GROUPS *) &buf[0], (DWORD) buf.size(), &returnLength))
        {
            break;
        }

        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
            return false;

        buf.clear();
        buf.resize(returnLength);
    }

    TOKEN_GROUPS *ptg = (TOKEN_GROUPS *) &buf[0];

    // Now we must create a System Identifier for the Admin group.

    SID_IDENTIFIER_AUTHORITY SystemSidAuthority = SECURITY_NT_AUTHORITY;
    PSID psidAdmin;

    if (!AllocateAndInitializeSid (&SystemSidAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                   DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &psidAdmin))
    {
        return false;
    }

    // Finally we'll iterate through the list of groups for this access
    // token looking for a match against the SID we created above.

    bool isAdmin = false;
    for (DWORD group = 0; group < ptg->GroupCount; group++) {
        if (EqualSid (ptg->Groups[group].Sid, psidAdmin)) {
            isAdmin = true;
            break;
        }
    }

    // Before we exit we must explicitly deallocate the SID we created.
    FreeSid (psidAdmin);

    return isAdmin;
}

/*
 * function to check if a given file name has one of the
 * list of file name extensions.
 */
bool has_extension(const string& filename, const char** extensions) {
    size_t last_dot = filename.find_last_of ('.');
    if (last_dot == string::npos) {
        return false;
    }

    string ext = tolowercase (filename.substr (last_dot + 1));

    for (int i = 0; extensions[i] != NULL; i++) {
        if (ext == extensions[i]) {
            return true;
        }
    }

    return false;
}

/*
 * list of file name extensions that are considered to be
 * executable files.
 */
const char* exe_extensions[] = {
    "exe",
    "dll",
    NULL
};

/*
 * function returning a boolean indication of the the given
 * file name is an executable file or not.
 */
bool is_executable(const string& filename) {
    return has_extension(filename, exe_extensions);
}

/* 
 * Returns the current locale.
 */
string getLocaleString() {
    size_t len = sizeof(localeIDMap) / sizeof(LCIDtoLocale);

    LCID lcid = GetThreadLocale();
    // first look for whole thing
    for (size_t i = 0; i < len; i++) {
        if (lcid == localeIDMap[i].winID) {
            return localeIDMap[i].javaID;
        }
    }
    lcid &= 0xff; // look for just language
    for (size_t i = 0; i < len; i++) {
        if (lcid == localeIDMap[i].winID) {
            return localeIDMap[i].javaID;
        }
    }
    return "en_US";
}

/*
 * Returns default memory page size.
 */
size_t getDefaultPageSize () {
    assert (SystemInfo.dwPageSize);
    return SystemInfo.dwPageSize;
}
