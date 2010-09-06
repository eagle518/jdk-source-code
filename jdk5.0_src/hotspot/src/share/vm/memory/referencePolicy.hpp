#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)referencePolicy.hpp	1.5 03/12/23 16:41:25 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

// referencePolicy is used to determine when soft reference objects
// should be cleared.


class ReferencePolicy : public ResourceObj {
 public:
  virtual bool should_clear_reference(oop p)       { ShouldNotReachHere(); return true; }
};

class NeverClearPolicy : public ReferencePolicy {
 public:
  bool should_clear_reference(oop p) { return false; }
};

class AlwaysClearPolicy : public ReferencePolicy {
 public:
  bool should_clear_reference(oop p) { return true; }
};

class LRUCurrentHeapPolicy : public ReferencePolicy {
 private:
  jlong _max_interval;

 public:
  LRUCurrentHeapPolicy();

  bool should_clear_reference(oop p);
};

class LRUMaxHeapPolicy : public ReferencePolicy {
 private:
  jlong _max_interval;

 public:
  LRUMaxHeapPolicy();

  bool should_clear_reference(oop p);
};
