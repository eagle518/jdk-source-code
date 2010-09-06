#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)frame_ia64.inline.hpp	1.18 04/03/08 11:15:09 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Inline functions for ia64 frames:

// Constructors

inline frame::frame() { 
    _raw_sp = NULL;
    _bsp = NULL;
    _pfs = 0;
    _pc_address = NULL;
    _pc = NULL;
}


// Accessors:

inline bool frame::equal(frame other) const {
  // Seems close enough
  return sp() == other.sp()
      && fp() == other.fp()
      && pc_addr() == other.pc_addr() 
      && pc() == other.pc();
}

// Return unique id for this frame. The id must have a value where we can distinguish
// identity and younger/older rlationship. NULL represents an invalid (incomparable)
// frame.
// All frames in the system should have unique windows so use _bsp
inline intptr_t* frame::id(void) const { return _bsp; }

// Relationals on frames based 
// Return true if the frame is younger (more recent activation) than the frame represented by id
inline bool frame::is_younger(intptr_t* id) const { assert(this->id() != NULL && id != NULL, "NULL frame id");
					            return this->id() > id ; }

// Return true if the frame is older (less recent activation) than the frame represented by id
inline bool frame::is_older(intptr_t* id)   const { assert(this->id() != NULL && id != NULL, "NULL frame id");
					            return this->id() < id ; }


inline int frame::frame_size() const { return sender_sp() - sp(); }

inline void frame::set_link(intptr_t* addr) { assert(link()==addr, "frame nesting is controlled by hardware"); }

// We we have to deal with interpreter_sp_adjustment??? QQQ

inline intptr_t* frame::unextended_sp() const { return sp(); }

inline intptr_t* frame::entry_frame_argument_at(int offset) const {
  // Since an entry frame always calls the interpreter first,
  // the parameters are on the stack and relative to known register in the
  // entry frame.
  intptr_t* tos = (intptr_t*) *register_addr(GR_entry_frame_TOS);
  return &tos[offset + 1]; // prepushed tos
}




// return address:

// Only called for interpreted frames??
inline address  frame::sender_pc()        const    { 
  assert(is_interpreted_frame(), "must be interpreted");
  return (address) (*register_addr(GR_Lsave_RP));
}

inline address*  frame::sender_pc_addr()   const    { 
  assert(is_interpreted_frame(), "must be interpreted");
  return (address*) (register_addr(GR_Lsave_RP));
}

// Only called for interpreted frames??
inline void     frame::set_sender_pc(address addr) { 
  assert(is_interpreted_frame(), "must be interpreted");
  *register_addr(GR_Lsave_RP) = (intptr_t) (addr - pc_return_offset);
}

inline intptr_t*    frame::sender_sp() const  { 
  if (is_interpreted_frame()) {
    return (intptr_t*) (*register_addr(GR_Lsave_SP));
  } else {
#ifndef CORE
    CodeBlob* cb =  CodeCache::find_blob(pc());
    assert(cb != NULL, "Didn't find code");
    return compiled_sender_sp(cb);
#else
    ShouldNotReachHere();
    return NULL;
#endif
  }
}

// Used only in frame::oopmapreg_to_location
inline int frame::pd_oop_map_offset_adjustment() const {
  return 0 /* _interpreter_sp_adjustment*/ ;
}

inline intptr_t** frame::interpreter_frame_locals_addr() const { 
  interpreterState istate = get_interpreterState();
  return (intptr_t**) &istate->_locals;
}

inline intptr_t* frame::interpreter_frame_bcx_addr() const {
  interpreterState istate = get_interpreterState();
  return (intptr_t*) &istate->_bcp;
}

#ifndef CORE
inline intptr_t* frame::interpreter_frame_mdx_addr() const {
  interpreterState istate = get_interpreterState();
  return (intptr_t*) &istate->_mdx;
}
#endif // !CORE

inline intptr_t& frame::interpreter_frame_local_at(int index) const {
    return  (*interpreter_frame_locals_addr()) [-index];
}


inline intptr_t* frame::interpreter_frame_expression_stack() const {
  return (intptr_t*)interpreter_frame_monitor_end() - 1; // QQQ
}

inline intptr_t& frame::interpreter_frame_expression_stack_at(jint offset) const {
  return interpreter_frame_expression_stack()[-offset]; 
}

inline jint frame::interpreter_frame_expression_stack_direction() { return -1; }

// top of expression stack
inline intptr_t* frame::interpreter_frame_tos_address() const {
  interpreterState istate = get_interpreterState();
  return istate->_stack + 1; 
} 

inline jint frame::interpreter_frame_expression_stack_size() const { 
  return interpreter_frame_expression_stack() - interpreter_frame_tos_address() + 1;
}

inline intptr_t& frame::interpreter_frame_tos_at(jint offset) const { 
  return interpreter_frame_tos_address()[offset];
}

// monitor elements

// in keeping with Intel side: end is lower in memory than begin;
// and beginning element is oldest element
// Also begin is one past last monitor.

inline BasicObjectLock* frame::interpreter_frame_monitor_begin()       const  { 
  return get_interpreterState()->monitor_base();
}

inline BasicObjectLock* frame::interpreter_frame_monitor_end()         const  { 
  return (BasicObjectLock*) get_interpreterState()->stack_base();
}


inline void frame::interpreter_frame_set_monitor_end(BasicObjectLock* value) {
  // QQQ umm this could be bad...
  ShouldNotReachHere();
}


inline int frame::interpreter_frame_monitor_size() {
  return round_to(BasicObjectLock::size(), WordsPerLong);
}

inline methodOop* frame::interpreter_frame_method_addr() const { 
  interpreterState istate = get_interpreterState();
  return &istate->_method;
}


// Constant pool cache

// where LcpoolCache is saved:
// Why are there two of these??
inline constantPoolCacheOop* frame::interpreter_frame_cpoolcache_addr() const { 
  interpreterState istate = get_interpreterState();
  return &istate->_constants; // should really use accessor
  }

inline constantPoolCacheOop* frame::interpreter_frame_cache_addr() const {
  interpreterState istate = get_interpreterState();
  return &istate->_constants;
}


inline JavaCallWrapper* frame::entry_frame_call_wrapper() const {
  // note: adjust this code if the link argument in StubGenerator::call_stub() changes!
  return (JavaCallWrapper*) *register_addr(GR_I0);
}


#if 0
inline int frame::local_offset_for_compiler(int local_index, int nof_args, int max_nof_locals, int max_nof_monitors) {
   // always allocate non-argument locals 0..5 as if they were arguments:
  int allocated_above_frame = nof_args;
  if (allocated_above_frame < callee_register_argument_save_area_words)
    allocated_above_frame = callee_register_argument_save_area_words;
  if (allocated_above_frame > max_nof_locals)
    allocated_above_frame = max_nof_locals;

  // Note: monitors (BasicLock blocks) are never allocated in argument slots
  //assert(local_index >= 0 && local_index < max_nof_locals, "bad local index");
  if (local_index < allocated_above_frame)
    return local_index + callee_register_argument_save_area_sp_offset;
  else
    return local_index - (max_nof_locals + max_nof_monitors*2) + compiler_frame_vm_locals_fp_offset;
}

inline int frame::monitor_offset_for_compiler(int local_index, int nof_args, int max_nof_locals, int max_nof_monitors) {
  assert(local_index >= max_nof_locals && ((local_index - max_nof_locals) & 1) && (local_index - max_nof_locals) < max_nof_monitors*2, "bad monitor index");

  // The compiler uses the __higher__ of two indexes allocated to the monitor.
  // Increasing local indexes are mapped to increasing memory locations,
  // so the start of the BasicLock is associated with the __lower__ index.

  int offset = (local_index-1) - (max_nof_locals + max_nof_monitors*2) + compiler_frame_vm_locals_fp_offset;

  // We allocate monitors aligned zero mod 8:
  assert((offset & 1) == 0, "monitor must be an an even address.");
  // This works because all monitors are allocated after
  // all locals, and because the highest address corresponding to any
  // monitor index is always even.
  assert((compiler_frame_vm_locals_fp_offset & 1) == 0, "end of monitors must be even address");

  return offset;
}

inline int frame::min_local_offset_for_compiler(int nof_args, int max_nof_locals, int max_nof_monitors) {
   // always allocate non-argument locals 0..5 as if they were arguments:
  int allocated_above_frame = nof_args;
  if (allocated_above_frame < callee_register_argument_save_area_words)
    allocated_above_frame = callee_register_argument_save_area_words;
  if (allocated_above_frame > max_nof_locals)
    allocated_above_frame = max_nof_locals;

  int allocated_in_frame = (max_nof_locals + max_nof_monitors*2) - allocated_above_frame;

  return compiler_frame_vm_locals_fp_offset - allocated_in_frame;
}

// On SPARC, the %lN and %iN registers are non-volatile.
inline bool frame::volatile_across_calls(Register reg) {
  // This predicate is (presently) applied only to temporary registers,
  // and so it need not recognize non-volatile globals.
  return true; 
  // QQQ
#if 0
  return reg->is_out() || reg->is_global();
#endif
}
#endif /* 0 */

#ifndef CORE
#undef GR8_RET_NAME
#define GR8_RET_NAME (VMReg::Name(GR8_RET_num))

inline oop frame::saved_oop_result(RegisterMap* map) const       {
  return *((oop*) map->location(GR8_RET_NAME));
}

inline void frame::set_saved_oop_result(RegisterMap* map, oop obj) {
  *((oop*) map->location(GR8_RET_NAME)) = obj;
}
#endif /* CORE */

