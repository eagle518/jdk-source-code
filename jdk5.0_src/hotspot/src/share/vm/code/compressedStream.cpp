#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)compressedStream.cpp	1.18 03/12/23 16:39:49 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_compressedStream.cpp.incl"


CompressedStream::CompressedStream(u_char* buffer, int position) {
  _buffer   = buffer;
  _position = position;
}

jlong CompressedReadStream::read_long() {
  jint low  = read_int();
  jint high = read_int();
  return jlong_from(high, low);
}

CompressedWriteStream::CompressedWriteStream(int initial_size) : CompressedStream(NULL, 0) {
  _buffer   = NEW_RESOURCE_ARRAY(u_char, initial_size);
  _size     = initial_size;
  _position = 0;
}

void CompressedWriteStream::grow() {
  u_char* _new_buffer = NEW_RESOURCE_ARRAY(u_char, _size * 2);
  memcpy(_new_buffer, _buffer, _position);
  _buffer = _new_buffer;
  _size   = _size * 2;
}

void CompressedWriteStream::write_long(jlong value) {
  write_int(low(value));
  write_int(high(value));
}
