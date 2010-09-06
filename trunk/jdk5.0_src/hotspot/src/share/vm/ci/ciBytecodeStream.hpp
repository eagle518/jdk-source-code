#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)ciBytecodeStream.hpp	1.9 03/12/23 16:39:25 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// A ciBytecodeStream is used for fast iteration over the bytecodes
// of a methodOop. Non-java bytecodes will be translated into the
// corresponding java bytecodes.
//
// Usage:
//
// ciBytecodeStream s(code_base, code_size);
// Bytecodes::Code c;
// while ((c = s.next()) >= 0) {
//   ...
// }

class ciBytecodeStream: StackObj {
 private:
  // method
  ciMethod*       _method;                       // the method holding the bytecodes

  // stream buffer
  address         _code_base;                    // the beginnining of the buffer
  int             _code_size;                    // the size of of the buffer

  // reading position
  int             _bci;                          // bci of current bytecode
  int             _next_bci;                     // bci of next bytecode
  int             _end_bci;                      // bci after the current iteration interval

  // last bytecode read
  Bytecodes::Code _code;
  bool            _is_wide;

 public:
  // Construction
  ciBytecodeStream(ciMethod* method);

  // Iteration control
  void set_interval(int beg_bci, int end_bci);   // iterate over the interval [beg_bci, end_bci[
  void set_start   (int beg_bci)                 { set_interval(beg_bci, _code_size); }
  
  // Accessors
  ciMethod* method() const                       { return _method; }

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
      address bcp = ciBytecodeStream::bcp();
      code        = Bytecodes::java_code((Bytecodes::Code)bcp[0]);
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

  // Stream attributes
  int             bci() const                    { return _bci; }
  int             next_bci() const               { return _next_bci; }
  int             end_bci() const                { return _end_bci; }

  Bytecodes::Code code() const                   { return _code; }
  bool            is_wide() const                { return _is_wide; }
  bool            is_last_bytecode() const       { return _next_bci >= _end_bci; }

  address         bcp() const                    { return _code_base + _bci; }
  address         next_bcp()                     { return _code_base + _next_bci; }

  // State changes
  void            set_next_bci(int bci)          { assert(0 <= bci && bci <= _code_size, "illegal bci"); _next_bci = bci; }

  // Bytecode-specific attributes
  int             dest() const                   { return bci() + (short)Bytes::get_Java_u2(bcp() + 1); }
  int             dest_w() const                 { return bci() + (int  )Bytes::get_Java_u4(bcp() + 1); }
  
  // Unsigned indices, widening
  int             get_index() const              { return (is_wide()) ? Bytes::get_Java_u2(bcp() + 2) : bcp()[1]; }  
  int             get_index_big() const          { return (int)Bytes::get_Java_u2(bcp() + 1);  }

  // Constanr/Field/Klass/Method indices
  int             get_constant_index() const;
  int             get_field_index() const;
  int             get_klass_index() const;
  int             get_method_index() const;

  // Constant/Field/Klass/Method access
  ciConstant      get_constant() const;
  ciField*        get_field() const;
  ciKlass*        get_klass() const;
  ciMethod*       get_method() const;

  ciKlass*        get_declared_method_holder();
  int get_method_holder_index();

};

