#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)c2compiler.hpp	1.17 03/12/23 16:42:15 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

class C2Compiler : public AbstractCompiler {
public:
  // Name
  const char *name() { return "opto"; }

  // Customization
  bool needs_adapters         () { return true; }
  bool needs_stubs            () { return true; }
  
  void initialize();

  // Compilation entry point for methods
  void compile_method(ciEnv* env,
                      ciMethod* target,
                      int entry_bci);
  
  // Compilation entry point for adapters
  void compile_adapter(ciEnv* env,
                       ciMethod* callee,
                       int/*AdapterType*/ type);

  // sentinel value used to trigger backtracking in compile_method().
  static const char* retry_no_subsuming_loads();

  // Print compilation timers and statistics
  void print_timers();
};







