#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)growableArray.cpp	1.29 03/12/23 16:44:46 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_growableArray.cpp.incl"

GenericGrowableArray::GenericGrowableArray(int initial_size, bool c_heap) {
  _len = 0;
  _max = initial_size;
  assert(_len >= 0 && _len <= _max, "initial_len too big");
  _arena = (c_heap ? (Arena*)1 : NULL);
  if (on_C_heap()) {
    _data = NEW_C_HEAP_ARRAY(GrET*, _max);
  } else {
    _data = NEW_RESOURCE_ARRAY(GrET*, _max);
  }
#ifdef ASSERT
  if (!on_C_heap()) {
    _nesting = Thread::current()->resource_area()->nesting();
  }
#endif
}

GenericGrowableArray::GenericGrowableArray(int initial_size, int initial_len, GrET* filler, bool c_heap) {
  _len = initial_len;
  _max = initial_size;
  assert(_len >= 0 && _len <= _max, "initial_len too big");
  _arena = (c_heap ? (Arena*)1 : NULL);
  if (on_C_heap()) {
    _data = NEW_C_HEAP_ARRAY(GrET*, _max);
  } else {
    _data = NEW_RESOURCE_ARRAY(GrET*, _max);
  }
  for (int i = 0; i < _len; i++) _data[i] = filler;
#ifdef ASSERT
  if (!on_C_heap()) {
    _nesting = Thread::current()->resource_area()->nesting();
  }
#endif
}

GenericGrowableArray::GenericGrowableArray(Arena* arena, int initial_size, int initial_len, GrET* filler) {
  _arena = arena;
  assert(on_arena(), "arena has taken on reserved value 0 or 1");
  _len = initial_len;
  _max = initial_size;
  assert(_len >= 0 && _len <= _max, "initial_len too big");
  _data = (GrET**)arena->Amalloc(sizeof(GrET*)*_max);
  for (int i = 0; i < _len; i++) _data[i] = filler;
}

#ifdef ASSERT
void GenericGrowableArray::check_nesting() {
  // Check for insidious allocation bug: if a GrowableArray overflows, the 
  // grown array must be allocated under the same ResourceMark as the original.
  // Otherwise, the _data array will be deallocated too early.
  if (on_stack() &&
      _nesting != Thread::current()->resource_area()->nesting()) {
    fatal("allocation bug: GrowableArray could grow within nested ResourceMark");
  }
}
#endif

void GenericGrowableArray::grow(int j) {
  // grow the array by doubling its size (amortized growth)
  if (_max == 0) _max = 1; // prevent endless loop
  while (j >= _max) _max = _max*2;
  // j < _max
  GrET** newData;
  if (on_stack()) {
    newData = NEW_RESOURCE_ARRAY(GrET*, _max);
  } else if (on_C_heap()) {
    newData = NEW_C_HEAP_ARRAY(GrET*, _max);
  } else {
    newData = (GrET**)_arena->Amalloc(sizeof(GrET*)*_max);
  }
  for (int i = 0; i < _len; i++) newData[i] = _data[i];
  if (on_C_heap() && _data != NULL) {
    FreeHeap(_data);
  }
  _data = newData;
}

#ifndef _LP64
void GenericGrowableArray::grow64(int j) {
  uint64_t* _data64 = (uint64_t*)_data;
  // grow the array by doubling its size (amortized growth)
  if (_max == 0) _max = 1; // prevent endless loop
  while (j >= _max) _max = _max*2;
  // j < _max
  uint64_t* newData;
  if (on_stack()) {
    newData = NEW_RESOURCE_ARRAY(uint64_t, _max);
  } else if (on_C_heap()) {
    newData = NEW_C_HEAP_ARRAY(uint64_t, _max);
  } else {
    newData = (uint64_t*)_arena->Amalloc(sizeof(uint64_t)*_max);
  }
  for (int i = 0; i < _len; i++) newData[i] = _data64[i];
  if (on_C_heap() && _data != NULL) {
    FreeHeap(_data);
  }
  _data = (GrET**)newData;
}
#endif

bool GenericGrowableArray::raw_contains(const GrET* elem) const {
  for (int i = 0; i < _len; i++) {
    if (_data[i] == elem) return true;
  }
  return false;
}

bool GenericGrowableArray::raw_contains_only(const GrET* elem) const {
  for (int i = 0; i < _len; i++) {
    if (_data[i] != elem) return false;
  }
  return true;
}

GenericGrowableArray* GenericGrowableArray::raw_copy() const {
  GenericGrowableArray* copy = new GenericGrowableArray(_max, _len, NULL);
  GrET** copy_data = copy->_data;
  for (int i = 0; i < _len; i++) {
    copy_data[i] = _data[i];
  }
  return copy;
}

void GenericGrowableArray::raw_appendAll(const GenericGrowableArray* l) {
  for (int i = 0; i < l->_len; i++) {
    raw_at_put_grow(_len, l->_data[i], NULL);
  }
}

int GenericGrowableArray::raw_find(const GrET* elem) const {
  for (int i = 0; i < _len; i++) {
    if (_data[i] == elem) return i;
  }
  return -1;
}

int GenericGrowableArray::raw_find(GrET* token, growableArrayFindFn f) const  {
  for (int i = 0; i < _len; i++) {
    if (f(token, _data[i])) return i;
  }
  return -1;
}

void GenericGrowableArray::raw_remove(const GrET* elem) {
  for (int i = 0; i < _len; i++) {
    if (_data[i] == elem) {
      for (int j = i + 1; j < _len; j++) _data[j-1] = _data[j];
      _len--;
      return;
    }
  }
  ShouldNotReachHere();
}

void GenericGrowableArray::raw_remove_at(int index) {
  assert(0 <= index && index < _len, "illegal index");
  for (int j = index + 1; j < _len; j++) _data[j-1] = _data[j];
  _len--;
}

void GenericGrowableArray::raw_apply(arrayDoFn f) const {
  for (int i = 0; i < _len; i++) f(_data[i]);
}

void* GenericGrowableArray::raw_at_grow(int i, const GrET* fill) {
  if (i >= _len) {
    if (i >= _max) grow(i);
    for (int j = _len; j <= i; j++) _data[j] = (GrET*)fill;
    _len = i+1;
  }
  return _data[i];
}

void GenericGrowableArray::raw_at_put_grow(int i, const GrET* p, const GrET* fill) {
  if (i >= _len) {
    if (i >= _max) grow(i);
    for (int j = _len; j < i; j++) _data[j] = (GrET*)fill;
    _len = i+1;
  }
  _data[i] = (GrET*)p;
}

void GenericGrowableArray::print() {
  print_short();
  tty->print(": length %ld (_max %ld) { ", _len, _max);
  for (int i = 0; i < _len; i++) tty->print("%#lx ", (long)_data[i]);
  tty->print("}\n");
}

void GenericGrowableArray::print_short() { tty->print("Growable Array %#lx", this); }

void GenericGrowableArray::raw_sort(_raw_sort_Fn f) {
  qsort(_data, length(), sizeof(void*), f);
}

bool GenericGrowableArray::equals(const GenericGrowableArray* other) const {
  if (this == other) return true; // same reference, same array
  if (this->length() != other->length()) {
    return false;
  } else {
    // same length
    for (int i = length() - 1; i >= 0; i--) {
      if (_data[i] != other->_data[i]) return false;
    }
  }
  return true; // same arrays
}

void GenericGrowableArray::clear_and_deallocate() {
  assert(on_C_heap(),
	 "clear_and_deallocate should only be called when on C heap");
  clear();
  if (_data != NULL) {
    FreeHeap(_data);
    _data = NULL;
  }
}
