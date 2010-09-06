#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)debugInfoRec.cpp	1.42 03/12/23 16:39:51 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_debugInfoRec.cpp.incl"

// Structure for generating PcDesc
class PcDescNode VALUE_OBJ_CLASS_SPEC  {
 private:
  int  _pc_offset;
  int  _stream_offset;
  bool _at_call;

 public:
  PcDescNode(int pc_offset, bool at_call, int stream_offset) {
   _pc_offset     = pc_offset;
   _at_call       = at_call;
   _stream_offset = stream_offset;
  }

  int  pc_offset()     const { return _pc_offset; }
  int  stream_offset() const { return _stream_offset; }
  void set_stream_offset(int offset) { _stream_offset = offset; }
  bool at_call()       const { return _at_call; }
};


void DebugInformationRecorder::check_phase(DebugInformationRecorder::Phase new_phase) {
  assert(phase <= new_phase, "information must be generated in right order");
  phase = new_phase;
}


DebugInformationRecorder::DebugInformationRecorder(OopRecorder* oop_recorder) {
  phase = describe_dependents;

  _pcs_size   = 100;
  _pcs        = NEW_RESOURCE_ARRAY(PcDescNode, _pcs_size);
  _pcs_length = 0;

  _stream = new DebugInfoWriteStream(this, 10 * K);
  // make sure that there is no stream_decode_offset that is zero, because
  // we use the signed decode offset to determine if a PCDesc is at_call or not
  _stream->write_byte((jbyte)0xFF); 

  _oop_recorder = oop_recorder;

  _first_dependent      = 0;
  _number_of_dependents = 0;
}


void DebugInformationRecorder::add_oopmap(int pc_offset, bool at_call, OopMap* map) {
  // !!!!! Preserve old style handling of oopmaps for now
  _oopmaps->add_gc_map(pc_offset, at_call, map);
}

void DebugInformationRecorder::add_safepoint(int pc_offset, bool at_call, OopMap* map) {
  check_phase(describe_safepoints);
  // Store the new safepoint

  // Add the oop map
  add_oopmap(pc_offset, at_call, map);

  // add the pcdesc
  if (_pcs_length == _pcs_size) {
    // Expand
    int         new_pcs_size = _pcs_size * 2;
    PcDescNode* new_pcs      = NEW_RESOURCE_ARRAY(PcDescNode, new_pcs_size);
    for (int index = 0; index < _pcs_length; index++) {
      new_pcs[index] = _pcs[index];
    }
    _pcs_size = new_pcs_size;
    _pcs      = new_pcs;
  }
  assert(_pcs_size > _pcs_length, "There must be room for after expanding");
  _pcs[_pcs_length++] = PcDescNode(pc_offset, at_call, DebugInformationRecorder::serialized_null);
}


int DebugInformationRecorder::serialize_monitor_values(GrowableArray<MonitorValue*>* monitors) {
  if (monitors == NULL || monitors->is_empty()) return DebugInformationRecorder::serialized_null;
  int result = stream()->position();
  stream()->write_int(monitors->length());
  for (int index = 0; index < monitors->length(); index++) {
    monitors->at(index)->write_on(stream());
  }
  return result;
}


int DebugInformationRecorder::serialize_scope_values(GrowableArray<ScopeValue*>* values) {
  if (values == NULL || values->is_empty()) return DebugInformationRecorder::serialized_null;
  int result = stream()->position();
  stream()->write_int(values->length());
  for (int index = 0; index < values->length(); index++) {
    values->at(index)->write_on(stream());
  }
  return result;
}


// must call add_safepoint before: it sets PcDesc and this routine uses
// the last PcDesc set
void DebugInformationRecorder::describe_scope(ciMethod*  method,
                                              int         bci,
                                              DebugToken* locals,
                                              DebugToken* expressions,
                                              DebugToken* monitors) {
  check_phase(describe_safepoints);

  guarantee( _pcs_length > 0, "safepoint must exists before describing scopes");

  PcDescNode* last_pd = &_pcs[_pcs_length-1];
  int sender_stream_offset = last_pd->stream_offset();

  // update the stream offset of current pc desc
  last_pd->set_stream_offset(_stream->position());

  // serialize sender stream offest
  stream()->write_int(sender_stream_offset);

  // serialize scope
  if (method == NULL) {
    stream()->write_int(append_handle(NULL));
  } else {
    stream()->write_int(append_handle(method->handle()));
  }
  stream()->write_int(bci);

  // serialize the locals/expressions/monitors  
  stream()->write_int((intptr_t) locals);
  stream()->write_int((intptr_t) expressions);
  stream()->write_int((intptr_t) monitors);
}


DebugToken* DebugInformationRecorder::create_scope_values(GrowableArray<ScopeValue*>* values) {
  check_phase(describe_safepoints);
  return (DebugToken*) serialize_scope_values(values);
}


DebugToken* DebugInformationRecorder::create_monitor_values(GrowableArray<MonitorValue*>* monitors) {
  check_phase(describe_safepoints);
  return (DebugToken*) serialize_monitor_values(monitors);
}


void DebugInformationRecorder::add_dependent(ciInstanceKlass* klass, ciMethod* method) {
  check_phase(describe_dependents);
  jobject kh = (klass != NULL) ? klass->handle() : NULL;
  jobject mh = (method != NULL) ? method->handle() : NULL;

  // Try not to emit redundant dependencies.
  if (_first_dependent > 0) {
    int oldest_to_check = _first_dependent;
    const int max_pairs_to_check = 10;
    if (_number_of_dependents > max_pairs_to_check*2)
      // Skip most of very long lists to avoid quadratic search.
      oldest_to_check += (_number_of_dependents - max_pairs_to_check*2);
    for (int i = _first_dependent + _number_of_dependents; i > oldest_to_check; i -= 2) {
      if (_oop_recorder->handle_at((i-2) - 1) == kh &&
	  _oop_recorder->handle_at((i-1) - 1) == mh) {
	// This particular pair is already recorded.  Don't duplicate it.
	return;
      }
    }
  }

  int klass_index  = append_handle(kh);
  int method_index = append_handle(mh);

  if (_first_dependent == 0)  _first_dependent = klass_index;
  _number_of_dependents += 2;
  assert(klass_index  == _first_dependent + _number_of_dependents - 2, "sane");
  assert(method_index == _first_dependent + _number_of_dependents - 1, "sane");
}


int DebugInformationRecorder::append_handle(jobject h) {
  return _oop_recorder->allocate_index(h);
}


int DebugInformationRecorder::data_size() {
  check_phase(completed);
  return _stream->position();
}


int DebugInformationRecorder::pcs_size() {
  check_phase(completed);
  return _pcs_length * sizeof(PcDesc);
}


void DebugInformationRecorder::copy_to(nmethod* nm) {
  check_phase(completed);

  oop_recorder()->copy_to(nm);
  nm->copy_scopes_data(stream()->buffer(), stream()->position());
  { // copy the PCDesc
    for (int index = 0; index < _pcs_length; index++) {
      PcDescNode* p = &_pcs[index];
      PcDesc pd(p->pc_offset(), p->at_call(), abs(p->stream_offset()));
      nm->copy_pc_at(index, &pd);
    }
  }
}


void DebugInformationRecorder::verify(const nmethod* code) {
  check_phase(completed);
  Unimplemented();
}



