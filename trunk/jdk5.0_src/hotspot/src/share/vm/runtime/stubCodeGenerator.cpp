#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)stubCodeGenerator.cpp	1.23 04/04/05 14:17:27 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_stubCodeGenerator.cpp.incl"


// Implementation of StubCodeDesc

StubCodeDesc* StubCodeDesc::_list = NULL;
int           StubCodeDesc::_count = 0;


StubCodeDesc* StubCodeDesc::desc_for(address pc) {
  StubCodeDesc* p = _list;
  while (p != NULL && !p->contains(pc)) p = p->_next;
  // p == NULL || p->contains(pc)
  return p;
}


StubCodeDesc* StubCodeDesc::desc_for_index(int index) {
  StubCodeDesc* p = _list;
  while (p != NULL && p->index() != index) p = p->_next;
  return p;
}


const char* StubCodeDesc::name_for(address pc) {
  StubCodeDesc* p = desc_for(pc);
  return p == NULL ? NULL : p->name();
}


void StubCodeDesc::print() {
  tty->print(group());
  tty->print("::");
  tty->print(name());
  tty->print(" [" INTPTR_FORMAT ", " INTPTR_FORMAT "[ (%d bytes)", begin(), end(), size_in_bytes());
}



// Implementation of StubCodeGenerator

void StubCodeGenerator::stub_prolog(StubCodeDesc* cdesc) {
  // default implementation - do nothing
}


void StubCodeGenerator::stub_epilog(StubCodeDesc* cdesc) {
  // default implementation - support printing
  if (PrintStubCode) {
    cdesc->print();
    tty->cr();
    Disassembler::decode(cdesc->begin(), cdesc->end());
    tty->cr();
  }
}


// Implementation of CodeMark

StubCodeMark::StubCodeMark(StubCodeGenerator* cgen, const char* group, const char* name) {
  _cgen  = cgen;
  _cdesc = new StubCodeDesc(group, name, _cgen->assembler()->pc());
  _cgen->stub_prolog(_cdesc);
  // define the stub's beginning (= entry point) to be after the prolog:
  _cdesc->set_begin(_cgen->assembler()->pc());
}

StubCodeMark::~StubCodeMark() {
  _cgen->assembler()->flush();
  _cdesc->set_end(_cgen->assembler()->pc());
  _cgen->stub_epilog(_cdesc);
  VTune::register_stub(_cdesc->name(), _cdesc->begin(), _cdesc->end());
  Forte::register_stub(_cdesc->name(), _cdesc->begin(), _cdesc->end());

  if (JvmtiExport::should_post_dynamic_code_generated()) {
    JvmtiExport::post_dynamic_code_generated(_cdesc->name(), _cdesc->begin(), _cdesc->end());
  }
}

