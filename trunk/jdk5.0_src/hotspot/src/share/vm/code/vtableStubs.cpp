#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)vtableStubs.cpp	1.40 04/04/05 13:05:48 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_vtableStubs.cpp.incl"

// -----------------------------------------------------------------------------------------
// Implementation of VtableStub

address VtableStub::_chunk = NULL;
address VtableStub::_chunk_end = NULL;
static int num_vtable_chunks = 0;


void* VtableStub::operator new(size_t size, int code_size) {
  assert(size == sizeof(VtableStub), "mismatched size");
  num_vtable_chunks++;
  // compute real VtableStub size (rounded to nearest word)
  const int real_size = round_to(code_size + sizeof(VtableStub), wordSize);
  // malloc them in chunks to minimize header overhead
  const int chunk_factor = 32;
  if (_chunk == NULL || _chunk + real_size > _chunk_end) {
    const int bytes = chunk_factor * real_size + pd_code_alignment();
    BufferBlob* blob = BufferBlob::create("vtable chunks", bytes);
    if( blob == NULL ) fatal1( "CodeCache: no room for %s", "vtable chunks");
    _chunk = blob->instructions_begin();
    _chunk_end = _chunk + bytes;
    VTune::register_stub("vtable stub", _chunk, _chunk_end);
    Forte::register_stub("vtable stub", _chunk, _chunk_end);
    // Notify JVMTI about this stub. The event will be recorded by the enclosing
    // JvmtiDynamicCodeEventCollector and posted when this thread has released
    // all locks.
    if (JvmtiExport::should_post_dynamic_code_generated()) {
      JvmtiExport::post_dynamic_code_generated_while_holding_locks("vtable stub", _chunk, _chunk_end);
    }
    align_chunk();
  } 
  assert(_chunk + real_size <= _chunk_end, "bad allocation");
  void* res = _chunk;
  _chunk += real_size;
  align_chunk();
 return res;
}


void VtableStub::print() {
  tty->print("vtable stub (index = %d, receiver_location = %d, code = [" INTPTR_FORMAT ", " INTPTR_FORMAT "[)",
	     index(), receiver_location(), code_begin(), code_end());
}


// -----------------------------------------------------------------------------------------
// Implementation of VtableStubs
//
// For each hash value there's a linked list of vtable stubs (with that
// hash value). Each list is anchored in a little hash _table, indexed
// by that hash value.

VtableStub* VtableStubs::_table[VtableStubs::N];
int VtableStubs::_number_of_vtable_stubs = 0;


void VtableStubs::initialize() {
  MutexLocker ml(VtableStubs_lock);
  assert(_number_of_vtable_stubs == 0, "potential performance bug: VtableStubs initialized more than once");
  assert(is_power_of_2(N), "N must be a power of 2");
  for (int i = 0; i < N; i++) _table[i] = NULL;
}


address VtableStubs::create_stub(bool is_vtable_stub, int vtable_index, methodOop method) {
  assert(vtable_index >= 0, "must be positive");
  int rec;
  COMPILER1_ONLY(rec = 0;)
  COMPILER2_ONLY(rec = Matcher::find_receiver(true);)
  
  VtableStub* s = ShareVtableStubs ? lookup(is_vtable_stub, vtable_index, rec) : NULL;
  if (s == NULL) {
    if (is_vtable_stub) {
      s = create_vtable_stub(vtable_index, rec);
    } else {
      s = create_itable_stub(vtable_index, rec);
    }
    enter(is_vtable_stub, vtable_index, rec, s);
  }
  return s->entry_point();
}


inline unsigned int VtableStubs::hash(bool is_vtable_stub, int vtable_index, int receiver_location){
  // Assumption: receiver_location < 4 in most cases.
  int hash = ((vtable_index << 2) ^ receiver_location) + vtable_index;  
  return (is_vtable_stub ? ~hash : hash)  & mask;
}


VtableStub* VtableStubs::lookup(bool is_vtable_stub, int vtable_index, int receiver_location) {
  MutexLocker ml(VtableStubs_lock);
  unsigned hash = VtableStubs::hash(is_vtable_stub, vtable_index, receiver_location);
  VtableStub* s = _table[hash];
  while( s && !s->matches(is_vtable_stub, vtable_index, receiver_location)) s = s->next();
  return s;
}


void VtableStubs::enter(bool is_vtable_stub, int vtable_index,int receiver_location,VtableStub* s) {
  MutexLocker ml(VtableStubs_lock);
  assert(s->matches(is_vtable_stub, vtable_index, receiver_location), "bad vtable stub");
  unsigned int h = VtableStubs::hash(is_vtable_stub, vtable_index, receiver_location);
  // enter s at the beginning of the corresponding list
  s->set_next(_table[h]);
  _table[h] = s;
  _number_of_vtable_stubs++;
}


bool VtableStubs::is_entry_point(address pc) {
  MutexLocker ml(VtableStubs_lock);
  VtableStub* stub = (VtableStub*)(pc - ((VtableStub*)NULL)->entry_point());
  unsigned hash = VtableStubs::hash(stub->is_vtable_stub(), stub->index(), stub->receiver_location());
  VtableStub* s;
  for (s = _table[hash]; s != NULL && s != stub; s = s->next()) ;
  return s == stub;
}


bool VtableStubs::contains(address pc) {
  // simple solution for now - we may want to use
  // a faster way if this function is called often
  return stub_containing(pc) != NULL;
}


VtableStub* VtableStubs::stub_containing(address pc) {
  // Note: No locking needed since any change to the data structure
  //       happens with an atomic store into it (we don't care about
  //       consistency with the _number_of_vtable_stubs counter).
  for (int i = 0; i < N; i++) {
    for (VtableStub* s = _table[i]; s != NULL; s = s->next()) {
      if (s->contains(pc)) return s;
    }
  }
  return NULL;
}

void vtableStubs_init() { 
  VtableStubs::initialize(); 
}


//-----------------------------------------------------------------------------------------------------
// Non-product code
#ifndef PRODUCT

extern "C" void bad_compiled_vtable_index(JavaThread* thread, oop receiver, int index) {
  ResourceMark rm;
  HandleMark hm;
  klassOop klass = receiver->klass();
  instanceKlass* ik = instanceKlass::cast(klass);
  klassVtable* vt = ik->vtable();
  klass->print();
  fatal3("bad compiled vtable dispatch: receiver " INTPTR_FORMAT ", index %d (vtable length %d)", receiver, index, vt->length());
}

#endif / Product





