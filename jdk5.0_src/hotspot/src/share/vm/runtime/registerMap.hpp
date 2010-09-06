#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)registerMap.hpp	1.6 03/12/23 16:44:05 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

class JavaThread;

//
// RegisterMap
//
// A companion structure used for stack traversal. The RegisterMap contains
// misc. information needed in order to do correct stack traversal of stack
// frames.  Hence, it must always be passed in as an argument to
// frame::sender(RegisterMap*).
//
// In particular, 
//   1) It provides access to the thread for which the stack belongs.  The
//      thread object is needed in order to get sender of a deoptimized frame.
//
//   2) It is used to pass information from a callee frame to its caller 
//      frame about how the frame should be traversed.  This is used to let
//      the caller frame take care of calling oops-do of out-going
//      arguments, when the callee frame is not instantiated yet.  This
//      happens, e.g., when a compiled frame calls into
//      resolve_virtual_call.  (Hence, it is critical that the same
//      RegisterMap object is used for the entire stack walk.  Normally,
//      this is hidden by using the StackFrameStream.)  This is used when
//      doing follow_oops and oops_do.
//
//   3) The RegisterMap keeps track of the values of callee-saved registers 
//      from frame to frame (hence, the name).  For some stack traversal the
//      values of the callee-saved registers does not matter, e.g., if you
//      only need the static properies such as frame type, pc, and such.
//      Updating of the RegisterMap can be turned off by instantiating the
//      register map as: RegisterMap map(thread, false);

class RegisterMap : public StackObj { 
 public:
    typedef julong LocationValidType;
  enum {
    reg_count = CORE_ONLY(RegisterImpl::number_of_registers)
                NOT_CORE(REG_COUNT),
    location_valid_type_size = sizeof(LocationValidType)*8,
    location_valid_size = (reg_count+location_valid_type_size-1)/location_valid_type_size
  };
 private:
  intptr_t*    _location[reg_count];    // Location of registers (intptr_t* looks better than address in the debugger)
  LocationValidType _location_valid[location_valid_size];
  bool        _include_argument_oops;   // Should include argument_oop marked locations for compiler  
  JavaThread* _thread;                  // Reference to current thread
  bool        _update_map;              // Tells if the register map need to be updated
                                        // when traversing the stack
  intptr_t*   _not_at_call_id;          // Location of a frame where the pc is not at a call (NULL if no frame exist)
 public:
  debug_only(intptr_t* _update_for_id;) // Assert that RegisterMap is not updated twice for same frame
  RegisterMap(JavaThread *thread, bool update_map = true);
  RegisterMap(const RegisterMap* map);
    
#ifndef CORE
  address location(VMReg::Name reg) const {
    int index = reg / location_valid_type_size;
    assert(0 <= reg && reg < reg_count, "range check");
    assert(0 <= index && index < location_valid_size, "range check");
    if (_location_valid[index] & ((LocationValidType)1 << (reg % location_valid_type_size))) {
      return (address) _location[reg];
    } else {
      return pd_location(reg);
    }
  }

  void set_location(VMReg::Name reg, address loc) {
    int index = reg / location_valid_type_size;
    assert(0 <= reg && reg < reg_count, "range check");
    assert(0 <= index && index < location_valid_size, "range check");
    assert(_update_map, "updating map that does not need updating");
    _location[reg] = (intptr_t*) loc;
    _location_valid[index] |= ((LocationValidType)1 << (reg % location_valid_type_size));
  }
#endif

  // Called by an entry frame.
  void clear(intptr_t* not_at_call_id);

  bool include_argument_oops() const      { return _include_argument_oops; }
  void set_include_argument_oops(bool f)  { _include_argument_oops = f; }

  JavaThread *thread() const { return _thread; }
  bool update_map()    const { return _update_map; }

  void print_on(outputStream* st) const;
  void print() const;

  // Checks if the pc is at a call for the given id.
  bool is_pc_at_call(intptr_t* id) const { return _not_at_call_id != id; }  

  // the following contains the definition of pd_xxx methods
# include "incls/_registerMap_pd.hpp.incl"
};
