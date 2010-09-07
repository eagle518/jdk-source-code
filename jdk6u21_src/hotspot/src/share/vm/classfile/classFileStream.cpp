/*
 * Copyright (c) 1997, 2005, Oracle and/or its affiliates. All rights reserved.
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

#include "incls/_precompiled.incl"
#include "incls/_classFileStream.cpp.incl"

void ClassFileStream::truncated_file_error(TRAPS) {
  THROW_MSG(vmSymbols::java_lang_ClassFormatError(), "Truncated class file");
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
    guarantee_more(1, CHECK_0);
  } else {
    assert(1 <= _buffer_end - _current, "buffer overflow");
  }
  return *_current++;
}

u2 ClassFileStream::get_u2(TRAPS) {
  if (_need_verify) {
    guarantee_more(2, CHECK_0);
  } else {
    assert(2 <= _buffer_end - _current, "buffer overflow");
  }
  u1* tmp = _current;
  _current += 2;
  return Bytes::get_Java_u2(tmp);
}

u4 ClassFileStream::get_u4(TRAPS) {
  if (_need_verify) {
    guarantee_more(4, CHECK_0);
  } else {
    assert(4 <= _buffer_end - _current, "buffer overflow");
  }
  u1* tmp = _current;
  _current += 4;
  return Bytes::get_Java_u4(tmp);
}

u8 ClassFileStream::get_u8(TRAPS) {
  if (_need_verify) {
    guarantee_more(8, CHECK_0);
  } else {
    assert(8 <= _buffer_end - _current, "buffer overflow");
  }
  u1* tmp = _current;
  _current += 8;
  return Bytes::get_Java_u8(tmp);
}

void ClassFileStream::skip_u1(int length, TRAPS) {
  if (_need_verify) {
    guarantee_more(length, CHECK);
  }
  _current += length;
}

void ClassFileStream::skip_u2(int length, TRAPS) {
  if (_need_verify) {
    guarantee_more(length * 2, CHECK);
  }
  _current += length * 2;
}
