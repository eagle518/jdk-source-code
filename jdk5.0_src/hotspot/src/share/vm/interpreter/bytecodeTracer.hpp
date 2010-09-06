#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)bytecodeTracer.hpp	1.18 03/12/23 16:40:34 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// The BytecodeTracer is a helper class used by the interpreter for run-time
// bytecode tracing. If bytecode tracing is turned on, trace() will be called
// for each bytecode.
//
// By specialising the BytecodeClosure, all kinds of bytecode traces can
// be done.

#ifndef PRODUCT
// class BytecodeTracer is only used by TraceBytecodes option

class BytecodeClosure;
class BytecodeTracer: AllStatic {
 private:
  static BytecodeClosure* _closure;

 public:
  static BytecodeClosure* std_closure();                        // a printing closure
  static BytecodeClosure* closure()				                      { return _closure; }
  static void             set_closure(BytecodeClosure* closure)	{ _closure = closure; }

  static void             trace(methodHandle method, address bcp, uintptr_t tos, uintptr_t tos2);
  static void             trace(methodHandle method, address bcp);
};


// For each bytecode, a BytecodeClosure's trace() routine will be called.

class BytecodeClosure {
 public:
  virtual void trace(methodHandle method, address bcp, uintptr_t tos, uintptr_t tos2) = 0;
  virtual void trace(methodHandle method, address bcp) = 0;
};

#endif // !PRODUCT
