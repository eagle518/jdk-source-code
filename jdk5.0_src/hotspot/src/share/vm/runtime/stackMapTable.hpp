#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)stackMapTable.hpp	1.6 03/12/23 16:44:11 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// StackMapTable class is the StackMap table used by type checker
class StackMapTable : public CHeapObj {
  friend class ClassVerifier;

 private:
  u2	            _frame_count;     // Stackmap frame count 
  StackMapFrame**	_frame_array;

  StackMapTable(StackMapReader reader, u2 max_locals, u2 max_stack, TRAPS);
  ~StackMapTable();

  inline u2 get_frame_count() const { return _frame_count; }
  inline u2 get_offset(int index) const { return _frame_array[index]->offset(); }

  // Match and/or update current_frame to the frame in stackmap table with
  // specified offset. Return true if the two frames match. 
  bool match_stackmap(StackMapFrame* current_frame, u2 offset,
                      bool match, bool update, TRAPS) const;
  // Match and/or update current_frame to the frame in stackmap table with
  // specified offset and frame index. Return true if the two frames match. 
  bool match_stackmap(StackMapFrame* current_frame, u2 offset, u2 frame_index,
                      bool match, bool update, TRAPS) const;

  // Check jump instructions. Make sure there are no uninitialized 
  // instances on backward branch.
  void check_jump_target(StackMapFrame* frame, u2 target, TRAPS) const;

  // The following methods are only used inside this class.

  // Returns the frame array index where the frame with offset is stored. 
  u2 get_index_from_offset(u2 offset) const;

  // Make sure that there's no uninitialized object exist on backward branch.
  void check_new_object(const StackMapFrame* frame, u2 target, TRAPS) const;

  // Debugging
  void print() const PRODUCT_RETURN;
};

class StackMapReader : StackObj {
  friend class StackMapTable;  
  friend class ClassVerifier;

 private:
  // information about the class and method 
  constantPoolHandle  _cp;
  ClassVerifier* _verifier;

  // information about stackmap attribute
  int          _u1_index;
  int          _u2_index;
  typeArrayOop _u1_array;
  typeArrayOop _u2_array;

  // information get from the attribute
  u2  _frame_count;       // frame count 

  // Constructor
  StackMapReader(ClassVerifier* v) : _u1_index(0), _u2_index(0) {
    _verifier = v;
    methodHandle m = v->_method;
    if (m->has_stackmap_table()) {
      _cp = m->constants();
      _u1_array = m->stackmap_u1();
      _u2_array = m->stackmap_u2();
      _frame_count = get_u2();
    } else {
      // There's no stackmap table present. Frame count and size are 0.
      _frame_count = 0;
    }
  }

  inline u2 get_frame_count() const		{ return _frame_count; }
  StackMapFrame* next(u2 max_locals, u2 max_stack, TRAPS);

  // The following methods are only used by this class.

  // Read u1 from stream
  inline u1 get_u1() {
    assert(_u1_index < _u1_array->length(), "u1 array overflow");
    return _u1_array->byte_at(_u1_index++);
  }

  // Read u2 from stream
  inline u2 get_u2() {
    assert(_u2_index < _u2_array->length(), "u2 array overflow");
    return _u2_array->short_at(_u2_index++);
  }

  VerificationType** parse_stackmap_variables(u2 size, u1* flags, TRAPS);
};
