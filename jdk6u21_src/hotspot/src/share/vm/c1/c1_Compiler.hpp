/*
 * Copyright (c) 1999, 2007, Oracle and/or its affiliates. All rights reserved.
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

// There is one instance of the Compiler per CompilerThread.

class Compiler: public AbstractCompiler {

 private:

 // Tracks whether runtime has been initialized
 static volatile int _runtimes;

 // In tiered it is possible for multiple threads to want to do compilation
 // only one can enter c1 at a time
 static volatile bool _compiling;

 public:
  // Creation
  Compiler();
  ~Compiler();

  // Name of this compiler
  virtual const char* name()                     { return "C1"; }

#ifdef TIERED
  virtual bool is_c1() { return true; };
#endif // TIERED


  // Missing feature tests
  virtual bool supports_native()                 { return true; }
  virtual bool supports_osr   ()                 { return true; }

  // Customization
  virtual bool needs_adapters         ()         { return false; }
  virtual bool needs_stubs            ()         { return false; }

  // Initialization
  virtual void initialize();

  // Compilation entry point for methods
  virtual void compile_method(ciEnv* env, ciMethod* target, int entry_bci);

  // Print compilation timers and statistics
  virtual void print_timers();
};
