#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)relocInfo.cpp	1.75 03/12/23 16:39:57 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_relocInfo.cpp.incl"


const RelocationHolder RelocationHolder::none; // its type is relocInfo::none


// Implementation of relocInfo

relocInfo::relocInfo(relocType t, int off, int f) {
  assert(t != data_prefix_tag, "cannot build a prefix this way");
  assert((t & type_mask) == t, "wrong type");
  assert((f & format_mask) == f, "wrong format");
  assert(off >= 0 && off < offset_limit(), "offset out off bounds");
  assert((off & (offset_unit-1)) == 0, "misaligned offset");
  int bits = (off / (unsigned)offset_unit) | (f << offset_width);
  (*this) = relocInfo(t, RAW_BITS, bits);
}

relocInfo* relocInfo::finish_prefix(short* prefix_limit) {
  assert(sizeof(relocInfo) == sizeof(short), "change this code");
  short* p = (short*) data();
  assert(prefix_limit >= p, "must be a valid span of data");
  int plen = prefix_limit - p;
  if (plen == 0) {
    debug_only(_value = 0xFFFF);
    return this;                         // no data: remove self completely
  }
  if (plen == 1) {
    int data0 = p[0];
    if (data0 >= 0 && data0 < immediate_limit) {
      (*this) = immediate_relocInfo(data0); // move data inside self
      return this+1;
    }
  }
  // cannot compact, so just update the count and return the limit pointer
  (*this) = prefix_relocInfo(plen);   // write new datalen
  assert(data() + datalen() == prefix_limit, "pointers must line up");
  return (relocInfo*)prefix_limit;
}


void relocInfo::set_type(relocType t) {
  int old_offset = addr_offset();
  int old_format = format();
  (*this) = relocInfo(t, old_offset, old_format);
  assert(type()==(int)t, "sanity check");
  assert(addr_offset()==old_offset, "sanity check");
  assert(format()==old_format, "sanity check");
}


void relocInfo::set_format(int f) {
  int old_offset = addr_offset();
  assert((f & format_mask) == f, "wrong format");
  _value = (_value & ~(format_mask << offset_width)) | (f << offset_width);
  assert(addr_offset()==old_offset, "sanity check");
}


void relocInfo::change_reloc_info_for_address(RelocIterator *itr, address pc, relocType old_type, relocType new_type) {
  bool found = false;  
  while (itr->next() && !found) {    
    if (itr->addr() == pc) {      
      assert(itr->type()==old_type, "wrong relocInfo type found");
      itr->current()->set_type(new_type);      
      found=true;
    }
  }
  assert(found, "no relocInfo found for pc");      
}


void relocInfo::remove_reloc_info_for_address(RelocIterator *itr, address pc, relocType old_type) {
  change_reloc_info_for_address(itr, pc, old_type, none);
}


// ----------------------------------------------------------------------------------------------------
// Implementation of RelocIterator


// First, the indexing structure.

void RelocIterator::initialize(intptr_t delta, CodeBlob* cb, address begin, address limit) {
#ifndef CORE  
  if (cb == NULL && delta == 0 && begin != NULL) {
    // allow CodeBlob to be deduced from beginning address
    cb = CodeCache::find_blob(begin);
  }
  assert(cb != NULL, "must be able to deduce nmethod from other arguments");

  _code    = cb;
  _current = cb->relocation_begin()-1;
  _end     = cb->relocation_end();
  _addr    = (address) cb->instructions_begin();

  assert(!has_current(), "just checking");  
  address code_end = cb->instructions_end();

  assert(begin == NULL || begin >= delta + cb->instructions_begin(), "in bounds");
 // FIX THIS  assert(limit == NULL || limit <= delta + code_end,                 "in bounds");
  if (begin != NULL)  begin -= delta;
  if (limit != NULL)  limit -= delta;
  set_limits(begin, limit);

  // after limits have been set and indexes consulted, then apply the delta
  if (delta == 0) {
    _is_copy = false;
  } else {
    _is_copy = true;
    _addr  += delta;		// we are operating on a copy of the code
    _limit += delta;
  }
#endif // !CORE
}


RelocIterator::RelocIterator(CodeBuffer* cb, address begin, address limit) {
#ifndef CORE
  _current = cb->locs_start()-1;
  _end     = cb->locs_end();
  _addr    = (address) cb->code_begin();
  _code    = cb->get_blob();

  assert(!has_current(), "just checking");

  assert(begin == NULL || begin >= cb->code_begin(), "in bounds");
  assert(limit == NULL || limit <= cb->code_end(),   "in bounds");
  set_limits(begin, limit);
#endif // !CORE
}


enum { indexCardSize = 128 };
struct RelocIndexEntry {
  jint addr_offset;          // offset from header_end of an addr()
  jint reloc_offset;         // offset from header_end of a relocInfo (prefix)
};


static inline int num_cards(int code_size) {
  return (code_size-1) / indexCardSize;
}


int RelocIterator::locs_and_index_size(int code_size, int locs_size) {
  if (!UseRelocIndex)  return locs_size;   // no index
  code_size = round_to(code_size, oopSize);
  locs_size = round_to(locs_size, oopSize);
  int index_size = num_cards(code_size) * sizeof(RelocIndexEntry);
  // format of indexed relocs:
  //   relocation_begin:   relocInfo ...
  //   index:              (addr,reloc#) ...
  //                       indexSize           :relocation_end
  return locs_size + index_size + BytesPerInt;
}


relocInfo* RelocIterator::create_index(CodeBuffer* src_cb, relocInfo* dest_begin, relocInfo* dest_end) {
  if (!UseRelocIndex)  return dest_begin;
  address relocation_begin = (address)dest_begin;
  address relocation_end   = (address)dest_end;
  int     total_size       = relocation_end - relocation_begin;
  int     locs_size        = src_cb->locs_size();
  int     index_size       = total_size - locs_size - BytesPerInt;	// find out how much space is left
  int     ncards           = index_size / sizeof(RelocIndexEntry);
  assert(total_size == locs_size + index_size + BytesPerInt, "checkin'");
  assert(index_size >= 0 && index_size % sizeof(RelocIndexEntry) == 0, "checkin'");
  jint*   index_size_addr  = (jint*)relocation_end - 1;

  assert(sizeof(jint) == BytesPerInt, "change this code");

  *index_size_addr = index_size;
  if (index_size != 0) {
    assert(index_size > 0, "checkin'");

    RelocIndexEntry* index = (RelocIndexEntry *)(relocation_begin + locs_size); 
    assert(index == (RelocIndexEntry*)index_size_addr - ncards, "checkin'");

    // walk over the relocations, and fill in index entries as we go
    RelocIterator iter;
    const address    initial_addr    = NULL;
    relocInfo* const src             = src_cb->locs_start();
    relocInfo* const initial_current = src - 1;  // biased by -1 like elsewhere

    iter._code    = NULL;
    iter._addr    = initial_addr;
    iter._limit   = (address)(ncards * indexCardSize);
    iter._current = initial_current;
    iter._end     = (relocInfo*)( (address)src + locs_size );
    iter._is_copy = true;

    int i = 0;
    address next_card_addr = (address)indexCardSize;
    int addr_offset = 0;
    int reloc_offset = 0;
    while (true) {
      // Checkpoint the iterator before advancing it.
      addr_offset  = iter._addr    - initial_addr;
      reloc_offset = iter._current - initial_current;
      if (!iter.next())  break;
      while (iter.addr() >= next_card_addr) {
	index[i].addr_offset  = addr_offset;
	index[i].reloc_offset = reloc_offset;
	i++;
	next_card_addr += indexCardSize;
      }
    }
    while (i < ncards) {
      index[i].addr_offset  = addr_offset;
      index[i].reloc_offset = reloc_offset;
      i++;
    }
  }
  return dest_begin;		// relocs go at the front, index at the back
}


void RelocIterator::set_limits(address begin, address limit) {
#ifndef CORE
  int index_size = 0;
  if (UseRelocIndex && _code != NULL) {
    index_size = ((jint*)_end)[-1];
    _end = (relocInfo*)( (address)_end - index_size - BytesPerInt );
  }

  _limit = limit;

  // the limit affects this next stuff:
  if (begin != NULL) {
#ifdef ASSERT
    // In ASSERT mode we do not actually use the index, but simply
    // check that its contents would have led us to the right answer.
    address addrCheck = _addr;
    relocInfo* infoCheck = _current;
#endif // ASSERT
    if (index_size > 0) {
      // skip ahead
      RelocIndexEntry* index       = (RelocIndexEntry*)_end;
      RelocIndexEntry* index_limit = (RelocIndexEntry*)((address)index + index_size);
      assert(_addr == _code->instructions_begin(), "_addr must be unadjusted");
      int card = (begin - _addr) / indexCardSize;
      if (card > 0) {
	if (index+card-1 < index_limit)  index += card-1;
	else                             index = index_limit - 1;
#ifdef ASSERT
        addrCheck = _addr    + index->addr_offset;
        infoCheck = _current + index->reloc_offset;
#else
        // Advance the iterator immediately to the last valid state
        // for the previous card.  Calling "next" will then advance
        // it to the first item on the required card.
        _addr    += index->addr_offset;
        _current += index->reloc_offset;
#endif // ASSERT
      }
    }

    relocInfo* backup;
    address    backup_addr;
    while (true) {
      backup      = _current;
      backup_addr = _addr;
#ifdef ASSERT
      if (backup == infoCheck) {
        assert(backup_addr == addrCheck, "must match"); addrCheck = NULL; infoCheck = NULL;
      } else {
        assert(addrCheck == NULL || backup_addr <= addrCheck, "must not pass addrCheck");
      }
#endif // ASSERT
      if (!next() || addr() >= begin) break;
    }
    assert(addrCheck == NULL || addrCheck == backup_addr, "must have matched addrCheck");
    assert(infoCheck == NULL || infoCheck == backup,      "must have matched infoCheck");
    // At this point, either we are at the first matching record,
    // or else there is no such record, and !has_current().
    // In either case, revert to the immediatly preceding state.
    _current = backup;
    _addr    = backup_addr;
    set_has_current(false);
  }
#endif // !CORE
}


void RelocIterator::set_limit(address limit) {
#ifndef CORE
  address code_end = (address)code() + code()->size();
  assert(limit == NULL || limit <= code_end, "in bounds");
  _limit = limit;
#endif // !CORE
}


void PatchingRelocIterator:: prepass() {
  // turn breakpoints off during patching
  _init_state = (*this);	// save cursor
  while (next()) {
    if (type() == relocInfo::breakpoint_type) {
      if (is_copy())
	breakpoint_reloc()->set_copy_active(false);
      else
	breakpoint_reloc()->set_active(false);
    }
  }
  (RelocIterator&)(*this) = _init_state;	// reset cursor for client
}


void PatchingRelocIterator:: postpass() {
  // turn breakpoints back on after patching
  if (is_copy())
    // ... unless we are working with a copy of the code:
    return;
  (RelocIterator&)(*this) = _init_state;	// reset cursor again
  while (next()) {
    if (type() == relocInfo::breakpoint_type) {
      breakpoint_Relocation* bpt = breakpoint_reloc();
      bpt->set_active(bpt->enabled());
    }
  }
}


// All the strange bit-encodings are in here.
// The idea is to encode relocation data which are small integers
// very efficiently (a single extra halfword).  Larger chunks of
// relocation data need a halfword header to hold their size.
void RelocIterator::advance_over_prefix() {
  assert(_current->type() == relocInfo::data_prefix_tag, "must be a prefix");
  if ((_current->_value & relocInfo::immediate_tag) == 0) {
    _data    = (short*) _current->data();
    _datalen =          _current->datalen();
    _current += _datalen + 1;   // skip the embedded data & header
  } else {
    _databuf = _current->immediate();
    _data = &_databuf;
    _datalen = 1;
    _current++;                 // skip the header
  }
  // The client will see the following relocInfo, whatever that is.
  // It is the reloc to which the preceding data applies.
}


Relocation* RelocIterator::reloc() {
  // (take the "switch" out-of-line)
  return make_reloc((relocInfo::relocType) type());
}



// Methods for BoundRelocation

// this updates the unpacked info for a new code buffer
address BoundRelocation::update_addrs(address old_addr, const CodeBuffer& new_cb, const CodeBuffer& old_cb) {

  // Point to the relocation info
  Relocation *r = reloc();

  // Transform the addr for the new code buffer
  address new_addr = old_cb.transform_address(new_cb, old_addr);

  // Set the transformed address
  set_addr( new_addr );

  // Update any internal addresses
  r->update_addrs(new_cb, old_cb);

  // Apply a delta to internal pointers
  r->fix_relocation_at_move(new_addr - old_addr);

  // Return the new address
  return (new_addr);
}


//////// Methods for flyweight Relocation types


RelocationHolder RelocationHolder::plus(int offset) const {
  if (offset != 0) {
    switch (type()) {
    case relocInfo::none:
      break;
    case relocInfo::oop_type:
      {
	oop_Relocation* r = (oop_Relocation*)reloc();
	return oop_Relocation::spec(r->oop_index(), r->offset() + offset);
      }
    default:
      ShouldNotReachHere();
    }
  }
  return (*this);
}


void Relocation::guarantee_size() {
  guarantee(false, "Make _relocbuf bigger!");
}

void Relocation::force_target(address target) {
  ShouldNotReachHere();
}

    // some relocations can compute their own values
address Relocation::value() {
  ShouldNotReachHere();
  return NULL;
}
  

void Relocation::set_value(address x) {
  ShouldNotReachHere();
}


RelocationHolder Relocation::spec_simple(relocInfo::relocType rtype) {
  if (rtype == relocInfo::none)  return RelocationHolder::none;

  BoundRelocation br;
  br.set_has_data(true);
  br.unpack_data(rtype);
  return br;
}


bool Relocation::is_copy() const {
#ifndef CORE
  CodeBlob* cb = code();
  address code_end = (address)cb + cb->size();
  return !(addr() >= cb->instructions_begin() && addr() <= code_end);
#else
  return false;
#endif // !CORE
}


static inline bool is_index(intptr_t index) {
  return 0 < index && index < os::vm_page_size();
}


intptr_t Relocation::runtime_address_to_index(address runtime_address) {
  assert(!is_index((intptr_t)runtime_address), "must not look like an index");

  if (runtime_address == NULL)  return 0;

  StubCodeDesc* p = StubCodeDesc::desc_for(runtime_address);
  if (p != NULL && p->begin() == runtime_address) {
    assert(is_index(p->index()), "there must not be too many stubs");
    return (intptr_t)p->index();
  } else {
    warning("random unregistered address in relocInfo: " INTPTR_FORMAT, runtime_address);
    return (intptr_t) runtime_address;
  }
}


address Relocation::index_to_runtime_address(intptr_t index) {
  if (index == 0)  return NULL;

  if (is_index(index)) {
    StubCodeDesc* p = StubCodeDesc::desc_for_index(index);
    assert(p != NULL, "there must be a stub for this index");
    return p->begin();
  } else {
    return (address) index;
  }
}


// this updates the unpacked info for a new code buffer
void Relocation::update_addrs(const CodeBuffer& new_cb, const CodeBuffer& old_cb) {
  // In the typical case, do nothing
}

void CallRelocation::set_destination(address x, intptr_t o) {
#ifndef CORE
  #ifdef ASSERT
  if (!is_copy() && type() == relocInfo::runtime_call_type) {
    // the runtime stubs are part of CodeCache, therefore the assertion is not valid
    // assert(!CodeCache::contains(x), "new destination must be external");
  }
  #endif
  pd_set_call_destination(x, o);
#endif // !CORE
}

void CallRelocation::fix_relocation_at_move(intptr_t delta) {
  // A self-relative reference to an external routine.
  // On some platforms, the reference is absolute, so value() isn't offset by delta.
  set_value(value(), delta);
}


//// pack/unpack methods

int oop_Relocation::pack_data() {
  return pack_2_ints(_oop_index, _offset);
}


void oop_Relocation::unpack_data() {
  unpack_2_ints(_oop_index, _offset);
}


int virtual_call_Relocation::pack_data() {

  // Try to make a pointer NULL first.  
  if (_oop_limit >= addr() && _oop_limit <= addr() + NativeCall::instruction_size) {
    _oop_limit = NULL;
  }
  // If the _oop_limit is NULL, it "defaults" to the end of the call.
  // See ic_call_Relocation::oop_limit() below.

  jint x0 = scaled_offset(_first_oop);
  jint x1 = scaled_offset(_oop_limit);
  return pack_2_ints(x0, x1);
}


void virtual_call_Relocation::unpack_data() {
  jint x0, x1; unpack_2_ints(x0, x1);
  _first_oop = address_from_scaled_offset(x0);
  _oop_limit = address_from_scaled_offset(x1);   // might be NULL
}


int static_stub_Relocation::pack_data() {
  return pack_1_int(scaled_offset(_static_call));
}

void static_stub_Relocation::unpack_data() {
  _static_call = address_from_scaled_offset(unpack_1_int());
}


int external_word_Relocation::pack_data() {
  return pack_1_int(runtime_address_to_index(_target));
}


void external_word_Relocation::unpack_data() {
  _target = index_to_runtime_address(unpack_1_int());
}


int internal_word_Relocation::pack_data() {
  // An instruction cannot point to itself, because we reserve "0"
  // to mean that the pointer itself is embedded in the code stream.
  assert(_target != addr(), "cannot point to self");
  return pack_1_int(scaled_offset(_target));
}


void internal_word_Relocation::unpack_data() {
  _target = address_from_scaled_offset(unpack_1_int());
}


int breakpoint_Relocation::pack_data() {
  short* p = data();

  assert(p == &live_bits(), "initializing live_bits");
  *p++ = _bits;

  jint target_bits = (jint) internal() ? scaled_offset           (_target)
				: runtime_address_to_index(_target);
  if (settable()) {
    // save space for set_target later
    add_long(p, target_bits);
  } else {
    add_int (p, target_bits);
  }

  for (int i = 0; i < instrlen(); i++) {
    add_short(p, (short)0x7777);    // placeholder value until bytes can be saved
  }

  return p - data();
}


void breakpoint_Relocation::unpack_data() {
  _bits = live_bits();

  int targetlen = datalen() - 1 - instrlen();
  jint target_bits = 0;
  if (targetlen == 0)       target_bits = 0;
  else if (targetlen == 1)  target_bits = binding()->short_data_at(1);
  else if (targetlen == 2)  target_bits = binding()-> long_data_at(1);
  else                      { ShouldNotReachHere(); }

  _target = internal() ? address_from_scaled_offset(target_bits)
		       : index_to_runtime_address  ((intptr_t)target_bits);
}


// this updates the unpacked info for a new code buffer
void breakpoint_Relocation::update_addrs(const CodeBuffer& new_cb, const CodeBuffer& old_cb) {
  _target = old_cb.transform_address(new_cb, _target);
}


//// miscellaneous methods
oop* oop_Relocation::oop_addr() {
#ifndef CORE
  int n = _oop_index;
  if (n == 0)
    return (oop*) pd_address_in_code();
  else {
      assert(code()->is_nmethod(), "must refer to an nmethod");
    return ((nmethod *)code())->oop_addr_at(n);
  }
#else
  return NULL;
#endif // !CORE
}


oop oop_Relocation::oop_value() {
  oop v = *oop_addr();
  if (v == (oop)Universe::non_oop_word())  v = NULL;
  return v;
}


void oop_Relocation::oops_do(void f(oop*)) {
  if (_oop_index == 0) {
    oop* addr = oop_addr();
    if (*addr != Universe::non_oop_word())
      f(addr);
  }
}


void oop_Relocation::fix_oop_relocation() {
  // re-extract the oop from the pool:
  if (_oop_index != 0)
    set_value(value());
}


RelocIterator virtual_call_Relocation::parse_ic(CodeBlob* &code, address &ic_call, address &first_oop, 
                                                oop* &oop_addr, bool *is_optimized) {
#ifndef CORE  
  assert(ic_call != NULL, "ic_call address must be set");
  assert(ic_call != NULL || first_oop != NULL, "must supply a non-null input");
  if (code == NULL) {
    if (ic_call != NULL) {
      code = CodeCache::find_blob(ic_call);
    } else if (first_oop != NULL) {
      code = CodeCache::find_blob(first_oop);
    }
    assert(code != NULL, "address to parse must be in CodeBlob");
  }
  assert(ic_call   == NULL || code->contains(ic_call),   "must be in CodeBlob");
  assert(first_oop == NULL || code->contains(first_oop), "must be in CodeBlob");
  
  address oop_limit = NULL;

  if (ic_call != NULL) {
    // search for the ic_call at the given address
    RelocIterator iter(code, ic_call, ic_call+1);
    bool ret = iter.next();
    assert(ret == true, "relocInfo must exist at this address");    
    assert(iter.addr() == ic_call, "must find ic_call");
    if (iter.type() == relocInfo::virtual_call_type) {      
      virtual_call_Relocation* r = iter.virtual_call_reloc();
      first_oop = r->first_oop();
      oop_limit = r->oop_limit();
      *is_optimized = false;
    } else {
      assert(iter.type() == relocInfo::opt_virtual_call_type, "must be a virtual call");
      *is_optimized = true;
      oop_addr = NULL;
      first_oop = NULL;
      return iter;
    }
  }   

  // search for the first_oop, to get its oop_addr
  RelocIterator all_oops(code, first_oop);
  RelocIterator iter = all_oops;
  iter.set_limit(first_oop+1);
  bool found_oop = false;
  while (iter.next()) {
    if (iter.type() == relocInfo::oop_type) {
      assert(iter.addr() == first_oop, "must find first_oop");
      oop_addr = iter.oop_reloc()->oop_addr();
      found_oop = true;
      break;
    }
  }
  assert(found_oop, "must find first_oop");

  bool did_reset = false;
  while (ic_call == NULL) {
    // search forward for the ic_call matching the given first_oop
    while (iter.next()) {
      if (iter.type() == relocInfo::virtual_call_type) {
	virtual_call_Relocation* r = iter.virtual_call_reloc();
	if (r->first_oop() == first_oop) {
	  ic_call   = r->addr();
	  oop_limit = r->oop_limit();
	  break;
	}
      }
    }
    guarantee(!did_reset, "cannot find ic_call");
    iter = RelocIterator(code);	// search the whole CodeBlob
    did_reset = true;
  }

  assert(oop_limit != NULL && first_oop != NULL && ic_call != NULL, "");
  all_oops.set_limit(oop_limit);
  return all_oops;
#else
  RelocIterator iter((CodeBuffer*) NULL);
  return iter;
#endif // !CORE
}


address virtual_call_Relocation::first_oop() {
  assert(_first_oop != NULL && _first_oop < addr(), "must precede ic_call");
  return _first_oop;
}


address virtual_call_Relocation::oop_limit() {
  if (_oop_limit == NULL)
    return addr() + NativeCall::instruction_size;
  else
    return _oop_limit;
}



void virtual_call_Relocation::clear_inline_cache() {
#ifndef CORE
  // No stubs for ICs
  // Clean IC
  ResourceMark rm;
  CompiledIC* icache = CompiledIC_at(this);  
  icache->set_to_clean();  
#endif // !CORE
}


void opt_virtual_call_Relocation::clear_inline_cache() {
#ifndef CORE
  // No stubs for ICs
  // Clean IC
  ResourceMark rm;
  CompiledIC* icache = CompiledIC_at(this);  
  icache->set_to_clean();  
#endif // !CORE
}


address opt_virtual_call_Relocation::static_stub() {
  // search for the static stub who points back to this static call
  address static_call_addr = addr();
  RelocIterator iter(code());
  while (iter.next()) {
    if (iter.type() == relocInfo::static_stub_type) {
      if (iter.static_stub_reloc()->static_call() == static_call_addr) {
	return iter.addr();
      }
    }
  }
  return NULL;
}


// this updates the unpacked info for a new code buffer
void virtual_call_Relocation::update_addrs(const CodeBuffer& new_cb, const CodeBuffer& old_cb) {
  _first_oop = old_cb.transform_address(new_cb, _first_oop);
  _oop_limit = old_cb.transform_address(new_cb, _oop_limit);
}

void static_call_Relocation::clear_inline_cache() {
#ifndef CORE
  // Safe call site info
  CompiledStaticCall* handler = compiledStaticCall_at(this);
  handler->set_to_clean();
#endif // !CORE
}


address static_call_Relocation::static_stub() {
  // search for the static stub who points back to this static call
  address static_call_addr = addr();
  RelocIterator iter(code());
  while (iter.next()) {
    if (iter.type() == relocInfo::static_stub_type) {
      if (iter.static_stub_reloc()->static_call() == static_call_addr) {
	return iter.addr();
      }
    }
  }
  return NULL;
}


void static_stub_Relocation::clear_inline_cache() {
#ifndef CORE
  // Call stub is only used when calling the interpreted code.
  // It does not really need to be cleared, except that we want to clean out the methodoop.
  CompiledStaticCall::set_stub_to_clean(this);
#endif // !CORE
}


void static_stub_Relocation::update_addrs(const CodeBuffer& new_cb, const CodeBuffer& old_cb) {
  _static_call = old_cb.transform_address(new_cb, _static_call);
}


void external_word_Relocation::fix_relocation_at_move(intptr_t delta) {
  // an absolute/relative reference to an external location
  if (_target == NULL) set_value(pd_get_address_from_code() - delta);
  else set_value(value()); // simply recalculate from addr()
}


address external_word_Relocation::target() {
  if (_target == NULL) return pd_get_address_from_code();
  else return _target;
}


// this updates the unpacked info for a new code buffer
void external_word_Relocation::update_addrs(const CodeBuffer& new_cb, const CodeBuffer& old_cb) {
  _target = old_cb.transform_address(new_cb, _target);
}


void internal_word_Relocation::fix_relocation_at_move(intptr_t delta) {
  // an absolute reference to an internal location
  if (_target == NULL) set_value(pd_get_address_from_code() + delta);
  else set_value(value()); // simply recalculate from addr()
}


address internal_word_Relocation::target() {
  if (_target == NULL) return pd_get_address_from_code();
  else return _target;
}


// this updates the unpacked info for a new code buffer
void internal_word_Relocation::update_addrs(const CodeBuffer& new_cb, const CodeBuffer& old_cb) {
  _target = old_cb.transform_address(new_cb, _target);
}


breakpoint_Relocation::breakpoint_Relocation(int kind, address target, bool internal) {
  bool active    = false;
  bool enabled   = (kind == initialization);
  bool removable = (kind != safepoint);
  bool settable  = (target == NULL);

  int bits = kind;
  if (enabled)    bits |= enabled_state;
  if (internal)   bits |= internal_attr;
  if (removable)  bits |= removable_attr;
  if (settable)   bits |= settable_attr;

  _bits = bits | high_bit;
  _target = target;

  assert(this->kind()      == kind,      "kind encoded");
  assert(this->enabled()   == enabled,   "enabled encoded");
  assert(this->active()    == active,    "active encoded");
  assert(this->internal()  == internal,  "internal encoded");
  assert(this->removable() == removable, "removable encoded");
  assert(this->settable()  == settable,  "settable encoded");
}


address breakpoint_Relocation::target() const {
  return _target;
}


void breakpoint_Relocation::set_target(address x) {
  assert(settable(), "must be settable");
  jint target_bits = (jint)internal() ? scaled_offset           (x)
				: runtime_address_to_index(x);
  short* p = &live_bits() + 1;
  add_long(p, target_bits);
  assert(p == instrs(), "new target must fit");
  _target = x;
}


void breakpoint_Relocation::set_enabled(bool b) {
  assert(!is_copy(), "cannot change breakpoint state when working on a copy");

  if (enabled() == b) return;

  if (b) {
    set_bits(bits() | enabled_state);
  } else {
    set_active(false);		// remove the actual breakpoint insn, if any
    set_bits(bits() & ~enabled_state);
  }
}


void breakpoint_Relocation::set_active(bool b) {
  assert(!is_copy(), "cannot change breakpoint state when working on a copy");

  assert(!b || enabled(), "cannot activate a disabled breakpoint");

  if (active() == b) return;

  // %%% should probably seize a lock here (might not be the right lock)
  //MutexLockerEx ml_patch(Patching_lock, true);
  //if (active() == b)  return;		// recheck state after locking

  if (b) {
    set_bits(bits() | active_state);
    if (instrlen() == 0)
      fatal("breakpoints in original code must be undoable");
    pd_swap_in_breakpoint (addr(), instrs(), instrlen());
  } else {
    set_bits(bits() & ~active_state);
    pd_swap_out_breakpoint(addr(), instrs(), instrlen());
  }
}


void breakpoint_Relocation::set_copy_active(bool b) {
  assert(is_copy(), "must be operating on a copy");

  if (b) {
    pd_swap_in_breakpoint (addr(), NULL, instrlen());
  } else {
    fatal("cannot remove a breakpoint from a code copy");
  }
}


//---------------------------------------------------------------------------------
// Non-product code

#ifndef PRODUCT

static const char* reloc_type_string(relocInfo::relocType t) {
  switch (t) {
  #define EACH_CASE(name) \
  case relocInfo::name##_type: \
    return #name;

  APPLY_TO_RELOCATIONS(EACH_CASE);
  #undef EACH_CASE

  case relocInfo::none:
    return "none";
  case relocInfo::data_prefix_tag:
    return "prefix";
  default:
    return "UNKNOWN RELOC TYPE";
  }
}


void RelocIterator::print_current() {
  if (!has_current()) {
    tty->print_cr("(no relocs)");
    return;
  }
  tty->print("relocInfo@" INTPTR_FORMAT " [type=%d(%s) addr=" INTPTR_FORMAT,
	     _current, type(), reloc_type_string((relocInfo::relocType) type()), _addr);
  if (current()->format() != 0)
    tty->print(" format=%d", current()->format());
  if (datalen() == 1) {
    tty->print(" data=%d", data()[0]);
  } else if (datalen() > 0) {
    tty->print(" data={");
    for (int i = 0; i < datalen(); i++) {
      tty->print("%04x", data()[i] & 0xFFFF);
    }
    tty->print("}");
  }
  tty->print("]");
  switch (type()) {
  case relocInfo::oop_type:
    {
      oop_Relocation* r = oop_reloc();
      oop* oop_addr  = NULL;
      oop  raw_oop   = NULL;
      oop  oop_value = NULL;
      if (code() != NULL || r->oop_index() == 0) {
	oop_addr  = r->oop_addr();
	raw_oop   = *oop_addr;
	oop_value = r->oop_value();
      }
      tty->print_cr(" | [oop_addr=" INTPTR_FORMAT " *=" INTPTR_FORMAT " offset=%d]",
		    oop_addr, raw_oop, r->offset());
      // Do not print the oop by default--we want this routine to
      // work even during GC or other inconvenient times.
      if (WizardMode && oop_value != NULL) {
	tty->print("oop_value=" INTPTR_FORMAT ": ", oop_value);
	oop_value->print_value_on(tty);
      }
      break;
    }
  case relocInfo::external_word_type:
  case relocInfo::internal_word_type:
    {
      DataRelocation* r = (DataRelocation*) reloc();
      tty->print(" | [target=" INTPTR_FORMAT "]", r->value()); //value==target
      break;
    }
  case relocInfo::static_call_type:
  case relocInfo::runtime_call_type:
    {
      CallRelocation* r = (CallRelocation*) reloc();
      tty->print(" | [destination=" INTPTR_FORMAT "]", r->destination());
      break;
    }
  case relocInfo::virtual_call_type:
    {
      virtual_call_Relocation* r = (virtual_call_Relocation*) reloc();
      tty->print(" | [destination=" INTPTR_FORMAT " first_oop=" INTPTR_FORMAT " oop_limit=" INTPTR_FORMAT "]",
		 r->destination(), r->first_oop(), r->oop_limit());
      break;
    }
  case relocInfo::static_stub_type:
    {
      static_stub_Relocation* r = (static_stub_Relocation*) reloc();
      tty->print(" | [static_call=" INTPTR_FORMAT "]", r->static_call());
      break;
    }
  }
  tty->cr();
}


void RelocIterator::print() {
  RelocIterator save_this = (*this);
  relocInfo* scan = _current;
  if (!has_current())  scan += 1;  // nothing to scan here!

  bool skip_next = has_current();
  bool got_next;
  while (true) {
    got_next = (skip_next || next());
    skip_next = false;

    tty->print("         @" INTPTR_FORMAT ": ", scan);
    relocInfo* newscan = _current+1;
    if (!has_current())  newscan -= 1;  // nothing to scan here!
    while (scan < newscan) {
      tty->print("%04x", *(short*)scan & 0xFFFF);
      scan++;
    }
    tty->cr();

    if (!got_next)  break;
    print_current();
  }

  (*this) = save_this;
}
#endif // !PRODUCT
