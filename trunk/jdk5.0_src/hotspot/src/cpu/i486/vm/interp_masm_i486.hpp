#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)interp_masm_i486.hpp	1.71 04/03/22 19:28:35 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// This file specializes the assember with interpreter-specific macros


class InterpreterMacroAssembler: public MacroAssembler {
 protected:
  // Interpreter specific version of call_VM_base
  virtual void call_VM_leaf_base(
    address entry_point,
    int     number_of_arguments
  );

  virtual void call_VM_base(
    Register oop_result,
    Register java_thread,
    Register last_java_sp,
    address  entry_point,
    int      number_of_arguments,
    bool     check_exceptions
  );

  virtual void check_and_handle_popframe(Register java_thread);

  // base routine for all dispatches
  void dispatch_base(TosState state, address* table, bool verifyoop = true);

 public:
  InterpreterMacroAssembler(CodeBuffer* code) : MacroAssembler(code) {}

  // Interpreter-specific registers
  void save_bcp()                                          { movl(Address(ebp, frame::interpreter_frame_bcx_offset * wordSize), esi); }
  void restore_bcp()                                       { movl(esi, Address(ebp, frame::interpreter_frame_bcx_offset * wordSize)); }
  void restore_locals()                                    { movl(edi, Address(ebp, frame::interpreter_frame_locals_offset * wordSize)); }

  // Helpers for runtime call arguments/results
  void get_method(Register reg)                            { movl(reg, Address(ebp, frame::interpreter_frame_method_offset * wordSize)); }
  void get_constant_pool(Register reg)                     { get_method(reg); movl(reg, Address(reg, methodOopDesc::constants_offset())); }
  void get_constant_pool_cache(Register reg)               { get_constant_pool(reg); movl(reg, Address(reg, constantPoolOopDesc::cache_offset_in_bytes())); }
  void get_cpool_and_tags(Register cpool, Register tags)   { get_constant_pool(cpool); movl(tags, Address(cpool, constantPoolOopDesc::tags_offset_in_bytes()));
  }
  void get_unsigned_2_byte_index_at_bcp(Register reg, int bcp_offset);
  void get_cache_and_index_at_bcp(Register cache, Register index, int bcp_offset);
  void get_cache_entry_pointer_at_bcp(Register cache, Register tmp, int bcp_offset);

  // Expression stack
  void f2ieee();                                           // truncate ftos to 32bits
  void d2ieee();                                           // truncate dtos to 64bits
  void pop (TosState state);                               // transition vtos -> state
  void push(TosState state);                               // transition state -> vtos
  void empty_expression_stack()                            { movl(esp, Address(ebp, frame::interpreter_frame_monitor_block_top_offset * wordSize)); }

  // Super call_VM calls - correspond to MacroAssembler::call_VM(_leaf) calls
  void super_call_VM_leaf(address entry_point);
  void super_call_VM_leaf(address entry_point, Register arg_1);
  void super_call_VM_leaf(address entry_point, Register arg_1, Register arg_2);
  void super_call_VM_leaf(address entry_point, Register arg_1, Register arg_2, Register arg_3);
  void super_call_VM(Register oop_result, Register last_java_sp, address entry_point, Register arg_1, Register arg_2);

  // Generate a subtype check: branch to ok_is_subtype if sub_klass is
  // a subtype of super_klass.  EAX holds the super_klass.  Blows ECX
  // and EDI.  Register sub_klass cannot be any of the above.
  void gen_subtype_check( Register sub_klass, Label &ok_is_subtype );

  // Dispatching
  void dispatch_prolog(TosState state, int step = 0);
  void dispatch_epilog(TosState state, int step = 0);
  void dispatch_only(TosState state);                      // dispatch via ebx (assume ebx is loaded already)
  void dispatch_only_normal(TosState state);               // dispatch normal table via ebx (assume ebx is loaded already)
  void dispatch_only_noverify(TosState state);
  void dispatch_next(TosState state, int step = 0);        // load ebx from [esi + step] and dispatch via ebx
  void dispatch_via (TosState state, address* table);      // load ebx from [esi] and dispatch via ebx and table

  // Returning from interpreted functions
  //
  // Removes the current activation (incl. unlocking of monitors)
  // and sets up the return address.  This code is also used for
  // exception unwindwing. In that case, we do not want to throw 
  // IllegalMonitorStateExceptions, since that might get us into an 
  // infinite rethrow exception loop.
  // Additionally this code is used for popFrame, in which case 
  // we want to skip throwing an exception, installing an exception, 
  // and notifying jvmdi.
  void remove_activation(TosState state, Register ret_addr, 
                         bool throw_monitor_exception = true, 
                         bool install_monitor_exception = true,
                         bool notify_jvmdi = true);

  // Object locking
  void lock_object  (Register lock_reg);
  void unlock_object(Register lock_reg);

  // Interpreter profiling operations
  void set_method_data_pointer_for_bcp();
  void test_method_data_pointer(Register mdp, Label& zero_continue);
  void verify_method_data_pointer();

  void set_mdp_data_at(Register mdp_in, int constant, Register value);
  void increment_mdp_data_at(Register mdp_in, int constant);
  void increment_mdp_data_at(Register mdp_in, Register reg, int constant);
  void set_mdp_flag_at(Register mdp_in, int flag_constant);
  void test_mdp_data_at(Register mdp_in, int offset, Register value, Label& not_equal_continue);

  void update_mdp_by_offset(Register mdp_in, int offset_of_offset);
  void update_mdp_by_offset(Register mdp_in, Register reg, int offset_of_disp);
  void update_mdp_by_constant(Register mdp_in, int constant);
  void update_mdp_for_ret(Register return_bci);

  void profile_taken_branch(Register mdp, Register bumped_count);
  void profile_not_taken_branch(Register mdp);
  void profile_call(Register mdp);
  void profile_final_call(Register mdp);
  void profile_virtual_call(Register receiver, Register mdp, Register scratch2);
  void profile_ret(Register return_bci, Register mdp);
  void profile_checkcast(bool is_null, Register mdp);
  void profile_switch_default(Register mdp);
  void profile_switch_case(Register index_in_scratch, Register mdp, Register scratch2);

  // Debugging
  void verify_oop(Register reg, TosState state = atos);    // only if +VerifyOops && state == atos
  void verify_FPU(int stack_depth, TosState state = ftos); // only if +VerifyFPU  && (state == ftos || state == dtos)
    
  // support for jvmdi/jvmpi
  void notify_method_entry();
  void notify_method_exit(TosState state);
  void notify_jvmpi_method_exit(TosState state);
  
};

