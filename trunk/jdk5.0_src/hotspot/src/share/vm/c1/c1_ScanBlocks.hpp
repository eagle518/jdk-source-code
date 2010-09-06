#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)c1_ScanBlocks.hpp	1.22 03/12/23 16:39:19 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

class BlockBegin;
class BlockList;

class ALocal: public CompilationResourceObj {
 private:
  int      _index;
  ValueTag _type;
  int      _access_count;
 public:
  ALocal(int index, ValueTag type, int access_count): _index(index), _type(type), _access_count(access_count) {}
  int  index() const  { return _index;  }
  bool is_oop() const { return _type == objectTag; }
  ValueTag type() const { return _type; }
  int  access_count() const { return _access_count; }

  static int sort_by_access_count(ALocal** a, ALocal** b) {
    return (*b)->access_count() - (*a)->access_count();
  }
  static int sort_by_index(ALocal** a, ALocal** b) {
    return (*b)->index() - (*a)->index();
  }
};


define_array(ALocalArray, ALocal*)
define_stack(ALocalList, ALocalArray)


class ScanResult: public CompilationResourceObj {
 private:
  bool _has_calls;
  bool _has_slow_cases;
  bool _has_doubles;
  bool _has_floats;
  bool _has_class_init;
 public:
  ScanResult(): 
       _has_calls(false)
     , _has_slow_cases(false)
     , _has_doubles(false)
     , _has_floats(false)
     , _has_class_init(false)
       {}
  bool has_calls() const      { return _has_calls;      }
  bool has_slow_cases() const { return _has_slow_cases; }
  bool has_doubles() const    { return _has_doubles;    }
  bool has_floats() const     { return _has_floats;     }
  bool has_class_init() const { return _has_class_init; }

  void set_has_calls(bool f)       { _has_calls = f;      }
  void set_has_slow_cases(bool f)  { _has_slow_cases = f; }
  void set_has_doubles(bool f)     { _has_doubles = f;    }
  void set_has_floats(bool f)      { _has_floats = f;     }
  void set_has_class_init(bool f)  { _has_class_init = f; }
};


// Analyzes specified blocks for various properties (see fields)
class ScanBlocks: public StackObj {
 private:
  BlockList*  _blocks;
  ScanResult* _desc;
  intStack    _access_count[number_of_tags];
  intStack    _tags;

  void accumulate_access(int index, ValueTag tag, int count);
  void scan_block(BlockBegin* b, ScanResult* desc, bool live_only);

  int int_count_at     (int ix) const;
  int long_count_at    (int ix) const;
  int float_count_at   (int ix) const;
  int double_count_at  (int ix) const;
  int obj_count_at     (int ix) const;
  int address_count_at (int ix) const;

  void print_access_count(const char* label, intStack* count);

  void update_type(int index, ValueTag tag);
  void increment_count(ValueTag, int index, int count);
  int count_at(ValueTag tag, int index) const;
  bool is_only(int index, ValueType* type) const;
  const intStack* get_array(ValueTag tag) const;
  intStack* get_array(ValueTag tag);

 public:
  ScanBlocks(BlockList* blocks);

  void scan(ScanResult* desc, bool live_only = false);

  // return sorted list of locals
  ALocalList* most_used_float_locals();
  ALocalList* most_used_locals();

  bool is_int_only     (int index) const;
  bool is_long_only    (int index) const;
  bool is_float_only   (int index) const;
  bool is_double_only  (int index) const;
  bool is_obj_only     (int index) const;
  bool is_address_only (int index) const;


  void print(ScanResult* r) PRODUCT_RETURN;
};
