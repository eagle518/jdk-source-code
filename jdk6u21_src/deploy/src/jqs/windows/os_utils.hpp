/*
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef OS_UTILS_HPP
#define OS_UTILS_HPP

#include "os_layer.hpp"

#include <string>

#include "jqs.hpp"
#include "print.hpp"



typedef void (*jqs_at_exit_callback_func_t)(int code);

/*
 * Sets the callback to be called when jqs_exit() is executed.
 */
extern void jqs_at_exit(jqs_at_exit_callback_func_t at_exit);

/*
 * Notifies the registered at_exit callback and exits the process.
 */
extern void jqs_exit (int code);


/*
 * These TRY/CATCH macros are intended to catch ALL system exceptions.
 */
#define TRY                     __try
#define CATCH_SYSTEM_EXCEPTIONS __except (jqs_exception_filter(GetExceptionInformation()))

/*
 * A Structured Exception Handling (SEH) filter function. Logs the system 
 * exception occurred and causes the __except handle to execute.
 */
extern int jqs_exception_filter(struct _EXCEPTION_POINTERS *ep);


/*
 * Returns the full path to the specified module. If NULL is passed, the function
 * return the full path of the process' executable.
 */
extern std::string getMyFileName(HMODULE hModule = NULL);

/*
 * Returns canonicalized name of the given path (expands . and ..).
 */
extern std::string getFullPath(const std::string& path);

/*
 * Extracts file name from given path.
 */
extern std::string getBaseName(const std::string& path);

/*
 * Constructs file name of the file located in the same directory as "my file".
 * "." and ".." are allowed here.
 */
extern std::string getSatelliteFileName (const std::string& name, HMODULE hModule = NULL);

/*
 * Converts file name of the mapping (in "device" notation) to ordinary path.
 */
extern std::string convertMappedFileName (const std::string& mappedFileName);

/*
 * Opens physical device of the specified file.
 * Returns INVALID_HANDLE_VALUE if device cannot be opened.
 */
extern HANDLE openDeviceByFileName (const char* path);

/* 
 * Simple wrapper for for automatic closing of Windows HANDLEs.
 */
class HandleCloser {
    HANDLE handle;
public:
    HandleCloser (HANDLE handle_)
        : handle(handle_)
    {}
    ~HandleCloser () {
        if (!CloseHandle(handle)) {
            jqs_error("Error closing handle (error %d).\n", GetLastError());
        }
    }
};

/*
 * Simple wrapper for for automatic closing of Windows registry keys.
 */
class RegKeyCloser {
    HKEY key;
public:
    RegKeyCloser (HKEY key_)
        : key(key_)
    {}
    ~RegKeyCloser () {
        LONG err = RegCloseKey(key);
        if (err != ERROR_SUCCESS) {
            jqs_error("Error closing registry key. Error %d.\n", err);
        }
    }
};


/*
 * Simple wrapper for for automatic unloading of DLL.
 */
class LibraryCloser {
    HMODULE module;
public:
    LibraryCloser (HMODULE module_)
        : module(module_)
    {}

    ~LibraryCloser () {
        if (!FreeLibrary(module)) {
            jqs_error("Error unloading library (error %d).\n", GetLastError());
        }
    }
};


/*
 * Simple wrapper for for automatic cleanup of OLE.
 */
class OLEFinalizer {
public:
    OLEFinalizer ()
    {}

    ~OLEFinalizer () {
        OleUninitialize();
    }
};

/*
 * Implementation of a critical section.
 */
class CriticalSection {
    CRITICAL_SECTION sec;

public:
    CriticalSection() {
        ::InitializeCriticalSection(&sec);
    }

    ~CriticalSection() {
        ::DeleteCriticalSection(&sec);
    }

    class Lock {
        CriticalSection& critSec;
    public:
        Lock(CriticalSection& cs)
            : critSec(cs)
        {
            critSec.Enter();
        }
        ~Lock() {
            critSec.Leave();
        }
    };

public:
    void Enter () {
        ::EnterCriticalSection(&sec);
    }

    void Leave () {
        ::LeaveCriticalSection(&sec);
    }
};


/*
 * This object is intended to set background priority for the process in 
 * constructor and reset the background priority in destructor.
 * Note: since background priority is supported only on Vista,
 * this class does nothing for older systems.
 */
class BackgroundPrioritySetter {
    bool backgroundModeSet;
public:
    BackgroundPrioritySetter() 
        : backgroundModeSet(false)
    {
        if (SetPriorityClass(CurProcessHandle, PROCESS_MODE_BACKGROUND_BEGIN)) {
            backgroundModeSet = true;

        } else {
            DWORD lastError = GetLastError();

            // Ignore invalid parameter errors; it just means we are not
            // running on Windows Vista.
            //
            if (lastError != ERROR_INVALID_PARAMETER) {
                jqs_warn ("Could not set background mode for I/O: SetPriorityClass failed (error %d).\n", lastError);
            }
        }
    }

    ~BackgroundPrioritySetter() {
        if (backgroundModeSet) {
            if (!SetPriorityClass(CurProcessHandle, PROCESS_MODE_BACKGROUND_END)) {
                jqs_warn ("Could not reset background mode for I/O: SetPriorityClass failed (error %d)\n", GetLastError());
            }
        }
    }

};

/*
 * Performance counters obtained by PerformanceCountersMonitor implementation.
 */
enum {
    PERF_COUNTER_PROCESSOR_TIME,
    PERF_COUNTER_DISK_TIME,
    N_PERF_COUNTERS
};

/*
 * The specified number of subsequent failures during obtaining performance 
 * counter values will cause the PerformanceCountersMonitor to return 100%,
 * while the next failures will cause the monitor to return 0%.
 */
#define PERF_COUNTER_FAILURES_LIMIT     5

/*
 * This class is responsible for obtaining the values of system performance 
 * counters.
 */
class PerformanceCountersMonitor {
    HQUERY   hQuery;
    HCOUNTER hCounters[N_PERF_COUNTERS];
    double   counterValues[N_PERF_COUNTERS];
    unsigned counterFailures[N_PERF_COUNTERS];
    
public:
    PerformanceCountersMonitor();
    ~PerformanceCountersMonitor();

    /*
     * Registers performance counters.
     */
    bool init();

    /*
     * Obtains performance counter values from the system.
     */
    void collect();

    /*
     * Returns the value of the requested performance counter in percent.
     */
    double getCounterValue(size_t index);
};


/*
 * structure to hold the working set size parameters
 */
typedef struct working_set_size {
    SIZE_T minimum;
    SIZE_T maximum;
} working_set_size_t;


/*
 * function to print the process working set size held in wss
 */
void print_process_working_set_size(const char* str, working_set_size_t* wss);

/*
 * this function gets the current process working set size
 */
void get_process_working_set_size(working_set_size_t* wss);

/*
 * this function sets the current process working set size
 */
void set_process_working_set_size(working_set_size_t* wss);

/*
 * Converts unicode string to multibyte (ANSI code page).
 */
std::string convUnicodeToString (const unicodechar_t* wcsString);

/*
 * Returns true if JQS is launched by user having administrator privileges.
 */
bool isAdmin();

/*
* function returning a boolean indication of the the given
* file name is an executable file or not.
*/
extern bool is_executable(const std::string& filename);

/* 
 * Returns the current locale.
 */
extern std::string getLocaleString ();

/*
 * Returns default memory page size.
 */
extern size_t getDefaultPageSize ();

#endif
