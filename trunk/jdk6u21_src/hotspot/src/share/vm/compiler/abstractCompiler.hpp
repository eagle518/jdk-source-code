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

typedef void (*initializer)(void);

class AbstractCompiler : public CHeapObj {
 private:
  bool _is_initialized; // Mark whether compiler object is initialized

 protected:
  // Used for tracking global state of compiler runtime initialization
  enum { uninitialized, initializing, initialized };

  // This method will call the initialization method "f" once (per compiler class/subclass)
  // and do so without holding any locks
  void initialize_runtimes(initializer f, volatile int* state);

 public:
  AbstractCompiler() : _is_initialized(false)    {}

  // Name of this compiler
  virtual const char* name() = 0;

  // Missing feature tests
  virtual bool supports_native()                 { return true; }
  virtual bool supports_osr   ()                 { return true; }
#if defined(TIERED) || ( !defined(COMPILER1) && !defined(COMPILER2))
  virtual bool is_c1   ()                        { return false; }
  virtual bool is_c2   ()                        { return false; }
#else
#ifdef COMPILER1
  bool is_c1   ()                                { return true; }
  bool is_c2   ()                                { return false; }
#endif // COMPILER1
#ifdef COMPILER2
  bool is_c1   ()                                { return false; }
  bool is_c2   ()                                { return true; }
#endif // COMPILER2
#endif // TIERED

  // Customization
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


  // Print compilation timers and statistics
  virtual void print_timers() {
    ShouldNotReachHere();
  }
};
