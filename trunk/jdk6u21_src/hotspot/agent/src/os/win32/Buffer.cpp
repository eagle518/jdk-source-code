/*
 * Copyright (c) 2001, Oracle and/or its affiliates. All rights reserved.
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

#include "Buffer.hpp"

#include <string.h>

Buffer::Buffer(int bufSize) {
  buf = new char[bufSize];
  sz = bufSize;
  fill = 0;
  drain = 0;
}

Buffer::~Buffer() {
  delete[] buf;
}

char*
Buffer::fillPos() {
  return buf + fill;
}

int
Buffer::remaining() {
  return sz - fill;
}

int
Buffer::size() {
  return sz;
}

bool
Buffer::incrFillPos(int amt) {
  if (fill + amt >= sz) {
    return false;
  }
  fill += amt;
  return true;
}

int
Buffer::readByte() {
  if (drain < fill) {
    return buf[drain++] & 0xFF;
  } else {
    return -1;
  }
}

int
Buffer::readBytes(char* data, int len) {
  int numRead = 0;
  while (numRead < len) {
    int c = readByte();
    if (c < 0) break;
    data[numRead++] = (char) c;
  }
  return numRead;
}

char*
Buffer::drainPos() {
  return buf + drain;
}

int
Buffer::drainRemaining() {
  return fill - drain;
}

bool
Buffer::incrDrainPos(int amt) {
  if (drainRemaining() < amt) {
    return false;
  }
  drain += amt;
  return true;
}

void
Buffer::compact() {
  // Copy down data
  memmove(buf, buf + drain, fill - drain);
  // Adjust positions
  fill -= drain;
  drain = 0;
}
