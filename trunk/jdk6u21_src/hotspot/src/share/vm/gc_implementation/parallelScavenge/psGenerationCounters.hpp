
/*
 * Copyright (c) 2004, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */

// A PSGenerationCounter is a holder class for performance counters
// that track a generation

class PSGenerationCounters: public GenerationCounters {
  friend class VMStructs;

 private:
  PSVirtualSpace*      _ps_virtual_space;

 public:
  PSGenerationCounters(const char* name, int ordinal, int spaces,
                     PSVirtualSpace* v);

  void update_all() {
    assert(_virtual_space == NULL, "Only one should be in use");
    _current_size->set_value(_ps_virtual_space->committed_size());
  }
};
