/*
 * Copyright (c) 2001, 2009, Oracle and/or its affiliates. All rights reserved.
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

// VM_operations for the G1 collector.
// VM_GC_Operation:
//   - VM_CGC_Operation
//   - VM_G1CollectFull
//   - VM_G1CollectForAllocation
//   - VM_G1IncCollectionPause
//   - VM_G1PopRegionCollectionPause

class VM_G1CollectFull: public VM_GC_Operation {
 private:
 public:
  VM_G1CollectFull(int gc_count_before,
                   GCCause::Cause gc_cause)
    : VM_GC_Operation(gc_count_before)
  {
    _gc_cause = gc_cause;
  }
  ~VM_G1CollectFull() {}
  virtual VMOp_Type type() const { return VMOp_G1CollectFull; }
  virtual void doit();
  virtual const char* name() const {
    return "full garbage-first collection";
  }
};

class VM_G1CollectForAllocation: public VM_GC_Operation {
 private:
  HeapWord*   _res;
  size_t      _size;                       // size of object to be allocated
 public:
  VM_G1CollectForAllocation(size_t size, int gc_count_before)
    : VM_GC_Operation(gc_count_before) {
    _size        = size;
    _res         = NULL;
  }
  ~VM_G1CollectForAllocation()        {}
  virtual VMOp_Type type() const { return VMOp_G1CollectForAllocation; }
  virtual void doit();
  virtual const char* name() const {
    return "garbage-first collection to satisfy allocation";
  }
  HeapWord* result() { return _res; }
};

class VM_G1IncCollectionPause: public VM_GC_Operation {
 public:
  VM_G1IncCollectionPause(int gc_count_before,
                          GCCause::Cause gc_cause = GCCause::_g1_inc_collection_pause) :
    VM_GC_Operation(gc_count_before) { _gc_cause = gc_cause; }
  virtual VMOp_Type type() const { return VMOp_G1IncCollectionPause; }
  virtual void doit();
  virtual const char* name() const {
    return "garbage-first incremental collection pause";
  }
};

// Concurrent GC stop-the-world operations such as initial and final mark;
// consider sharing these with CMS's counterparts.
class VM_CGC_Operation: public VM_Operation {
  VoidClosure* _cl;
  const char* _printGCMessage;
 public:
  VM_CGC_Operation(VoidClosure* cl, const char *printGCMsg) :
    _cl(cl),
    _printGCMessage(printGCMsg)
    {}

  ~VM_CGC_Operation() {}

  virtual VMOp_Type type() const { return VMOp_CGC_Operation; }
  virtual void doit();
  virtual bool doit_prologue();
  virtual void doit_epilogue();
  virtual const char* name() const {
    return "concurrent gc";
  }
};
