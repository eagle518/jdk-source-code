#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)classFileStream.cpp	1.33 03/12/23 16:43:33 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_classFileStream.cpp.incl"

void inline check_truncated_file(bool b, TRAPS) {
  if (b) { THROW_MSG(vmSymbols::java_lang_ClassFormatError(), "Truncated class file"); }
}

ClassFileStream::ClassFileStream(u1* buffer, int length, char* source) {
  _buffer_start = buffer;
  _buffer_end   = buffer + length;
  _current      = buffer;
  _source       = source;
  _need_verify  = false;
}

u1 ClassFileStream::get_u1(TRAPS) {
  if (_need_verify) {
    check_truncated_file(_current + 1 > _buffer_end, CHECK_0);
  } else {
    assert(_current + 1 <= _buffer_end, "buffer overflow");
  }
  return *_current++;
}

u2 ClassFileStream::get_u2(TRAPS) {
  if (_need_verify) {
    check_truncated_file(_current + 2 > _buffer_end, CHECK_0);
  } else {
    assert(_current + 2 <= _buffer_end, "buffer overflow");
  }
  u1* tmp = _current;
  _current += 2;
  return Bytes::get_Java_u2(tmp);
}

u4 ClassFileStream::get_u4(TRAPS) {
  if (_need_verify) {
    check_truncated_file(_current + 4 > _buffer_end, CHECK_0);
  } else {
    assert(_current + 4 <= _buffer_end, "buffer overflow");
  }
  u1* tmp = _current;
  _current += 4;
  return Bytes::get_Java_u4(tmp);
}

u8 ClassFileStream::get_u8(TRAPS) {
  if (_need_verify) {
    check_truncated_file(_current + 8 > _buffer_end, CHECK_0);
  } else {
    assert(_current + 8 <= _buffer_end, "buffer overflow");
  }
  u1* tmp = _current;
  _current += 8;
  return Bytes::get_Java_u8(tmp);
}

void ClassFileStream::skip_u1(int length, TRAPS) {
  if (_need_verify) {
    check_truncated_file(_current + length > _buffer_end, CHECK);
  } 
  _current += length;
}

void ClassFileStream::skip_u2(int length, TRAPS) {
  if (_need_verify) {
    check_truncated_file(_current + length * 2 > _buffer_end, CHECK);
  } 
  _current += length * 2;
}
