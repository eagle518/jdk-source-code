#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)growableArray.hpp	1.37 03/12/23 16:44:47 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// A growable array.

/*************************************************************************/
/*                                                                       */
/*     WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING   */
/*                                                                       */
/* Should you use GrowableArrays to contain handles you must be certain  */
/* the the GrowableArray does not outlive the HandleMark that contains   */
/* the handles. Since GrowableArrays are typically resource allocated    */
/* the following is an example of INCORRECT CODE,                        */
/*                                                                       */
/* ResourceMark rm;                                                      */
/* GrowableArray<Handle>* arr = new GrowableArray<Handle>(size);         */
/* if (blah) {                                                           */
/*    while (...) {                                                      */
/*      HandleMark hm;                                                   */
/*      ...                                                              */
/*      Handle h(THREAD, some_oop);                                      */
/*      arr->append(h);                                                  */
/*    }                                                                  */
/* }                                                                     */
/* if (arr->length() != 0 ) {                                            */
/*    oop bad_oop = arr->at(0)(); // Handle is BAD HERE.                 */
/*    ...                                                                */
/* }                                                                     */
/*                                                                       */
/* If the GrowableArrays you are creating is C_Heap allocated then it    */
/* hould not old handles since the handles could trivially try and       */
/* outlive their HandleMark. In some situations you might need to do     */
/* this and it would be legal but be very careful and see if you can do  */
/* the code in some other manner.                                        */
/*                                                                       */
/*************************************************************************/


class GrET {};    // dummy element type to avoid void*

// Need the correct linkage to call qsort without warnings
extern "C" {
  typedef int (*_raw_sort_Fn)(const void *, const void *);
}

typedef void (*arrayDoFn)(GrET* p);
typedef bool (*growableArrayFindFn)(GrET* token, GrET* elem);

class GenericGrowableArray : public NonPrintingResourceObj {
 protected:
  int    _len;		// current length
  int    _max;		// maximum length
  GrET** _data;		// data array
  Arena* _arena;        // Indicates where allocation occurs:
                        //   0 means default ResourceArea
                        //   1 means on C heap
                        //   otherwise, allocate in _arena
#ifdef ASSERT
  int    _nesting;      // resource area nesting at creation
  void   check_nesting();
#else
#define  check_nesting();
#endif

  // Where are we going to allocate memory?
  bool on_C_heap() { return _arena == (Arena*)1; }
  bool on_stack () { return _arena == NULL;      }
  bool on_arena () { return _arena >  (Arena*)1;  } 

  void grow(int j);	// grow data array (double length until j is a valid index)
#ifndef _LP64
  void grow64(int j);	// grow 64 bit data array 
#endif

  bool  raw_contains(const GrET* p) const;
  bool  raw_contains_only(const GrET* p) const;
  int   raw_find(const GrET* p) const; 
  int   raw_find(GrET* token, growableArrayFindFn f) const; 
  void  raw_remove(const GrET* p);
  void  raw_remove_at(int index);
  void  raw_apply(arrayDoFn f) const;
  void* raw_at_grow(int i, const GrET* fill);
  void  raw_at_put_grow(int i, const GrET* p, const GrET* fill);
  void  raw_appendAll(const GenericGrowableArray* l);
  GenericGrowableArray* raw_copy() const;
  void  raw_sort(_raw_sort_Fn);

  GenericGrowableArray(int initial_size, bool on_C_heap = false);
  GenericGrowableArray(int initial_size, int initial_len, GrET* filler, bool on_C_heap = false);
  GenericGrowableArray(Arena* arena, int initial_size, int initial_len, GrET* filler);

 public:
  void  clear()    		{ _len = 0; }
  void  clear_and_deallocate();
  int   length() const  	{ return _len; }
  void	trunc_to(int l)		{ assert(l <= _len,"cannot increase length"); _len = l; }
  int   capacity() const 	{ return _max; }
  bool  is_empty() const  	{ return _len == 0; }
  bool  is_nonempty() const 	{ return _len != 0; }
  bool  is_full() const   	{ return _len == _max; }
  GrET** data_addr() const	{ return _data; }	// for sorting

  int  max_length(const GenericGrowableArray* other) const { // return max. length of receiver and other
    return MAX2(_len, other->length()); }
  int  min_length(const GenericGrowableArray* other) const { // return min. length of receiver and other
    return MIN2(_len, other->length()); }
  bool equals    (const GenericGrowableArray* other) const;   // true if contents are bitwise equal

  void print_short();
  void print();
};


template<class E> class GrowableArray : public GenericGrowableArray {
 public:
  GrowableArray(int initial_size, bool on_C_heap = false) : GenericGrowableArray(initial_size, on_C_heap) { }
  GrowableArray(int initial_size, int initial_len, E filler, bool on_C_heap = false) : GenericGrowableArray(initial_size, initial_len, (GrET*)filler, on_C_heap) {}
  GrowableArray(Arena* arena, int initial_size, int initial_len, E filler) : GenericGrowableArray(arena, initial_size, initial_len, (GrET*)filler) {}
  GrowableArray() : GenericGrowableArray(2) {}

  void append(const E elem) {
    check_nesting();
    if (_len == _max) grow(_len);
    _data[_len++] = (GrET*) elem;
  }

  void append_if_missing(const E elem) {
    if (!contains(elem)) append(elem);
  }

  E at(int i) const {
    assert(0 <= i && i < _len, "illegal index");
    return (E) (intptr_t)_data[i];      // NB: intermediate cast to int is needed to allow enum elements
  }

  E* adr_at(int i) const {
    assert(0 <= i && i < _len, "illegal index");
    return (E*) (int*)&_data[i];
  }

  E first() const {
    assert(_len > 0, "empty list");
    return (E) (intptr_t)_data[0];
  }

  E last() const {                                                  
    assert(_len > 0, "empty list");
    return (E) (intptr_t)_data[_len-1];
  }

  void push(const E elem) { append(elem); }

  E pop() {
    assert(_len > 0, "empty list");
    return (E) (intptr_t)_data[--_len];
  }

  E top() const { return last(); }

  void at_put(int i, const E elem) {
    assert(0 <= i && i < _len, "illegal index");
    _data[i] = (GrET*) elem;
  }

  E at_grow(int i, const E fill = (E)(intptr_t)NULL) {
    assert(0 <= i, "negative index");
    check_nesting();
    return (E) (intptr_t)raw_at_grow(i, (GrET*) fill);
  }

  void at_put_grow(int i, const E elem, const E fill = (E)(intptr_t)NULL) {
    assert(0 <= i, "negative index");
    check_nesting();
    raw_at_put_grow(i, (GrET*) elem, (GrET*) fill);
  }

  bool contains(const E elem) const		           { return raw_contains((const GrET*) elem);           }
  bool contains_only(const E elem) const                   { return raw_contains_only((const GrET*) elem);      }
  int  find(const E elem) const 		           { return raw_find((const GrET*) elem);               }
  int  find(void* token, bool f(void*, E)) const           { return raw_find((GrET*)token, (growableArrayFindFn)f);    }
  void remove(const E elem)   			           { raw_remove((const GrET*) elem);                    }
  void remove_at(int index)   			           { raw_remove_at(index);                              }
  void apply(void f(E)) const 			           { raw_apply((arrayDoFn)f);	                        }
  GrowableArray<E>* copy() const		           { return (GrowableArray<E>*) raw_copy();             }
  void appendAll(const GrowableArray<E>* l) 	           { raw_appendAll((GenericGrowableArray*)l);           }  
  void sort(int f(E*,E*))                                  { raw_sort((_raw_sort_Fn) f); }

  int  max_length(const GrowableArray<E>* other) const     { return GenericGrowableArray::max_length((GenericGrowableArray*)other); }
  int  min_length(const GrowableArray<E>* other) const     { return GenericGrowableArray::min_length((GenericGrowableArray*)other); }
  bool equals    (const GrowableArray<E>* other) const     { return GenericGrowableArray::equals((GenericGrowableArray*)other);     }
};


#ifndef _LP64
// GrowableArray supporting the uint64_t type in 32 bit VM's
//   Note:  This class is not complete and currently is only used 
//          for fingerprint handlers and JVMTI object tagging functions.
template <> class GrowableArray<uint64_t> : public GenericGrowableArray {
 public:
  GrowableArray<uint64_t>(int initial_size, bool on_C_heap = false) : 
  GenericGrowableArray(initial_size * (sizeof(uint64_t)/sizeof(GrET*)), 
                                                            on_C_heap) {
   // Adjust initial _max value if pointers are not 64 bit
    _max = _max / (sizeof(uint64_t)/sizeof(GrET*));
 }

  void append(const uint64_t elem) {
    uint64_t* _data64;
    check_nesting();
    if (_len == _max) grow64(_len);
    _data64 = (uint64_t*)_data;
    _data64[_len++] = (uint64_t) elem;
  }

  uint64_t at(int i) const {
    assert(0 <= i && i < _len, "illegal index");
    uint64_t* _data64 = (uint64_t*)_data;
    return _data64[i];
  }

  int  find(const uint64_t elem) const {
    uint64_t* _data64 = (uint64_t*) _data;
    for (int i = 0; i < _len; i++) {
      if (_data64[i] == elem) return i;
    }
    return -1;
  }
};
#endif
