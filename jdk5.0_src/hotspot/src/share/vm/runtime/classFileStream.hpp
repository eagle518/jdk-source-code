#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)classFileStream.hpp	1.25 03/12/23 16:43:33 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Input stream for reading .class file
//
// The entire input stream is present in a buffer allocated by the caller.
// The caller is responsible for deallocating the buffer and for using
// ResourceMarks appropriately when constructing streams.

class ClassFileStream: public ResourceObj {
 private:
  u1*   _buffer_start; // Buffer bottom
  u1*   _buffer_end;   // Buffer top (one past last element)
  u1*   _current;      // Current buffer position
  char* _source;       // Source of stream (directory name, ZIP/JAR archive name)
  bool  _need_verify;  // True if verification is on for the class file

 public:
  // Constructor
  ClassFileStream(u1* buffer, int length, char* source);

  // Buffer access
  u1* buffer() const           { return _buffer_start; }
  int length() const           { return _buffer_end - _buffer_start; }
  char* source() const         { return _source; }
  void set_verify(bool flag)   { _need_verify = flag; }

  // Read u1 from stream
  u1 get_u1(TRAPS);

  // Read u2 from stream
  u2 get_u2(TRAPS);

  // Read u4 from stream
  u4 get_u4(TRAPS);

  // Read u8 from stream
  u8 get_u8(TRAPS);

  // Get direct pointer into stream at current position. 
  // Returns NULL if length elements are not remaining. The caller is 
  // responsible for calling skip below if buffer contents is used.
  u1* get_u1_buffer(int length) {
    return _current;
  }

  u2* get_u2_buffer(int length) {
    return (u2*) _current;
  }

  // Skip length u1 or u2 elements from stream
  void skip_u1(int length, TRAPS);

  void skip_u2(int length, TRAPS);

  // Tells whether eos is reached
  bool at_eos() const          { return _current == _buffer_end; }
};
