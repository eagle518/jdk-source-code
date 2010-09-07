/*
 * @(#)DeployPerf.cpp	1.2 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "DeployPerf.h"
#ifdef WIN32
#include "BasicPerfHelper.h"
#include "BasicPerfStore.h"
#include "WinLock.h"
#include "WinTime.h"
#else
#include "SharedMemPerfHelper.h"
#include "UnixTime.h"
#endif


#ifdef WIN32
static WinTime         g_perfClock;
static WinLock         g_perfSync;
static BasicPerfHelper g_deployPerf(g_perfClock, g_perfSync);
#else
static UnixTime            g_perfClock;
static SharedMemPerfHelper g_deployPerf(g_perfClock);
#endif


PERFLIB_API DeployPerf * GetDeployPerf(void) {
    return (&g_deployPerf);
}
