#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)gSpaceCounters.cpp	1.5 04/02/06 20:26:59 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_gSpaceCounters.cpp.incl"

GSpaceCounters::GSpaceCounters(const char* name, int ordinal, size_t max_size,
                               Generation* g, GenerationCounters* gc,
                               bool sampled) :
   _gen(g) {
   
  if (UsePerfData) {
    EXCEPTION_MARK;
    ResourceMark rm;

    const char* cns = PerfDataManager::name_space(gc->name_space(), "space",
                                                  ordinal);

    _name_space = NEW_C_HEAP_ARRAY(char, strlen(cns)+1);
    strcpy(_name_space, cns);

    const char* cname = PerfDataManager::counter_name(_name_space, "name");
    PerfDataManager::create_string_constant(SUN_GC, cname, name, CHECK);

    cname = PerfDataManager::counter_name(_name_space, "maxCapacity");
    PerfDataManager::create_constant(SUN_GC, cname, PerfData::U_Bytes,
                                     (jlong)max_size, CHECK);

    cname = PerfDataManager::counter_name(_name_space, "capacity");
    _capacity = PerfDataManager::create_variable(SUN_GC, cname,
                                                 PerfData::U_Bytes,
                                                 _gen->capacity(), CHECK);

    cname = PerfDataManager::counter_name(_name_space, "used");
    if (sampled) {
      _used = PerfDataManager::create_variable(SUN_GC, cname, PerfData::U_Bytes,
                                               new GenerationUsedHelper(_gen),
                                               CHECK);
    }
    else {
      _used = PerfDataManager::create_variable(SUN_GC, cname, PerfData::U_Bytes,
                                               (jlong)0, CHECK);
    }

    cname = PerfDataManager::counter_name(_name_space, "initCapacity");
    PerfDataManager::create_constant(SUN_GC, cname, PerfData::U_Bytes,
                                     _gen->capacity(), CHECK);
  }
}
