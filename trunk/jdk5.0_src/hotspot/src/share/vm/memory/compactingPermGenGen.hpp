#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)compactingPermGenGen.hpp	1.6 04/04/09 23:48:31 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// All heaps contains a "permanent generation," containing permanent
// (reflective) objects.  This is like a regular generation in some ways,
// but unlike one in others, and so is split apart.

class PermanentGenerationSpec;

// This is the "generation" view of a CompactingPermGen.
class CompactingPermGenGen: public OneContigSpaceCardGeneration {
  friend class VMStructs;
  // Abstractly, this is a subtype that gets access to protected fields.
  friend class CompactingPermGen;

private:
  // Shared spaces
  PermanentGenerationSpec* _spec;
  size_t _shared_space_size;
  VirtualSpace _ro_vs;
  VirtualSpace _rw_vs;
  VirtualSpace _md_vs;
  VirtualSpace _mc_vs;
  BlockOffsetSharedArray* _ro_bts;
  BlockOffsetSharedArray* _rw_bts;
  OffsetTableContigSpace* _ro_space;
  OffsetTableContigSpace* _rw_space;

  // These values are redundant, but are called out separately to avoid
  // going through heap/space/gen pointers for performance.
  static HeapWord* unshared_bottom;
  static HeapWord* unshared_end;
  static HeapWord* shared_bottom;
  static HeapWord* readonly_bottom;
  static HeapWord* readonly_end;
  static HeapWord* readwrite_bottom;
  static HeapWord* readwrite_end;
  static HeapWord* miscdata_bottom;
  static HeapWord* miscdata_end;
  static HeapWord* misccode_bottom;
  static HeapWord* misccode_end;
  static HeapWord* shared_end;

  // List of klassOops whose vtbl entries are used to patch others.
  static void**        _vtbl_list;

  // Performance Counters
  GenerationCounters*  _gen_counters;
  CSpaceCounters*      _space_counters;

  void initialize_performance_counters();

  void make_rw_space_walkable();

public:

  enum {
    vtbl_list_size = 16, // number of entries in the shared space vtable list.
    num_virtuals = 100   // number of virtual methods in Klass (or
                         // subclass) objects, or greater.
  };

  enum {
    ro = 0,  // read-only shared space in the heap
    rw = 1,  // read-write shared space in the heap
    md = 2,  // miscellaneous data for initializing tables, etc.
    mc = 3,  // miscellaneous code - vtable replacement.
    n_regions = 4
  };

  CompactingPermGenGen(ReservedSpace rs, ReservedSpace shared_rs,
                       size_t initial_byte_size, int level, GenRemSet* remset,
		       ContiguousSpace* space,
                       PermanentGenerationSpec* perm_spec);

  const char* name() const {
    return "compacting perm gen";
  }
  
  const char* short_name() const {
    return "Perm";
  }

  void update_counters();

  void compute_new_size() {
    assert(false, "Should not call this -- handled at PermGen level.");
  }

  bool must_be_youngest() const { return false; }
  bool must_be_oldest() const { return false; }

  OffsetTableContigSpace* ro_space() const { return _ro_space; }
  OffsetTableContigSpace* rw_space() const { return _rw_space; }
  VirtualSpace*           md_space()       { return &_md_vs; }
  VirtualSpace*           mc_space()       { return &_mc_vs; }
  ContiguousSpace* unshared_space() const { return _the_space; }

  static bool inline is_shared(const oopDesc* p) {
    return (HeapWord*)p >= shared_bottom && (HeapWord*)p < shared_end;
  }
  static bool inline is_shared_readonly(const oopDesc* p) {
    return (HeapWord*)p >= readonly_bottom && (HeapWord*)p < readonly_end;
  }
  static bool inline is_shared_readwrite(const oopDesc* p) {
    return (HeapWord*)p >= readwrite_bottom && (HeapWord*)p < readwrite_end;
  }

  inline bool is_in(const void* p) const {
    return p >= unshared_bottom && p < shared_end;
  }

  inline PermanentGenerationSpec* spec() const { return _spec; }
  inline void set_spec(PermanentGenerationSpec* spec) { _spec = spec; }

  inline bool is_in_shared(const void* p) const {return true;}

  void pre_adjust_pointers();
  void adjust_pointers();
  void space_iterate(SpaceClosure* blk, bool usedOnly = false);
  void print_on(outputStream* st) const;
  void younger_refs_iterate(OopsInGenClosure* blk);
  void compact();
  void post_compact();
  size_t contiguous_available() const;
  bool grow_by(size_t bytes);
  void grow_to_reserved();

  void clear_remembered_set();
  void invalidate_remembered_set();

  inline bool block_is_obj(const HeapWord* addr) const {
    if      (addr < the_space()->top()) return true;
    else if (addr < the_space()->end()) return false;
    else if (addr < ro_space()->top())  return true;
    else if (addr < ro_space()->end())  return false;
    else if (addr < rw_space()->top())  return true;
    else                                return false;
  }


  inline size_t block_size(const HeapWord* addr) const {
    if (addr < the_space()->top()) {
      return oop(addr)->size();
    }
    else if (addr < the_space()->end()) {
      assert(addr == the_space()->top(), "non-block head arg to block_size");
      return the_space()->end() - the_space()->top();
    }

    else if (addr < ro_space()->top()) {
      return oop(addr)->size();
    }
    else if (addr < ro_space()->end()) {
      assert(addr == ro_space()->top(), "non-block head arg to block_size");
      return ro_space()->end() - ro_space()->top();
    }

    else if (addr < rw_space()->top()) {
      return oop(addr)->size();
    }
    else {
      assert(addr == rw_space()->top(), "non-block head arg to block_size");
      return rw_space()->end() - rw_space()->top();
    }
  }

  static void generate_vtable_methods(void** vtbl_list,
                                      void** vtable,
                                      char** md_top, char* md_end,
                                      char** mc_top, char* mc_end);

#ifndef PRODUCT
  void verify(bool allow_dirty);
#endif

  // Serialization
  static void initialize_oops();
  static void serialize_oops(SerializeOopClosure* soc);
  void serialize_bts(SerializeOopClosure* soc);

  // Initiate dumping of shared file.
  static jint dump_shared(TRAPS);
};
