#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)deoptimization.cpp	1.251 04/03/30 16:53:35 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_deoptimization.cpp.incl"

bool DeoptimizationMarker::_is_active = false;

Deoptimization::UnrollBlock::UnrollBlock(int  size_of_deoptimized_frame,
                                         int  adapter_size,
					 int  new_adapter,
                                         int  caller_adjustment,
                                         int  number_of_frames,
                                         intptr_t* frame_sizes,
                                         address* frame_pcs,
                                         BasicType return_type) {
  _size_of_deoptimized_frame = size_of_deoptimized_frame;
  _adapter_size              = adapter_size;
  _new_adapter               = new_adapter;
  _caller_adjustment         = caller_adjustment;
  _number_of_frames          = number_of_frames;
  _frame_sizes               = frame_sizes;
  _frame_pcs                 = frame_pcs;
  _register_block            = NEW_C_HEAP_ARRAY(intptr_t, RegisterMap::reg_count * 2);
  _return_type               = return_type;
  // PD (x86 only)
  _counter_temp              = 0;
  _initial_fp                = 0;
  _unpack_kind               = 0;
  _sender_sp_temp            = 0;
}


Deoptimization::UnrollBlock::~UnrollBlock() {
  FREE_C_HEAP_ARRAY(intptr_t, _frame_sizes);
  FREE_C_HEAP_ARRAY(intptr_t, _frame_pcs);
  FREE_C_HEAP_ARRAY(intptr_t, _register_block);
}


intptr_t* Deoptimization::UnrollBlock::value_addr_at(int register_number) const {
  assert(register_number < RegisterMap::reg_count, "checking register number");
  return &_register_block[register_number * 2];
}



int Deoptimization::UnrollBlock::size_of_frames() const {
  // Acount first for the adjustment of the initial frame
  int result = _caller_adjustment;
  for (int index = 0; index < number_of_frames(); index++) {
    result += frame_sizes()[index];
  }
  return result;
}


void Deoptimization::UnrollBlock::print() {
  ttyLocker ttyl;
  tty->print_cr("UnrollBlock");
  tty->print_cr("  size_of_deoptimized_frame = %d", _size_of_deoptimized_frame);
  tty->print(   "  frame_sizes: ");
  for (int index = 0; index < number_of_frames(); index++) {
    tty->print("%d ", frame_sizes()[index]);
  }
  tty->cr();
}


JRT_LEAF(Deoptimization::UnrollBlock*, Deoptimization::fetch_unroll_info(JavaThread* thread))
  // It is actually ok to allocate handles in a leaf method. It causes no safepoints,
  // but makes the entry a little slower. There is however a little dance we have to
  // do in debug mode to get around the NoHandleMark code in the JRT_LEAF macro
  ResetNoHandleMark rnhm; // No-op in release/product versions  
  HandleMark hm;

  // fetch_unroll_info() is called at the beginning of the deoptimization
  // handler. Note this fact before we start generating temporary frames
  // that can confuse an asynchronous stack walker. This counter is
  // decremented at the end of unpack_frames().
  thread->inc_in_deopt_handler();

  return fetch_unroll_info_helper(thread);
JRT_END


// This is factored, since it is both called from a JRT_LEAF (deoptimization) and a JRT_ENTRY (uncommon_trap)
Deoptimization::UnrollBlock* Deoptimization::fetch_unroll_info_helper(JavaThread* thread) {

  // Note: there is a safepoint safety issue here. No matter whether we enter
  // via vanilla deopt or uncommon trap we MUST NOT stop at a safepoint once
  // the vframeArray is created. 
  //
  // In fixing bug 4992987 it would have been possible to optimize the
  // generation of the c2i adapter by placing the request in this method.
  // Generating the adapter may require a safepoint which would be safe
  // if the request comes before the vframeArray is created. This bug
  // was found late in the 1.5 release cycle and fixing it here was
  // deemed riskier than the chosen fix. 
  //
  // The advantage to fixing 4992987 here is the centralization of the code
  // and a possible performance advantage. Because c2i adapters are cached
  // by signature the performance advantage is highly debatable since we
  // rarely have to generate code for a new adapter.


  // Allocate our special deoptimization ResourceMark
  DeoptResourceMark* dmark = new DeoptResourceMark(thread);
  assert(thread->deopt_mark() == NULL, "Pending deopt!");
  thread->set_deopt_mark(dmark);

  frame stub_frame = thread->last_frame(); // Makes stack walkable as side effect

  // Until at_call is removed we must figure out if we deopted at_call or not.
  // Must be done before we create the registerMap for the vframeArray creation.
  RegisterMap rmap(thread, false);
  frame deoptee = stub_frame.sender(&rmap);
  CodeBlob* nm = (nmethod*) CodeCache::find_blob(deoptee.pc());

  assert(nm != NULL && nm->is_nmethod(), "Must be nmethod");
  if (SafepointPolling) {
    // SafepointPolling produces ambiguous pc's as far as at_call
    // not_at_call. Until at_call is removed and SafepointPolling
    // is fixed we must figure out if we are at at_call the hard
    // way.
    if (nm->is_at_poll_or_poll_return(deoptee.pc())) {
      thread->set_pc_not_at_call_for_frame(deoptee.id());
    }
  } else {
    relocInfo::relocType type = nm->reloc_type_for_address(deoptee.pc());
    if ( type == relocInfo::safepoint_type ) {
      thread->set_pc_not_at_call_for_frame(deoptee.id());
    }
  }

  RegisterMap map(thread, true);
  RegisterMap dummy_map(thread, false);
  // Now get the deoptee again with a valid map
  deoptee = stub_frame.sender(&map);

  // We are safepoint safe up to this call
  vframeArray* array = create_vframeArray(thread, deoptee, &map);

  // We are no longer safepoint safe. If a safepoint occurs from here on
  // out the java state residing in the vframeArray will be missed.
  No_Safepoint_Verifier no_safepoint;

  assert(thread->vframe_array_head() == NULL, "Pending deopt!");;
  thread->set_vframe_array_head(array);

  // Now that the vframeArray has been created if we have any deferred local writes
  // added by jvmti then we can free up that structure as the data is now in the
  // vframeArray

  if (thread->deferred_locals() != NULL) {
    GrowableArray<jvmtiDeferredLocalVariableSet*>* list = thread->deferred_locals();
    int i = 0;
    do {
      // Because of inlining we could have multiple vframes for a single frame
      // and several of the vframes could have deferred writes. Find them all.
      if (list->at(i)->id() == array->original().id()) {
	jvmtiDeferredLocalVariableSet* dlv = list->at(i);
	list->remove_at(i);
	// individual jvmtiDeferredLocalVariableSet are CHeapObj's
	delete dlv;
      } else {
	i++;
      }
    } while ( i < list->length() );
    if (list->length() == 0) {
      thread->set_deferred_locals(NULL);
      FreeHeap(list);
    }
    
  }

  // Compute the caller frame based on the sender sp of stub_frame and stored frame sizes info.
  CodeBlob* cb = CodeCache::find_blob(stub_frame.pc());
  // Verify we have the right vframeArray
  assert(cb->frame_size() >= 0, "Unexpected frame size");
  intptr_t* unpack_sp = stub_frame.sp() + cb->frame_size();

#ifdef ASSERT
  assert(cb->is_deoptimization_stub() || cb->is_uncommon_trap_stub(), "just checking");
#ifdef COMPILER1
#ifdef ASSERT
  if (array->unextended_sp() != unpack_sp) {
    tty->print_cr("array->unextended_sp() = "INTPTR_FORMAT "\n", array->unextended_sp());
    tty->print_cr("unpack_sp = " INTPTR_FORMAT "\n", unpack_sp);
    tty->print_cr("original pc = " INTPTR_FORMAT "\n", array->original_pc());
    nmethod* nm = (nmethod*)CodeCache::find_blob(array->original_pc());
    nm->print();
  }
#endif // ASSERT
#endif // COMPILER1
  Events::log("fetch unroll sp " INTPTR_FORMAT, unpack_sp);
#endif
  // This is a guarantee instead of an assert because if vframe doesn't match
  // we will unpack the wrong deoptimized frame and wind up in strange places
  // where it will be very difficult to figure out what went wrong. Better
  // to die an early death here than some very obscure death later when the
  // trail is cold.
  // Note: on ia64 this guarantee can be fooled by frames with no memory stack
  // in that it will fail to detect a problem when there is one. This needs
  // more work in tiger timeframe.
  guarantee(array->unextended_sp() == unpack_sp, "vframe_array_head must contain the vframeArray to unpack");

  int number_of_frames = array->frames();
  int need_new_adapter = 0;
#ifdef COMPILER2
  // If the caller of the deoptimized frame is compiled, a new C2I adapter has been 
  // allocated and must be included in the vframe count.
  // Otherwise if the caller was not a compiled frame then a now useless I2C or
  // OSR adapter frame will be popped.

  C2IAdapter* new_adapter = array->new_adapter();
  need_new_adapter = new_adapter != NULL ? 1 : 0;
  number_of_frames += need_new_adapter;
#endif

  // Compute the vframes' sizes.  Note that frame_sizes[] entries are ordered from outermost to innermost
  // virtual activation, which is the reverse of the elements in the vframes array.
  intptr_t* frame_sizes = NEW_C_HEAP_ARRAY(intptr_t, number_of_frames);
  // +1 because we always have an interpreter return address for the final slot.
  address* frame_pcs = NEW_C_HEAP_ARRAY(address, number_of_frames + 1);
  int callee_parameters = 0;
  int callee_locals = 0;
  int popframe_extra_args = 0;
  // Create an interpreter return address for the stub to use as its return
  // address so the skeletal frames are perfectly walkable
  frame_pcs[number_of_frames] = AbstractInterpreter::deopt_entry(vtos, 0);

  // PopFrame requires that the preserved incoming arguments from the recently-popped topmost
  // activation be put back on the expression stack of the caller for reexecution
  if (JvmtiExport::can_pop_frame() && thread->popframe_forcing_deopt_reexecution()) {
    popframe_extra_args = in_words(thread->popframe_preserved_args_size_in_words());
  }

  //
  // frame_sizes/frame_pcs[0] oldest frame (int or c2i)
  // frame_sizes/frame_pcs[1] next oldest frame (int)
  // frame_sizes/frame_pcs[n] youngest frame (int)
  //
  // Now a pc in frame_pcs is actually the return address to the frame's caller (a frame
  // owns the space for the return address to it's caller).  Confusing ain't it.
  // 
  // The vframe array can address vframes with indices running from
  // 0.._frames-1. Index  0 is the youngest frame and _frame - 1 is the oldest (root) frame.
  // When we create the skeletal frames we need the oldest frame to be in the zero slot
  // in the frame_sizes/frame_pcs so the assembly code can do a trivial walk.
  // so things look a little strange in this loop.
  //
  for (int index = 0; index < array->frames(); index++ ) {
    // frame[number_of_frames - 1 ] = on_stack_size(youngest)
    // frame[number_of_frames - 2 ] = on_stack_size(sender(youngest))
    // frame[number_of_frames - 3 ] = on_stack_size(sender(sender(youngest)))
    frame_sizes[number_of_frames - 1 - index] = BytesPerWord * array->element(index)->on_stack_size(callee_parameters, 
												    callee_locals,
												    index == 0,
												    popframe_extra_args);
    // This pc doesn't have to be perfect just good enough to identify the frame
    // as interpreted so the skeleton frame will be walkable
    // The correct pc will be set when the skeleton frame is completely filled out
    // The final pc we store in the loop is wrong and will be overwritten below
    frame_pcs[number_of_frames - 1 - index ] = AbstractInterpreter::deopt_entry(vtos, 0) - frame::pc_return_offset;

    callee_parameters = array->element(index)->method()->size_of_parameters();
    callee_locals = array->element(index)->method()->max_locals();
    popframe_extra_args = 0;
  }

  // Compute whether the root vframe returns a float or double value.
  BasicType return_type;
  {
    HandleMark hm;
    methodHandle method(thread, array->element(0)->method());
    Bytecode_invoke* invoke = Bytecode_invoke_at_check(method, array->element(0)->bci());
    return_type = (invoke != NULL) ? invoke->result_type(thread) : T_ILLEGAL;
  }

  // Compute information for handling adapters and adjusting the frame size of the caller.
  int adapter_size = 0;
  int caller_adjustment = 0;

  // If we are popping an adapter compute its size. 
  // Strange but true: you'd think that this size would simply be frame_size in the
  // CodeBlob but you'd be wrong because the adapter frame can be variable sized.
  // So we have to calculate its size here./ So rather than this:
  //
  // if (array->old_adapter() != NULL)
  //   adapter_size = array->old_adapter()->frame_size() * BytesPerWord;
  //
  // We have:
  //
  if (array->old_adapter() != NULL) 
    adapter_size = (array->adapter_caller().unextended_sp() - (array->sp() + array->frame_size())) * BytesPerWord;

#ifdef COMPILER1
  if (callee_locals > callee_parameters) {
    // The caller frame may need extending to accommodate non-parameter locals of the first unpacked interpreted
    // frame.  Compute that adjustment.
    caller_adjustment = array->extend_caller_frame(callee_locals, callee_parameters);
  }
#endif
#ifdef COMPILER2

  // Compute the size that the first interpreter frame will have to adjust on the callers frame
  // The caller could be a c2i we create here or if we are popping an i2c then the
  // caller is an interpreter frame.
  caller_adjustment = last_frame_adjust(callee_parameters, callee_locals);

  if (new_adapter != NULL) {
    assert(array->adapter_caller().sp() == NULL, "should not be set");
    // Compute the size of a new C2I adapter frame that will preceed the unrolled frames.
    frame_sizes[0] = new_adapter->frame_size() * BytesPerWord;
    // The first interpreter frame returns to the adapter not to what we just set in place
    frame_pcs[1] = new_adapter->return_from_interpreter() - frame::pc_return_offset;
  } else {
    // We're removing an old adapter
    assert(array->adapter_caller().sp() != NULL, "must have caller of adapter to be popped");
    assert(array->old_adapter() != NULL, "must have an old adapter to be popped");
  }
#endif
  // Find the current pc for sender of the deoptee. Since the sender may have been deoptimized
  // itself since the deoptee vframeArray was created we must get a fresh value of the pc rather
  // than simply use array->sender.pc(). This requires us to walk the current set of frames
  //
  frame deopt_sender = stub_frame.sender(&dummy_map); // First is the deoptee frame
  deopt_sender = deopt_sender.sender(&dummy_map);     // Now deoptee caller
  // Is deoptee caller and adapter?
  if (array->old_adapter() != NULL)
    deopt_sender = deopt_sender.sender(&dummy_map);

  // Hate this subtraction
  frame_pcs[0] = deopt_sender.pc() - frame::pc_return_offset;

  assert(CodeCache::find_blob_unsafe(frame_pcs[0]) != NULL, "bad pc");

  UnrollBlock* info = new UnrollBlock(array->frame_size() * BytesPerWord, 
				      adapter_size,
				      need_new_adapter,
				      caller_adjustment * BytesPerWord,
				      number_of_frames,
				      frame_sizes,
				      frame_pcs,
				      return_type);
#ifdef COMPILER2
  // We need a way to pass fp to the unpacking code so the skeletal frames
  // come out correct. This is only needed for x86 because of c2 using ebp
  // as an allocatable register. So this update is useless (and harmless)
  // on the other platforms. It would be nice to do this in a different
  // way but even the old style deoptimization had a problem with deriving
  // this value. NEEDS_CLEANUP
  intptr_t** fp_addr = (intptr_t**) (((address)info) + info->initial_fp_offset_in_bytes());
  *fp_addr = array->sender().fp(); // was adapter_caller
#endif

  if (array->frames() > 1) {
    if (VerifyStack && TraceDeoptimization) {
      tty->print_cr("Deoptimizing method containing inlining");
    }
  }

  array->set_unroll_block(info);
  return info;
}
// Return BasicType of value being returned
JRT_LEAF(BasicType, Deoptimization::unpack_frames(JavaThread* thread, int exec_mode))

  // We are already active int he special DeoptResourceMark any ResourceObj's we
  // allocate will be freed at the end of the routine.

  // It is actually ok to allocate handles in a leaf method. It causes no safepoints,
  // but makes the entry a little slower. There is however a little dance we have to
  // do in debug mode to get around the NoHandleMark code in the JRT_LEAF macro
  ResetNoHandleMark rnhm; // No-op in release/product versions
  HandleMark hm;

  frame stub_frame = thread->last_frame();

  // Since the frame to unpack is the top frame of this thread, the vframe_array_head
  // must point to the vframeArray for the unpack frame.
  vframeArray* array = thread->vframe_array_head();

#ifndef PRODUCT
  if (TraceDeoptimization) {
    tty->print_cr("DEOPT UNPACKING thread " INTPTR_FORMAT " vframeArray " INTPTR_FORMAT " mode %d", thread, array, exec_mode);
  }
#endif

  UnrollBlock* info = array->unroll_block();

  thread->set_vframe_array_head(NULL);

  // Unpack the interpreter frames and any adapter frame (c2 only) we might create.
  array->unpack_to_stack(stub_frame, exec_mode);


  BasicType bt = info->return_type();

  // If we have an exception pending, claim that the return type is an oop 
  // so the deopt_blob does not overwrite the exception_oop. 

  if (exec_mode == Unpack_exception)
    bt = T_OBJECT;


  // Free the previous UnrollBlock
  vframeArray* old_array = thread->vframe_array_last();
  thread->set_vframe_array_last(array);

  if (old_array != NULL) {
    UnrollBlock* old_info = old_array->unroll_block();
    old_array->set_unroll_block(NULL);
    delete old_info;
    delete old_array;
  }


  // Deallocate any resource creating in this routine and any ResourceObjs allocated
  // inside the vframeArray (StackValueCollections)

  delete thread->deopt_mark();
  thread->set_deopt_mark(NULL);


  if (JvmtiExport::can_pop_frame()) {
#ifndef CC_INTERP
    // Regardless of whether we entered this routine with the pending
    // popframe condition bit set, we should always clear it now
    thread->clear_popframe_condition();
#else
    // C++ interpeter will clear has_pending_popframe when it enters
    // with method_resume. For deopt_resume2 we clear it now.
    if (thread->popframe_forcing_deopt_reexecution())
	thread->clear_popframe_condition();
#endif /* CC_INTERP */
  }
  // unpack_frames() is called at the end of the deoptimization handler
  // and (in C2) at the end of the uncommon trap handler. Note this fact
  // so that an asynchronous stack walker can work again. This counter is
  // incremented at the beginning of fetch_unroll_info() and (in C2) at
  // the beginning of uncommon_trap().
  thread->dec_in_deopt_handler();

#ifndef PRODUCT
  if (VerifyStack) {
    ResourceMark res_mark;

    // Verify that the just-unpacked frames match the interpreter's
    // notions of expression stack and locals
    vframeArray* cur_array = thread->vframe_array_last();
    RegisterMap rm(thread, false);
    rm.set_include_argument_oops(false);
    bool is_top_frame = true;
    int callee_size_of_parameters = 0;
    int callee_max_locals = 0;
    for (int i = 0; i < cur_array->frames(); i++) {
      vframeArrayElement* el = cur_array->element(i);
      frame* iframe = el->iframe();
      guarantee(iframe->is_interpreted_frame(), "Wrong frame type");

      // Get the oop map for this bci
      InterpreterOopMap mask;
      int cur_invoke_parameter_size = 0;
      bool try_next_mask = false;
      int next_mask_expression_stack_size = -1;
      int top_frame_expression_stack_adjustment = 0;
      methodHandle mh(thread, iframe->interpreter_frame_method());
      OopMapCache::compute_one_oop_map(mh, iframe->interpreter_frame_bci(), &mask);
      BytecodeStream str(mh);
      str.set_start(iframe->interpreter_frame_bci());
      int max_bci = mh->code_size();
      // Get to the next bytecode if possible
      assert(str.bci() < max_bci, "bci in interpreter frame out of bounds");
      // Check to see if we can grab the number of outgoing arguments
      // at an uncommon trap for an invoke (where the compiler
      // generates debug info before the invoke has executed)
      Bytecodes::Code cur_code = str.next();
      if (cur_code == Bytecodes::_invokevirtual ||
          cur_code == Bytecodes::_invokespecial ||
          cur_code == Bytecodes::_invokestatic  ||
          cur_code == Bytecodes::_invokeinterface) {
        Bytecode_invoke* invoke = Bytecode_invoke_at(mh, iframe->interpreter_frame_bci());
        symbolHandle signature(thread, invoke->signature());
        ArgumentSizeComputer asc(signature);
        cur_invoke_parameter_size = asc.size();
        if (cur_code != Bytecodes::_invokestatic) {
          // Add in receiver
          ++cur_invoke_parameter_size;
        }
      }
      if (str.bci() < max_bci) {
        Bytecodes::Code bc = str.next();
        if (bc >= 0) {
          // The interpreter oop map generator reports results before
          // the current bytecode has executed except in the case of
          // calls. It seems to be hard to tell whether the compiler
          // has emitted debug information matching the "state before"
          // a given bytecode or the state after, so we try both
          switch (cur_code) {
            case Bytecodes::_invokevirtual:
            case Bytecodes::_invokespecial:     
            case Bytecodes::_invokestatic:      
            case Bytecodes::_invokeinterface:   
              break;
            default: {
              InterpreterOopMap next_mask;
              OopMapCache::compute_one_oop_map(mh, str.bci(), &next_mask);
              next_mask_expression_stack_size = next_mask.expression_stack_size();
              // Need to subtract off the size of the result type of
              // the bytecode because this is not described in the
              // debug info but returned to the interpreter in the TOS
              // caching register
              BasicType bytecode_result_type = Bytecodes::result_type(cur_code);
              if (bytecode_result_type != T_ILLEGAL) {
                top_frame_expression_stack_adjustment = type2size[bytecode_result_type];
              }
              assert(top_frame_expression_stack_adjustment >= 0, "");
              try_next_mask = true;
              break;
            }
          }
        }
      }

      // Verify stack depth and oops in frame
      // This assertion may be dependent on the platform we're running on and may need modification (tested on x86 and sparc)
      if (!(
            /* SPARC */
            (iframe->interpreter_frame_expression_stack_size() == mask.expression_stack_size() + callee_size_of_parameters) ||
            /* x86 */
            (iframe->interpreter_frame_expression_stack_size() == mask.expression_stack_size() + callee_max_locals) ||
            (try_next_mask &&
             (iframe->interpreter_frame_expression_stack_size() == (next_mask_expression_stack_size -
                                                                    top_frame_expression_stack_adjustment))) ||
            (is_top_frame && (exec_mode == Unpack_exception) && iframe->interpreter_frame_expression_stack_size() == 0) ||
            (is_top_frame && (exec_mode == Unpack_uncommon_trap || exec_mode == Unpack_reexecute) &&
             (iframe->interpreter_frame_expression_stack_size() == mask.expression_stack_size() + cur_invoke_parameter_size))
            )) {
        ttyLocker ttyl;

        // Print out some information that will help us debug the problem
        tty->print_cr("Wrong number of expression stack elements during deoptimization");
        tty->print_cr("  Error occurred while verifying frame %d (0..%d, 0 is topmost)", i, cur_array->frames() - 1);
        tty->print_cr("  Fabricated interpreter frame had %d expression stack elements",
                      iframe->interpreter_frame_expression_stack_size());
        tty->print_cr("  Interpreter oop map had %d expression stack elements", mask.expression_stack_size());
        tty->print_cr("  try_next_mask = %d", try_next_mask);
        tty->print_cr("  next_mask_expression_stack_size = %d", next_mask_expression_stack_size);
        tty->print_cr("  callee_size_of_parameters = %d", callee_size_of_parameters);
        tty->print_cr("  callee_max_locals = %d", callee_max_locals);
        tty->print_cr("  top_frame_expression_stack_adjustment = %d", top_frame_expression_stack_adjustment);
        tty->print_cr("  exec_mode = %d", exec_mode);
        tty->print_cr("  cur_invoke_parameter_size = %d", cur_invoke_parameter_size);
        tty->print_cr("  Thread = " INTPTR_FORMAT ", thread ID = " UINTX_FORMAT, thread, thread->osthread()->thread_id());
        tty->print_cr("  Interpreted frames:");
        for (int k = 0; k < cur_array->frames(); k++) {
          vframeArrayElement* el = cur_array->element(k);
          tty->print_cr("    %s (bci %d)", el->method()->name_and_sig_as_C_string(), el->bci());
        }
        cur_array->print_on_2(tty);
        guarantee(false, "wrong number of expression stack elements during deopt");
      }
      VerifyOopClosure verify;
      iframe->oops_interpreted_do(&verify, &rm, false);
      callee_size_of_parameters = mh->size_of_parameters();
      callee_max_locals = mh->max_locals();
      is_top_frame = false;
    }
  }
#endif /* !PRODUCT */

  return bt;
JRT_END


int Deoptimization::deoptimize_dependents() {
  Threads::deoptimized_wrt_marked_nmethods();
  return 0;
}

vframeArray* Deoptimization::create_vframeArray(JavaThread* thread, frame fr, RegisterMap *reg_map) {
  // Create a growable array of VFrames where each VFrame represents an inlined
  // Java frame.  This storage is allocated with the usual system arena.
#ifdef ASSERT
  bool is_deopted;
  assert(fr.is_compiled_frame(&is_deopted), "Wrong frame type");
#endif /* ASSERT */
  GrowableArray<compiledVFrame*>* chunk = new GrowableArray<compiledVFrame*>(10);  
  vframe* vf = vframe::new_vframe(&fr, reg_map, thread);        
  while (!vf->is_top()) {
    assert(vf->is_compiled_frame(), "Wrong frame type");
    chunk->push(compiledVFrame::cast(vf));
    vf = vf->sender();
  }
  assert(vf->is_compiled_frame(), "Wrong frame type");
  chunk->push(compiledVFrame::cast(vf));

#ifndef PRODUCT
  if (TraceDeoptimization) {
    ttyLocker ttyl;
    tty->print("DEOPT PACKING thread " INTPTR_FORMAT " ", thread);
    fr.print_on(tty);
    tty->print_cr("     Virtual frames (innermost first):");
    for (int index = 0; index < chunk->length(); index++) {
      compiledVFrame* vf = chunk->at(index);
      tty->print("       %2d - ", index);
      vf->print_value();
      int bci = chunk->at(index)->raw_bci();
      const char* code_name;
      if (bci == SynchronizationEntryBCI) {
        code_name = "sync entry";
      } else {
        Bytecodes::Code code = Bytecodes::code_at(vf->method(), bci);
        code_name = Bytecodes::name(code);
      }
      tty->print(" - %s", code_name);
      tty->print_cr(" @ bci %d ", bci);
      if (Verbose) {
        vf->print();
        tty->cr();
      }
    }
  }
#endif

  // Register map for next frame (used for stack crawl).  We capture
  // the state of the deopt'ing frame's caller.  Thus if we need to
  // stuff a C2I adapter we can properly fill in the callee-save
  // register locations.
  frame caller = fr.sender(reg_map);
  int frame_size = caller.sp() - fr.sp();

  CodeBlob* old_adapter = NULL;

  // If the caller of the deoptimized frame is an I2C or OSR adapter, it will be removed to achieve an
  // I2I transition.  Otherwise, the caller must be a compiled frame and for Compiler2, a C2I adapter
  // will be created when the vframeArray is allocated below.
  frame adapter_caller;  // treated as null unless its sp() gets set
  frame sender = caller;
#ifdef COMPILER1
  if (caller.is_osr_adapter_frame() ||
      caller.pc() == Runtime1::entry_for(Runtime1::alignment_frame_return_id)) {
    old_adapter = CodeCache::find_blob(caller.pc());
    assert(old_adapter, "Must have adapter to remove");
    adapter_caller = caller.sender(reg_map);
    sender = adapter_caller;
  }
#endif
#ifdef COMPILER2
  CodeBlob *cb = CodeCache::find_blob(caller.pc());
  if (cb && (cb->is_osr_adapter() || cb->is_i2c_adapter())) {
    // Capture the I2C frame; this holds info we need to populate the last
    // interpreted frame with for when it unwinds.  This kills callee-save
    // values for the caller BUT the caller's caller was the interpreter
    // and he has no callee-save values.
    old_adapter = cb;
    adapter_caller = caller.sender(reg_map);
    sender = adapter_caller;
    NOT_PRODUCT(OptoRuntime::_compress_i2c2i_ctr++;)
  }
#endif
 
  // Since the Java thread being deoptimized will eventually adjust it's own stack,
  // the vframeArray containing the unpacking information is allocated in the C heap.
  // For Compiler1, the caller of the deoptimized frame is saved for use by unpack_frames().
  vframeArray* array = vframeArray::allocate(thread, frame_size, chunk, reg_map, sender, caller, fr, adapter_caller);
  array->set_old_adapter(old_adapter);
#ifdef COMPILER2
  assert(fr.unextended_sp() == fr.sp(), "callee must be C2I adapter; does not change caller's SP");
  array->set_adjust_adapter_caller(cb && cb->is_i2c_adapter());
#endif

  // Compare the vframeArray to the collected vframes
  assert(array->structural_compare(thread, chunk), "just checking");
  Events::log("# vframes = %d", (intptr_t)chunk->length());

#ifndef PRODUCT
  if (TraceDeoptimization) {
    ttyLocker ttyl;
    tty->print_cr("     Created vframeArray " INTPTR_FORMAT, array);
    if (Verbose) {
      int count = 0;
      // this used to leak deoptimizedVFrame like it was going out of style!!!
      for (int index = 0; index < array->frames(); index++ ) {
        vframeArrayElement* e = array->element(index);
        e->print(tty);

	/*
	  No printing yet.
        array->vframe_at(index)->print_activation(count++);
	// better as...
        array->print_activation_for(index, count++);
	*/
      }
    }
  }
#endif // PRODUCT

  return array;
}


void Deoptimization::deoptimize_single_frame(JavaThread* thread, frame fr) {
  assert(fr.can_be_deoptimized(), "checking frame type");

  gather_statistics(Reason_constraint, Action_none, Bytecodes::_illegal);

  EventMark m("Deoptimization (pc=" INTPTR_FORMAT ", sp=" INTPTR_FORMAT ")", fr.pc(), fr.id());

#ifdef COMPILER2

  // We must always have a c2i adapter available for any c2 method
  // we deoptimize since it may have been called by compiled code
  // and therefore need an adapter after deoptimization. We can
  // only generate that adapter at a point when we can tolerate
  // a safepoint and this point is one of them. We used to do
  // this generation only if this particular frame needed the
  // adapter. Bug 4992987 was caused by having another thread
  // that needed an adapter and we didn't generate one while
  // we could.

  nmethod *nm = CodeCache::find_nmethod(fr.pc());
  if (nm->c2i_adapter() == NULL) {
    methodOop callee = nm->method();
    nm->set_c2i_adapter(C2IAdapterGenerator::adapter_for(callee));
  }

#endif /* COMPILER2 */

  // Patch the nmethod so that when execution returns to it we will
  // deopt the execution state and return to the interpreter.

  fr.deoptimize(thread);

}

void Deoptimization::deoptimize(JavaThread* thread, frame fr) {
  bool is_deopted;
  // Deoptimize only if the frame comes from compile code.
  // Do not deoptimize the frame which is already patched 
  // during the execution of the loops below.
  if (!fr.is_compiled_frame(&is_deopted) || is_deopted)
    return;
  ResourceMark rm;
  DeoptimizationMarker dm;    
  deoptimize_single_frame(thread, fr);
  /* for machines that only patch a few bytes we must make sure
     all threads executing in the nmethod just deoptimized will
     also be deoptimized so they don't stumble across newly 
     patched code. Sparc doesn't have this issue since the
     first deopt nops the entire nmethod so everyone gets evicted
     at once. code generation pattern for ia64 suggest it
     is immune also even though we only patch individual
     instructions. x86 could be a (very rare) problem.
     We do ia64 just to be safe and x86 because we must.
  */
  if (nmethod::evict_all_threads_at_deopt()) {
    CodeBlob *cb = CodeCache::find_blob(fr.pc());
    for (JavaThread* jt = Threads::first(); jt != NULL ; jt = jt->next()) {
      if (jt->has_last_Java_frame()) {
	for(StackFrameStream fst(jt, false); !fst.is_done(); fst.next()) {
	  if (fst.current()->is_compiled_frame(&is_deopted) && is_deopted) {
            // Do not deoptimize the frame which we did already above.
            if (fr.id() == fst.current()->id()) continue;
            // Find other frames which belong to the same nmethod.
            if (cb == CodeCache::find_blob(fst.current()->pc())) {
              // Do not patch pc() which we did already above.
              if (fr.pc() == fst.current()->pc()) continue;
	      // Some frames can't be deoptimized no matter what (at return point).
	      if (fst.current()->can_be_deoptimized()) {
		deoptimize_single_frame(jt, *fst.current());
	      }
	    }
	  }
	}
      }
    }
  }

}


void Deoptimization::deoptimize_frame(JavaThread* thread, intptr_t* id) {
  // Compute frame and register map based on thread and sp.
  RegisterMap reg_map(thread, false);
  frame fr = thread->last_frame();
  while (fr.id() != id) {
    fr = fr.sender(&reg_map);
  }
  deoptimize(thread, fr);
}


// JVMTI PopFrame support
JRT_LEAF(void, Deoptimization::popframe_preserve_args(JavaThread* thread, int bytes_to_save, void* start_address))
{
  thread->popframe_preserve_args(in_ByteSize(bytes_to_save), start_address);
}
JRT_END


#ifdef COMPILER2
void Deoptimization::load_class_by_index(constantPoolHandle constant_pool, int index, TRAPS) {
  // in case of an unresolved klass entry, load the class.
  if (constant_pool->tag_at(index).is_unresolved_klass()) {
    klassOop tk = constant_pool->klass_at(index, CHECK);
    return;
  }

  if (!constant_pool->tag_at(index).is_symbol()) return;

  Handle class_loader (THREAD, instanceKlass::cast(constant_pool->pool_holder())->class_loader());
  symbolHandle symbol (THREAD, constant_pool->symbol_at(index));

  // class name?
  if (symbol->byte_at(0) != '(') {
    Handle protection_domain (THREAD, Klass::cast(constant_pool->pool_holder())->protection_domain());
    SystemDictionary::resolve_or_null(symbol, class_loader, protection_domain, CHECK);
    return;
  }

  // then it must be a signature!
  for (SignatureStream ss(symbol); !ss.is_done(); ss.next()) {
    if (ss.is_object()) {
      symbolOop s = ss.as_symbol(CHECK);
      symbolHandle class_name (THREAD, s);
      Handle protection_domain (THREAD, Klass::cast(constant_pool->pool_holder())->protection_domain());
      SystemDictionary::resolve_or_null(class_name, class_loader, protection_domain, CHECK);
    }
  }
}


void Deoptimization::load_class_by_index(constantPoolHandle constant_pool, int index) {
  EXCEPTION_MARK;
  load_class_by_index(constant_pool, index, THREAD);
  if (PENDING_EXCEPTION) {
    // Exception happened during classloading. We ignore the exception here, since it
    // is going to be rethrown since the current activation is going to be deoptimzied and
    // the interpreter will re-execute the bytecode. 
    CLEAR_PENDING_EXCEPTION;
  }
}

JRT_ENTRY(void, Deoptimization::uncommon_trap_inner(JavaThread* thread, jint trap_request)) {
  HandleMark hm;

  // uncommon_trap() is called at the beginning of the uncommon trap
  // handler. Note this fact before we start generating temporary frames
  // that can confuse an asynchronous stack walker. This counter is
  // decremented at the end of unpack_frames().
  thread->inc_in_deopt_handler();

  RegisterMap reg_map(thread, false); // Do not need to update callee-saved registers
  frame stub_frame = thread->last_frame();
  frame fr = stub_frame.sender(&reg_map);
  // Make sure the calling nmethod is not getting deoptimized and removed 
  // before we are done with it.
  nmethodLocker nl(fr.pc());
#ifdef COMPILER2
  {
    // We must always have a c2i adapter available for any c2 method
    // we deoptimize since it may have been called by compiled code
    // and therefore need an adapter after deoptimization. We can
    // only generate that adapter at a point when we can tolerate
    // a safepoint and this point is one of them. We used to do
    // this generation only if this particular frame needed the
    // adapter. Bug 4992987 was caused by having another thread
    // that needed an adapter and we didn't generate one while
    // we could.

    nmethod *nm = CodeCache::find_nmethod(fr.pc());
    if (nm->c2i_adapter() == NULL) {
      methodOop callee = nm->method();
      nm->set_c2i_adapter(C2IAdapterGenerator::adapter_for(callee));
    }
    frame caller = fr.sender(&reg_map);
    CodeBlob* cb = CodeCache::find_blob(caller.pc());

  }
#endif /* COMPILER2 */

  {
    ResourceMark rm;
  
    DeoptReason reason = trap_request_reason(trap_request);
    DeoptAction action = trap_request_action(trap_request);
    jint unloaded_class_index = trap_request_index(trap_request); // CP idx or -1

    Events::log("Uncommon trap occurred @" INTPTR_FORMAT " unloaded_class_index = %d", fr.pc(), (int) trap_request);
    vframe*  vf  = vframe::new_vframe(&fr, &reg_map, thread);
    nmethod* nm = compiledVFrame::cast(vf)->code();

    ScopeDesc*      trap_scope  = compiledVFrame::cast(vf)->scope();
    methodHandle    trap_method = trap_scope->method();
    int             trap_bci    = trap_scope->bci();
    Bytecodes::Code trap_bc     = Bytecode_at(trap_method->bcp_from(trap_bci))->java_code();

    // Record this event in the histogram.
    gather_statistics(reason, action, trap_bc);

    // Ensure that we can record deopt. history:
    bool create_if_missing = ProfileTraps;

    methodDataHandle trap_mdo
      = get_method_data(thread, trap_method, create_if_missing);

    // Print a bunch of diagnostics, if requested.
    if (TraceDeoptimization || LogCompilation) {
      ResourceMark rm;
      ttyLocker ttyl;
      char buf[100];
      if (xtty != NULL) {
	xtty->begin_head("uncommon_trap thread='" UINTX_FORMAT"'",
			 os::current_thread_id(),
                         format_trap_request(buf, sizeof(buf), trap_request));
	xtty->print(" compile_id='%d'", nm->compile_id());
	const char* nm_kind = nm->compile_kind();
	if (nm_kind != NULL)  xtty->print(" compile_kind='%s'", nm_kind);
      }
      symbolHandle class_name;
      bool unresolved = false;
      if (unloaded_class_index >= 0) {
	constantPoolHandle constants (THREAD, trap_method->constants());
	if (constants->tag_at(unloaded_class_index).is_unresolved_klass()) {
	  class_name = constants->klass_name_at(unloaded_class_index);
	  unresolved = true;
	  if (xtty != NULL)
	    xtty->print(" unresolved='1'");
	} else if (constants->tag_at(unloaded_class_index).is_symbol()) {
	  class_name = constants->symbol_at(unloaded_class_index);
	}
	if (xtty != NULL)
	  xtty->name(class_name);
      }
      if (xtty != NULL && trap_mdo.not_null()) {
        // Dump the relevant MDO state.
        // This is the deopt count for the current reason, any previous
        // reasons or recompiles seen at this point.
        int dcnt = trap_mdo->trap_count(reason);
        if (dcnt != 0)
          xtty->print(" count='%d'", dcnt);
        ProfileData* pdata = trap_mdo->bci_to_data(trap_bci);
        int dos = (pdata == NULL)? 0: pdata->trap_state();
        if (dos != 0) {
          xtty->print(" state='%s'", format_trap_state(buf, sizeof(buf), dos));
          if (trap_state_is_recompiled(dos)) {
            int recnt2 = trap_mdo->overflow_recompile_count();
            if (recnt2 != 0)
              xtty->print(" recompiles2='%d'", recnt2);
          }
        }
      }
      if (xtty != NULL) {
	xtty->stamp();
	xtty->end_head();
      }
      if (TraceDeoptimization) {  // make noise on the tty
	tty->print("Uncommon trap occurred in");
	nm->method()->print_short_name(tty);
	tty->print(" (@" INTPTR_FORMAT ") thread=%d reason=%s action=%s unloaded_class_index=%d",
                   fr.pc(), 
                   (int) os::current_thread_id(),
                   trap_reason_name(reason),
                   trap_action_name(action),
		   unloaded_class_index);
	if (class_name.not_null()) {
	  tty->print(unresolved ? " unresolved class: " : " symbol: ");
	  class_name->print_symbol_on(tty);
	}
	tty->cr();
      }
      if (xtty != NULL) {
	// Log the precise location of the trap.
	for (ScopeDesc* sd = trap_scope; ; sd = sd->sender()) {
	  xtty->begin_elem("jvms bci='%d'", sd->bci());
	  xtty->method(sd->method());
	  xtty->end_elem();
	  if (sd->is_top())  break;
	}
	xtty->tail("uncommon_trap");
      }
    }
    // (End diagnostic printout.)

    // Load class if necessary 
    if (unloaded_class_index >= 0) {
      constantPoolHandle constants(THREAD, trap_method->constants());
      load_class_by_index(constants, unloaded_class_index);
    }

    // Flush the nmethod if necessary and desirable.
    //
    // We need to avoid situations where we are re-flushing the nmethod
    // because of a hot deoptimization site.  Repeated flushes at the same
    // point need to be detected by the compiler and avoided.  If the compiler
    // cannot avoid them (or has a bug and "refuses" to avoid them), this
    // module must take measures to avoid an infinite cycle of recompilation
    // and deoptimization.  There are several such measures:
    //
    //   1. If a recompilation is ordered a second time at some site X
    //   and for the same reason R, the action is adjusted to 'reinterpret',
    //   to give the interpreter time to exercise the method more thoroughly.
    //   If this happens, the method's overflow_recompile_count is incremented.
    //
    //   2. If the compiler fails to reduce the deoptimization rate, then
    //   the method's overflow_recompile_count will begin to exceed the set
    //   limit PerBytecodeRecompilationCutoff.  If this happens, the action
    //   is adjusted to 'make_not_compilable', and the method is abandoned
    //   to the interpreter.  This is a performance hit for hot methods,
    //   but is better than a disastrous infinite cycle of recompilations.
    //   (Actually, only the method containing the site X is abandoned.)
    //
    //   3. In parallel with the previous measures, if the total number of
    //   recompilations of a method exceeds the much larger set limit
    //   PerMethodRecompilationCutoff, the method is abandoned.
    //   This should only happen if the method is very large and has
    //   many "lukewarm" deoptimizations.  The code which enforces this
    //   limit is elsewhere (class nmethod, class methodOopDesc).
    //
    // Note that the per-BCI 'is_recompiled' bit gives the compiler one chance
    // to recompile at each bytecode independently of the per-BCI cutoff.
    //
    // The decision to update code is up to the compiler, and is encoded
    // in the Action_xxx code.  If the compiler requests Action_none
    // no trap state is changed, no compiled code is changed, and the
    // computation suffers along in the interpreter.
    //
    // The other action codes specify various tactics for decompilation
    // and recompilation.  Action_maybe_recompile is the loosest, and
    // allows the compiled code to stay around until enough traps are seen,
    // and until the compiler gets around to recompiling the trapping method.
    //
    // The other actions cause immediate removal of the present code.

    bool update_trap_state = true;
    bool make_not_entrant = false;
    bool make_not_compilable = false;
    bool reset_counters = false;
    switch (action) {
    case Action_none:
      // Keep the old code.
      update_trap_state = false;
      break;
    case Action_maybe_recompile:
      // Do not need to invalidate the present code, but we can
      // initiate another 
      // Start compiler without (necessarily) invalidating the nmethod.
      // The system will tolerate the old code, but new code should be
      // generated when possible.
      break;
    case Action_reinterpret:
      // Go back into the interpreter for a while, and then consider
      // recompiling form scratch.
      make_not_entrant = true;
      // Reset invocation counter for outer most method.
      // This will allow the interpreter to exercise the bytecodes
      // for a while before recompiling.
      // By contrast, Action_make_not_entrant is immediate.
      //
      // Note that the compiler will track null_check, div0_check,
      // range_check, and class_check events and log them as if they
      // had been traps taken from compiled code.  This will update
      // the MDO trap history so that the next compilation will
      // properly detect hot trap sites.
      reset_counters = true;
      break;
    case Action_make_not_entrant:
      // Request immediate recompilation, and get rid of the old code.
      // Make them not entrant, so next time they are called they get
      // recompiled.  Unloaded classes are loaded now so recompile before next
      // time they are called.  Same for uninitialized.  The interpreter will
      // link the missing class, if any.
      make_not_entrant = true;
      break;
    case Action_make_not_compilable:
      // Give up on compiling this method at all.
      make_not_entrant = true;
      make_not_compilable = true;
      break;
    default:
      ShouldNotReachHere();
    }

    // Setting +ProfileTraps fixes the following, on all platforms:
    // 4852688: ProfileInterpreter is off by default for ia64.  The result is
    // infinite heroic-opt-uncommon-trap/deopt/recompile cycles, since the
    // recompile relies on a methodDataOop to record heroic opt failures.

    // Whether the interpreter is producing MDO data or not, we also need
    // to use the MDO to detect hot deoptimization points and control
    // aggressive optimization.
    if (ProfileTraps && update_trap_state && trap_mdo.not_null()) {
      assert(trap_mdo() == get_method_data(thread, trap_method, false), "sanity");
      uint this_trap_count = 0;
      bool maybe_prior_trap = false;
      bool maybe_prior_recompile = false;
      ProfileData* pdata
        = query_update_method_data(trap_mdo, trap_bci, reason,
                                   //outputs:
                                   this_trap_count,
                                   maybe_prior_trap,
                                   maybe_prior_recompile);
      // Because the interpreter counts div0, null, range, and class
      // checks, these traps from compiled code are double-counted.
      // This is harmless; it just means that the PerXTrapLimit values
      // are in effect a little smaller than they look.

      if (reason_is_recorded_per_bytecode(reason)) {
        // Now take action based on the partially known per-BCI history.
        if (maybe_prior_trap
            && this_trap_count >= (uint)PerBytecodeTrapLimit) {
          // If there are too many traps at this BCI, force a recompile.
          // This will allow the compiler to see the limit overflow, and
          // take corrective action, if possible.  The compiler generally
          // does not use the exact PerBytecodeTrapLimit value, but instead
          // changes its tactics if it sees any traps at all.  This provides
          // a little hysteresis, delaying a recompile until a trap happens
          // several times.
          //
          // Actually, since there is only one bit of counter per BCI,
          // the possible per-BCI counts are {0,1,(per-method count)}.
          // This produces accurate results if in fact there is only
          // one hot trap site, but begins to get fuzzy if there are
          // many sites.  For example, if there are ten sites each
          // trapping two or more times, they each get the blame for
          // all of their traps.
          make_not_entrant = true;
        }

        // Detect repeated recompilation at the same BCI, and enforce a limit.
        if (make_not_entrant && maybe_prior_recompile) {
          // More than one recompile at this point.
          trap_mdo->inc_overflow_recompile_count();
          if (maybe_prior_trap
              && ((uint)trap_mdo->overflow_recompile_count()
                  > (uint)PerBytecodeRecompilationCutoff)) {
            // Give up on the method containing the bad BCI.
            if (trap_method() == nm->method()) {
              make_not_compilable = true;
            } else {
              trap_method->set_not_compilable();
              // But give grace to the enclosing nm->method().
            }
          }
        }
      } else {
        // For reasons which are not recorded per-bytecode, we simply
        // force recompiles unconditionally.
        // (Note that PerMethodRecompilationCutoff is enforced elsewhere.)
        make_not_entrant = true;
      }

      // Go back to the compiler if there are too many traps in this method.
      if (this_trap_count >= (uint)PerMethodTrapLimit) {
        // If there are too many traps in this method, force a recompile.
        // This will allow the compiler to see the limit overflow, and
        // take corrective action, if possible.
        // (This condition is an unlikely backstop only, because the
        // PerBytecodeTrapLimit is more likely to take effect first,
        // if it is applicable.)
        make_not_entrant = true;
      }

      // Here's more hysteresis:  If there has been a recompile at
      // this trap point already, run the method in the interpreter
      // for a while to exercise it more thoroughly.
      if (make_not_entrant && maybe_prior_recompile && maybe_prior_trap) {
        reset_counters = true;
      }

      if (make_not_entrant && pdata != NULL) {
        // Record the recompilation event, if any.
        int tstate0 = pdata->trap_state();
        int tstate1 = trap_state_set_recompiled(tstate0, true);
        if (tstate1 != tstate0)
          pdata->set_trap_state(tstate1);
      }
    }

    // Take requested actions on the method:

    // Reset invocation counters
    if (reset_counters) {
      if (nm->is_osr_method())
        reset_invocation_counter(trap_scope, CompileThreshold);
      else
        reset_invocation_counter(trap_scope);
    }

    // Recompile
    if (make_not_entrant) {
      nm->make_not_entrant();
    }

    // Give up compiling
    if (make_not_compilable) {
      assert(make_not_entrant, "consistent");
      nm->method()->set_not_compilable();
    }

  } // Free marked resources

}
JRT_END

methodDataOop
Deoptimization::get_method_data(JavaThread* thread, methodHandle m,
                                int create_if_missing) {
  Thread* THREAD = thread;
  methodDataOop mdo = m()->method_data();
  if (mdo == NULL && create_if_missing && !HAS_PENDING_EXCEPTION) {
    // Build an MDO.  Ignore errors like OutOfMemory;
    // that simply means we won't have an MDO to update.
    methodOopDesc::build_interpreter_method_data(m, THREAD);
    if (HAS_PENDING_EXCEPTION) {
      assert((PENDING_EXCEPTION->is_a(SystemDictionary::OutOfMemoryError_klass())), "we expect only an OOM error here");
      CLEAR_PENDING_EXCEPTION;
    }
    mdo = m()->method_data();
  }
  return mdo;
}

ProfileData*
Deoptimization::query_update_method_data(methodDataHandle trap_mdo,
                                         int trap_bci,
                                         Deoptimization::DeoptReason reason,
                                         //outputs:
                                         uint& ret_this_trap_count,
                                         bool& ret_maybe_prior_trap,
                                         bool& ret_maybe_prior_recompile) {
  uint prior_trap_count = trap_mdo->trap_count(reason);
  uint this_trap_count  = trap_mdo->inc_trap_count(reason);

  // If the runtime cannot find a place to store trap history,
  // it is estimated based on the general condition of the method.
  // If the method has ever been recompiled, or has ever incurred
  // a trap with the present reason , then this BCI is assumed
  // (pessimistically) to be the culprit.
  bool maybe_prior_trap      = (prior_trap_count != 0);
  bool maybe_prior_recompile = (trap_mdo->decompile_count() != 0);
  ProfileData* pdata = NULL;

  // For reasons which are recorded per bytecode, we check per-BCI data.
  if (reason_is_recorded_per_bytecode(reason)) {
    // Find the profile data for this BCI.  If there isn't one,
    // try to allocate one from the MDO's set of spares.
    // This will let us detect a repeated trap at this point.
    pdata = trap_mdo->allocate_bci_to_data(trap_bci);

    if (pdata != NULL) {
      // Query the trap state of this profile datum.
      int tstate0 = pdata->trap_state();
      if (!trap_state_has_reason(tstate0, reason))
        maybe_prior_trap = false;
      if (!trap_state_is_recompiled(tstate0))
        maybe_prior_recompile = false;

      // Update the trap state of this profile datum.
      int tstate1 = tstate0;
      // Record the reason.
      tstate1 = trap_state_add_reason(tstate1, reason);
      // Store the updated state on the MDO, for next time.
      if (tstate1 != tstate0)
        pdata->set_trap_state(tstate1);
    } else {
      if (LogCompilation && xtty != NULL)
        // Missing MDP?  Leave a small complaint in the log.
        xtty->elem("missing_mdp bci='%d'", trap_bci);
    }
  }

  // Return results:
  ret_this_trap_count = this_trap_count;
  ret_maybe_prior_trap = maybe_prior_trap;
  ret_maybe_prior_recompile = maybe_prior_recompile;
  return pdata;
}

void
Deoptimization::update_method_data_from_interpreter(methodDataHandle trap_mdo, int trap_bci, int reason) {
  ResourceMark rm;
  // Ignored outputs:
  uint ignore_this_trap_count;
  bool ignore_maybe_prior_trap;
  bool ignore_maybe_prior_recompile;
  query_update_method_data(trap_mdo, trap_bci,
                           (DeoptReason)reason,
                           ignore_this_trap_count,
                           ignore_maybe_prior_trap,
                           ignore_maybe_prior_recompile);
}

void Deoptimization::reset_invocation_counter(ScopeDesc* trap_scope, jint top_count) {
  ScopeDesc* sd = trap_scope;
  for (; !sd->is_top(); sd = sd->sender()) {
    // Reset ICs of inlined methods, since they can trigger compilations also.
    sd->method()->invocation_counter()->reset();
  }
  InvocationCounter* c = sd->method()->invocation_counter();
  if (top_count != _no_count) {
    // It was an OSR method, so bump the count higher.
    c->set(c->state(), top_count);
  } else {
    c->reset();
  }
}

Deoptimization::UnrollBlock* Deoptimization::uncommon_trap(JavaThread* thread, jint trap_request) {
  // Still in Java no safepoints
  {
    // This enters VM and may safepoint
    uncommon_trap_inner(thread, trap_request);
  }
  return fetch_unroll_info_helper(thread);
}

// Local derived constants.
enum {
  // Further breakdown of DataLayout::trap_state, as promised by DataLayout.
  DS_REASON_MASK   = DataLayout::trap_mask >> 1,
  DS_RECOMPILE_BIT = DataLayout::trap_mask - DS_REASON_MASK
};

//---------------------------trap_state_reason---------------------------------
Deoptimization::DeoptReason
Deoptimization::trap_state_reason(int trap_state) {
  // This assert provides the link between the width of DataLayout::trap_bits
  // and the encoding of "recorded" reasons.  It ensures there are enough
  // bits to store all needed reasons in the per-BCI MDO profile.
  assert(DS_REASON_MASK >= Reason_RECORDED_LIMIT, "enough bits");
  int recompile_bit = (trap_state & DS_RECOMPILE_BIT);
  trap_state -= recompile_bit;
  if (trap_state == DS_REASON_MASK) {
    return Reason_many;
  } else {
    assert((int)Reason_none == 0, "state=0 => Reason_none");
    return (DeoptReason)trap_state;
  }
}
//-------------------------trap_state_has_reason-------------------------------
int Deoptimization::trap_state_has_reason(int trap_state, int reason) {
  assert(reason_is_recorded_per_bytecode((DeoptReason)reason), "valid reason");
  assert(DS_REASON_MASK >= Reason_RECORDED_LIMIT, "enough bits");
  int recompile_bit = (trap_state & DS_RECOMPILE_BIT);
  trap_state -= recompile_bit;
  if (trap_state == DS_REASON_MASK) {
    return -1;  // true, unspecifically (bottom of state lattice)
  } else if (trap_state == reason) {
    return 1;   // true, definitely
  } else if (trap_state == 0) {
    return 0;   // false, definitely (top of state lattice)
  } else {
    return 0;   // false, definitely
  }
}
//-------------------------trap_state_add_reason-------------------------------
int Deoptimization::trap_state_add_reason(int trap_state, int reason) {
  assert(reason_is_recorded_per_bytecode((DeoptReason)reason) || reason == Reason_many, "valid reason");
  int recompile_bit = (trap_state & DS_RECOMPILE_BIT);
  trap_state -= recompile_bit;
  if (trap_state == DS_REASON_MASK) {
    return trap_state + recompile_bit;     // already at state lattice bottom
  } else if (trap_state == reason) {
    return trap_state + recompile_bit;     // the condition is already true
  } else if (trap_state == 0) {
    return reason + recompile_bit;          // no condition has yet been true
  } else {
    return DS_REASON_MASK + recompile_bit;  // fall to state lattice bottom
  }
}
//-----------------------trap_state_is_recompiled------------------------------
bool Deoptimization::trap_state_is_recompiled(int trap_state) {
  return (trap_state & DS_RECOMPILE_BIT) != 0;
}
//-----------------------trap_state_set_recompiled-----------------------------
int Deoptimization::trap_state_set_recompiled(int trap_state, bool z) {
  if (z)  return trap_state |  DS_RECOMPILE_BIT;
  else    return trap_state & ~DS_RECOMPILE_BIT;
}
//---------------------------format_trap_state---------------------------------
// This is used for debugging and diagnostics, including hotspot.log output.
const char* Deoptimization::format_trap_state(char* buf, size_t buflen,
                                              int trap_state) {
  DeoptReason reason      = trap_state_reason(trap_state);
  bool        recomp_flag = trap_state_is_recompiled(trap_state);
  // Re-encode the state from its decoded components.
  int decoded_state = 0;
  if (reason_is_recorded_per_bytecode(reason) || reason == Reason_many)
    decoded_state = trap_state_add_reason(decoded_state, reason);
  if (recomp_flag)
    decoded_state = trap_state_set_recompiled(decoded_state, recomp_flag);
  // If the state re-encodes properly, format it symbolically.
  // Because this routine is used for debugging and diagnostics,
  // be robust even if the state is a strange value.
  size_t len;
  if (decoded_state != trap_state) {
    // Random buggy state that doesn't decode??
    len = jio_snprintf(buf, buflen, "#%d", trap_state);
  } else {
    len = jio_snprintf(buf, buflen, "%s%s",
                       trap_reason_name(reason),
                       recomp_flag ? " recompiled" : "");
  }
  if (len >= buflen)
    buf[buflen-1] = '\0';
  return buf;
}


//--------------------------------statics--------------------------------------
Deoptimization::DeoptAction Deoptimization::_unloaded_action
  = Deoptimization::Action_reinterpret;
const char* Deoptimization::_trap_reason_name[Reason_LIMIT] = {
  // Note:  Keep this in sync. with enum DeoptReason.
  "none",
  "null_check",
  "div0_check",
  "range_check",
  "class_check",
  "array_check",
  "intrinsic",
  "unloaded",
  "uninitialized",
  "unreached",
  "unhandled",
  "constraint",
  "age"
};
const char* Deoptimization::_trap_action_name[Action_LIMIT] = {
  // Note:  Keep this in sync. with enum DeoptAction.
  "none",
  "maybe_recompile",
  "reinterpret",
  "make_not_entrant",
  "make_not_compilable"
};

const char* Deoptimization::trap_reason_name(int reason) {
  if (reason == Reason_many)  return "many";
  if ((uint)reason < Reason_LIMIT)
    return _trap_reason_name[reason];
  static char buf[20];
  sprintf(buf, "reason%d", reason);
  return buf;
}
const char* Deoptimization::trap_action_name(int action) {
  if ((uint)action < Action_LIMIT)
    return _trap_action_name[action];
  static char buf[20];
  sprintf(buf, "action%d", action);
  return buf;
}

// This is used for debugging and diagnostics, including hotspot.log output.
const char* Deoptimization::format_trap_request(char* buf, size_t buflen,
                                                int trap_request) {
  jint unloaded_class_index = trap_request_index(trap_request);
  const char* reason = trap_reason_name(trap_request_reason(trap_request));
  const char* action = trap_action_name(trap_request_action(trap_request));
  size_t len;
  if (unloaded_class_index < 0) {
    len = jio_snprintf(buf, buflen, "reason='%s' action='%s'",
                       reason, action);
  } else {
    len = jio_snprintf(buf, buflen, "reason='%s' action='%s' index='%d'",
                       reason, action, unloaded_class_index);
  }
  if (len >= buflen)
    buf[buflen-1] = '\0';
  return buf;
}

juint Deoptimization::_deoptimization_hist
        [Deoptimization::Reason_LIMIT]
    [1 + Deoptimization::Action_LIMIT]
        [Deoptimization::BC_CASE_LIMIT]
  = {0};

enum {
  LSB_BITS = 8,
  LSB_MASK = right_n_bits(LSB_BITS)
};

void Deoptimization::gather_statistics(DeoptReason reason, DeoptAction action,
                                       Bytecodes::Code bc) {
  assert(reason >= 0 && reason < Reason_LIMIT, "oob");
  assert(action >= 0 && action < Action_LIMIT, "oob");
  _deoptimization_hist[Reason_none][0][0] += 1;  // total
  _deoptimization_hist[reason][0][0]      += 1;  // per-reason total
  juint* cases = _deoptimization_hist[reason][1+action];
  juint* bc_counter_addr = NULL;
  juint  bc_counter      = 0;
  // Look for an unused counter, or an exact match to this BC.
  if (bc != Bytecodes::_illegal) {
    for (int bc_case = 0; bc_case < BC_CASE_LIMIT; bc_case++) {
      juint* counter_addr = &cases[bc_case];
      juint  counter = *counter_addr;
      if ((counter == 0 && bc_counter_addr == NULL)
          || (Bytecodes::Code)(counter & LSB_MASK) == bc) {
        // this counter is either free or is already devoted to this BC
        bc_counter_addr = counter_addr;
        bc_counter = counter | bc;
      }
    }
  }
  if (bc_counter_addr == NULL) {
    // Overflow, or no given bytecode.
    bc_counter_addr = &cases[BC_CASE_LIMIT-1];
    bc_counter = (*bc_counter_addr & ~LSB_MASK);  // clear LSB
  }
  *bc_counter_addr = bc_counter + (1 << LSB_BITS);
}

jint Deoptimization::total_deoptimization_count() {
  return _deoptimization_hist[Reason_none][0][0];
}

jint Deoptimization::deoptimization_count(DeoptReason reason) {
  assert(reason >= 0 && reason < Reason_LIMIT, "oob");
  return _deoptimization_hist[reason][0][0];
}

void Deoptimization::print_statistics() {
  juint total = total_deoptimization_count();
  juint account = total;
  if (total != 0) {
    tty->print_cr("Deoptimization traps recorded:");
    ttyLocker ttyl;
    #define PRINT_STAT_LINE(name, r) \
      tty->print_cr("  %4d (%4.1f%%) %s", (int)(r), ((r) * 100.0) / total, name);
    PRINT_STAT_LINE("total", total);
    // For each non-zero entry in the histogram, print the reason,
    // the action, and (if specifically known) the type of bytecode.
    for (int reason = 0; reason < Reason_LIMIT; reason++) {
      for (int action = 0; action < Action_LIMIT; action++) {
        juint* cases = _deoptimization_hist[reason][1+action];
        for (int bc_case = 0; bc_case < BC_CASE_LIMIT; bc_case++) {
          juint counter = cases[bc_case];
          if (counter != 0) {
            char name[1*K];
            Bytecodes::Code bc = (Bytecodes::Code)(counter & LSB_MASK);
            if (bc_case == BC_CASE_LIMIT && (int)bc == 0)
              bc = Bytecodes::_illegal;
            sprintf(name, "%s/%s/%s",
                    trap_reason_name(reason),
                    trap_action_name(action),
                    Bytecodes::is_defined(bc)? Bytecodes::name(bc): "other");
            juint r = counter >> LSB_BITS;
            tty->print_cr("  %40s: " UINT32_FORMAT " (%.1f%%)", name, r, (r * 100.0) / total);
            account -= r;
          }
        }
      }
    }
    if (account != 0) {
      PRINT_STAT_LINE("unaccounted", account);
    }
    #undef PRINT_STAT_LINE
  }
}

#else // COMPILER2

// Stubs for C1 system.
bool Deoptimization::trap_state_is_recompiled(int trap_state) {
  return false;
}

const char* Deoptimization::trap_reason_name(int reason) {
  return "unknown";
}

void Deoptimization::print_statistics() {
  // no output
}

void
Deoptimization::update_method_data_from_interpreter(methodDataHandle trap_mdo, int trap_bci, int reason) {
  // no udpate
}

int Deoptimization::trap_state_has_reason(int trap_state, int reason) {
  return 0;
}

void Deoptimization::gather_statistics(DeoptReason reason, DeoptAction action,
                                       Bytecodes::Code bc) {
  // no update
}

const char* Deoptimization::format_trap_state(char* buf, size_t buflen,
                                              int trap_state) {
  jio_snprintf(buf, buflen, "#%d", trap_state);
  return buf;
}

#endif // COMPILER2
