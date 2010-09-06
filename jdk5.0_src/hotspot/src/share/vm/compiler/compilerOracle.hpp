#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)compilerOracle.hpp	1.13 03/12/23 16:40:03 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// CompilerOracle is an interface for turning on and off compilation
// for some methods

class symbolHandle;

class CompilerOracle : AllStatic {
 private:
  static void read_from_line(char* line);
 public:
  // Add element to should exclude list
  static void add_should_exclude(symbolHandle class_name, symbolHandle method_name);
 
  // Tells whether we to exclude compilation of method
  static bool should_exclude(methodHandle method);

  // Add element to should exclude list
  static void add_should_inline(symbolHandle class_name, symbolHandle method_name);
 
  // Tells whether we to exclude compilation of method
  static bool should_inline(methodHandle method);

  // Add element to should print list
  static void add_should_print(symbolHandle class_name, symbolHandle method_name);

  // Tells whether we should print the assembly for this method
  static bool should_print(methodHandle method);

  // Add element to should log list
  static void add_should_log(symbolHandle class_name, symbolHandle method_name);

  // Tells whether we should log the compilation data for this method
  static bool should_log(methodHandle method);

  // Add element to should break at list
  static void add_should_break_at(symbolHandle class_name, symbolHandle method_name);

  // Tells whether to break when compiling method
  static bool should_break_at(methodHandle method);

  // Reads from file and adds to lists 
  static void parse_from_file();
  // Reads from string instead of file
  static void parse_from_string(const char* command_string);

  // For updating the oracle file
  static void append_comment_to_file(const char* message);
  static void append_exclude_to_file(methodHandle method);
};
