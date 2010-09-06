#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)codeBuffer.hpp	1.49 03/12/23 16:38:56 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

class  AbstractAssembler;
class  MacroAssembler;
class  PhaseCFG;
class  Compile;
class  BufferBlob;
extern MacroAssembler* theMacroAssm;

// This class is used to cache arguments passed to "relocate" until
// after code generation is complete, in order to maintain monotonically
// increasing addresses in relocation records.
class RelocateBuffer: public ResourceObj {
 private:
  address          _addr;	// "at"     passed to relocate
  RelocationHolder _spec;	// "spec"   passed to relocate
  int              _format;	// "format" passed to relocate

 public:
  enum {
    alloc_incr = 16
  };

  void init(address addr, RelocationHolder const& spec, int format) {
    _addr = addr; _spec = spec; _format = format;
  }

  address addr() const { return _addr; };

  RelocationHolder& spec() { return _spec; };

  int format() const { return _format; };
};


// A CodeBuffer describes a memory space into which assembly
// code is generated. A code buffer comes in two variants:
//
// (1) A CodeBuffer referring to an already allocated piece of memory:
//     This is used to direct 'static' code generation (e.g. for interpreter
//     or stubroutine generation, etc.).  This code comes with NO relocation
//     information.
//
// (2) A CodeBuffer referring to a piece of memory allocated when the
//     CodeBuffer is allocated.  This is used for nmethod generation.

class CodeBuffer: public ResourceObj {

 private:
  address     _instsStart;	// first byte of instructions
  address     _instsEnd;	// first byte after instructions
  address     _instsOverflow;	// first byte after instructions buffer
  address     _mark;            // user mark
  int         _exception_offset;// Offset to exception handler
  // Stubs share the instruction space, up to _instsOverflow
  address     _stubsStart;      // first byte of stubs
  address     _stubsEnd;        // first byte after stubs
  address     _stubsOverflow;   // first byte after instruction buffer

  // Stub generation requires the code position to be temporarily
  // shifted. These hold the original values until the stub generation
  // is complete
  address     _instsEnd_before_stubs; // Store _instsEnd here while doing stubs
  address     _instsOverflow_before_stubs; // store _instsOverflow while doing stubs

  // Stub relocation info is stored until code generation is finished.
  RelocateBuffer *_stubsReloc;	// Storage for stub's relocation information
  int         _stubs_reloc_count;	// Number of used      relocation entries in stubs
  int         _stubs_reloc_alloc;	// Number of allocated relocation entries in stubs

  // Constant Table information
  address     _constStart;		// start of constant table
  address     _constEnd;		// end of allocated constant table
  address     _constOverflow;		// upper limit of constant table
  
  relocInfo*  _locsOverflow;		// first byte after relocation information buf
  int         _last_reloc_offset;

  OopRecorder* _oop_recorder;

  address     _decode_begin;	// start address for decode
  address     decode_begin();

  // Used internally to store reloc info during stub generation
  // After code generation, call relocate_stubs()
  void        add_stub_reloc(address at, RelocationHolder const & rspec, int format);

  void        put_reloc(address at, relocInfo::relocType rt, int format = 0);

  relocInfo*  _locsStart;	// first byte of relocation information
  relocInfo*  _locsEnd;		// first byte after relocation information

  BufferBlob  *_blob;		// Buffer in CodeCache for generated code
  bool	      _auto_free_blob;	// Free BufferBlob when CodeBuffer is freed
  bool        _allow_resizing;  // Allow resizing of the CodeBuffer


 public:
  // (1) code buffer referring to pre-allocated memory
  CodeBuffer(address code_start, int code_size); 
  // (2) code buffer allocating memory for code & relocation info
  //     instsSize must include both code and stubs sizes.
  // 
  // "name" is used if the passed BufferBlob is NULL.
  CodeBuffer(int instsSize, int locsSize, int stubsSize, int constSize, 
             int locsStubSize, bool needs_oop_recorder, BufferBlob *blob = NULL, 
             relocInfo *locs_memory = NULL, RelocateBuffer *locs_stub_memory = NULL,
	     bool auto_free_blob = false,
             OopRecorder* oop_recorder = NULL,
             const char* name = NULL,
             bool allow_resizing = false,
	     bool soft_fail = false);

  ~CodeBuffer();

  static int insts_memory_size(int instsSize);
  static int locs_memory_size (int locsSize);
  static int locs_stub_memory_size(int locsSize);
  
  // Properties
  address code_begin() const          { return _instsStart; };
  address code_end() const            { return _instsEnd; }
  address code_limit() const          { return _instsOverflow; };
  bool in_code(address pc) const      { return code_begin() <= pc && pc <= code_end(); }
  bool contains(address pc) const 
  { return code_begin() <= pc && pc < code_end(); }

  void    set_mark()                  { _mark = _instsEnd; }
  address mark() const                { return _mark; }
  
  // size in bytes
  int code_size() const { return _instsEnd - _instsStart; } 

  // capacity in bytes
  int code_capacity() const { return _instsOverflow - _instsStart; } 

  // Exception code
  int exception_offset() const { return _exception_offset; }
  void set_exception_offset( int off ) { _exception_offset = off; }

  // Access methods for the stubs
  address    stub_begin() const		{ return _stubsStart; }
  address    stub_end() const		{ return _stubsEnd; }
  int        stub_size() const		{ return stub_end() - stub_begin(); }
  address    stub_limit() const		{ return _stubsOverflow; };
  int        stub_capacity() const	{ return stub_limit() - stub_begin(); }
  bool    in_stub(address pc) const	{ return stub_begin() <= pc && pc <= stub_end(); }

  void       set_stubs_begin(address a)	{ _stubsStart = a; }
  void       set_stubs_end  (address a)	{ _stubsEnd   = a; }

  // Allocate a new relocate record to hold stub relocation information
  RelocateBuffer*  alloc_relocate();

  // Inform codeBuffer when generating stub code and relocation info
  void       start_a_stub();
  void       end_a_stub();

  // Return the beginning of the code section
  // (code or stub, based on if stubs are being generated)
  address    code_section() const       { return (_instsEnd_before_stubs ? _stubsStart : _instsStart); }

  // Install relocation info stored during stub generation
  void       relocate_stubs();

  // Access methods for the constant table
  address    ctable_start() const	{ return _constStart;  }
  address    ctable_end() const		{ return _constEnd;    }
  int        ctable_size() const	{ return ctable_end() - ctable_start(); }
  address    ctable_limit() const	{ return _constOverflow;    }
  int        ctable_capacity() const	{ return ctable_limit() - ctable_start(); }
  bool    in_ctable(address pc) const	{ return ctable_start() <= pc && pc <= ctable_end(); }

  void       set_ctable_begin(address a) { _constStart = a; }
  void       set_ctable_end  (address a) { _constEnd   = a; }

  relocInfo* locs_start() const       { return _locsStart; }
  relocInfo* locs_end() const         { return _locsEnd; }
  int        locs_size() const        { return (char*)locs_end() - (char*)locs_start(); }
  int        locs_count() const       { return locs_end() - locs_start(); }
  relocInfo* locs_limit() const       { return _locsOverflow; }
  int        locs_capacity() const    { return (char*)locs_limit() - (char*)locs_start(); }

  void       set_locs_start(relocInfo *a) { _locsStart = a; }
  void       set_locs_end(relocInfo *a)   { _locsEnd = a; }

  OopRecorder* oop_recorder()         const { return _oop_recorder;     }
  void     set_oop_recorder(OopRecorder* r) {        _oop_recorder = r; }

  // Resize the code buffer in case of potential overflow
  void resize();

  // Resize the code buffer to explicit sizes
  void resize(int new_code_size, int new_stub_size, int new_ctable_size, int new_locs_size);

  // Verify that there is enough space remaining for one instruction
  void force_space_in_buffer(int inst_size, int stubs_size, int const_size, int locs_size) {
    if ( allow_resizing() &&
         ( ((address)  code_end() + inst_size  >= (address)  code_limit()) ||
           ((address)  stub_end() + stubs_size >= (address)  stub_limit()) ||
           ((address)ctable_end() + const_size >= (address)ctable_limit()) ||
           ((address)  locs_end() + locs_size  >= (address)  locs_limit())) )
      resize();
  }


  // return the total size of relocation information, plus indexes, in nmethod
  int reloc_size() const { return RelocIterator::locs_and_index_size(code_size(), locs_size()); }

  // Allocate or reallocate a new relocation buffer
  void alloc_relocation(uint relocation_size);
  void realloc_relocation(uint relocation_size);

  // Code generation
  inline void set_code_end(address end);    // adjusts the code end
  inline void set_code_end_after_constants(); // join constants

  // Allow the code end to be temporarily set to the end of the buffer
  inline void set_code_end_after_constants(address& code_end, address& code_overflow);
  inline void reset_code_end(address& code_end, address& code_overflow);

  void relocate(address at, RelocationHolder const& rspec, int format = 0);
  void relocate(address at,    relocInfo::relocType rtype, int format = 0) {
    if (rtype != relocInfo::none)
      relocate(at, Relocation::spec_simple(rtype), format);
  }
  void relocate_raw(address at, relocInfo::relocType rtype, const short* data = NULL, int datalen = 0, int format = 0);

  // NMethod generation
  void    copy_relocation(CodeBlob* blob);
  void    copy_code(CodeBlob* nm);

  // GC support
  void    oops_do(void f(oop*));

  // Routines for handling temporary CodeBlob buffer associated
  // with CodeBuffers.  This routines are used to free these
  // buffers that are not in the RESOURCE_ARRAY.
  bool auto_free_blob()              { return _auto_free_blob ; }
  BufferBlob* get_blob()             { return _blob; }

  // Indicate if resizing is allowed
  bool allow_resizing() const      { return _allow_resizing; }

  // fp constants support
  address insert_double_constant(jdouble c);
  address insert_float_constant (jfloat c);

  // Transform an address from the code in this code buffer to a specified code buffer
  address transform_address(const CodeBuffer &cb, address addr) const;

#ifndef PRODUCT
 public:
  // Printing / Decoding
  // decodes from decode_begin() to code_end() and sets decode_begin to end
  void    decode();		
  void    decode_all();		// decodes all the code
  void    skip_decode();	// sets decode_begin to code_end();
  void    print();
#endif


  // The following header contains architecture-specific implementations
  #include "incls/_codeBuffer_pd.hpp.incl"
};


inline void CodeBuffer::set_code_end( address end ) {
  // Note: since this is called after a write, end == overflow is ok.
  // (the next emit() will overwrite some memory after _instsOverflow,
  // but this one did not)
  assert( end >= _instsStart && end <= _instsOverflow, "CodeBuffer overflow");
  //  assert( end >= _instsEnd, "illegal set_code_end" );
  _instsEnd = end;
}


inline void CodeBuffer::set_code_end_after_constants() {
  _instsEnd = ctable_end();
  _instsOverflow = _constOverflow;
}


inline void CodeBuffer::set_code_end_after_constants(address& c_end, address& c_overflow) {
  c_end          = code_end();
  c_overflow     = code_limit();
  _instsEnd      = ctable_end();
  _instsOverflow = ctable_limit();
}


inline void CodeBuffer::reset_code_end(address& c_end, address& c_overflow) {
  _instsEnd      = c_end;
  _instsOverflow = c_overflow;
}
