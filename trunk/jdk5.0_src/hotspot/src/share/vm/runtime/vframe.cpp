#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)vframe.cpp	1.134 04/03/29 14:12:32 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_vframe.cpp.incl"

vframe::vframe(const frame* fr, const RegisterMap* reg_map, JavaThread* thread) 
: _reg_map(reg_map), _thread(thread) {
  assert(fr != NULL, "must have frame");
  _fr = *fr;
}

vframe::vframe(const frame* fr, JavaThread* thread) 
: _reg_map(thread), _thread(thread) {
  assert(fr != NULL, "must have frame");
  _fr = *fr;
}

vframe* vframe::new_vframe(const frame* f, const RegisterMap* reg_map, JavaThread* thread) {
  // Interpreter frame
  if (f->is_interpreted_frame()) {
    return new interpretedVFrame(f, reg_map, thread);
  }  

#ifndef CORE
  // Compiled frame
  CodeBlob* cb = CodeCache::find_blob(f->pc());
  if (cb != NULL) {  
    if (cb->is_nmethod()) {      
      nmethod* nm = (nmethod*)cb;            
      // Compiled method (native stub or Java code)
      ScopeDesc* scope  = nm->scope_desc_at(f->pc(), reg_map->is_pc_at_call(f->id()));
      return new compiledVFrame(f, reg_map, thread, scope);  
    }

    if (f->is_glue_frame()) {
      // This is a conversion frame. Skip this frame and try again.      
      RegisterMap temp_map = *reg_map;
      frame s = f->sender(&temp_map);
      return new_vframe(&s, &temp_map, thread);
    }
  }
#endif

  // External frame
  return new externalVFrame(f, reg_map, thread);
}

vframe* vframe::sender() const {
  RegisterMap temp_map = *register_map();
  assert(is_top(), "just checking"); 
  if (_fr.is_entry_frame() && _fr.is_first_frame()) return NULL;
  frame s = _fr.real_sender(&temp_map);
  if (s.is_first_frame()) return NULL;
  return vframe::new_vframe(&s, &temp_map, thread());
}

vframe* vframe::top() const {
  vframe* vf = (vframe*) this;
  while (!vf->is_top()) vf = vf->sender();
  return vf;
}


javaVFrame* vframe::java_sender() const {
  vframe* f = sender();
  while (f != NULL) {
    if (f->is_java_frame()) return javaVFrame::cast(f);
    f = f->sender();
  }
  return NULL;
}

// ------------- javaVFrame --------------

//
// Fabricate heavyweight monitor information for each lightweight monitor
// found in the Java VFrame.
//
void javaVFrame::jvmpi_fab_heavy_monitors(bool query, int* fab_index, int frame_count, GrowableArray<ObjectMonitor*>* fab_list) {
  assert(SafepointSynchronize::is_at_safepoint(), "must be at safepoint");
  ResourceMark rm;

  GrowableArray<MonitorInfo*>* mons = monitors();
  if (mons->is_empty()) return;

  bool found_first_monitor = false;
  for (int index = (mons->length()-1); index >= 0; index--) {
    MonitorInfo* monitor = mons->at(index);
    if (monitor->owner() == NULL) continue; // skip unowned monitor
    //
    // If we haven't found a monitor before, this is the first frame, and
    // the thread is blocked, then we are trying to enter this monitor.
    // We skip it because we have already seen it before from the monitor 
    // cache walk.
    //
    if (!found_first_monitor && frame_count == 0) {
      switch (thread()->thread_state()) {
      case _thread_blocked:
      case _thread_blocked_trans:
        continue;
      }
    }
    found_first_monitor = true;

    markOop mark = monitor->owner()->mark();
    if (mark->has_locker()) {
      if (!query) {   // not just counting so create and store at the current element
        // fabricate the heavyweight monitor from lightweight info
        ObjectMonitor *heavy = new ObjectMonitor();
        heavy->set_object(monitor->owner());  // use the owning object
        heavy->set_owner(thread());           // use thread instead of stack address for speed
        fab_list->at_put(*fab_index, heavy);
      }
      (*fab_index)++;
    }
  }
}


void javaVFrame::print_lock_info(int frame_count) {        
  ResourceMark rm;

  // If this is the first frame, and java.lang.Object.wait(...) then print out the receiver.
  if (frame_count == 0 && method()->name() == vmSymbols::wait_name() && 
      instanceKlass::cast(method()->method_holder())->name() == vmSymbols::java_lang_Object()) {
    StackValueCollection* locs = locals();      
    if (!locs->is_empty()) {
      StackValue* sv = locs->at(0);
      if (sv->type() == T_OBJECT) {
        Handle o = locs->at(0)->get_obj();    
        if (o.not_null()) {
          instanceKlass* ik = instanceKlass::cast(o->klass());
          tty->print_cr("\t- waiting on <" INTPTR_FORMAT "> (a %s)", o(), ik->external_name());
        }
      }
    }        
  }    
  
  // Print out all monitors that we have locked or are trying to lock
  GrowableArray<MonitorInfo*>* mons = monitors();
  if (!mons->is_empty()) {
    bool found_first_monitor = false;
    for (int index = (mons->length()-1); index >= 0; index--) {
      MonitorInfo* monitor = mons->at(index);
      if (monitor->owner() != NULL) {
        //
        // First, assume we have the monitor locked. If we haven't found an
        // owned monitor before and this is the first frame, then we need to
        // see if the thread is blocked.
        //
        const char *lock_state = "locked"; // assume we have the monitor locked
        if (!found_first_monitor && frame_count == 0) {
          switch (thread()->thread_state()) {
          case _thread_blocked:
          case _thread_blocked_trans:
            lock_state = "waiting to lock";
            break;
          }
        }
        found_first_monitor = true;
        instanceKlass* ik = instanceKlass::cast(monitor->owner()->klass());
        tty->print_cr("\t- %s <" INTPTR_FORMAT "> (a %s)", lock_state, monitor->owner(), ik->external_name());
      }
    }  
  }
}

// ------------- interpretedVFrame --------------

u_char* interpretedVFrame::bcp() const {
  return fr().interpreter_frame_bcp();
}

void interpretedVFrame::set_bcp(u_char* bcp) {
  fr().interpreter_frame_set_bcp(bcp);
}

intptr_t* interpretedVFrame::locals_addr_at(int offset) const {
  assert(fr().is_interpreted_frame(), "frame should be an interpreted frame");
  return &fr().interpreter_frame_local_at(offset);
}


intptr_t* interpretedVFrame::expression_stack_addr_at(int offset) const {
  return &fr().interpreter_frame_expression_stack_at(offset);
}

GrowableArray<MonitorInfo*>* interpretedVFrame::monitors() const {
  GrowableArray<MonitorInfo*>* result = new GrowableArray<MonitorInfo*>(5);
  for (BasicObjectLock* current = (fr().previous_monitor_in_interpreter_frame(fr().interpreter_frame_monitor_begin()));
       current >= fr().interpreter_frame_monitor_end();
       current = fr().previous_monitor_in_interpreter_frame(current)) {
    result->push(new MonitorInfo(current->obj(), current->lock()));
  }
  return result;
}

int interpretedVFrame::bci() const {
  return method()->bci_from(bcp());
}

methodOop interpretedVFrame::method() const {
  return fr().interpreter_frame_method();
}

StackValueCollection* interpretedVFrame::locals() const {
  int length = method()->max_locals();
 
  if (method()->is_native()) {
    // If the method is native, max_locals is not telling the truth.
    // maxlocals then equals the size of parameters 
    length = method()->size_of_parameters();
  }

  StackValueCollection* result = new StackValueCollection(length);

  // Get oopmap describing oops and int for current bci
  InterpreterOopMap oop_mask;
  if (TraceDeoptimization && Verbose) {
    methodHandle m_h(thread(), method());
    OopMapCache::compute_one_oop_map(m_h, bci(), &oop_mask);
  } else {
    method()->mask_for(bci(), &oop_mask);
  }

  // handle locals
  for(int i=0; i < length; i++) {
    // Find stack location
    intptr_t *addr = locals_addr_at(i); 

    // Depending on oop/int put it in the right package
    StackValue *sv;    
    if (oop_mask.is_oop(i)) {
      // oop value
      Handle h(*(oop *)addr);
      sv = new StackValue(h);
    } else {
      // integer
      sv = new StackValue(*addr);
    }
    assert(sv != NULL, "sanity check");
    result->add(sv);
  }

  return result;
}

void interpretedVFrame::set_locals(StackValueCollection* values) const {
  if (values == NULL || values->size() == 0) return;

  int length = method()->max_locals();
  if (method()->is_native()) {
    // If the method is native, max_locals is not telling the truth.
    // maxlocals then equals the size of parameters 
    length = method()->size_of_parameters();
  }

  assert(length == values->size(), "Mismatch between actual stack format and supplied data");

  // Get oopmap describing oops and int for current bci
  InterpreterOopMap oop_mask;
  if (TraceDeoptimization && Verbose) {
    methodHandle m_h(thread(), method());
    OopMapCache::compute_one_oop_map(m_h, bci(), &oop_mask);
  } else {
    method()->mask_for(bci(), &oop_mask);
  }

  // handle locals
  for (int i = 0; i < length; i++) {
    // Find stack location
    intptr_t *addr = locals_addr_at(i); 

    // Depending on oop/int put it in the right package
    StackValue *sv = values->at(i);
    assert(sv != NULL, "sanity check");
    if (oop_mask.is_oop(i)) { // oop value
      *(oop *) addr = (sv->get_obj())();
    } else {                   // integer
      *addr = sv->get_int();
    }
  }
}

StackValueCollection*  interpretedVFrame::expressions() const {
  int length = fr().interpreter_frame_expression_stack_size();
  if (method()->is_native()) {
    // If the method is native, there is no expression stack
    length = 0;
  }

  int nof_locals = method()->max_locals();
  StackValueCollection* result = new StackValueCollection(length);

  // Get oopmap describing oops and int for current bci
  InterpreterOopMap oop_mask;
  if (TraceDeoptimization && Verbose) {
    methodHandle m_h(thread(), method());
    OopMapCache::compute_one_oop_map(m_h, bci(), &oop_mask);
  } else {
    method()->mask_for(bci(), &oop_mask);
  }
  
  // handle locals
  for(int i=0; i < length; i++) {
    // Find stack location
    intptr_t *addr = expression_stack_addr_at(i);           

    // Depending on oop/int put it in the right package
    StackValue *sv;    
    if (oop_mask.is_oop(i + nof_locals)) {
      // oop value
      Handle h(*(oop *)addr);
      sv = new StackValue(h);
    } else {
      // integer
      sv = new StackValue(*addr);
    }    
    assert(sv != NULL, "sanity check");
    result->add(sv);
  }

  return result;
}


// ------------- cChunk --------------

entryVFrame::entryVFrame(const frame* fr, const RegisterMap* reg_map, JavaThread* thread)
: externalVFrame(fr, reg_map, thread) {}


#ifndef CORE
bool vframeStreamCommon::fill_in_compiled_inlined_sender() {
  if (_sender_decode_offset == DebugInformationRecorder::serialized_null) {
    return false;
  }
  fill_from_compiled_frame(_code, _sender_decode_offset);
  return true;
}


void vframeStreamCommon::fill_from_compiled_frame(nmethod* code, int decode_offset) {
  _mode = compiled_mode;
  _code = code;

  // Decode first part of scopeDesc
  u_char* buffer = code->scopes_data_begin() + decode_offset;
  _sender_decode_offset = CompressedReadStream::raw_read_int(buffer);
  _method               = methodOop(_code->oop_at(CompressedReadStream::raw_read_int(buffer)));
  _bci                  = CompressedReadStream::raw_read_int(buffer);

  assert(_method->is_method(), "checking type of decoded method");  
}


// The native frames are handled specially. We do not rely on ScopeDesc info
// since the pc might not be exact due to the _last_native_pc trick.
void vframeStreamCommon::fill_from_compiled_native_frame(nmethod* code) {
  _mode = compiled_mode;
  _code = code;
  _sender_decode_offset = DebugInformationRecorder::serialized_null; 
  _method = code->method();
  _bci = 0;
}

#endif // CORE


void vframeStreamCommon::fill_from_interpreter_frame() {
  _mode = interpreted_mode;
  _method = _frame.interpreter_frame_method();
  _bci    = _method->bci_from(_frame.interpreter_frame_bcp());
}


bool vframeStreamCommon::fill_from_frame() {  
  // Interpreted frame
  if (_frame.is_interpreted_frame()) {
    fill_from_interpreter_frame();
    return true;
  }

  // Compiled frame  

#ifndef CORE
  CodeBlob* code = CodeCache::find_blob(_frame.pc());
  if (code != NULL && code->is_nmethod()) {
    nmethod* nm = (nmethod*)code;
    if (nm->is_native_method()) {
      // Do not rely on scopeDesc since the pc might be unprecise due to the _last_native_pc trick.
      fill_from_compiled_native_frame(nm);
    } else {    
      bool at_call = _reg_map.is_pc_at_call(_frame.id());
      PcDesc* pc_desc = nm->pc_desc_at(_frame.pc(), at_call);
#ifdef ASSERT
      if (pc_desc == NULL) {
        tty->print_cr("Error in fill_from_frame: pc_desc for " INTPTR_FORMAT " not found", _frame.pc());
        nm->print();
        nm->method()->print_codes();
        nm->print_code();
        nm->print_pcs();
      }
#endif
      assert(pc_desc != NULL, "scopeDesc must exist");
      fill_from_compiled_frame(nm, pc_desc->scope_decode_offset());
    }
    return true;
  }
#endif

  // End of stack?
  if (_frame.is_first_frame() || (_stop_at_java_call_stub && _frame.is_entry_frame())) {
    _mode = at_end_mode;
    return true;
  }    

  return false;
}


vframeStream::vframeStream(JavaThread* thread, bool stop_at_java_call_stub)
  : vframeStreamCommon(thread) {
  _stop_at_java_call_stub = stop_at_java_call_stub;

  if (!thread->has_last_Java_frame()) {
    _mode = at_end_mode;
    return;
  }

  _frame = _thread->last_frame();
  while (!fill_from_frame()) {
    _frame = _frame.sender(&_reg_map);
  }
}


// top-frame will be skipped
vframeStream::vframeStream(JavaThread* thread, frame top_frame,
  bool stop_at_java_call_stub) : vframeStreamCommon(thread) {
  _stop_at_java_call_stub = stop_at_java_call_stub;

  // skip top frame, as it may not be at safepoint
  _frame  = top_frame.sender(&_reg_map);
  while (!fill_from_frame()) {
    _frame = _frame.sender(&_reg_map);
  }
}


void vframeStreamCommon::next() {
  // handle frames with inlining
#ifndef CORE
  if (_mode == compiled_mode    && fill_in_compiled_inlined_sender()) return;
#endif

  // handle general case
  nmethod* code = _mode == compiled_mode ? _code : NULL;
  do {
    _frame = _frame.sender(&_reg_map, (CodeBlob*)code);    
    code = NULL; // After first iteration, we got no cached nmethod
  } while (!fill_from_frame());
}


// Step back n frames, skip and pseudo frames in between.
// This function is used in Class.forName, Class.newInstance, Method.Invoke, 
// AccessController.doPrivileged.
//
// NOTE that in JDK 1.4 this has been exposed to Java as
// sun.reflect.Reflection.getCallerClass(), which can be inlined.
// Inlined versions must match this routine's logic. See, for example,
// Parse::inline_native_Reflection_getCallerClass in
// opto/library_call.cpp.
void vframeStreamCommon::security_get_caller_frame(int depth) {    
  while (depth-- > 0) {
    // skip Method.invoke() and auxiliary frames
    skip_method_invoke_and_aux_frames();
    if (at_end()) return;    
    // Skip real frame
    next();
  }

  // skip Method.invoke() and auxiliary frames
  skip_method_invoke_and_aux_frames();
}
  

void vframeStreamCommon::skip_method_invoke_and_aux_frames() {
  while (!at_end() &&
         (method() == Universe::reflect_invoke_method()
          || (Universe::is_gte_jdk14x_version() && UseNewReflection &&
              Klass::cast(method()->method_holder())
              ->is_subclass_of(SystemDictionary::reflect_method_accessor_klass())))) {
    next();
  }
}


void vframeStreamCommon::skip_reflection_related_frames() {
  while (!at_end() &&
         (Universe::is_gte_jdk14x_version() && UseNewReflection &&
          (Klass::cast(method()->method_holder())->is_subclass_of(SystemDictionary::reflect_method_accessor_klass()) ||
           Klass::cast(method()->method_holder())->is_subclass_of(SystemDictionary::reflect_constructor_accessor_klass())))) {
    next();
  }
}


#ifndef PRODUCT
void vframe::print() {
  if (WizardMode) _fr.print_value_on(tty,NULL);
}


void vframe::print_value() const {
  ((vframe*)this)->print();
}

 
void entryVFrame::print_value() const { 
  ((entryVFrame*)this)->print();
}

void entryVFrame::print() {
  vframe::print();
  tty->print_cr("C Chunk inbetween Java");
  tty->print_cr("C     link " INTPTR_FORMAT, _fr.link());
}


// ------------- javaVFrame --------------

static void print_stack_values(const char* title, StackValueCollection* values) {
  if (values->is_empty()) return;
  tty->print_cr("\t%s:", title);
  values->print();
}


void javaVFrame::print() {
  ResourceMark rm;
  vframe::print();
  tty->print("\t"); 
  method()->print_value();
  tty->cr();
  tty->print_cr("\tbci:    %d", bci());

  print_stack_values("locals",      locals());
  print_stack_values("expressions", expressions());

  GrowableArray<MonitorInfo*>* list = monitors();
  if (list->is_empty()) return;
  tty->print_cr("\tmonitor list:");
  for (int index = (list->length()-1); index >= 0; index--) {
    MonitorInfo* monitor = list->at(index);
    tty->print("\t  obj\t"); monitor->owner()->print_value(); 
    tty->print("(" INTPTR_FORMAT ")", monitor->owner());
    tty->cr();
    tty->print("\t  ");
    monitor->lock()->print_on(tty);
    tty->cr(); 
  }
}


void javaVFrame::print_value() const {
  methodOop  m = method();
  klassOop   k = m->method_holder();
  tty->print("%s.%s", Klass::cast(k)->internal_name(), m->name()->as_C_string());

  if (!m->is_native()) {
    symbolOop  source_name = instanceKlass::cast(k)->source_file_name();
    int        line_number = m->line_number_from_bci(bci());
    if (source_name != NULL && (line_number != -1)) {
      tty->print("(%s:%d)", source_name->as_C_string(), line_number);
    }
  } else {
    tty->print("(Native Method)");
  }
  // Check frame size and print warning if it looks suspiciously large
  if (fr().sp() != NULL) {
    int size = fr().frame_size();
#ifdef _LP64
    if (size > 8*K) warning("SUSPICIOUSLY LARGE FRAME (%d)", size);
#else
    if (size > 4*K) warning("SUSPICIOUSLY LARGE FRAME (%d)", size);
#endif
  }
}


bool javaVFrame::structural_compare(javaVFrame* other) {
  // Check static part
  if (method() != other->method()) return false;
  if (bci()    != other->bci())    return false;

  // Check locals
  StackValueCollection *locs = locals();
  StackValueCollection *other_locs = other->locals();
  assert(locs->size() == other_locs->size(), "sanity check");
  int i;
  for(i = 0; i < locs->size(); i++) {
    // it might happen the compiler reports a conflict and
    // the interpreter reports a bogus int.
    if (       is_compiled_frame() &&       locs->at(i)->type() == T_CONFLICT) continue;
    if (other->is_compiled_frame() && other_locs->at(i)->type() == T_CONFLICT) continue;

    if (!locs->at(i)->equal(other_locs->at(i)))
      return false;
  }

  // Check expressions
  StackValueCollection* exprs = expressions();
  StackValueCollection* other_exprs = other->expressions();
  assert(exprs->size() == other_exprs->size(), "sanity check");
  for(i = 0; i < exprs->size(); i++) {
    if (!exprs->at(i)->equal(other_exprs->at(i)))
      return false;
  }

  return true;
}


void javaVFrame::print_activation(int index) const {
  // frame number and method
  tty->print("%2d - ", index);
  ((vframe*)this)->print_value();
  tty->cr();
    
  if (WizardMode) {
    ((vframe*)this)->print();
    tty->cr();
  }
}


void javaVFrame::verify() const {
}


void interpretedVFrame::verify() const {
}


// ------------- externalVFrame --------------

void externalVFrame::print() {
  _fr.print_value_on(tty,NULL);
}


void externalVFrame::print_value() const {
  ((vframe*)this)->print();
}
#endif // PRODUCT
