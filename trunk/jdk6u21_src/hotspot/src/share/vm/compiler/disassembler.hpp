/*
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
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

class decode_env;

// The disassembler prints out assembly code annotated
// with Java specific information.

class Disassembler {
  friend class decode_env;
 private:
  // this is the type of the dll entry point:
  typedef void* (*decode_func)(void* start, void* end,
                               void* (*event_callback)(void*, const char*, void*),
                               void* event_stream,
                               int (*printf_callback)(void*, const char*, ...),
                               void* printf_stream,
                               const char* options);
  // points to the library.
  static void*    _library;
  // bailout
  static bool     _tried_to_load_library;
  // points to the decode function.
  static decode_func _decode_instructions;
  // tries to load library and return whether it succedded.
  static bool load_library();

  // Machine dependent stuff
  #include "incls/_disassembler_pd.hpp.incl"

 public:
  static bool can_decode() {
    return (_decode_instructions != NULL) || load_library();
  }
  static void decode(CodeBlob *cb,               outputStream* st = NULL);
  static void decode(nmethod* nm,                outputStream* st = NULL);
  static void decode(address begin, address end, outputStream* st = NULL);
};
