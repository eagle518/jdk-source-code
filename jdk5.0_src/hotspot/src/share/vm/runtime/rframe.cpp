#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)rframe.cpp	1.32 03/12/23 16:44:06 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"

#include "incls/_rframe.cpp.incl"

static RFrame*const  noCaller    = (RFrame*) 0x1;		// no caller (i.e., initial frame)
static RFrame*const  noCallerYet = (RFrame*) 0x0;		// caller not yet computed

RFrame::RFrame(frame fr, JavaThread* thread, RFrame*const callee) : 
  _fr(fr), _thread(thread), _callee(callee), _num(callee ? callee->num() + 1 : 0) {
  _caller = (RFrame*)noCallerYet;
  _invocations = 0;
  _distance = 0;
}

void RFrame::set_distance(int d) { 
  assert(is_compiled() || d >= 0, "should be positive");
  _distance = d; 
}

InterpretedRFrame::InterpretedRFrame(frame fr, JavaThread* thread, RFrame*const callee)
: RFrame(fr, thread, callee) {
  RegisterMap map(thread, false);
  _vf     = javaVFrame::cast(vframe::new_vframe(&_fr, &map, thread));
  _method = methodHandle(thread, _vf->method());
  assert(   _vf->is_interpreted_frame(), "must be interpreted");
  init();
}

InterpretedRFrame::InterpretedRFrame(frame fr, JavaThread* thread, methodHandle m)
: RFrame(fr, thread, NULL) {
  RegisterMap map(thread, false);
  _vf     = javaVFrame::cast(vframe::new_vframe(&_fr, &map, thread));
  _method = m;

  assert(   _vf->is_interpreted_frame(),  "must be interpreted");
  init();
}

CompiledRFrame::CompiledRFrame(frame fr, JavaThread* thread, RFrame*const  callee)
: RFrame(fr, thread, callee) { 
  init();
}

CompiledRFrame::CompiledRFrame(frame fr, JavaThread* thread)
: RFrame(fr, thread, NULL) { 
  init();
}

DeoptimizedRFrame::DeoptimizedRFrame(frame fr, JavaThread* thread, RFrame*const  callee)
: InterpretedRFrame(fr, thread, callee) {}

RFrame* RFrame::new_RFrame(frame fr, JavaThread* thread, RFrame*const  callee) {
  RFrame* rf;
  int dist = callee ? callee->distance() : -1;
  bool is_deopted;
  if (fr.is_interpreted_frame()) {
    rf = new InterpretedRFrame(fr, thread, callee);
    dist++;
  } else if (fr.is_compiled_frame(&is_deopted)) { 
    // Even deopted frames look compiled because the deopt
    // is invisible until it happens.
    rf = new CompiledRFrame(fr, thread, callee);
  } else {
    assert(false, "Unhandled frame type");
  }
  rf->set_distance(dist);
  rf->init();
  return rf;
}

RFrame* RFrame::caller() {
  if (_caller != noCallerYet) return (_caller == noCaller) ? NULL : _caller;	// already computed caller
  
  // caller not yet computed; do it now
  if (_fr.is_first_java_frame()) {
    _caller = (RFrame*)noCaller;
    return NULL;
  }

  RegisterMap map(_thread, false);
  frame sender = _fr.real_sender(&map);
  if (sender.is_java_frame()) {
    _caller = new_RFrame(sender, thread(), this);
    return _caller;
  }

  // Real caller is not java related
  _caller = (RFrame*)noCaller;
  return NULL;
}

int InterpretedRFrame::cost() const {
  return _method->code_size();    // fix this
  //return _method->estimated_inline_cost(_receiverKlass);
}

int CompiledRFrame::cost() const {
  return top_method()->code()->code_size();
}

void CompiledRFrame::init() {
  RegisterMap map(thread(), false);
  vframe* vf = vframe::new_vframe(&_fr, &map, thread());
  assert(vf->is_compiled_frame(), "must be compiled");
  _nm = compiledVFrame::cast(vf)->code();
  vf = vf->top();
  _vf = javaVFrame::cast(vf);  
  _method = methodHandle(thread(), CodeCache::find_nmethod(_fr.pc())->method());
  assert(_method(), "should have found a method");
#ifndef PRODUCT
#ifndef CORE
  _invocations = _method->compiled_invocation_count();
#endif
#endif
}

void InterpretedRFrame::init() {
  _invocations = _method->invocation_count() + _method->backedge_count();
}

void RFrame::print(const char* kind) {
#ifndef PRODUCT
#ifdef COMPILER2
  int cnt = top_method()->interpreter_invocation_count();
#else
  int cnt = top_method()->invocation_count();
#endif
  tty->print("%3d %s ", _num, is_interpreted() ? "I" : "C"); 
  top_method()->print_short_name(tty);
  tty->print_cr(": inv=%5d(%d) cst=%4d", _invocations, cnt, cost());
#endif
}

void CompiledRFrame::print() {
  RFrame::print("comp");
}

void InterpretedRFrame::print() {
  RFrame::print("int.");
}

void DeoptimizedRFrame::print() {
  RFrame::print("deopt.");
}

