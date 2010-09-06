#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)psVirtualspace.hpp	1.6 03/12/23 16:40:20 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// VirtualSpace for the parallel scavenge collector.
// 
// VirtualSpace is data structure for committing a previously reserved address
// range in smaller chunks.

class PSVirtualSpace : public CHeapObj {
  friend class VMStructs;
 protected:
  // The space is committed/uncommited in chunks of size _alignment.  The
  // ReservedSpace passed to initialize() must be aligned to this value.
  const size_t _alignment;

  // Reserved area
  char* _reserved_low_addr;
  char* _reserved_high_addr; 

  // Committed area
  char* _committed_low_addr;
  char* _committed_high_addr;

  // Convenience wrapper.
  inline static size_t pointer_delta(const char* left, const char* right);

 public:
  PSVirtualSpace(ReservedSpace rs, size_t alignment);
  PSVirtualSpace(ReservedSpace rs);

  ~PSVirtualSpace();

  // Eventually all instances should be created with the above 1- or 2-arg
  // constructors.  Then the 1st constructor below should become protected and
  // the 2nd ctor and initialize() removed.
  PSVirtualSpace(size_t alignment): _alignment(alignment) { }
  PSVirtualSpace();
  bool initialize(ReservedSpace rs, size_t commit_size);

  bool contains(void* p)      const;

  // Accessors (all sizes are bytes).
  size_t alignment()          const { return _alignment; }
  char* reserved_low_addr()   const { return _reserved_low_addr; }
  char* reserved_high_addr()  const { return _reserved_high_addr; }
  char* committed_low_addr()  const { return _committed_low_addr; }
  char* committed_high_addr() const { return _committed_high_addr; }

  inline size_t committed_size()   const;
  inline size_t reserved_size()    const;
  inline size_t uncommitted_size() const;

  // Operations.
  inline  void   set_reserved(char* low_addr, char* high_addr);
  inline  void   set_reserved(ReservedSpace rs);
  inline  void   set_committed(char* low_addr, char* high_addr);
  virtual bool   expand_by(size_t bytes);
  virtual bool   shrink_by(size_t bytes);
  virtual size_t expand_into(PSVirtualSpace* space, size_t bytes);
  void           release();

#ifndef PRODUCT
  // Debugging
  static  bool is_aligned(size_t val, size_t align);
          bool is_aligned(size_t val) const;
          bool is_aligned(char* val) const;
          void verify() const;
          void print() const;
  virtual bool grows_up() const   { return true; }
          bool grows_down() const { return !grows_up(); }

  // Helper class to verify a space when entering/leaving a block.
  class PSVirtualSpaceVerifier: public StackObj {
   private:
    const PSVirtualSpace* const _space;
   public:
    PSVirtualSpaceVerifier(PSVirtualSpace* space): _space(space) {
      _space->verify();
    }
    ~PSVirtualSpaceVerifier() { _space->verify(); }
  };
#endif

  virtual void print_space_boundaries_on(outputStream* st) const;

 // Included for compatibility with the original VirtualSpace.
 public:
  // Committed area
  char* low()  const { return committed_low_addr(); }
  char* high() const { return committed_high_addr(); }

  // Reserved area
  char* low_boundary()  const { return reserved_low_addr(); }
  char* high_boundary() const { return reserved_high_addr(); }
};

// A virtual space that grows from high addresses to low addresses.
class PSVirtualSpaceHighToLow : public PSVirtualSpace {
  friend class VMStructs;
 public:
  PSVirtualSpaceHighToLow(ReservedSpace rs, size_t alignment);
  PSVirtualSpaceHighToLow(ReservedSpace rs);

  virtual bool   expand_by(size_t bytes);
  virtual bool   shrink_by(size_t bytes);
  virtual size_t expand_into(PSVirtualSpace* space, size_t bytes);

  virtual void print_space_boundaries_on(outputStream* st) const;

#ifndef PRODUCT
  // Debugging
  virtual bool grows_up() const   { return false; }
#endif
};

// 
// PSVirtualSpace inlines.
// 
inline size_t
PSVirtualSpace::pointer_delta(const char* left, const char* right) {
  return ::pointer_delta((void *)left, (void*)right, sizeof(char));
}

inline size_t PSVirtualSpace::committed_size() const {
  return pointer_delta(committed_high_addr(), committed_low_addr());
}

inline size_t PSVirtualSpace::reserved_size() const {
  return pointer_delta(reserved_high_addr(), reserved_low_addr());
}

inline size_t PSVirtualSpace::uncommitted_size() const {
  return reserved_size() - committed_size();
}

inline void PSVirtualSpace::set_reserved(char* low_addr, char* high_addr) {
  _reserved_low_addr = low_addr;
  _reserved_high_addr = high_addr;
}

inline void PSVirtualSpace::set_reserved(ReservedSpace rs) {
  set_reserved(rs.base(), rs.base() + rs.size());
}

inline void PSVirtualSpace::set_committed(char* low_addr, char* high_addr) {
  _committed_low_addr = low_addr;
  _committed_high_addr = high_addr;
}
