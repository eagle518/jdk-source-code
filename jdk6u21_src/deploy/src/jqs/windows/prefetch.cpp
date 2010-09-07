/*
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "os_layer.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include "prefetch.hpp"
#include "jqs.hpp"
#include "utils.hpp"
#include "timer.hpp"
#include "print.hpp"
#include "service.hpp"
#include "jqs_api_server.hpp"
#include "sockets.hpp"
#include "os_utils.hpp"

using namespace std;

/*
 * Touch & read statistics for one refresh cycle.
 */
// total number of bytes to touch without profile during current refresh
uint64_t TotalBytesToTouch = 0;
// number of bytes actually touched during current refresh
uint64_t BytesActuallyTouched = 0;

// total number of bytes to read without profile during current refresh
uint64_t TotalBytesToRead = 0;
// number of bytes actually read during current refresh
uint64_t BytesActuallyRead = 0;

/*
 * JQS Capabilities.
 */
bool capabilityAdjustWorkingSetSize = false;
bool capabilityLockMemoryPages = false;
bool capabilityLargePages = false;
bool capabilityLowMemoryNotifications = false;
bool capabilitySetLibraryPath = false;
bool capabilityCheckPowerStatus = false;
bool capabilityDeviceEventNotifications = false;
bool capabilityUserLogonNotifications = false;

/*
 * Synchronizes accesses to global data used by the prefetcher.
 */
CriticalSection PrefetchLock;

/*
 * These flags are used to pass asynchronous events such as pause or terminate
 * to the JQS main cycle.
 */
volatile bool asyncEventOccured;
volatile bool state_terminating = false;
volatile bool state_paused = false;

/*
 * Default values for page sizes.
 */
static size_t default_large_page_size = 0;
static size_t default_page_size = 0;

/*
 * These flags are set if the JQS process has SE_LOCK_MEMORY_NAME or 
 * SE_INC_BASE_PRIORITY_NAME respectively.
 */
static bool have_lock_memory_privilege = false;
static bool have_inc_base_priority_privilege = false;

/*
 * Current working set size of the JQS process.
 */
static working_set_size_t current_wss;

/*
 * Handle to the JQS process.
 */
static HANDLE hProcess = NULL;

/*
 * This object is used to access to Windows' performance counters' values.
 */
static PerformanceCountersMonitor* performanceCountersMonitor = NULL;

/*
 * Event for waiting for resume when JQS operation is paused.
 */
static HANDLE hIdleEvent;

/*
 * This event is notified when JQS API receives NOTIFY command.
 */
static HANDLE hJQSNotificationEvent;


/*
 * This function requests privilege indicated in priv for the process
 * indicated by the process token in token. It returns TRUE if the
 * privilege was acquired, and FALSE otherwise.
 */
static bool request_privilege(HANDLE token, char* priv) {
    LUID luid;
    TOKEN_PRIVILEGES tp;
    TOKEN_PRIVILEGES tpp;
    DWORD tpp_sz = sizeof(TOKEN_PRIVILEGES);

    assert (_LookupPrivilegeValue && _AdjustTokenPrivileges);

    if (!_LookupPrivilegeValue(NULL, priv, &luid)) {
        jqs_warn ("Unable to set the required privilege %s: LookupPrivilegeValue failed (error %d)\n",
                  priv, GetLastError());
        return false;
    }

    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    tp.Privileges[0].Attributes = 0;

    if (!_AdjustTokenPrivileges(token, FALSE, &tp, sizeof(TOKEN_PRIVILEGES),
                                &tpp, &tpp_sz)) 
    {
        jqs_warn ("Unable to read privileges: AdjustTokenPrivilege failed (error %d)\n",
                  GetLastError());
        return false;
    }

    tpp.PrivilegeCount = 1;
    tpp.Privileges[0].Luid = luid;
    tpp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    BOOL rc = _AdjustTokenPrivileges(token, FALSE, &tpp, sizeof(TOKEN_PRIVILEGES),
                                     NULL, NULL);
    DWORD lastError = GetLastError();
    if (rc && lastError == ERROR_SUCCESS) {
        return true;

    } else {
        jqs_warn ("Could not set privilege %s: AdjustTokenPrivilege failed (error %d)\n", priv, lastError);
        return false;
    }
}

/*
 * This console handler function arranges for proper termination of JQS process.
 */
static BOOL WINAPI consoleHandler(DWORD event) {

    switch(event) {
        case CTRL_C_EVENT:
        case CTRL_BREAK_EVENT:
        case CTRL_CLOSE_EVENT:
        case CTRL_SHUTDOWN_EVENT:
        case CTRL_LOGOFF_EVENT:
            TRY {
                terminateJQSService();
            } CATCH_SYSTEM_EXCEPTIONS {
                jqs_exit(1);
            }
            return TRUE;

        default:
            break;
    }
    return FALSE;
}

/*
 * Function to increment the process working set size by the given amount.
 */
void inc_process_working_set_size(size_t size) {
    current_wss.minimum += size;
    current_wss.maximum += roundUp (size, default_page_size);
    set_process_working_set_size(&current_wss);
}

/*
 * Function to decrement the process working set size by the given amount.
 */
void dec_process_working_set_size(size_t size) {
    current_wss.minimum -= size;
    current_wss.maximum -= roundUp (size, default_page_size);
    set_process_working_set_size(&current_wss);
}

/*
 * This function is responsible for initializing all global data.
 */
void initialize() {

    // install our own console handler.
    //
    if (!service_mode) {
        if (!SetConsoleCtrlHandler(consoleHandler, TRUE)) {
            jqs_warn ("Could not install console handler: SetConsoleCtrlHandler failed (error %d)\n", GetLastError());
        }
    }

    initTimer ();

    hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_SET_QUOTA | PROCESS_VM_READ,
                           FALSE, GetCurrentProcessId());
    if (hProcess == NULL) {
        DWORD lastError = GetLastError();
        DWORD cpid = GetCurrentProcessId();
        jqs_error ("Could not open process %d: error %d\n", cpid, lastError);
        exit(1);
    }

    HANDLE hToken = NULL;
    assert (_OpenProcessToken);
    if (!_OpenProcessToken(hProcess, TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY,
                           &hToken)) 
    {
        jqs_error ("Could not open process token: error %d\n", GetLastError());
        exit(1);
    }

    HandleCloser hTokenCloser(hToken);


#if 0
    // EXPERIMENTAL: 

    // The following code is attempting to see if we are running
    // as a 32-bit process on a 64-bit windows system. In such a
    // case, the GetNativeSystemInfo API needs to be used to get
    // unfiltered data about the system configuration.
    //
    // the wow64 branch of the following code has not been tested
    // as of this writing.
    //
    bool is_wow64_process = false;
    if (_IsWow64Process != NULL) {
        // check if we are a 32-bit process running on 64-bit windows
        //
        BOOL value;

        _IsWow64Process(hProcess, &value);
        is_wow64_process = value ? true : false;  // avoids a c++ warning when
                                                  // attempting a (bool) cast.
    }

    SYSTEM_INFO sysinfo;
    if (is_wow64_process) {
        assert(_GetNativeSystemInfo);
        _GetNativeSystemInfo(&sysinfo);
    
    } else {
        GetSystemInfo(&sysinfo);
    }

    default_page_size = sysinfo.dwPageSize;
#else
    default_page_size = getDefaultPageSize();
#endif

    //
    // Windows large page support is available on Windows 2003 and later.
    // In order to use large pages, the administrator must first assign
    // the 'Lock pages in memory' privileges to the appropriate users,
    // including itself, as Administrator doesn't have this privilege by
    // default. How this privilege is added differs on different versions
    // of Windows.
    // Windows 2003/XP:
    //   Control Panel -> Administrative Tools -> Local Security Policy
    //     expand Local Policies
    //     expand User Rights Assignment
    //     double click on "Lock pages in memory"
    //     click "Add User or Group..."
    //     click "Object Types ..."
    //     check "Users"
    //   reboot
    //
    // In order to lock a sufficient amount of virtual memory into physical
    // RAM, the user must also have the 'Increase base priority' privilege
    //
    have_lock_memory_privilege = request_privilege(hToken, SE_LOCK_MEMORY_NAME);
    if (!have_lock_memory_privilege) {
        jqs_warn ("Could not get %s privilege: error %d\n",
                  SE_LOCK_MEMORY_NAME, GetLastError());
    }

    // request the SE_INC_BASE_PRIORITY_NAME privilege. This privilege
    // needed so we can increase our working set size. This privilege
    // it typically available to Administrators and Power Users,
    // but the ability to require it depends on the process handle
    // being opened with PROCESS_SET_QUOTA access privileges.
    //
    have_inc_base_priority_privilege = request_privilege(hToken, SE_INC_BASE_PRIORITY_NAME);
    if (!have_inc_base_priority_privilege) {
          jqs_warn ("Could not get %s privilege: error %d\n",
                    SE_INC_BASE_PRIORITY_NAME, GetLastError());
    }

    capabilityAdjustWorkingSetSize = have_inc_base_priority_privilege &&
                                     (_SetProcessWorkingSetSize != NULL) &&
                                     (_GetProcessWorkingSetSize != NULL);

    capabilityLockMemoryPages = have_lock_memory_privilege &&
                                have_inc_base_priority_privilege;

    if (!capabilityLockMemoryPages) {
        jqs_warn ("Lock memory requests will be ignored: Insufficient privileges\n");
    }

#if 0
    // EXPERIMENTAL: Windows does not support large pages for regular file mappings.
    //
    if (_GetLargePageMinimum != NULL) {
        default_large_page_size = _GetLargePageMinimum();
        if (!default_large_page_size) {
            jqs_warn ("Large page requests will be ignored: No operating system support\n");
        
        } else {
            if (have_lock_memory_privilege) {
                capabilityLargePages = true;
            } else {
                jqs_warn ("Large page requests will be ignored: Insufficient privileges\n");
            }
        }
    } else {
        jqs_warn ("Large page requests will be ignored: No operating system support\n");
    }
#endif

    capabilityLowMemoryNotifications = (_CreateMemoryResourceNotification != NULL);

    capabilitySetLibraryPath = (_SetDllDirectory != NULL);

    capabilityCheckPowerStatus = (_GetSystemPowerStatus != NULL) &&
                                 (_GetDevicePowerState != NULL);

    capabilityDeviceEventNotifications = service_mode &&
                                         (_RegisterServiceCtrlHandlerEx != NULL) &&
                                         (_RegisterDeviceNotification != NULL)  &&
                                         (_UnregisterDeviceNotification != NULL);

    // Session change notifications are available since Windows XP
    capabilityUserLogonNotifications = service_mode &&
                                       ((OSVersion.dwMajorVersion > 5) ||
                                        ((OSVersion.dwMajorVersion == 5) && (OSVersion.dwMinorVersion >= 1)));

    if (capabilityAdjustWorkingSetSize) {
        working_set_size_t initial_wss;
        get_process_working_set_size(&initial_wss);
        print_process_working_set_size("Initial", &initial_wss);

        // set the current working set size such that the min and max are 
        // the same. this prevents the OS from reducing our working set
        // size below the maximum unless the OS is in desperate need.
        //
        current_wss.maximum = initial_wss.maximum;
        current_wss.minimum = current_wss.maximum;
        set_process_working_set_size(&current_wss);
    }

    // Put the process in the IDLE priority class. Note - we may want
    // to provide a way to disable this from the command line. 
    //
    if (!SetPriorityClass(GetCurrentProcess(), IDLE_PRIORITY_CLASS)) {
        jqs_warn ("Could not enter IDLE priority class: SetPriorityClass failed (error %d)\n", GetLastError());
    }

    // Create auto-reset events in non-signaled state.
    hIdleEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    hJQSNotificationEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    if (!startJQSAPIServer()) {
        jqs_error ("Unable to start JQS API Server thread\n");
        jqs_exit(1);
    }
}

/*
 * This function is responsible for any cleanup needed before termination.
 */
void cleanup() {
    unload_all();
    stopJQSAPIServer();
    delete performanceCountersMonitor;

    cleanupOSLayer();
    if (hProcess != NULL) {
        CloseHandle(hProcess);
    }
}

/*
 * Function to get the page size for the system, using the
 * requested page size if available.
 */
size_t get_pagesize(size_t requested) {
    assert (default_page_size != 0);
#if 0
    // EXPERIMENTAL: Windows does not support large pages for regular file mappings.
    //
    if (requested != 0) {
        if (requested >= default_large_page_size) {
            return roundUp(requested, default_large_page_size);
        }
    }
#endif
    return default_page_size;
}

/*
 * Function for checking exception codes from touching
 * pages of memory. Certain exceptions are not unexpected
 * and this function filters those exceptions out and
 * allows the touch_page() loop to continue.
 */
DWORD filter_expected_errors(DWORD ex, QSEntry* ent, char* addr) {
    if ((ex == EXCEPTION_IN_PAGE_ERROR) ||
        (ex == EXCEPTION_ACCESS_VIOLATION) ||
        (ex == EXCEPTION_GUARD_PAGE)) 
    {
        return EXCEPTION_EXECUTE_HANDLER;
    }
    jqs_warn("Fatal Exception %ld at " PTR_FORMAT " in %s\n",
             ex, addr, ent->getFileName());
    return EXCEPTION_CONTINUE_SEARCH;
}

/*
 * Function to touch (read) the given address. This function also
 * uses MS structured exception handling to handle cases where the
 * given address cannot be read.
 */
char touch(QSEntry* ent, char* cp) {
    char ch;

    __try {
        ch = *cp;
    } __except(filter_expected_errors(GetExceptionCode(), ent, cp)) {
        ch = 0;
    }
    return ch;
}

/*
 * Function to touch (read) the given address. This function also
 * uses MS structured exception handling to handle cases where the
 * given address cannot be read. This is the verbose version of the
 * touch() function, which provide additional debug output.
 */
char touch_verbose(QSEntry* ent, char* cp) {
    char ch;

    __try {
        ch = *cp;
    } __except(filter_expected_errors(GetExceptionCode(), ent, cp)) {
        jqs_info (6, "Non-fatal Exception at " PTR_FORMAT ", ignored\n", cp);
        ch = 0;
    }
    return ch;
}

/*
 * Function to touch all pages between beginOffset and endOffset of 
 * the specified entry.
 */
size_t touch_region (QSEntry* ent, size_t beginOffset, size_t endOffset, size_t* bytesTouched) {
    size_t sum = 0;

    char* mapaddr = (char *) ent->getMapAddr();
    char* begin = mapaddr + beginOffset;
    char* end = mapaddr + endOffset;

    size_t pageSize = ent->getPageSize();
#if 0
    // EXPERIMENTAL: Windows does not support large pages for regular file mappings.
    //
#else
    assert (pageSize == default_page_size);
#endif

    if (verbose < 6) {
        for (char* cp = begin; cp < end; cp += pageSize) {
            CHECK_ASYNC_EVENTS();
            sum += (ULONG)touch(ent, cp);
        }
    } else {
        for (char* cp = begin; cp < end; cp += pageSize) {
            CHECK_ASYNC_EVENTS();
            jqs_info (6, "  touching " PTR_FORMAT "\n", cp);
            sum += (ULONG)touch_verbose(ent, cp);
        }
    }
    *bytesTouched += endOffset - beginOffset;
    return sum;
}

/*
 * Function to touch pages of the mapped entry according to profile information. 
 * If the profile is not specified for the entry all pages are touched.
 * A sum of the touched bytes is taken and used inside the function, so the 
 * compiler can't optimize the loop away.
 */
void touch_pages(QSEntry* ent) {
    size_t sum = 0;

    if (!ent->isLoaded()) {
        return;
    }

    const Regions& regions = ent->getRegions ();

    size_t bytesTouched = 0;

    if (!regions.empty()) {
        jqs_info (4, "  touching \"%s\" according to profile information\n", ent->getFileName());
        TraceTime t2("Touch time", 1, print_times >= 2);

        for (size_t i = 0; i < regions.size (); i++) {
            const Region& region = regions [i];
            if (region.start >= (uint64_t) ent->getMapSize()) {
                break;
            }
            // these casts cause no truncation, because:
            // 1) ent->getMapSize() is size_t;
            // 2) region.start < ent->getMapSize(), so region.start will fit into size_t;
            // 3) min(region.end, (uint64_t)ent->getMapSize()) <= ent->getMapSize(),
            //    so it will fit into size_t as well.
            size_t start = (size_t) region.start; 
            size_t end = (size_t) min(region.end, (uint64_t)ent->getMapSize());
            sum += touch_region (ent, start, end, &bytesTouched);
        }

    } else {
        jqs_info (4, "  touching the whole file \"%s\" (no profile information)\n", ent->getFileName());
        TraceTime t2("Touch time", 1, print_times >= 2);

        // read from each page of the mapping to force the pages
        // into physical memory.

        sum += touch_region (ent, 0, ent->getMapSize(), &bytesTouched);
    }

    double percentTouched = 0.0;
    if (ent->getMapSize() != 0) {
        percentTouched = (double)bytesTouched / (double)ent->getMapSize() * 100.0;
    }
    jqs_info (4, "  touched " SIZET_FORMAT " bytes of \"%s\" (%2.1f%%), sum = %lu\n",
             bytesTouched, ent->getFileName(), percentTouched, sum);

    BytesActuallyTouched += bytesTouched;
    TotalBytesToTouch += ent->getMapSize();
}

/*
 * Function to lock or touch a memory region specified in the
 * given quick starter entry. Locking will only work if the
 * process has the necessary privileges. If locking is indicated
 * but cannot be performed, a warning will be emitted and the
 * entry marked for touching instead of locking.
 */
void lock_or_touch(QSEntry* ent) {
    if (have_inc_base_priority_privilege) {
        // bump up our working set size to include this entire file.
        //
        inc_process_working_set_size(ent->getMapSize());
    }

    if (!(have_lock_memory_privilege && have_inc_base_priority_privilege)) {
        // we don't have memory lock or set quota privileges so
        // VirtualLock is not going to work. Turn off any lock requests.
        //
        if ((ent->getCmd() == QS_LOAD) || (ent->getCmd() == QS_LOADLIB)) {
            ent->setLocked(false);
        }
    }

    if (ent->isLocked()) {

        // FIXME - monitor for effects of removing the underlying file
        //
        {
            TraceTime t2("Lock time", 1, print_times >= 2);

            if (VirtualLock(ent->getMapAddr(), ent->getMapSize()) == 0) {
                // could not lock some or all of the memory - not enough
                // system memory?
                //
                jqs_warn("Could not lock virtual memory: %s : error %d\n",
                         ent->getFileName(), GetLastError());

                ent->setLocked(false);

                // since the lock failed, we need to touch all the pages
                // to force them into the file system cache.
                //
                touch_pages(ent);
            } else {
                jqs_info (4, "  locked " SIZET_FORMAT " bytes at " PTR_FORMAT "\n",
                         ent->getMapSize(), ent->getMapAddr());
            }
        }
    } else {
        // touch all the pages to force them into the file system cache
        //
        touch_pages(ent);
    }
}

/*
 * Function to load all dependent libraries of the given entry and touch 
 * their pages according to profile information.
 *
 * Note: the experiments showed that use of refreshlib commands with libdepend 
 * option does not give valuable performance improvement whereas increases
 * JQS memory consumption. It is recommended to use a sequence of refreshlib 
 * commands instead.
 */
void load_dependent_dlls(QSEntry* entry) {
    if (entry->getDependentLibs().empty()) {
        return;
    }
    assert (entry->getCmd() == QS_REFRESHLIB);

    TraceTime t1("Load Dependent Libraries time", 1, print_times >= 2);

    const QSEntries& dependentLibs = entry->getDependentLibs ();
    for (size_t i = 0; i < dependentLibs.size (); i++) {
        CHECK_ASYNC_EVENTS();
        
        QSEntry* mod = dependentLibs[i];
        assert (mod->getCmd() == QS_REFRESHLIB);

        if (!mod->isLoaded()) {
            do_loadlib(mod);
        }
    }
}

/*
 * Function to unload any loaded dependent dlls associated with given entry.
 *
 * Note: the experiments showed that use of refreshlib commands with libdepend 
 * option does not give valuable performance improvement whereas increases
 * JQS memory consumption. It is recommended to use a sequence of refreshlib 
 * commands instead.
 */
void unload_dependent_dlls(QSEntry* entry) {
    const QSEntries& dependentLibs = entry->getDependentLibs ();
    for (size_t i = 0; i < dependentLibs.size (); i++) {
        QSEntry* mod = dependentLibs[i];
        do_unload(mod);
    }
}

/*
 * Function to add directories to the dll search path.
 */
static void set_dll_dirs(const string& path) {
    assert(_SetDllDirectory);

    if (path.empty ()) {
        return;
    }

    // march through the ';' delimited paths and set them as 
    // LoadLibrary search directories.
    //
    size_t curPos = 0;
    while (curPos < path.length()) {
        size_t startPos = curPos;
        
        while ((curPos < path.length()) && (path[curPos] != ';')) {
            curPos++;
        }
        
        if (startPos < path.length()) {
            string dir = path.substr(startPos, curPos - startPos);
            jqs_info (4, "  adding %s to library search path\n", dir.c_str());
            if (!_SetDllDirectory(dir.c_str())) {
                jqs_warn("Could not set search path \"%s\" for dependent libs: SetDllDirectory failed (error %d)\n", dir.c_str(), GetLastError());
            }
        }
    }
}

/*
 * Function to restore the library path to the default value.
 */
void restore_libpath(const string& path) {
    assert(_SetDllDirectory);

    if (path.empty ()) {
        return;
    }

    // reset the DLL search order to the default
    //
    jqs_info (4, "  restoring library search path\n");
    _SetDllDirectory(NULL);
}

/*
 * Helper object, sets library path for the given entry in constructor and
 * resrores the path in desctuctor (the RAII idiom).
 */
class LibPathSetter {
    QSEntry* ent;
public:
    LibPathSetter (QSEntry* ent_) 
        : ent(ent_)
    {
        if (_SetDllDirectory != NULL) {
            TraceTime t0("SetDllDirectory", 1, print_times >= 3);
            set_dll_dirs(ent->getLibPath());
        }
    }
    ~LibPathSetter () {
        // restore the default search path.
        if (_SetDllDirectory != NULL) {
            restore_libpath(ent->getLibPath());
        }
    }
};

/*
 * Function to load a dll and touch its pages. This function adds
 * the directories in the entries' libpath field to the DLL search
 * path. It will also attempt to lock the library into memory when
 * a QS_LOADLIB entry specifies locking. For QS_REFRESHLIB entries,
 * it will load any dependent libraries before attempting to load
 * the library indicated in the filename field. 
 */
void do_loadlib(QSEntry* ent) {

    jqs_info (3, "Loading library %s\n", ent->getFileName());

    TraceTime t1("Library load time", 1, print_times >= 1);

    LibPathSetter libPathSetter(ent);

    load_dependent_dlls(ent);

    HANDLE fh;
    {
        TraceTime t2("Open time", 1, print_times >= 2);

        fh = CreateFile(ent->getFileName(), 
                        GENERIC_READ,
                        FILE_SHARE_DELETE | FILE_SHARE_WRITE | FILE_SHARE_READ,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);

        if (fh == INVALID_HANDLE_VALUE) {
            jqs_warn ("Could not open \"%s\": error %d\n", ent->getFileName(), GetLastError());
            return;
        }
    }

    HMODULE hmodule;
    {
        TraceTime t1((string) "LoadLibraryEx time: " + ent->getFileName(), 1, print_times >= 1);
        hmodule = LoadLibraryEx(ent->getFileName(), NULL, DONT_RESOLVE_DLL_REFERENCES);
    }

    if (hmodule == NULL) {
        jqs_warn ("Could not load library \"%s\": error %d\n", ent->getFileName(), GetLastError());
        CloseHandle (fh);
        return;
    }

    ent->getOS().hModule = hmodule;

    MODULEINFO modinfo;

    { 
        TraceTime t1("GetModuleInformation time", 1, print_times >= 2);

        // get library addresses
        if (!_GetModuleInformation ||
            !_GetModuleInformation(hProcess, hmodule, &modinfo, sizeof(modinfo))) 
        {
            if (!_GetModuleInformation) {
                jqs_warn("Could not get \"%s\" library information: GetModuleInformation is not available\n",
                         ent->getFileName());
            } else {
                jqs_warn("Could not get \"%s\" library information: GetModuleInformation failed (error %d)\n",
                         ent->getFileName(), GetLastError());
            }

            CloseHandle (fh);
            do_unload(ent);
            return;
        }
    }

    // set up the fields to allow touch_pages to read the library's pages
    //
    ent->getOS().hFile = fh;
    ent->getOS().hDevNotify = state_terminating ? NULL : registerDeviceNotification (fh, ent->getFileName());
    ent->setMapping(modinfo.lpBaseOfDll, modinfo.SizeOfImage);

    // touch library pages
    //
    lock_or_touch(ent);
}

/*
 * Function to load the file described by the quick starter entry
 * with the requested attributes. If the entry requests locking
 * but the entry cannot be locked, the lock failure is announced,
 * but the lock request is ignored. If the entry requests large
 * pages, but large pages were not available, the failure is
 * announced, but the request is simply ignored.
 */
void do_load(QSEntry* ent) {

    assert (!ent->isLoaded());

    jqs_info (3, "Loading %s\n", ent->getFileName());
    jqs_info (4, "  file size = " UINT64_FORMAT "\n", ent->getFileSize());

    bool is_exe = false;
#if 0
    // EXPERIMENTAL: load / refresh mapped modes are deprecated for executable files,
    // loadlib / refreshlib modes should be used instead
    is_exe = is_executable(ent->getFileName())
#else
    assert (!is_executable(ent->getFileName()));
#endif

    HANDLE fh;
    HANDLE fmh;

    uint64_t fileSize64 = ent->getFileSize();
    size_t fileSize = (size_t) fileSize64;

    if (fileSize64 > (uint64_t) fileSize) {
        jqs_warn ("File %s is too large (" UINT64_FORMAT " bytes): unable to map it into memory\n", ent->getFileName(), fileSize64);
        return;
    }

    {
        TraceTime t2("Open time", 1, print_times >= 2);

        fh = CreateFile(ent->getFileName(),
                        is_exe ? (GENERIC_READ | GENERIC_EXECUTE | FILE_EXECUTE)
                               : GENERIC_READ,
                        FILE_SHARE_DELETE | FILE_SHARE_WRITE | FILE_SHARE_READ,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL|FILE_FLAG_SEQUENTIAL_SCAN,
                        NULL);

        if (fh == INVALID_HANDLE_VALUE) {
            jqs_warn ("Could not open \"%s\": error %d\n", ent->getFileName(), GetLastError());
            return;
        }
    }

    void* mapAddr = NULL;
    size_t mapSize = fileSize;

    DWORD cfmFlags = PAGE_READONLY;
    DWORD mapAccess = FILE_MAP_READ;

#if 0
    // EXPERIMENTAL: load / refresh mapped modes are deprecated for executable files,
    // loadlib / refreshlib modes should be used instead

    // Note: do_load() works incorrectly for executable file:
    // it is mapped as image (SEC_IMAGE), but memory is touched according to file size,
    // which could be large than image size.

    const bool is_exe = is_executable(ent->getFileName());

    if (is_exe) {
        jqs_info (4, "  file is an executable\n");
    }

    if(is_exe) {
        // note - PAGE_EXECUTE_READ is only available on
        // windows server 2003 SP1 and windows XP SP2. It's
        // likely that this code won't even run on windows 98
        // and windows 2000.
        //

        cfmFlags = PAGE_EXECUTE_READ | SEC_IMAGE;

        // FILE_MAP_EXECUTE is not defined in vc7 header files and
        // requires downloading a more recent Platform SDK.
        //
        mapAccess = FILE_MAP_EXECUTE | FILE_MAP_READ;
    }

    // EXPERIMENTAL: Windows does not support large pages for regular file mappings.
    //
    // MSDN description of SEC_LARGE_PAGES: Enables large pages to be used when mapping images
    // or backing from the pagefile, but not when mapping data for regular files.
    //
    // In fact, specifying SEC_LARGE_PAGES on Vista causes CreateFileMapping to fail with
    // error ERROR_INVALID_PARAMETER.
    //

    if (ent->getPageSize() != default_page_size && _GetLargePageMinimum != NULL &&
        !is_exe && have_lock_memory_privilege) 
    {
        // mapping length must be a multiple of the page size when
        // using large pages
        //
        mapSize = roundUp(mapSize, ent->getPageSize());

        cfmFlags |= SEC_LARGE_PAGES;
    }

#endif

    jqs_info (5, "  cfmFlags = %08x \n", cfmFlags);
    jqs_info (5, "  mapAccess = %08x \n", mapAccess);

    {
        TraceTime t2("Map time", 1, print_times >= 2);

        fmh = CreateFileMapping(fh, NULL, cfmFlags, 0,
                                (mapSize == fileSize) ? 0 : (DWORD)mapSize, NULL);

        if (fmh == NULL) {
            jqs_warn ("Could not create file mapping \"%s\": error %d\n",
                      ent->getFileName(), GetLastError());
            CloseHandle (fh);
            return;
        }

        HandleCloser fmhCloser(fmh);

        // map the file into memory 
        //
        mapAddr = (char*) MapViewOfFile(fmh, mapAccess, 0, 0,
                                        (mapSize == fileSize) ? 0 : mapSize);

        if (mapAddr == NULL) {
            jqs_warn("Could not map view of file \"%s\": error %d\n",
                     ent->getFileName(), GetLastError());
            CloseHandle (fh);
            return;
        }
    }

    ent->getOS().hFile = fh;
    ent->getOS().hDevNotify = state_terminating ? NULL : registerDeviceNotification (fh, ent->getFileName());
    ent->setMapping(mapAddr, mapSize);

    jqs_info (4, "  mapped " SIZET_FORMAT " bytes at " PTR_FORMAT "\n",
             ent->getMapSize(), ent->getMapAddr());

    lock_or_touch(ent);
}

/*
 * Function unload the quick starter file passed in ent.
 */
void do_unload(QSEntry* ent) {
    CriticalSection::Lock lock(PrefetchLock);

    if (ent->isLoaded()) {

        jqs_info (3, "Unloading %s\n", ent->getFileName());

        if (ent->isLocked()) {
            VirtualUnlock(ent->getMapAddr(), ent->getMapSize());
        }

        if (ent->getCmd() == QS_LOADLIB || ent->getCmd() == QS_REFRESHLIB) {
            HMODULE hModule = ent->getOS().hModule;
            if (hModule != NULL) {
                FreeLibrary(hModule);
                ent->getOS().hModule = NULL;
            }
        } else {
            UnmapViewOfFile(ent->getMapAddr());
        }

        if (have_inc_base_priority_privilege) {
            dec_process_working_set_size(ent->getMapSize());
        }

        unregisterDeviceNotification (ent->getOS().hDevNotify);
        ent->getOS().hDevNotify = NULL;

        if (ent->getOS().hFile) {
            CloseHandle (ent->getOS().hFile);
            ent->getOS().hFile = NULL;
        }

       if (ent->getOS().hDevice) {
            if (ent->getOS().hDevice != INVALID_HANDLE_VALUE) {
                CloseHandle (ent->getOS().hDevice);
            }
            ent->getOS().hDevice = NULL;
        }

        ent->setMapping(NULL, 0);
    }

    // unload all dependent libraries.
    //
    unload_dependent_dlls(ent);
}

/*
 * Helper function to read given number of bytes of the specified entry starting 
 * from given offset.
 */
void read_region (QSEntry* ent, HANDLE fh, std::vector<char>& buf, uint64_t start, uint64_t size) {
    LARGE_INTEGER li;
    li.QuadPart = start;
    li.LowPart = SetFilePointer (fh, li.LowPart, &li.HighPart, FILE_BEGIN);

    DWORD lastError = GetLastError();
    if (li.LowPart == INVALID_SET_FILE_POINTER && lastError != NO_ERROR) {
        jqs_warn ("File seek error \"%s\": error %d\n", ent->getFileName(), lastError);
        return;
    }

    uint64_t remaining = size;
    DWORD bufsize = (DWORD) buf.size ();

    while (remaining > 0) {
        CHECK_ASYNC_EVENTS();

        // the following cast does not cause truncation, since
        // min (remaining, (uint64_t) bufsize) <= bufsize, and bufsize is DWORD
        DWORD bsize = (DWORD) min (remaining, (uint64_t) bufsize);

        DWORD bread = 0;
        if (ReadFile(fh, &buf[0], bsize, &bread, NULL) != 0) {
            if (bread == 0) {
                // EOF reached
                break;
            }
            remaining -= bread;
        } else {
            DWORD lastError = GetLastError();
            jqs_warn ("File read error \"%s\": error %d\n", ent->getFileName(), lastError);
            return;
        }
    }
}

/*
 * For non-exe files in refresh mode, open-read-close semantics are
 * sufficient to populate the file system cache. This results is a lower
 * peak memory usage in refresh mode when large non-executable files are
 * refreshed.
 */
static void do_read(QSEntry* ent) {

    HANDLE fh;

    jqs_info (3, "Reading %s\n", ent->getFileName());
    TraceTime t1("File refresh time", 1, print_times >= 1);

    {
        TraceTime t2("Open time", 1, print_times >= 2);

        fh = CreateFile(ent->getFileName(), GENERIC_READ,
                        FILE_SHARE_DELETE | FILE_SHARE_WRITE | FILE_SHARE_READ,
                        NULL, OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                        NULL);

        if (fh == INVALID_HANDLE_VALUE) {
            jqs_warn ("Could not open \"%s\": error %d\n",
                      ent->getFileName(), GetLastError());
            return;
        }
    }

    HandleCloser fhCloser(fh);

    vector<char> buf(ent->getBufferSize());

    const Regions& regions = ent->getRegions ();

    uint64_t bytesRead = 0;

    if (!regions.empty()) {
        jqs_info (4, "  reading \"%s\" according to profile information\n", ent->getFileName());
        TraceTime t2("Read time", 1, print_times >= 2);

        for (size_t i = 0; i < regions.size (); i++) {
            const Region& region = regions [i];
            if (region.start >= ent->getFileSize()) {
                break;
            }
            uint64_t size = min(region.end, ent->getFileSize()) - region.start;
            read_region (ent, fh, buf, region.start, size);
            bytesRead += size;
        }

    } else {
        jqs_info (4, "  reading the whole file \"%s\" (no profile information)\n", ent->getFileName());
        TraceTime t2("Read time", 1, print_times >= 2);

        // read from each page of the mapping to force the pages
        // into physical memory.

        read_region (ent, fh, buf, 0, ent->getFileSize());
        bytesRead += ent->getFileSize();
    }

    double percentRead = 0.0;
    if (ent->getFileSize() != 0) {
        percentRead = (double)bytesRead / (double)ent->getFileSize() * 100.0;
    }
    jqs_info (4, "  read " UINT64_FORMAT " bytes of \"%s\" (%2.1f%%)\n",
             bytesRead, ent->getFileName(), percentRead);

    BytesActuallyRead += bytesRead;
    TotalBytesToRead += ent->getFileSize ();
}

/*
 * Simple helper class which calls do_unload() for given entry 
 * from destructor.
 */
class QSEntryUnloader {
    QSEntry* ent;
public:
    QSEntryUnloader (QSEntry* ent_)
        : ent (ent_)
    {}

    ~QSEntryUnloader () {
        do_unload(ent);
    }
};

/*
 * Function to refresh files. Refreshing a file involved touching
 * or read pages of the file such that the page is reloaded
 * into the file system's page cache.
 */
void do_refresh(QSEntry* ent) {

#if 0
    // EXPERIMENTAL: this piece of code makes sense only for Vista which 
    // supports I/O operations prioritization, however the experiments 
    // show that when the background priority is set, the refresh process
    // takes hundreds times as much time in comparison with normal execution.
    BackgroundPrioritySetter bps;
#endif

    switch (ent->getCmd()) {
        case QS_LOAD:
        case QS_LOADLIB:
            touch_pages(ent);
            break;

        case QS_REFRESH:
#if 0
            // EXPERIMENTAL: load / refresh mapped modes are deprecated for executable files,
            // loadlib / refreshlib modes should be used instead

            if (!ent->isMapped() && is_executable(ent->getFileName())) {
                // The user did not specify mapped mode, but we must use
                // mapped mode for exe files, as they must be loaded using file
                // mapping semantics to get the proper file caching effects.
                // non-exe files can be read into a buffer using open/read/write
                // semantics, which can greatly reduce the peak memory footprint
                // of the process. We set this entries cmd_options.refresh.mapped
                // variable to true here to force these entries through the 
                // do_load()/do_unload() code just above.
                //
                ent->setMapped(true);
            }
#endif

            if (ent->isMapped()) {
                QSEntryUnloader entUnloader (ent); // ensures that do_unload(ent)
                                                   // will be called even if
                                                   // do_load() throw an exception
                do_load(ent);
            } else {
                do_read(ent);
            }
            break;

        case QS_REFRESHLIB:
            {
                QSEntryUnloader entUnloader (ent); // ensures that do_unload(ent)
                                                   // will be called even if
                                                   // do_loadlib() throw an exception
                do_loadlib(ent);
            }
            break;
    }
}

/*
 * Function to read directory contents into the file system's page cache. 
 * This improves the speed of iteration of files in the directory. 
 * Note: directory prefetching does not cause reloading of the contents the 
 * files to file system's cache.
 */
void do_readdir (const string& dir, bool recursive) {

    WIN32_FIND_DATA findData;

    string mask = dir + FILE_SEPARATOR_CHAR + "*.*";

    HANDLE findHandle = FindFirstFile (mask.c_str(), &findData);
    if (findHandle == INVALID_HANDLE_VALUE) {
        return;
    }

    do {
        if (recursive &&
            (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            // skip "." and ".."
            if (!strcmp (findData.cFileName, ".") || !strcmp (findData.cFileName, "..")) {
                continue;
            }
            do_readdir(dir + FILE_SEPARATOR_CHAR + findData.cFileName, recursive);
        }
    } while (FindNextFile (findHandle, &findData));

    FindClose(findHandle);
}

/*
 * Function to read directory contents into the file system's page cache. 
 * This improves the speed of iteration of files in the directory. 
 * Note: directory prefetching does not cause reloading of the contents the 
 * files to file system's cache.
 */
void do_refreshdir(QSEntry* ent) {
    jqs_info (3, "Refreshing directory %s\n", ent->getFileName());

    {
        TraceTime t("Directory refresh time", print_times >= 1);
        do_readdir(ent->getFileName(), ent->isRecursive());
    }
}

/*
 * Function to sleep for sec seconds and then poll for high memory condition.
 * If a high memory condition exists, the function returns. Otherwise, it
 * sleeps for another sec seconds and then check again. The function will
 * not return until a high memory condition exists.
 */
void wait_high_mem_or_timeout_poll_ex(unsigned int sec) {
    BOOL low_memory = false;
    MEMORYSTATUSEX memx;

    assert(_GlobalMemoryStatusEx);

    while (true) {
        CHECK_ASYNC_EVENTS();

        WaitForSingleObject(hJQSNotificationEvent, sec*1000);

        memx.dwLength = sizeof(MEMORYSTATUSEX);
        _GlobalMemoryStatusEx(&memx);

        low_memory = (memx.ullAvailPhys < memoryLimit);

        jqs_info (4, "Memory status: total = " SIZET_FORMAT
                    " available = " SIZET_FORMAT
                    " limit = " SIZET_FORMAT "\n",
                    (size_t)memx.ullTotalPhys/M, 
                    (size_t)memx.ullAvailPhys/M, 
                    memoryLimit/M);

        if (!low_memory) {
            return;
        }
        jqs_info (3, "Low memory condition - skipping refresh operation\n");
    }
}

/*
 * Function to sleep for sec seconds and then poll for high memory condition.
 * If a high memory condition exists, the function returns. Otherwise, it
 * sleeps for another sec seconds and then check again. The function will
 * not return until a high memory condition exists.
 */
void wait_high_mem_or_timeout_poll(unsigned int sec) {
    BOOL low_memory = false;
    MEMORYSTATUS mem;

    while (true) {
        CHECK_ASYNC_EVENTS();

        WaitForSingleObject(hJQSNotificationEvent, sec*1000);

        GlobalMemoryStatus(&mem);

        low_memory = (mem.dwAvailPhys < memoryLimit);

        jqs_info (4, "Memory status: total = " SIZET_FORMAT
                    " available = " SIZET_FORMAT
                    " limit = " SIZET_FORMAT "\n",
                    mem.dwTotalPhys/M, mem.dwAvailPhys/M, memoryLimit/M);

        if (!low_memory) {
            return;
        }
        jqs_info (3, "Low memory condition - skipping refresh operation\n");
    }
}

/*
 * Function to sleep for sec seconds and then poll for high memory condition.
 * If a high memory condition exists, the function returns. Otherwise, it
 * sleeps for another sec seconds and then check again. The function will
 * not return until a high memory condition exists.
 * This function uses Windows' memory resource notifications to poll for
 * memory conditions.
 */
void wait_high_mem_or_timeout_notify(unsigned int sec) {
    static HANDLE lowMemHdl = NULL;
    static HANDLE highMemHdl = NULL;
    BOOL low_memory = false;
    BOOL high_memory = false;

    assert(_CreateMemoryResourceNotification);
    assert(_QueryMemoryResourceNotification);

    if (lowMemHdl == NULL) {
        lowMemHdl  = _CreateMemoryResourceNotification(LowMemoryResourceNotification);
        highMemHdl = _CreateMemoryResourceNotification(HighMemoryResourceNotification);
    }

    while (true) {
        CHECK_ASYNC_EVENTS();

        _QueryMemoryResourceNotification(lowMemHdl, &low_memory);
        _QueryMemoryResourceNotification(highMemHdl, &high_memory);

        if (low_memory || !high_memory) {
            jqs_info (3, "Low memory condition - blocking until cleared\n");
            WaitForSingleObject(highMemHdl, INFINITE);

            jqs_info (3, "Low memory condition cleared\n");
            return;

        } else if (high_memory) {
            HANDLE events[] = {hJQSNotificationEvent, lowMemHdl};
            DWORD rc = WaitForMultipleObjects(2, events, FALSE, sec*1000);
            if ((rc == WAIT_TIMEOUT) || (rc == WAIT_OBJECT_0)) {
                // timeout elapsed or notification signaled
                return;
            }
        }
    }
}

/*
 * Function to sleep for sec seconds and then poll for high memory condition.
 * If a high memory condition exists, the function returns. Otherwise, it
 * sleeps for another sec seconds and then check again. The function will
 * not return until a high memory condition exists.
 * The function chooses the best suitable implementation for current operating 
 * system.
 */
void wait_for_high_mem_or_timeout(unsigned int sec) {
    if (_CreateMemoryResourceNotification == NULL) {
        if (_GlobalMemoryStatusEx == NULL) {
            wait_high_mem_or_timeout_poll(sec);
        } else {
            wait_high_mem_or_timeout_poll_ex(sec);
        }
    } else {
        wait_high_mem_or_timeout_notify(sec);
    }
}

/*
 * Processes asynchronous events such as termination event or pause event.
 * If the event occur, throws proper exception.
 * This function is called from CHECK_ASYNC_EVENTS() macro's body.
 */
void processAsyncEvents() {
    assert(asyncEventOccured);
    asyncEventOccured = false;

    if (state_terminating) {
        throw TerminationEventException();
    
    } else if (state_paused) {
        throw PauseEventException();
    }
}

/*
 * Waits for the JQS to be resumed.
 */
void wait_for_resume() {
    for (;;) {
        if (!state_paused) {
            break;
        }
        WaitForSingleObject(hIdleEvent, INFINITE);
    }
}

/*
 * Pauses JQS operation.
 */
void pauseJQSService() {
    jqs_info (4, "Signaling PAUSE event.\n");
    state_paused = true;
    asyncEventOccured = true;
}

/*
 * Resumes JQS operation.
 */
void resumeJQSService() {
    jqs_info (4, "Signaling RESUME event.\n");
    state_paused = false;
    SetEvent(hIdleEvent);
}

/*
 * Notifies JQS to perform a refresh if system condition allows that.
 */
void notifyJQSService() {
    jqs_info (4, "Signaling NOTIFY event.\n");
    SetEvent(hJQSNotificationEvent);
}

/*
 * Arranges JQS termination sequence.
 */
void terminateJQSService() {
    jqs_info (4, "Signaling TERMINATE event.\n");
    state_terminating = true;
    asyncEventOccured = true;
    SetEvent(hJQSNotificationEvent);
}

/*
 * Auxiliary function, searches for the entry by file handle.
 */
static QSEntry* findQSEntryByFileHandle (HANDLE handle) {
    for (size_t i = 0; i < g_QSEntries.size(); i++) {
        QSEntry* entry = g_QSEntries[i];
        if (entry->getOS().hFile == handle) {
            return entry;
        }
        const QSEntries& dependentLibs = entry->getDependentLibs();
        for (size_t j = 0; j < dependentLibs.size (); j++) {
            QSEntry* entry = dependentLibs[j];
            if (entry->getOS().hFile == handle) {
                return entry;
            }
        }
    }
    return NULL;
}

/*
 * Unloads quick starter entry with specified file handle.
 */
void unloadQSEntryByFileHandle (HANDLE handle) {
    pauseJQSService();

    CriticalSection::Lock lock(PrefetchLock);

    QSEntry* entry = findQSEntryByFileHandle (handle);

    if (entry) {
        do_unload (entry);
    }

    // enable prefetcher
    state_paused = false;
}

/*
 * Returns true if the AC power status is online.
 */
bool checkPowerStatus() {
    if (!_GetSystemPowerStatus) {
        return true;
    }

    SYSTEM_POWER_STATUS powerStatus;
    if (!_GetSystemPowerStatus(&powerStatus)) {
        jqs_warn ("GetSystemPowerStatus failed (error %d)\n", GetLastError());
        return true;
    }

    if (powerStatus.ACLineStatus == 0) {
        jqs_info (3, "No AC power detected: device power state monitoring is enabled\n");
        return false;
    }
    return true;
}

const int PERF_MONITORING_INTERVAL = 1000;

/*
 * Obtains values of CPU usage and Disk I/O usage performance counters in a loop
 * waiting for values that fit the threshold values specified in the configuration 
 * file.
 * Note: the function uses different thresholds for the boot time refresh and for the 
 * ordinary refreshes.
 */
void waitForSystemIdle(bool boot) {
    if (boot) {
        TraceTime t("Initializing performance counters", print_times >= 2);

        performanceCountersMonitor = new PerformanceCountersMonitor();
        if (!performanceCountersMonitor->init ()) {
            delete performanceCountersMonitor;
            performanceCountersMonitor = NULL;
        }
    }

    if (!performanceCountersMonitor) {
        return;
    }

    PerfCountersThreshold* thresholds = boot ? &bootThresholds : &refreshThresholds;
    for (;;) {
        CHECK_ASYNC_EVENTS();

        {
            TraceTime t("Collecting performance counters", print_times >= 2);
            performanceCountersMonitor->collect ();
        }

        double diskTime      = performanceCountersMonitor->getCounterValue (PERF_COUNTER_DISK_TIME);
        double processorTime = performanceCountersMonitor->getCounterValue (PERF_COUNTER_PROCESSOR_TIME);

        if ((diskTime <= thresholds->diskTime) &&
            (processorTime <= thresholds->processorTime)) 
        {
            break;
        }
        jqs_info (3, "Waiting for system idle. Disk time %3.1f%%, Processor time %3.1f%%\n", 
                  diskTime, processorTime);
        WaitForSingleObject(hJQSNotificationEvent, PERF_MONITORING_INTERVAL);
    }
}


/*
 * Checks if device on which QS entry resides is going to suspend.
 * In such case, prefetching is skipped as it could awake device.
 */
bool checkForDeviceSuspend (QSEntry* ent) {
    if (!_GetDevicePowerState) {
        return true;
    }

    HANDLE hDevice = ent->getOS().hDevice;
    if (!hDevice) {
        hDevice = openDeviceByFileName (ent->getFileName());
        ent->getOS().hDevice = hDevice;
    }

    if (hDevice == INVALID_HANDLE_VALUE) {
        // device cannot be opened
        return true;
    }

    BOOL on = 0;
    if (!_GetDevicePowerState(hDevice, &on)) {
        jqs_warn ("Unable to get device power state: GetDevicePowerState failed (error %d)\n", GetLastError());
        on = 1;
    }
    if (!on) {
        jqs_info (3, "Device for \"%s\" is suspended - avoid prefetching\n", ent->getFileName());
        return false;
    }

    DISK_PERFORMANCE perf;
    DWORD bytesRet;
    if (!DeviceIoControl(hDevice,
                         IOCTL_DISK_PERFORMANCE,
                         NULL, 0,
                         &perf,
                         sizeof(perf),
                         &bytesRet,
                         NULL))
    {
        DWORD lastError = GetLastError();

        if (lastError == ERROR_NOT_SUPPORTED) {
            jqs_info (4, "Device for \"%s\" does not support performance metrics.\n", ent->getFileName());

            CloseHandle (hDevice);
            ent->getOS().hDevice = INVALID_HANDLE_VALUE;
        } else {
            jqs_warn ("Unable to get device performance metrics: DeviceIoControl failed (error %d)\n", lastError);
        }

        return true;
    }

    if (bytesRet < sizeof(perf)) {
        jqs_warn ("Unable to get device performance metrics: DeviceIoControl failed (returned %d bytes <%d)\n", bytesRet, sizeof(perf));
        return true;
    }

    DISK_PERFORMANCE& prevPerf = ent->getOS().devicePerfCounters;

    jqs_info (4, "%s: read  count %d => %d (delta = %d)\n", ent->getFileName(), prevPerf.ReadCount,  perf.ReadCount,  perf.ReadCount  - prevPerf.ReadCount);
    jqs_info (4, "%s: write count %d => %d (delta = %d)\n", ent->getFileName(), prevPerf.WriteCount, perf.WriteCount, perf.WriteCount - prevPerf.WriteCount);

    if ((perf.ReadCount  == prevPerf.ReadCount) && (perf.WriteCount == prevPerf.WriteCount)) {
        jqs_info (3, "Device for \"%s\" is going to suspended - avoid prefetching\n", ent->getFileName());
        return false;
    }

    return true;
}

/*
 * 
 */
void prefetchingFinished (QSEntry* ent) {
    HANDLE hDevice = ent->getOS().hDevice;

    if (!hDevice || (hDevice == INVALID_HANDLE_VALUE)) {
        return;
    }

    DISK_PERFORMANCE& perf = ent->getOS().devicePerfCounters;
    memset (&perf, 0, sizeof(perf));

    DWORD bytesRet;
    if (!DeviceIoControl(hDevice,
                         IOCTL_DISK_PERFORMANCE,
                         NULL, 0,
                         &perf,
                         sizeof(perf),
                         &bytesRet,
                         NULL))
    {
        DWORD lastError = GetLastError();

        if (lastError == ERROR_NOT_SUPPORTED) {
            jqs_info (4, "Device for \"%s\" does not support performance metrics.\n", ent->getFileName());

            CloseHandle (hDevice);
            ent->getOS().hDevice = INVALID_HANDLE_VALUE;
        } else {
            jqs_warn ("Unable to get device performance metrics: DeviceIoControl failed (error %d)\n", lastError);
        }

        return;
    }

    if (bytesRet < sizeof(perf)) {
        jqs_warn ("Unable to get device performance metrics: DeviceIoControl failed (returned %d bytes <%d)\n", bytesRet, sizeof(perf));
        return;
    }
}

