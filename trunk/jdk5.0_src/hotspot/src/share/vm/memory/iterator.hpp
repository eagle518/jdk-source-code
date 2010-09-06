#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)iterator.hpp	1.29 03/12/23 16:41:17 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// The following classes are C++ `closures` for iterating over objects, roots and spaces

class ReferenceProcessor;

// OopClosure is used for iterating through roots (oop*)

class OopClosure : public StackObj {
 public:
  ReferenceProcessor* _ref_processor;
  OopClosure(ReferenceProcessor* rp) : _ref_processor(rp) { }
  OopClosure() : _ref_processor(NULL) { }
  virtual void do_oop(oop* o) = 0;
  virtual void do_oop_v(oop* o) { do_oop(o); }

  // In support of post-processing of weak links of KlassKlass objects;
  // see KlassKlass::oop_oop_iterate().
  virtual const bool should_remember_klasses() const { return false;    }
  virtual void remember_klass(Klass* k) { /* do nothing */ }

  // If "true", invoke on nmethods (when scanning compiled frames).
  virtual const bool do_nmethods() const { return false; }

  // The methods below control how object iterations invoking this closure
  // should be performed:

  // If "true", invoke on header klass field.
  bool do_header() { return true; } // Note that this is non-virtual.
  // Controls how prefetching is done for invocations of this closure.
  Prefetch::style prefetch_style() { // Note that this is non-virtual.
    return Prefetch::do_none;
  } 
};

// ObjectClosure is used for iterating through an object space

class ObjectClosure : public StackObj {
 public:
  // Called for each object.
  virtual void do_object(oop obj) = 0;
};


class BoolObjectClosure : public ObjectClosure {
 public:
  virtual bool do_object_b(oop obj) = 0;
};

// Applies an oop closure to all ref fields in objects iterated over in an
// object iteration.
class ObjectToOopClosure: public ObjectClosure {
  OopClosure* _cl;
public:
  void do_object(oop obj);
  ObjectToOopClosure(OopClosure* cl) : _cl(cl) {}
};

// A version of ObjectClosure with "memory" (see _previous_address below)
class UpwardsObjectClosure: public ObjectClosure {
  HeapWord* _previous_address;
 public:
  UpwardsObjectClosure() : _previous_address(0) { }
  void set_previous(HeapWord* addr) { _previous_address = addr; }
  HeapWord* previous()              { return _previous_address; }
  // Forget the _previous_address value, making it 0
  void      forget()                { _previous_address = 0; }
};

// A version of UpwardsObjectClosure that is expected to be robust
// in the face of possibly uninitialized objects.
class MemObjectClosureCareful : public UpwardsObjectClosure {
 public:
  void do_object(oop p) {
    guarantee(false, "call do_object_careful instead");
  }
  virtual size_t do_object_careful(oop p) = 0;
};

// The following are used in CompactibleFreeListSpace and
// ConcurrentMarkSweepGeneration.

// Blk closure (abstract class)
class BlkClosure : public StackObj {
 public:
  virtual size_t do_blk(HeapWord* addr) = 0;
};

// A version of BlkClosure that is expected to be robust
// in the face of possibly uninitialized objects.
class BlkClosureCareful : public BlkClosure {
 public:
  size_t do_blk(HeapWord* addr) {
    guarantee(false, "call do_blk_careful instead");
    return 0;
  }
  virtual size_t do_blk_careful(HeapWord* addr) = 0;
};

// SpaceClosure is used for iterating over spaces

class Space;
class CompactibleSpace;

class SpaceClosure : public StackObj {
 public:
  // Called for each space
  virtual void do_space(Space* s) = 0;
};

class CompactibleSpaceClosure : public StackObj {
 public:
  // Called for each compactible space
  virtual void do_space(CompactibleSpace* s) = 0;
};



// MonitorClosure is used for iterating over monitors in the monitors cache

class ObjectMonitor;

class MonitorClosure : public StackObj {
 public:
  // called for each monitor in cache
  virtual void do_monitor(ObjectMonitor* m) = 0;
};

// A closure that is applied without any arguments.
class VoidClosure : public StackObj {
 public:
  // I would have liked to declare this a pure virtual, but that breaks
  // in mysterious ways, for unknown reasons.
  virtual void do_void();
};


// YieldClosure is intended for use by iteration loops
// to incrementalize their work, allowing interleaving
// of an interruptable task so as to allow other
// threads to run (which may not otherwise be able to access
// exclusive resources, for instance). Additionally, the
// closure also allows for aborting an ongoing iteration
// by means of checking the return value from the polling
// call.
class YieldClosure : public StackObj {
  public:
   virtual bool should_return() = 0;
};

// Abstract closure for serializing data (read or write).

class SerializeOopClosure : public OopClosure {
public:
  // Return bool indicating whether closure implements read or write.
  virtual bool reading() const = 0;

  // Read/write the int pointed to by i.
  virtual void do_int(int* i) = 0;

  // Read/write the size_t pointed to by i.
  virtual void do_size_t(size_t* i) = 0;

  // Read/write the void pointer pointed to by p.
  virtual void do_ptr(void** p) = 0;

  // Read/write the HeapWord pointer pointed to be p.
  virtual void do_ptr(HeapWord** p) = 0;

  // Read/write the region specified.
  virtual void do_region(u_char* start, size_t size) = 0;

  // Check/write the tag.  If reading, then compare the tag against
  // the passed in value and fail is they don't match.  This allows
  // for verification that sections of the serialized data are of the
  // correct length.
  virtual void do_tag(int tag) = 0;
};
