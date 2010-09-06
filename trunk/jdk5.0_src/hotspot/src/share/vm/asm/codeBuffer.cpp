#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)codeBuffer.cpp	1.84 04/03/18 11:03:50 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_codeBuffer.cpp.incl"

// The structure of the CodeBuffer:
//
//       _instsStart ->   +----------------+
//                        |                |
//                        |                |
//                        |     Code       |
//                        |                |
//                        |                |
//                        +----------------+
//    _instsOverflow ->   |                |
//    _stubsStart         |      Stubs     |
//                        +----------------+
//    _stubsOverflow ->   |                |
//    _constStart         |   FP constants |
//                        +----------------+
//    _constOverflow ->   |                |

MacroAssembler* theMacroAssm = NULL;

int CodeBuffer::insts_memory_size(int instsSize) {
  instsSize = round_to( instsSize, oopSize);
  return instsSize + oopSize*2;
}

int CodeBuffer::locs_memory_size(int locsSize) {
  locsSize = round_to( locsSize,  oopSize);
  return locsSize * (sizeof(relocInfo) * 4);
}

int CodeBuffer::locs_stub_memory_size(int locsSize) {
  return locsSize * sizeof(RelocateBuffer);
}

// instsSize includes the stubsSize measurement
CodeBuffer::CodeBuffer(int instsSize, int locsSize, int stubsSize, 
                       int constSize, int locsStubSize,
                       bool needs_oop_recorder,
                       BufferBlob *blob,
                       relocInfo *locs_memory,
                       RelocateBuffer *locs_stub_memory,
                       bool auto_free_blob,
                       OopRecorder *oop_recorder,
                       const char* name,
                       bool allow_resizing,
		       bool soft_fail) {
  // Compute maximal alignment
  int alignSize  = MAX2((intx) sizeof(jdouble), CodeEntryAlignment);
  assert(is_power_of_2(alignSize), "");

  // Keep original instSize since Stubs are not oopSize aligned.
  int instsOrig  = instsSize;

  // Requirements are that the instruction area and the
  // stubs area must start on CodeEntryAlignment, and
  // the ctable on sizeof(jdouble)
  instsSize      = round_to( instsSize, CodeEntryAlignment );
  stubsSize      = round_to( instsSize + stubsSize, sizeof(jdouble) ) - instsSize;
  constSize      = round_to( constSize, sizeof(jdouble) );

  int totalSize  = instsSize + stubsSize + constSize;

  // +alignSize*2 so that we have enough space for double-word alignment of
  // insts and fp constants tables
  int instsSlop = alignSize*2;
  address insts;

  _auto_free_blob = auto_free_blob;
  _allow_resizing = allow_resizing;

  // Initialize the buffer for relocate records to empty
  _stubsReloc        = NULL;
  _stubs_reloc_count = 0;
  _stubs_reloc_alloc = 0;

  // Create a BufferBlob from the Code Cache if one was not provided
  // Warning:  This memory will not be release when the CodeBuffer
  // is destroyed unless you set auto_free_blob!
  if ( blob == NULL ) {
    if (name == NULL) name = "CodeBuffer constructor";
      BufferBlob* newblob = BufferBlob::create(name, totalSize + instsSlop);
      if( newblob == NULL ) {
	if (!soft_fail) {
	  fatal1( "CodeCache: no room for %s", name);
	}
	_blob = NULL;
	insts = NULL;
	_instsStart    = NULL;
	_instsStart    = NULL;
	_instsEnd      = NULL;
	_instsOverflow = NULL;
	_instsEnd_before_stubs      = NULL;
	_instsOverflow_before_stubs = NULL;
	_stubsStart    = NULL;
	_stubsEnd      = NULL;
	_stubsOverflow = NULL;
	_constStart    = NULL;
	_constEnd      = NULL;
	_constOverflow = NULL;
	_locsStart    = NULL;
	_locsEnd      = NULL;
	_locsOverflow = NULL;
	_stubsReloc        = NULL;
	_stubs_reloc_count = 0;
	_stubs_reloc_alloc = 0;
	return;
    } else {
      insts = newblob->instructions_begin();
      _blob = newblob;
    }
  }
  else {
    // [RGV] are there any fields in the blob needing re-initialization
    // if we reuse it?
    _blob = blob;
    insts = blob->instructions_begin();
  }

  // Align everything to the greater of CodeEntryAlignment and the size of
  // a double.
  _instsStart    = insts + alignSize;
  _instsStart    = _instsStart - (((intptr_t)_instsStart) & ((intptr_t)(CodeEntryAlignment-1)));
  _instsEnd      = _instsStart;
  _instsOverflow = _instsStart + instsSize;
  assert ((uintptr_t)_instsStart % CodeEntryAlignment == 0, "instruction start not code entry aligned");

  // mark stubs boundaries
  _instsEnd_before_stubs      = NULL;
  _instsOverflow_before_stubs = NULL;

  _stubsStart    = _instsOverflow;
  _stubsEnd      = _instsOverflow;
  _stubsOverflow = _stubsStart + stubsSize;
  assert ((uintptr_t)_stubsStart % CodeEntryAlignment == 0, "stubs not code entry aligned");

  _constStart    = _stubsOverflow;
  _constEnd      = _constStart;
  _constOverflow = _stubsOverflow + constSize;
  assert ((uintptr_t)_constStart % sizeof(jdouble) == 0, "constant table not double word aligned");

  if( locs_memory == NULL ) {

    // conservatively align relocation size to oopSize, for later fill/copy
    locsSize     = round_to( locsSize,  oopSize );

    alloc_relocation( locsSize );
  }
  else {
    _locsStart    = locs_memory;
    _locsEnd      = _locsStart;
    _locsOverflow = (relocInfo*) ((address) _locsStart + locsSize);
  }

  if( locs_stub_memory != NULL ) {
    _stubsReloc        = locs_stub_memory;
    _stubs_reloc_count = 0;
    _stubs_reloc_alloc = locsStubSize / sizeof(RelocateBuffer);
  }

  _last_reloc_offset = code_size();
  _decode_begin  = NULL;
  _oop_recorder  = needs_oop_recorder ? (oop_recorder == NULL ? new OopRecorder() : oop_recorder) : NULL;

  pd_initialize();
}


CodeBuffer::CodeBuffer(address code_start, int code_size) {
  _instsStart    = _instsEnd = code_start;
  _instsOverflow = _instsStart + code_size;
  _instsEnd_before_stubs      = NULL;
  _instsOverflow_before_stubs = NULL;
  _stubsStart    = _stubsEnd = NULL;
  _locsStart     = _locsEnd = NULL;
  _locsOverflow  = NULL;
  _stubsReloc    = NULL;
  _stubs_reloc_count = 0;
  _stubs_reloc_alloc = 0;
  _last_reloc_offset = CodeBuffer::code_size();
  _decode_begin  = NULL;
  _constStart = _constEnd = _constOverflow = NULL;
  _oop_recorder  = NULL;
  _auto_free_blob = false;
  _allow_resizing = false;
  _blob = NULL;

  pd_initialize();
}


CodeBuffer::~CodeBuffer() {
  // If we allocate our code buffer from the CodeCache
  // via a BufferBlob, and it's not permanent, then
  // free the BufferBlob.
  // The rest of the memory will be freed when the ResourceObj
  // is released.
  if ( _auto_free_blob && ( _blob != NULL ) ) BufferBlob::free( _blob );
}


address CodeBuffer::decode_begin() {
  return _decode_begin == NULL ? _instsStart : _decode_begin;
}


// private
// Allocate a new record for storing stub arguments to relocate until
// after all code for the method has been generate (to maintain monotonically
// increasing addresses for relocation records).
RelocateBuffer * CodeBuffer::alloc_relocate() {

  // Insufficient room, allocate more space
  if (_stubs_reloc_count >= _stubs_reloc_alloc) {
    int new_alloc = _stubs_reloc_count + RelocateBuffer::alloc_incr;

    _stubsReloc = _stubsReloc
      ? REALLOC_RESOURCE_ARRAY(RelocateBuffer, _stubsReloc, _stubs_reloc_alloc, new_alloc)
      : NEW_RESOURCE_ARRAY(RelocateBuffer, new_alloc);

    _stubs_reloc_alloc = new_alloc;
  }

  // Point to the next element
  return &_stubsReloc[_stubs_reloc_count++];
}

// Store out-of-line stub relocation info until finished with code generation.
void CodeBuffer::add_stub_reloc(address at, RelocationHolder const &rspec, int format) {
  // Allocate a buffer entry for the arguments to "relocate"
  RelocateBuffer* reloc = alloc_relocate();

  // Copy the arguments to "relocate" for later
  reloc->init(at, rspec, format);
}

// Call CodeBuffer::relocate with the stub relocations,
// when finished emiting code.
void CodeBuffer::relocate_stubs() {
  assert( _stubsStart != NULL, "Do not have any stubs to relocate");

  // No "relocate" arguments cached, just quit
  if ( _stubs_reloc_alloc == 0 )
    return;

  // Permit references within the stub region
  address temp = _instsEnd;
  _instsEnd    = _stubsEnd;
  
  // Get stored "relocate" arguments and add to relocation info.
  for( int count = 0; count < _stubs_reloc_count; ++count ) {
    RelocateBuffer *reloc = &_stubsReloc[count];
    relocate( reloc->addr(), reloc->spec(), reloc->format() );
  }

  // Deallocate the arguments
  FREE_RESOURCE_ARRAY(RelocateBuffer, _stubsReloc, _stubs_reloc_alloc);
  _stubsReloc        = NULL;
  _stubs_reloc_count = 0;
  _stubs_reloc_alloc = 0;

  // reset the end of the code
  _instsEnd = temp;
}

// public inline
// Inform CodeBuffer that incoming code and relocation will be for stubs
void CodeBuffer::start_a_stub() {
  assert( _stubsStart != NULL, "Do not have any stubs to relocate");
  
  _instsEnd_before_stubs	= _instsEnd;
  _instsOverflow_before_stubs	= _instsOverflow;
  _instsEnd			= _stubsEnd;
  _instsOverflow  		= _stubsOverflow;
}

// public inline
// Inform CodeBuffer that incoming code and relocation will be code
void CodeBuffer::end_a_stub() {
  assert( _stubsStart != NULL, "Do not have any stubs to relocate");
  
  _stubsEnd			= _instsEnd;
  _stubsOverflow		= _instsOverflow;
  _instsEnd			= _instsEnd_before_stubs;
  _instsOverflow		= _instsOverflow_before_stubs;
  _instsEnd_before_stubs	= NULL;
  _instsOverflow_before_stubs	= NULL;
}

void CodeBuffer::relocate(address at, RelocationHolder const& spec, int format) {
  relocInfo::relocType rtype = (relocInfo::relocType) spec.reloc()->type();
  if (rtype == relocInfo::none)  return;
  if (rtype == relocInfo::oop_type && at == NULL)  return;    // no reloc necessary

  // The assertion below has been adjusted, to also work for
  // relocation for fixup.  Sometimes we want to put relocation
  // information for the next instruction, since it will be patched
  // with a call.
  assert( code_begin() <= at && at <= code_end()+1, 
          "cannot relocate data outside code boundaries");

  // Store stub relocation info until finished with code generation.
  if( _instsEnd_before_stubs /* doing stubs */ ) {
    add_stub_reloc( at, spec, format );
    return;
  }

  if (_locsEnd == NULL) {
    // no space for relocation information provided => code cannot be
    // relocated.  Make sure that relocate is only called with rtypes
    // that can be ignored for this kind of code.
    assert(rtype == relocInfo::none              ||
           rtype == relocInfo::runtime_call_type ||
           rtype == relocInfo::internal_word_type||
           rtype == relocInfo::external_word_type, 
           "code needs relocation information");
    return;
  }

  assert( sizeof(relocInfo) == sizeof(short), "change this code");

  int len = at - _instsStart;
  int offset = len - _last_reloc_offset;
  assert(offset >= 0, "relocation addrs may not decrease");
  _last_reloc_offset = len;

  while( offset >= relocInfo::offset_limit() ) {
    // Check for overflow
    if( _allow_resizing && _locsEnd + 4 >= _locsOverflow )
      realloc_relocation(locs_size()*2 + 4);

    *_locsEnd++ = filler_relocInfo();
    offset -= filler_relocInfo().addr_offset();
  }

  // Check again for overflow
  if( _allow_resizing && _locsEnd + 4 >= _locsOverflow )
    realloc_relocation(locs_size()*2 + 4);

  // write the prefix data, if any
  _locsEnd = spec.pack_data(_locsEnd, at);

  assert( offset < relocInfo::offset_limit(), "Check offset" );
  *_locsEnd++ = relocInfo( rtype, offset );
  // Note: since this is called after a write, end == overflow is ok.
  // (the next relocate() will overwrite some memory after _locsOverflow,
  // but this one did not)
  assert( _locsEnd <= _locsOverflow, "Estimate of code size or relocation entries was too low");

  if (relocInfo::have_format) {
    if (format != 0)
      (_locsEnd-1)->set_format(format);
  } else {
    assert(format == 0, "bad format");
  }
}


void CodeBuffer::relocate_raw(address at, relocInfo::relocType rtype, const short* data, int datalen, int format) {
  // a debugging routine, at present
  assert(_locsEnd != NULL, "must be collecting relocation info");

  // put in a fake relocation, to shake out various corner cases
  relocate(at, runtime_call_Relocation::spec());

  _locsEnd -= 1;
  int offset = _locsEnd->addr_offset();
  *_locsEnd = prefix_relocInfo();
  short* dp = (short*)_locsEnd->data();
  for (int i = 0; i < datalen; i++)
    *dp++ = data[i];
  _locsEnd = _locsEnd->finish_prefix(dp);
  *_locsEnd++ = relocInfo(rtype, offset, format);

  assert( _locsEnd < _locsOverflow, "routine is too long to compile");
}

void CodeBuffer::copy_code(CodeBlob* cb) { 
#ifndef CORE
  // %%% make sure the new location is sufficiently aligned
  const u_char fill_code = Assembler::code_fill_byte();

#ifdef ASSERT
  if ( (_stubsEnd - _stubsStart) != 0 ) {
    // Post-method stubs are considered part of the code
    assert (_instsEnd >= _stubsEnd, "_instsEnd not set correctly");
  }

  if ( (_constEnd - _constStart) != 0 ) {
    assert (_instsEnd >= _constEnd, "_instsEnd not set correctly");
  }
#endif

  // align code
  // Copy all instructions, code and stubs
  while (code_size() % HeapWordSize != 0) *_instsEnd++ = fill_code;
  Copy::disjoint_words((HeapWord*)_instsStart,
		       (HeapWord*)cb->instructions_begin(),
		       code_size() / HeapWordSize);

  int alignSize = MAX2((intx) sizeof(jdouble), CodeEntryAlignment);
  assert( (cb->instructions_begin() - _instsStart) % alignSize == 0, "copy must preserve alignment");

  // Fix the pc relative information after the move
  intptr_t delta = (address)cb->instructions_begin() - _instsStart;
  cb->fix_relocation_at_move(delta);

  // Flush generated code
  ICache::invalidate_range(cb->instructions_begin(), code_size());

#ifndef PRODUCT
  if (PrintAssembly) {
    Disassembler::decode(cb);
  }
#endif
#endif
}

void CodeBuffer::copy_relocation(CodeBlob* blob) {
#ifndef CORE
 // align relocation info
  while (locs_size() % HeapWordSize != 0) *_locsEnd++  = filler_relocInfo();
  relocInfo* locs = RelocIterator::create_index(this, blob->relocation_begin(), blob->relocation_end());
  assert(locs >= blob->relocation_begin(), "relocs in bounds");

  assert((address)locs + locs_size() <= (address)blob->relocation_end(), "relocs in bounds");
  Copy::disjoint_words((HeapWord*)_locsStart, (HeapWord*)locs, locs_size() / HeapWordSize);
#endif
}

void CodeBuffer::oops_do( void f(oop*) ) {
#ifndef CORE
  RelocIterator iter(this);
  while (iter.next()) {
    if (iter.type() == relocInfo::oop_type) {
      iter.reloc()->oops_do(f);
    }
  }
#endif
}


// fp constants support

// enters the constant c into the constant table and
// returns the address of the entry
address CodeBuffer::insert_double_constant(jdouble c) {
  assert (_constEnd != NULL && _constStart != NULL && _constOverflow != NULL, "fp constant table not initialized");

  // Align _constEnd to a multiple of sizeof(jdouble)
  // The following is derived from:  if _ctableEnd % sizeof(jdouble) is nonzero,
  //   then _ctableEnd += sizeof(jdouble) - ((uintptr_t)_constEnd % sizeof(jdouble))
  // Only the following is much simpler, and has no control flow
  _constEnd = _constEnd + ((-((intptr_t)_constEnd)) & (sizeof(jdouble)-1));

  address double_address = _constEnd;
  assert ((intptr_t)double_address % sizeof(jdouble) ==0, "double constant not aligned");
  *((jdouble*)_constEnd) = c;
  _constEnd += sizeof(jdouble);
  assert (_constStart <= _constEnd && _constEnd <= _constOverflow, "fp constant table overflow");
  return double_address;
}


address CodeBuffer::insert_float_constant(jfloat c) {
  assert (_constEnd != NULL && _constStart != NULL && _constOverflow != NULL, "fp constant table not initialized");

  address float_address = _constEnd;

  assert ((intptr_t)float_address % sizeof(jfloat) == 0, "float constant not aligned");
  *((jfloat*)_constEnd) = c;
  _constEnd += sizeof(jfloat);
  assert (_constStart <= _constEnd && _constEnd <= _constOverflow, "fp constant table overflow");
  return float_address;
}

address CodeBuffer::transform_address(const CodeBuffer &cb, address addr) const {
  if( _instsEnd_before_stubs /* Doing Stubs */ ) {
    if (_instsStart <= addr && addr <= _instsEnd_before_stubs)
       return (addr + ((intptr_t)cb.code_begin() - (intptr_t)code_begin()));
    if (_stubsStart <= addr && addr <= _instsEnd)
       return (addr + ((intptr_t)cb.stub_begin() - (intptr_t)stub_begin()));
  }
  else {
    if (in_code(addr))
      return (addr + ((intptr_t)cb.code_begin() - (intptr_t)code_begin()));
    if (in_stub(addr))
      return (addr + ((intptr_t)cb.stub_begin() - (intptr_t)stub_begin()));
  }

  if (in_ctable(addr))
    return (addr + ((intptr_t)cb.ctable_start() - (intptr_t)ctable_start()));

  return addr;
}

#ifndef CORE
void CodeBuffer::resize() {

  // When computing the new sizes, compensate for when the code is in the
  // middle of stub generation
  int old_code_size, old_code_capacity;
  int old_stub_size, old_stub_capacity;

  if( _instsEnd_before_stubs /* Doing Stubs */ ) {
    old_code_size     = _instsEnd_before_stubs      - _instsStart;
    old_code_capacity = _instsOverflow_before_stubs - _instsStart;
    old_stub_size     = _instsEnd      - _stubsStart;
    old_stub_capacity = _instsOverflow - _stubsStart;
  }
  else {
    old_code_size     = _instsEnd      - _instsStart;
    old_code_capacity = _instsOverflow - _instsStart;
    old_stub_size     = _stubsEnd      - _stubsStart;
    old_stub_capacity = _stubsOverflow - _stubsStart;
  }

  int new_code_size   = MAX2( old_code_size * 2, old_code_capacity );
  int new_stub_size   = MAX2( old_stub_size * 2, old_stub_capacity );
  int new_ctable_size = MAX2( ctable_size() * 2, ctable_capacity() );
  int new_locs_size   = MAX2( locs_size()   * 2, locs_capacity() );

#ifndef PRODUCT
#ifdef COMPILER2
  if (PrintOpto && Verbose) {
    tty->print_cr("    ... resize   code: %d(%d)->%d, stub: %d(%d)->%d, ctable: %d(%d)->%d, locs: %d(%d)->%d",
      old_code_capacity, old_code_size, new_code_size,
      old_stub_capacity, old_stub_size, new_stub_size,
      ctable_capacity(), ctable_size(), new_ctable_size,
      locs_capacity(),   locs_size(),   new_locs_size);
  }
#endif // COMPILER2
#endif // PRODUCT

  resize( new_code_size, new_stub_size, new_ctable_size, new_locs_size );
}

void CodeBuffer::resize(int new_code_size, int new_stub_size, int new_ctable_size, int new_locs_size) {

  // Resizing must be allowed
  assert( _allow_resizing, "resizing must be allowed" );

  // See if we are in the middle of stub generation
  bool in_stub_generation = false;

  if( _instsEnd_before_stubs /* Doing Stubs */ ) {
    end_a_stub();
    in_stub_generation = true;
  }

  // Create a new (temporary) code buffer to hold all the new data
  CodeBuffer cb(new_code_size, new_locs_size, new_stub_size, new_ctable_size, 0,
                oop_recorder() != NULL, NULL, NULL, NULL, true, oop_recorder(), _blob->name(), true);

  // Get the Cpu-Specific Data
  cb.getCpuData(this);

  // Copy the non-relocation info
  memmove(cb.  code_begin(),   code_begin(),   code_size());
  memmove(cb.  stub_begin(),   stub_begin(),   stub_size());
  memmove(cb.ctable_start(), ctable_start(), ctable_size());

  // Set the endpoints
  cb.set_code_end  (cb.  code_begin() +   code_size());
  cb.set_stubs_end (cb.  stub_begin() +   stub_size());
  cb.set_ctable_end(cb.ctable_start() + ctable_size());

  // Update internal pointer
  cb._mark             = transform_address(cb, _mark);

  // Copy everything else
  cb._exception_offset = _exception_offset;

  // Build an iterator to walk over the relocation records
  RelocIterator iter(this);

  // Disable resizing during this pass
  cb._allow_resizing = false;

  // Iterate over the relocation records
  while (iter.next()) {
    BoundRelocation br = (BoundRelocation)iter;
    br.unpack_data(iter.type());

    // Update the addresses to refer to the new code buffer
    address new_addr = br.update_addrs(br.addr(), cb, *this);

    // Temporarily reset the range to allow writes to stubs
    address cb_code_end = cb._instsEnd;
    cb._instsEnd = cb.ctable_end();

    // Generate the relocation record
    cb.relocate(new_addr, br, iter.format());

    // Reset the range
    cb._instsEnd = cb_code_end;
  }

  // Copy the stub relocation information
  cb._stubs_reloc_count = _stubs_reloc_count;
  cb._stubs_reloc_alloc = _stubs_reloc_alloc;

  if (cb._stubs_reloc_alloc > 0) {
    cb._stubsReloc = NEW_RESOURCE_ARRAY(RelocateBuffer, cb._stubs_reloc_alloc);

    // copy the relocation information. Note that this assumes that none
    // of the relocation records are internal
    for (int i = 0; i < _stubs_reloc_count; i++) {
      RelocateBuffer *old = &_stubsReloc[i];

      // Point to the relocation info
      Relocation *r = old->spec().reloc();

      // Transform the addr for the new code buffer
      address new_addr = transform_address(cb, old->addr());

      // Update any internal addresses
      r->update_addrs(cb, *this);

      // Apply a delta to internal pointers
      r->fix_relocation_at_move(new_addr - old->addr());

      // Copy the initialization record
      cb._stubsReloc[i].init( new_addr, old->spec(), old->format() );
    }
  }

  // If we are in the middle of stub generation, then mark it as so
  if( in_stub_generation )
    cb.start_a_stub();

  // Free the old blob
  BufferBlob::free( _blob );

  // Reset resizing
  cb._allow_resizing = _allow_resizing;

  // Copy the temporary code buffer into the current code buffer
  *this = cb;

  // Clear the code blob in the temporary code buffer to prevent it's deletion
  cb._blob = NULL;
}
#endif

void CodeBuffer::alloc_relocation(uint relocation_size) {

  assert( sizeof(relocInfo) == sizeof(short), "change this code");
  uint new_count = (relocation_size + 1) >> 1;
  if( new_count < 4 ) new_count = 4;

  // Allocate the space for the relocation
  _locsStart    = NEW_RESOURCE_ARRAY(relocInfo, new_count);
  _locsEnd      = _locsStart;
  _locsOverflow = _locsStart + new_count;

  // Allow new relocation information to be generated
  _last_reloc_offset = 0;
}

void CodeBuffer::realloc_relocation(uint relocation_size) {

  // Asserts that the relocInfo entries 
  const int log2_size = 1;
  assert( sizeof(relocInfo) == (1 << log2_size), "change this code");

  uint old_size  = locs_size();
  uint old_count = ((uint)locs_capacity()) >> log2_size;
  uint new_count = ((uint)(relocation_size + (sizeof(relocInfo) - 1))) >> log2_size;

  // Allocate the space for the relocation
  relocInfo* old_locs = _locsStart;
  relocInfo* new_locs = REALLOC_RESOURCE_ARRAY(relocInfo, old_locs, old_count, new_count);

  // Copy the information if the buffer changed
  _locsStart    = new_locs;
  _locsEnd      = (relocInfo*)((address)new_locs + old_size);
  _locsOverflow = new_locs + (new_count - 1);
}

#ifndef PRODUCT

void CodeBuffer::decode() {
  Disassembler::decode(decode_begin(), code_end());
  _decode_begin = code_end();
}


void CodeBuffer::skip_decode() {
  _decode_begin = code_end();
}


void CodeBuffer::decode_all() {
  Disassembler::decode(code_begin(), code_end());
}


void CodeBuffer::print() {
  if (this == NULL) {
    tty->print_cr("NULL CodeBuffer pointer");
    return;
  }

  tty->print_cr("CodeBuffer:");
  tty->print_cr("  Code  = " PTR_FORMAT " : " PTR_FORMAT " : " PTR_FORMAT " (%d of %d)",
                _instsStart, _instsEnd, _instsOverflow, _instsEnd-_instsStart, _instsOverflow-_instsEnd );
  tty->print_cr("  Stubs = " PTR_FORMAT " : " PTR_FORMAT " : " PTR_FORMAT " (%d of %d)",
                _stubsStart, _stubsEnd, _stubsOverflow, _stubsEnd-_stubsStart, _stubsOverflow-_stubsEnd );
  tty->print_cr("  Reloc = " PTR_FORMAT " : " PTR_FORMAT " : " PTR_FORMAT " (%d of %d)",
                _locsStart, _locsEnd, _locsOverflow, _locsEnd-_locsStart, _locsOverflow-_locsEnd );
  tty->print_cr("  Const = " PTR_FORMAT " : " PTR_FORMAT " : " PTR_FORMAT " (%d of %d)",
                _constStart, _constEnd, _constOverflow, _constEnd-_constStart, _constOverflow-_constEnd );
}

#endif // PRODUCT
