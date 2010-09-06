#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)relocInfo.hpp	1.69 03/12/23 16:39:57 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Types in this file:
//    relocInfo
//      One element of an array of halfwords encoding compressed relocations.
//      Also, the source of relocation types (relocInfo::oop_type, ...).
//    Relocation
//      A flyweight object representing a single relocation.
//      It is fully unpacked from the compressed relocation array.
//    oop_Relocation, ... (subclasses of Relocation)
//      The location of some type-specific operations (oop_addr, ...).
//      Also, the source of relocation specs (oop_Relocation::spec, ...).
//    RelocationHolder
//      A ValueObj type which acts as a union holding a Relocation object.
//      Represents a relocation spec passed into a CodeBuffer during assembly.
//    RelocIterator
//      A StackObj which iterates over the relocations associated with
//      a range of code addresses.  Can be used to operate a copy of code.
//    PatchingRelocIterator
//      Specialized subtype of RelocIterator which removes breakpoints
//      temporarily during iteration, then restores them.
//    BoundRelocation
//      An internal type shared by readers and writers of relocations.
//      It is a sort of "curry", a Relocation with an associated address
//      and pointer into a relocInfo stream.  It is used either to pack
//      data into a new stream (in a CodeBuffer) or unpack data from
//      an existing stream within a RelocIterator.


// Notes on relocType:
//
// These hold enough information to read or write a value embedded in
// the instructions of an CodeBlob.  They're used to update:
//
//   1) embedded oops     (isOop()          == true)
//   2) inline caches     (isIC()           == true)
//   3) runtime calls     (isRuntimeCall()  == true)
//   4) internal word ref (isInternalWord() == true)
//   5) external word ref (isExternalWord() == true)
//
// when objects move (GC) or if code moves (compacting the code heap).
// They are also used to patch the code (if a call site must change)
//
// A relocInfo is represented in 16 bits:
//   4 bits indicating the relocation type
//  12 bits indicating the offset from the previous relocInfo address
//
// The offsets accumulate along the relocInfo stream to encode the
// address within the CodeBlob, which is named RelocIterator::addr().
// The address of a particular relocInfo always points to the first
// byte of the relevant instruction (and not to any of its subfields
// or embedded immediate constants).
//
// The offset value is scaled appropriately for the target machine.
// (See relocInfo_<arch>.hpp for the offset scaling.)
//
// On some machines, there may also be a "format" field which may provide
// additional information about the format of the instruction stream
// at the corresponding code address.  The format value is usually zero.
// Any machine (such as Intel) whose instructions can sometimes contain
// more than one relocatable constant needs format codes to distinguish
// which operand goes with a given relocation.
//
// If the target machine needs N format bits, the offset has 12-N bits,
// the format is encoded between the offset and the type, and the
// relocInfo_<arch>.hpp file has manifest constants for the format codes.
//
// If the type is "data_prefix_tag" then the offset bits are further encoded,
// and in fact represent not a code-stream offset but some inline data.
// The data takes the form of a counted sequence of halfwords, which
// precedes the actual relocation record.  (Clients never see it directly.)
// The interpetation of this extra data depends on the relocation type.
//
// On machines that have 32-bit immediate fields, there is usually
// little need for relocation "prefix" data, because the instruction stream
// is a perfectly reasonable place to store the value.  On machines in
// which 32-bit values must be "split" across instructions, the relocation
// data is the "true" specification of the value, which is then applied
// to some field of the instruction (22 or 13 bits, on SPARC).
//
// Whenever the location of the CodeBlob changes, any PC-relative
// relocations, and any internal_word_type relocations, must be reapplied.
// After the GC runs, oop_type relocations must be reapplied.
//
//
// Here are meanings of the types:
//
// relocInfo::none -- a filler record
//   Value:  none
//   Instruction: The corresponding code address is ignored
//   Data:  Any data prefix and format code are ignored
//   (This means that any relocInfo can be disabled by setting
//   its type to none.  See relocInfo::remove.)
//
// relocInfo::oop_type -- a reference to an oop
//   Value:  an oop, or else the address (handle) of an oop
//   Instruction types: memory (load), set (load address)
//   Data:  []       an oop stored in 4 bytes of instruction
//          [n]      n is the index of an oop in the CodeBlob's oop pool
//          [[N]n l] and l is a byte offset to be applied to the oop
//          [Nn Ll]  both index and offset may be 32 bits if necessary
//   Here is a special hack, used only by the old compiler:
//          [[N]n 00] the value is the __address__ of the nth oop in the pool
//   (Note that the offset allows optimal references to class variables.)
//
// relocInfo::internal_word_type -- an address within the same CodeBlob
//   Value:  an address in the CodeBlob's code or constants section
//   Instruction types: memory (load), set (load address)
//   Data:  []     stored in 4 bytes of instruction
//          [[L]l] a relative offset (see [About Offsets] below)
//
// relocInfo::external_word_type -- a fixed address in the runtime system
//   Value:  an address
//   Instruction types: memory (load), set (load address)
//   Data:  []   stored in 4 bytes of instruction
//          [n]  the index of a "well-known" stub (usual case on RISC)
//          [Ll] a 32-bit address
//
// relocInfo::runtime_call_type -- a fixed subroutine in the runtime system
//   Value:  an address
//   Instruction types: PC-relative call (or a PC-relative branch)
//   Data:  []   stored in 4 bytes of instruction
//
// relocInfo::static_call_type -- a static call
//   Value:  an CodeBlob, a stub, or a fixup routine
//   Instruction types: a call
//   Data:  []
//   The identity of the callee is extracted from debugging information.
//   //%note reloc_3
//
// relocInfo::virtual_call_type -- a virtual call site (which includes an inline 
//                                 cache)
//   Value:  an CodeBlob, a stub, the interpreter, or a fixup routine
//   Instruction types: a call, plus some associated set-oop instructions
//   Data:  []       the associated set-oops are adjacent to the call
//          [n]      n is a relative offset to the first set-oop
//          [[N]n l] and l is a limit within which the set-oops occur
//          [Nn Ll]  both n and l may be 32 bits if necessary
//   The identity of the callee is extracted from debugging information.
//
// relocInfo::opt_virtual_call_type -- a virtual call site that is statically bound
//
//    Same info as a static_call_type. We use a special type, so the handling of
//    virtuals and statics are separated.
//
//
//   The offset n points to the first set-oop.  (See [About Offsets] below.)
//   In turn, the set-oop instruction specifies or contains an oop cell devoted
//   exclusively to the IC call, which can be patched along with the call.
//
//   The locations of any other set-oops are found by searching the relocation
//   information starting at the first set-oop, and continuing until all
//   relocations up through l have been inspected.  The value l is another
//   relative offset.  (Both n and l are relative to the call's first byte.)
//
//   The limit l of the search is exclusive.  However, if it points within
//   the call (e.g., offset zero), it is adjusted to point after the call and
//   any associated machine-specific delay slot.
//
//   Since the offsets could be as wide as 32-bits, these conventions
//   put no restrictions whatever upon code reorganization.
//
//   The compiler is responsible for ensuring that transition from a clean
//   state to a monomorphic compiled state is MP-safe.  This implies that
//   the system must respond well to intermediate states where a random
//   subset of the set-oops has been correctly from the clean state
//   upon entry to the VEP of the compiled method.  In the case of a
//   machine (Intel) with a single set-oop instruction, the 32-bit
//   immediate field must not straddle a unit of memory coherence.
//   //%note reloc_3
//
// relocInfo::breakpoint_type -- a conditional breakpoint in the code
//   Value:  none
//   Instruction types: any whatsoever
//   Data:  [b [T]t  i...]
//   The b is a bit-packed word representing the breakpoint's attributes.
//   The t is a target address which the breakpoint calls (when it is enabled).
//   The i... is a place to store one or two instruction words overwritten
//   by a trap, so that the breakpoint may be subsequently removed.
//
// relocInfo::static_stub_type -- an extra stub for each static_call_type
//   Value:  none
//   Instruction types: a virtual call:  { set_oop; jump; }
//   Data:  [[N]n]  the offset of the associated static_call reloc
//   This stub becomes the target of a static call which must be upgraded
//   to a virtual call (because the callee is interpreted).
//   See [About Offsets] below.
//   //%note reloc_2
//
// For example:
//
//   INSTRUCTIONS                        RELOC: TYPE    PREFIX DATA
//   ------------                               ----    -----------
// sethi      %hi(myObject),  R               oop_type [n(myObject)]
// ld      [R+%lo(myObject)+fldOffset], R2    oop_type [n(myObject) fldOffset]
// add R2, 1, R2
// st  R2, [R+%lo(myObject)+fldOffset]        oop_type [n(myObject) fldOffset]
//%note reloc_1
//
// This uses 4 instruction words, 8 relocation halfwords,
// and an entry (which is sharable) in the CodeBlob's oop pool,
// for a total of 36 bytes.
// 
// Note that the compiler is responsible for ensuring the "fldOffset" when
// added to "%lo(myObject)" does not overflow the immediate fields of the
// memory instructions.
//
//
// [About Offsets] Relative offsets are supplied to this module as
// positive byte offsets, but they may be internally stored scaled
// and/or negated, depending on what is most compact for the target
// system.  Since the object pointed to by the offset typically
// precedes the relocation address, it is profitable to store
// these negative offsets as positive numbers, but this decision
// is internal to the relocation information abstractions.
//

class Relocation;
class CodeBuffer;
class RelocIterator;

class relocInfo VALUE_OBJ_CLASS_SPEC {
  friend class RelocIterator;
 public:
  enum relocType {
    none	            =  0, // Used when no relocation should be generated
    oop_type                =  1, // embedded oop
    virtual_call_type       =  2, // a standard inline cache call for a virtual send
    opt_virtual_call_type   =  3, // a virtual call that has been statically bound (i.e., no IC cache)
    static_call_type        =  4, // a static send 
    static_stub_type        =  5, // stub-entry for static send  (takes care of interpreter case)    
    runtime_call_type       =  6, // Relative reference to external segment (f.ex, call to stub routines)
    external_word_type      =  7, // Absolute reference to external segment
    internal_word_type      =  8, // Absolute reference to local    segment    
    safepoint_type          =  9, // internal backwards branch    
    return_type             = 10, // return instruction 
    poll_type               = 11, // polling instruction for safepoints
    poll_return_type        = 12, // polling instruction for safepoints at return
    breakpoint_type         = 13, // an initialization barrier or safepoint
    data_prefix_tag         = 14, // tag for a relocation-data prefix
    yet_unused_type         = 15, // Still unused
    type_mask               = 15  // A mask which selects only the above values
  };

 protected:
  unsigned short _value;

  enum RawBitsToken { RAW_BITS };
  relocInfo(relocType type, RawBitsToken ignore, int bits)
    : _value((type << nontype_width) + bits) { }

 public:
  // constructor
  relocInfo(relocType type, int offset, int format = 0);

  #define APPLY_TO_RELOCATIONS(visitor) \
    visitor(oop) \
    visitor(virtual_call) \
    visitor(opt_virtual_call) \
    visitor(static_call) \
    visitor(static_stub) \
    visitor(runtime_call) \
    visitor(external_word) \
    visitor(internal_word) \
    visitor(safepoint) \
    visitor(return) \
    visitor(poll) \
    visitor(poll_return) \
    visitor(breakpoint) \


 protected:
  enum {
    value_width             = sizeof(unsigned short) * BitsPerByte,
    type_width              = 4,   // == log2(type_mask+1)
    nontype_width           = value_width - type_width,
    immediate_width         = nontype_width-1,
    immediate_tag           = 1 << immediate_width,  // or-ed into _value
    immediate_limit         = 1 << immediate_width
  };

  // accessors
 public: 
  relocType  type()       const { return (relocType)bitfield((intptr_t)_value, nontype_width,   type_width);  }
  int  format()           const { return            bitfield((intptr_t)_value,  offset_width, format_width);  }
  int  addr_offset()      const { assert(!is_prefix(), "must have offset");
				  return bitfield((intptr_t)_value, 0,  offset_width) * offset_unit; }

 public:
  const short* data()     const { assert(is_datalen(), "must have data");  return (const short*)(this + 1); }
  int          datalen()  const { assert(is_datalen(), "must have data");
				  return bitfield((intptr_t)_value,  0,         immediate_width);           }
 protected:
  int immediate()         const { assert(is_immediate(), "must have immediate");
				  return bitfield((intptr_t)_value,  0,         immediate_width);           }
 public:

  static int addr_unit()        { return offset_unit; }
  static int offset_limit()     { return (1 << offset_width) * offset_unit; }
  static int datalen_limit()    { return immediate_limit; }

  void set_type(relocType type);
  void set_format(int format);

  void remove() { set_type(none); }

 protected:
  bool is_none()                const { return type() == none;                      }

  // marks any call?
  bool is_call() const {
    switch (type()) { 
      case virtual_call_type:
      case opt_virtual_call_type:
      case static_call_type:
      case runtime_call_type:
	return true;
      default:
	return false;
    }
  }

  bool is_prefix()              const { return type() == data_prefix_tag;                    }
  bool is_datalen()             const { return is_prefix() && (_value & immediate_tag) == 0; }
  bool is_immediate()           const { return is_prefix() && (_value & immediate_tag) != 0; }

 public:
  // Occasionally records of type relocInfo::none will appear in the stream.
  // We do not bother to filter these out, but clients should ignore them.
  // These records serve as "filler" in three ways:
  //  - to skip large spans of unrelocated code (this is rare)
  //  - to pad out the relocInfo array to the required oop alignment
  //  - to disable old relocation information which is no longer applicable

  inline friend relocInfo filler_relocInfo();

  // Every non-prefix relocation may be preceded by at most one prefix,
  // which supplies 1 or more halfwords of associated data.  Conventionally,
  // an int is represented by 0, 1, or 2 halfwords, depending on how
  // many bits are required to represent the value.  (In addition,
  // if the sole halfword is a 10-bit unsigned number, it is made
  // "immediate" in the prefix header word itself.  This optimization
  // is invisible outside this module.)

  inline friend relocInfo prefix_relocInfo(int datalen = 0);

 protected:
  // an immediate relocInfo optimizes a prefix with one 10-bit unsigned value
  static relocInfo immediate_relocInfo(int data0) {
    assert(data0 >= 0 && data0 < immediate_limit, "data0 in limits");
    return relocInfo(relocInfo::data_prefix_tag, RAW_BITS, immediate_tag|data0);
  }

 public:
  // Support routines for compilers.

  // This routine updates a prefix and returns the limit pointer.
  // It tries to compress the prefix from 32 to 16 bits, and if
  // successful returns a reduced "prefix_limit" pointer.
  relocInfo* finish_prefix(short* prefix_limit);

  // bit-packers for the data array:

  // As it happens, the bytes within the shorts are ordered natively,
  // but the shorts within the word are ordered big-endian.
  // This is an arbitrary choice, made this way mainly to ease debugging.
  static int data0_from_int(jint x)         { return x >> value_width; }
  static int data1_from_int(jint x)         { return (short)x; }
  static jint long_from_data(short* data) {
    return (data[0] << value_width) + (unsigned short)data[1];
  }

  static jint short_data_at(int n, short* data, int datalen) {
    return datalen > n ? data[n] : 0;
  }

  static jint long_data_at(int n, short* data, int datalen) {
    return datalen > n+1 ? long_from_data(&data[n]) : short_data_at(n, data, datalen);
  }

  // Update methods for relocation information
  // (since code is dynamically patched, we also need to dynamically update the relocation info)
  // Both methods takes old_type, so it is able to performe sanity checks on the information removed.
  static void change_reloc_info_for_address(RelocIterator *itr, address pc, relocType old_type, relocType new_type);
  static void remove_reloc_info_for_address(RelocIterator *itr, address pc, relocType old_type);

  // Machine dependent stuff
  #include "incls/_relocInfo_pd.hpp.incl"

 protected:
  // Derived constant, based on format_width which is PD:
  enum {
    offset_width       = nontype_width - format_width,
    format_mask        = (1<<format_width) - 1
  };
 public:
  enum {
    have_format        = format_width > 0
  };
};


inline relocInfo filler_relocInfo() {
  return relocInfo(relocInfo::none, relocInfo::offset_limit() - relocInfo::offset_unit);
}

inline relocInfo prefix_relocInfo(int datalen) {
  assert(datalen >= 0 && datalen < relocInfo::datalen_limit(), "datalen in limits");
  return relocInfo(relocInfo::data_prefix_tag, relocInfo::RAW_BITS, datalen);
}


// Holder for flyweight relocation objects.
// Although the flyweight subclasses are of varying sizes,
// the holder is "one size fits all".
class RelocationHolder VALUE_OBJ_CLASS_SPEC {
  friend class Relocation;

  #ifdef ASSERT
 protected:
  bool                 _is_bound;       // part of a BoundRelocation?
 public:
  bool is_bound () { return _is_bound; }
  #endif

 private:
  // this preallocated memory must accommodate all subclasses of Relocation
  // (this number is assertion-checked in Relocation::operator new)
  enum { _relocbuf_size = 3 };
  void* _relocbuf[ _relocbuf_size ];

 public:
  Relocation* reloc() const { return (Relocation*) &_relocbuf[0]; }
  inline relocInfo::relocType type() const;

  RelocationHolder plus(int offset) const;

  inline relocInfo* pack_data(relocInfo* ri, address addr) const;

  inline RelocationHolder();                // initializes type to none

  inline RelocationHolder(Relocation* r);   // make a copy

  static const RelocationHolder none;
};


// Augmented holder, for a "live" relocation bound to a particular address.
// It also has access to the raw auxiliary relocation data for that address.
class BoundRelocation : public RelocationHolder {
  friend class RelocationHolder;
  friend class Relocation;
//  friend class CodeBuffer;

  // as if ": public StackObj"
 public:
  void* operator new(size_t size) { return StackObj::operator new(size); }
  void  operator delete(void* p)  {        StackObj::operator delete(p); }

 protected:
  CodeBlob*  _code;    // compiled method containing _addr
  address    _addr;    // instruction to which the relocation applies
  short*     _data;    // pointer to the relocation's data
  short      _datalen; // number of halfwords in _data
  char       _format;  // position within the instruction

  void set_has_data(bool b) {
    _datalen = !b ? -1 : 0;
    debug_only(_data = NULL);
  }

 public:
  // accessors
  //int     type()         const -- inherited
  address   addr()         const { return _addr; }
  CodeBlob* code()         const { return _code; }
  short*    data()         const { return _data; }
  int       datalen()      const { return _datalen; }
  bool  has_data()         const { return _datalen >= 0; }
  int       format()       const { return (relocInfo::have_format) ? _format : 0; }

  jint    long_data_at(int n) const { return relocInfo:: long_data_at(n, _data, _datalen); }
  jint   short_data_at(int n) const { return relocInfo::short_data_at(n, _data, _datalen); }

  void      set_addr(address addr) { _addr = addr; }

  // The address points to the affected displacement part of the instruction.
  // For RISC, this is just the whole instruction.
  // For Intel, this is an unaligned 32-bit word.

 protected:
  BoundRelocation(CodeBlob* code = NULL) : _code(code) {
    debug_only(_addr     = NULL);
    debug_only(_is_bound = true);
    set_has_data(false);
  }

  BoundRelocation(RelocationHolder const& holder, address addr, int format = 0)
      : RelocationHolder(holder), _addr(addr), _code(NULL) {
    debug_only(_is_bound = true);
    set_has_data(false);
    if (relocInfo::have_format)  _format = format;
  }

public:
  inline relocInfo* pack_data(relocInfo* ri);
  inline void     unpack_data(relocInfo::relocType t);

  // this updates the unpacked info for a new code buffer
  address update_addrs(address old_addr, const CodeBuffer& new_cb, const CodeBuffer& old_cb);
};
// (An "unbound" relocation is simply a packet of values being passed
// around, which will eventually be committed to a relocation record.
// By contrast, a "bound" relocation knows its place in a given code
// stream, and is capable of either writing or reading its reloc. data.)


// class oop_Relocation; ...
#define EACH_TYPE(name) \
   class name##_Relocation;

APPLY_TO_RELOCATIONS(EACH_TYPE);
#undef EACH_TYPE




// A RelocIterator iterates through the relocation information of a CodeBlob.
// It is a variable BoundRelocation which is able to take on successive
// values as it is advanced through a code stream.
// Usage:
//   RelocIterator iter(nm);
//   while (iter.next()) {
//     iter.reloc()->some_operation();
//   }
// or:
//   RelocIterator iter(nm);
//   while (iter.next()) {
//     switch (iter.type()) {
//      case relocInfo::oop_type          :
//      case relocInfo::ic_type           :
//      case relocInfo::prim_type         :
//      case relocInfo::uncommon_type     :
//      case relocInfo::runtime_call_type :
//      case relocInfo::internal_word_type: 
//      case relocInfo::external_word_type: 
//      ...
//     }
//   }

class RelocIterator: public BoundRelocation {
  friend class Relocation;
  friend class relocInfo;	// for change_reloc_info_for_address only

 private:  
  address    _limit;   // stop producing relocations after this _addr
  relocInfo* _current; // the current relocation information
  relocInfo* _end;     // end marker; we're done iterating when _current == _end
  short      _databuf; // spare buffer for compressed data
  short      _is_copy; // addr points into a copy of the code, not the CodeBlob

  relocInfo* current() const { assert(has_current(), "must have current");
                               return _current; }

  Relocation* make_reloc(relocInfo::relocType t) {
     // Lazy construction of flyweight relocation objects.
     assert(t == type(), "type must agree with current relocInfo");
     unpack_data(t);
     return BoundRelocation::reloc();
  }

  void set_limits(address begin, address limit);

  void advance_over_prefix();    // helper method

  void initialize(intptr_t delta, CodeBlob* nm, address begin, address limit);


  friend class PatchingRelocIterator;
  // make an uninitialized one, for PatchingRelocIterator:
  RelocIterator() { }

  void set_has_current(bool b) { set_has_data(b); }
  
 public:
  // constructor
  RelocIterator(intptr_t delta,	// delta from CodeBlob code to working copy
		CodeBlob* cb,    address begin = NULL, address limit = NULL);
  RelocIterator(CodeBlob* cb,    address begin = NULL, address limit = NULL);
  RelocIterator(CodeBuffer *cb, address begin = NULL, address limit = NULL);

  // get next reloc info, return !eos
  bool next() {
    _current++;
    assert(_current <= _end, "must not overrun relocInfo");
    if (_current == _end) {
      set_has_current(false);
      return false;
    }
    set_has_current(true);

    if (_current->is_prefix()) {
      advance_over_prefix();
      assert(!current()->is_prefix(), "only one prefix at a time");
    }

    _addr += _current->addr_offset();

    if (_limit != NULL && _addr >= _limit) {
      set_has_current(false);
      return false;
    }

    if (relocInfo::have_format)  _format = current()->format();
    return true;
  }

  // accessors  
  address      limit()        const { return _limit; }
  void     set_limit(address x);
  bool         is_copy()      const { return _is_copy; }

  Relocation* reloc();      // return make_reloc(type());

  // type-specific relocation accessors:  oop_Relocation* oop_reloc(), etc.
  #define EACH_TYPE(name) \
      name##_Relocation* name##_reloc() \
	{ return (name##_Relocation*) make_reloc(relocInfo::name##_type); }

  APPLY_TO_RELOCATIONS(EACH_TYPE);
  #undef EACH_TYPE

  relocInfo::relocType          type()         const { return current()->type(); }
  bool  has_current()         const { return has_data(); }

  // CodeBlob's have relocation indexes for faster random access:
  static int locs_and_index_size(int code_size, int locs_size);
  // Store an index into [dest_start..dest_end), returning a pointer to the src->locs_size()
  // bytes of storage in which the relocation stream must be copied.
  static relocInfo* create_index(CodeBuffer* src, relocInfo* dest_begin, relocInfo* dest_end);

 private:
  friend class CompiledCodeSafepointHandler;  // for inst_addr oly
  address inst_addr() const { return addr(); } // %%%% get rid of on next merge

#ifndef PRODUCT
 public:
  void print();  
  void print_current();  
#endif
};


// A Relocation is a flyweight object allocated within a RelocationHolder.
// It represents the relocation data of relocation record.
// So, the RelocIterator unpacks relocInfos into Relocations.

class Relocation VALUE_OBJ_CLASS_SPEC {
  friend class RelocationHolder;
  friend class BoundRelocation;

 private:
  static void guarantee_size();

 protected:

  static RelocationHolder newHolder() {
    return RelocationHolder();
  }

  void* operator new(size_t size, const RelocationHolder& holder) {
    if (size > sizeof(holder._relocbuf)) guarantee_size();
    assert((void* const *)holder.reloc() == &holder._relocbuf[0], "ptrs must agree");
    return holder.reloc();
  }

 public:
  RelocationHolder& holder() const {
    const int holder_offset = (intptr_t)&((RelocationHolder*)0)->_relocbuf[0];
    return *(RelocationHolder*)( (address)this - holder_offset );
  }

  BoundRelocation* binding() const {
    assert(holder().is_bound(), "holder must be a BoundRelocation");
    return (BoundRelocation*) &holder();
  }

 public:
  // make a generic relocation for a given type (if possible)
  static RelocationHolder spec_simple(relocInfo::relocType rtype);

  // here is the type-specific hook which writes relocation data:
  virtual int    pack_data()      { return 0; }

  // here is the type-specific hook which reads (unpacks) relocation data:
  virtual void unpack_data()      { assert(datalen()==0 || type()==relocInfo::none, "no data here"); }

  // this updates the unpacked info for a new code buffer
  virtual void update_addrs(const CodeBuffer& new_cb, const CodeBuffer& old_cb);

 protected:
  // Helper functions for pack_data() and unpack_data().

  // Most of the compression logic is confined here.
  // (The "immediate data" mechanism of relocInfo works independently
  // of this stuff, and acts to further compress most 1-word data prefixes.)

  // A variable-width int is encoded as a short if it will fit in 16 bits.
  // The decoder looks at datalen to decide whether to unpack short or long.
  // Most relocation records are quite simple, containing at most two ints.

  static bool is_short(jint x) { return x == (short)x; }
  static void add_short(short* &p, int x)  { *p++ = x; }
  static void add_long (short* &p, jint x) {
    *p++ = relocInfo::data0_from_int(x); *p++ = relocInfo::data1_from_int(x);
  }
  static void add_int  (short* &p, jint x) {   // add a variable-width int
    if (is_short(x))  add_short(p, x);
    else              add_long (p, x);
  }


  int pack_1_int (jint x0) {			// [] [x] [Xx]
    short* p = data();
    if (x0 != 0)  add_int(p, x0);
    return p - data();
  }
  int unpack_1_int() {
    assert(datalen() <= 2, "too much data");
    return binding()->long_data_at(0);
  }

  // With two ints, the short form is used only if both ints are short.
  int pack_2_ints(jint x0, jint x1) {		// [] [x y?] [Xx Y?y]
    short* p = data();
    if (x0 == 0 && x1 == 0) {
      // no halfwords needed to store zeroes
    } else if (is_short(x0) && is_short(x1)) {
      // 1-2 halfwords needed to store shorts
      add_short(p, x0); if (x1!=0) add_short(p, x1);
    } else {
      // 3-4 halfwords needed to store longs
      add_long (p, x0);            add_int  (p, x1);
    }
    return p - data();
  }
  void unpack_2_ints(jint& x0, jint& x1) {
    if (datalen() <= 2) {
      x0 = binding()->short_data_at(0);
      x1 = binding()->short_data_at(1);
    } else {
      assert(datalen() <= 4, "too much data");
      x0 = binding()->long_data_at(0);
      x1 = binding()->long_data_at(2);
    }
  }

 public:
  // accessors, some of which only make sense for a 'live' Relocation
  address   addr()         const { return binding()->addr(); }
  CodeBlob* code()         const { return binding()->code(); }
 protected:
  short*   data()         const { return binding()->data(); }
  int      datalen()      const { return binding()->datalen(); }
  int      format()       const { return binding()->format(); }
  bool     is_copy()      const;

  // platform-dependent utilities for decoding and patching instructions
  void       pd_set_data_value       (address x, intptr_t off); // a set or mem-ref
  address    pd_call_destination     ();
  void       pd_set_call_destination (address x, intptr_t off);
  void       pd_swap_in_breakpoint   (address x, short* instrs, int instrlen);
  void       pd_swap_out_breakpoint  (address x, short* instrs, int instrlen);
  static int pd_breakpoint_size      ();

  // this extracts the address of an address in the code stream instead of the reloc data
  address* pd_address_in_code       ();

  // this extracts an address from the code stream instead of the reloc data
  address  pd_get_address_from_code ();

  // these convert from byte offsets, to scaled offsets, to addresses
  jint scaled_offset(address x) {
    if (x == NULL)  return 0;
    int byte_offset = x - addr();
    assert(byte_offset != 0, "must not be identical with the relocation addr");
    int offset = -byte_offset / relocInfo::addr_unit();
    assert(address_from_scaled_offset(offset) == x, "just checkin'");
    return offset;
  }
  address address_from_scaled_offset(jint offset) {
    if (offset == 0)  return NULL;
    int byte_offset = -( offset * relocInfo::addr_unit() );
    return addr() + byte_offset;
  }

  // these convert between indexes and addresses in the runtime system
  static intptr_t    runtime_address_to_index(address runtime_address);
  static address index_to_runtime_address(intptr_t index);

 public:
  virtual relocInfo::relocType     type()                         { return relocInfo::none; }

  // is it a call instruction?
  virtual bool is_call()                         { return false; }

  // is it a data movement instruction?
  virtual bool is_data()                         { return false; }

  // some relocations can compute their own values
  virtual address  value();

  // all relocations are able to reassert their values
  virtual void set_value(address x);

  virtual void clear_inline_cache()              { }

  // This method assumes that all virtual/static (inline) caches are cleared (since for static_call_type and
  // ic_call_type is not always posisition dependent (depending on the state of the cache)). However, this is
  // probably a reasonable assumption, since empty caches simplifies code reloacation.
  virtual void fix_relocation_at_move(intptr_t delta) { }

  // This is used in the very specific case of updating for LoadPC
  virtual void force_target(address target);

  // these apply to oop_type relocs:
  virtual void  oops_do(void f(oop*))            { }
  virtual void fix_oop_relocation()              { }

  void print();
};


// certain inlines must be deferred until class Relocation is defined:

inline RelocationHolder::RelocationHolder() {
  // initialize the vtbl, just to keep things type-safe
  new(*this) Relocation();
  debug_only(_is_bound = false);
}


inline RelocationHolder::RelocationHolder(Relocation* r) {
  (*this) = r->holder();
  debug_only(_is_bound = false);
}


relocInfo::relocType RelocationHolder::type() const {
  return reloc()->type();
}


inline relocInfo* RelocationHolder::pack_data(relocInfo* ri, address addr) const {
  BoundRelocation br(*this, addr);    // associate the reloc with addr
  return br.pack_data(ri);
}


inline relocInfo* BoundRelocation::pack_data(relocInfo* ri) {
  (*ri)     = prefix_relocInfo();  // provisional prefix
  _data     = (short*) ri->data(); // we are going to write it not read it
  _datalen  = reloc()->pack_data();
  return ri->finish_prefix(_data + _datalen);
}


// A DataRelocation always points at a memory or load-constant instruction..
// It is absolute on most machines, and the constant is split on RISCs.
// The specific subtypes are oop, external_word, and internal_word.
// By convention, the "value" does not include a separately reckoned "offset".
class DataRelocation : public Relocation {
 public:
  bool          is_data()                      { return true; }

  // both target and offset must be computed somehow from relocation data
  virtual int    offset()                      { return 0; }
  address         value()                      = 0;
  void        set_value(address x)             { set_value(x, offset()); }
  void        set_value(address x, intptr_t o) { pd_set_data_value(x, o); }

  // The "o" (displacement) argument is relevant only to split relocations
  // on RISC machines.  In some CPUs (SPARC), the set-hi and set-lo ins'ns
  // can encode more than 32 bits between them.  This allows compilers to
  // share set-hi instructions between addresses that differ by a small
  // offset (e.g., different static variables in the same class).
  // On such machines, the "x" argument to set_value on all set-lo
  // instructions must be the same as the "x" argument for the
  // corresponding set-hi instructions.  The "o" arguments for the
  // set-hi instructions are ignored, and must not affect the high-half
  // immediate constant.  The "o" arguments for the set-lo instructions are
  // added into the low-half immediate constant, and must not overflow it.
};

// A CallRelocation always points at a call instruction.
// It is PC-relative on most machines.
class CallRelocation : public Relocation {
 public:
  bool is_call() { return true; }

  address  destination()                           { return pd_call_destination(); }
  void     set_destination(address x, intptr_t o); // pd_set_call_destination

  void     fix_relocation_at_move(intptr_t delta);
  address  value()                          { return destination();  }
  void     set_value(address x)             { set_destination(x, 0); }
  void     set_value(address x, intptr_t o) { set_destination(x, o); }
};

class oop_Relocation : public DataRelocation {
  relocInfo::relocType type() { return relocInfo::oop_type; }

 public:
  // encode in one of these formats:  [] [n] [n l] [Nn l] [Nn Ll]
  // an oop in the CodeBlob's oop pool
  static RelocationHolder spec(int oop_index, int offset = 0) {
    assert(oop_index > 0, "must be a pool-resident oop");
    RelocationHolder rh = newHolder();
    return (new(rh) oop_Relocation(oop_index, offset))->holder();
  }
  // an oop in the instruction stream
  static RelocationHolder spec_for_immediate() {
    const int oop_index = 0;
    const int offset    = 0;    // if you want an offset, use the oop pool
    RelocationHolder rh = newHolder();
    return (new(rh) oop_Relocation(oop_index, offset))->holder();
  }

 private:
  jint _oop_index;                  // if > 0, index into CodeBlob::oop_at
  jint _offset;                     // byte offset to apply to the oop itself

  oop_Relocation(int oop_index, int offset) {
    _oop_index = oop_index; _offset = offset;
  }

  friend class BoundRelocation; 
  oop_Relocation() { }

 public:
  int oop_index() { return _oop_index; }
  int offset()    { return _offset; }

  // data is packed in "2_ints" format:  [i o] or [Ii Oo]
  int    pack_data();
  void unpack_data();

  void oops_do(void f(oop*));       // visits addr, if long_data==0
  void fix_oop_relocation();        // reasserts oop value

  address value()  { return (address) *oop_addr(); }

  oop* oop_addr();                  // addr or &pool[long_data]
  oop  oop_value();                 // *oop_addr
  // Note:  oop_value transparently converts Universe::non_oop_word to NULL.
};

class virtual_call_Relocation : public CallRelocation {
  relocInfo::relocType type() { return relocInfo::virtual_call_type; }

 public:
  // "first_oop" points to the first associated set-oop.
  // The oop_limit helps find the last associated set-oop.
  // (See comments at the top of this file.)
  static RelocationHolder spec(address first_oop, address oop_limit = NULL) {
    RelocationHolder rh = newHolder();
    return (new(rh) virtual_call_Relocation(first_oop, oop_limit))->holder();
  }

  virtual_call_Relocation(address first_oop, address oop_limit) {
    _first_oop = first_oop; _oop_limit = oop_limit;
    assert(first_oop != NULL, "first oop address must be specified");
  }

 private:
  address _first_oop;               // location of first set-oop instruction
  address _oop_limit;               // search limit for set-oop instructions

  friend class BoundRelocation; 
  virtual_call_Relocation() { }


 public:
  address first_oop();
  address oop_limit();

  // data is packed as scaled offsets in "2_ints" format:  [f l] or [Ff Ll]
  // oop_limit is set to 0 if the limit falls somewhere within the call.
  // When unpacking, a zero oop_limit is taken to refer to the end of the call.
  // (This has the effect of bringing in the call's delay slot on SPARC.)
  int    pack_data();
  void unpack_data();

  void clear_inline_cache();

  // this updates the unpacked info for a new code buffer
  void update_addrs(const CodeBuffer& new_cb, const CodeBuffer& old_cb);

  // Figure out where an ic_call is hiding, given a set-oop or call.
  // Either ic_call or first_oop must be non-null; the other is deduced.
  // Code if non-NULL must be the CodeBlob, else it is deduced.
  // The address of the patchable oop is also deduced.
  // The returned iterator will enumerate over the oops and the ic_call,
  // as well as any other relocations that happen to be in that span of code.
  // Recognize relevant set_oops with:  oop_reloc()->oop_addr() == oop_addr.
  static RelocIterator parse_ic(CodeBlob* &code, address &ic_call, address &first_oop, oop* &oop_addr, bool *is_optimized);
};


class opt_virtual_call_Relocation : public CallRelocation {
  relocInfo::relocType type() { return relocInfo::opt_virtual_call_type; }

 public:
  static RelocationHolder spec() {
    RelocationHolder rh = newHolder();
    return (new(rh) opt_virtual_call_Relocation())->holder();
  }

 private:
  friend class BoundRelocation; 
  opt_virtual_call_Relocation() { }

 public:
  void clear_inline_cache();

  // find the matching static_stub
  address static_stub();
};


class static_call_Relocation : public CallRelocation {
  relocInfo::relocType type() { return relocInfo::static_call_type; }

 public:
  static RelocationHolder spec() {
    RelocationHolder rh = newHolder();
    return (new(rh) static_call_Relocation())->holder();
  }

 private:
  friend class BoundRelocation; 
  static_call_Relocation() { }

 public:
  void clear_inline_cache();

  // find the matching static_stub
  address static_stub();
};

class static_stub_Relocation : public Relocation {
  relocInfo::relocType type() { return relocInfo::static_stub_type; }

 public:
  static RelocationHolder spec(address static_call) {
    RelocationHolder rh = newHolder();
    return (new(rh) static_stub_Relocation(static_call))->holder();
  }

 private:
  address _static_call;             // location of corresponding static_call

  static_stub_Relocation(address static_call) {
    _static_call = static_call;
  }

  friend class BoundRelocation; 
  static_stub_Relocation() { }

 public:
  void clear_inline_cache();

  address static_call() { return _static_call; }

  // data is packed as a scaled offset in "1_int" format:  [c] or [Cc]
  int    pack_data();
  void unpack_data();

  // this updates the unpacked info for a new code buffer
  void update_addrs(const CodeBuffer& new_cb, const CodeBuffer& old_cb);
};

class runtime_call_Relocation : public CallRelocation {
  relocInfo::relocType type() { return relocInfo::runtime_call_type; }

 public:
  static RelocationHolder spec() {
    RelocationHolder rh = newHolder();
    return (new(rh) runtime_call_Relocation())->holder();
  }

 private:
  friend class BoundRelocation; 
  runtime_call_Relocation() { }

 public:
};

class external_word_Relocation : public DataRelocation {
  relocInfo::relocType type() { return relocInfo::external_word_type; }

 public:
  static RelocationHolder spec(address target) {
    assert(target != NULL, "must not be null");
    RelocationHolder rh = newHolder();
    return (new(rh) external_word_Relocation(target))->holder();
  }

  // use this one where all 32 bits of the target can fit in the code stream:
  static RelocationHolder spec_for_immediate() {
    RelocationHolder rh = newHolder();
    return (new(rh) external_word_Relocation(NULL))->holder();
  }

 private:
  address _target;                  // address in runtime

  external_word_Relocation(address target) {
    _target = target;
  }

  friend class BoundRelocation; 
  external_word_Relocation() { }

 public:
  // data is packed as a well-known address in "1_int" format:  [a] or [Aa]
  // The function runtime_address_to_index is used to turn full addresses
  // to short indexes, if they are pre-registered by the stub mechanism.
  // If the "a" value is 0 (i.e., _target is NULL), the address is stored
  // in the code stream.  See external_word_Relocation::target().
  int    pack_data();
  void unpack_data();

  void fix_relocation_at_move(intptr_t delta);
  address  target();        // if _target==NULL, fetch addr from code stream
  address  value()          { return target(); }

  // this updates the unpacked info for a new code buffer
  void update_addrs(const CodeBuffer& new_cb, const CodeBuffer& old_cb);
};

class internal_word_Relocation : public DataRelocation {
  relocInfo::relocType type() { return relocInfo::internal_word_type; }

 public:
  static RelocationHolder spec(address target) {
    assert(target != NULL, "must not be null");
    RelocationHolder rh = newHolder();
    return (new(rh) internal_word_Relocation(target))->holder();
  }

  // use this one where all 32 bits of the target can fit in the code stream:
  static RelocationHolder spec_for_immediate() {
    RelocationHolder rh = newHolder();
    return (new(rh) internal_word_Relocation(NULL))->holder();
  }

  internal_word_Relocation(address target) {
    _target = target;
  }

 private:
  address _target;                  // address in CodeBlob

  friend class BoundRelocation; 
  internal_word_Relocation() { }

 public:
  // data is packed as a scaled offset in "1_int" format:  [o] or [Oo]
  // If the "o" value is 0 (i.e., _target is NULL), the offset is stored
  // in the code stream.  See internal_word_Relocation::target().
  int    pack_data();
  void unpack_data();

  void fix_relocation_at_move(intptr_t delta);
  address  target();        // if _target==NULL, fetch addr from code stream
  address  value()          { return target();   }

  // this updates the unpacked info for a new code buffer
  void update_addrs(const CodeBuffer& new_cb, const CodeBuffer& old_cb);

  // This is used in the very specific case of updating for LoadPC
  void force_target(address target) { _target = target; fix_relocation_at_move(0); }
};

class safepoint_Relocation : public Relocation {
  relocInfo::relocType type() { return relocInfo::safepoint_type; }
};

class return_Relocation : public Relocation {
  relocInfo::relocType type() { return relocInfo::return_type; }
};

class poll_Relocation : public Relocation {
  relocInfo::relocType type() { return relocInfo::poll_type; }
};

class poll_return_Relocation : public Relocation {
  relocInfo::relocType type() { return relocInfo::poll_return_type; }
};


class breakpoint_Relocation : public Relocation {
  relocInfo::relocType type() { return relocInfo::breakpoint_type; }

  enum {
    // attributes which affect the interpretation of the data:
    removable_attr = 0x0010,   // buffer [i...] allows for undoing the trap
    internal_attr  = 0x0020,   // the target is an internal addr (local stub)
    settable_attr  = 0x0040,   // the target is settable

    // states which can change over time:
    enabled_state  = 0x0100,   // breakpoint must be active in running code
    active_state   = 0x0200,   // breakpoint instruction actually in code

    kind_mask      = 0x000F,   // mask for extracting kind
    high_bit       = 0x4000    // extra bit which is always set
  };

 public:
  enum {
    // kinds:
    initialization = 1,
    safepoint      = 2
  };

  // If target is NULL, 32 bits are reserved for a later set_target().
  static RelocationHolder spec(int kind, address target = NULL, bool internal_target = false) {
    RelocationHolder rh = newHolder();
    return (new(rh) breakpoint_Relocation(kind, target, internal_target))->holder();
  }

 private:
  // We require every bits value to NOT to fit into relocInfo::immediate_width,
  // because we are going to actually store state in the reloc, and so
  // cannot allow it to be compressed (and hence copied by the iterator).

  short   _bits;                  // bit-encoded kind, attrs, & state
  address _target;

  breakpoint_Relocation(int kind, address target, bool internal_target);

  friend class BoundRelocation; 
  breakpoint_Relocation() { }

  short    bits()       const { return _bits; }
  short&   live_bits()  const { return data()[0]; }
  short*   instrs()     const { return data() + datalen() - instrlen(); }
  int      instrlen()   const { return removable() ? pd_breakpoint_size() : 0; }

  void set_bits(short x) {
    assert(live_bits() == _bits, "must be the only mutator of reloc info");
    live_bits() = _bits = x;
  }

  void must_not_be_active() {
    assert(!active(), "cannot perform relocation on enabled breakpoints");
  }

 public:
  address  target()     const;
  void set_target(address x);

  int  kind()           const { return  bits() & kind_mask; }
  bool enabled()        const { return (bits() &  enabled_state) != 0; }
  bool active()         const { return (bits() &   active_state) != 0; }
  bool internal()       const { return (bits() &  internal_attr) != 0; }
  bool removable()      const { return (bits() & removable_attr) != 0; }
  bool settable()       const { return (bits() &  settable_attr) != 0; }

  void set_enabled(bool b);	// to activate, you must also say set_active
  void set_active(bool b);	// actually inserts bpt (must be enabled 1st)
  void set_copy_active(bool b); // actually inserts bpt into a code copy
  // set_active only works on the original; set_copy_active only on a copy

  // data is packed as 16 bits, followed by the target (1 or 2 words), followed
  // if necessary by empty storage for saving away original instruction bytes.
  int    pack_data();
  void unpack_data();

  // this updates the unpacked info for a new code buffer
  void update_addrs(const CodeBuffer& new_cb, const CodeBuffer& old_cb);

  // during certain operations, breakpoints must be out of the way:
  void fix_relocation_at_move(intptr_t delta) { must_not_be_active(); }
  void fix_oop_relocation()              { must_not_be_active(); }
};


#pragma optimize("", off)
// We know all the xxx_Relocation classes, so now we can define this:
inline void BoundRelocation::unpack_data(relocInfo::relocType t) {
  // The purpose of the placed "new" is (i) to re-use the same
  // stack storage for each new iteration, and (ii) to allow
  // direct calculation of Relocation::binding().
  Relocation *r;
  switch (t) {
    #define EACH_CASE(name) \
    case relocInfo::name##_type: \
      r = new(*this) name##_Relocation(); break;

    APPLY_TO_RELOCATIONS(EACH_CASE);
    #undef EACH_CASE

    default:
      r = new(*this) Relocation(); break;
  }
  assert(r == reloc(), "ptrs must agree");
  r->unpack_data();
}
#pragma optimize("", on)


inline RelocIterator::RelocIterator(intptr_t delta, CodeBlob* cb, address begin, address limit) {
  initialize(delta, cb, begin, limit);
}

inline RelocIterator::RelocIterator(CodeBlob* cb, address begin, address limit) {
  initialize(0, cb, begin, limit);
}

// if you are going to patch code, you should use this subclass of
// RelocIterator
class PatchingRelocIterator : public RelocIterator {
 private:
  RelocIterator _init_state;

  void prepass();		// deactivates all breakpoints
  void postpass();		// reactivates all enabled breakpoints

  // do not copy these puppies; it would have unpredictable side effects
  // these are private and have no bodies defined because they should not be called
  PatchingRelocIterator(const RelocIterator&);
  void        operator=(const RelocIterator&);

 public:
  PatchingRelocIterator(intptr_t delta,
			CodeBlob* cb, address begin =NULL, address limit =NULL)
    : RelocIterator(delta, cb, begin, limit)         { prepass();  }

  PatchingRelocIterator(CodeBlob* cb, address begin =NULL, address limit =NULL)
    : RelocIterator(cb, begin, limit)                { prepass();  }

  ~PatchingRelocIterator()                           { postpass(); }
};
