#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)assembler.inline.hpp	1.17 03/12/23 16:38:55 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

inline void AbstractDisplacement::bind(Label &L, int pos, AbstractAssembler* masm) {
  ((Displacement*)this)->bind(L, pos, masm);
}


inline void AbstractDisplacement::next(Label& L) const {
  ((Displacement*)this)->next(L);
}

inline void AbstractAssembler::emit_byte(int x) {
  assert(isByte(x), "not a byte");
  *(unsigned char*)_code_pos = (unsigned char)x;
  _code_pos += sizeof(unsigned char);
  code()->set_code_end(_code_pos);
}


inline void AbstractAssembler::emit_word(int x) {
  emit_byte(x & 0xFF);
  emit_byte((x >> 8) & 0xFF);
}


inline void AbstractAssembler::emit_long(jint x) {
  *(jint*)_code_pos = x;
  _code_pos += sizeof(jint);
  code()->set_code_end(_code_pos);
}


inline void AbstractAssembler::relocate(RelocationHolder const& rspec, int format) {
  assert(_inst_mark == NULL || _inst_mark == _code_pos, "call relocate() between instructions");
  code()->relocate(_code_pos, rspec, format);
}


inline void AbstractAssembler::relocate_raw(relocInfo::relocType rtype, const short* data, int datalen, int format) {
  assert(_inst_mark == NULL || _inst_mark == _code_pos, "call relocate() between instructions");
  code()->relocate_raw(_code_pos, rtype, data, datalen, format);
}


inline Displacement AbstractAssembler::disp_at(Label& L) const {
  return ((Assembler*)this)->disp_at(L);
}


inline void AbstractAssembler::disp_at_put(Label& L, Displacement& disp) {
  ((Assembler*)this)->disp_at_put(L, disp);
}


