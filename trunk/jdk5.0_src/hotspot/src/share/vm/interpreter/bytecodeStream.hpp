#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)bytecodeStream.hpp	1.43 03/12/23 16:40:34 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// A BytecodeStream is used for fast iteration over the bytecodes
// of a methodOop. 
//
// Usage:
//
// BytecodeStream s(method);
// Bytecodes::Code c;
// while ((c = s.next()) >= 0) {
//   ...
// }
//
// A RawBytecodeStream is a simple version of BytecodeStream.
// It is used ONLY when we know the bytecodes haven't been rewritten
// yet, such as in the rewriter or the verifier. Currently only the
// verifier uses this class.

class RawBytecodeStream: StackObj {
 protected:
  // stream buffer
  methodHandle    _method;                       // read from method directly

  // reading position
  int             _bci;                          // bci if current bytecode
  int             _next_bci;                     // bci of next bytecode
  int             _end_bci;                      // bci after the current iteration interval

  // last bytecode read
  Bytecodes::Code _code;
  bool            _is_wide;

 public:
  // Construction
  RawBytecodeStream(methodHandle method);

  // Iteration control
  void set_interval(int beg_bci, int end_bci);   // iterate over the interval [beg_bci, end_bci[
  void set_start   (int beg_bci)                 { set_interval(beg_bci, _method->code_size()); }
  
  // Iteration
  // Use raw_next() rather than virtual next() for faster method reference on sparc.
  Bytecodes::Code raw_next() {
    Bytecodes::Code code;
    // set reading position
    _bci = _next_bci;
    if (is_last_bytecode()) {
      // indicate end of bytecode stream
      code = Bytecodes::_illegal;
    } else {
      // The method cannot have breakpoints in it because it hasn't
      // been rewritten yet.
      address bcp = RawBytecodeStream::bcp();
      code        = (Bytecodes::Code)(*bcp);
      // Assert that we find no breakpoints or rewritten code.
      assert(code != Bytecodes::_breakpoint, "unexpected breakpoint");
      assert(code == Bytecodes::java_code(code), "code has been rewritten");

      // set next bytecode position
      int l = Bytecodes::length_for(code);
      if (l == 0) l = Bytecodes::length_at(bcp);
      _next_bci  += l;
      assert(_bci < _next_bci, "length must be > 0");
      // set attributes
      _is_wide      = false;
      // check for special (uncommon) cases
      if (code == Bytecodes::_wide) {
        code = (Bytecodes::Code)bcp[1];
        _is_wide = true;
      }
    }
    _code = code;
    return _code;
  }

  // Stream attributes
  methodHandle    method() const                 { return _method; }

  int             bci() const                    { return _bci; }
  int             next_bci() const               { return _next_bci; }
  int             end_bci() const                { return _end_bci; }

  Bytecodes::Code code() const                   { return _code; }
  bool            is_wide() const                { return _is_wide; }
  bool            is_last_bytecode() const       { return _next_bci >= _end_bci; }

  address         bcp() const                    { return method()->code_base() + _bci; }
  address         next_bcp()                     { return method()->code_base() + _next_bci; }

  // State changes
  void            set_next_bci(int bci)          { assert(0 <= bci && bci <= method()->code_size(), "illegal bci"); _next_bci = bci; }

  // Bytecode-specific attributes
  int             dest() const                   { return bci() + (short)Bytes::get_Java_u2(bcp() + 1); }
  int             dest_w() const                 { return bci() + (int  )Bytes::get_Java_u4(bcp() + 1); }
  
  // Unsigned indices, widening
  int             get_index() const              { return (is_wide()) ? Bytes::get_Java_u2(bcp() + 2) : bcp()[1]; }  
  int             get_index_big() const          { return (int)Bytes::get_Java_u2(bcp() + 1);  }
};

// In BytecodeStream, non-java bytecodes will be translated into the
// corresponding java bytecodes.

class BytecodeStream: public RawBytecodeStream {
 public:
  // Construction
  BytecodeStream(methodHandle method) : RawBytecodeStream(method) { }
  
  // Iteration
  Bytecodes::Code next() {
    Bytecodes::Code code;
    // set reading position
    _bci = _next_bci;
    if (is_last_bytecode()) {
      // indicate end of bytecode stream
      code = Bytecodes::_illegal;
    } else {
      // get bytecode
      address bcp = BytecodeStream::bcp();
      code        = Bytecodes::java_code(Bytecodes::code_at(bcp));
      // set next bytecode position
      //
      // note that we cannot advance before having the
      // tty bytecode otherwise the stepping is wrong!
      // (carefull: length_for(...) must be used first!) 
      int l = Bytecodes::length_for(code);
      if (l == 0) l = Bytecodes::length_at(bcp);
      _next_bci  += l;
      assert(_bci < _next_bci, "length must be > 0");
      // set attributes
      _is_wide      = false;
      // check for special (uncommon) cases
      if (code == Bytecodes::_wide) {
        code = (Bytecodes::Code)bcp[1];
        _is_wide = true;
      }
      assert(Bytecodes::is_java_code(code), "sanity check");
    }
    _code = code;
    return _code;
  }

  bool            is_active_breakpoint() const   { return Bytecodes::is_active_breakpoint_at(bcp()); }
};


