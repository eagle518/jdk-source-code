/*
 * @(#)awt_Multimon.h	1.9 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
#ifndef     _INC_MULTIMON_
#define     _INC_MULTIMON_
#endif
//
// build defines that replace the regular APIs with our versions
//
#undef GetMonitorInfo
#undef GetSystemMetrics
#undef MonitorFromWindow
#undef MonitorFromRect
#undef MonitorFromPoint
#undef EnumDisplayMonitors
#undef EnumDisplayDevices

#include    "awt_MMStub.h"

#define GetSystemMetricsMM      _getSystemMetrics
#define MonitorFromWindow       _monitorFromWindow
#define MonitorFromRect         _monitorFromRect
#define MonitorFromPoint        _monitorFromPoint
#define GetMonitorInfo          _getMonitorInfo
#define EnumDisplayMonitors     _enumDisplayMonitors
#define EnumDisplayDevices      _enumDisplayDevices


#define CountMonitors           _countMonitors
#define CollectMonitors         _collectMonitors
#define MonitorBounds           _monitorBounds
#define MakeDCFromMonitor       _makeDCFromMonitor
#define CreateWindowOnMonitor   _createWindowOM


















































