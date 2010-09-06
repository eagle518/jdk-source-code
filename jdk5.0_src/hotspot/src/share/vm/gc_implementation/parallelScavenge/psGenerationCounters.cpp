#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)psGenerationCounters.cpp	1.1 04/06/16 15:26:45 JVM"
#endif

/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_psGenerationCounters.cpp.incl"


PSGenerationCounters::PSGenerationCounters(const char* name,
                                       int ordinal, int spaces,
                                       PSVirtualSpace* v):
    _ps_virtual_space(v) {

  if (UsePerfData) {

    EXCEPTION_MARK;
    ResourceMark rm;

    const char* cns = PerfDataManager::name_space("generation", ordinal);

    _name_space = NEW_C_HEAP_ARRAY(char, strlen(cns)+1);
    strcpy(_name_space, cns);

    const char* cname = PerfDataManager::counter_name(_name_space, "name");
    PerfDataManager::create_string_constant(SUN_GC, cname, name, CHECK);

    cname = PerfDataManager::counter_name(_name_space, "spaces");
    PerfDataManager::create_constant(SUN_GC, cname, PerfData::U_None,
                                     spaces, CHECK);

    cname = PerfDataManager::counter_name(_name_space, "minCapacity");
    PerfDataManager::create_constant(SUN_GC, cname, PerfData::U_Bytes,
      _ps_virtual_space->committed_size(), CHECK);

    cname = PerfDataManager::counter_name(_name_space, "maxCapacity");
    PerfDataManager::create_constant(SUN_GC, cname, PerfData::U_Bytes,
      _ps_virtual_space->reserved_size(), CHECK);

    cname = PerfDataManager::counter_name(_name_space, "capacity");
    _current_size = PerfDataManager::create_variable(SUN_GC, cname,
       PerfData::U_Bytes, _ps_virtual_space->committed_size(), CHECK);
  }
}
