#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)heap.hpp	1.34 03/12/23 16:41:16 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Blocks

class HeapBlock VALUE_OBJ_CLASS_SPEC {
  friend class VMStructs;

 public:
  struct Header {
    size_t  _length;                             // the length in segments    
    bool    _used;                               // Used bit
  };

 protected:
  union {
    Header _header;
    int64_t _padding[ (sizeof(Header) + sizeof(int64_t)-1) / sizeof(int64_t) ];
                        // pad to 0 mod 8
  };

 public:
  // Initialization
  void initialize(size_t length)                 { _header._length = length; set_used(); }

  // Accessors
  void* allocated_space() const                  { return (void*)(this + 1); }
  size_t length() const                          { return _header._length; }  

  // Used/free
  void set_used()                                { _header._used = true; }
  void set_free()                                { _header._used = false; }
  bool free()                                    { return !_header._used; }
};

class FreeBlock: public HeapBlock {
  friend class VMStructs;
 protected:  
  FreeBlock* _link; 

 public:
  // Initialization
  void initialize(size_t length)             { HeapBlock::initialize(length); _link= NULL; }

  // Merging    
  void set_length(size_t l)                  { _header._length = l; }
  
  // Accessors    
  FreeBlock* link() const                    { return _link; }  
  void set_link(FreeBlock* link)             { _link = link; }  
};

class CodeHeap : public CHeapObj {
  friend class VMStructs;
 private:
  VirtualSpace _memory;                          // the memory holding the blocks
  VirtualSpace _segmap;                          // the memory holding the segment map

  size_t       _number_of_committed_segments;
  size_t       _number_of_reserved_segments;
  size_t       _segment_size;
  int          _log2_segment_size;

  size_t       _next_segment;

  FreeBlock*   _freelist; 
  size_t       _free_segments;                   // No. of segments in freelist

  // Helper functions
  size_t   number_of_segments(size_t size) const { return (size + _segment_size - 1) >> _log2_segment_size; }
  size_t   size(size_t number_of_segments) const { return number_of_segments << _log2_segment_size; }

  size_t   segment_for(void* p) const            { return ((char*)p - _memory.low()) >> _log2_segment_size; }
  HeapBlock* block_at(size_t i) const            { return (HeapBlock*)(_memory.low() + (i << _log2_segment_size)); }

  void  mark_segmap_as_free(size_t beg, size_t end);
  void  mark_segmap_as_used(size_t beg, size_t end);

  // Freelist management helpers      
  FreeBlock* following_block(FreeBlock *b);
  void insert_after(FreeBlock* a, FreeBlock* b);
  void merge_right (FreeBlock* a);
  
  // Toplevel freelist management
  void add_to_freelist(HeapBlock *b);
  FreeBlock* search_freelist(size_t length);

  // Iteration helpers
  void*      next_free(HeapBlock* b) const;
  HeapBlock* first_block() const;
  HeapBlock* next_block(HeapBlock* b) const;
  HeapBlock* block_start(void* p) const;

 public:
  CodeHeap();

  // Heap extents
  bool  reserve(size_t reserved_size, size_t committed_size, size_t segment_size);
  void  release();                               // releases all allocated memory
  bool  expand_by(size_t size);                  // expands commited memory by size
  void  shrink_by(size_t size);                  // shrinks commited memory by size
  void  clear();                                 // clears all heap contents

  // Memory allocation
  void* allocate  (size_t size);                 // allocates a block of size or returns NULL
  void  deallocate(void* p);                     // deallocates a block

  // Attributes
  void*  begin() const                           { return _memory.low (); }
  void*  end() const                             { return _memory.high(); }
  bool   contains(void* p) const                 { return begin() <= p && p < end(); }
  void*  find_start(void* p) const;              // returns the block containing p or NULL
  size_t alignment_unit() const;                 // alignment of any block
  size_t alignment_offset() const;               // offset of first byte of any block, within the enclosing alignment unit
  static size_t header_size();                   // returns the header size for each heap block

  // Iteration
  
  // returns the first block or NULL
  void* first() const       { return next_free(first_block()); }
  // returns the next block given a block p or NULL
  void* next(void* p) const { return next_free(next_block(block_start(p))); }

  // Statistics
  size_t capacity() const;  
  size_t max_capacity() const;
  size_t allocated_capacity() const;
  size_t unallocated_capacity() const            { return max_capacity() - allocated_capacity(); }
  
  // Debugging
  void verify() PRODUCT_RETURN;
  void print()  PRODUCT_RETURN;
};
