/*
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef OS_LAYER_HPP
#define OS_LAYER_HPP

#define _WIN32_WINNT 0x0501
#define NOMINMAX

#include <winsock2.h>
#include <windows.h>
#include <winioctl.h>
#include <psapi.h>   // only for dll loading interfaces
#include <Pbt.h>
#include <Dbt.h>
#include <Pdh.h>

#ifndef PROCESS_MODE_BACKGROUND_BEGIN
#define PROCESS_MODE_BACKGROUND_BEGIN 0x00100000
#endif

#ifndef PROCESS_MODE_BACKGROUND_END
#define PROCESS_MODE_BACKGROUND_END 0x00200000
#endif

#ifndef FILE_MAP_EXECUTE
#define FILE_MAP_EXECUTE 0x0020
#endif

#ifndef SEC_LARGE_PAGES
#define SEC_LARGE_PAGES  0x80000000     
#endif


/*
 * The following functions are only available on certain versions of
 * Windows. We dynamically resolve these functions so we can run on
 * just about any Windows version.
 */
typedef BOOL (WINAPI *SetDllDirectory_func_t)(LPCTSTR);
typedef BOOL (WINAPI *GlobalMemoryStatusEx_func_t)(LPMEMORYSTATUSEX);
typedef BOOL (WINAPI *QueryMemoryResourceNotification_func_t)(HANDLE, PBOOL);
typedef HANDLE (WINAPI *CreateMemoryResourceNotification_func_t)(MEMORY_RESOURCE_NOTIFICATION_TYPE);
typedef BOOL (WINAPI *GetNativeSystemInfo_func_t)(LPSYSTEM_INFO);
typedef BOOL (WINAPI *IsWow64Process_func_t)(HANDLE, PBOOL);
typedef BOOL (WINAPI *SetProcessWorkingSetSize_func_t)(HANDLE, SIZE_T, SIZE_T);
typedef SIZE_T (WINAPI *GetLargePageMinimum_func_t)(void);
typedef BOOL (WINAPI *GetProcessWorkingSetSize_func_t)(HANDLE, PSIZE_T, PSIZE_T);

typedef BOOL (WINAPI *AdjustTokenPrivileges_func_t)
           (HANDLE, BOOL, PTOKEN_PRIVILEGES, DWORD, PTOKEN_PRIVILEGES, PDWORD);
typedef BOOL (WINAPI *OpenProcessToken_func_t)(HANDLE, DWORD, PHANDLE);
typedef BOOL (WINAPI *LookupPrivilegeValue_func_t)(LPCTSTR, LPCTSTR, PLUID);
typedef BOOL (WINAPI *ChangeServiceConfig2_func_t) (SC_HANDLE, DWORD, LPVOID);

typedef BOOL (WINAPI *SetFilePointerEx_func_t) (HANDLE, LARGE_INTEGER, PLARGE_INTEGER, DWORD);
typedef BOOL (WINAPI *GetDevicePowerState_func_t) (HANDLE, BOOL*);


typedef BOOL (WINAPI *GetModuleInformation_func_t) (HANDLE, HMODULE, LPMODULEINFO, DWORD);
typedef DWORD (WINAPI *GetMappedFileName_func_t) (HANDLE, LPVOID, LPSTR, DWORD);
typedef BOOL (WINAPI *QueryWorkingSet_func_t) (HANDLE, PVOID, DWORD);
typedef BOOL (WINAPI* EnumProcessModules_func_t) (HANDLE, HMODULE *, DWORD, LPDWORD);

typedef SERVICE_STATUS_HANDLE (WINAPI *RegisterServiceCtrlHandlerEx_func_t) (LPCTSTR, 
                                                                             LPHANDLER_FUNCTION_EX, 
                                                                             LPVOID);

typedef HDEVNOTIFY (WINAPI *RegisterDeviceNotification_func_t) (HANDLE, LPVOID, DWORD);
typedef BOOL (WINAPI *UnregisterDeviceNotification_func_t) (HDEVNOTIFY);

typedef BOOL (WINAPI* GetSystemPowerStatus_func_t) (LPSYSTEM_POWER_STATUS);


typedef PDH_STATUS (WINAPI* PdhOpenQuery_func_t) (LPCSTR, DWORD_PTR, PDH_HQUERY*);
typedef PDH_STATUS (WINAPI* PdhAddCounter_func_t) (PDH_HQUERY, LPCSTR, DWORD_PTR, PDH_HCOUNTER*);
typedef PDH_STATUS (WINAPI* PdhRemoveCounter_func_t) (PDH_HCOUNTER);
typedef PDH_STATUS (WINAPI* PdhCollectQueryData_func_t) (PDH_HQUERY);
typedef PDH_STATUS (WINAPI* PdhCloseQuery_func_t) (PDH_HQUERY);
typedef PDH_STATUS (WINAPI* PdhGetFormattedCounterValue_func_t) (PDH_HCOUNTER, DWORD, LPDWORD, PPDH_FMT_COUNTERVALUE);
typedef PDH_STATUS (WINAPI* PdhMakeCounterPath_func_t) (PPDH_COUNTER_PATH_ELEMENTS_A, LPSTR, LPDWORD, DWORD);


extern SetDllDirectory_func_t                   _SetDllDirectory;
extern GlobalMemoryStatusEx_func_t              _GlobalMemoryStatusEx;
extern QueryMemoryResourceNotification_func_t   _QueryMemoryResourceNotification;
extern CreateMemoryResourceNotification_func_t  _CreateMemoryResourceNotification;
extern GetNativeSystemInfo_func_t               _GetNativeSystemInfo;
extern IsWow64Process_func_t                    _IsWow64Process;
extern SetProcessWorkingSetSize_func_t          _SetProcessWorkingSetSize;
extern GetProcessWorkingSetSize_func_t          _GetProcessWorkingSetSize;
extern GetLargePageMinimum_func_t               _GetLargePageMinimum;
extern AdjustTokenPrivileges_func_t             _AdjustTokenPrivileges;
extern OpenProcessToken_func_t                  _OpenProcessToken;
extern LookupPrivilegeValue_func_t              _LookupPrivilegeValue;
extern ChangeServiceConfig2_func_t              _ChangeServiceConfig2;
extern SetFilePointerEx_func_t                  _SetFilePointerEx;
extern GetDevicePowerState_func_t               _GetDevicePowerState;

extern GetModuleInformation_func_t              _GetModuleInformation;
extern GetMappedFileName_func_t                 _GetMappedFileName;
extern QueryWorkingSet_func_t                   _QueryWorkingSet;
extern EnumProcessModules_func_t                _EnumProcessModules;

extern RegisterServiceCtrlHandlerEx_func_t      _RegisterServiceCtrlHandlerEx;
extern RegisterDeviceNotification_func_t        _RegisterDeviceNotification;
extern UnregisterDeviceNotification_func_t      _UnregisterDeviceNotification;
extern GetSystemPowerStatus_func_t              _GetSystemPowerStatus;

extern PdhOpenQuery_func_t                      _PdhOpenQuery;
extern PdhAddCounter_func_t                     _PdhAddCounter;
extern PdhRemoveCounter_func_t                  _PdhRemoveCounter;
extern PdhCollectQueryData_func_t               _PdhCollectQueryData;
extern PdhCloseQuery_func_t                     _PdhCloseQuery;
extern PdhGetFormattedCounterValue_func_t       _PdhGetFormattedCounterValue;
extern PdhMakeCounterPath_func_t                _PdhMakeCounterPath;


#if !defined(_WIN64)
/*
 * These definitions were taken from the latest version of Windows Platform SDK.
 */

typedef union _PSAPI_WORKING_SET_BLOCK {
    ULONG_PTR Flags;
    struct {
        ULONG_PTR Protection  :5;
        ULONG_PTR ShareCount  :3;
        ULONG_PTR Shared  :1;
        ULONG_PTR Reserved  :3;
        ULONG_PTR VirtualPage  :20;
    };
} PSAPI_WORKING_SET_BLOCK,  *PPSAPI_WORKING_SET_BLOCK;

typedef struct _PSAPI_WORKING_SET_INFORMATION {
    ULONG_PTR NumberOfEntries;
    PSAPI_WORKING_SET_BLOCK WorkingSetInfo[1];
} PSAPI_WORKING_SET_INFORMATION,  *PPSAPI_WORKING_SET_INFORMATION;

#endif



/*
 * OS version information, initialized in initOSLayer().
 */
extern OSVERSIONINFO OSVersion;
/*
 * System information, initialized in initOSLayer().
 */
extern SYSTEM_INFO SystemInfo;

/*
 * Current process handle obtained by GetCurrentProcess(), initialized 
 * in initOSLayer().
 */
extern HANDLE CurProcessHandle;

/*
 * This function performs dynamic lookup of symbols that
 * may only exist in windows NT and later.
 */
extern void initOSLayer();

/*
 * Frees all resources used.
 */
extern void cleanupOSLayer();

#endif 
