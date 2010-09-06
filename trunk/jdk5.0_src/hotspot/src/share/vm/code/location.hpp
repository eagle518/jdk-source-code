#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)location.hpp	1.39 03/12/23 16:39:53 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// A Location describes a concrete machine variable location
// (such as integer or floating point register or a stack-held
// variable). Used when generating debug-information for nmethods.
//
// Encoding:
//
// bits:
//  Where:  [15]
//  Type:   [14..12]
//  Offset: [11..0]

class Location VALUE_OBJ_CLASS_SPEC { 
  friend class VMStructs;
 public:
  enum Where {
    on_stack,
    in_register
  };

  enum Type {
    normal,                     // Ints, floats, double halves
    oop,                        // Oop (please GC me!)
    int_in_long,                // Integer held in long register
    lng,                        // Long held in one register
    float_in_dbl,               // Float held in double register
    dbl,                        // Double held in one register
    addr,                       // JSR return address
    invalid                     // Invalid location
  };


 private:
  static const int OFFSET_MASK;
  static const int OFFSET_SHIFT;
  static const int TYPE_MASK;
  static const int TYPE_SHIFT;
  static const int WHERE_MASK;
  static const int WHERE_SHIFT;

  uint16_t _value;

  // Create a bit-packed Location
  Location(Where where_, Type type_, unsigned offset_) {
    set(where_, type_, offset_);
    assert( where () == where_ , "" );
    assert( type  () == type_  , "" );
    assert( offset() == offset_, "" );
  }

  inline void set(Where where_, Type type_, unsigned offset_) {
    _value = (uint16_t) ((where_  << WHERE_SHIFT) |
                         (type_   << TYPE_SHIFT)  |
                         ((offset_ << OFFSET_SHIFT) & OFFSET_MASK));
  }

 public:

  // Stack location Factory.  Offset is 4-byte aligned; remove low bits
  static Location new_stk_loc( Type t, int offset ) { return Location(on_stack,t,offset>>LogBytesPerInt); }
  // Register location Factory
  static Location new_reg_loc( Type t, int regnum ) { return Location(in_register,t,regnum); }
  // Invalid 
  static Location invalid_loc(                    ) { return Location(in_register,invalid,(unsigned) -1); }
  // Default constructor
  Location() { set(on_stack,invalid,(unsigned) -1); }

  // Bit field accessors
  Where where()  const { return (Where)       ((_value & WHERE_MASK)  >> WHERE_SHIFT);}
  Type  type()   const { return (Type)        ((_value & TYPE_MASK)   >> TYPE_SHIFT); }
  unsigned offset() const { return (unsigned) ((_value & OFFSET_MASK) >> OFFSET_SHIFT); }

  // Accessors
  bool is_register() const    { return where() == in_register; }
  bool is_stack() const       { return where() == on_stack;    }

  int stack_offset() const    { assert(where() == on_stack,    "wrong Where"); return offset()<<LogBytesPerInt; }
  int register_number() const { assert(where() == in_register, "wrong Where"); return offset()   ; }
  
  // Printing
  void print_on(outputStream* st) const;

  // Serialization of debugging information
  Location(DebugInfoReadStream* stream);
  void write_on(DebugInfoWriteStream* stream);

  // check
  static bool legal_offset_in_bytes(int offset_in_bytes);
};
