/*
 * Copyright (c) 1999, 2008, Oracle and/or its affiliates. All rights reserved.
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

class C2Compiler : public AbstractCompiler {
private:

  static void initialize_runtime();

public:
  // Name
  const char *name() { return "C2"; }

  static volatile int _runtimes;

#ifdef TIERED
  virtual bool is_c2() { return true; };
#endif // TIERED

  // Customization
  bool needs_adapters         () { return true; }
  bool needs_stubs            () { return true; }

  void initialize();

  // Compilation entry point for methods
  void compile_method(ciEnv* env,
                      ciMethod* target,
                      int entry_bci);

  // sentinel value used to trigger backtracking in compile_method().
  static const char* retry_no_subsuming_loads();
  static const char* retry_no_escape_analysis();

  // Print compilation timers and statistics
  void print_timers();
};
