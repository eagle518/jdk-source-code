/*
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
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

class VM_ParallelGCFailedAllocation: public VM_GC_Operation {
 private:
  size_t    _size;
  bool      _is_tlab;
  HeapWord* _result;

 public:
  VM_ParallelGCFailedAllocation(size_t size, bool is_tlab,
                                unsigned int gc_count);

  virtual VMOp_Type type() const {
    return VMOp_ParallelGCFailedAllocation;
  }
  virtual void doit();

  HeapWord* result() const       { return _result; }
};

class VM_ParallelGCFailedPermanentAllocation: public VM_GC_Operation {
private:
  size_t    _size;
  HeapWord* _result;

 public:
  VM_ParallelGCFailedPermanentAllocation(size_t size,
                                         unsigned int gc_count,
                                         unsigned int full_gc_count);
  virtual VMOp_Type type() const {
    return VMOp_ParallelGCFailedPermanentAllocation;
  }
  virtual void doit();
  HeapWord* result() const       { return _result; }
};

class VM_ParallelGCSystemGC: public VM_GC_Operation {
 public:
  VM_ParallelGCSystemGC(unsigned int gc_count, unsigned int full_gc_count,
                        GCCause::Cause gc_cause);
  virtual VMOp_Type type() const { return VMOp_ParallelGCSystemGC; }
  virtual void doit();
};
