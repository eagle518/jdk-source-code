#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)parseHelper.cpp	1.179 04/03/03 08:20:04 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_parseHelper.cpp.incl"

//------------------------------make_jvmpi_method_entry------------------------
// JVMPI -- record entry to a method if compiled while JVMPI is turned on
void GraphKit::make_jvmpi_method_entry() {
  const TypeFunc *call_type    = OptoRuntime::jvmpi_method_entry_Type();
  address         call_address = OptoRuntime::jvmpi_method_entry_Java();
  const char     *call_name    = OptoRuntime::stub_name( call_address );
  assert(bci() == InvocationEntryBci, "must be outside all blocks");
  const TypeInstPtr *method_type = TypeInstPtr::make(TypePtr::Constant, method()->klass(), true, method(), 0);
  Node *methodOop_node = _gvn.transform( new ConPNode(method_type) );
  Node *receiver_node  = (method() && !method()->is_static()) // IF  (virtual call)
    ? map()->in(TypeFunc::Parms)                              // THEN 'this' pointer, receiver,
    : null();                                                 // ELSE NULL

  kill_dead_locals();
  make_slow_call( call_type, call_address, NULL, control(), methodOop_node, receiver_node );
}

//------------------------------make_jvmpi_method_exit-------------------------
// JVMPI -- record entry to a method if compiled while JVMPI is turned on
void GraphKit::make_jvmpi_method_exit(ciMethod* method) {
  const TypeFunc *call_type    = OptoRuntime::jvmpi_method_exit_Type();
  address         call_address = OptoRuntime::jvmpi_method_exit_Java(); // CAST_FROM_FN_PTR(address, SharedRuntime::jvmpi_method_exit); // OptoRuntime::jvmpi_method_exit_Java();
  const char     *call_name    = "jvmpi_method_exit"; // OptoRuntime::stub_name( call_address );
  // assert triggers on exception exits with other BCIs
  // assert(bci() == InvocationEntryBci, "must be outside all blocks");
  const TypeInstPtr* method_type = TypeInstPtr::make(TypePtr::Constant, method->klass(), true, method, 0);
  Node *method_node = _gvn.transform( new ConPNode(method_type) );

  kill_dead_locals();
  make_slow_call( call_type, call_address, NULL, control(), method_node, null() );
}

//=============================================================================
//------------------------------do_checkcast-----------------------------------
void Parse::do_checkcast() {
  bool will_link;
  ciKlass* klass = iter().get_klass(will_link);

  Node *obj = peek();

  // Throw uncommon trap if class is not loaded or the value we are casting
  // _from_ is not loaded, and value is not null.  If the value _is_ NULL,
  // then the checkcast does nothing.
  const TypeInstPtr *tp = _gvn.type(obj)->isa_instptr();
  if (!will_link || (tp && !tp->is_loaded())) {
    if (C->log() != NULL) {
      if (!will_link) {
        C->log()->elem("assert_null reason='checkcast' klass='%d'",
                       C->log()->identify(klass));
      }
      if (tp && !tp->is_loaded()) {
        // %%% Cannot happen?
        C->log()->elem("assert_null reason='checkcast source' klass='%d'",
                       C->log()->identify(tp->klass()));
      }
    }
    do_null_assert(obj, T_OBJECT);
    assert( stopped() || _gvn.type(peek())->higher_equal(TypePtr::NULL_PTR), "what's left behind is null" );
    if (!stopped()) {
      profile_null_checkcast();
    }
    return;
  }

  Node *res = gen_checkcast(obj, makecon(TypeKlassPtr::make(klass)) );

  // Pop from stack AFTER gen_checkcast because it can uncommon trap and
  // the debug info has to be correct.
  pop();
  push(res);
}


//------------------------------do_instanceof----------------------------------
void Parse::do_instanceof() {
  if (stopped())  return;
  // We would like to return false if class is not loaded, emitting a
  // dependency, but Java requires instanceof to load its operand.

  // Throw uncommon trap if class is not loaded
  bool will_link;
  ciKlass* klass = iter().get_klass(will_link);

  if (!will_link) {
    if (C->log() != NULL) {
      C->log()->elem("assert_null reason='instanceof' klass='%d'",
                     C->log()->identify(klass));
    }
    do_null_assert(peek(), T_OBJECT);
    assert( stopped() || _gvn.type(peek())->higher_equal(TypePtr::NULL_PTR), "what's left behind is null" );
    if (!stopped()) {
      // The object is now known to be null.
      // Shortcut the effect of gen_instanceof and return "false" directly.
      pop();                   // pop the null
      push(_gvn.intcon(0));    // push false answer
    }
    return;
  }

  // Push the bool result back on stack
  push( gen_instanceof( pop(), makecon(TypeKlassPtr::make(klass)) ) );
}

//------------------------------array_store_check------------------------------
// pull array from stack and check that the store is valid
void Parse::array_store_check() {

  // Shorthand access to array store elements
  Node *obj = stack(_sp-1);
  Node *idx = stack(_sp-2);
  Node *ary = stack(_sp-3);

  // Extract the array klass type
  int klass_offset = oopDesc::klass_offset_in_bytes();
  Node* p = basic_plus_adr( ary, ary, klass_offset );
  // p's type is array-of-OOPS plus klass_offset
  Node* array_klass = _gvn.transform(new (3) LoadKlassNode(0, memory(TypeInstPtr::KLASS), p, TypeInstPtr::KLASS));
  // Get the array klass
  const TypeKlassPtr *tak = _gvn.type(array_klass)->is_klassptr();

  // array_klass's type is generally INexact array-of-oop.  Heroically
  // cast the array klass to EXACT array and uncommon-trap if the cast
  // fails.
  bool always_see_exact_class = false;
  if (MonomorphicArrayCheck
      && !too_many_traps(Deoptimization::Reason_array_check)) {
    always_see_exact_class = true;
    // (If no MDO at all, hope for the best, until a trap actually occurs.)
  }

  // Is the array klass is exactly its defined type?
  if (always_see_exact_class && !tak->klass_is_exact()) {
    // Make a constant out of the inexact array klass
    const TypeKlassPtr *extak = tak->cast_to_exactness(true)->is_klassptr();
    Node* con = makecon(extak);
    Node* cmp = _gvn.transform(new (3) CmpPNode( array_klass, con ));
    Node* bol = _gvn.transform(new (2) BoolNode( cmp, BoolTest::eq ));
    Node* ctrl= control();
    { BuildCutout unless(this, bol, PROB_MAX);
      uncommon_trap(Deoptimization::Reason_array_check,
                    Deoptimization::Action_maybe_recompile,
                    tak->klass());
    }
    if (stopped()) {          // MUST uncommon-trap?
      set_control(ctrl);      // Then Don't Do It, just fall into the normal checking
    } else {                  // Cast array klass to exactness: 
      // Use the exact constant value we know it is.
      replace_in_map(array_klass,con);
      CompileLog* log = C->log();
      if (log != NULL) {
        log->elem("cast_up reason='monomorphic_array' from='%d' to='(exact)'",
                  log->identify(tak->klass()));
      }
      array_klass = con;      // Use cast value moving forward
    }
  }

  // Come here for polymorphic array klasses

  // Extract the array element class
  int element_klass_offset = objArrayKlass::element_klass_offset_in_bytes() + sizeof(oopDesc);
  Node *p2 = basic_plus_adr(array_klass, array_klass, element_klass_offset);
  Node *a_e_klass = _gvn.transform(new (3) LoadKlassNode(0, memory(tak), p2, tak));

  // Check (the hard way) and throw if not a subklass.
  // Result is ignored, we just need the CFG effects.
  gen_checkcast( obj, a_e_klass );
}


//------------------------------do_new-----------------------------------------
void Parse::do_new() {
  kill_dead_locals();

  // The allocator will coalesce int->oop copies away.  See comment in
  // coalesce.cpp about how this works.  It depends critically on the exact
  // code shape produced here, so if you are changing this code shape
  // make sure the GC info for the heap-top is correct in and around the
  // slow-path call.

  bool will_link;
  ciInstanceKlass* klass = iter().get_klass(will_link)->as_instance_klass();
  assert(will_link, "_new: typeflow responsibility");

  // Should initialize, or throw an InstantiationError?
  if (!klass->is_initialized() ||
      klass->is_abstract() || klass->is_interface() ||
      klass->name() == ciSymbol::java_lang_Class()) {
    uncommon_trap(Deoptimization::Reason_uninitialized,
                  Deoptimization::Action_reinterpret,
                  klass);
    return;
  }

  Node* obj = new_instance(klass);

  // Push resultant oop onto stack
  push(obj);
}

#ifndef PRODUCT
//------------------------------dump_map_adr_mem-------------------------------
// Debug dump of the mapping from address types to MergeMemNode indices.
void Parse::dump_map_adr_mem() const {
  tty->print_cr("--- Mapping from address types to memory Nodes ---");
  MergeMemNode *mem = map() == NULL ? NULL : map()->memory()->is_MergeMem();
  for (uint i = 0; i < (uint)C->num_alias_types(); i++) {
    C->alias_type(i)->print_on(tty);
    tty->print("\t");
    // Node mapping, if any
    if (mem && i < mem->req() && mem->in(i) && mem->in(i) != mem->empty_memory()) {
      mem->in(i)->dump();
    } else {
      tty->cr();
    }
  }
}

#endif


//=============================================================================
//
// parser methods for profiling


//----------------------test_counter_against_threshold ------------------------
void Parse::test_counter_against_threshold(Node* cnt, int limit) {
  // Test the counter against the limit and uncommon trap if greater.

  // This code is largely copied from the range check code in 
  // array_addressing()
  
  // Test invocation count vs threshold
  Node *threshold = makecon(TypeInt::make(limit));
  Node *chk   = _gvn.transform( new (3) CmpUNode( cnt, threshold) );
  BoolTest::mask btest = BoolTest::lt;
  Node *tst   = _gvn.transform( new (2) BoolNode( chk, btest) );
  // Branch to failure if threshold exceeded
  { BuildCutout unless(this, tst, PROB_ALWAYS);
    uncommon_trap(Deoptimization::Reason_age,
                  Deoptimization::Action_maybe_recompile);
  }
}

//----------------------increment_and_test_invocation_counter-------------------
void Parse::increment_and_test_invocation_counter(int limit) {
  if (!count_invocations()) return;

  // Get the methodOop node.
  const TypePtr* adr_type = TypeOopPtr::make_from_constant(method());
  Node *methodOop_node = makecon(adr_type);

  // Load the interpreter_invocation_counter from the methodOop.
  int offset = methodOopDesc::interpreter_invocation_counter_offset_in_bytes();
  Node* adr_node = basic_plus_adr(methodOop_node, methodOop_node, offset);
  Node* cnt = make_load(NULL, adr_node, TypeInt::INT, T_INT, adr_type);

  test_counter_against_threshold(cnt, limit);

  // Add one to the counter and store
  Node* incr = _gvn.transform(new (3) AddINode(cnt, _gvn.intcon(1)));
  store_to_memory( NULL, adr_node, incr, T_INT, adr_type );
}

//----------------------------method_data_addressing--------------------------- 
Node* Parse::method_data_addressing(ciMethodData* md, ciProfileData* data, ByteSize counter_offset, Node* idx, uint stride) { 
  // Get offset within methodDataOop of the data array
  ByteSize data_offset = methodDataOopDesc::data_offset();
  
  // Get cell offset of the ProfileData within data array
  int cell_offset = md->dp_to_di(data->dp());

  // Add in counter_offset, the # of bytes into the ProfileData of counter or flag
  int offset = in_bytes(data_offset) + cell_offset + in_bytes(counter_offset);

  const TypePtr* adr_type = TypeOopPtr::make_from_constant(md); 
  Node* mdo = makecon(adr_type); 
  Node* ptr = basic_plus_adr(mdo, mdo, offset); 
 
  if (stride != 0) { 
    Node* str = _gvn.MakeConX(stride); 
    Node* scale = _gvn.transform( new (3) MulXNode( idx, str ) ); 
    ptr   = _gvn.transform( new (4) AddPNode( mdo, ptr, scale ) ); 
  }

  return ptr;
}

//--------------------------increment_md_counter_at----------------------------
void Parse::increment_md_counter_at(ciMethodData* md, ciProfileData* data, ByteSize counter_offset, Node* idx, uint stride) {
  Node* adr_node = method_data_addressing(md, data, counter_offset, idx, stride);

  const TypePtr* adr_type = _gvn.type(adr_node)->is_ptr();
  Node* cnt  = make_load(NULL, adr_node, TypeInt::INT, T_INT, adr_type);
  Node* incr = _gvn.transform(new (3) AddINode(cnt, _gvn.intcon(DataLayout::counter_increment)));
  store_to_memory(NULL, adr_node, incr, T_INT, adr_type );
}

//--------------------------test_for_osr_md_counter_at-------------------------
void Parse::test_for_osr_md_counter_at(ciMethodData* md, ciProfileData* data, ByteSize counter_offset, int limit) {
  Node* adr_node = method_data_addressing(md, data, counter_offset);

  const TypePtr* adr_type = _gvn.type(adr_node)->is_ptr();
  Node* cnt  = make_load(NULL, adr_node, TypeInt::INT, T_INT, adr_type);

  test_counter_against_threshold(cnt, limit);
}

//-------------------------------set_md_flag_at--------------------------------
void Parse::set_md_flag_at(ciMethodData* md, ciProfileData* data, int flag_constant) {
  Node* adr_node = method_data_addressing(md, data, in_ByteSize(0));

  const TypePtr* adr_type = _gvn.type(adr_node)->is_ptr();
  Node* flags = make_load(NULL, adr_node, TypeInt::INT, T_INT, adr_type);
  Node* incr = _gvn.transform(new (3) OrINode(flags, _gvn.intcon(flag_constant)));
  store_to_memory(NULL, adr_node, incr, T_INT, adr_type );
}

//----------------------------profile_taken_branch-----------------------------
void Parse::profile_taken_branch(int target_bci) {
  // This is a potential osr_site if we have a backedge.
  int cur_bci = bci();
  bool osr_site = 
    (target_bci <= cur_bci) && count_invocations() && UseOnStackReplacement;

  // If we are going to OSR, restart at the target bytecode.
  set_bci(target_bci);

  // To do: factor out the the limit calculations below. These duplicate
  // the similar limit calculations in the interpreter.

  if (method_data_update()) {
    ciMethodData* md = method()->method_data();
    assert(md != NULL, "expected valid ciMethodData");
    ciProfileData* data = md->bci_to_data(cur_bci);
    assert(data->is_JumpData(), "need JumpData for taken branch");
    increment_md_counter_at(md, data, JumpData::taken_offset());
    
    if (osr_site) {
      int limit = (CompileThreshold 
                   * (OnStackReplacePercentage - InterpreterProfilePercentage)) / 100;
      test_for_osr_md_counter_at(md, data, JumpData::taken_offset(), limit);
    }
  } else {
    // With method data update off, use the invocation counter to trigger an
    // OSR compilation, as done in the interpreter.
    if (osr_site) {
      int limit = (CompileThreshold * OnStackReplacePercentage) / 100;
      increment_and_test_invocation_counter(limit);
    }
  }

  // Restore the original bytecode.
  set_bci(cur_bci);
}

//--------------------------profile_not_taken_branch---------------------------
void Parse::profile_not_taken_branch() {
  if (!method_data_update()) return;

  ciMethodData* md = method()->method_data();
  assert(md != NULL, "expected valid ciMethodData");
  ciProfileData* data = md->bci_to_data(bci());
  assert(data->is_BranchData(), "need BranchData for not taken branch");
  increment_md_counter_at(md, data, BranchData::not_taken_offset());
}

//---------------------------------profile_call--------------------------------
void Parse::profile_call(Node* receiver) {
  if (!method_data_update()) return;

  profile_generic_call(); 
 
  switch (bc()) {
  case Bytecodes::_invokevirtual:
  case Bytecodes::_invokeinterface:
    profile_virtual_call(receiver);
    break;
  case Bytecodes::_invokestatic:  
  case Bytecodes::_invokespecial:
    break;
  default: fatal("unexpected call bytecode");
  }
}

//------------------------------profile_generic_call---------------------------
void Parse::profile_generic_call() {
  assert(method_data_update(), "must be generating profile code");

  ciMethodData* md = method()->method_data();
  assert(md != NULL, "expected valid ciMethodData");
  ciProfileData* data = md->bci_to_data(bci());
  assert(data->is_CounterData(), "need CounterData for not taken branch");
  increment_md_counter_at(md, data, CounterData::count_offset());
}

//------------------------------profile_virtual_call---------------------------
void Parse::profile_virtual_call(Node* receiver) {
  assert(method_data_update(), "must be generating profile code");

  // Skip if we aren't tracking receivers
  if (TypeProfileWidth < 1) return;

  ciMethodData* md = method()->method_data();
  assert(md != NULL, "expected valid ciMethodData");
  ciProfileData* data = md->bci_to_data(bci());
  assert(data->is_VirtualCallData(), "need VirtualCallData at call site");
  ciVirtualCallData* call_data = (ciVirtualCallData*)data->as_VirtualCallData();

  Node* method_data = method_data_addressing(md, call_data, in_ByteSize(0));

  // The following construction of the CallLeafNode is almost identical to
  // make_slow_call().  However, with make_slow_call(), the merge mem 
  // characteristics were causing incorrect anti-deps to be added.

  CallRuntimeNode *call = new CallLeafNode(OptoRuntime::profile_virtual_call_Type(), CAST_FROM_FN_PTR(address, OptoRuntime::profile_virtual_call_C), "profile_virtual_call_C");

  set_predefined_input_for_runtime_call(call);
  call->set_req( TypeFunc::Parms+0, method_data );
  call->set_req( TypeFunc::Parms+1, receiver );

  Node* c = _gvn.transform(call);

  set_predefined_output_for_runtime_call(c);
}

//---------------------------------profile_ret---------------------------------
void Parse::profile_ret(int target_bci) {
  if (!method_data_update()) return;

  // Skip if we aren't tracking ret targets
  if (TypeProfileWidth < 1) return;

  ciMethodData* md = method()->method_data();
  assert(md != NULL, "expected valid ciMethodData");
  ciProfileData* data = md->bci_to_data(bci());
  assert(data->is_RetData(), "need RetData for ret");
  ciRetData* ret_data = (ciRetData*)data->as_RetData();

  // Look for the target_bci is already in the table
  uint row;
  bool table_full = true;
  for (row = 0; row < ret_data->row_limit(); row++) {
    int key = ret_data->bci(row);
    table_full &= (key != RetData::no_bci);
    if (key == target_bci) break;
  }

  if (row >= ret_data->row_limit()) {
    // The target_bci was not found in the table.
    if (!table_full) {
      // XXX: Make slow call to update RetData
    }
    return;
  }

  // the target_bci is already in the table
  increment_md_counter_at(md, data, RetData::bci_count_offset(row));
}

//--------------------------profile_null_checkcast----------------------------
void Parse::profile_null_checkcast() {
  // Set the null-seen flag, done in conjunction with the usual null check. We
  // never unset the flag, so this is a one-way switch.
  if (!method_data_update()) return;

  ciMethodData* md = method()->method_data();
  assert(md != NULL, "expected valid ciMethodData");
  ciProfileData* data = md->bci_to_data(bci());
  assert(data->is_BitData(), "need BitData for checkcast");
  set_md_flag_at(md, data, BitData::null_flag_constant());
}

//-----------------------------profile_switch_case-----------------------------
void Parse::profile_switch_case(int table_index) {
  if (!method_data_update()) return;

  ciMethodData* md = method()->method_data();
  assert(md != NULL, "expected valid ciMethodData");

  ciProfileData* data = md->bci_to_data(bci());
  assert(data->is_MultiBranchData(), "need MultiBranchData for switch case");
  if (table_index >= 0) {
    increment_md_counter_at(md, data, MultiBranchData::case_count_offset(table_index));
  } else {
    increment_md_counter_at(md, data, MultiBranchData::default_count_offset());
  }
}


