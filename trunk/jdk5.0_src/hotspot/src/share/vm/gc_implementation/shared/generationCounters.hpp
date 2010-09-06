#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)generationCounters.hpp	1.8 04/06/18 10:30:32 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// A GenerationCounter is a holder class for performance counters
// that track a generation

class GenerationCounters: public CHeapObj {
  friend class VMStructs;

 protected:
  PerfVariable*      _current_size;
  VirtualSpace*      _virtual_space;

  // Constant PerfData types don't need to retain a reference.
  // However, it's a good idea to document them here.
  // PerfStringConstant*     _name;
  // PerfConstant*           _min_size;
  // PerfConstant*           _max_size;
  // PerfConstant*           _spaces;

  char*              _name_space;

  // This constructor is only meant for use with the PSGenerationCounters
  // constructor.  The need for such an constructor should be eliminated
  // when VirtualSpace and PSVirtualSpace are unified.
  GenerationCounters() : _name_space(NULL), _current_size(NULL), _virtual_space(NULL) {}
 public:

  GenerationCounters(const char* name, int ordinal, int spaces,
                     VirtualSpace* v);

  ~GenerationCounters() {
    if (_name_space != NULL) FREE_C_HEAP_ARRAY(char, _name_space);
  }

  virtual void update_all() {
    _current_size->set_value(_virtual_space->committed_size());
  }

  const char* name_space() const        { return _name_space; }
};
