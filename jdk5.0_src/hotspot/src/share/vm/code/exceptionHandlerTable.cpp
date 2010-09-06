#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)exceptionHandlerTable.cpp	1.24 04/04/14 17:27:21 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_exceptionHandlerTable.cpp.incl"

#ifdef COMPILER2

void ExceptionHandlerTable::add_entry(HandlerTableEntry entry) {
  _nesting.check();
  if (_length >= _size) {
    // not enough space => grow the table (amortized growth, double its size)
    guarantee(_size > 0, "no space allocated => cannot grow the table since it is part of nmethod");
    int new_size = _size * 2;
    _table = REALLOC_RESOURCE_ARRAY(HandlerTableEntry, _table, _size, new_size);
    _size = new_size;
  }
  assert(_length < _size, "sanity check");
  _table[_length++] = entry;
}


HandlerTableEntry* ExceptionHandlerTable::subtable_for(int catch_pco) const {
  int i = 0;
  while (i < _length) {
    HandlerTableEntry* t = _table + i;
    if (t->pco() == catch_pco) {
      // found subtable matching the catch_pco
      return t;
    } else {
      // advance to next subtable
      i += t->len() + 1; // +1 for header
    }
  }
  return NULL;
}


ExceptionHandlerTable::ExceptionHandlerTable(int initial_size) {
  guarantee(initial_size > 0, "initial size must be > 0");
  _table  = NEW_RESOURCE_ARRAY(HandlerTableEntry, initial_size);
  _length = 0;
  _size   = initial_size;
}


ExceptionHandlerTable::ExceptionHandlerTable(const nmethod* nm) {
  _table  = (HandlerTableEntry*)nm->handler_table_begin();
  _length = nm->handler_table_size() / sizeof(HandlerTableEntry);
  _size   = 0; // no space allocated by ExeptionHandlerTable!
}


void ExceptionHandlerTable::add_subtable(
  int                 catch_pco,
  GrowableArray<intptr_t>* handler_bcis,
  GrowableArray<intptr_t>* handler_pcos
) {
  assert(subtable_for(catch_pco) == NULL, "catch handlers for this catch_pco added twice");
  assert(handler_bcis->length() == handler_pcos->length(), "bci & pc table have different length");
  if (handler_bcis->length() > 0) {
    // add subtable header
    add_entry(HandlerTableEntry(handler_bcis->length(), catch_pco));
    // add individual entries
    for (int i = 0; i < handler_bcis->length(); i++) {
      add_entry(HandlerTableEntry(handler_bcis->at(i), handler_pcos->at(i)));
      assert(entry_for(catch_pco, handler_bcis->at(i))->pco() == handler_pcos->at(i), "entry not added correctly");
    }
  }
}


void ExceptionHandlerTable::copy_to(nmethod* nm) {
  assert(size_in_bytes() == nm->handler_table_size(), "size of space allocated in nmethod incorrect");
  memmove(nm->handler_table_begin(), _table, size_in_bytes());
}


HandlerTableEntry* ExceptionHandlerTable::entry_for(int catch_pco, int handler_bci) const {
  HandlerTableEntry* t = subtable_for(catch_pco);
  if (t != NULL) {
    int l = t->len();
    while (l-- > 0) {
      t++;
      if (t->bci() == handler_bci) return t;
    }
  }
  return NULL;
}


void ExceptionHandlerTable::print_subtable(HandlerTableEntry* t) const {
  int l = t->len();
  tty->print_cr("catch_pco = %d (%d entries)", t->pco(), l);
  while (l-- > 0) {
    t++;
    tty->print_cr("  bci %d -> pco %d", t->bci(), t->pco());
  }
}


void ExceptionHandlerTable::print() const {
  tty->print_cr("ExceptionHandlerTable (size = %d bytes)", size_in_bytes());
  int i = 0;
  while (i < _length) {
    HandlerTableEntry* t = _table + i;
    print_subtable(t);
    // advance to next subtable
    i += t->len() + 1; // +1 for header
  }
}

void ExceptionHandlerTable::print_subtable_for(int catch_pco) const {
  HandlerTableEntry* subtable = subtable_for(catch_pco);

  if( subtable != NULL ) { print_subtable( subtable ); }
}

#endif /* COMPILER2 */

// ----------------------------------------------------------------------------
// Implicit null exception tables.  Maps an exception PC offset to a
// continuation PC offset.  Table has a first length word, then pairs of
// <excp-offset, const-offset>.
void ImplicitExceptionTable::set_size( uint size ) {
  _size = size;
  _data = NEW_RESOURCE_ARRAY(implicit_null_entry, (size*2+1));
  _data[0] = 0;                 // NOT size!
}

void ImplicitExceptionTable::append( uint exec_off, uint cont_off ) {
  assert( (sizeof(implicit_null_entry) >= 4) || (exec_off < 65535), "" );
  assert( (sizeof(implicit_null_entry) >= 4) || (cont_off < 65535), "" );
  uint l = len();
  if (l == _size) {
    uint old_size_in_elements = _size*2 +1;
    if (_size == 0) _size = 4;
    _size *= 2;
    uint new_size_in_elements = _size*2 +1;
    _data = REALLOC_RESOURCE_ARRAY(uint, _data, old_size_in_elements, new_size_in_elements);
  }
  *(adr(l)  ) = exec_off;
  *(adr(l)+1) = cont_off;
  _data[0] = l+1;
};

void ImplicitExceptionTable::replicate_faulting_entry( uint old_exec_off, uint new_exec_off ) {
  _nesting.check();
  uint l = len();
  for( uint i=0; i<l; i++ ) {
    if( *adr(i) == old_exec_off ) {
      uint cont_off = *(adr(i)+1); // get it before reallocation (_data may move)
      if (_size <= len()) {
        uint old_size_in_elements = _size*2 +1;
        _size *= 2;
        uint new_size_in_elements = _size*2 +1;
        _data = REALLOC_RESOURCE_ARRAY(uint, _data, old_size_in_elements, new_size_in_elements);
      }
      append(new_exec_off, cont_off);
      break;
    }
  }
};

uint ImplicitExceptionTable::at( uint exec_off ) const {
  uint l = len();
  for( uint i=0; i<l; i++ )
    if( *adr(i) == exec_off )
      return *(adr(i)+1);
  return 0;                     // Failed to find any execption offset
}

void ImplicitExceptionTable::print(address base) const {
  tty->print("{");
  for( uint i=0; i<len(); i++ )
    tty->print("< "INTPTR_FORMAT", "INTPTR_FORMAT" > ",base + *adr(i), base + *(adr(i)+1));
  tty->print_cr("}");
}

ImplicitExceptionTable::ImplicitExceptionTable(const nmethod* nm) {
  _data  = (implicit_null_entry*)nm->nul_chk_table_begin();
  _size = len();
}

void ImplicitExceptionTable::copy_to( nmethod* nm ) {
  assert(size_in_bytes() <= nm->nul_chk_table_size(), "size of space allocated in nmethod incorrect");
  memmove( nm->nul_chk_table_begin(), _data, size_in_bytes() );
}

void ImplicitExceptionTable::verify(nmethod *nm) const {
  for (uint i = 0; i < len(); i++) {
     if ((*adr(i) > (unsigned int)nm->code_size()) ||
         (*(adr(i)+1) > (unsigned int)nm->code_size()))
       fatal1("Invalid offset in ImplicitExceptionTable at %lx", _data);
  }
}


//----------------------------------------------------------------------
#ifdef COMPILER1
void ExceptionRangeTable::add_entry(ExceptionRangeEntry entry) {
#ifdef ASSERT
  if (_length > 0) {
    assert(entry.start_pco() >= _table[_length - 1].start_pco(),
           "entries must be added in ascending PC offset order");
  }
#endif /* ASSERT */
  if (_length >= _size) {
    // not enough space => grow the table (amortized growth, double its size)
    int new_size = _size * 2;
    _table = REALLOC_C_HEAP_ARRAY(ExceptionRangeEntry, _table, new_size);
    _size = new_size;
  }
  assert(_length < _size, "sanity check");
  _table[_length++] = entry;
}

ExceptionRangeEntry::ExceptionRangeEntry(int start_pco, int end_pco, int scope_count,
                                         int exception_type, int handler_pco, int handler_bci)
  : _start_pco(start_pco)
  , _end_pco(end_pco)
  , _scope_count(scope_count)
  , _exception_type(exception_type)
  , _handler_pco(handler_pco)
  , _handler_bci(handler_bci)
{
}

ExceptionRangeTable::ExceptionRangeTable(int initial_size) {
  guarantee(initial_size > 0, "initial size must be > 0");
  _table  = NEW_C_HEAP_ARRAY(ExceptionRangeEntry, initial_size);
  _length = 0;
  _size   = initial_size;
}

ExceptionRangeTable::~ExceptionRangeTable() {
  FREE_C_HEAP_ARRAY(ExceptionRangeEntry, _table);
}

int ExceptionRangeTable::compute_modified_at_call_pco(int pco, bool at_call) {
  // If at_call is true, we have to return a PC offset that is less
  // than the current one but greater than any previously recorded
  // one. We can not subtract NativeCall::instruction_size (which in
  // some sense would be the correct thing to do) because (at least
  // for C1) we can emit "at_call" debug information for non-call
  // instructions, in particular the nop that is at the prologue of an
  // athrow bytecode's generated code, which can cause us to end up
  // before a potentially exception-throwing instruction that comes
  // just before the nop.
  return pco - (at_call ? 1 : 0);
}

void ExceptionRangeTable::add_entry(int start_pco, int end_pco,
                                    int scope_count, int exception_type,
                                    int handler_pco, int handler_bci) {
  add_entry(ExceptionRangeEntry(start_pco, end_pco,
                                scope_count, exception_type,
                                handler_pco, handler_bci));
}

ExceptionRangeEntry* ExceptionRangeTable::entry_at(int i) const {
  assert(0 <= i && i < _length, "index out of bounds");
  return &_table[i];
}

int ExceptionRangeTable::entry_index_for_pco(int pco) const {
  // Should change to use binary search
  int dist = -1;
  int ret  = -1;
  for (int i = length() - 1; i >= 0; --i) {
    int start_pco = _table[i].start_pco();
    if (start_pco <= pco) {
      int tmp_dist = pco - start_pco;
      if (dist < 0) {
        dist = tmp_dist;
      } else if (tmp_dist > dist) {
        return i + 1;
      }
      ret = i;
    }
  }

  return ret;
}

void ExceptionRangeTable::copy_to(nmethod* nm) {
  ExceptionRangeTable* nmtable = (ExceptionRangeTable*)nm->handler_table_begin();
  *nmtable = *this;
  nmtable->_table = (ExceptionRangeEntry*)(((address)nmtable) + sizeof(ExceptionRangeTable));
  nmtable->_size = nmtable->_length;
  memcpy(nmtable->_table, _table, _length * sizeof(ExceptionRangeEntry));
}

#ifndef PRODUCT
void ExceptionRangeTable::print(address base) const {
  tty->print_cr("ExceptionRangeTable (size = %d bytes)", size_in_bytes());
  for (int i = 0; i < length(); i++) {
    ExceptionRangeEntry* e = entry_at(i);
    tty->print_cr("("INTPTR_FORMAT" "INTPTR_FORMAT") scope count %d type %d handler pco "INTPTR_FORMAT" bci %d\n",
                  base + e->start_pco(),
                  base + e->end_pco(),
                  e->scope_count(),
                  e->exception_type(),
                  base + e->handler_pco(),
                  e->handler_bci());
  }
}
#endif


#endif /* COMPILER1 */
