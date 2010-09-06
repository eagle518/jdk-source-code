#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)frame_amd64.inline.hpp	1.5 03/12/23 16:35:47 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Inline functions for amd64 frames:

// Constructors:

inline frame::frame() { _pc = NULL; _sp = NULL; _fp = NULL; }

// Accessors

inline bool frame::equal(frame other) const {
  return sp() == other.sp()
      && fp() == other.fp()
      && pc() == other.pc();
}

// Return unique id for this frame.  The id must have a value where we
// can distinguish identity and younger/older relationship. NULL
// represents an invalid (incomparable) frame.
inline intptr_t* frame::id(void) const
{
  return _sp;
}

// Relationals on frames based 
// Return true if the frame is younger (more recent activation) than
// the frame represented by id
inline bool frame::is_younger(intptr_t* id) const
{
  assert(this->id() != NULL && id != NULL, "NULL frame id");
  return this->id() < id;
}
// Return true if the frame is older (less recent activation) than the
// frame represented by id
inline bool frame::is_older(intptr_t* id) const
{
  assert(this->id() != NULL && id != NULL, "NULL frame id");
  return this->id() > id;
}


inline int frame::frame_size() const 
{
  return sender_sp() - sp();
}


inline intptr_t* frame::link() const
{
  return (intptr_t*) *(intptr_t **)addr_at(link_offset);
}

inline void frame::set_link(intptr_t* addr)
{
  *(intptr_t**) addr_at(link_offset) = addr;
}


inline intptr_t* frame::unextended_sp() const
{
  return sp();
}

inline intptr_t* frame::entry_frame_argument_at(int offset) const
{
  // Since an entry frame always calls the interpreter first,
  // the format of the call is always compatible with the interpreter.
  return &interpreter_frame_tos_at(offset);
}

#ifdef COMPILER2
inline intptr_t& frame::c2i_argument_at(int offset) const {
  // c2i adapter frames have a format compatible with the interpreter.
  return interpreter_frame_tos_at(offset);
}
#endif /* COMPILER2 */


// Return address:

inline address* frame::sender_pc_addr() const
{
  return (address*) addr_at( return_addr_offset);
}

inline address frame::sender_pc() const
{
  return *sender_pc_addr();
}

// return address of param, zero origin index.
inline address* frame::native_param_addr(int idx) const
{
  return (address*) addr_at( native_frame_initial_param_offset+idx);
}

inline intptr_t* frame::sender_sp() const
{
  return addr_at(sender_sp_offset);
}

inline int frame::pd_oop_map_offset_adjustment() const
{
  return 0;
}

inline intptr_t** frame::interpreter_frame_locals_addr() const
{ 
  return (intptr_t**) addr_at(interpreter_frame_locals_offset); 
}

inline intptr_t* frame::interpreter_frame_bcx_addr() const
{
  return (intptr_t*) addr_at(interpreter_frame_bcx_offset);
}

#ifndef CORE
inline intptr_t* frame::interpreter_frame_mdx_addr() const
{
  return (intptr_t*) addr_at(interpreter_frame_mdx_offset);
}
#endif // !CORE

inline intptr_t& frame::interpreter_frame_local_at(int index) const
{
  return  (*interpreter_frame_locals_addr()) [-index];
}


inline int frame::interpreter_frame_monitor_size()
{
  return BasicObjectLock::size();
}


// expression stack
// (the max_stack arguments are used by the GC; see class FrameClosure)

inline intptr_t* frame::interpreter_frame_expression_stack() const
{
  intptr_t* monitor_end = (intptr_t*) interpreter_frame_monitor_end();
  return monitor_end - 1;
}


inline intptr_t& 
frame::interpreter_frame_expression_stack_at(jint offset) const
{
  return interpreter_frame_expression_stack()[-offset];
}


inline jint frame::interpreter_frame_expression_stack_direction() 
{
  return -1;
}

// top of expression stack
inline intptr_t* frame::interpreter_frame_tos_address() const 
{
  return sp();
}

inline intptr_t& frame::interpreter_frame_tos_at(jint offset) const 
{ 
  return interpreter_frame_tos_address()[offset];
}


inline jint frame::interpreter_frame_expression_stack_size() const
 { 
   return 
     interpreter_frame_expression_stack() -
     interpreter_frame_tos_address() +
     1;
}


// Method

inline methodOop* frame::interpreter_frame_method_addr() const 
{ 
  return (methodOop*) addr_at(interpreter_frame_method_offset);
}


// Constant pool cache

inline constantPoolCacheOop* frame::interpreter_frame_cache_addr() const
{
  return (constantPoolCacheOop*) addr_at(interpreter_frame_cache_offset);
}


// Entry frames

inline JavaCallWrapper* frame::entry_frame_call_wrapper() const
{
 return (JavaCallWrapper*) at(entry_frame_call_wrapper_offset); 
}


// Compiled frames

inline int frame::local_offset_for_compiler(int local_index,
                                            int nof_args,
                                            int max_nof_locals,
                                            int max_nof_monitors)
{
  return nof_args - local_index + (local_index < nof_args ? 1: -1); 
}

inline int frame::monitor_offset_for_compiler(int local_index,
                                              int nof_args,
                                              int max_nof_locals,
                                              int max_nof_monitors)
{
  return local_offset_for_compiler(local_index,
                                   nof_args,
                                   max_nof_locals,
                                   max_nof_monitors);
}

inline int frame::min_local_offset_for_compiler(int nof_args,
                                                int max_nof_locals,
                                                int max_nof_monitors)
{
  return nof_args - (max_nof_locals + max_nof_monitors * 2) - 1;
}

inline bool frame::volatile_across_calls(Register reg)
{
  return true;
}


#ifndef CORE

#undef RAX_NAME
#ifdef COMPILER1
#define RAX_NAME (VMReg::Name(reax->encoding()))
#endif /* COMPILER1 */
#ifdef COMPILER2
#define RAX_NAME (VMReg::Name(RAX_num))
#endif /* COMPILER2 */

inline oop frame::saved_oop_result(RegisterMap* map) const       { 
  return *((oop*) map->location(RAX_NAME));
}

inline void frame::set_saved_oop_result(RegisterMap* map, oop obj) {
  *((oop*) map->location(RAX_NAME)) = obj;
}

#ifdef COMPILER1

inline oop frame::saved_receiver(RegisterMap* map) const       {
  return *((oop*) map->location(RAX_NAME));
}
inline void frame::set_saved_receiver(RegisterMap* map, oop obj) {
  *((oop*) map->location(RAX_NAME)) = obj;
}

#endif /* COMPILER1 */
#endif /* CORE */
