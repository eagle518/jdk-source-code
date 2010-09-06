#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)verifier.hpp	1.23 03/12/23 16:44:24 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// The verifier class
class Verifier : AllStatic {
 public:
  enum {
    STACKMAP_ATTRIBUTE_MAJOR_VERSION = 49
  };

  // Return false if the class is loaded by the bootstrap loader.
  static bool should_verify_for(oop class_loader);

  // Relax certain verifier checks to enable some broken 1.1 apps to run on 1.2.
  static bool relax_verify_for(oop class_loader);

  // Verify the byte codes of the class
  static void verify_byte_codes(instanceKlassHandle klass, TRAPS);
};

class ObjType;
class RefType;

// Hash bucket in class type hash table
class ClassTypeBucket : public CHeapObj {
  friend class ClassTypeHashtable;

  RefType *_class_type;         // The class type may be ObjType or ArrType
  bool _loadable;               // true if loadable by current class loader
  ClassTypeBucket *_next; 

  ClassTypeBucket(RefType *t) : _class_type(t), _next(NULL) { }
};

// Hash table to store class type objects 
class ClassTypeHashtable : public CHeapObj {
  friend class ClassVerifier;

 private:
  int _table_size;            // size of this table
  ClassTypeBucket **_table;
  ClassVerifier* _verifier;

  // construtor with initialized table
  ClassTypeHashtable(int hash_table_size, ClassVerifier* v);

  // destructor
  ~ClassTypeHashtable();

  // the hash function
  unsigned int hash_fun(symbolOop name);

 public:
  // Put the object/class type with name in table and return the type.
  // Name is in the form as in constantPool, e.g. java/lang/Object or [Ljava/lang/String;
  RefType* get_class_type_from_name(symbolHandle name, TRAPS);

  // used to store ObjType with klass loaded
  ObjType* get_object_type(symbolHandle name, instanceKlassHandle klass, bool loadable, TRAPS);
};

class RawBytecodeStream;
class StackMapFrame;
class StackMapTable;

// A new instance of this class is created for each class being verified
class ClassVerifier : public StackObj {
 private:
  // constructor
  ClassVerifier(instanceKlassHandle klass, TRAPS);

  // destructor
  ~ClassVerifier();

  void verify_class(TRAPS);
  void verify_method(methodHandle method, TRAPS);
  void verify_exception_handler_table(u4 code_length, int* code_data, TRAPS);
  void verify_local_variable_table(u4 code_length, int* code_data, TRAPS);
  klassOop load_class(symbolHandle name, TRAPS);
  klassOop load_cp_class(int index, constantPoolHandle cp, TRAPS);

  RefType* cp_index_to_type(int index, constantPoolHandle cp, TRAPS) {
    return _local_class_type_table->get_class_type_from_name(cp->klass_name_at(index), CHECK_0);
  }  

  RefType* cp_ref_index_to_type(int index, constantPoolHandle cp, TRAPS) {
    return cp_index_to_type(cp->klass_ref_index_at(index), cp, CHECK_0);
  }

  klassOop load_cp_ref_class(int index, constantPoolHandle cp, TRAPS) {
    return load_cp_class(cp->klass_ref_index_at(index), cp, CHECK_0);
  }
  
  bool is_protected_access(instanceKlassHandle this_class, klassOop target_class,
                           symbolOop field_name, symbolOop field_sig, bool is_method);

  int* generate_code_data(address bcp, u4 code_length, TRAPS);
  
  // Use this method to check legal target without throwing any exception.
  // Currently it is used to check legal pc values in exception table and local variable table,
  // where ClassFormatError is thrown when illegal targets are found.
  static bool is_legal_target(u2 target, u4 code_length, int* code_data) {
    return (target >= 0 && target < code_length && code_data[target] >= 0);
  }

  // This method throws VerifyError if branch target is illegal.
  void check_legal_target(u2 target, u4 code_length, int* code_data, u2 bci, TRAPS);

  void verify_constant_pool_type(int index, constantPoolHandle cp, unsigned int types, TRAPS);
  void verify_cp_class_type(int index, constantPoolHandle cp, TRAPS);

  u2 verify_stackmap_table(u2 stackmap_index, u2 bci, StackMapFrame* current_frame, 
                           StackMapTable* stackmap_table, bool no_control_flow, TRAPS);

  void verify_exception_handler_targets(u2 bci, bool this_uninit, StackMapFrame* current_frame, 
                                        StackMapTable* stackmap_table, TRAPS);

  bool verify_instruction(RawBytecodeStream* bcs, bool* this_uninit, u4 code_length, 
                          int* code_data, VerificationType* return_type, 
                          StackMapFrame* current_frame, StackMapTable* stackmap_table, 
                          constantPoolHandle cp, TRAPS);

  void verify_ldc(int opcode, u2 index, StackMapFrame *current_frame, 
                  constantPoolHandle cp, u2 bci, TRAPS);

  void verify_switch(RawBytecodeStream* bcs, u4 code_length, int* code_data, 
                     StackMapFrame* current_frame, StackMapTable* stackmap_table, TRAPS);

  void verify_field_instructions(RawBytecodeStream* bcs, StackMapFrame* current_frame, 
                                 constantPoolHandle cp, TRAPS);

  void verify_invoke_init(RawBytecodeStream* bcs, RefType* ref_class_type, StackMapFrame* current_frame, 
                          u4 code_length, bool* this_uninit, constantPoolHandle cp, TRAPS);

  void verify_invoke_instructions(RawBytecodeStream* bcs, u4 code_length, StackMapFrame* current_frame, 
                                  bool* this_uninit, VerificationType* return_type, 
                                  constantPoolHandle cp, TRAPS);

  VerificationType* get_newarray_type(u2 index, u2 bci, TRAPS);
  void verify_anewarray(u2 index, constantPoolHandle cp, StackMapFrame* current_frame, TRAPS);
  void verify_return_value(VerificationType* return_type, VerificationType* type, u2 offset, TRAPS);

  void verify_iload (u2 index, StackMapFrame* current_frame, TRAPS);
  void verify_lload (u2 index, StackMapFrame* current_frame, TRAPS);
  void verify_fload (u2 index, StackMapFrame* current_frame, TRAPS);
  void verify_dload (u2 index, StackMapFrame* current_frame, TRAPS);
  void verify_aload (u2 index, StackMapFrame* current_frame, TRAPS);
  void verify_istore(u2 index, StackMapFrame* current_frame, TRAPS);
  void verify_lstore(u2 index, StackMapFrame* current_frame, TRAPS);
  void verify_fstore(u2 index, StackMapFrame* current_frame, TRAPS);
  void verify_dstore(u2 index, StackMapFrame* current_frame, TRAPS);
  void verify_astore(u2 index, StackMapFrame* current_frame, TRAPS);
  void verify_iinc  (u2 index, StackMapFrame* current_frame, TRAPS);

  friend class ArrType;
  friend class ObjType;
  friend class VerificationType;
  friend class StackMapFrame;
  friend class StackMapReader;
  friend class StackMapTable;
  friend class ClassTypeHashtable;
  friend class Verifier;

  ObjType*  _object_class_type;
  ObjType*  _string_class_type;
  ObjType*  _throwable_class_type;
  ObjType*  _current_class_type;
  ObjType*  _super_class_type;

  instanceKlassHandle _klass;  // the class being verified
  methodHandle        _method; // current method being verified

  ClassTypeHashtable* _local_class_type_table;   // stores class types

  static bool _verify_verbose;  // for debugging
};
