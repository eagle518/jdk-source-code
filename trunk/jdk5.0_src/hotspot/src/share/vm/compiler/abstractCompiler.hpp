#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)abstractCompiler.hpp	1.15 03/12/23 16:40:00 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

class AbstractCompiler : public CHeapObj {
 private:
  bool _is_initialized;

 public:
  AbstractCompiler() : _is_initialized(false)    {}

  // Name of this compiler
  virtual const char* name()                     { return "compiler"; }

  // Missing feature tests
  virtual bool supports_native()                 { return true; }
  virtual bool supports_osr   ()                 { return true; } 

  // Customization
  virtual bool needs_adapters         ()         = 0;
  virtual bool needs_stubs            ()         = 0;

  void mark_initialized()                        { _is_initialized = true; }
  bool is_initialized()                          { return _is_initialized; }

  virtual void initialize()                      = 0;

  // Compilation entry point for methods
  virtual void compile_method(ciEnv* env,
			      ciMethod* target,
			      int entry_bci) {
    ShouldNotReachHere();
  }
  
  // Compilation entry point for adapters
  virtual void compile_adapter(ciEnv* env,
		               ciMethod* callee,
		               int kind) {
    ShouldNotReachHere();
  }

  // Print compilation timers and statistics
  virtual void print_timers() {
    ShouldNotReachHere();
  }
};
