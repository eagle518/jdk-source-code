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

#ifndef _BUFFER_
#define _BUFFER_

// A Buffer is the backing store for the IOBuf abstraction and
// supports producer-consumer filling and draining.

class Buffer {
public:
  Buffer(int bufSize);
  ~Buffer();

  char* fillPos();   // Position of the place where buffer should be filled
  int   remaining(); // Number of bytes that can be placed starting at fillPos
  int   size();      // Size of the buffer
  // Move up fill position by amount (decreases remaining()); returns
  // false if not enough space
  bool  incrFillPos(int amt);

  // Read single byte (0..255); returns -1 if no data available.
  int   readByte();
  // Read multiple bytes, non-blocking (this buffer does not define a
  // fill mechanism), into provided buffer. Returns number of bytes read.
  int   readBytes(char* buf, int len);

  // Access to drain position. Be very careful using this.
  char* drainPos();
  int   drainRemaining();
  bool  incrDrainPos(int amt);

  // Compact buffer, removing already-consumed input. This must be
  // called periodically to yield the illusion of an infinite buffer.
  void  compact();

private:
  Buffer(const Buffer&);
  Buffer& operator=(const Buffer&);

  char* buf;
  int   sz;
  int   fill;
  int   drain;
};

#endif // #defined _BUFFER_
