#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)scopeDesc.hpp	1.26 03/12/23 16:39:58 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// SimpleScopeDesc is used when all you need to extract from
// a given pc,nmethod pair is a methodOop and a bci. This is
// quite a bit faster than allocating a full ScopeDesc, but
// very limited in abilities.

class SimpleScopeDesc : public StackObj {
 private:
  methodOop _method;
  int _bci;

 public:
  SimpleScopeDesc(nmethod* code,address pc,bool at_call) {
    PcDesc* pc_desc = code->pc_desc_at(pc, at_call);
    assert(pc_desc != NULL, "Must be able to find matching PcDesc");
    u_char* buffer = code->scopes_data_begin() + pc_desc->scope_decode_offset();
    CompressedReadStream::raw_read_int(buffer);
    _method = methodOop(code->oop_at(CompressedReadStream::raw_read_int(buffer)));
    _bci = CompressedReadStream::raw_read_int(buffer);
  }
  
  methodOop method() { return _method; }
  int bci() { return _bci; }
};

// ScopeDescs contain the information that makes source-level debugging of
// nmethods possible; each scopeDesc describes a method activation

class ScopeDesc : public ResourceObj {
 public:
  // Constructor
  ScopeDesc(const nmethod* code, int decode_offset);

  // JVM state
  methodHandle method() const { return _method; }
  int          bci()    const { return _bci;    }
  
  GrowableArray<ScopeValue*>*   locals();
  GrowableArray<ScopeValue*>*   expressions();  
  GrowableArray<MonitorValue*>* monitors();

  // Stack walking, returns NULL if this is the outer most scope.
  ScopeDesc* sender() const;

  // Returns where the scope was decoded
  int decode_offset() const { return _decode_offset; }

  // Tells whether sender() returns NULL
  bool is_top() const;
  // Tells whether sd is equal to this
  bool is_equal(ScopeDesc* sd) const;

 private:
  // JVM state
  methodHandle  _method;
  int           _bci;

  // Decoding offsets
  int _decode_offset;
  int _sender_decode_offset;
  int _locals_decode_offset;
  int _expressions_decode_offset;
  int _monitors_decode_offset;

  // Nmethod information
  const nmethod* _code;

  // Decoding operations
  void decode_body();
  GrowableArray<ScopeValue*>* decode_scope_values(int decode_offset);
  GrowableArray<MonitorValue*>* decode_monitor_values(int decode_offset);

  DebugInfoReadStream* stream_at(int decode_offset) const;

#ifndef PRODUCT
 public:
  // Verification
  void verify();

  // Printing support
  void print_on(outputStream* st) const;
  void print_value_on(outputStream* st) const;
#endif
};



