#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)generateOptoStub.cpp	1.85 04/06/09 09:33:00 JVM"
#endif
//
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

#include "incls/_precompiled.incl"
#include "incls/_generateOptoStub.cpp.incl"

//-----------------------plus_adr----------------------------------------------
static Node *plus_adr( Node *ptr, intptr_t con ) {
  Node *top = Compile::current()->top();
  Node *connode = new (1) ConXNode( TypeX::make(con) );
  return new (4) AddPNode( top/*No base for non-oop ptrs*/, ptr, connode );
}

//------------------------gen_native_wrapper-----------------------------------
void GraphKit::gen_native_wrapper(ciMethod* method) {
  address native_entry = method->native_entry();
  assert(native_entry != NULL, "bad native entry");

  const char* rstr = method->name()->as_utf8();
  char* str = NEW_ARENA_ARRAY(C->comp_arena(), char, strlen(rstr) + 1);
  strcpy(str, rstr);

  gen_stub_or_native_wrapper(native_entry, str, method, 
                 0, true, true, false, false);
}

//-----------------------------gen_stub----------------------------------------
void GraphKit::gen_stub(address stub_function, const char* stub_name,
                        int is_fancy_jump, bool pass_tls, bool return_pc) {
  gen_stub_or_native_wrapper(stub_function, stub_name, (ciMethod*)NULL,
                             is_fancy_jump, false, false,
                             pass_tls, return_pc);
}


//--------------------gen_stub_or_native_wrapper-------------------------------
void GraphKit::gen_stub_or_native_wrapper(address C_function,
                                          const char *name, 
                                          ciMethod* method, 
                                          int is_fancy_jump,
                                          bool wrap_oops, 
                                          bool jni_sig, 
                                          bool pass_tls,
                                          bool return_pc) {
  ResourceMark rm;

  assert( !(jni_sig && pass_tls), "JNI calls use handles, do not pass oop thru TLS" );

  const TypeTuple *jdomain = C->tf()->domain();
  const TypeTuple *jrange  = C->tf()->range();

  // The procedure start
  StartNode* start = new StartNode(root(), jdomain);
  _gvn.set_type_bottom(start);

  // Make a map, with JVM state
  uint parm_cnt = jdomain->cnt();
  uint max_map = MAX2(2*parm_cnt+1, jrange->cnt());
  // %%% SynchronizationEntryBCI is redundant; use InvocationEntryBci in interfaces
  assert(SynchronizationEntryBCI == InvocationEntryBci, "");
  JVMState* jvms = method ? new JVMState(method, NULL) : new JVMState(0);
  jvms->set_bci(InvocationEntryBci);
  jvms->set_monoff(max_map); 
  jvms->set_endoff(max_map);
  {
    SafePointNode *map = new SafePointNode( max_map, jvms );
    jvms->set_map(map);
    set_jvms(jvms);
    assert(map == this->map(), "kit.map is set");
  }

  // Make up the parameters
  uint i;
  for( i = 0; i < parm_cnt; i++ ) 
    map()->set_req(i, _gvn.transform(new (1) ParmNode(start, i)));
  for( ; i<map()->req(); i++ )
    map()->set_req(i, top());      // For nicer debugging

  // GraphKit requires memory to be a MergeMemNode:
  set_all_memory(map()->memory());

  // Make a class-mirror constant oop Node for static natives.
  // Used to synchronize and as an argument to static natives.
  Node *mirror_node = NULL;
  Node *mirror_box = NULL;
  ciInstanceKlass *static_klass = NULL;
  if (method != NULL && method->is_static()) {
    static_klass = method->holder();
    ciInstance* mirror = static_klass->java_mirror();
    mirror_node = makecon(TypeInstPtr::make(mirror));
    assert(jni_sig, "sanity");
    // Static JNI calls pass the static_klass mirror.  They need it
    // boxed.  The box has to update the stack prior to setting
    // thread_in_native in order to be GC safe.
    mirror_box = _gvn.transform( new (2) BoxNode(mirror_node) );
  }

  // Call jvmpi before we wrap incoming arguments so we can
  // pass the receiver as an oop and not a handle.

  if (C->need_jvmpi_method_event()) {
    make_jvmpi_method_entry();
  }

  // Do we need to wrap incoming arguments in handles?
  // Generate code to put the oop on the stack and return its address.
  Node *unwrapped_this = map()->in(TypeFunc::Parms+0);
  Node** parm_boxes = NULL;
  uint num_parm_boxes = 0;
  if( wrap_oops ) {
    parm_boxes = NEW_RESOURCE_ARRAY(Node*, parm_cnt);  // generous, OK!
    for( i = TypeFunc::Parms; i < parm_cnt; i++ ) {
      if (jdomain->field_at(i)->isa_oop_ptr()) {
        // Replace parameter with handle
        // if oop is null, handle must be null !

        // Always create handle, so that oop is always set to correct value
        // even if the oop is null; otherwise we will have an uninitialized
        // oop location.  A BoxNode represents a reg-reg move where the
        // destination register is a stack-slot and the result is the address
        // of the stack slot.  The stack-slot must remain live past the native
        // call so via a special allocator hack it is alive over the entire
        // method.  That means the location must be initialized before any GC
        // is attempted.  The first possible GC is if we block locking a
        // synchronized method.  Next GC point is any instruction past setting
        // thread-in-native up till clearing thread-in-native.
        Node* box = _gvn.transform( new (2) BoxNode(map()->in(i)) );
        parm_boxes[num_parm_boxes++] = box;
        // The "obvious" thing to do here is to add a precedence edge
        // to keep the BoxNode from sinking too far.  For machines
        // with a conditional-move, the c-mov represents a normal use
        // along the main path and so unavoidably hoists the BoxNode.
        Node* cmp = _gvn.transform( new (3) CmpPNode(map()->in(i), null()) );
        Node* b = _gvn.transform( new (2) BoolNode(cmp, BoolTest::eq) );
        Node* cmov = _gvn.transform( CMoveNode::make(NULL, b, box, map()->in(i), TypeRawPtr::BOTTOM) );
        map()->set_req( i, cmov );
      }
    }
  }


  Node* flock_obj = NULL;
  Node* flock_obj_box = NULL;
  FastLockNode* flock = NULL;
  // Do we need to lock argument 0?  Is this a synchronized native call?
  if (method != NULL && method->flags().is_synchronized()) {
    // Lock either the class-mirror or the regular 'this' pointer
    flock_obj = static_klass ? mirror_node : unwrapped_this;
    flock_obj_box = static_klass ? mirror_box : parm_boxes[0];
    flock = shared_lock(flock_obj);
  }
  assert(C->sync() == (method && method->flags().is_synchronized() ? 1 : 0), "sync slots counted right");

  // Get base of thread-local storage area
  Node* thread = _gvn.transform( new (1) ThreadLocalNode() );

  const int NoAlias = Compile::AliasIdxBot;

  // reset handle block
  if( jni_sig ) {
    Node* adr_hb = basic_plus_adr(top(), thread, in_bytes(JavaThread::active_handles_offset()));
    Node* aho = make_load(NULL, adr_hb, TypeRawPtr::BOTTOM, T_ADDRESS, NoAlias);
    Node* adr_to = basic_plus_adr(top(), aho, JNIHandleBlock::top_offset_in_bytes() );
    store_to_memory(NULL, adr_to, null(), T_ADDRESS, NoAlias);
  }


  Node* pc_node = _gvn.transform( new (1) LoadPCNode(NULL) );
  _gvn.set_type(pc_node, TypeRawPtr::BOTTOM); // %%% LoadPCNode::bottom_type is bogus
  Node* adr_last_Java_pc = basic_plus_adr(top(), 
					    thread, 
					    in_bytes(JavaThread::frame_anchor_offset()) +
					    in_bytes(JavaFrameAnchor::last_Java_pc_offset()));
  if( jni_sig ) {
    store_to_memory(NULL, adr_last_Java_pc, pc_node, T_ADDRESS, NoAlias);
  }

  Node* adr_flags = basic_plus_adr(top(), 
				   thread, 
				   in_bytes(JavaThread::frame_anchor_offset()) +
				   in_bytes(JavaFrameAnchor::flags_offset()));


  // Drop in the last_Java_sp.  last_Java_fp is not touched.
  // Done for both JNI and runtime calls.
  // Always do this after the other "last_Java_frame" fields are set since
  // as soon as last_Java_sp != NULL the has_last_Java_frame is true and
  // users will look at the other fields.
  //
  Node *adr_sp = basic_plus_adr(top(), thread, in_bytes(JavaThread::last_Java_sp_offset()));
#ifndef IA64
  Node *last_sp = basic_plus_adr(top(), frameptr(), STACK_BIAS);
  store_to_memory(NULL, adr_sp, last_sp, T_ADDRESS, NoAlias);
#endif

  // Set _thread_in_native
  // The order of stores into TLS is critical!  Setting _thread_in_native MUST
  // be last, because a GC is allowed at any time after setting it and the GC
  // will require last_Java_pc and last_Java_sp.
  Node* adr_state = basic_plus_adr(top(), thread, in_bytes(JavaThread::thread_state_offset()));
  if( jni_sig ) {    
#ifndef IA64
    store_to_memory(NULL, adr_state, intcon(_thread_in_native), T_INT, NoAlias);
#endif
    Node* memory = this->memory(NoAlias);
    // Add a slew of precedence edges 
    if( mirror_box ) 
      memory->add_prec(mirror_box);
    for (i = 0; i < num_parm_boxes; i++)
      memory->add_prec(parm_boxes[i]);
  }

  //-----------------------------
  // Compute signature for C call.  Varies from the Java signature!
  const Type **fields = TypeTuple::fields(2*parm_cnt+2);
  uint cnt = TypeFunc::Parms;
  if( jni_sig ) {               // JNI style call or not?
    i = TypeFunc::Parms;
    // NOTE: All calls get the JNIEnv as an extra leading argument.
    // Static calls get the class oop where virtual calls get the reciever
    fields[cnt++] = TypeRawPtr::BOTTOM; // JNI Env pointer
    fields[cnt++] = (static_klass != NULL)
      ? TypeRawPtr::BOTTOM      // klass mirror handle
      : jdomain->field_at(i++); // Else copy reciever type
    // Copy the remaining fields.  
    for( ciSignatureStream ss(method->signature()); !ss.at_return_type(); ss.next()) {
      fields[cnt++] = jdomain->field_at(i++);
      BasicType bt = ss.type()->basic_type();
      if( bt == T_DOUBLE || bt == T_LONG ) {
        assert(jdomain->field_at(i) == Type::HALF, "");
        fields[cnt++] = jdomain->field_at(i++);
      }
#ifdef _LP64
      // The 64-bit Sparc ABI promotes all subword ints to longs
      if( jdomain->field_at(i-1)->isa_int() ) {
        fields[cnt-1] = TypeLong::LONG; 
        fields[cnt++] = Type::HALF; // halve is not part of any interpreter state as it goes to native C code
      }
#endif
    }

  } else {
    // The C routines gets the base of thread-local storage passed in as an
    // extra argument.  Not all calls need it, but its cheap to add here.
    for( ; cnt<parm_cnt; cnt++ )
      fields[cnt] = jdomain->field_at(cnt);
    fields[cnt++] = TypeRawPtr::BOTTOM; // Thread-local storage
    // Also pass in the caller's PC, if asked for.
    if( return_pc )
      fields[cnt++] = TypeRawPtr::BOTTOM; // Return PC
  }
  const TypeTuple* domain = TypeTuple::make(cnt,fields);
  // The C routine we are about to call cannot return an oop; it can block on
  // exit and a GC will trash the oop while it sits in C-land.  Instead, we
  // return the oop through TLS for runtime calls and use a handle for JNI
  // calls.  Also, C routines returning integer subword values leave the high
  // order bits dirty; these must be cleaned up by explicit sign extension.
  const Type* retval = (jrange->cnt() == TypeFunc::Parms) ? Type::TOP : jrange->field_at(TypeFunc::Parms);
  // Make a private copy of jrange->fields();
  const Type **rfields = TypeTuple::fields(jrange->cnt() - TypeFunc::Parms);
  // Fixup oop returns
  int retval_ptr = retval->isa_oop_ptr();
  if( retval_ptr ) {
    if( jni_sig ) {             // JNI unboxes returned result
      rfields[TypeFunc::Parms] = TypeRawPtr::BOTTOM; // return Handle
    } else {                    // Else C-land signature is void or address
      assert( pass_tls, "Oop must be returned thru TLS" );
      // Fancy-jumps return address; others return void
      rfields[TypeFunc::Parms] = is_fancy_jump ? TypeRawPtr::BOTTOM : Type::TOP;
    }

  } else if( retval->isa_int() ) { // Returning any integer subtype?
    // "Fatten" byte, char & short return types to 'int' to show that
    // the native C code can return values with junk high order bits.
    // We'll sign-extend it below later.
    rfields[TypeFunc::Parms] = TypeInt::INT; // It's "dirty" and needs sign-ext

  } else if( jrange->cnt() >= TypeFunc::Parms+1 ) { // Else copy other types
    rfields[TypeFunc::Parms] = jrange->field_at(TypeFunc::Parms);
    if( jrange->cnt() == TypeFunc::Parms+2 ) 
      rfields[TypeFunc::Parms+1] = jrange->field_at(TypeFunc::Parms+1);
  }
  const TypeTuple* range = TypeTuple::make(jrange->cnt(),rfields);

  // Final C signature
  const TypeFunc *c_sig = TypeFunc::make(domain,range);

  //-----------------------------
  // Make the call node
  CallRuntimeNode *call = jni_sig 
    ? new CallNativeNode ( c_sig, C_function, name )
    : new CallRuntimeNode( c_sig, C_function, name );
  //-----------------------------

  // Fix-up the debug info for the call
  call->set_jvms( method ? new JVMState(method, NULL) : new JVMState(0) );
  call->jvms()->set_bci(0);
  call->jvms()->set_offsets(cnt);
  
  // Set fixed predefined input arguments
  cnt = 0;
  for( i=0; i<TypeFunc::Parms; i++ )
    call->set_req( cnt++, map()->in(i) );
  // A little too aggressive on the parm copy; return address is not an input
  call->set_req(TypeFunc::ReturnAdr, top());
  // Add JNI required arguments
  if( jni_sig ) {               // JNI style call or not?
    Node *box = basic_plus_adr(top(), thread, in_bytes(JavaThread::jni_environment_offset()));
    call->set_req(cnt++, box ); // Pass handle to JNIEnv
    if( static_klass )          // Pass klass mirror handle for statics
      call->set_req(cnt++, mirror_box );
    for( ; i<parm_cnt; i++ ) {  // Regular input arguments
#ifdef _LP64
      if( jdomain->field_at(i)->isa_int() ) {
        // LP64 promotes ints to longs
        Node* promote = _gvn.transform( new (2) ConvI2LNode(map()->in(i)) );
        call->set_req( cnt++, promote );
        call->set_req( cnt++, top() );
      } else                    // Normal argument copy
#endif
        call->set_req( cnt++, map()->in(i) );
    }
  } else {
    for( ; i<parm_cnt; i++ )    // Regular input arguments
      call->set_req( cnt++, map()->in(i) );
  }
  if( !jni_sig ) {              // Runtime calls get the extra thread parm
    call->set_req( cnt++, thread );
    if( return_pc )             // Return PC, if asked for
      call->set_req( cnt++, returnadr() );
  }
  _gvn.transform_no_reclaim(call);


  //-----------------------------
  // Now set up the return results
  set_control( _gvn.transform( new (1) ProjNode(call,TypeFunc::Control)) );
  set_i_o(     _gvn.transform( new (1) ProjNode(call,TypeFunc::I_O    )) );
  set_all_memory_call(call);
  if (range->cnt() > TypeFunc::Parms) {
    Node* retnode = _gvn.transform( new (1) ProjNode(call,TypeFunc::Parms) );
    // C-land is allowed to return sub-word values.  Convert to integer type.
    assert( retval != Type::TOP, "" );
    if (retval == TypeInt::BOOL) {
      retnode = _gvn.transform( new (3) AndINode(retnode, intcon(0xFF)) );
    } else if (retval == TypeInt::CHAR) {
      retnode = _gvn.transform( new (3) AndINode(retnode, intcon(0xFFFF)) );
    } else if (retval == TypeInt::BYTE) {
      retnode = _gvn.transform( new (3) LShiftINode(retnode, intcon(24)) );
      retnode = _gvn.transform( new (3) RShiftINode(retnode, intcon(24)) );
    } else if (retval == TypeInt::SHORT) {
      retnode = _gvn.transform( new (3) LShiftINode(retnode, intcon(16)) );
      retnode = _gvn.transform( new (3) RShiftINode(retnode, intcon(16)) );
    }
    map()->set_req( TypeFunc::Parms, retnode );
  }

  //-----------------------------

  if( jni_sig ) {    

    // Mark thread in transition
    store_to_memory(NULL, adr_state, intcon(_thread_in_native_trans), T_INT, NoAlias);
    if( os::is_MP() ) insert_mem_bar( new MemBarVolatileNode() );


    Node* fast_mem = merged_memory();
    Node* fast_io = i_o();

    // If a safepoint operation is in progress then we have to block
    // before entering Java code again
    // Check (SafepointSynchronize::_state != 0)
    Node* adr_as = makecon(TypeRawPtr::make( SafepointSynchronize::address_of_state() ));
    Node* sstate = make_load(NULL, adr_as, TypeInt::INT, T_INT, NoAlias);

    // Block when ((SafepointSynchronize::_state != 0) || (current_thread->_suspend_flags != 0))
    // implement as ((SafepointSynchronize::_state | current_thread->_suspend_flags) != 0)
    // since this avoids additional merge code and in the expected case both loads will be necessary anyway
    assert(SafepointSynchronize::_not_synchronized == 0, "implementation still correct if states are 2^n");
    Node* adr_sf = basic_plus_adr(top(), thread, in_bytes(JavaThread::suspend_flags_offset()));
    Node* sflag  = make_load(NULL, adr_sf, TypeInt::INT, T_INT, NoAlias);
    sstate = _gvn.transform( new (3) OrINode(sstate, sflag) );

    Node* cmp = _gvn.transform( new (3) CmpINode(sstate, intcon(SafepointSynchronize::_not_synchronized)) );
    Node* b = _gvn.transform( new (2) BoolNode(cmp, BoolTest::ne) );
    IfNode* iff = create_and_map_if(control(), b, PROB_MIN, COUNT_UNKNOWN);
    Node* if_block    = _gvn.transform( new (1) IfFalseNode(iff) );
    Node* if_no_block = _gvn.transform( new (1) IfTrueNode(iff)  );

    // Create signature for call to "block"
    const Type **fields = TypeTuple::fields(1);
    fields[TypeFunc::Parms+0] = TypeRawPtr::BOTTOM; // Thread
    const TypeTuple *domain = TypeTuple::make(TypeFunc::Parms+1,fields);
    fields = TypeTuple::fields(1);
    fields[TypeFunc::Parms+0] = Type::TOP;
    const TypeTuple *range = TypeTuple::make(TypeFunc::Parms+1,fields);
    // Make a call to the runtime to "block"
    CallNode *block = new CallRuntimeNode(TypeFunc::make(domain,range), 
                                          CAST_FROM_FN_PTR(address, JavaThread::check_safepoint_and_suspend_for_native_trans),
                                          "JavaThread::check_safepoint_and_suspend_for_native_trans" );
    // The debug info is the only real input to this call.

    // Create debug-info for call, so ScopeDesc's are getting properly
    // recorded.  This code should be factored somehow.
    block->set_jvms( method ? new JVMState(method, NULL) : new JVMState(0) );
    block->jvms()->set_bci(0);
    int arg_limit = block->tf()->domain()->cnt();
    block->jvms()->set_offsets(arg_limit);
    
    // Set up inputs to call to "block"
    set_control(if_no_block);
    set_predefined_input_for_runtime_call(block);
    block->set_req(TypeFunc::Parms+0, thread);
    _gvn.transform_no_reclaim(block);
    set_predefined_output_for_runtime_call(block, NULL);

    // Merge control flow post call
    RegionNode *region = new RegionNode(3);
    region->set_req( 1, control() );
    region->set_req( 2, if_block );
    set_control( _gvn.transform(region) );

    // Setup merge point post the call
    PhiNode* iophi = PhiNode::make(region, fast_io);
    iophi->set_req(1, _gvn.transform( new (1) ProjNode(block,TypeFunc::I_O) ));
    set_i_o(_gvn.transform(iophi));
    merge_fast_memory(fast_mem, region, 1);

    //-----------------------------
    // Mark thread in Java code
#ifdef IA64
    if( os::is_MP() ) insert_mem_bar( new MemBarReleaseNode() );
#endif
    store_to_memory(NULL, adr_state, intcon(_thread_in_Java), T_INT, NoAlias);
#ifdef IA64
    if( os::is_MP() ) insert_mem_bar( new MemBarVolatileNode() );
#endif

    //-----------------------------
    // JNI calls return boxed results.  Unbox.
    if( retval_ptr ) {
      // Test oop for NULL at runtime; unbox handle if not null
      Node* cmp = _gvn.transform( new (3) CmpPNode(map()->in(TypeFunc::Parms), null()) );
      Node* b = _gvn.transform( new (2) BoolNode(cmp, BoolTest::ne) );
      IfNode *iff = create_and_map_if(control(), b, PROB_STATIC_INFREQUENT, COUNT_UNKNOWN);
      Node* if_null     = _gvn.transform( new (1) IfFalseNode(iff) );
      Node* if_not_null = _gvn.transform( new (1) IfTrueNode(iff)  );
      RegionNode* region = new RegionNode(3);
      region->set_req(1, if_not_null );
      region->set_req(2, if_null );
      PhiNode* phi = new PhiNode(region, retval);
      Node* unbox = make_load(if_not_null, map()->in(TypeFunc::Parms), retval, T_ADDRESS, NoAlias);
      phi->set_req(1,unbox);
      phi->set_req(2,map()->in(TypeFunc::Parms));
      map()->set_req(TypeFunc::Parms, _gvn.transform(phi));
      set_control( _gvn.transform(region) );
    }

    // JVMDI jframeIDs are invalidated on exit from native method.
    // JVMTI does not use jframeIDs, this whole mechanism must be removed when JVMDI is removed.
    if (JvmtiExport::must_purge_jvmdi_frames_on_native_exit() && jni_sig) { 
      // Make a call to the runtime to inform JVMTI that thread is leaving native code.
      // No debug info is sent to this leaf call.  See interpreter_<arch>.cpp.
      const TypeTuple* domain = TypeTuple::make(TypeFunc::Parms, TypeTuple::fields(0));
      const TypeTuple* range  = TypeTuple::make(TypeFunc::Parms, TypeTuple::fields(0));
      const TypeFunc* jvmti_call_type = TypeFunc::make(domain,range);
      make_slow_call(jvmti_call_type,
                     CAST_FROM_FN_PTR(address, JvmtiExport::thread_leaving_native_code),
                     "JvmtiExport::thread_leaving_native_code",
                     control(), NULL, NULL);
    }

  } // End of if JNI call

  //-----------------------------
  // Clear last_Java_sp
#ifdef IA64
  if( os::is_MP() ) insert_mem_bar( new MemBarReleaseNode() );
#endif

  store_to_memory(NULL, adr_sp, null(), T_ADDRESS, NoAlias);
#ifdef IA64
  if( os::is_MP() ) insert_mem_bar( new MemBarVolatileNode() );
#endif
  // Clear last_Java_pc and _flags
  store_to_memory(NULL, adr_last_Java_pc, null(), T_ADDRESS, NoAlias);
  store_to_memory(NULL, adr_flags, intcon(0), T_INT, NoAlias);
#ifdef IA64
  Node* adr_last_Java_fp = basic_plus_adr(top(), thread, in_bytes(JavaThread::last_Java_fp_offset()));
  if( os::is_MP() ) insert_mem_bar( new MemBarReleaseNode() );
  store_to_memory(NULL, adr_last_Java_fp,    null(),    T_ADDRESS, NoAlias);
#endif

  // For is-fancy-jump, the C-return value is also the branch target
  Node* target = map()->in(TypeFunc::Parms);
  // Runtime call returning oop in TLS?  Fetch it out
  if( pass_tls ) {
    Node* adr = basic_plus_adr(top(), thread, in_bytes(JavaThread::vm_result_offset()));
    Node* vm_result = make_load(NULL, adr, TypeOopPtr::BOTTOM, T_OBJECT, NoAlias);
    map()->set_req(TypeFunc::Parms, vm_result); // vm_result passed as result
    // clear thread-local-storage(tls)
    store_to_memory(NULL, adr, null(), T_ADDRESS, NoAlias);
  }

  //-----------------------------
  // Unlock
  if (flock != NULL) {
    // Reload the oop that we locked out of the box. On register windowed
    // machines the oop might be still live in a register but we can't
    // use it since there is a race between gc and returning from native
    // (see 4786868 and 4954584 ). Since we boxed it was a parameter to 
    // the native call it is live and valid in its box so we reload it 
    // from there after // the change to java (above).
    //
    // Since the box looks like raw memory the load we generate here 
    // will be sequenced after the store(s) to tls above (which are also raw)
    // and so the load should not float above the thread state change.

    Node* adr = basic_plus_adr(top(), flock_obj_box, 0);
    // Actually load the oop from the box with its original type. This
    // is really somewhat overkill since we could just load it with
    // TypeOopPtr::BOTTOM and not really lose any optimization chances.
    Node* flock_obj2 = make_load(control(), adr, flock_obj->Value(&_gvn), T_OBJECT);
    shared_unlock(flock->box_node(), flock_obj2);
  }
  if (C->need_jvmpi_method_event()) {
    make_jvmpi_method_exit(method);
  }

  //-----------------------------
  // check exception
  Node* adr = basic_plus_adr(top(), thread, in_bytes(Thread::pending_exception_offset()));
  Node* pending = make_load(NULL, adr, TypeOopPtr::BOTTOM, T_OBJECT, NoAlias);

  Node* exit_memory = reset_memory();

  Node* cmp = _gvn.transform( new (3) CmpPNode(pending, null()) );
  Node* bo  = _gvn.transform( new (2) BoolNode(cmp, BoolTest::ne) );
  IfNode   *iff = create_and_map_if(control(), bo, PROB_MIN, COUNT_UNKNOWN);

  Node* if_null     = _gvn.transform( new (1) IfFalseNode(iff) );
  Node* if_not_null = _gvn.transform( new (1) IfTrueNode(iff)  );

  assert (StubRoutines::forward_exception_entry() != NULL, "must be generated before");
  Node *exc_target = makecon(TypeRawPtr::make( StubRoutines::forward_exception_entry() ));
  Node *to_exc = new TailCallNode(if_not_null,
                                  i_o(),
                                  exit_memory,
                                  frameptr(),
                                  returnadr(),
                                  exc_target, null());
  root()->add_req(_gvn.transform(to_exc));  // bind to root to keep live
  C->init_start(start);

  //-----------------------------
  // If this is a normal subroutine return, issue the return and be done.
  Node *ret;
  switch( is_fancy_jump ) {
  case 0:                       // Make a return instruction
    // Return to caller, free any space for return address
    ret = new ReturnNode(if_null,
                         i_o(),
                         exit_memory,
                         frameptr(),
                         returnadr());
    if (C->tf()->range()->cnt() > TypeFunc::Parms)
      ret->add_req( map()->in(TypeFunc::Parms) );
    break;
  case 1:    // This is a fancy tail-call jump.  Jump to computed address.
    // Jump to new callee; leave old return address alone.
    ret = new TailCallNode(if_null,
                           i_o(),
                           exit_memory,
                           frameptr(),
                           returnadr(),
                           target, map()->in(TypeFunc::Parms));
    break;
  case 2:                       // Pop return address & jump
    // Throw away old return address; jump to new computed address
    //assert(C_function == CAST_FROM_FN_PTR(address, OptoRuntime::rethrow_C), "fancy_jump==2 only for rethrow");
    ret = new TailJumpNode(if_null,
                           i_o(),
                           exit_memory,
                           frameptr(),
                           target, map()->in(TypeFunc::Parms));
    break;
  default:
    ShouldNotReachHere();
  }
  root()->add_req(_gvn.transform(ret));
}

//------------------------------Generate--------------------------------------
// Create a Java-calling-convention-compatible piece of code.  This code calls
// C++ function with the same signature but using the C calling convention.
// This code marshals the arguments between Java and C.  It knows that C
// returns oops in thread local storage.  It knows to set the last_Java_sp so
// the stack crawlers can get their starting point.  It can box oops for 
// native routines (can box for runtime calls as well, but they generally do
// not need it).

// If 'is_fancy_jump' is set, then this code makes a fancy jump, instead of
// being a true subroutine; i.e. I do the classic tail-call optimization also
// known as a interprocedural-jump.  


//------------------------------Generate_Compiled_To_Interpreter_Graph--------
// From the TypeFunc signature, generate code to pass arguments 
// from the Java calling convention to the Interpreter's calling convention.
// 
// Compiled code always passes the MethodKlassHolder
void Compile::Generate_Compiled_To_Interpreter_Graph( const TypeFunc *java_sig, address interpreter_entry ) {
  uint       i;
  ciMethod* method   = NULL;

  // Expand signature to include inline_cached_klass and method_oop
  // Caller provides MethodKlassHolder, mkh.
  // at entry point, expand mkh into inline-cache-class and method-oop
  const TypeTuple *domain   = StartC2INode::c2i_domain(java_sig->domain());
  const TypeFunc *start_sig = TypeFunc::make(domain,   java_sig->range());

  uint j_parm_cnt           = java_sig->domain()->cnt();
  uint start_parm_cnt       = domain->cnt();

  // The procedure start
  StartNode *start = new StartC2INode(root(), domain);

  // Create projections from start for incoming parameters in signature
  Node *map = new Node(MAX2(start_parm_cnt, java_sig->range()->cnt()));
  for( i=0; i<start_parm_cnt; i++ ) 
    map->set_req(i,new (1) ParmNode(start,i));

  // ---------- call interpreter ----------

  // Parameter Copy, from compiled convention to interpreter convention,
  // is done by the register allocator using the RegMask assignment
  // for incoming and outgoing parameters

  // Make the call node
  const TypeFunc  *call_sig  = start_sig;
  uint interp_parm_cnt       = start_parm_cnt;

  // Make the call node
  CallNode *call = new CallInterpreterNode( call_sig, interpreter_entry );

  // Fix-up the debug info for the call
  call->set_jvms( method ? new JVMState(method, NULL) : new JVMState(0) );
  call->jvms()->set_offsets( interp_parm_cnt );

  // Set fixed predefined input arguments
  for( i=0; i<interp_parm_cnt; i++ )
    call->set_req( i, map->in(i) );
  // A little too aggressive on the parm copy; return address is not an input
  call->set_req(TypeFunc::ReturnAdr, top());

  // Now set up the return results
  map->set_req( TypeFunc::Control, new (1) ProjNode(call,TypeFunc::Control) );
  map->set_req( TypeFunc::I_O    , new (1) ProjNode(call,TypeFunc::I_O    ) );
  map->set_req( TypeFunc::Memory , new (1) ProjNode(call,TypeFunc::Memory ) );
  // Capture 0 or 1 return values.
  if (java_sig->range()->cnt() > TypeFunc::Parms) {
    map->set_req(TypeFunc::Parms, new (1) ProjNode(call,TypeFunc::Parms ) );
  }

  // ---------- return instruction ----------

  // Make a return instruction
  Node *ret = new ReturnNode(map->in(TypeFunc::Control),
                             map->in(TypeFunc::I_O),
                             map->in(TypeFunc::Memory),
                             map->in(TypeFunc::FramePtr),
                             map->in(TypeFunc::ReturnAdr));
  if (java_sig->range()->cnt() > TypeFunc::Parms)
    ret->add_req( map->in(TypeFunc::Parms) );
  root()->add_req(ret);       // Set compiler to new graph
  init_tf(start_sig);         // Set compiler to new type function
}


//------------------------------Generate_Interpreter_To_Compiled_Graph---------
// From the TypeFunc signature, generate code to pass arguments from the
// Interpreter's calling convention to the Compiler's convention.  
//
// The i2c adapter is effectively passed 2 arguments: the method_oop and a
// pointer to the parameters, passed var-args style.  All the interpreter's
// arguments are on the Java stack in memory.  This adapter-generator makes
// code to load up those arguments and put them in the Compiler's convention.
// This generally means handling mis-aligned double & long loads and putting
// things in registers or aligned on the stack.  The methodOop is passed as 
// extra argument tacked on the end; it's allocated to inline_cache_reg().
void Compile::Generate_Interpreter_To_Compiled_Graph(const TypeFunc *java_sig){
  int i;
  
  // Update routine's signature for all components of compiler
  const TypeTuple *jdomain = java_sig->domain();
  const TypeTuple *jrange  = java_sig->range ();

  // ---------- StartI2CNode and its parameters
  StartNode *start = new StartI2CNode(root(), TypeTuple::START_I2C);

  // Create projections from start for incoming parameters in signature
  int java_arg_cnt = jdomain->cnt();
  int call_arg_cnt = java_arg_cnt + 1; // Call also takes methodOop
  Node *map = new Node(call_arg_cnt);
  for( i=0; i<TypeFunc::Parms; i++ )
    map->set_req( i, new (1) ParmNode( start, i ) );
  Node *moop = new (1) ParmNode( start, TypeFunc::Parms+0 );
  Node *argp = new (1) ParmNode( start, TypeFunc::Parms+1 );

  // ---------- Load up all the arguments
  Node *mem = map->in(TypeFunc::Memory);

  for( i = TypeFunc::Parms; i < java_arg_cnt; i++ ) {
    BasicType bt = jdomain->field_at(i)->basic_type();
    // Arguments are laid out in reverse order, so we negate index i while
    // computing addresses.  The arg ptr actually points 1 word past the
    // first (last) argument.
    Node *adr = plus_adr( argp, (java_arg_cnt - i)*wordSize );

    // Very similar to LoadNode::make, except we handle un-aligned longs and
    // doubles on Sparc.  Intel can handle them just fine directly.
    Node *l;
    switch( bt ) {              // Signature is flattened
    case T_INT:     l = new (3) LoadINode( 0, mem, adr, TypeRawPtr::BOTTOM ); break;
    case T_FLOAT:   l = new (3) LoadFNode( 0, mem, adr, TypeRawPtr::BOTTOM ); break;
    case T_ADDRESS: 
    case T_OBJECT:  l = new (3) LoadPNode( 0, mem, adr, TypeRawPtr::BOTTOM, TypeInstPtr::BOTTOM ); break;
    case T_LONG:
    case T_DOUBLE: {
      // Since arguments are in reverse order, the argument address 'adr'
      // refers to the back half of the long/double.  Recompute adr.
      adr = plus_adr( argp, (java_arg_cnt - (i+1))*wordSize );
      if( Matcher::misaligned_doubles_ok ) {
        l = (bt == T_DOUBLE) 
          ? (Node*)new (3) LoadDNode( 0, mem, adr, TypeRawPtr::BOTTOM )
          : (Node*)new (3) LoadLNode( 0, mem, adr, TypeRawPtr::BOTTOM );
      } else {
        l = (bt == T_DOUBLE) 
          ? (Node*)new (3) LoadD_unalignedNode( 0, mem, adr, TypeRawPtr::BOTTOM )
          : (Node*)new (3) LoadL_unalignedNode( 0, mem, adr, TypeRawPtr::BOTTOM );
      }
      map->set_req(i,l);
      l = Compile::current()->top();
      i++;
      assert( jdomain->field_at(i) == Type::HALF, "missing half of long/double" );
      break;
    }
    default: ShouldNotReachHere();
    }
    map->set_req(i,l);
  }
  // Now tuck in the methodOop on the end
  map->set_req(java_arg_cnt, moop);

  // ---------- call compiled ----------

  // Parameter Copy, from compiled convention to interpreter convention,
  // is done by the register allocator using signatures

  // The compiler's signature is the same as the interpreter's with the
  // methodOop appended on the end.
  const Type **fields = TypeTuple::fields(call_arg_cnt);
  for( i=0; i<java_arg_cnt; i++ ) // Copy Java signature
    fields[i] = jdomain->field_at(i);
  fields[java_arg_cnt + 0] = TypeInstPtr::BOTTOM;  // append methodOop
  const TypeTuple *cdomain = TypeTuple::make( call_arg_cnt, fields );
  const TypeFunc *call_sig = TypeFunc::make(cdomain, jrange);

  // Make the call node
  CallNode *call = new CallCompiledJavaNode( call_sig, NULL );

  // Fix-up the debug info for the call
  call->set_jvms( new JVMState(0) );
  call->jvms()->set_offsets( call_arg_cnt );

  // Set fixed predefined input arguments
  for( i=0; i<call_arg_cnt; i++ )
    call->set_req( i, map->in(i) );
  // return address is not an input
  call->set_req(TypeFunc::ReturnAdr, top());

  // Now set up the return results
  map->set_req( TypeFunc::Control, new (1) ProjNode(call,TypeFunc::Control) );
  map->set_req( TypeFunc::I_O    , new (1) ProjNode(call,TypeFunc::I_O    ) );
  map->set_req( TypeFunc::Memory , new (1) ProjNode(call,TypeFunc::Memory ) );
  // Capture 0 or 1 return values.
  if (jrange->cnt() > TypeFunc::Parms)
    map->set_req( TypeFunc::Parms , new (1) ProjNode(call,TypeFunc::Parms ) );

  // ---------- return instruction ----------
  // Make a return instruction
  Node *ret = new ReturnNode(map->in(TypeFunc::Control),
                             map->in(TypeFunc::I_O),
                             map->in(TypeFunc::Memory),
                             map->in(TypeFunc::FramePtr),
                             map->in(TypeFunc::ReturnAdr));
  if (jrange->cnt() > TypeFunc::Parms) 
    ret->add_req( map->in(TypeFunc::Parms) );
  root()->add_req(ret);         // Set compiler to new graph

  const TypeFunc *sig = TypeFunc::make(TypeTuple::START_I2C, jrange);
  init_tf(sig);                 // Set compiler to new type function
  return;
}
