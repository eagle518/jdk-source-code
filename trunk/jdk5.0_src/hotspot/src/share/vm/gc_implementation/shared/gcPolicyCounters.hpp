#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)gcPolicyCounters.hpp	1.7 04/02/05 12:28:04 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// GCPolicyCounters is a holder class for performance counters
// that track a generation

class GCPolicyCounters: public CHeapObj {
  friend class VMStructs;

  private:

    // Constant PerfData types don't need to retain a reference.
    // However, it's a good idea to document them here.
    // PerfStringConstant*     _name;
    // PerfStringConstant*     _collector_size;
    // PerfStringConstant*     _generation_size;

    PerfVariable*     _tenuring_threshold;
    PerfVariable*     _desired_survivor_size;

    const char* _name_space;

  public:

    GCPolicyCounters(const char* name, int collectors, int generations);

    inline PerfVariable* tenuring_threshold() const  {
      return _tenuring_threshold;
    }

    inline PerfVariable* desired_survivor_size() const  {
      return _desired_survivor_size;
    }

    const char* name_space() const { return _name_space; }
};
