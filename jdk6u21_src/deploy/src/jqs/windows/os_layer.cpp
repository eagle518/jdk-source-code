/*
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "os_layer.hpp"

#include "print.hpp"
#include "os_defs.hpp"
#include "utils.hpp"
#include "os_utils.hpp"


/*
 * The following functions are only available on certain versions of
 * Windows. We dynamically resolve these functions so we can run on
 * just about any Windows version.
 */
SetDllDirectory_func_t                  _SetDllDirectory = NULL;
GlobalMemoryStatusEx_func_t             _GlobalMemoryStatusEx = NULL;
QueryMemoryResourceNotification_func_t  _QueryMemoryResourceNotification = NULL;
CreateMemoryResourceNotification_func_t _CreateMemoryResourceNotification = NULL;
GetNativeSystemInfo_func_t              _GetNativeSystemInfo = NULL;
IsWow64Process_func_t                   _IsWow64Process = NULL;
SetProcessWorkingSetSize_func_t         _SetProcessWorkingSetSize = NULL;
GetProcessWorkingSetSize_func_t         _GetProcessWorkingSetSize = NULL;
GetLargePageMinimum_func_t              _GetLargePageMinimum = NULL;
AdjustTokenPrivileges_func_t            _AdjustTokenPrivileges = NULL;
OpenProcessToken_func_t                 _OpenProcessToken = NULL;
LookupPrivilegeValue_func_t             _LookupPrivilegeValue = NULL;
ChangeServiceConfig2_func_t             _ChangeServiceConfig2 = NULL;
SetFilePointerEx_func_t                 _SetFilePointerEx = NULL;
GetDevicePowerState_func_t              _GetDevicePowerState = NULL;

GetModuleInformation_func_t             _GetModuleInformation = NULL;
GetMappedFileName_func_t                _GetMappedFileName = NULL;
QueryWorkingSet_func_t                  _QueryWorkingSet = NULL;
RegisterServiceCtrlHandlerEx_func_t     _RegisterServiceCtrlHandlerEx = NULL;
RegisterDeviceNotification_func_t       _RegisterDeviceNotification = NULL;
UnregisterDeviceNotification_func_t     _UnregisterDeviceNotification = NULL;
GetSystemPowerStatus_func_t             _GetSystemPowerStatus = NULL;
EnumProcessModules_func_t               _EnumProcessModules = NULL;

PdhOpenQuery_func_t                     _PdhOpenQuery = NULL;
PdhAddCounter_func_t                    _PdhAddCounter = NULL;
PdhRemoveCounter_func_t                 _PdhRemoveCounter = NULL;
PdhCollectQueryData_func_t              _PdhCollectQueryData = NULL;
PdhCloseQuery_func_t                    _PdhCloseQuery = NULL;
PdhGetFormattedCounterValue_func_t      _PdhGetFormattedCounterValue = NULL;
PdhMakeCounterPath_func_t               _PdhMakeCounterPath = NULL;


/*
 * OS version information, initialized in initOSLayer().
 */
OSVERSIONINFO OSVersion;

/*
 * System information, initialized in initOSLayer().
 */
SYSTEM_INFO SystemInfo;

/*
 * Current process handle obtained by GetCurrentProcess(), initialized 
 * in initOSLayer().
 */
HANDLE CurProcessHandle = NULL;

/*
 * Handles for system libraries.
 */
static HINSTANCE kernel32 = NULL;
static HINSTANCE advapi32 = NULL;
static HINSTANCE user32   = NULL;
static HINSTANCE psapi    = NULL;
static HINSTANCE pdh      = NULL;


/*
 * Auxiliary function, initializes given func with the function address 
 * obrained, logs results.
 */
template<typename T>
static void InitFunc (T& func, HINSTANCE library, const char* name) {
    func = (T) GetProcAddress(library, name);
    jqs_info (5, "System function %s = " PTR_FORMAT "\n", name, func);
}

/*
 * This function performs dynamic lookup of symbols that
 * may only exist in windows NT and later.
 */
void initOSLayer() {

    // do not show any message boxes on fatal errors
    SetErrorMode(SEM_FAILCRITICALERRORS);

    OSVersion.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
    GetVersionEx (&OSVersion);

    GetSystemInfo(&SystemInfo);

    if ((OSVersion.dwPlatformId == VER_PLATFORM_WIN32s) ||
        (OSVersion.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS))
    {
        jqs_error("Java Quick Starter does not support your version of Windows\n");
        jqs_exit(1);
    }

    kernel32 = LoadLibrary("kernel32.dll");
    if (!kernel32) {
        jqs_error ("Unable to load kernel32.dll (error %d)\n", GetLastError ());
        jqs_exit (1);
    }

    advapi32 = LoadLibrary("advapi32.dll");
    if (!advapi32) {
        jqs_error ("Unable to load advapi32.dll (error %d)\n", GetLastError ());
        jqs_exit (1);
    }

    user32 = LoadLibrary("user32.dll");
    if (!user32) {
        jqs_error ("Unable to load user32.dll (error %d)\n", GetLastError ());
        jqs_exit (1);
    }

    psapi = LoadLibrary("psapi.dll");
    if (!psapi) {
        jqs_warn("Unable to load psapi.dll (error %d)\n", GetLastError ());
    }

    pdh = LoadLibrary("pdh.dll");
    if (!pdh) {
        jqs_warn("Unable to load pdh.dll (error %d)\n", GetLastError ());
    }

    InitFunc (_SetDllDirectory, kernel32, "SetDllDirectoryA");
    InitFunc (_GlobalMemoryStatusEx, kernel32, "GlobalMemoryStatusEx");
    InitFunc (_QueryMemoryResourceNotification, kernel32, "QueryMemoryResourceNotification");
    InitFunc (_CreateMemoryResourceNotification, kernel32, "CreateMemoryResourceNotification");
    InitFunc (_GetNativeSystemInfo, kernel32, "GetNativeSystemInfo");
    InitFunc (_IsWow64Process, kernel32, "IsWow64Process");
    InitFunc (_SetProcessWorkingSetSize, kernel32, "SetProcessWorkingSetSize");
    InitFunc (_GetProcessWorkingSetSize, kernel32, "GetProcessWorkingSetSize");
    InitFunc (_GetLargePageMinimum, kernel32, "GetLargePageMinimum");
    InitFunc (_GetSystemPowerStatus, kernel32, "GetSystemPowerStatus");
    InitFunc (_SetFilePointerEx, kernel32, "SetFilePointerEx");    
    InitFunc (_GetDevicePowerState, kernel32, "GetDevicePowerState");  

    InitFunc (_AdjustTokenPrivileges, advapi32, "AdjustTokenPrivileges");
    InitFunc (_OpenProcessToken, advapi32, "OpenProcessToken");
    InitFunc (_LookupPrivilegeValue, advapi32, "LookupPrivilegeValueA");
    InitFunc (_ChangeServiceConfig2, advapi32, "ChangeServiceConfig2A");
    InitFunc (_RegisterServiceCtrlHandlerEx, advapi32, "RegisterServiceCtrlHandlerExA");

    InitFunc (_RegisterDeviceNotification, user32, "RegisterDeviceNotificationA");
    InitFunc (_UnregisterDeviceNotification, user32, "UnregisterDeviceNotification");

    if (psapi) {
        InitFunc (_GetModuleInformation, psapi, "GetModuleInformation");
        InitFunc (_GetMappedFileName, psapi, "GetMappedFileNameA");
        InitFunc (_QueryWorkingSet, psapi, "QueryWorkingSet");
        InitFunc (_EnumProcessModules, psapi, "EnumProcessModules");
    }

    if (pdh) {
        InitFunc (_PdhOpenQuery, pdh, "PdhOpenQueryA");
        InitFunc (_PdhAddCounter, pdh, "PdhAddCounterA");
        InitFunc (_PdhRemoveCounter, pdh, "PdhRemoveCounter");
        InitFunc (_PdhCollectQueryData, pdh, "PdhCollectQueryData");
        InitFunc (_PdhCloseQuery, pdh, "PdhCloseQuery");
        InitFunc (_PdhGetFormattedCounterValue, pdh, "PdhGetFormattedCounterValue");
        InitFunc (_PdhMakeCounterPath, pdh, "PdhMakeCounterPathA");
    }

    CurProcessHandle = GetCurrentProcess();
}

/*
 * Frees all resources used.
 */
void cleanupOSLayer() {
    _SetDllDirectory = NULL;
    _GlobalMemoryStatusEx = NULL;
    _QueryMemoryResourceNotification = NULL;
    _CreateMemoryResourceNotification = NULL;
    _GetNativeSystemInfo = NULL;
    _IsWow64Process = NULL;
    _SetProcessWorkingSetSize = NULL;
    _GetProcessWorkingSetSize = NULL;
    _GetLargePageMinimum = NULL;
    _AdjustTokenPrivileges = NULL;
    _OpenProcessToken = NULL;
    _LookupPrivilegeValue = NULL;
    _ChangeServiceConfig2 = NULL;
    _SetFilePointerEx = NULL;
    _GetDevicePowerState = NULL;

    _GetModuleInformation = NULL;
    _GetMappedFileName = NULL;
    _QueryWorkingSet = NULL;
    _RegisterServiceCtrlHandlerEx = NULL;
    _RegisterDeviceNotification = NULL;
    _UnregisterDeviceNotification = NULL;
    _GetSystemPowerStatus = NULL;
    _EnumProcessModules = NULL;

    _PdhOpenQuery = NULL;
    _PdhAddCounter = NULL;
    _PdhRemoveCounter = NULL;
    _PdhCollectQueryData = NULL;
    _PdhCloseQuery = NULL;
    _PdhGetFormattedCounterValue = NULL;
    _PdhMakeCounterPath = NULL;

    if (pdh) {
        FreeLibrary(pdh);
    }
    if (psapi) {
        FreeLibrary(psapi);
    }
    if (user32) {
        FreeLibrary(user32);
    }
    if (advapi32) {
        FreeLibrary(advapi32);
    }
    if (kernel32) {
        FreeLibrary(kernel32);
    }
}
