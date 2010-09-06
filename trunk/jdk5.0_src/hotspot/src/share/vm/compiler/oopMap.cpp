#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)oopMap.cpp	1.133 03/12/23 16:40:04 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_oopMap.cpp.incl"

// OopMapStream

OopMapStream::OopMapStream(OopMap* oop_map) {
  if(oop_map->omv_data() == NULL) {
    _stream = new CompressedReadStream(oop_map->write_stream()->buffer());
  } else {
    _stream = new CompressedReadStream(oop_map->omv_data());
  }
  _mask = OopMapValue::type_mask_in_place;
  _size = oop_map->omv_count();
  _position = 0;
  _valid_omv = false;
}


OopMapStream::OopMapStream(OopMap* oop_map, int oop_types_mask) {
  if(oop_map->omv_data() == NULL) {
    _stream = new CompressedReadStream(oop_map->write_stream()->buffer());
  } else {
    _stream = new CompressedReadStream(oop_map->omv_data());
  }
  _mask = oop_types_mask;
  _size = oop_map->omv_count();
  _position = 0;
  _valid_omv = false;
}


void OopMapStream::find_next() {
  while(_position++ < _size) {
    _omv.read_from(_stream);
    if(((int)_omv.type() & _mask) > 0) {
      _valid_omv = true;
      return;
    }
  }
  _valid_omv = false;
}


// OopMap

// frame_size units are stack-slots (4 bytes) NOT intptr_t; we can name odd
// slots to hold 4-byte values like ints and floats in the LP64 build.
OopMap::OopMap(int frame_size, int arg_count) {
  int min_size = (frame_size + arg_count) << LogBytesPerInt;
  set_write_stream(new CompressedWriteStream(min_size));
  set_omv_data(NULL);
  set_omv_count(0);

#ifdef ASSERT  
  _locs_length = SharedInfo::stack0 + frame_size + arg_count;
  _locs_used   = NEW_RESOURCE_ARRAY(OopMapValue::oop_types, _locs_length);
  for(int i = 0; i < _locs_length; i++) _locs_used[i] = OopMapValue::unused_value;
#endif
}


OopMap::OopMap(OopMap::DeepCopyToken, OopMap* source) {
  // This constructor does a deep copy
  // of the source OopMap.
  set_write_stream(new CompressedWriteStream(source->omv_count() * 2));
  set_omv_data(NULL);
  set_omv_count(0);
  set_offset(source->offset(),source->at_call());

#ifdef ASSERT  
  _locs_length = source->_locs_length;
  _locs_used = NEW_RESOURCE_ARRAY(OopMapValue::oop_types, _locs_length);
  for(int i = 0; i < _locs_length; i++) _locs_used[i] = OopMapValue::unused_value;
#endif

  // We need to copy the entries too.
  for (OopMapStream oms(source); !oms.is_done(); oms.next()) {
    OopMapValue omv = oms.current();
    omv.write_on(write_stream());
    increment_count();
  }
}


OopMap* OopMap::deep_copy() {
  return new OopMap(_deep_copy_token, this);
}


void OopMap::copy_to(address addr) {
  memcpy(addr,this,sizeof(OopMap));
  memcpy(addr + sizeof(OopMap),write_stream()->buffer(),write_stream()->position());
  OopMap* new_oop = (OopMap*)addr;
  new_oop->set_omv_data_size(write_stream()->position());
  new_oop->set_omv_data((unsigned char *)(addr + sizeof(OopMap)));
  new_oop->set_write_stream(NULL);
}


int OopMap::heap_size() const {
  int size = sizeof(OopMap);
  int align = sizeof(void *) - 1;
  if(write_stream() != NULL) {
    size += write_stream()->position();
  } else {
    size += omv_data_size();
  }
  // Align to a reasonable ending point
  size = ((size+align) & ~align);
  return size;
}


// frame_size units are stack-slots (4 bytes) NOT intptr_t; we can name odd
// slots to hold 4-byte values like ints and floats in the LP64 build.
VMReg::Name OopMap::map_compiler_reg_to_oopmap_reg(OptoReg::Name reg, int frame_size, int arg_count) {
#ifdef COMPILER1
  return VMReg::Name(reg);
#else
  if (reg < SharedInfo::stack0) {
    return VMReg::Name(reg);
  } else {
    return (reg < SharedInfo::stack2reg(arg_count))
      ? VMReg::Name(reg + frame_size)
      : VMReg::Name(reg - arg_count);
  }
#endif
}


// frame_size units are stack-slots (4 bytes) NOT intptr_t; we can name odd
// slots to hold 4-byte values like ints and floats in the LP64 build.
void OopMap::set_xxx(OptoReg::Name reg, OopMapValue::oop_types x, int frame_size, int arg_count, OptoReg::Name optional) {
  // The compiler uses a different register numbering scheme than the rest of 
  // the VM.  In both systems register numbers below SharedInfo::stack0 refer
  // to machine registers (output by the ADLC).  In the compiler, numbers above
  // SharedInfo::stack0 are 'warped' by the framesize.  This is because the
  // allocator has to have some kind of numbering scheme to use before it knows
  // how big a frame is.  Afterwards, the VM just wants stack offsets, so it 
  // uses 'unwarped' numbers.
  
  assert((int)reg < _locs_length, "too big reg value for stack size");
  assert( _locs_used[(int)reg] == OopMapValue::unused_value, "cannot insert twice" );
  debug_only( _locs_used[(int)reg] = x; )

  OopMapValue o(map_compiler_reg_to_oopmap_reg(reg,frame_size,arg_count),x);

  if(x == OopMapValue::callee_saved_value) {
    // This can never be a stack location, so we don't need to transform it.
    assert(optional < SharedInfo::stack0,"Trying to callee save a stack location");
    o.set_content_reg(VMReg::Name(optional));
  } else if(x == OopMapValue::derived_oop_value) {
    o.set_content_reg(map_compiler_reg_to_oopmap_reg(optional,frame_size,arg_count));
  }

  o.write_on(write_stream());
  increment_count();
}


void OopMap::set_oop(OptoReg::Name reg, int frame_size, int arg_count) {
  set_xxx(reg, OopMapValue::oop_value, frame_size, arg_count, OptoReg::Bad);
}


void OopMap::set_value(OptoReg::Name reg, int frame_size, int arg_count) {
  // At this time, we only need value entries in our OopMap when ZapDeadCompiledLocals is active.
  if (ZapDeadCompiledLocals)
    set_xxx(reg, OopMapValue::value_value, frame_size, arg_count, OptoReg::Bad);
}


void OopMap::set_dead(OptoReg::Name reg, int frame_size, int arg_count) {
  // At this time, we only need dead entries in our OopMap when ZapDeadCompiledLocals is active.
  if (ZapDeadCompiledLocals) {
    set_xxx(reg, OopMapValue::dead_value, frame_size, arg_count, OptoReg::Bad);
  }
}


void OopMap::set_callee_saved(OptoReg::Name reg, int frame_size, int arg_count, OptoReg::Name caller_machine_register ) {
  set_xxx(reg, OopMapValue::callee_saved_value, frame_size, arg_count, caller_machine_register);
}


void OopMap::set_derived_oop(OptoReg::Name reg, int frame_size, int arg_count, OptoReg::Name derived_from_local_register ) {
  if( reg == derived_from_local_register ) {
    // Actually an oop, derived shares storage with base, 
    set_oop(reg, frame_size, arg_count);
  } else {
    set_xxx(reg, OopMapValue::derived_oop_value, frame_size, arg_count, derived_from_local_register);
  }
}


// This routine sets all unused_value slots in [start..limit) to dead_value.
void OopMap::set_unused_as_dead(int start, int limit, int frame_size, int arg_count) {
  assert (ZapDeadCompiledLocals,
          "set_unused_as_dead should only be called when ZapDeadCompiledLocals is active");
#ifdef ASSERT
  for (int i = start; i < limit; i++) {
    if (_locs_used[i] == OopMapValue::unused_value) {
      set_dead(OptoReg::Name(i), frame_size, arg_count);
    }
  }
#else
  Unimplemented();
#endif
}


// OopMapSet

OopMapSet::OopMapSet() {
  set_om_size(MinOopMapAllocation);
  set_om_count(0);
  OopMap** temp = NEW_RESOURCE_ARRAY(OopMap*, om_size());
  set_om_data(temp);
}


void OopMapSet::grow_om_data() {  
  int new_size = om_size() * 2;
  OopMap** new_data = NEW_RESOURCE_ARRAY(OopMap*, new_size);
  memcpy(new_data,om_data(),om_size() * sizeof(OopMap*));
  set_om_size(new_size);
  set_om_data(new_data);
}


void OopMapSet::copy_to(address addr) {
  address temp = addr;
  int align = sizeof(void *) - 1;
  // Copy this
  memcpy(addr,this,sizeof(OopMapSet));
  temp += sizeof(OopMapSet);
  temp = (address)((intptr_t)(temp + align) & ~align);
  // Do the needed fixups to the new OopMapSet
  OopMapSet* new_set = (OopMapSet*)addr;
  new_set->set_om_data((OopMap**)temp);
  // Allow enough space for the OopMap pointers
  temp += (om_count() * sizeof(OopMap*));

  for(int i=0; i < om_count(); i++) {
    OopMap* map = at(i);
    map->copy_to((address)temp);
    new_set->set(i,(OopMap*)temp);
    temp += map->heap_size();
  }
  // This "locks" the OopMapSet
  new_set->set_om_size(-1);
}


void OopMapSet::add_gc_map(int pc_offset, bool at_call, OopMap *map ) {
  assert(om_size() != -1,"Cannot grow a fixed OopMapSet");

  if(om_count() >= om_size()) {
    grow_om_data();
  }
  map->set_offset(pc_offset, at_call);

#ifdef ASSERT
  if(om_count() > 0) {
    OopMap* last = at(om_count()-1);
    if (last->offset() == map->offset() &&
        last->at_call() == map->at_call()) {
      fatal("OopMap inserted twice");
    }
    if(last->offset() > map->offset()) {
      tty->print_cr( "WARNING, maps not sorted: pc[%d]=%d, pc[%d]=%d",
                      om_count(),last->offset(),om_count()+1,map->offset());
    }
  }
#endif // ASSERT

  set(om_count(),map);
  increment_count();
}


int OopMapSet::heap_size() const {
  // The space we use
  int size = sizeof(OopMap);
  int align = sizeof(void *) - 1;
  size = ((size+align) & ~align);
  size += om_count() * sizeof(OopMap*);

  // Now add in the space needed for the indivdiual OopMaps
  for(int i=0; i < om_count(); i++) {
    size += at(i)->heap_size();
  }
  // We don't need to align this, it will be naturally pointer aligned
  return size;
}


OopMap* OopMapSet::singular_oop_map() {
  guarantee(om_count() == 1, "Make sure we only have a single gc point");
  return at(0);
}


OopMap* OopMapSet::find_map_at_offset(int pc_offset, bool at_call) const {
  int i, len = om_count();
  assert( len > 0, "must have pointer maps" );

  // Scan through oopmaps. Stop when current offset is either equal or greater
  // than the one we are looking for.
  for( i = 0; i < len; i++) {
    if( at(i)->offset() >= pc_offset )
      break;
  }
  
  assert( i < len, "oopmap not found" );
  assert( at(i)->offset() == pc_offset, "oopmap not found" );

  OopMap* m = at(i);
  if (m->at_call() != at_call) {    
    i += 1; // It must be the next
    assert( i < len, "oopmap not found" );
    m = at(i);
    assert(m->at_call() == at_call, "wrong at_call state found in oopmap");
    assert(m->offset() == pc_offset, "wrong offset found in oopmap");    
  }
  return m;
}

class DoNothingClosure: public OopClosure {
public: void do_oop(oop* p) {}
};
static DoNothingClosure do_nothing;

static void add_derived_oop(oop* base, oop* derived) {
  COMPILER1_ONLY(ShouldNotReachHere();)
  COMPILER2_ONLY(DerivedPointerTable::add(derived, base);)
}


#ifndef CORE
#ifndef PRODUCT
static void trace_codeblob_maps(const frame *fr, CodeBlob* cb, const RegisterMap *reg_map) {
  // Print oopmap and regmap
  tty->print_cr("------ ");
  OopMapSet* maps = cb->oop_maps();
  OopMap* map = cb->oop_map_for_return_address(fr->pc(), reg_map->is_pc_at_call(fr->id()));
  map->print();
  if( cb->is_nmethod() ) {
    nmethod* nm = (nmethod*)cb;            
    ScopeDesc* scope  = nm->scope_desc_at(fr->pc(), reg_map->is_pc_at_call(fr->id()));
    tty->print("bci: %d ",scope->bci());
  }
  tty->cr();
  fr->print_on(tty);   
  tty->print("     "); 
  cb->print_value_on(tty);  tty->cr();
  reg_map->print();
  tty->print_cr("------ ");
  
}
#endif // PRODUCT

void OopMapSet::oops_do(const frame *fr, CodeBlob* cb, const RegisterMap* reg_map, OopClosure* f) {
  // add derived oops to a table
  all_do(fr, cb, reg_map, f, add_derived_oop, &do_nothing, &do_nothing);
}


void OopMapSet::all_do(const frame *fr, CodeBlob* cb, const RegisterMap *reg_map, 
                       OopClosure* oop_fn, void derived_oop_fn(oop*, oop*),
		       OopClosure* value_fn, OopClosure* dead_fn) {    
  { debug_only(CodeBlob* t_cb = CodeCache::find_blob(fr->pc());)
    assert(cb != NULL && cb == t_cb, "wrong codeblob passed in");      
  }  

  NOT_PRODUCT(if (TraceCodeBlobStacks) trace_codeblob_maps(fr, cb, reg_map);)

  OopMapSet* maps = cb->oop_maps();
  OopMap* map  = cb->oop_map_for_return_address(fr->pc(), reg_map->is_pc_at_call(fr->id()));
  assert(map != NULL, " no ptr map found");   
  
  // handle derived pointers first (otherwise base pointer may be
  // changed before derived pointer offset has been collected)
  OopMapValue omv;
  {    
    OopMapStream oms(map,OopMapValue::derived_oop_value);
    if (!oms.is_done()) {
      COMPILER1_ONLY(ShouldNotReachHere();)
      // Protect the operation on the derived pointers.  This
      // protects the addition of derived pointers to the shared 
      // derived pointer table in DerivedPointerTable::add().
      MutexLockerEx x(DerivedPointerTableGC_lock, Mutex::_no_safepoint_check_flag);
      do {
        omv = oms.current();
        oop* loc = fr->oopmapreg_to_location(omv.reg(),reg_map);
        if ( loc != NULL ) {
          oop *base_loc    = fr->oopmapreg_to_location(omv.content_reg(), reg_map);
          oop *derived_loc = loc;
          derived_oop_fn(base_loc, derived_loc); 
        }
	oms.next();
      }  while (!oms.is_done()); 
    }
  }

  // We want dead, value and oop oop_types
  int mask = OopMapValue::oop_value | OopMapValue::value_value | OopMapValue::dead_value;
  {
    for (OopMapStream oms(map,mask); !oms.is_done(); oms.next()) {
      omv = oms.current();
      oop* loc = fr->oopmapreg_to_location(omv.reg(),reg_map);
      if ( loc != NULL ) {
        if ( omv.type() == OopMapValue::oop_value ) {
#ifdef ASSERT
          if (!Universe::heap()->is_in_or_null(*loc)) {
            tty->print_cr("# Found non oop pointer.  Dumping state at failure");
            // try to dump out some helpful debugging information
            trace_codeblob_maps(fr, cb, reg_map);
            omv.print();
            tty->print_cr("loc = %p *loc = %p\n", loc, *loc);
            // do the real assert.
            assert(Universe::heap()->is_in_or_null(*loc), "found non oop pointer");
          }
#endif // ASSERT
          oop_fn->do_oop(loc);
        } else if ( omv.type() == OopMapValue::value_value ) {
          value_fn->do_oop(loc);
        } else if ( omv.type() == OopMapValue::dead_value ) {
          dead_fn->do_oop(loc);
        }
      }
    }
  }
}


// Update callee-saved register info for the following frame
void OopMapSet::update_register_map(const frame *fr, CodeBlob* cb, RegisterMap *reg_map) {
  ResourceMark rm;

  // Any reg might be saved by a safepoint handler.
  // (See generate_illegal_instruction_handler_blob.)
  const int max_saved_on_entry_reg_count = REG_COUNT;
  assert( reg_map->_update_for_id == NULL || fr->is_older(reg_map->_update_for_id),
         "already updated this map; do not 'update' it twice!" );
  debug_only(reg_map->_update_for_id = fr->id());

  // Check if caller must update oop argument  
  reg_map->set_include_argument_oops(cb->caller_must_gc_arguments(reg_map->thread()));

  int nof_callee = 0;
  oop*        locs[2*max_saved_on_entry_reg_count+1]; 
  VMReg::Name regs[2*max_saved_on_entry_reg_count+1]; 
  // ("+1" because max_saved_on_entry_reg_count might be zero)

  // Scan through oopmap and find location of all callee-saved registers
  // (we do not do update in place, since info could be overwritten)
  OopMap* map  = cb->oop_map_for_return_address(fr->pc(), reg_map->is_pc_at_call(fr->id()));
  assert(map != NULL, " no ptr map found"); 

  OopMapValue omv;
  for(OopMapStream oms(map,OopMapValue::callee_saved_value); !oms.is_done(); oms.next()) {
    omv = oms.current();
    assert(nof_callee < 2*max_saved_on_entry_reg_count, "overflow");
    regs[nof_callee] = omv.content_reg();
    locs[nof_callee] = fr->oopmapreg_to_location(omv.reg(),reg_map);
    nof_callee++;
  }

  // Check that runtime stubs and C2I adapters save all callee-saved registers
#ifdef COMPILER2
  assert(!cb->is_c2i_adapter() || nof_callee == SAVED_ON_ENTRY_REG_COUNT, "C2I must save all");
  assert(!cb->is_runtime_stub() || 
         (nof_callee >= SAVED_ON_ENTRY_REG_COUNT || nof_callee >= C_SAVED_ON_ENTRY_REG_COUNT),
         "must save all");
#endif // COMPILER2

  // Copy found callee-saved register to reg_map
  for(int i = 0; i < nof_callee; i++) {
    reg_map->set_location(regs[i], (address)locs[i]);
  }
}
#endif // !CORE


//=============================================================================
// Non-Product code

#ifndef PRODUCT

bool OopMap::has_derived_pointer() const {
#ifdef COMPILER2
  OopMapStream oms((OopMap*)this,OopMapValue::derived_oop_value);
  return oms.is_done();
#else
  return false;
#endif // COMPILER2
}


void print_register_name(VMReg::Name reg) {
  if( reg < SharedInfo::stack0 ) {
    assert( SharedInfo::regName[reg], "" );
    tty->print("%s",SharedInfo::regName[reg]);
  } else {
    int stk = reg - SharedInfo::stack0;
    tty->print("[%d]", stk*4);
  }
}


void print_register_type(OopMapValue::oop_types x, VMReg::Name optional) {
  switch( x ) {
  case OopMapValue::oop_value:
    tty->print("Oop");
    break;
  case OopMapValue::value_value:
    tty->print("Value" );
    break;
  case OopMapValue::dead_value:
    tty->print("Dead" );
    break;
  case OopMapValue::callee_saved_value:
    tty->print("Callers_" );
    print_register_name(optional);
    break;
  case OopMapValue::derived_oop_value:
    tty->print("Derived_oop_" );
    print_register_name(optional);
    break;
  default:
    ShouldNotReachHere();
  }
}


void OopMapValue::print() const {
  print_register_name(reg());
  tty->print("=");
  print_register_type(type(),content_reg());
  tty->print(" ");
}


void OopMap::print() const {
  OopMapValue omv;
  for(OopMapStream oms((OopMap*)this); !oms.is_done(); oms.next()) {
    omv = oms.current();
    omv.print();
  }
}


void OopMapSet::print() const {
  int i, len = om_count();

  tty->print_cr("OopMapSet contains %d OopMaps\n",len);
  
  for( i = 0; i < len; i++) {
    OopMap* m = at(i);
    tty->print_cr("OopMap #%d offset:%p",i,m->offset());
    tty->print_cr("OopMap #%d at_call:%d",i,m->at_call());
    m->print();
    tty->print_cr("\n");
  }
}

#endif // !PRODUCT


//------------------------------DerivedPointerTable---------------------------

#ifdef COMPILER2

class DerivedPointerEntry : public CHeapObj {
 private:
  oop*     _location; // Location of derived pointer (also pointing to the base)
  intptr_t _offset;   // Offset from base pointer   
 public:
  DerivedPointerEntry(oop* location, intptr_t offset) { _location = location; _offset = offset; }
  oop* location()    { return _location; }
  intptr_t  offset() { return _offset; }
};


GrowableArray<DerivedPointerEntry*>* DerivedPointerTable::_list = NULL;
bool DerivedPointerTable::_active = false;


void DerivedPointerTable::clear() {
  // The first time, we create the list.  Otherwise it should be
  // empty.  If not, then we have probably forgotton to call
  // update_pointers after last GC/Scavenge.
  assert (!_active, "should not be active");
  assert(_list == NULL || _list->length() == 0, "table not empty");
  if (_list == NULL) {
    _list = new (ResourceObj::C_HEAP) GrowableArray<DerivedPointerEntry*>(10, true); // Allocated on C heap
  }
  _active = true;
}


// Returns value of location as an int
intptr_t value_of_loc(oop *pointer) { return (intptr_t)(*pointer); }


void DerivedPointerTable::add(oop *derived_loc, oop *base_loc) {      
  assert(Universe::heap()->is_in_or_null(*base_loc), "not an oop");  
  assert(derived_loc != base_loc, "Base and derived in same location");
  if (_active) {
    assert(*derived_loc != (oop)base_loc, "location already added");    
    assert(_list != NULL, "list must exist");
    intptr_t offset = value_of_loc(derived_loc) - value_of_loc(base_loc);
    assert(offset >= -1000000, "wrong derived pointer info");

    if (TraceDerivedPointers) {
      tty->print_cr(
        "Add derived pointer@" INTPTR_FORMAT 
	" - Derived: " INTPTR_FORMAT 
	" Base: " INTPTR_FORMAT " (@" INTPTR_FORMAT ") (Offset: %d)", 
        derived_loc, *derived_loc, *base_loc, base_loc, offset
      );
    }    
    // Set derived oop location to point to base.
    *derived_loc = (oop)base_loc;  
    assert_lock_strong(DerivedPointerTableGC_lock);
    DerivedPointerEntry *entry = new DerivedPointerEntry(derived_loc, offset);
    _list->append(entry);    
  }
}


void DerivedPointerTable::update_pointers() {  
  assert(_list != NULL, "list must exist");
  for(int i = 0; i < _list->length(); i++) {
    DerivedPointerEntry* entry = _list->at(i);
    oop* derived_loc = entry->location();
    intptr_t offset  = entry->offset();
    // The derived oop was setup to point to location of base
    oop  base        = **(oop**)derived_loc; 
    assert(Universe::heap()->is_in_or_null(base), "must be an oop");
    
    *derived_loc = (oop)(((address)base) + offset);
    assert(value_of_loc(derived_loc) - value_of_loc(&base) == offset, "sanity check");

    if (TraceDerivedPointers) {
      tty->print_cr("Updating derived pointer@" INTPTR_FORMAT 
		    " - Derived: " INTPTR_FORMAT "  Base: " INTPTR_FORMAT " (Offset: %d)",
          derived_loc, *derived_loc, base, offset);
    }

    // Delete entry
    delete entry;
    _list->at_put(i, NULL);
  }
  // Clear list, so it is ready for next traversal (this is an invariant)
  if (TraceDerivedPointers && !_list->is_empty()) {
    tty->print_cr("--------------------------");
  }
  _list->clear();
  _active = false;
}

#endif // COMPILER2
