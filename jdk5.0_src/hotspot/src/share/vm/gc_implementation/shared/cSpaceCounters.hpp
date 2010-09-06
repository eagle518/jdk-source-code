#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)cSpaceCounters.hpp	1.6 04/02/06 20:46:37 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// A CSpaceCounters is a holder class for performance counters
// that track a space;

class CSpaceCounters: public CHeapObj {
  friend class VMStructs;

 private:
  PerfVariable*      _capacity;
  PerfVariable*      _used;

  // Constant PerfData types don't need to retain a reference.
  // However, it's a good idea to document them here.
  // PerfConstant*     _size;

  ContiguousSpace*     _space;
  char*                _name_space;

 public:

  CSpaceCounters(const char* name, int ordinal, size_t max_size,
                 ContiguousSpace* s, GenerationCounters* gc);

  ~CSpaceCounters() {
      if (_name_space != NULL) FREE_C_HEAP_ARRAY(char, _name_space);
  }
  
  inline void update_capacity() {
    _capacity->set_value(_space->capacity());
  }

  inline void update_used() {
    _used->set_value(_space->used());
  }

  inline void update_all() {
    update_used();
    update_capacity();
  }

  const char* name_space() const        { return _name_space; }
};

class ContiguousSpaceUsedHelper : public PerfLongSampleHelper {
  private:
    ContiguousSpace* _space;

  public:
    ContiguousSpaceUsedHelper(ContiguousSpace* space) : _space(space) { }

    inline jlong take_sample() {
      return _space->used();
    }
};

