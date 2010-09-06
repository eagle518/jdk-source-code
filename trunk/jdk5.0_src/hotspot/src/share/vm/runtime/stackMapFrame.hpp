#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)stackMapFrame.hpp	1.7 03/12/23 16:44:11 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// A StackMapFrame represents one frame in the stack map attribute.

enum {
  FLAG_THIS_UNINIT = 0x01
};

extern void verify_error(const char* msg, methodHandle m, u2 offset, TRAPS);

class StackMapFrame : public CHeapObj {
  friend class StackMapTable;
  friend class StackMapReader;
  friend class ClassVerifier;

  u2 _offset;
  u2 _locals_size;  // number of valid type elements in _locals 
  u2 _stack_size;   // number of valid type elements in _stack

  u2 _max_locals;
  u2 _max_stack;

  u1 _flags;
  VerificationType** _locals; // local variable type array
  VerificationType** _stack;  // operand stack type array

  ClassVerifier* _verifier;  // the verifier verifying this method

  // constructors

  // This constructor is used by the type checker to allocate frames 
  // in type state, which have _max_locals and _max_stack array elements
  // in _locals and _stack.
  StackMapFrame(u2 max_locals, u2 max_stack, ClassVerifier* verifier);

  // Copy constructor
  StackMapFrame(StackMapFrame* f);

  // This constructor is used to initialize stackmap frames in stackmap table,
  // which have _locals_size and _stack_size array elements in _locals and _stack.
  StackMapFrame(u2 offset,
                u1 flags,
                u2 locals_size,
                u2 stack_size, 
                u2 max_locals,
                u2 max_stack,
                VerificationType** locals,
                VerificationType** stack) : _offset(offset), _flags(flags),
                                            _locals_size(locals_size), _stack_size(stack_size),                                          
                                            _max_locals(max_locals), _max_stack(max_stack),
                                            _locals(locals), _stack(stack) { }

  ~StackMapFrame();
  inline void set_offset(u2 offset)           { _offset = offset; }
  inline void set_verifier(ClassVerifier* v)  { _verifier = v; }
  inline void set_flags(u1 flags)             { _flags = flags; }
  inline void set_locals_size(u2 locals_size) { _locals_size = locals_size; }
  inline void set_stack_size(u2 stack_size)   { _stack_size = stack_size; }
  inline void clear_stack()                   { _stack_size = 0; }
  inline u2 offset()	const                   { return _offset; }
  inline ClassVerifier* verifier() const      { return _verifier; }
  inline u1 flags() const                     { return _flags; }
  inline u2 locals_size() const               { return _locals_size; }
  inline VerificationType** locals() const    { return _locals; }
  inline u2 stack_size() const                { return _stack_size; }
  inline VerificationType** stack() const     { return _stack; }
  inline u2 max_locals() const                { return _max_locals; }
  inline u2 max_stack() const                 { return _max_stack; }
  inline bool flag_this_uninit() const        { return _flags & FLAG_THIS_UNINIT; }

  // Set locals and stack types to bogus
  inline void reset() {
    int i;
    for (i = 0; i < _max_locals; i++) {
      _locals[i] = VerificationType::_bogus_type;
    }
    for (i = 0; i < _max_stack; i++) {
      _stack[i] = VerificationType::_bogus_type;
    }
  }

  // Return a StackMapFrame with the same local variable array and empty stack.
  // Stack array is allocate with unused one element.
  StackMapFrame* frame_in_exception_handler(u1 flags);

  // Set local variable type array based on m's signature.
  VerificationType* set_locals_from_arg(const methodHandle m, ObjType* klass, TRAPS);

  // Search local variable type array and stack type array.
  // Return true if an uninitialized object is found.
  bool has_new_object() const;

  // Search local variable type array and stack type array.
  // Set every element with type of old_object to new_object.
  void initialize_object(UninitializedType* old_object, ObjType* new_object);

  // Copy local variable type array in src into this local variable type array.
  void copy_locals(const StackMapFrame* src);

  // Copy stack type array in src into this stack type array.
  void copy_stack(const StackMapFrame* src);

  // Return true if this stack map frame is assignable to target.
  bool is_assignable_to(const StackMapFrame* target, TRAPS) const;

  // Push type into stack type array.
  inline void push_stack(VerificationType* type, TRAPS) {
    if (_stack_size >= _max_stack) {
      verify_error("Operand stack overflow in method %s at offset %d",
                    _verifier->_method, _offset, CHECK);
    }
    _stack[_stack_size++] = type;
  }

  // Pop and return the top type on stack without verifying.
  inline VerificationType* pop_stack(TRAPS) {
    if (_stack_size <= 0) {
      verify_error("Operand stack underflow in method %s at offset %d",
                   _verifier->_method, _offset, CHECK_0);
    }
    // Put bogus type to indicate it's no longer valid.
    // Added to make it consistent with the other pop_stack method.
    VerificationType* top = _stack[--_stack_size];
    NOT_PRODUCT( _stack[_stack_size] = VerificationType::_bogus_type; )
    return top;
  }

  // Pop and return the top type on stack type array after verifying it
  // is assignable to type.
  VerificationType* pop_stack(VerificationType* type, TRAPS);

  // Return the type at index in local variable array after verifying
  // it is assignable to type.
  VerificationType* get_local(u2 index, VerificationType* type, TRAPS);

  // Set element at index in local variable array to type.
  void set_local(u2 index, VerificationType* type, TRAPS);

  // Private auxiliary method used only in is_assignable_to(StackMapFrame).
  // Returns true if src is assignable to target.
  static bool is_assignable_to(VerificationType** src, VerificationType** target, u2 len, TRAPS);

  // Debugging
  void print() const PRODUCT_RETURN;
};

