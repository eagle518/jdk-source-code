#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)allocationStats.hpp	1.6 03/12/23 16:40:51 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

class AllocationStats VALUE_OBJ_CLASS_SPEC {
  int	     _desired;		// needed between sweeps: prevSweep - count(start 
				// of sweep) + splitBirths - splitDeaths
  int        _coalDesired;   	// desired +/- small-percent for tuning coalescing

  int        _surplus;       	// count - (desired +/- small-percent), 
				// used to tune splitting in best fit
  int        _bfrSurp;       	// surplus at start of current sweep
  uint       _prevSweep;     	// count from end of previous sweep
  uint       _beforeSweep;   	// count from before current sweep
  uint       _coalBirths;    	// additional chunks from coalescing
  uint       _coalDeaths;    	// loss from coalescing
  uint       _splitBirths;   	// additional chunks from splitting
  uint       _splitDeaths;   	// loss from splitting
  size_t     _returnedBytes;	// number of bytes returned to list.
 public:
  void initialize() {
    _desired = 0;
    _coalDesired = 0;
    _surplus = 0;
    _bfrSurp = 0;
    _prevSweep = 0;
    _beforeSweep = 0;
    _coalBirths = 0;
    _coalDeaths = 0;
    _splitBirths = 0;
    _splitDeaths = 0;
    _returnedBytes = 0;
  }

  AllocationStats() {
    initialize();
  }

  int desired() const { return _desired; }
  void set_desired(int v) { _desired = v; }
  int coalDesired() const { return _coalDesired; }
  void set_coalDesired(int v) { _coalDesired = v; }

  int surplus() const { return _surplus; }
  void set_surplus(int v) { _surplus = v; }
  void increment_surplus() { _surplus++; }
  void decrement_surplus() { _surplus--; }

  int bfrSurp() const { return _bfrSurp; }
  void set_bfrSurp(int v) { _bfrSurp = v; }
  int prevSweep() const { return _prevSweep; }
  void set_prevSweep(int v) { _prevSweep = v; }
  int beforeSweep() const { return _beforeSweep; }
  void set_beforeSweep(int v) { _beforeSweep = v; }

  int coalBirths() const { return _coalBirths; }
  void set_coalBirths(int v) { _coalBirths = v; }
  void increment_coalBirths() { _coalBirths++; }

  int coalDeaths() const { return _coalDeaths; }
  void set_coalDeaths(int v) { _coalDeaths = v; }
  void increment_coalDeaths() { _coalDeaths++; }

  int splitBirths() const { return _splitBirths; }
  void set_splitBirths(int v) { _splitBirths = v; }
  void increment_splitBirths() { _splitBirths++; }

  int splitDeaths() const { return _splitDeaths; }
  void set_splitDeaths(int v) { _splitDeaths = v; }
  void increment_splitDeaths() { _splitDeaths++; }

  NOT_PRODUCT(
    size_t returnedBytes() const { return _returnedBytes; }
    void set_returnedBytes(size_t v) { _returnedBytes = v; }
  )
};
