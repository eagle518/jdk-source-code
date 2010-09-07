/*
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "os_layer.hpp"

#include <stdio.h> 
#include <stdlib.h>
#include <stdarg.h>
#include <sys/stat.h>

#include <sstream>

#include "jqs.hpp"
#include "parse.hpp"
#include "print.hpp"
#include "utils.hpp"
#include "service.hpp"
#include "thread.hpp"
#include "prefetch.hpp"
#include "os_utils.hpp"
#include "timer.hpp"

using namespace std;


/*
 * Current service status value, one of the SERVICE_*.
 */
static DWORD currentServiceStatus;

/*
 * The JQS service handler.
 */
static SERVICE_STATUS_HANDLE scmHandle;

/*
 * The termination event is signaled when a service control handler function
 * receives SERVICE_CONTROL_SHUTDOWN or SERVICE_CONTROL_STOP event from
 * the system and this causes the main service function to finish.
 */
static HANDLE terminationEvent;


/*
 * The main JQS operations, such as loading and refreshing quick starter entries 
 * specified in the configuration file, are executed in this thread.
 */
class ServiceThread : public Thread {
public:
    virtual void run () {
        jqs_info (1, "%s service started\n", JQS_SERVICE_DISPLAY_NAME);
        do_jqs(interval);
        cleanup();
    }
};

/*
 * The service thread instance.
 */
static ServiceThread serviceThread;


/*
 * Informs the system about current service status.
 */
static BOOL UpdateServiceStatus(DWORD newState, DWORD SsExitCode,
                                DWORD checkPoint, DWORD waitHint) 
{
    SERVICE_STATUS status;

    status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    status.dwCurrentState = newState;

    if (newState == SERVICE_START_PENDING) {
        status.dwControlsAccepted = 0;
    } else {
        status.dwControlsAccepted = SERVICE_ACCEPT_STOP |
                                    SERVICE_ACCEPT_PAUSE_CONTINUE |
                                    SERVICE_ACCEPT_SHUTDOWN;

        if (_RegisterServiceCtrlHandlerEx) {
            status.dwControlsAccepted |= SERVICE_ACCEPT_POWEREVENT;

            if (capabilityUserLogonNotifications) {
                status.dwControlsAccepted |= SERVICE_ACCEPT_SESSIONCHANGE;
            }
        }
    }

    status.dwServiceSpecificExitCode = SsExitCode;
    status.dwWin32ExitCode = (SsExitCode != 0 ? ERROR_SERVICE_SPECIFIC_ERROR : NO_ERROR);

    status.dwCheckPoint = checkPoint;
    status.dwWaitHint = waitHint;

    if (!SetServiceStatus(scmHandle, &status)) {
        jqs_warn("Unable to set service status %d: SetServiceStatus failed (error %d)\n", newState, GetLastError());
        return false;
    }
    if (newState == SERVICE_STOPPED) {
        jqs_info (1, "%s service stopped\n", JQS_SERVICE_DISPLAY_NAME);
    }
    return true;
}

/*
 * The implementation of the callback function called when the jqs_exit() is
 * requested.
 */
static void at_service_exit(int code) {
    assert (service_mode);

    if (currentServiceStatus != SERVICE_STOPPED) {
        currentServiceStatus = SERVICE_STOPPED;
        UpdateServiceStatus(SERVICE_STOPPED, code, 0, 0);
    }
}

/*
 * Service control handler function.
 */
static void WINAPI ServiceControlHandler(DWORD dwControl) {
    switch (dwControl) {
        case SERVICE_CONTROL_PAUSE:
            if (currentServiceStatus == SERVICE_RUNNING) {
                UpdateServiceStatus(SERVICE_PAUSE_PENDING, 0, 1, 120000);
                pauseJQSService();
                currentServiceStatus = SERVICE_PAUSED;
                UpdateServiceStatus(SERVICE_PAUSED, 0, 0, 0);
                return;
            }
            break;

        case SERVICE_CONTROL_CONTINUE:
            if (currentServiceStatus == SERVICE_PAUSED) {
                UpdateServiceStatus(SERVICE_CONTINUE_PENDING, 0, 1, 2000);
                resumeJQSService();
                currentServiceStatus = SERVICE_RUNNING;
                UpdateServiceStatus(SERVICE_RUNNING, 0, 0, 0);
                return;
            }
            break;

        case SERVICE_CONTROL_INTERROGATE:
            break;

        case SERVICE_CONTROL_SHUTDOWN:
        case SERVICE_CONTROL_STOP:
            if (currentServiceStatus != SERVICE_STOPPED) {
                UpdateServiceStatus(SERVICE_STOP_PENDING, 0, 1, 2000);
                terminateJQSService();
                SetEvent(terminationEvent);
                currentServiceStatus = SERVICE_STOPPED;
                UpdateServiceStatus(SERVICE_STOPPED, 0, 0, 0);
                return;
            }
            break;

        default:
            // ignore unknown actions
            break;
    }

    UpdateServiceStatus(currentServiceStatus, 0, 0, 0);
}

/*
 * Service control handler function. This handler is used if the system
 * supports RegisterServiceCtrlHandlerEx().
 */
static DWORD WINAPI ServiceControlHandlerEx(DWORD dwControl, 
                                            DWORD dwEventType, 
                                            LPVOID lpEventData, 
                                            LPVOID lpContext) 
{
    switch (dwControl) {
        case SERVICE_CONTROL_DEVICEEVENT:
            if (dwEventType == DBT_DEVICEQUERYREMOVE) {
                jqs_info (4, "Received SERVICE_CONTROL_DEVICEEVENT,  dwEventType=DBT_DEVICEQUERYREMOVE\n");

                DEV_BROADCAST_HDR* hdr = (DEV_BROADCAST_HDR*)lpEventData;
                if (hdr->dbch_devicetype == DBT_DEVTYP_VOLUME) {
                    DEV_BROADCAST_VOLUME* devinfo = (DEV_BROADCAST_VOLUME*)hdr;
                    jqs_info (5, "dbch_devicetype=DBT_DEVTYP_VOLUME, dbcv_unitmask=%#x\n", devinfo->dbcv_unitmask);
                
                } else if (hdr->dbch_devicetype == DBT_DEVTYP_HANDLE) {
                    DEV_BROADCAST_HANDLE* devinfo = (DEV_BROADCAST_HANDLE*)hdr;
                    jqs_info (5, "dbch_devicetype=DBT_DEVTYP_HANDLE, dbch_handle=%#x\n", devinfo->dbch_handle);
                    unloadQSEntryByFileHandle (devinfo->dbch_handle);

                } else {
                    jqs_info (5, "dbch_devicetype=%#x\n", hdr->dbch_devicetype);
                }

            } else {
                jqs_info (4, "Received SERVICE_CONTROL_DEVICEEVENT dwEventType=%#x\n", dwEventType);
            }
            break;

        case SERVICE_CONTROL_SESSIONCHANGE:
            if (dwEventType == WTS_SESSION_LOGON) {
                jqs_info (4, "Received SERVICE_CONTROL_SESSIONCHANGE, dwEventType=WTS_SESSION_LOGON\n");
                notifyJQSService();

            } else if (dwEventType == WTS_CONSOLE_CONNECT) {
                jqs_info (4, "Received SERVICE_CONTROL_SESSIONCHANGE, dwEventType=WTS_CONSOLE_CONNECT\n");
                notifyJQSService();

            } else {
                jqs_info (4, "Received SERVICE_CONTROL_SESSIONCHANGE, dwEventType=%#x\n", dwEventType);
            }
            break;

        default:
            ServiceControlHandler(dwControl);
            return NO_ERROR;
    }

    UpdateServiceStatus(currentServiceStatus, 0, 0, 0);
    return NO_ERROR;
}

/*
 * Enables device notification events for the file. These events are signaled to 
 * the service for each file located on the device that the system tries to unmount.
 * The file name passed is used in error reporting.
 * Returns device notification handle.
 */
HDEVNOTIFY registerDeviceNotification(HANDLE hFile, const char* fileName) {
    if (!service_mode) {
        return NULL;
    }

    if (!_RegisterServiceCtrlHandlerEx || 
        !_RegisterDeviceNotification ||
        !_UnregisterDeviceNotification) 
    {
        return NULL;
    }

    DEV_BROADCAST_HANDLE devinfo;
    memset (&devinfo, 0, sizeof (devinfo));
    devinfo.dbch_size = sizeof (devinfo);
    devinfo.dbch_devicetype = DBT_DEVTYP_HANDLE;
    devinfo.dbch_handle = hFile;

    HDEVNOTIFY hDevNotify = _RegisterDeviceNotification(scmHandle, &devinfo, DEVICE_NOTIFY_SERVICE_HANDLE);
    if (!hDevNotify) {
        jqs_warn ("Device event notifications for \"%s\" are not available: RegisterDeviceNotification failed (error %d)\n", fileName, GetLastError ());
    }
    return hDevNotify;
}

/*
 * Unregisters device notification by handle.
 */
void unregisterDeviceNotification(HDEVNOTIFY hDevNotify) {
    if (hDevNotify) {
        assert (service_mode);
        assert (_UnregisterDeviceNotification);
        _UnregisterDeviceNotification(hDevNotify);
    }
}

/*
 * The JQS service body.
 */
static void WINAPI JqsServiceMain(int argc, char** argv) {

    TRY {

        // register the service control handler
        //
        if (_RegisterServiceCtrlHandlerEx) {
            scmHandle = _RegisterServiceCtrlHandlerEx(JQS_SERVICE_NAME,
                                                      (LPHANDLER_FUNCTION_EX) ServiceControlHandlerEx,
                                                      NULL);

        } else {
            scmHandle = RegisterServiceCtrlHandler(JQS_SERVICE_NAME,
                                                   (LPHANDLER_FUNCTION) ServiceControlHandler);
        }

        if (scmHandle == 0) {
            jqs_error("Unable to initialize service: RegisterServiceCtrlHandler failed (error %d)\n",
                       GetLastError());
            return;
        }

        // report progress to the SCM
        //
        currentServiceStatus = SERVICE_START_PENDING;
        if (!UpdateServiceStatus(SERVICE_START_PENDING, 0, 1, 5000)) {
            return;
        }

        jqs_at_exit(at_service_exit);

        // create our termination event object
        //
        if ((terminationEvent = CreateEvent(0, TRUE, FALSE, 0)) == 0) {
            DWORD lastError = GetLastError();
            currentServiceStatus = SERVICE_STOPPED;
            jqs_error("Unable to initialize service: CreateEvent failed (error %d)\n", lastError);
            UpdateServiceStatus(SERVICE_STOPPED, 0, 0, 0);
            return;
        }

        // report progress to the SCM
        //
        if (!UpdateServiceStatus(SERVICE_START_PENDING, 0, 2, 1000)) {
            return;
        }

        initialize();

        // report progress to the SCM
        //
        if (!UpdateServiceStatus(SERVICE_START_PENDING, 0, 3, 5000)) {
            return;
        }

        // read configuration file
        //
        parseConfigFile(configFileName);
        if (g_QSEntries.empty()) {
            currentServiceStatus = SERVICE_STOPPED;
            jqs_error("No entries found in %s\n", configFileName.c_str ());
            UpdateServiceStatus(SERVICE_STOPPED, 1, 0, 0);
            return;
        }

        if(!profileFileName.empty()) {
            parseProfile(profileFileName);
        }

        // report progress to the SCM
        //
        if (!UpdateServiceStatus(SERVICE_START_PENDING, 0, 4, 5000)) {
            return;
        }

        // start the service thread
        //
        if (!serviceThread.start()) {
            currentServiceStatus = SERVICE_STOPPED;
            jqs_error("Failed to start JQS service thread\n");
            UpdateServiceStatus(SERVICE_STOPPED, 0, 0, 0);
            return;
        }

        // report progress to the SCM
        //
        currentServiceStatus = SERVICE_RUNNING;
        if (!UpdateServiceStatus(SERVICE_RUNNING, 0, 0, 0)) {
            return;
        }

        // wait for termination event notification
        //
        WaitForSingleObject(terminationEvent, INFINITE);

    } CATCH_SYSTEM_EXCEPTIONS {
        jqs_exit(1);
    }
}

/*
 * The JQS service body.
 */
void runService() {

    addEventSource ();

    // our service table entry
    //
    SERVICE_TABLE_ENTRY stEntry[] = {
        { JQS_SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION) JqsServiceMain },
        { NULL, NULL }
    };

    // register the service with the SCM
    //
    if (!StartServiceCtrlDispatcher(stEntry)) {
        jqs_error("Could not register service with the service manager: StartServiceCtrlDispatcher failed (error %d)\n", GetLastError());
    }
}


//////////////////////////////////////////////////////////////////////////

/*
 * Helper function to call specified entry point function such as DllRegisterServer
 * or DllUnregisterServer from given library.
 */
static HRESULT callDLLEntryPoint (const char* lib, const char* entryPointName) {
    // Initialize OLE. 
    if (FAILED(OleInitialize(NULL))) { 
        jqs_error ("Failed to initialize OLE (error %d)\n", GetLastError ());
        return E_FAIL; 
    } 

    OLEFinalizer oleFinalizer;

    HMODULE hmodule = LoadLibrary (lib);
    if (hmodule == NULL) {
        jqs_error ("Failed to load library %s (error %d)\n", lib, GetLastError ());
        return E_FAIL;
    }

    LibraryCloser libCloser(hmodule);

    typedef HRESULT (STDAPICALLTYPE * DllEntryPoint)(void); 

    DllEntryPoint dllEntryPoint = (DllEntryPoint) GetProcAddress (hmodule, entryPointName);
    if (dllEntryPoint == NULL) {
        jqs_error ("Failed to find entry point %s in library %s (error %d)\n", 
            entryPointName, lib, GetLastError ());
        return E_FAIL;
    }

    jqs_info (4, "Function called %s:%s\n", lib, entryPointName);

    return dllEntryPoint ();
}

/*
 * Registers IE start detector plugin.
 */
void registerIEStartDetector () {
    // IE plug-in
    string ieplugin = getSatelliteFileName (JQS_IE_PLUGIN_NAME);

    // register COM server
    HRESULT res = callDLLEntryPoint (ieplugin.c_str (), "DllRegisterServer");

    if (res != S_OK) {
        jqs_error ("Failed to register IE start detector. Error %x\n", res);
        return;
    }
}

/*
 * Unregisters IE start detector plugin.
 */
void unregisterIEStartDetector () {
    // IE plug-in
    string ieplugin = getSatelliteFileName (JQS_IE_PLUGIN_NAME);

    // unregister COM server
    HRESULT res = callDLLEntryPoint (ieplugin.c_str (), "DllUnregisterServer");
    if (res != S_OK) {
        if ((res == TYPE_E_REGISTRYACCESS) || (res == TYPE_E_CANTLOADLIBRARY)) {
            jqs_error ("Failed to unregister IE start detector: JQS is not registered\n");
        } else {
            jqs_error ("Failed to unregister IE start detector. Error %x\n", res);
        }
        return;
    }
}

//////////////////////////////////////////////////////////////////////////

const char* FF_EXTENSIONS_KEY = "SOFTWARE\\Mozilla\\Firefox\\extensions";

/*
 * Registers FireFox start detector extension.
 */
static void registerFFStartDetector () {
    HKEY hk;
    DWORD dwDisp; 
    LONG err;

    err = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                         FF_EXTENSIONS_KEY,
                         0, NULL, REG_OPTION_NON_VOLATILE,
                         KEY_WRITE, NULL, &hk, &dwDisp);
    
    if (err != ERROR_SUCCESS) {
        jqs_warn("Could not create the registry key HKEY_LOCAL_MACHINE\\%s. Error %d\n", 
            FF_EXTENSIONS_KEY, err);
        return;
    }

    RegKeyCloser keyCloser(hk);

    string ffplugindir = getFullPath (getSatelliteFileName (JQS_FF_PLUGIN_DIR));

    err = RegSetValueEx(hk,
                        JQS_FF_PLUGIN_ID,
                        0,
                        REG_EXPAND_SZ,
                        (LPBYTE) ffplugindir.c_str(),
                        (DWORD) ffplugindir.length() + 1);
    
    if (err != ERROR_SUCCESS) {
        jqs_error("Could not add Firefox extensions directory. Error %d\n", err); 
        return;
    }
}

/*
 * Unregisters FireFox start detector extension.
 */
static void unregisterFFStartDetector () {
    HKEY hk;
    LONG err;

    err = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                       FF_EXTENSIONS_KEY,
                       0,
                       KEY_WRITE, &hk);
    
    if (err != ERROR_SUCCESS) {
        if (err == ERROR_FILE_NOT_FOUND) {
            jqs_info (4, "Registry key HKEY_LOCAL_MACHINE\\%s does not exist\n", 
                FF_EXTENSIONS_KEY);

        } else {
            jqs_warn("Could not open the registry key HKEY_LOCAL_MACHINE\\%s. Error %d\n", 
                FF_EXTENSIONS_KEY, err);
        }
        return;
    }

    RegKeyCloser keyCloser(hk);

    err = RegDeleteValue(hk, JQS_FF_PLUGIN_ID);
    if (err != ERROR_SUCCESS) {
        if (err == ERROR_FILE_NOT_FOUND) {
            jqs_info (4, "Registry value HKEY_LOCAL_MACHINE\\%s\\%s does not exist\n", 
                FF_EXTENSIONS_KEY, JQS_FF_PLUGIN_ID);

        } else {
            jqs_warn("Could not delete the registry value HKEY_LOCAL_MACHINE\\%s\\%s. Error %d\n", 
                FF_EXTENSIONS_KEY, JQS_FF_PLUGIN_ID, err);
        }
        return;
    }
}


//////////////////////////////////////////////////////////////////////////

/*
 * Installs JQS service and registers IE and FireFox startup detector plugins.
 */
bool installService() {
    configFileName = getFullPath(configFileName);

    struct stat statbuf;
    if (stat(configFileName.c_str(), &statbuf) < 0) {
        jqs_error("Could not find %s: %s\n", configFileName.c_str(), strerror(errno));
        return false;
    }

    // register browser startup detectors
    registerIEStartDetector ();
    registerFFStartDetector ();

    ostringstream scmd;
    scmd << "\"" << getMyFileName() << "\"";
    scmd << " " << CMDLINE_ARG_SERVICE;
    scmd << " " << CMDLINE_ARG_CONFIG << " \"" << configFileName << "\"";
    if (!logFileName.empty()) {
        scmd << " " << CMDLINE_ARG_LOGFILE << " \"" << getFullPath(logFileName) << "\"";
    }
    if (!profileFileName.empty()) {
        if (stat(profileFileName.c_str(), &statbuf) < 0) {
            jqs_warn("Could not find profile \"%s\": %s\n", profileFileName.c_str(), strerror(errno));
        }
        scmd << " " << CMDLINE_ARG_PROFILE << " \"" << getFullPath(profileFileName) << "\"";
    }
    if (verbose > 0) {
        scmd << " " << CMDLINE_ARG_VERBOSE << " " << verbose;
    }
    if (print_times > 0) {
        scmd << " " << CMDLINE_ARG_TIMING << " " << print_times;
    }
    string cmd = scmd.str();

    jqs_info (1, "Installing JQS service: %s\n", cmd.c_str());

    SC_HANDLE scmHandle = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
    if (scmHandle == NULL) {
        jqs_error("Unable to install service: OpenSCManager failed (error %d)\n", GetLastError());
        return false;
    }

    SC_HANDLE service = CreateService(scmHandle,
                                      JQS_SERVICE_NAME,
                                      JQS_SERVICE_DISPLAY_NAME,
                                      SERVICE_ALL_ACCESS,
                                      SERVICE_WIN32_OWN_PROCESS,
                                      SERVICE_AUTO_START,
                                      SERVICE_ERROR_NORMAL,
                                      cmd.c_str(),
                                      NULL, NULL, NULL, NULL, NULL);
    if (service == NULL) {
        DWORD lastError = GetLastError();
        if (lastError == ERROR_SERVICE_EXISTS) {
            jqs_error("Unable to register service: %s already installed\n", JQS_SERVICE_DISPLAY_NAME);
        } else {
            jqs_error("Unable to register service: CreatedService failed (error %d)\n", lastError);
        }
        return false;
    }

    // need at least Windows 2000 to set description

    if ((OSVersion.dwPlatformId == VER_PLATFORM_WIN32_NT) &&
        (OSVersion.dwMajorVersion >= 5))
    {
        if (_ChangeServiceConfig2 != NULL) {
            SERVICE_DESCRIPTION desc = {JQS_SERVICE_DESCRIPTION};
            if (!_ChangeServiceConfig2 (service, SERVICE_CONFIG_DESCRIPTION, (LPVOID) &desc)) {
                jqs_warn("Failed to set service description: ChangeServiceConfig2 failed (error %d)\n", GetLastError ());
            }
        
        } else {
            jqs_warn("Unable to set service description: ChangeServiceConfig2 is not available\n");
        }
    }

    CloseServiceHandle(scmHandle);

    // Close the handle to this service object
    CloseServiceHandle(service);

    return true;
}

/*
 * If true is passed to this function, it sets service startup type to automatic 
 * state and starts the service. Otherwise, it stops the service and sets service 
 * startup type to disabled state.
 */
bool enableService(bool enable) {
    SC_HANDLE scmHandle = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
    if (scmHandle == NULL) {
        jqs_error("Unable to %s service: OpenSCManager failed (error %d)\n",
                  (enable ? "enable" : "disable"), GetLastError());
        return false;
    }

    SC_HANDLE service = OpenService(scmHandle,
                                    JQS_SERVICE_NAME,
                                    SERVICE_ALL_ACCESS);
    if (service == NULL) {
        DWORD lastError = GetLastError();
        if (lastError == ERROR_SERVICE_DOES_NOT_EXIST) {
            jqs_error("%s service does not exist\n", JQS_SERVICE_DISPLAY_NAME);
        } else {
            jqs_error("Unable to open %s service: OpenService failed (error %d)\n", JQS_SERVICE_DISPLAY_NAME, lastError);
        }
        return false;
    }

    if (!ChangeServiceConfig (service,
                              SERVICE_NO_CHANGE, // dwServiceType
                              enable ? SERVICE_AUTO_START : SERVICE_DISABLED, // dwStartType
                              SERVICE_NO_CHANGE, // dwErrorControl
                              NULL,     // lpBinaryPathName
                              NULL,     // lpLoadOrderGroup
                              NULL,     // lpdwTagId
                              NULL,     // lpDependencies
                              NULL,     // lpServiceStartName
                              NULL,     // lpPassword
                              NULL))    // lpDisplayName
                              
    {
        DWORD lastError = GetLastError();
        jqs_error("Unable to %s service: ChangeServiceConfig failed (error %d)\n",
                  (enable ? "enable" : "disable"), lastError);
        return false;
    }

    if (enable) {
        if (StartService (service, 0, NULL)) {
            // Wait a second...
            Sleep (1000);

            // Poll the status of the service for SERVICE_START_PENDING
            SERVICE_STATUS ssStatus;
            while (QueryServiceStatus(service, &ssStatus)) {

                // If the service has not stopped, wait another second
                if (ssStatus.dwCurrentState == SERVICE_START_PENDING)
                    Sleep (1000);
                else
                    break;
            }
        }

    } else {
        // Now, try to stop the service by passing a STOP code through the control manager
        SERVICE_STATUS ssStatus;
        if (ControlService (service, SERVICE_CONTROL_STOP, &ssStatus)) {
            // Wait a second...
            Sleep (1000);

            // Poll the status of the service for SERVICE_STOP_PENDING
            while (QueryServiceStatus(service, &ssStatus)) {

                // If the service has not stopped, wait another second
                if (ssStatus.dwCurrentState == SERVICE_STOP_PENDING)
                    Sleep (1000);
                else
                    break;
            }
        }
    }

    bool res = true;

    SERVICE_STATUS ssStatus;
    if (QueryServiceStatus(service, &ssStatus)) {
        res = (enable  && ssStatus.dwCurrentState == SERVICE_RUNNING) ||
              (!enable && ssStatus.dwCurrentState == SERVICE_STOPPED);
    }

    CloseServiceHandle(scmHandle);
    CloseServiceHandle(service);

    return res;
}

/*
 * Uninstalls JQS service and unregisters IE and FireFox startup detector plugins.
 */
bool uninstallService() {
    SC_HANDLE scmHandle = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
    if (scmHandle == NULL) {
        jqs_error("Unable to uninstall service: OpenSCManager failed (error %d)\n", GetLastError());
        return false;
    }

    SC_HANDLE service = OpenService(scmHandle,
                                    JQS_SERVICE_NAME,
                                    SERVICE_ALL_ACCESS);
    if (service == NULL) {
        DWORD lastError = GetLastError();
        if (lastError == ERROR_SERVICE_DOES_NOT_EXIST) {
            jqs_error("%s service does not exist\n", JQS_SERVICE_DISPLAY_NAME);
        } else {
            jqs_error("Could not open %s service: OpenService failed (error %d)\n", JQS_SERVICE_DISPLAY_NAME, lastError);
        }
        return false;
    }

    // Now, try to stop the service by passing a STOP code through the control manager
    SERVICE_STATUS ssStatus;
    if (ControlService (service, SERVICE_CONTROL_STOP, &ssStatus)) {
        // Wait a second...
        Sleep (1000);

        // Poll the status of the service for SERVICE_STOP_PENDING
        while (QueryServiceStatus(service, &ssStatus)) {

            // If the service has not stopped, wait another second
            if (ssStatus.dwCurrentState == SERVICE_STOP_PENDING)
                Sleep (1000);
            else
                break;
        }
    }

    // Now try to remove the service
    if (!DeleteService(service)) {
        jqs_error("Unable to remove %s service: DeleteService failed (error %d)\n", JQS_SERVICE_DISPLAY_NAME, GetLastError());
        return false;
    }

    CloseServiceHandle(scmHandle);
    CloseServiceHandle(service);

    removeEventSource();

    // unregister browser startup detectors
    unregisterIEStartDetector ();
    unregisterFFStartDetector ();

    return true;
}
