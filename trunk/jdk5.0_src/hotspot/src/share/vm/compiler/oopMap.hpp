#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)oopMap.hpp	1.68 03/12/23 16:40:05 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Interface for generating the frame map for compiled code.  A frame map 
// describes for a specific pc whether each register and frame stack slot is:
//   Oop         - A GC root for current frame
//   Value       - Live non-oop, non-float value: int, either half of double
//   Dead        - Dead; can be Zapped for debugging
//   CalleeXX    - Callee saved; also describes which caller register is saved
//   DerivedXX   - A derived oop; original oop is described.
//
// OopMapValue describes a single OopMap entry

class frame;
class RegisterMap;
class DerivedPointerEntry;

class OopMapValue: public StackObj {
  friend class VMStructs;
private:
  short _value;
  int value() const                                 { return _value; }
  void set_value(int value)                         { _value = value; }
  short _content_reg;

public:
  // Constants
  enum { type_bits                = 5,
         register_bits            = BitsPerShort - type_bits };
 
  enum { type_shift               = 0,
         register_shift           = type_bits };

  enum { type_mask                = right_n_bits(type_bits),
         type_mask_in_place       = type_mask << type_shift,
         register_mask            = right_n_bits(register_bits),
         register_mask_in_place   = register_mask << register_shift };

  enum oop_types {              // must fit in type_bits
         unused_value =0,       // powers of 2, for masking OopMapStream
         oop_value = 1,
         value_value = 2,
         dead_value = 4,
         callee_saved_value = 8,
         derived_oop_value= 16 };

  // Constructors
  OopMapValue () { set_value(0); set_content_reg(VMReg::Name(0)); }
  OopMapValue (VMReg::Name reg, oop_types t) { set_reg_type(reg,t); }
  OopMapValue (VMReg::Name reg, oop_types t, VMReg::Name reg2) { set_reg_type(reg,t); set_content_reg(reg2); }
  OopMapValue (CompressedReadStream* stream) { read_from(stream); }

  // Archiving
  void write_on(CompressedWriteStream* stream) {
    stream->write_int(value());
    if(is_callee_saved() || is_derived_oop()) {
      stream->write_int(content_reg());
    }
  }

  void read_from(CompressedReadStream* stream) {
    set_value(stream->read_int());
    if(is_callee_saved() || is_derived_oop()) {
      set_content_reg(VMReg::Name(stream->read_int()));
    }
  }

  // Querying
  bool is_oop()               { return mask_bits(value(), type_mask_in_place) == oop_value; } 
  bool is_value()             { return mask_bits(value(), type_mask_in_place) == value_value; } 
  bool is_dead()              { return mask_bits(value(), type_mask_in_place) == dead_value; } 
  bool is_callee_saved()      { return mask_bits(value(), type_mask_in_place) == callee_saved_value; } 
  bool is_derived_oop()       { return mask_bits(value(), type_mask_in_place) == derived_oop_value; } 

  void set_oop()              { set_value((value() & register_mask_in_place) | oop_value); } 
  void set_value()            { set_value((value() & register_mask_in_place) | value_value); } 
  void set_dead()             { set_value((value() & register_mask_in_place) | dead_value); } 
  void set_callee_saved()     { set_value((value() & register_mask_in_place) | callee_saved_value); } 
  void set_derived_oop()      { set_value((value() & register_mask_in_place) | derived_oop_value); } 

  VMReg::Name reg() const { return VMReg::Name(mask_bits(value(), register_mask_in_place) >> register_shift); }
  oop_types type() const      { return (oop_types)mask_bits(value(), type_mask_in_place); }

  void set_reg_type(VMReg::Name p, oop_types t) { 
    set_value(((int)p << register_shift) | t);
    assert(reg() == p, "sanity check" );
    assert(type() == t, "sanity check" );
  }


  VMReg::Name content_reg() const       { return VMReg::Name(_content_reg); }
  void set_content_reg(VMReg::Name r)   { _content_reg = r; }

  // Physical location queries
  bool is_register_loc()      { return (reg() <  (VMReg::Name)SharedInfo::stack0); }
  bool is_stack_loc()         { return (reg() >= (VMReg::Name)SharedInfo::stack0); }

  // Returns offset from sp.
  int stack_offset() {
    assert(is_stack_loc(), "must be stack location");
    return reg() - SharedInfo::stack0;
  }

  void print( ) const PRODUCT_RETURN;
};


class OopMap: public ResourceObj {
  friend class OopMapStream;
  friend class VMStructs;
 private:
  int  _pc_offset; 
  bool _at_call;
  int  _omv_count;
  int  _omv_data_size;
  unsigned char* _omv_data;
  CompressedWriteStream* _write_stream;

  debug_only( OopMapValue::oop_types* _locs_used; int _locs_length;)

  // Accessors
  unsigned char* omv_data() const             { return _omv_data; }
  void set_omv_data(unsigned char* value)     { _omv_data = value; }
  int omv_data_size() const                   { return _omv_data_size; }
  void set_omv_data_size(int value)           { _omv_data_size = value; }
  int omv_count() const                       { return _omv_count; }
  void set_omv_count(int value)               { _omv_count = value; }
  void increment_count()                      { _omv_count++; }
  CompressedWriteStream* write_stream() const { return _write_stream; }
  void set_write_stream(CompressedWriteStream* value) { _write_stream = value; }
  // frame_size units are stack-slots (4 bytes) NOT intptr_t; we can name odd
  // slots to hold 4-byte values like ints and floats in the LP64 build.
  VMReg::Name map_compiler_reg_to_oopmap_reg(OptoReg::Name reg, int frame_size, int arg_count);

 private:
  enum DeepCopyToken { _deep_copy_token };
  OopMap(DeepCopyToken, OopMap* source);  // used only by deep_copy

 public:
  OopMap(int frame_size, int arg_count);

  // (pc-offset, at_call) handling
  int offset() const     { return _pc_offset; }
  bool at_call() const   { return _at_call; }
  void set_offset(int o, bool at_call) { _pc_offset = o; _at_call = at_call; }

  // Check to avoid double insertion
  debug_only(OopMapValue::oop_types locs_used( int indx ) { return _locs_used[indx]; })
  
  // Construction
  // frame_size units are stack-slots (4 bytes) NOT intptr_t; we can name odd
  // slots to hold 4-byte values like ints and floats in the LP64 build.
  void set_oop  ( OptoReg::Name local, int frame_size, int arg_count );
  void set_value( OptoReg::Name local, int frame_size, int arg_count );
  void set_dead ( OptoReg::Name local, int frame_size, int arg_count );
  void set_callee_saved( OptoReg::Name local, int frame_size, int arg_count, OptoReg::Name caller_machine_register );
  void set_derived_oop ( OptoReg::Name local, int frame_size, int arg_count, OptoReg::Name derived_from_local_register );
  void set_xxx(OptoReg::Name reg, OopMapValue::oop_types x, int frame_size, int arg_count, OptoReg::Name optional);

  int heap_size() const;
  void copy_to(address addr);
  OopMap* deep_copy();

  bool has_derived_pointer() const PRODUCT_RETURN0;

  // This routine sets all unused_value slots in [start..limit) to dead_value.
  void set_unused_as_dead(int start, int limit, int frame_size, int arg_count);

  // Printing
  void print() const PRODUCT_RETURN;
};


class OopMapSet : public ResourceObj {
  friend class VMStructs;
 private:
  int _om_count;
  int _om_size;
  OopMap** _om_data;

  int om_count() const              { return _om_count; }
  void set_om_count(int value)      { _om_count = value; }
  void increment_count()            { _om_count++; }
  int om_size() const               { return _om_size; }
  void set_om_size(int value)       { _om_size = value; }
  OopMap** om_data() const          { return _om_data; }
  void set_om_data(OopMap** value)  { _om_data = value; }
  void grow_om_data();
  void set(int index,OopMap* value) { assert((index == 0) || ((index > 0) && (index < om_size())),"bad index"); _om_data[index] = value; }

 public:
  OopMapSet();

  // returns the number of OopMaps in this OopMapSet
  int size() const            { return _om_count; }
  // returns the OopMap at a given index
  OopMap* at(int index) const { assert((index >= 0) && (index <= om_count()),"bad index"); return _om_data[index]; }

  // Collect OopMaps.
  void add_gc_map(int pc, bool at_call, OopMap* map);

  // Returns the only oop map. Used for reconstructing 
  // Adapter frames during deoptimization
  OopMap* singular_oop_map();

  // returns OopMap in that is anchored to the pc
  OopMap* find_map_at_offset(int pc_offset, bool at_call) const; 

  int heap_size() const;
  void copy_to(address addr);

#ifndef CORE
  // Iterates through frame for a compiled method
  static void oops_do            (const frame* fr, CodeBlob* cb,
				  const RegisterMap* reg_map, OopClosure* f); 
  static void update_register_map(const frame* fr, CodeBlob* cb,       RegisterMap *reg_map);

  // Iterates through frame for a compiled method for dead ones and values, too
  static void all_do(const frame* fr, CodeBlob* cb, const RegisterMap* reg_map,
                     OopClosure* oop_fn,
		     void derived_oop_fn(oop* base, oop* derived),
                     OopClosure* value_fn, OopClosure* dead_fn);
#endif // CORE

  // Printing
  void print() const PRODUCT_RETURN;
};


class OopMapStream : public StackObj {
 private:
  CompressedReadStream* _stream;
  int _mask;
  int _size;
  int _position;
  bool _valid_omv;
  OopMapValue _omv;
  void find_next();

 public:
  OopMapStream(OopMap* oop_map);
  OopMapStream(OopMap* oop_map, int oop_types_mask);
  bool is_done()                        { if(!_valid_omv) { find_next(); } return !_valid_omv; }
  void next()                           { find_next(); }
  OopMapValue current()                 { return _omv; }
};


// Derived pointer support. This table keeps track of all derived points on a
// stack.  It is cleared before each scavenge/GC.  During the traversal of all
// oops, it is filled in with references to all locations that contains a
// derived oop (assumed to be very few).  When the GC is complete, the derived
// pointers are updated based on their base pointers new value and an offset.
#ifdef COMPILER2
class DerivedPointerTable : public AllStatic {
  friend class VMStructs;
 private:
   static GrowableArray<DerivedPointerEntry*>* _list; 
   static bool _active;                      // do not record pointers for verify pass etc.
 public:  
  static void clear();                       // Called before scavenge/GC
  static void add(oop *derived, oop *base);  // Called during scavenge/GC
  static void update_pointers();             // Called after  scavenge/GC  
  static bool is_empty()                     { return _list == NULL || _list->is_empty(); }
  static bool is_active()                    { return _active; }
  static void set_active(bool value)         { _active = value; }
};

// A utility class to temporarily "deactivate" the DerivedPointerTable.
// (Note: clients are responsible for any MT-safety issues)
class DerivedPointerTableDeactivate: public StackObj {
 private:
  bool _active;
 public:
  DerivedPointerTableDeactivate() {
    _active = DerivedPointerTable::is_active();
    if (_active) {
      DerivedPointerTable::set_active(false);
    }
  }

  ~DerivedPointerTableDeactivate() {
    assert(!DerivedPointerTable::is_active(),
           "Inconsistency: not MT-safe");
    if (_active) {
      DerivedPointerTable::set_active(true);
    }
  }
};
#endif // COMPILER2
