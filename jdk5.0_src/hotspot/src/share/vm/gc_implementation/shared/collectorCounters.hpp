#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)collectorCounters.hpp	1.4 04/02/06 20:26:17 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// CollectorCounters is a holder class for performance counters
// that track a collector

class CollectorCounters: public CHeapObj {
  friend class VMStructs;

  private:
    PerfCounter*      _invocations;
    PerfCounter*      _time;
    PerfVariable*     _last_entry_time;
    PerfVariable*     _last_exit_time;

    // Constant PerfData types don't need to retain a reference.
    // However, it's a good idea to document them here.
    // PerfStringConstant*     _name;

    char*             _name_space;

  public:

    CollectorCounters(const char* name, int ordinal);

    ~CollectorCounters() {
      if (_name_space != NULL) FREE_C_HEAP_ARRAY(char, _name_space);
    }
  
    inline PerfCounter* invocation_counter() const  { return _invocations; }

    inline PerfCounter* time_counter() const        { return _time; }

    inline PerfVariable* last_entry_counter() const { return _last_entry_time; }

    inline PerfVariable* last_exit_counter() const  { return _last_exit_time; }

    const char* name_space() const                  { return _name_space; }
};

class TraceCollectorStats: public PerfTraceTimedEvent {

  protected:
    CollectorCounters* _c;

  public:
    inline TraceCollectorStats(CollectorCounters* c) :
           PerfTraceTimedEvent(c->time_counter(), c->invocation_counter()),
           _c(c) {

      if (UsePerfData) {
         _c->last_entry_counter()->set_value(os::elapsed_counter());
      }
    }

    inline ~TraceCollectorStats() {
      if (UsePerfData) _c->last_exit_counter()->set_value(os::elapsed_counter());
    }
};

