#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)c1_CodeStubs.hpp	1.76 04/04/14 17:27:28 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

class CodeEmitInfo;
class LIR_Assembler;
class RInfo;

// CodeStubs are little 'out-of-line' pieces of code that
// usually handle slow cases of operations. All code stubs
// are collected and code is emitted at the end of the
// nmethod.

class CodeStub: public CompilationResourceObj {
 protected:
  Label _entry;                                  // label at the stub entry point
  Label _continuation;                           // label where stub continues, if any

 public:
  // code generation
  void assert_no_unbound_labels()                { assert(!_entry.is_unbound() && !_continuation.is_unbound(), "unbound label"); }
  virtual void emit_code(LIR_Assembler* e) = 0;
  virtual CodeEmitInfo* info() const             { return NULL; }
  virtual bool is_call_stub() const              { return false; }
  virtual bool is_exception_throw_stub() const   { return false; }
  virtual bool is_range_check_stub() const       { return false; }
  virtual bool is_divbyzero_stub() const         { return false; }
#ifndef PRODUCT
  virtual void print_name() const = 0;
#endif

  // label access
  Label* entry()                                 { return &_entry; }
  Label* continuation()                          { return &_continuation; }
  // for LIR
  virtual void set_code_pc(address pc)           { }
  virtual void visit(LIR_OpVisitState* visit) {
#ifndef PRODUCT
    if (LIRTracePeephole && Verbose) {
      tty->print("no visitor for ");
      print_name();
      tty->cr();
    }
#endif
  }
};


define_array(CodeStubArray, CodeStub*)
define_stack(_CodeStubList, CodeStubArray)

class CodeStubList: public _CodeStubList {
 public:
  CodeStubList(): _CodeStubList() {}

  void append(CodeStub* stub) {
    if (!contains(stub)) {
      _CodeStubList::append(stub);
    }
  }
};


class StaticCallStub: public CodeStub {
 private:
  address               _call_pc;

 public:
  StaticCallStub(address call_pc = NULL)
  : _call_pc(call_pc) {}
  virtual void emit_code(LIR_Assembler* e);
  virtual bool is_call_stub() const              { return true; }
  virtual void set_code_pc(address pc)           { _call_pc = pc; }
  virtual void visit(LIR_OpVisitState* visitor) {
    visitor->do_call();
  }
#ifndef PRODUCT
  virtual void print_name() const                { tty->print("StaticCallStub"); }
#endif // PRODUCT
};


// Throws ArrayIndexOutOfBoundsException by default but can be
// configured to throw IndexOutOfBoundsException in constructor
class RangeCheckStub: public CodeStub {
 private:
  CodeEmitInfo* _info;
  RInfo         _index_rinfo;
  int           _index_value;
  bool          _throw_index_out_of_bounds_exception;

 public:
  RangeCheckStub(CodeEmitInfo* info, RInfo index_rinfo, int index_value, bool throw_index_out_of_bounds_exception = false);
  virtual void emit_code(LIR_Assembler* e);
  virtual CodeEmitInfo* info() const             { return _info; }
  virtual bool is_exception_throw_stub() const   { return true; }
  virtual bool is_range_check_stub() const       { return true; }
  virtual void visit(LIR_OpVisitState* visitor) {
    visitor->do_stub_info(_info);
    visitor->do_rinfo(_index_rinfo);
  }
#ifndef PRODUCT
  virtual void print_name() const                { tty->print("RangeCheckStub"); }
#endif // PRODUCT
};


class DivByZeroStub: public CodeStub {
 private:
  CodeEmitInfo* _info;
  int           _offset;

 public:
  DivByZeroStub(CodeEmitInfo* info)
    : _info(info), _offset(-1) {
    _info->check_is_exception_info();
  }
  DivByZeroStub(int offset, CodeEmitInfo* info)
    : _info(info), _offset(offset) {
    _info->check_is_exception_info();
  }
  virtual void emit_code(LIR_Assembler* e);
  virtual CodeEmitInfo* info() const             { return _info; }
  virtual bool is_exception_throw_stub() const   { return true; }
  virtual bool is_divbyzero_stub() const         { return true; }
  virtual void visit(LIR_OpVisitState* visitor) {
    visitor->do_stub_info(_info);
  }
#ifndef PRODUCT
  virtual void print_name() const                { tty->print("DivByZeroStub"); }
#endif // PRODUCT
};


class ImplicitNullCheckStub: public CodeStub {
 private:
  CodeEmitInfo* _info;
  int           _offset;

 public:
  ImplicitNullCheckStub(int offset, CodeEmitInfo* info)
    : _offset(offset), _info(info) {
    _info->check_is_exception_info();
  }
  void set_offset(int offset) { _offset = offset; }
  virtual void emit_code(LIR_Assembler* e);
  virtual CodeEmitInfo* info() const             { return _info; }
  virtual bool is_exception_throw_stub() const   { return true; }
  virtual void visit(LIR_OpVisitState* visitor) {
    visitor->do_stub_info(_info);
  }
#ifndef PRODUCT
  virtual void print_name() const                { tty->print("ImplicitNullCheckStub"); }
#endif // PRODUCT
};


class NewInstanceStub: public CodeStub {
 private:
  ciInstanceKlass* _klass;
  RInfo            _klass_reg;
  LIR_Opr          _result;
  CodeEmitInfo*    _info;
  Runtime1::StubID _stub_id;

 public:
  NewInstanceStub(RInfo klass_reg, LIR_Opr result, ciInstanceKlass* klass, CodeEmitInfo* info, Runtime1::StubID stub_id);
  virtual void emit_code(LIR_Assembler* e);
  virtual CodeEmitInfo* info() const             { return _info; }
  virtual void visit(LIR_OpVisitState* visitor) {
    visitor->do_stub_info(_info);
    visitor->do_rinfo(_klass_reg);
    visitor->do_output(_result);
    visitor->do_call();
  }
#ifndef PRODUCT
  virtual void print_name() const                { tty->print("NewInstanceStub"); }
#endif // PRODUCT
};


class NewTypeArrayStub: public CodeStub {
 private:
  RInfo         _klass_reg;
  RInfo         _length;
  RInfo         _result;
  CodeEmitInfo* _info;

 public:
  NewTypeArrayStub(RInfo klass_reg, RInfo length, RInfo result, CodeEmitInfo* info);
  virtual void emit_code(LIR_Assembler* e);
  virtual CodeEmitInfo* info() const             { return _info; }
  virtual void visit(LIR_OpVisitState* visitor) {
    visitor->do_stub_info(_info);
    visitor->do_rinfo(_klass_reg);
    visitor->do_rinfo(_length);
    visitor->do_call();
    visitor->do_rinfo(_result);
  }
#ifndef PRODUCT
  virtual void print_name() const                { tty->print("NewTypeArrayStub"); }
#endif // PRODUCT
};


class NewObjectArrayStub: public CodeStub {
 private:
  RInfo          _klass_reg;
  RInfo          _length;
  RInfo          _result;
  CodeEmitInfo*  _info;

 public:
  NewObjectArrayStub(RInfo klass_reg, RInfo length, RInfo result, CodeEmitInfo* info);
  virtual void emit_code(LIR_Assembler* e);
  virtual CodeEmitInfo* info() const             { return _info; }
  virtual void visit(LIR_OpVisitState* visitor) {
    visitor->do_stub_info(_info);
    visitor->do_rinfo(_klass_reg);
    visitor->do_rinfo(_length);
    visitor->do_call();
    visitor->do_rinfo(_result);
  }
#ifndef PRODUCT
  virtual void print_name() const                { tty->print("NewObjectArrayStub"); }
#endif // PRODUCT
};


class MonitorAccessStub: public CodeStub {
 protected:
  RInfo _obj_reg;
  RInfo _lock_reg;

 public:
  MonitorAccessStub(RInfo obj_reg, RInfo lock_reg) {
    _obj_reg  = obj_reg;
    _lock_reg = lock_reg;
  }
  virtual void visit(LIR_OpVisitState* visitor) {
    visitor->do_rinfo(_obj_reg);
    visitor->do_rinfo(_lock_reg);
    visitor->do_call();
  }

#ifndef PRODUCT
  virtual void print_name() const                { tty->print("MonitorAccessStub"); }
#endif // PRODUCT
};


class MonitorEnterStub: public MonitorAccessStub {
 private:
  CodeEmitInfo* _info;

 public:
  MonitorEnterStub(RInfo obj_reg, RInfo lock_reg, CodeEmitInfo* info);
  virtual void emit_code(LIR_Assembler* e);
  virtual CodeEmitInfo* info() const             { return _info; }
  virtual void visit(LIR_OpVisitState* visitor) {
    MonitorAccessStub::visit(visitor);
    visitor->do_stub_info(_info);
  }
#ifndef PRODUCT
  virtual void print_name() const                { tty->print("MonitorEnterStub"); }
#endif // PRODUCT
};


class MonitorExitStub: public MonitorAccessStub {
 private:
  bool _compute_lock;
  int  _monitor_ix;

 public:
  MonitorExitStub(RInfo lock_reg, bool compute_lock, int monitor_ix)
  : MonitorAccessStub(norinfo, lock_reg)
  { _compute_lock = compute_lock;
    _monitor_ix   = monitor_ix;
  }
  virtual void emit_code(LIR_Assembler* e);
  // temporary fix: must be created after exceptionhandler, therefore as call stub
  virtual bool is_call_stub() const              { return true; }
#ifndef PRODUCT
  virtual void print_name() const                { tty->print("MonitorExitStub"); }
#endif // PRODUCT
};


class PatchingStub: public CodeStub {
 public:
  enum PatchID {
    access_field_id,
    load_klass_id
  };
  enum constants {
    patch_info_size = 3
  };
 private:
  PatchID       _id;
  address       _pc_start;
  int           _bytes_to_copy;
  Label         _patched_code_entry;
  Label         _patch_site_entry;
  Label         _patch_site_continuation;
  Register      _obj;
  CodeEmitInfo* _info;
  static int    _patch_info_offset;

  void align_patch_site(MacroAssembler* masm);

 public:
  static int patch_info_offset() { return _patch_info_offset; }

  PatchingStub(MacroAssembler* masm, PatchID id):
      _id(id)
    , _info(NULL) {
    if (os::is_MP()) {
      // force alignment of patch sites on MP hardware so we
      // can guarantee atomic writes to the patch site.
      align_patch_site(masm);
    }
    _pc_start = masm->pc();
    masm->bind(_patch_site_entry);
  }

  void install(MacroAssembler* masm, LIR_Op1::LIR_PatchCode patch_code, Register obj, CodeEmitInfo* info) {
    _info = info;
    _obj = obj;
    masm->bind(_patch_site_continuation);
    _bytes_to_copy = masm->pc() - pc_start();
    if (_id == PatchingStub::access_field_id) {
      assert(_obj != noreg, "must have register object for access_field");
      // embed a fixed offset to handle long patches which need to be offset by a word.
      // the patching code will just add the field offset field to this offset so
      // that we can refernce either the high or low word of a double word field.
      int field_offset = 0;
      switch (patch_code) {
      case LIR_Op1::patch_low:    field_offset = lo_word_offset_in_bytes; break;
      case LIR_Op1::patch_high:   field_offset = hi_word_offset_in_bytes; break;
      case LIR_Op1::patch_normal: field_offset = 0; break;
      default: ShouldNotReachHere();
      }
      NativeMovRegMem* n_move = nativeMovRegMem_at(pc_start());
      n_move->set_offset(field_offset);
    } else if (_id == load_klass_id) {
#ifdef ASSERT
      // verify that we're pointing at a NativeMovConstReg
      nativeMovConstReg_at(pc_start());
#endif
    } else {
      ShouldNotReachHere();
    }
    assert(_bytes_to_copy <= (masm->pc() - pc_start()), "not enough bytes");
  }

  address pc_start() const                       { return _pc_start; }
  PatchID id() const                             { return _id; }

  virtual void emit_code(LIR_Assembler* e);
  virtual CodeEmitInfo* info() const             { return _info; }
  virtual void set_code_pc(address pc)           { ShouldNotReachHere(); }
  virtual void visit(LIR_OpVisitState* visitor) {
    visitor->do_call();
    visitor->do_stub_info(_info);
  }
#ifndef PRODUCT
  virtual void print_name() const                { tty->print("PatchingStub"); }
#endif // PRODUCT
};


class SimpleExceptionStub: public CodeStub {
 private:
  RInfo            _obj;
  Runtime1::StubID _stub;
  CodeEmitInfo*    _info_for_exception;

 public:
  SimpleExceptionStub(Runtime1::StubID stub, RInfo obj, CodeEmitInfo* info_for_exception):
    _obj(obj), _info_for_exception(info_for_exception), _stub(stub) {
    _info_for_exception->check_is_exception_info();
  }

  virtual void emit_code(LIR_Assembler* e);
  virtual CodeEmitInfo* info() const             { return _info_for_exception; }
  virtual bool is_exception_throw_stub() const   { return true; }
  virtual void visit(LIR_OpVisitState* visitor) {
    visitor->do_stub_info(_info_for_exception);
    visitor->do_rinfo(_obj);
  }
#ifndef PRODUCT
  virtual void print_name() const                { tty->print("SimpleExceptionStub"); }
#endif // PRODUCT
};



class ArrayStoreExceptionStub: public CodeStub {
 private:
  CodeEmitInfo* _info_for_exception;

 public:
  ArrayStoreExceptionStub(CodeEmitInfo* info_for_exception);
  virtual void emit_code(LIR_Assembler* emit);
  virtual CodeEmitInfo* info() const             { return _info_for_exception; }
  virtual bool is_exception_throw_stub() const   { return true; }
  virtual void visit(LIR_OpVisitState* visitor) {
    visitor->do_stub_info(_info_for_exception);
  }
#ifndef PRODUCT
  virtual void print_name() const                { tty->print("ArrayStoreExceptionStub"); }
#endif // PRODUCT
};


class ArrayCopyStub: public CodeStub {
 private:
  CodeEmitInfo*  _info;
  Label _noCheckEntry;
  RInfo _src;
  RInfo _dst;
  RInfo _src_pos;
  RInfo _dst_pos;
  RInfo _length;
  RInfo _tmp;
  StaticCallStub* _call_stub;

 public:
  ArrayCopyStub(CodeEmitInfo* info, StaticCallStub* call_stub);

  void set_src(RInfo r)                   { _src = r; }
  void set_dst(RInfo r)                   { _dst = r; }
  void set_src_pos(RInfo r)               { _src_pos = r; }
  void set_dst_pos(RInfo r)               { _dst_pos = r; }
  void set_length(RInfo r)                { _length = r;  }
  void set_tmp(RInfo r)                   { _tmp = r;  }

  RInfo src() const                       { return _src; }
  RInfo src_pos() const                   { return _src_pos; }
  RInfo dst() const                       { return _dst; }
  RInfo dst_pos() const                   { return _dst_pos; }
  RInfo length() const                    { return _length; }
  RInfo tmp() const                       { return _tmp; }

  virtual void emit_code(LIR_Assembler* e);
  virtual CodeEmitInfo* info() const          { return _info; }
  Label* noCheckEntry()                       { return &_noCheckEntry; }
  virtual void visit(LIR_OpVisitState* visitor) {
    visitor->do_stub_info(_info);
    visitor->do_rinfo(src());
    visitor->do_rinfo(src_pos());
    visitor->do_rinfo(dst());
    visitor->do_rinfo(dst_pos());
    visitor->do_rinfo(length());
    visitor->do_rinfo(tmp());
    visitor->do_call();
  }
#ifndef PRODUCT
  virtual void print_name() const                { tty->print("ArrayCopyStub"); }
#endif // PRODUCT
};

