#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)gcPolicyCounters.cpp	1.7 04/02/06 20:27:47 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_gcPolicyCounters.cpp.incl"

GCPolicyCounters::GCPolicyCounters(const char* name, int collectors,
                                   int generations) {

  if (UsePerfData) {
    EXCEPTION_MARK;
    ResourceMark rm;

    _name_space = "policy";

    char* cname = PerfDataManager::counter_name(_name_space, "name");
    PerfDataManager::create_string_constant(SUN_GC, cname, name, CHECK);

    cname = PerfDataManager::counter_name(_name_space, "collectors");
    PerfDataManager::create_constant(SUN_GC, cname,  PerfData::U_None,
                                     collectors, CHECK);

    cname = PerfDataManager::counter_name(_name_space, "generations");
    PerfDataManager::create_constant(SUN_GC, cname,  PerfData::U_None,
                                     generations, CHECK);

    cname = PerfDataManager::counter_name(_name_space, "maxTenuringThreshold");
    PerfDataManager::create_constant(SUN_GC, cname, PerfData::U_None,
                                     MaxTenuringThreshold, CHECK);

    cname = PerfDataManager::counter_name(_name_space, "tenuringThreshold");
    _tenuring_threshold =
        PerfDataManager::create_variable(SUN_GC, cname, PerfData::U_None,
                                         MaxTenuringThreshold, CHECK);

    cname = PerfDataManager::counter_name(_name_space, "desiredSurvivorSize");
    _desired_survivor_size =
        PerfDataManager::create_variable(SUN_GC, cname, PerfData::U_Bytes,
                                         CHECK);
  }
}
