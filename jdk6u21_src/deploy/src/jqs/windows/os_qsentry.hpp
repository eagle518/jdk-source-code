/*
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef OS_QSENTRY_HPP
#define OS_QSENTRY_HPP

#include "os_layer.hpp"

/*
 * OS specific data structure for quick starter entries.
 */
class QSEntryOS {
public:
    QSEntryOS()
        : hModule(0)
        , hFile(0)
        , hDevNotify(0)
        , hDevice(0)
    {
        memset (&devicePerfCounters, 0, sizeof(devicePerfCounters));
    }

    /*
     * Module handle used for loading dlls.
     */
    HMODULE hModule;
    /*
     * File handle used for receiving device notification events.
     */
    HANDLE  hFile;
    /*
     * Device event notification handle for the entry.
     */
    HDEVNOTIFY hDevNotify;
    /*
     * Device handle for the entry.
     */
    HANDLE hDevice;
    DISK_PERFORMANCE devicePerfCounters;
};

#endif
