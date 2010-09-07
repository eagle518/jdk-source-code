/*
 * @(#)DeployPerfUtil.cpp	1.2 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "stdafx.h"
#include "DeployPerfUtil.h"
#include "pluginVersion.h"


// initialize the static data members
DeployPerfUtil::state   DeployPerfUtil::m_state      = DeployPerfUtil::unloaded;
DeployPerf            * DeployPerfUtil::m_deployPerf = NULL;


bool DeployPerfUtil::isEnabled(void) {
    char * perf_env = getenv("DEPLOY_PERF_ENABLED");
    bool   result   = false;

    if (perf_env != NULL) {
#ifdef WIN32
        // Windows treats ENV vars as case insensitive
        result = (_stricmp(perf_env, "false") != 0);
#else
        // Real OS'es treat unique symbols like unique symbols
        result = (strcmp(perf_env, "false") != 0);
#endif
    }

    return (result);
}


DeployPerfUtil::state DeployPerfUtil::prepDeployPerf(void) {
    if (m_state == unloaded) {
        m_deployPerf = loadLibFuncs();
        if (m_deployPerf != NULL) {
            m_state = loaded;
        }
        else {
            m_state = error;
        }
    }

    return (m_state);
}


DeployPerfUtil::state DeployPerfUtil::initLogging(void) {
    if (m_state == loaded) {
        if (isEnabled() == true) {
            m_state = ((m_deployPerf->initStore() == true) ?
                       initialized : error);
        }
        else {
            m_state = disabled;
        }
    }
    else if (m_state == unloaded) {
        // somebody screwed up the call sequence!
        m_state = error;
    }

    return (m_state);
}


DeployPerf * DeployPerfUtil::loadLibFuncs(void) {
    char         path[MAX_PATH];
    DeployPerf * result = NULL;

    if (getPath(path) == true) {
        // load the library
#ifdef WIN32
        HINSTANCE instance = LoadLibrary(path);
#else
        // TODO: implement runtime dynamic library linking for Unix
        void * instance = NULL;
#endif

        if (instance != NULL) {
            // get the address of the GetDeployPerf function
            typedef DeployPerf * (* fpGetDeployPerf)(void);

            fpGetDeployPerf _GetDeployPerf =
#ifdef WIN32
                (fpGetDeployPerf) GetProcAddress(instance, "GetDeployPerf");
#else
                // TODO: implement runtime dynamic library linking for Unix
                NULL;
#endif

            if (_GetDeployPerf != NULL) {
                // finally, get a pointer to the DeployPerf object
                result = _GetDeployPerf();
            }
            else {
                // something failed, so release the library
#ifdef WIN32
                FreeLibrary(instance);
                instance = NULL;
#else
                // TODO: implement runtime dynamic library linking for Unix
#endif
            }
        }
    }

    return (result);
}


bool DeployPerfUtil::getPath(TCHAR * path) {
    bool result = false;
#ifdef WIN32
    HKEY parent = NULL;
    LONG status = ERROR_SUCCESS;

    status = RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                           JAVAPLUGINREGISTRYKEY,
                           0,
                           KEY_READ,
                           &parent);
    if (status == ERROR_SUCCESS) {
        DWORD type = REG_SZ;
        DWORD size = MAX_PATH;

        status = RegQueryValueExA(parent,
                                  JAVAHOMEVALUE,
                                  NULL,
                                  &type,
                                  (LPBYTE) path,
                                  &size);
        if( ERROR_SUCCESS == status ) {
            _tcscat(path, "\\bin\\deploy.dll");
            result = true;
        }

        RegCloseKey(parent);
    }
#else
    // TODO: implement library path lookup for Unix
#endif

    return (result);
}
