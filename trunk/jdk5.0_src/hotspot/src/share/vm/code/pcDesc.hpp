#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)pcDesc.hpp	1.28 03/12/23 16:39:57 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// PcDescs map a physical PC (given as offset from start of nmethod) to
// the corresponding source scope and byte code index.

class PcDesc VALUE_OBJ_CLASS_SPEC {
  friend class VMStructs;
 private:
  int _pc_offset;           // offset from start of nmethod
  int _scope_decode_offset; // [+-]offset for scope in nmethod (sign is used for at_call)

 public:
  int pc_offset() const           { return _pc_offset;   }
  int scope_decode_offset() const { return at_call() ? _scope_decode_offset : -_scope_decode_offset;}
  bool at_call() const            { return _scope_decode_offset >= 0; }

  // Constructor (only used for static in nmethod.cpp)
  // Also used by ScopeDesc::sender()]
  PcDesc(int pc_offset, bool at_call, int scope_decode_offset);

  // Returns the real pc
  address real_pc(const nmethod* code) const;

  void print(nmethod* code);
  bool verify(nmethod* code);
};

