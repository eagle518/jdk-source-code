#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)interp_masm_sparc.hpp	1.87 04/03/22 19:28:24 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// This file specializes the assember with interpreter-specific macros

REGISTER_DECLARATION(     Register, Otos_i , O0); // tos for ints, etc
REGISTER_DECLARATION(     Register, Otos_l , O0); // for longs
REGISTER_DECLARATION(     Register, Otos_l1, O0); // for 1st part of longs
REGISTER_DECLARATION(     Register, Otos_l2, O1); // for 2nd part of longs
REGISTER_DECLARATION(FloatRegister, Ftos_f , F0); // for floats
REGISTER_DECLARATION(FloatRegister, Ftos_d , F0); // for doubles
REGISTER_DECLARATION(FloatRegister, Ftos_d1, F0); // for 1st part of double
REGISTER_DECLARATION(FloatRegister, Ftos_d2, F1); // for 2nd part of double

#ifndef DONT_USE_REGISTER_DEFINES
#define Otos_i  O0
#define Otos_l  O0
#define Otos_l1 O0
#define Otos_l2 O1
#define Ftos_f  F0
#define Ftos_d  F0
#define Ftos_d1 F0
#define Ftos_d2 F1
#endif // DONT_USE_REGISTER_DEFINES

class InterpreterMacroAssembler: public MacroAssembler {
 protected:
  // Interpreter specific version of call_VM_base
    virtual void call_VM_leaf_base(
    Register java_thread,
    address  entry_point,
    int      number_of_arguments
  );

  virtual void call_VM_base(
    Register        oop_result,
    Register        java_thread,
    Register        last_java_sp,
    address         entry_point,
    int             number_of_arguments,
    bool	    check_exception=true
  );

  virtual void check_and_handle_popframe(Register java_thread);

  // base routine for all dispatches
  void dispatch_base(TosState state, address* table);

 public:
  InterpreterMacroAssembler(CodeBuffer* c)
    : MacroAssembler(c) {}

  static const Address l_tmp ;
  static const Address d_tmp ;

  // helper routine for frame allocation/deallocation
  // compute the delta by which the caller's SP has to
  // be adjusted to accomodate for the non-argument
  // locals
  void compute_extra_locals_size_in_bytes(Register args_size, Register locals_size, Register delta);

  // dispatch routines
  void dispatch_prolog(TosState state, int step = 0);
  void dispatch_epilog(TosState state, int step = 0);
  void dispatch_only(TosState state);
  void dispatch_normal(TosState state);
  void dispatch_next(TosState state, int step = 0);
  void dispatch_next_noverify_oop(TosState state, int step = 0);
  void dispatch_via (TosState state, address* table);


 protected:
  void dispatch_Lbyte_code(TosState state, address* table, int bcp_incr = 0, bool verify = true);

 public:
  // Super call_VM calls - correspond to MacroAssembler::call_VM(_leaf) calls
  void super_call_VM_leaf(Register thread_cache, address entry_point, Register arg_1);
  void super_call_VM(Register thread_cache, Register oop_result, Register last_java_sp, address entry_point, Register arg_1, Register arg_2, bool check_exception = true);

  // Generate a subtype check: branch to ok_is_subtype if sub_klass is
  // a subtype of super_klass.  Blows registers tmp1, tmp2 and tmp3.
  void gen_subtype_check( Register sub_klass, Register super_klass, Register tmp1, Register tmp2, Register tmp3, Label &ok_is_subtype );

  // helpers for tossing exceptions
  void throw_if_not_1_icc( Condition ok_condition, Label& ok );
  void throw_if_not_1_xcc( Condition ok_condition, Label& ok );
  void throw_if_not_1_x  ( Condition ok_condition, Label& ok ); // chooses icc or xcc based on _LP64

  void throw_if_not_2( address throw_entry_point, Register Rscratch, Label& ok);

  void throw_if_not_icc( Condition ok_condition, address throw_entry_point, Register Rscratch );
  void throw_if_not_xcc( Condition ok_condition, address throw_entry_point, Register Rscratch );
  void throw_if_not_x  ( Condition ok_condition, address throw_entry_point, Register Rscratch );

  // helpers for expression stack

  void pop_i(     Register r = Otos_i);
  void pop_ptr(   Register r = Otos_i);
  void pop_l(     Register r = Otos_l1);
  void pop_f(FloatRegister f = Ftos_f);
  void pop_d(FloatRegister f = Ftos_d1);

  void push_i(     Register r = Otos_i);
  void push_ptr(     Register r = Otos_i);
  void push_l(     Register r = Otos_l1);
  void push_f(FloatRegister f = Ftos_f);
  void push_d(FloatRegister f = Ftos_d1);


  void pop (TosState state);           // transition vtos -> state
  void push(TosState state);           // transition state -> vtos
  void empty_expression_stack();       // resets both Lesp and SP

#ifdef ASSERT
  void verify_sp(Register Rsp, Register Rtemp);
  void verify_esp(Register Resp);      // verify that Lesp points to a word in the temp stack
#endif // ASSERT

 public:
  void if_cmp(Condition cc, bool ptr_compare);

  // Load values from bytecode stream:

  enum signedOrNot { Signed, Unsigned };
  enum setCCOrNot  { set_CC,  dont_set_CC };

  void get_2_byte_integer_at_bcp( int         bcp_offset, 
                                  Register    Rtmp, 
				  Register    Rdst,
                                  signedOrNot is_signed,
                        	  setCCOrNot  should_set_CC = dont_set_CC );

  void get_4_byte_integer_at_bcp( int        bcp_offset,
                                  Register   Rtmp, 
				  Register   Rdst,
				  setCCOrNot should_set_CC = dont_set_CC );

  void get_cache_and_index_at_bcp(Register cache, Register tmp, int bcp_offset);
  void get_cache_entry_pointer_at_bcp(Register cache, Register tmp, int bcp_offset);


  // common code

  void field_offset_at(int n, Register tmp, Register dest, Register base);
  int  field_offset_at(Register object, address bcp, int offset);
  void fast_iaaccess(int n, address bcp);
  void fast_iagetfield(address bcp);
  void fast_iaputfield(address bcp, bool do_store_check );
  
  void index_check(Register array, Register index, int index_shift, Register tmp, Register res);
  void index_check_without_pop(Register array, Register index, int index_shift, Register tmp, Register res);

  void get_constant_pool(Register Rdst);
  void get_constant_pool_cache(Register Rdst);
  void get_cpool_and_tags(Register Rcpool, Register Rtags);
  void is_a(Label& L);


  // --------------------------------------------------

  void unlock_if_synchronized_method(TosState state, bool throw_monitor_exception = true, bool install_monitor_exception = true);

  void add_monitor_to_stack( bool stack_is_empty,
		             Register Rtemp,
			     Register Rtemp2 );

  void access_local_int( Register index, Register dst );
  void access_local_ptr( Register index, Register dst );
  void access_local_long( Register index, Register dst );
  void access_local_float( Register index, FloatRegister dst );
  void access_local_double( Register index, FloatRegister dst );
#ifdef ASSERT
  void check_for_regarea_stomp( Register Rindex, int offset, Register Rlimit, Register Rscratch, Register Rscratch1);
#endif // ASSERT
  void store_local_int( Register index, Register src );
  void store_local_ptr( Register index, Register src );
  void store_local_long( Register index, Register src );
  void store_local_float( Register index, FloatRegister src );
  void store_local_double( Register index, FloatRegister src );


  Address first_local_in_stack();
  static int top_most_monitor_byte_offset(); // offset in bytes to top of monitor block
  Address top_most_monitor();
  void compute_stack_base( Register Rdest );

  enum LoadOrStore { load, store };
  void static_iload_or_store( int which_local, LoadOrStore direction, Register Rtmp );
  void static_aload_or_store( int which_local, LoadOrStore direction, Register Rtmp );
  void static_dload_or_store( int which_local, LoadOrStore direction );

  void static_iinc(           int which_local, jint increment, Register Rtmp, Register Rtmp2 );

  void increment_invocation_counter( Register Rtmp, Register Rtmp2 );
  void increment_backedge_counter( Register Rtmp, Register Rtmp2 );
  void test_backedge_count_for_osr( Register backedge_count, Register branch_bcp, Register Rtmp );

  void record_static_call_in_profile( Register Rentry, Register Rtmp );
  void record_receiver_call_in_profile( Register Rklass, Register Rentry, Register Rtmp );

  // Object locking
  void lock_object  (Register lock_reg, Register obj_reg);
  void unlock_object(Register lock_reg);

  // Interpreter profiling operations
  void set_method_data_pointer() { set_method_data_pointer_offset(noreg); }
  void set_method_data_pointer_for_bcp();
  void set_method_data_pointer_offset(Register mdi_reg);
  void test_method_data_pointer(Label& zero_continue);
  void verify_method_data_pointer();
  void test_invocation_counter_for_mdp(Register invocation_count, Register cur_bcp, Register Rtmp, Label &profile_continue);

  void set_mdp_data_at(int constant, Register value);
  void increment_mdp_data_at(int constant, Register bumped_count);
  void increment_mdp_data_at(Register reg, int constant,
			     Register bumped_count, Register scratch2);
  void set_mdp_flag_at(int flag_constant, Register scratch, Register scratch2);
  void test_mdp_data_at(int offset, Register value, Label& not_equal_continue,
			Register scratch);

  void update_mdp_by_offset(int offset_of_disp, Register scratch);
  void update_mdp_by_offset(Register reg, int offset_of_disp,
			    Register scratch);
  void update_mdp_by_constant(int constant);
  void update_mdp_for_ret(TosState state, Register return_bci);

  void profile_taken_branch(Register scratch, Register bumped_count);
  void profile_not_taken_branch(Register scratch);
  void profile_call(Register scratch);
  void profile_final_call(Register scratch);
  void profile_virtual_call(Register receiver, Register scratch);
  void profile_ret(TosState state, Register return_bci, Register scratch);
  void profile_checkcast(bool is_null, Register scratch1, Register scratch2);
  void profile_switch_default(Register scratch);
  void profile_switch_case(Register index,
			   Register scratch1,
			   Register scratch2,
			   Register scratch3);

  // Debugging
  void verify_oop(Register reg, TosState state = atos);    // only if +VerifyOops && state == atos
  void verify_oop_or_return_address(Register reg, Register rtmp); // for astore
  void verify_FPU(int stack_depth, TosState state = ftos); // only if +VerifyFPU  && (state == ftos || state == dtos)

  // support for jvmdi/jvmpi
  void notify_method_entry();
  void notify_method_exit(bool save_result, TosState state);
  void notify_jvmpi_method_exit(bool save_result, TosState state);

};

//Reconciliation History
// 1.16 97/11/16 18:25:23 interp_masm_i486.hpp
// 1.18 97/12/11 17:10:49 interp_masm_i486.hpp
// 1.21 98/02/10 13:33:51 interp_masm_i486.hpp
// 1.22 98/02/23 15:09:35 interp_masm_i486.hpp
// 1.24 98/03/04 13:43:11 interp_masm_i486.hpp
// 1.25 98/03/18 11:21:21 interp_masm_i486.hpp
// 1.27 98/03/26 16:49:51 interp_masm_i486.hpp
// 1.30 98/04/13 13:37:58 interp_masm_i486.hpp
// 1.31 98/05/11 13:42:30 interp_masm_i486.hpp
// 1.33 98/06/19 17:46:59 interp_masm_i486.hpp
// 1.34 98/07/01 15:42:16 interp_masm_i486.hpp
// 1.36 98/07/13 14:03:06 interp_masm_i486.hpp
// 1.37 98/10/02 13:39:53 interp_masm_i486.hpp
// 1.38 98/11/25 16:05:18 interp_masm_i486.hpp
// 1.39 99/01/25 13:50:17 interp_masm_i486.hpp
// 1.43 99/03/10 18:06:14 interp_masm_i486.hpp
// 1.44 99/04/02 16:24:07 interp_masm_i486.hpp
// 1.47 99/06/28 09:58:57 interp_masm_i486.hpp
// 1.44 99/04/01 16:52:59 interp_masm_i486.hpp
// 1.45 99/04/05 16:58:01 interp_masm_i486.hpp
// 1.48 99/07/06 16:02:53 interp_masm_i486.hpp
//End
