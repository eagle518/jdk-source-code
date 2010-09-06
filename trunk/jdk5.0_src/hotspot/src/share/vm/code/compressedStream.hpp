#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)compressedStream.hpp	1.19 03/12/23 16:39:50 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Simple interface for filing out and filing in basic types
// Used for writing out and reading in debugging information.

class CompressedStream : public ResourceObj {
  friend class VMStructs;
 protected:
  u_char* _buffer;
  int     _position;

 public:
  CompressedStream(u_char* buffer, int position = 0);

  u_char* buffer() const               { return _buffer; }

  // Positioning
  int position() const                 { return _position; }
  void set_position(int position)      { _position = position; }
};


class CompressedReadStream : public CompressedStream {
 private:
  inline u_char read()                 { return _buffer[_position++]; }

 public:
  CompressedReadStream(u_char* buffer, int position = 0) 
  : CompressedStream(buffer, position) {}

  jboolean read_bool()                 { return (jboolean) read();     }
  jbyte    read_byte()                 { return (jbyte   ) read();     }
  jchar    read_char()                 { return (jchar   ) read_int(); }
  jshort   read_short()                { return (jshort  ) read_int(); }
  jint     read_int() {
    int shift = 0;
    int value = 0;
    u_char ch = read();
    while (ch < 0x80) {
      value += ch << shift;
      shift += 7;
      ch = read();
    }
    value += (ch - 192) << shift;
    return value;
  }

  // special version of read for compressed integers 
  // used in vframeStream to unpack debugging information.
  static jint raw_read_int(u_char*& buffer) {
    int shift = 0;
    int value = 0;
    u_char ch = *buffer++;
    while (ch < 0x80) {
      value += ch << shift;
      shift += 7;
      ch = *buffer++;
    }
    value += (ch - 192) << shift;
    return value;
  }

  jfloat   read_float()                { return jfloat_cast(read_int());   }
  jdouble  read_double()               { return jdouble_cast(read_long()); }
  jlong    read_long();
};


class CompressedWriteStream : public CompressedStream {
 private:
  inline void write(u_char b) { 
    if (_position >= _size) grow();
    _buffer[_position++] = b;
  }
  void grow();

 protected:
  int _size;

 public:
  CompressedWriteStream(int initial_size);

  void write_bool(jboolean value)      { write(value);     }
  void write_byte(jbyte value)         { write(value);     }
  void write_char(jchar value)         { write_int(value); }
  void write_short(jshort value)       { write_int(value); }
  void write_int(jint value) {
    jint x = value;
    while(x < -64 || x > 63) {
      write(x & 0x7F);
      x >>= 7;
    }
    write(x + 192);
  }

  void write_float(jfloat value)       { write_int (jint_cast(value));  }
  void write_double(jdouble value)     { write_long(jlong_cast(value)); }
  void write_long(jlong value);
};
