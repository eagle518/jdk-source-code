/*
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef SERVICE_HPP
#define SERVICE_HPP

#include "os_layer.hpp"
#include "jqs.hpp"

/*
 * Path to IE start detector plugin.
 */
#define JQS_IE_PLUGIN_NAME              JAVA_HOME_RELATIVE_TO_JQS "/lib/deploy/jqs/ie/jqs_plugin.dll"

/*
 * ID and path to FireFox start detector extension.
 */
#define JQS_FF_PLUGIN_ID                "jqs@sun.com"
#define JQS_FF_PLUGIN_DIR               JAVA_HOME_RELATIVE_TO_JQS "/lib/deploy/jqs/ff"


/*
 * Installs JQS service and registers IE and FireFox start detector plugins.
 */
extern bool installService();

/*
 * Uninstalls JQS service and unregisters IE and FireFox start detector plugins.
 */
extern bool uninstallService();

/*
 * If true is passed to this function, it sets service startup type to automatic 
 * state and starts the service. Otherwise, it stops the service and sets service 
 * startup type to disables state.
 */
extern bool enableService(bool enable);

/*
 * The JQS service body.
 */
extern void runService();

/*
 * Enables device notification events for the file. These events are signaled to 
 * the service for each file located on the device that the system tries to unmount.
 * The file name passed is used in error reporting.
 * Returns device notification handle.
 */
extern HDEVNOTIFY registerDeviceNotification(HANDLE hFile, const char* fileName);

/*
 * Unregisters device notification by handle.
 */
extern void unregisterDeviceNotification(HDEVNOTIFY hDevNotify);

#endif
