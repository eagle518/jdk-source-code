/*
 * Copyright (c) 1997, 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */


  protected:

  // Size of interpreter code.  Increase if too small.  Interpreter will
  // fail with a guarantee ("not enough space for interpreter generation");
  // if too small.
  // Run with +PrintInterpreter to get the VM to print out the size.
  // Max size with JVMTI and TaggedStackInterpreter
#ifdef _LP64
  // The sethi() instruction generates lots more instructions when shell
  // stack limit is unlimited, so that's why this is much bigger.
  const static int InterpreterCodeSize = 210 * K;
#else
  const static int InterpreterCodeSize = 180 * K;
#endif
