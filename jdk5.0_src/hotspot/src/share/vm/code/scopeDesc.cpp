#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)scopeDesc.cpp	1.47 03/12/23 16:39:58 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_scopeDesc.cpp.incl"


ScopeDesc::ScopeDesc(const nmethod* code, int decode_offset) {
  _code          = code;
  _decode_offset = decode_offset;

  // decode header
  DebugInfoReadStream* stream  = stream_at(_decode_offset);

  _sender_decode_offset = stream->read_int();
  _method = methodHandle((methodOop) stream->read_handle()());
  _bci    = stream->read_int();
  // decode offsets for body and sender
  _locals_decode_offset      = stream->read_int();
  _expressions_decode_offset = stream->read_int();
  _monitors_decode_offset    = stream->read_int();
}


GrowableArray<ScopeValue*>* ScopeDesc::decode_scope_values(int decode_offset) {
  if (decode_offset == DebugInformationRecorder::serialized_null) return NULL;
  DebugInfoReadStream* stream  = stream_at(decode_offset);
  int length = stream->read_int();
  GrowableArray<ScopeValue*>* result = new GrowableArray<ScopeValue*> (length);
  for (int index = 0; index < length; index++) {
    result->push(ScopeValue::read_from(stream));
  }
  return result;
}


GrowableArray<MonitorValue*>* ScopeDesc::decode_monitor_values(int decode_offset) {
  if (decode_offset == DebugInformationRecorder::serialized_null) return NULL;
  DebugInfoReadStream* stream  = stream_at(decode_offset);
  int length = stream->read_int();
  GrowableArray<MonitorValue*>* result = new GrowableArray<MonitorValue*> (length);
  for (int index = 0; index < length; index++) {
    result->push(new MonitorValue(stream));
  }
  return result;
}

DebugInfoReadStream* ScopeDesc::stream_at(int decode_offset) const {
  return new DebugInfoReadStream(_code, decode_offset);
}

GrowableArray<ScopeValue*>* ScopeDesc::locals() {
  return decode_scope_values(_locals_decode_offset);
}

GrowableArray<ScopeValue*>* ScopeDesc::expressions() {
  return decode_scope_values(_expressions_decode_offset);
}

GrowableArray<MonitorValue*>* ScopeDesc::monitors() {
  return decode_monitor_values(_monitors_decode_offset);
}

bool ScopeDesc::is_top() const {
 return _sender_decode_offset == DebugInformationRecorder::serialized_null;
}

ScopeDesc* ScopeDesc::sender() const {
  if (is_top()) return NULL;
  return new ScopeDesc(_code, _sender_decode_offset);
}


#ifndef PRODUCT

void ScopeDesc::print_value_on(outputStream* st) const {
  st->print("(0x%lx)", method()());
  method()()->print_name(st);
  int lineno = method()->line_number_from_bci(bci());
  if (lineno != -1) {
    st->print_cr("@%d (line %d", bci(), lineno);
  } else {
    st->print_cr("@%d", bci());
  }
}

void ScopeDesc::print_on(outputStream* st) const {
  // header
  st->print("ScopeDesc[%d]@0x%lx ", _decode_offset, _code->instructions_begin());
  print_value_on(st);
  // decode offsets
  if (WizardMode) {
    st->print_cr("offset:     %d",    _decode_offset);
    st->print_cr("bci:        %d",    bci());
    st->print_cr("locals:     %d",    _locals_decode_offset);
    st->print_cr("stack:      %d",    _expressions_decode_offset);
    st->print_cr("monitor:    %d",    _monitors_decode_offset);
    st->print_cr("sender:     %d",    _sender_decode_offset);
  }
  // locals
  { GrowableArray<ScopeValue*>* l = ((ScopeDesc*) this)->locals();
    if (l != NULL) {
      tty->print_cr("Locals");
      for (int index = 0; index < l->length(); index++) {
        st->print(" - l%d: ", index);
        l->at(index)->print_on(st);
        st->cr();
      }
    }
  }
  // expressions
  { GrowableArray<ScopeValue*>* l = ((ScopeDesc*) this)->expressions();
    if (l != NULL) {
      st->print_cr("Expression stack");
      for (int index = 0; index < l->length(); index++) {
        st->print(" - @%d: ", index);
        l->at(index)->print_on(st);
        st->cr();
      }
    }
  }
  // monitors
  { GrowableArray<MonitorValue*>* l = ((ScopeDesc*) this)->monitors();
    if (l != NULL) {
      st->print_cr("Monitor stack");
      for (int index = 0; index < l->length(); index++) {
        st->print(" - @%d: ", index);
        l->at(index)->print_on(st);
        st->cr();
      }
    }
  }

  if (!is_top()) {
    st->print_cr("Sender:");
    sender()->print_on(st);
  }
}

void ScopeDesc::verify() {
  ResourceMark rm;
  guarantee(method()->is_method(), "type check");

  // check if we have any illegal elements on the expression stack
  { GrowableArray<ScopeValue*>* l = expressions();
    if (l != NULL) {
      for (int index = 0; index < l->length(); index++) {
       //guarantee(!l->at(index)->is_illegal(), "expression element cannot be illegal");
      }
    }
  }
}

#endif
