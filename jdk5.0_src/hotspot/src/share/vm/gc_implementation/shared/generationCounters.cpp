#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)generationCounters.cpp	1.4 04/02/06 20:28:34 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_generationCounters.cpp.incl"


GenerationCounters::GenerationCounters(const char* name,
                                       int ordinal, int spaces,
                                       VirtualSpace* v):
                    _virtual_space(v) {

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
                                     _virtual_space->committed_size(), CHECK);

    cname = PerfDataManager::counter_name(_name_space, "maxCapacity");
    PerfDataManager::create_constant(SUN_GC, cname, PerfData::U_Bytes,
                                     _virtual_space->reserved_size(), CHECK);

    cname = PerfDataManager::counter_name(_name_space, "capacity");
    _current_size = PerfDataManager::create_variable(SUN_GC, cname,
                                      PerfData::U_Bytes,
                                     _virtual_space->committed_size(), CHECK);
  }
}
