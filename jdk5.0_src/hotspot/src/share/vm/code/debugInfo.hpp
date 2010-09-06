#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)debugInfo.hpp	1.26 03/12/23 16:39:50 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Classes used for serializing debugging information.
// These abstractions are introducted to provide symmetric 
// read and write operations.

// ScopeValue        describes the value of a variable/expression in a scope
// - LocationValue   describes a value in a given location (in frame or register)
// - ConstantValue   describes a constant

class ScopeValue: public ResourceObj {
 public:
  // Testers
  virtual bool is_location() const { return false; }
  virtual bool is_constant_int() const { return false; }
  virtual bool is_constant_double() const { return false; }
  virtual bool is_constant_long() const { return false; }
  virtual bool is_constant_oop() const { return false; }
  virtual bool equals(ScopeValue* other) const { return false; }

  // Serialization of debugging information
  virtual void write_on(DebugInfoWriteStream* stream) = 0;
  static ScopeValue* read_from(DebugInfoReadStream* stream);
};


// A Location value describes a value in a given location; i.e. the corresponding
// logical entity (e.g., a method temporary) lives in this location.

class LocationValue: public ScopeValue {
 private:
  Location  _location;
 public:
  LocationValue(Location location)           { _location = location; }
  bool      is_location() const              { return true; }
  Location  location() const                 { return _location; }

  // Serialization of debugging information
  LocationValue(DebugInfoReadStream* stream);
  void write_on(DebugInfoWriteStream* stream);

  // Printing
  void print_on(outputStream* st) const;
};

// A ConstantIntValue describes a constant int; i.e., the corresponding logical entity
// is either a source constant or its computation has been constant-folded.

class ConstantIntValue: public ScopeValue {
 private:
  int _value;
 public:
  ConstantIntValue(int value)          { _value = value; }
  int value() const                    { return _value;  }
  bool is_constant_int() const         { return true;    }
  bool equals(ScopeValue* other) const { return false;   }

  // Serialization of debugging information
  ConstantIntValue(DebugInfoReadStream* stream);
  void write_on(DebugInfoWriteStream* stream);

  // Printing
  void print_on(outputStream* st) const;
};

class ConstantLongValue: public ScopeValue {
 private:
  jlong _value;
 public:
  ConstantLongValue(jlong value)       { _value = value; }
  jlong value() const                  { return _value;  }
  bool is_constant_long() const        { return true;    }
  bool equals(ScopeValue* other) const { return false;   }

  // Serialization of debugging information
  ConstantLongValue(DebugInfoReadStream* stream);
  void write_on(DebugInfoWriteStream* stream);

  // Printing
  void print_on(outputStream* st) const;
};

class ConstantDoubleValue: public ScopeValue {
 private:
  jdouble _value;
 public:
  ConstantDoubleValue(double value)    { _value = value; }
  double value() const                 { return _value;  }
  bool is_constant_double() const      { return true;    }
  bool equals(ScopeValue* other) const { return false;   }

  // Serialization of debugging information
  ConstantDoubleValue(DebugInfoReadStream* stream);
  void write_on(DebugInfoWriteStream* stream);

  // Printing
  void print_on(outputStream* st) const;
};

// A ConstantOopWriteValue is created by the compiler to
// be written as debugging information.

class ConstantOopWriteValue: public ScopeValue {
 private:
  jobject _value;
 public:
  ConstantOopWriteValue(jobject value) { _value = value; }
  jobject value() const                { return _value;  }
  bool is_constant_oop() const         { return true;    }
  bool equals(ScopeValue* other) const { return false;   }

  // Serialization of debugging information
  void write_on(DebugInfoWriteStream* stream);

  // Printing
  void print_on(outputStream* st) const;
};

// A ConstantOopReadValue is created by the VM when reading
// debug information

class ConstantOopReadValue: public ScopeValue {
 private:
  Handle _value;
 public:
  Handle value() const                 { return _value;  }
  bool is_constant_oop() const         { return true;    }
  bool equals(ScopeValue* other) const { return false;   }

  // Serialization of debugging information
  ConstantOopReadValue(DebugInfoReadStream* stream);
  void write_on(DebugInfoWriteStream* stream);

  // Printing
  void print_on(outputStream* st) const;
};

// MonitorValue describes the pair used for monitor_enter and monitor_exit.

class MonitorValue: public ResourceObj {
 private:
  ScopeValue* _owner;
  Location    _basic_lock;
 public:
  // Constructor
  MonitorValue(ScopeValue* owner, Location basic_lock);

  // Accessors
  ScopeValue*  owner()      const { return _owner; }
  Location     basic_lock() const { return _basic_lock;  }

  // Serialization of debugging information
  MonitorValue(DebugInfoReadStream* stream);
  void write_on(DebugInfoWriteStream* stream);

  // Printing
  void print_on(outputStream* st) const;
};

// DebugInfoReadStream specializes CompressedReadStream for reading
// debugging information. Used by ScopeDesc.

class DebugInfoReadStream : public CompressedReadStream {
 private:
  const nmethod* _code;
  const nmethod* code() const { return _code; }
 public:
  DebugInfoReadStream(const nmethod* code, int offset);
  Handle read_handle();
};

// DebugInfoWriteStream specializes CompressedWriteStream for
// writing debugging information. Used by ScopeDescRecorder.

class DebugInfoWriteStream : public CompressedWriteStream {
 private:
  DebugInformationRecorder* _recorder;
  DebugInformationRecorder* recorder() const { return _recorder; }
 public:
  DebugInfoWriteStream(DebugInformationRecorder* recorder, int initial_size);
  void write_handle(jobject h);
};

