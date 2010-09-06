#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)ciOopMap.hpp	1.13 04/03/29 14:12:27 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

// Note: This classes have been derived from class in oopMapCache.?pp;
//       We need to factorize this operations if possible

// This class holds oop maps generated in the compiler interface
// No oops allowed inside this structure as, it is passed and referenced
// by the compiler

#ifdef COMPILER1
define_array(intStackArray, intStack*)

class ciLocalMapIterator;

// The class contains two structures, one describing the bci that has 
// the oop map and the other describing the oop-map; the oop map is compressed
// and contains nof_gc_points x max_nof_locals entries, with each entry being 
// either 1 or 2 bit wide; this is an array of masks, all of fixed length 
// (number of bits = max_nof_locals*entry_size).; there are nof_gc_points
// masks
class ciLocalMap: public ResourceObj {
 public:
  enum {
    bits_per_entry   = 2,
    addr_bit_number  = 1,
    oop_bit_number   = 0
  };

 private:
  Arena*        _arena;
  const int     _max_nof_locals;
  const int     _code_size;
  const int     _nof_gc_points;
  int           _nof_entries;
  unsigned int* _index_for_bci;
  unsigned int* _bci_for_index;
  unsigned int* _map;
  int           _mask_size; // in bits
  int           _mask_words;

  int           _nof_initialize;
  unsigned int* _locals;

  int  mask_size() const { return _mask_size; }

  unsigned int entry_at(int map_index, int offset) const {
    assert (0 <= map_index && map_index < _nof_gc_points, "wrong offset");
    unsigned int* bit_mask = map_for_index(map_index);
    int i = offset * bits_per_entry;
    return bit_mask[i / BitsPerInt] >> (i % BitsPerInt);
  }

  void set_init_local(int index, int local_index);
  void set_nof_initialize(int n);
  void set_nof_entries(int n) { _nof_entries = n; }

  friend class ciGenerateLocalMap;

  void set_bci_for_index(int map_index, int bci);

 public:
  ciLocalMap(Arena* arena, int max_nof_locals, int code_size, int nof_gc_points);

  bool is_oop  (int map_index, int offset) const  { return (entry_at(map_index, offset) & (1 << oop_bit_number )) != 0; }
  bool is_addr (int map_index, int offset) const  { return (entry_at(map_index, offset) & (1 << addr_bit_number)) != 0; }

  // map_index must be : 0 .. _nof_gc_points - 1
  unsigned int* map_for_index(int map_index) const {
    assert(0 <= map_index && map_index < _nof_gc_points, "");
    assert(_map != NULL, "");
    int start_index = map_index * _mask_words;
    return &_map[start_index];
  }

  int bci_for_index (int map_index)  const;
  int index_for_bci (int bci)  const;

  int nof_gc_points()  const { return _nof_gc_points;  }
  int nof_entries()    const { return _nof_entries;    }
  int max_nof_locals() const { return _max_nof_locals; }

  // info about locals to initialize
  int nof_locals_to_initialize()  const { return _nof_initialize; }
  int local_to_initialize(int index) const { assert(0<=index && index<_nof_initialize,"range chek"); return _locals[index]; }

  bool is_address_local(int local, int bci);

  void print() PRODUCT_RETURN;
};


class ciLocalMapIterator: public StackObj {
 private:
  int               _index;
  int               _offset;
  const ciLocalMap* _map;

  void find_next() {
    while (_offset < _map->max_nof_locals()) {
      if (_map->is_oop(_index, _offset)) return;
      _offset++;
    }
  }

 public:
  ciLocalMapIterator(ciLocalMap* map, int bci): _map(map), _index(-1) {
    if (_map->nof_gc_points() > 0) {
      _index = _map->index_for_bci(bci);
    }
    if (_index != -1) {
      _offset = 0;
    } else {
      _offset = _map->max_nof_locals();
    }
    find_next();
  }

  bool done() { return _offset >= _map->max_nof_locals(); }
  int next_oop_offset() {
    assert(!done(), "shouldn't be done");
    int offset = _offset;
    _offset++;
    find_next();
    return offset;
  }
};


class ciGenerateLocalMap: public GenerateOopMap {
 private:
  Arena*         _arena;
  bool           _has_exceptions_handlers;
  bool           _is_synchronized;
  GrowableArray<bool>* _is_gc_point; // for exception handler starts and for jsr return points
  int            _nof_gc_points;
  ciLocalMap*    _oop_maps;

  int            _fill_counter;

  virtual bool possible_gc_point(BytecodeStream *bcs); // tells if current operation is a GC point
  virtual void fill_stackmap_prolog(int nof_gc_points);   // reports nof of gc point
  virtual bool report_results() const            { return true;  }
  virtual bool report_init_vars() const          { return true;  }
  virtual void fill_stackmap_epilog()            { _oop_maps->set_nof_entries(_fill_counter); }

  virtual void fill_stackmap_for_opcodes(BytecodeStream *bcs,
                                         CellTypeState* vars, 
                                         CellTypeState* stack, 
                                         int stackTop) ;
  virtual void fill_init_vars(GrowableArray<intptr_t> *init_vars);

  void find_jsr_return_points(methodHandle method);

 public:
  ciGenerateLocalMap(Arena* arena, methodHandle m);

  ciLocalMap* get_oop_maps() const               { return _oop_maps; }

  // Helper routine also used by C1 to eliminate generation of debug info
  // where this is necessary for correctness
  static bool bytecode_is_gc_point(Bytecodes::Code code, bool method_has_exception_handlers, bool method_is_synchronized);
};


#endif// COMPILER1

