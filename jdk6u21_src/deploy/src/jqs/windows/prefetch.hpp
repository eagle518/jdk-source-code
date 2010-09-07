/*
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef PREFETCH_HPP
#define PREFETCH_HPP

#include "jqs.hpp"
#include "qsentry.hpp"
#include "os_utils.hpp"

extern CriticalSection PrefetchLock;

/*
 * This function is responsible for initializing all global data.
 */
extern void initialize();

/*
 * This function is responsible for any cleanup needed before termination.
 */
extern void cleanup();

/*
 * The implementation of respective commands supported by JQS.
 */
extern void do_load(QSEntry* ent);
extern void do_refresh(QSEntry* ent);
extern void do_loadlib(QSEntry* ent);
extern void do_refreshlib(QSEntry* ent);
extern void do_refreshdir(QSEntry* ent);
extern void do_unload(QSEntry* ent);

/*
 * Function to get the page size for the system, using the
 * requested page size if available.
 */
extern size_t get_pagesize(size_t requested);

/*
 * Function to sleep for sec seconds and then poll for high memory condition.
 * If a high memory condition exists, the function returns. Otherwise, it
 * sleeps for another sec seconds and then check again. The function will
 * not return until a high memory condition exists.
 * The function chooses the best suitable implementation for current operating 
 * system.
 */
extern void wait_for_high_mem_or_timeout(unsigned int seconds);

/*
 * Waits for the JQS to be resumed.
 */
extern void wait_for_resume();

/*
 * Pauses JQS operation.
 */
extern void pauseJQSService();

/*
 * Resumes JQS operation.
 */
extern void resumeJQSService();

/*
 * Notifies JQS to perform a refresh if system condition allows that.
 */
extern void notifyJQSService();

/*
 * Arranges JQS termination sequence.
 */
extern void terminateJQSService();

/*
 * Unloads quick starter entry with specified file handle.
 */
extern void unloadQSEntryByFileHandle (HANDLE handle);

/*
 * Returns true if the AC power status is online.
 */
extern bool checkPowerStatus();

/*
 * Obtains values of CPU usage and Disk I/O usage performance counters in a loop
 * waiting for values that fit the threshold values specified in the configuration 
 * file.
 * Note: the function uses different thresholds for the boot time refresh and for the 
 * ordinary refreshes.
 */
extern void waitForSystemIdle(bool boot);


/*
 * Checks if device on which QS entry resides is going to suspend.
 * In such case, prefetching is skipped as it could awake device.
 */
extern bool checkForDeviceSuspend (QSEntry* ent);
extern void prefetchingFinished (QSEntry* ent);


/*
 * Touch & read statistics for one refresh cycle.
 */
// total number of bytes to touch without profile during current refresh
extern uint64_t TotalBytesToTouch;
// number of bytes actually touched during current refresh
extern uint64_t BytesActuallyTouched;

// total number of bytes to read without profile during current refresh
extern uint64_t TotalBytesToRead;
// number of bytes actually read during current refresh
extern uint64_t BytesActuallyRead;

/*
 * JQS Capabilities.
 */
extern bool capabilityAdjustWorkingSetSize;
extern bool capabilityLockMemoryPages;
extern bool capabilityLargePages;
extern bool capabilityLowMemoryNotifications;
extern bool capabilitySetLibraryPath;
extern bool capabilityCheckPowerStatus;
extern bool capabilityDeviceEventNotifications;
extern bool capabilityUserLogonNotifications;

#endif
