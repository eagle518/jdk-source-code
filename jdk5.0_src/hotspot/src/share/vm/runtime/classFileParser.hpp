#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)classFileParser.hpp	1.71 04/02/06 00:29:13 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Parser for for .class files
//
// The bytes describing the class file structure is read from a Stream object

class ClassFileParser VALUE_OBJ_CLASS_SPEC {
 private:
  bool _need_verify;
  bool _relax_verify;  
  u2   _major_version;
  u2   _minor_version;
  symbolHandle _class_name;

  ClassFileStream* _stream;              // Actual input stream

  enum { LegalClass, LegalField, LegalMethod }; // used to verify unqualified names

  // Accessors
  ClassFileStream* stream()                        { return _stream; }
  void set_stream(ClassFileStream* st)             { _stream = st; }

  // Constant pool parsing
  void parse_constant_pool_utf8_entry(constantPoolHandle cp, int index, TRAPS);
  void parse_constant_pool_integer_entry(constantPoolHandle cp, int index, TRAPS);
  void parse_constant_pool_float_entry(constantPoolHandle cp, int index, TRAPS);
  void parse_constant_pool_long_entry(constantPoolHandle cp, int index, TRAPS);
  void parse_constant_pool_double_entry(constantPoolHandle cp, int index, TRAPS);
  void parse_constant_pool_class_entry(constantPoolHandle cp, int index, TRAPS);
  void parse_constant_pool_string_entry(constantPoolHandle cp, int index, TRAPS);
  void parse_constant_pool_fieldref_entry(constantPoolHandle cp, int index, TRAPS);
  void parse_constant_pool_methodref_entry(constantPoolHandle cp, int index, TRAPS);
  void parse_constant_pool_interfacemethodref_entry(constantPoolHandle cp, int index, TRAPS);
  void parse_constant_pool_nameandtype_entry(constantPoolHandle cp, int index, TRAPS);
  void parse_constant_pool_entries(constantPoolHandle cp, int length, TRAPS);

  constantPoolHandle parse_constant_pool(TRAPS);

  // Interface parsing
  objArrayHandle parse_interfaces(constantPoolHandle cp, Handle class_loader, 
                                  Handle protection_domain, TRAPS);

  // Field parsing
  void parse_field_attributes(constantPoolHandle cp, bool is_static, u2 signature_index, 
                              u2* constantvalue_index_addr, bool* is_synthetic_addr, 
                              u2* generic_signature_index_addr,
                              typeArrayHandle* field_annotations, TRAPS);
  typeArrayHandle parse_fields(constantPoolHandle cp, bool is_interface, 
                               struct FieldAllocationCount *fac,
                               objArrayHandle* fields_annotations, TRAPS);

  // Method parsing
  methodHandle parse_method(constantPoolHandle cp, bool is_interface, 
                            AccessFlags* promoted_flags,
                            typeArrayHandle* method_annotations,
                            typeArrayHandle* method_parameter_annotations,
                            typeArrayHandle* method_default_annotations,
                            TRAPS);
  objArrayHandle parse_methods (constantPoolHandle cp, bool is_interface, 
                                AccessFlags* promoted_flags,
                                objArrayOop* methods_annotations_oop,
                                objArrayOop* methods_parameter_annotations_oop,
                                objArrayOop* methods_default_annotations_oop,
                                TRAPS);
  typeArrayHandle sort_methods (objArrayHandle methods,
                                objArrayHandle methods_annotations,
                                objArrayHandle methods_parameter_annotations,
                                objArrayHandle methods_default_annotations,
                                TRAPS);
  typeArrayHandle parse_exception_table(u4 code_length, u4 exception_table_length, 
                                        constantPoolHandle cp, TRAPS);
  u_char* parse_linenumber_table(u4 code_attribute_length, u4 code_length,
                                 int* compressed_linenumber_table_size, TRAPS);
  u2* parse_localvariable_table(u4 code_length, u2 max_locals, u4 code_attribute_length,
                                constantPoolHandle cp, u2* localvariable_table_length,
                                bool isLVTT, TRAPS);
  u2* parse_checked_exceptions(u2* checked_exceptions_length, u4 method_attribute_length,
                               constantPoolHandle cp, TRAPS);
  void parse_type_array(u2 array_length, u4 code_length, u4* u1_index, u4* u2_index,
                        u1* u1_array, u2* u2_array, constantPoolHandle cp, TRAPS);
  void parse_stackmap_table(u4 code_attribute_length, u4 code_length, u2 max_locals, u2 max_stack, 
                            typeArrayHandle* stackmap_u1, typeArrayHandle* stackmap_u2, 
                            constantPoolHandle cp, TRAPS);

  // Classfile attribute parsing
  void parse_classfile_sourcefile_attribute(constantPoolHandle cp, instanceKlassHandle k, TRAPS);
  void parse_classfile_source_debug_extension_attribute(constantPoolHandle cp, 
                                                instanceKlassHandle k, int length, TRAPS);
  u2   parse_classfile_inner_classes_attribute(constantPoolHandle cp, 
                                               instanceKlassHandle k, TRAPS);
  void parse_classfile_attributes(constantPoolHandle cp, instanceKlassHandle k, TRAPS);
  void parse_classfile_synthetic_attribute(constantPoolHandle cp, instanceKlassHandle k, TRAPS);
  void parse_classfile_signature_attribute(constantPoolHandle cp, instanceKlassHandle k, TRAPS);
  
  // Annotations handling
  typeArrayHandle assemble_annotations(u1* runtime_visible_annotations,
                                       int runtime_visible_annotations_length,
                                       u1* runtime_invisible_annotations,
                                       int runtime_invisible_annotations_length, TRAPS);

  // Final setup
  int  compute_oop_map_size(instanceKlassHandle super, int nonstatic_oop_count, 
                            int first_nonstatic_oop_offset);
  void fill_oop_maps(instanceKlassHandle k, int nonstatic_oop_count, 
                     int first_nonstatic_oop_offset);
  void set_precomputed_flags(instanceKlassHandle k);
  objArrayHandle compute_transitive_interfaces(instanceKlassHandle super, 
                                               objArrayHandle local_ifs, TRAPS);

  // Special handling for certain classes.
  // Add the "discovered" field to java.lang.ref.Reference if
  // it does not exist.
  void java_lang_ref_Reference_fix_pre(typeArrayHandle* fields_ptr, 
    constantPoolHandle cp, FieldAllocationCount *fac_ptr, TRAPS);
  // Adjust the field allocation counts for java.lang.Class to add
  // fake fields.
  void java_lang_Class_fix_pre(objArrayHandle* methods_ptr,
    FieldAllocationCount *fac_ptr, TRAPS);
  // Adjust the next_nonstatic_oop_offset to place the fake fields
  // before any Java fields.
  void java_lang_Class_fix_post(int* next_nonstatic_oop_offset);

  // Format checker methods
  inline void classfile_parse_error(const char* msg, TRAPS) {
    ResourceMark rm(THREAD);
    Exceptions::fthrow(THREAD_AND_LOCATION, vmSymbols::java_lang_ClassFormatError(),
                       msg, _class_name->as_C_string(), CHECK);
  }
  inline void classfile_parse_error(const char* msg, int index, TRAPS) {
    ResourceMark rm(THREAD);
    Exceptions::fthrow(THREAD_AND_LOCATION, vmSymbols::java_lang_ClassFormatError(),
                       msg, index, _class_name->as_C_string(), CHECK);
  }
  inline void classfile_parse_error(const char* msg, const char *name, TRAPS) {
    ResourceMark rm(THREAD);
    Exceptions::fthrow(THREAD_AND_LOCATION, vmSymbols::java_lang_ClassFormatError(),
                       msg, name, _class_name->as_C_string(), CHECK);
  }
  inline void classfile_parse_error(const char* msg, int index, const char *name, TRAPS) {
    ResourceMark rm(THREAD);
    Exceptions::fthrow(THREAD_AND_LOCATION, vmSymbols::java_lang_ClassFormatError(),
                       msg, index, name, _class_name->as_C_string(), CHECK);
  }
  inline void truncated_class_file_error(TRAPS) {
    classfile_parse_error("Truncated class file %s", CHECK);
  }
  inline void guarantee_property(bool b, const char* msg, TRAPS) {
    if (!b) { classfile_parse_error(msg, CHECK); }
  }
  inline void guarantee_property(bool b, const char* msg, int index, TRAPS) {
    if (!b) { classfile_parse_error(msg, index, CHECK); }
  }
  inline void guarantee_property(bool b, const char* msg, const char *name, TRAPS) {
    if (!b) { classfile_parse_error(msg, name, CHECK); }
  }
  inline void guarantee_property(bool b, const char* msg, int index, const char *name, TRAPS) {
    if (!b) { classfile_parse_error(msg, index, name, CHECK); }
  }

  void check_property(bool property, const char* msg, int index, TRAPS);
  void verify_constantvalue(int constantvalue_index, int signature_index, constantPoolHandle cp, TRAPS);
  void verify_legal_utf8(const unsigned char* buffer, int length, TRAPS);
  void verify_legal_class_name(symbolHandle name, TRAPS);
  void verify_legal_field_name(symbolHandle name, TRAPS);
  void verify_legal_method_name(symbolHandle name, TRAPS);
  void verify_legal_field_signature(symbolHandle fieldname, symbolHandle signature, TRAPS);
  int  verify_legal_method_signature(symbolHandle methodname, symbolHandle signature, TRAPS);
  void verify_legal_class_modifiers(jint flags, TRAPS);
  void verify_legal_field_modifiers(jint flags, bool is_interface, TRAPS);
  void verify_legal_method_modifiers(jint flags, bool is_interface, symbolHandle name, TRAPS);
  bool verify_unqualified_name(char* name, unsigned int length, int type);
  char* skip_over_field_name(char* name, bool slash_ok, unsigned int length);
  char* skip_over_field_signature(char* signature, bool void_ok, unsigned int length, TRAPS);

 public:
  // Constructor
  ClassFileParser(ClassFileStream* st) { set_stream(st); }

  // Parse .class file and return new klassOop. The klassOop is not hooked up
  // to the system dictionary or any other structures, so a .class file can 
  // be loaded several times if desired. 
  // The system dictionary hookup is done by the caller.
  //
  // "parsed_name" is updated by this method, and is the name found
  // while parsing the stream.
  instanceKlassHandle parseClassFile(symbolHandle name, 
                                     Handle class_loader, 
                                     Handle protection_domain, 
                                     symbolHandle& parsed_name,
                                     TRAPS);

  // Verifier checks
  static void check_super_class_access(instanceKlassHandle this_klass, TRAPS);
  static void check_super_interface_access(instanceKlassHandle this_klass, TRAPS);
  static void check_final_method_override(instanceKlassHandle this_klass, TRAPS);
  static void check_illegal_static_method(instanceKlassHandle this_klass, TRAPS);
};
