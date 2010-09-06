#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)classFileParser.cpp	1.238 04/07/13 10:07:23 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_classFileParser.cpp.incl"

// We generally try to create the oops directly when parsing, rather than allocating
// temporary data structures and copying the bytes twice. A temporary area is only
// needed when parsing utf8 entries in the constant pool and when parsing line number
// tables.

// We add assert in debug mode when class format is not checked.

#ifdef ASSERT
void inline assert_property(bool b, const char* msg, TRAPS) {
  if (!b) { fatal(msg); }
}
#else
#define assert_property(b, msg, traps)
#endif

void ClassFileParser::check_property(bool property, const char* msg, int index, TRAPS) {
  if (_need_verify) {
    guarantee_property(property, msg, index, CHECK);
  } else {
    assert_property(property, msg, CHECK);
  }
}


#define JAVA_CLASSFILE_MAGIC              0xCAFEBABE
#define JAVA_MIN_SUPPORTED_VERSION        45
#define JAVA_MAX_SUPPORTED_VERSION        49
#define JAVA_MAX_SUPPORTED_MINOR_VERSION  0

// Used for two backward compatibility reasons:
// - to check for new additions to the class file format in JDK1.5
// - to check for bug fixes in the format checker in JDK1.5
#define JAVA_1_5_VERSION                  49


void ClassFileParser::parse_constant_pool_utf8_entry(constantPoolHandle cp, int index, TRAPS) {  
  u2  utf8_length = stream()->get_u2(CHECK);
  u1* utf8_buffer = stream()->get_u1_buffer(utf8_length);

  if (utf8_buffer != NULL) {
    // Before storing the symbol, make sure it's legal
    verify_legal_utf8((unsigned char*)utf8_buffer, utf8_length, CHECK);
    // Got utf8 string, set stream position forward
    stream()->skip_u1(utf8_length, CHECK);
    // Optimistically assume that only 1 byte UTF format is used (common case)
    symbolOop result = oopFactory::new_symbol((char*)utf8_buffer, utf8_length, CHECK);
    cp->symbol_at_put(index, result);
  } else {
    truncated_class_file_error(CHECK);
  }
}



void ClassFileParser::parse_constant_pool_integer_entry(constantPoolHandle cp, int index, TRAPS) {
  u4 bytes = stream()->get_u4(CHECK);
  cp->int_at_put(index, (jint) bytes);
}


void ClassFileParser::parse_constant_pool_float_entry(constantPoolHandle cp, int index, TRAPS) {
  u4 bytes = stream()->get_u4(CHECK);
  cp->float_at_put(index, *(jfloat*)&bytes);      // %%%%% move cast into separate function
}


void ClassFileParser::parse_constant_pool_long_entry(constantPoolHandle cp, int index, TRAPS) {
  u8 bytes = stream()->get_u8(CHECK);
  cp->long_at_put(index, bytes);
}


void ClassFileParser::parse_constant_pool_double_entry(constantPoolHandle cp, int index, TRAPS) {
  u8 bytes = stream()->get_u8(CHECK);
  cp->double_at_put(index, *(jdouble*)&bytes);    // %%%%% move cast into separate function
}


void ClassFileParser::parse_constant_pool_class_entry(constantPoolHandle cp, int index, TRAPS) {
  u2 name_index = stream()->get_u2(CHECK);
  cp->klass_index_at_put(index, name_index);
}


void ClassFileParser::parse_constant_pool_string_entry(constantPoolHandle cp, int index, TRAPS) {
  u2 string_index = stream()->get_u2(CHECK);
  cp->string_index_at_put(index, string_index);
}


void ClassFileParser::parse_constant_pool_fieldref_entry(constantPoolHandle cp, int index, TRAPS) {
  u2 class_index = stream()->get_u2(CHECK);
  u2 name_and_type_index = stream()->get_u2(CHECK);
  cp->field_at_put(index, class_index, name_and_type_index);
}


void ClassFileParser::parse_constant_pool_methodref_entry(constantPoolHandle cp, int index, TRAPS) {
  u2 class_index = stream()->get_u2(CHECK);
  u2 name_and_type_index = stream()->get_u2(CHECK);
  cp->method_at_put(index, class_index, name_and_type_index);
}


void ClassFileParser::parse_constant_pool_interfacemethodref_entry(constantPoolHandle cp, 
                                                                   int index, TRAPS) {
  u2 class_index = stream()->get_u2(CHECK);
  u2 name_and_type_index = stream()->get_u2(CHECK);
  cp->interface_method_at_put(index, class_index, name_and_type_index);
}

void ClassFileParser::parse_constant_pool_nameandtype_entry(constantPoolHandle cp, 
                                                            int index, TRAPS) {
  u2 name_index = stream()->get_u2(CHECK);
  u2 signature_index = stream()->get_u2(CHECK);
  cp->name_and_type_at_put(index, name_index, signature_index);
}

void ClassFileParser::parse_constant_pool_entries(constantPoolHandle cp, int length, TRAPS) {
  HandleMark hm(THREAD);
    
  // parsing  Index 0 is unused
  for (int index = 1; index < length; index++) {
    u1 tag = stream()->get_u1(CHECK);
    switch (tag) {
      case JVM_CONSTANT_Class :
        parse_constant_pool_class_entry(cp, index, CHECK);
        break;
      case JVM_CONSTANT_Fieldref :
        parse_constant_pool_fieldref_entry(cp, index, CHECK);
        break;
      case JVM_CONSTANT_Methodref :
        parse_constant_pool_methodref_entry(cp, index, CHECK);
        break;
      case JVM_CONSTANT_InterfaceMethodref :
        parse_constant_pool_interfacemethodref_entry(cp, index, CHECK);
        break;
      case JVM_CONSTANT_String :
        parse_constant_pool_string_entry(cp, index, CHECK);
        break;
      case JVM_CONSTANT_Integer :
        parse_constant_pool_integer_entry(cp, index, CHECK);
        break;
      case JVM_CONSTANT_Float :
        parse_constant_pool_float_entry(cp, index, CHECK);
        break;
      case JVM_CONSTANT_Long :
        // A mangled type might cause you to overrun allocated memory
        check_property(index+1 < length, 
                       "Invalid constant pool entry %d in class file %s", 
                       index, 
                       CHECK);
        parse_constant_pool_long_entry(cp, index, CHECK);
        index++;   // Skip entry following eigth-byte constant, see JVM book p. 98
        break;
      case JVM_CONSTANT_Double :
        // A mangled type might cause you to overrun allocated memory
        check_property(index+1 < length, 
                       "Invalid constant pool entry %d in class file %s", 
                       index, 
                       CHECK);
        parse_constant_pool_double_entry(cp, index, CHECK);
        index++;   // Skip entry following eigth-byte constant, see JVM book p. 98
        break;
      case JVM_CONSTANT_NameAndType :
        parse_constant_pool_nameandtype_entry(cp, index, CHECK);
        break;
      case JVM_CONSTANT_Utf8 :
        parse_constant_pool_utf8_entry(cp, index, CHECK);
        break;
      default:
        classfile_parse_error("Unknown constant tag %d in class file %s", tag, CHECK);
        break;
    }
  }
}

bool inline valid_cp_range(int index, int length) { return (index > 0 && index < length); }

constantPoolHandle ClassFileParser::parse_constant_pool(TRAPS) {
  constantPoolHandle nullHandle;

  u2 length = stream()->get_u2(CHECK_(nullHandle));
  check_property(length >= 1, 
                 "Illegal constant pool size %d in class file %s", 
                 length,
                 CHECK_(nullHandle));
  constantPoolOop constant_pool =
                      oopFactory::new_constantPool(length, CHECK_(nullHandle));
  constantPoolHandle cp (THREAD, constant_pool);
  
  #ifndef PRODUCT
  cp->set_partially_loaded();    // Enables heap verify to work on partial constantPoolOops
  #endif

  // parsing constant pool entries
  parse_constant_pool_entries(cp, length, CHECK_(nullHandle));

  int index = 1;  // declared outside of loops for portability

  // first verification pass - validate cross references and fixup class and string constants
  for (index = 1; index < length; index++) {          // Index 0 is unused
    switch (cp->tag_at(index).value()) {
      case JVM_CONSTANT_Class :
        ShouldNotReachHere();     // Only JVM_CONSTANT_ClassIndex should be present
        break;
      case JVM_CONSTANT_Fieldref :
        // fall through
      case JVM_CONSTANT_Methodref :
        // fall through
      case JVM_CONSTANT_InterfaceMethodref : {
        int klass_ref_index = cp->klass_ref_index_at(index);
        int name_and_type_ref_index = cp->name_and_type_ref_index_at(index);
        check_property(valid_cp_range(klass_ref_index, length) &&
                       cp->tag_at(klass_ref_index).is_klass_reference(), 
                       "Invalid constant pool index %d in class file %s", 
                       klass_ref_index, 
                       CHECK_(nullHandle));
        check_property(valid_cp_range(name_and_type_ref_index, length) &&
                       cp->tag_at(name_and_type_ref_index).is_name_and_type(), 
                       "Invalid constant pool index %d in class file %s", 
                       name_and_type_ref_index,
                       CHECK_(nullHandle));
        break;
      }
      case JVM_CONSTANT_String :
        ShouldNotReachHere();     // Only JVM_CONSTANT_StringIndex should be present
        break;
      case JVM_CONSTANT_Integer :
        break;
      case JVM_CONSTANT_Float :
        break;
      case JVM_CONSTANT_Long :
      case JVM_CONSTANT_Double :
        index++;
        check_property((index < length && cp->tag_at(index).is_invalid()), 
                       "Improper constant pool long/double index %d in class file %s", 
                       index,
                       CHECK_(nullHandle));
        break;
      case JVM_CONSTANT_NameAndType : {
        int name_ref_index = cp->name_ref_index_at(index);
        int signature_ref_index = cp->signature_ref_index_at(index);
        check_property(valid_cp_range(name_ref_index, length) && cp->tag_at(name_ref_index).is_utf8(), 
                      "Invalid constant pool index %d in class file %s", 
                       name_ref_index, 
                       CHECK_(nullHandle));
        check_property(valid_cp_range(signature_ref_index, length) && 
                       cp->tag_at(signature_ref_index).is_utf8(), 
                       "Invalid constant pool index %d in class file %s", 
                       signature_ref_index,
                       CHECK_(nullHandle));
        break;
      }
      case JVM_CONSTANT_Utf8 :
        break;
      case JVM_CONSTANT_UnresolvedClass :
        ShouldNotReachHere();     // Only JVM_CONSTANT_ClassIndex should be present
        break;
      case JVM_CONSTANT_ClassIndex :
        {
          int class_index = cp->klass_index_at(index);
          check_property(valid_cp_range(class_index, length) && cp->tag_at(class_index).is_utf8(), 
                         "Invalid constant pool index %d in class file %s", 
                         class_index,
                         CHECK_(nullHandle));
          cp->unresolved_klass_at_put(index, cp->symbol_at(class_index));
        }
        break;
      case JVM_CONSTANT_UnresolvedString :
        ShouldNotReachHere();     // Only JVM_CONSTANT_StringIndex should be present
        break;
      case JVM_CONSTANT_StringIndex :
        {
          int string_index = cp->string_index_at(index);
          check_property(valid_cp_range(string_index, length) && cp->tag_at(string_index).is_utf8(), 
                         "Invalid constant pool index %d in class file %s", 
                         string_index,
                         CHECK_(nullHandle));
          symbolOop sym = cp->symbol_at(string_index);
          cp->unresolved_string_at_put(index, sym);
        }
        break;
      default:
        fatal1("bad constant pool tag value %d", cp->tag_at(index).value());
        ShouldNotReachHere();
        break;
    } // end of switch
  } // end of for

  if (!_need_verify) {
    return cp;
  }

  // second verification pass - checks the strings are of the right format.
  for(index = 1; index < length; index++) {
    jbyte tag = cp->tag_at(index).value();
    switch (tag) {
      case JVM_CONSTANT_UnresolvedClass: {
        verify_legal_class_name(cp->unresolved_klass_at(index), CHECK_(nullHandle));
        break;
      }
      case JVM_CONSTANT_Fieldref:
      case JVM_CONSTANT_Methodref:
      case JVM_CONSTANT_InterfaceMethodref: {
        int name_and_type_ref_index = cp->name_and_type_ref_index_at(index);
        // already verified to be utf8
        int name_ref_index = cp->name_ref_index_at(name_and_type_ref_index);  
        // already verified to be utf8
        int signature_ref_index = cp->signature_ref_index_at(name_and_type_ref_index); 
        symbolHandle name(THREAD, cp->symbol_at(name_ref_index));
        symbolHandle signature(THREAD, cp->symbol_at(signature_ref_index));
        if (tag == JVM_CONSTANT_Fieldref) {
          verify_legal_field_name(name, CHECK_(nullHandle));
          verify_legal_field_signature(name, signature, CHECK_(nullHandle));
        } else {
          verify_legal_method_name(name, CHECK_(nullHandle));
          verify_legal_method_signature(name, signature, CHECK_(nullHandle));
          if (tag == JVM_CONSTANT_Methodref) {
            // 4509014: If a class method name begins with '<', it must be "<init>".
            assert(!name.is_null(), "method name in constant pool is null");
            unsigned int name_len = name->utf8_length();
            assert(name_len > 0, "bad method name");  // already verified as legal name
            if (name->byte_at(0) == '<') {
              if (name() != vmSymbols::object_initializer_name()) {
                classfile_parse_error("Bad method name at constant pool index %d in class file %s", 
                                      name_ref_index,
                                      CHECK_(nullHandle));
              }
            }
          }
        }
        break;
      }                                                  
    }  // end of switch
  }  // end of for
  
  return cp;
}


class NameSigHash: public CHeapObj {
 public:
  symbolHandle  _name;       // name
  symbolHandle  _sig;        // signature
  NameSigHash*  _next;       // Next entry in hash table
};


#define HASH_ROW_SIZE 256

unsigned int hash(const char *name, const char *sig) {
  unsigned int raw_hash = 0;
  while (*name != '\0') {
    raw_hash = *name++ + raw_hash * 37;
  }
  if (sig != NULL) {
    while (*sig != '\0') {
      raw_hash = *sig++ + raw_hash * 37;
    }
  }

  return raw_hash % HASH_ROW_SIZE;
}


void initialize_hashtable(NameSigHash** table) {
  for (int i = 0; i < HASH_ROW_SIZE; i++) {
    table[i] = NULL;
  }
}

void clear_hashtable(NameSigHash** table) {
  for (int i = 0; i < HASH_ROW_SIZE; i++) {
    NameSigHash* current = table[i];
    NameSigHash* next;
    while (current != NULL) {
      next = current->_next;
      current->_next = NULL;
      delete current;
      current = next;
    }
  }
}


// Return false if the name/sig combination is found in table.
// Return true if no duplicate is found. And name/sig is added as a new entry in table.
// The old format checker uses heap sort to find duplicates.
bool put_after_lookup(symbolHandle name, symbolHandle sig, NameSigHash** table) {
  assert(!name.is_null(), "name in constant pool is NULL");

  ResourceMark rm; // needed here for name->as_utf8()
  const char* name_str = name->as_utf8();
  int name_len = name->utf8_length();
  const char* sig_str = sig.is_null() ? NULL : sig->as_utf8();
  int sig_len = (sig_str == NULL) ? 0 : sig->utf8_length();

  // First lookup for duplicates
  int index = hash(name_str, sig_str);
  NameSigHash* entry = table[index];
  while (entry != NULL) {
    if (entry->_name->equals(name_str, name_len)) {
      if (entry->_sig.is_null()) {
        if (sig.is_null()) { return false; }
      } else if(entry->_sig->equals(sig_str, sig_len)) {
        return false;
      }
    }
    entry = entry->_next;
  }

  // No duplicate is found, allocate a new entry and fill it.
  if ((entry = new NameSigHash()) == NULL) {
    return false;
  }
  entry->_name = name;
  entry->_sig = sig;
 
  // Insert into hash table
  entry->_next = table[index];
  table[index] = entry;

  return true;
}


objArrayHandle ClassFileParser::parse_interfaces(constantPoolHandle cp, 
                                                 Handle class_loader, 
                                                 Handle protection_domain, TRAPS) {  
  objArrayHandle nullHandle;
  u2 length = stream()->get_u2(CHECK_(nullHandle));
  if (length == 0) {
    return objArrayHandle(THREAD, Universe::the_empty_system_obj_array());
  } else {        
    objArrayOop interface_oop = oopFactory::new_system_objArray(length, CHECK_(nullHandle));
    objArrayHandle interfaces (THREAD, interface_oop);

    int index;
    for (index = 0; index < length; index++) {
      u2 interface_index = stream()->get_u2(CHECK_(nullHandle));
      check_property(valid_cp_range(interface_index, cp->length()) && 
                     cp->tag_at(interface_index).is_unresolved_klass(), 
                     "Interface name has bad constant pool index %d in class file %s", 
                     interface_index, 
                     CHECK_(nullHandle));
      symbolHandle unresolved_klass (THREAD, cp->klass_name_at(interface_index));

      // Don't need to check legal name because it's checked when parsing constant pool.
      // But need to make sure it's not an array type.
      guarantee_property(unresolved_klass->byte_at(0) != JVM_SIGNATURE_ARRAY, 
                         "Bad interface name in class file %s", CHECK_(nullHandle));

      klassOop k = SystemDictionary::resolve_or_fail(unresolved_klass,
                    class_loader, protection_domain, true, CHECK_(nullHandle));
      KlassHandle interf (THREAD, k);
      if (!Klass::cast(interf())->is_interface()) {
        THROW_MSG_(vmSymbols::java_lang_IncompatibleClassChangeError(), "Implementing class", nullHandle);
      }
      interfaces->obj_at_put(index, interf());
    }

    if (!_need_verify) {
      return interfaces;
    }

    // Check if there's any duplicates in interfaces
    ResourceMark rm(THREAD);
    NameSigHash** interface_names = NEW_RESOURCE_ARRAY(NameSigHash*, HASH_ROW_SIZE);
    initialize_hashtable(interface_names);
    for (index = 0; index < length; index++) {
      klassOop k = (klassOop)interfaces->obj_at(index);
      symbolHandle k_name(THREAD, instanceKlass::cast(k)->name());
      symbolHandle k_sig;   // is null
      // If no duplicates, add name/sig in hashtable interface_names.
      if(put_after_lookup(k_name, k_sig, interface_names) == false) {
        clear_hashtable(interface_names);
        classfile_parse_error("Repetitive interface name in class file %s", CHECK_(nullHandle));
      }
    }
    clear_hashtable(interface_names);

    return interfaces;
  }
}


void ClassFileParser::verify_constantvalue(int constantvalue_index, int signature_index, constantPoolHandle cp, TRAPS) {
  // Make sure the constant pool entry is of a type appropriate to this field
  guarantee_property((constantvalue_index > 0 && constantvalue_index < cp->length()), 
                     "Bad initial value index %d in ConstantValue attribute in class file %s", 
                     constantvalue_index,
                     CHECK); 
  constantTag value_type = cp->tag_at(constantvalue_index);
  switch ( cp->basic_type_for_signature_at(signature_index) ) {
    case T_LONG:
      guarantee_property(value_type.is_long(), "Inconsistent constant value type in class file %s", CHECK);
      break;
    case T_FLOAT:
      guarantee_property(value_type.is_float(), "Inconsistent constant value type in class file %s", CHECK);
      break;
    case T_DOUBLE:
      guarantee_property(value_type.is_double(), "Inconsistent constant value type in class file %s", CHECK);
      break;
    case T_BYTE: case T_CHAR: case T_SHORT: case T_BOOLEAN: case T_INT:
      guarantee_property(value_type.is_int(), "Inconsistent constant value type in class file %s", CHECK);
      break;
    case T_OBJECT: 
      guarantee_property((cp->symbol_at(signature_index)->equals("Ljava/lang/String;", 18) 
                         && (value_type.is_string() || value_type.is_unresolved_string())),
                         "Bad string initial value in class file %s", CHECK);
      break;
    default:
      classfile_parse_error("Unable to set initial value %d in class file %s", constantvalue_index, CHECK);
  }
}


// Parse attributes for a field.
void ClassFileParser::parse_field_attributes(constantPoolHandle cp, bool is_static, u2 signature_index,
                                             u2* constantvalue_index_addr, bool* is_synthetic_addr, u2* generic_signature_index_addr,
                                             typeArrayHandle* field_annotations, TRAPS) {
  u2 attributes_count = stream()->get_u2(CHECK);
  u2 constantvalue_index = 0;
  u2 generic_signature_index = 0;
  bool is_synthetic = false;
  u1* runtime_visible_annotations = NULL;
  int runtime_visible_annotations_length = 0;
  u1* runtime_invisible_annotations = NULL;
  int runtime_invisible_annotations_length = 0;
  while (attributes_count--) {
    u2 attribute_name_index = stream()->get_u2(CHECK);
    u4 attribute_length = stream()->get_u4(CHECK);
    check_property(valid_cp_range(attribute_name_index, cp->length()) &&
                   cp->tag_at(attribute_name_index).is_utf8(), 
                   "Invalid field attribute index %d in class file %s", 
                   attribute_name_index,
                   CHECK);
    symbolOop attribute_name = cp->symbol_at(attribute_name_index);
    if (is_static && attribute_name == vmSymbols::tag_constant_value()) { 
      // ignore if non-static   
      if (constantvalue_index != 0) {
        classfile_parse_error("Repetitive ConstantValue attribute in class file %s", CHECK);
      }
      check_property(attribute_length == 2, 
                     "Invalid ConstantValue field attribute length %d in class file %s", 
                     attribute_length,
                     CHECK);
      constantvalue_index = stream()->get_u2(CHECK);
      if (_need_verify) { 
        verify_constantvalue(constantvalue_index, signature_index, cp, CHECK); 
      }
    } else if (attribute_name == vmSymbols::tag_synthetic()) {
      if (attribute_length != 0) {
        classfile_parse_error("Invalid Synthetic field attribute length %d in class file %s", attribute_length, CHECK);
      }
      is_synthetic = true;
    } else if (attribute_name == vmSymbols::tag_deprecated()) { // 4276120
      if (attribute_length != 0) {
        classfile_parse_error("Invalid Deprecated field attribute length %d in class file %s", attribute_length, CHECK);
      }
    } else if (_major_version >= JAVA_1_5_VERSION) {
      if (attribute_name == vmSymbols::tag_signature()) {
        if (attribute_length != 2) {
          classfile_parse_error("Wrong size %d for field's Signature attribute in class file %s", attribute_length, CHECK);
        }
        generic_signature_index = stream()->get_u2(CHECK);
      } else if (attribute_name == vmSymbols::tag_runtime_visible_annotations()) {
        runtime_visible_annotations_length = attribute_length;
        runtime_visible_annotations = stream()->get_u1_buffer(runtime_visible_annotations_length);
        if (runtime_visible_annotations == NULL) truncated_class_file_error(CHECK);
        stream()->skip_u1(runtime_visible_annotations_length, CHECK);
      } else if (PreserveAllAnnotations && attribute_name == vmSymbols::tag_runtime_invisible_annotations()) {
        runtime_invisible_annotations_length = attribute_length;
        runtime_invisible_annotations = stream()->get_u1_buffer(runtime_invisible_annotations_length);
        if (runtime_invisible_annotations == NULL) truncated_class_file_error(CHECK);
        stream()->skip_u1(runtime_invisible_annotations_length, CHECK);
      } else {
        stream()->skip_u1(attribute_length, CHECK);  // Skip unknown attributes
      }
    } else {
      stream()->skip_u1(attribute_length, CHECK);  // Skip unknown attributes			
    }
  }

  *constantvalue_index_addr = constantvalue_index;
  *is_synthetic_addr = is_synthetic;
  *generic_signature_index_addr = generic_signature_index;
  *field_annotations = assemble_annotations(runtime_visible_annotations,
                                            runtime_visible_annotations_length,
                                            runtime_invisible_annotations,
                                            runtime_invisible_annotations_length,
                                            CHECK);
  return;
}
  

// Field allocation types. Used for computing field offsets.

enum FieldAllocationType {
  STATIC_OOP,		// Oops
  STATIC_BYTE,		// Boolean, Byte, char
  STATIC_SHORT,		// shorts
  STATIC_WORD,		// ints
  STATIC_DOUBLE,	// long or double
  STATIC_ALIGNED_DOUBLE,// aligned long or double
  NONSTATIC_OOP,	 
  NONSTATIC_BYTE,
  NONSTATIC_SHORT,
  NONSTATIC_WORD,
  NONSTATIC_DOUBLE,
  NONSTATIC_ALIGNED_DOUBLE
};


struct FieldAllocationCount {
  int static_oop_count;
  int static_byte_count;
  int static_short_count;
  int static_word_count;
  int static_double_count;
  int nonstatic_oop_count;
  int nonstatic_byte_count;
  int nonstatic_short_count;
  int nonstatic_word_count;
  int nonstatic_double_count;
};

typeArrayHandle ClassFileParser::parse_fields(constantPoolHandle cp, bool is_interface, 
                                              struct FieldAllocationCount *fac,
                                              objArrayHandle* fields_annotations, TRAPS) {
  typeArrayHandle nullHandle;
  u2 length = stream()->get_u2(CHECK_(nullHandle));   
  // Tuples of shorts [access, name index, sig index, initial value index, byte offset, generic signature index]
  typeArrayOop new_fields = oopFactory::new_permanent_shortArray(length*instanceKlass::next_offset, CHECK_(nullHandle));
  typeArrayHandle fields(THREAD, new_fields);
 
  // Used to check if there's any duplicate fields.
  ResourceMark rm(THREAD);
  NameSigHash** names_and_sigs = NEW_RESOURCE_ARRAY(NameSigHash*, HASH_ROW_SIZE);
  initialize_hashtable(names_and_sigs);

  int index = 0;
  typeArrayHandle field_annotations;
  for (int n = 0; n < length; n++) {
    AccessFlags access_flags;
    jint flags = stream()->get_u2(CHECK_(nullHandle)) & JVM_RECOGNIZED_FIELD_MODIFIERS;
    verify_legal_field_modifiers(flags, is_interface, CHECK_(nullHandle));
    access_flags.set_flags(flags);

    u2 name_index = stream()->get_u2(CHECK_(nullHandle));
    int cp_size = cp->length();
    check_property(valid_cp_range(name_index, cp_size) && cp->tag_at(name_index).is_utf8(), 
                   "Invalid constant pool index %d for field name in class file %s", 
                   name_index,
                   CHECK_(nullHandle));
    symbolHandle name(THREAD, cp->symbol_at(name_index));
    verify_legal_field_name(name, CHECK_(nullHandle));

    u2 signature_index = stream()->get_u2(CHECK_(nullHandle));
    check_property(valid_cp_range(signature_index, cp_size) && 
                   cp->tag_at(signature_index).is_utf8(), 
                   "Invalid constant pool index %d for field signature in class file %s", 
                   signature_index,
                   CHECK_(nullHandle));
    symbolHandle sig(THREAD, cp->symbol_at(signature_index));
    verify_legal_field_signature(name, sig, CHECK_(nullHandle));

    if (_need_verify) {
      if(put_after_lookup(name, sig, names_and_sigs) == false) {
        clear_hashtable(names_and_sigs);
        classfile_parse_error("Repetitive field name/signature in class file %s", CHECK_(nullHandle));
      }
    }

    u2 constantvalue_index = 0;
    bool is_synthetic = false;
    u2 generic_signature_index = 0;
    bool is_static = access_flags.is_static();
    parse_field_attributes(cp, is_static, signature_index, &constantvalue_index, 
                           &is_synthetic, &generic_signature_index, &field_annotations, CHECK_(nullHandle));
    if (field_annotations.not_null()) {
      if (fields_annotations->is_null()) {
        objArrayOop md = oopFactory::new_system_objArray(length, CHECK_(nullHandle));
        *fields_annotations = objArrayHandle(THREAD, md);
      }
      (*fields_annotations)->obj_at_put(n, field_annotations());
    }
    if (is_synthetic) {
      access_flags.set_is_synthetic();
    }
    
    fields->short_at_put(index++, access_flags.as_short());
    fields->short_at_put(index++, name_index);
    fields->short_at_put(index++, signature_index);
    fields->short_at_put(index++, constantvalue_index);	

    // Remember how many oops we encountered and compute allocation type
    BasicType type = cp->basic_type_for_signature_at(signature_index);
    FieldAllocationType atype;
    if ( is_static ) {
      switch ( type ) {
        case  T_BOOLEAN:
        case  T_BYTE:
          fac->static_byte_count++;
          atype = STATIC_BYTE;
	        break;
        case  T_LONG:
  	    case  T_DOUBLE:
          if (Universe::field_type_should_be_aligned(type)) {
	          atype = STATIC_ALIGNED_DOUBLE;
          }
 	        else {
	          atype = STATIC_DOUBLE;
          }
	        fac->static_double_count++;
	        break;
  	    case  T_CHAR:     
  	    case  T_SHORT: 
          fac->static_short_count++;
          atype = STATIC_SHORT;
	        break;
  	    case  T_FLOAT:
  	    case  T_INT:
	        fac->static_word_count++;
	        atype = STATIC_WORD;
	        break;
        case  T_ARRAY: 
        case  T_OBJECT:
  	      fac->static_oop_count++;
	        atype = STATIC_OOP;
	        break;
  	    case  T_ADDRESS: 
        case  T_VOID:
        default: 
	        assert(0, "bad field type");
	        break;
      }
    } else {
      switch ( type ) {
        case  T_BOOLEAN:
  	    case  T_BYTE:
          fac->nonstatic_byte_count++;
          atype = NONSTATIC_BYTE;
	        break;
        case  T_LONG:
  	    case  T_DOUBLE:
          if (Universe::field_type_should_be_aligned(type)) {
	          atype = NONSTATIC_ALIGNED_DOUBLE;
          } else {
	          atype = NONSTATIC_DOUBLE;
          }
	        fac->nonstatic_double_count++;
	        break;
  	    case  T_CHAR:     
  	    case  T_SHORT: 
          fac->nonstatic_short_count++;
          atype = NONSTATIC_SHORT;
	        break;
  	    case  T_FLOAT:
  	    case  T_INT:
	        fac->nonstatic_word_count++;
	        atype = NONSTATIC_WORD;
	        break;
        case  T_ARRAY: 
        case  T_OBJECT:
	        fac->nonstatic_oop_count++;
	        atype = NONSTATIC_OOP;
	        break;
  	    case  T_ADDRESS: 
        case  T_VOID:
        default: 
	        assert(0, "bad field type");
	        break;
      }
    }

    // The correct offset is computed later (all oop fields will be located together)
    // We temporarily store the allocation type in the offset field
    fields->short_at_put(index++, atype);
    fields->short_at_put(index++, 0);  // Clear out high word of byte offset
    fields->short_at_put(index++, generic_signature_index);
  }
  clear_hashtable(names_and_sigs);

  return fields;
}


static void copy_u2_with_conversion(u2* dest, u2* src, int length) {
  while (length-- > 0) {
    *dest++ = Bytes::get_Java_u2((u1*) (src++));
  }
}


typeArrayHandle ClassFileParser::parse_exception_table(u4 code_length, 
                                                       u4 exception_table_length, 
                                                       constantPoolHandle cp, 
                                                       TRAPS) {
  typeArrayHandle nullHandle;

  // 4-tuples of ints [start_pc, end_pc, handler_pc, catch_type index]
  typeArrayOop eh = oopFactory::new_permanent_intArray(exception_table_length*4, CHECK_(nullHandle));
  typeArrayHandle exception_handlers = typeArrayHandle(THREAD, eh);
  
  int index = 0;
  for (unsigned int i = 0; i < exception_table_length; i++) {
    u2 start_pc = stream()->get_u2(CHECK_(nullHandle));
    u2 end_pc = stream()->get_u2(CHECK_(nullHandle));
    u2 handler_pc = stream()->get_u2(CHECK_(nullHandle));
    u2 catch_type_index = stream()->get_u2(CHECK_(nullHandle));
    // Will check legal target after parsing code array in verifier.
    if (_need_verify) {
      guarantee_property((start_pc < end_pc) && (end_pc <= code_length),
                         "Illegal exception table range in class file %s", CHECK_(nullHandle));
      guarantee_property(handler_pc < code_length,
                         "Illegal exception table handler in class file %s", CHECK_(nullHandle));
      if (catch_type_index != 0) {
        guarantee_property(valid_cp_range(catch_type_index, cp->length()) && 
                          (cp->tag_at(catch_type_index).is_klass() || 
                           cp->tag_at(catch_type_index).is_unresolved_klass()),
                           "Catch type in exception table has bad constant type in class file %s", CHECK_(nullHandle));
      }
    }	      
    exception_handlers->int_at_put(index++, start_pc); 
    exception_handlers->int_at_put(index++, end_pc);  
    exception_handlers->int_at_put(index++, handler_pc);  
    exception_handlers->int_at_put(index++, catch_type_index);  
  }
  return exception_handlers;
}

u_char* ClassFileParser::parse_linenumber_table(u4 code_attribute_length, 
                                                u4 code_length,
                                                int* compressed_linenumber_table_size, 
                                                TRAPS) {
  unsigned int linenumber_table_length = stream()->get_u2(CHECK_0);

  // Verify line number attribute and table length
  if (_need_verify) {
    guarantee_property(code_attribute_length == (sizeof(u2) /* linenumber table length */ +
                                                 linenumber_table_length*(sizeof(u2) /* start_pc */ +
                                                                          sizeof(u2) /* line_number */)),
                       "LineNumberTable attribute has wrong length in class file %s", CHECK_0);
  }          
  
  u_char* compressed_linenumber_table = NULL;
  if (linenumber_table_length > 0) {
    // initial_size large enough
    CompressedLineNumberWriteStream c_stream(linenumber_table_length * sizeof(u2) * 2);  
    while (linenumber_table_length-- > 0) {
      u2 bci  = stream()->get_u2(CHECK_0); // start_pc
      u2 line = stream()->get_u2(CHECK_0); // line_number
      if (_need_verify) {
        guarantee_property(bci < code_length,
                           "Invalid pc in LineNumberTable in class file %s", CHECK_0);
      }
      c_stream.write_pair(bci, line);
    }
    c_stream.write_terminator();
    *compressed_linenumber_table_size = c_stream.position();
    compressed_linenumber_table = c_stream.buffer();
  }
  return compressed_linenumber_table;
}


// Class file LocalVariableTable elements.
class Classfile_LVT_Element VALUE_OBJ_CLASS_SPEC {
 public:
  u2 start_bci;
  u2 length;
  u2 name_cp_index;
  u2 descriptor_cp_index;
  u2 slot;
};


class LVT_Hash: public CHeapObj {
 public:
  LocalVariableTableElement  *_elem;  // element
  LVT_Hash*                   _next;  // Next entry in hash table
};

unsigned int hash(LocalVariableTableElement *elem) {
  unsigned int raw_hash = elem->start_bci;

  raw_hash = elem->length        + raw_hash * 37;
  raw_hash = elem->name_cp_index + raw_hash * 37;
  raw_hash = elem->slot          + raw_hash * 37;

  return raw_hash % HASH_ROW_SIZE;
}

void initialize_hashtable(LVT_Hash** table) {
  for (int i = 0; i < HASH_ROW_SIZE; i++) {
    table[i] = NULL;
  }
}

void clear_hashtable(LVT_Hash** table) {
  for (int i = 0; i < HASH_ROW_SIZE; i++) {
    LVT_Hash* current = table[i];
    LVT_Hash* next;
    while (current != NULL) {
      next = current->_next;
      current->_next = NULL;
      delete(current);
      current = next;
    }
    table[i] = NULL;
  }
}

LVT_Hash* LVT_lookup(LocalVariableTableElement *elem, int index, LVT_Hash** table) {
  LVT_Hash* entry = table[index];

  /*
   * 3-tuple start_bci/length/slot has to be unique key,
   * so the following comparison seems to be redundant:
   *       && elem->name_cp_index == entry->_elem->name_cp_index
   */
  while (entry != NULL) {
    if (elem->start_bci           == entry->_elem->start_bci
     && elem->length              == entry->_elem->length 
     && elem->name_cp_index       == entry->_elem->name_cp_index
     && elem->slot                == entry->_elem->slot
    ) {
      return entry;
    }
    entry = entry->_next;
  }
  return NULL;
}

// Return false if the local variable is found in table.
// Return true if no duplicate is found.
// And local variable is added as a new entry in table.
bool LVT_put_after_lookup(LocalVariableTableElement *elem, LVT_Hash** table) {
  // First lookup for duplicates
  int index = hash(elem);
  LVT_Hash* entry = LVT_lookup(elem, index, table);

  if (entry != NULL) {
      return false;
  }
  // No duplicate is found, allocate a new entry and fill it.
  if ((entry = new LVT_Hash()) == NULL) {
    return false;
  }
  entry->_elem = elem;
 
  // Insert into hash table
  entry->_next = table[index];
  table[index] = entry;

  return true;
}

void copy_lvt_element(Classfile_LVT_Element *src, LocalVariableTableElement *lvt) {
  lvt->start_bci           = Bytes::get_Java_u2((u1*) &src->start_bci);
  lvt->length              = Bytes::get_Java_u2((u1*) &src->length);
  lvt->name_cp_index       = Bytes::get_Java_u2((u1*) &src->name_cp_index);
  lvt->descriptor_cp_index = Bytes::get_Java_u2((u1*) &src->descriptor_cp_index);
  lvt->signature_cp_index  = 0;
  lvt->slot                = Bytes::get_Java_u2((u1*) &src->slot);
}

// Function is used to parse both attributes:
//       LocalVariableTable (LVT) and LocalVariableTypeTable (LVTT)
u2* ClassFileParser::parse_localvariable_table(u4 code_length,
                                               u2 max_locals,
                                               u4 code_attribute_length,
                                               constantPoolHandle cp,
                                               u2* localvariable_table_length,
                                               bool isLVTT,
                                               TRAPS) {
  const char * tbl_name = (isLVTT) ? "LocalVariableTypeTable" : "LocalVariableTable";
  *localvariable_table_length = stream()->get_u2(CHECK_0);
  unsigned int size = (*localvariable_table_length) * sizeof(Classfile_LVT_Element) / sizeof(u2);
  // Verify local variable table attribute has right length
  if (_need_verify) {
    guarantee_property(code_attribute_length == (sizeof(*localvariable_table_length) + size * sizeof(u2)),
                       "%s has wrong length in class file %s", tbl_name, CHECK_0);
  }
  u2* localvariable_table_start = stream()->get_u2_buffer(size);
  if (localvariable_table_start == NULL) { truncated_class_file_error(CHECK_0); }
  if (!_need_verify) { 
    stream()->skip_u2(size, CHECK_0); 
  } else {
    for(int i = 0; i < (*localvariable_table_length); i++) {
      u2 start_pc = stream()->get_u2(CHECK_0);
      u2 length = stream()->get_u2(CHECK_0);
      u2 name_index = stream()->get_u2(CHECK_0);
      u2 descriptor_index = stream()->get_u2(CHECK_0);
      u2 index = stream()->get_u2(CHECK_0);
      // Assign to a u4 to avoid overflow
      u4 end_pc = (u4)start_pc + (u4)length;

      if (start_pc >= code_length) {
        classfile_parse_error("Invalid start_pc %d in %s in class file %s", start_pc, tbl_name, CHECK_0);
      }
      if (end_pc > code_length) {
        classfile_parse_error("Invalid length %d in %s in class file %s", length, tbl_name, CHECK_0);
      }
      int cp_size = cp->length();
      guarantee_property(valid_cp_range(name_index, cp_size) && cp->tag_at(name_index).is_utf8(),
                         "Name index %d in %s has bad constant type in class file %s",
                          name_index, tbl_name, CHECK_0);
      guarantee_property(valid_cp_range(descriptor_index, cp_size) &&
                         cp->tag_at(descriptor_index).is_utf8(),
                         "Signature index %d in %s has bad constant type in class file %s",
                         descriptor_index, tbl_name, CHECK_0);

      symbolHandle name(THREAD, cp->symbol_at(name_index));
      symbolHandle sig(THREAD, cp->symbol_at(descriptor_index));
      verify_legal_field_name(name, CHECK_0);
      u2 extra_slot = 0;
      if (!isLVTT) {
        verify_legal_field_signature(name, sig, CHECK_0);

        // 4894874: check special cases for double and long local variables
        if (sig() == vmSymbols::type_signature(T_DOUBLE) || 
            sig() == vmSymbols::type_signature(T_LONG)) {
          extra_slot = 1;
        }
      }
      guarantee_property((index + extra_slot) < max_locals,
                          "Invalid index %d in %s in class file %s",
                          index, tbl_name, CHECK_0);
    }
  }
  return localvariable_table_start;
}


void ClassFileParser::parse_type_array(u2 array_length, u4 code_length, u4* u1_index, u4* u2_index,
                                      u1* u1_array, u2* u2_array, constantPoolHandle cp, TRAPS) {
  u2 index = 0; // index in the array with long/double occupying two slots
  u4 i1 = *u1_index;
  u4 i2 = *u2_index + 1;  
  for(int i = 0; i < array_length; i++) {
    u1 tag = u1_array[i1++] = stream()->get_u1(CHECK);
    index++;
    if (tag == ITEM_Long || tag == ITEM_Double) {
      index++; 
    } else if (tag == ITEM_Object) {
      u2 class_index = u2_array[i2++] = stream()->get_u2(CHECK);
      guarantee_property(valid_cp_range(class_index, cp->length()) &&
                         cp->tag_at(class_index).is_unresolved_klass(), 
                         "Bad class index %d in StackMap in class file %s", 
                         class_index, CHECK);
    } else if (tag == ITEM_Uninitialized) {
      u2 offset = u2_array[i2++] = stream()->get_u2(CHECK);
      guarantee_property(offset < code_length, 
                         "Bad uninitialized type offset %d in StackMap in class file %s", 
                         offset, CHECK);
    } else {
      guarantee_property(tag <= ITEM_Uninitialized,
                         "Unknown variable type %d in StackMap in class file %s", 
                         tag, CHECK);
    }
  }
  u2_array[*u2_index] = index; 
  *u1_index = i1;
  *u2_index = i2;
}

void ClassFileParser::parse_stackmap_table(u4 code_attribute_length,
                                           u4 code_length,
                                           u2 max_locals,
                                           u2 max_stack,
                                           typeArrayHandle* stackmap_u1,
                                           typeArrayHandle* stackmap_u2,
                                           constantPoolHandle cp,
                                           TRAPS) {
  // Use code_attribute_length as stackmap_size first, verify it later
  u1* stackmap_table_start = stream()->get_u1_buffer(code_attribute_length);
  if (stackmap_table_start == NULL) { truncated_class_file_error(CHECK); }

  // If verifier is off, skip the stackmap table
  if (!_need_verify) {
    stream()->skip_u1(code_attribute_length, CHECK);
    return;
  }

  // Use temporary arrays to hold table contents first: table size is unknown yet
  ResourceMark rm(THREAD);
  u1* u1_array = NEW_RESOURCE_ARRAY(u1, code_attribute_length);
  u2* u2_array = NEW_RESOURCE_ARRAY(u2, code_attribute_length);
  u4 u1_index = 0; 
  u4 u2_index = 0;

  // Get frame count
  u2 frame_count = u2_array[u2_index++] = stream()->get_u2(CHECK); 

  // Count stack map size
  u2 offset, locals_size, stack_size;
  for (u2 i = 0; i < frame_count; i++) {
    // offset
    offset = u2_array[u2_index++] = stream()->get_u2(CHECK);   
    guarantee_property(offset < code_length, 
                       "StackMap offset overflows in class file %s", CHECK);

    // locals_size
    locals_size = stream()->get_u2(CHECK);
    guarantee_property(locals_size <= max_locals, 
                       "StackMap has wrong locals size %d in class file %s", 
                       locals_size, CHECK);

    // local variable type array
    parse_type_array(locals_size, code_length, &u1_index, &u2_index, 
                     u1_array, u2_array, cp, CHECK);

    // stack_size
    stack_size = stream()->get_u2(CHECK);
    guarantee_property(stack_size <= max_stack, 
                       "StackMap has wrong stack size %d in class file %s", 
                       stack_size, CHECK);
         
    // stack type array 
    parse_type_array(stack_size, code_length, &u1_index, &u2_index, 
                     u1_array, u2_array, cp, CHECK);	
  }  // end for
  guarantee_property(code_attribute_length == u1_index * sizeof(u1) + u2_index * sizeof(u2),
                     "StackMap table has wrong length in class file %s", CHECK);

  // Create stackmap_u1 and stackmap_u2
  typeArrayOop sm_u1 = oopFactory::new_permanent_byteArray(u1_index, CHECK);
  *stackmap_u1 = typeArrayHandle(THREAD, sm_u1);
  for (u4 i1 = 0; i1 < u1_index; i1++) {
    (*stackmap_u1)->byte_at_put(i1, u1_array[i1]); 
  }
  typeArrayOop sm_u2 = oopFactory::new_permanent_shortArray(u2_index, CHECK);
  *stackmap_u2 = typeArrayHandle(THREAD, sm_u2);
  for (u4 i2 = 0; i2 < u2_index; i2++) {
    (*stackmap_u2)->short_at_put(i2, u2_array[i2]); 
  }
}


u2* ClassFileParser::parse_checked_exceptions(u2* checked_exceptions_length, 
                                              u4 method_attribute_length,
                                              constantPoolHandle cp, TRAPS) {
  *checked_exceptions_length = stream()->get_u2(CHECK_0);
  unsigned int size = (*checked_exceptions_length) * sizeof(CheckedExceptionElement) / sizeof(u2);
  u2* checked_exceptions_start = stream()->get_u2_buffer(size);
  if (checked_exceptions_start == NULL) truncated_class_file_error(CHECK_0);
  if (!_need_verify) { 
    stream()->skip_u2(size, CHECK_0); 
  } else {
    // Verify each value in the checked exception table
    u2 checked_exception;
    for (int i = 0; i < (*checked_exceptions_length); i++) {
      checked_exception = stream()->get_u2(CHECK_0);
      check_property(valid_cp_range(checked_exception, cp->length()) &&
                     cp->tag_at(checked_exception).is_klass_reference(), 
                     "Exception name has bad type at constant pool %d in class file %s", 
                     checked_exception,
                     CHECK_0);
    }
  }
  // check exceptions attribute length
  if (_need_verify) {
    guarantee_property(method_attribute_length == (sizeof(*checked_exceptions_length) +
                                                   sizeof(u2) * size),
                      "Exceptions attribute has wrong length in class file %s", CHECK_0);
  }
  return checked_exceptions_start;
}


#define MAX_ARGS_SIZE 255
#define MAX_CODE_SIZE 65535
#define INITIAL_MAX_LVT_NUMBER 256

// Note: the parse_method below is big and clunky because all parsing of the code and exceptions
// attribute is inlined. This is curbersome to avoid since we inline most of the parts in the
// methodOop to save footprint, so we only know the size of the resulting methodOop when the
// entire method attribute is parsed.
//
// The promoted_flags parameter is used to pass relevant access_flags
// from the method back up to the containing klass. These flag values
// are added to klass's access_flags.

methodHandle ClassFileParser::parse_method(constantPoolHandle cp, bool is_interface,
                                           AccessFlags *promoted_flags,
                                           typeArrayHandle* method_annotations,
                                           typeArrayHandle* method_parameter_annotations,
                                           typeArrayHandle* method_default_annotations,
                                           TRAPS) {
  methodHandle nullHandle;
  ResourceMark rm(THREAD);
  // Parse fixed parts
  int flags = stream()->get_u2(CHECK_(nullHandle));    
  u2 name_index = stream()->get_u2(CHECK_(nullHandle));
  int cp_size = cp->length();
  check_property(valid_cp_range(name_index, cp_size) && cp->tag_at(name_index).is_utf8(), 
                 "Illegal constant pool index %d for method name in class file %s", 
                 name_index, 
                 CHECK_(nullHandle));
  symbolHandle name(THREAD, cp->symbol_at(name_index));
  verify_legal_method_name(name, CHECK_(nullHandle));  

  u2 signature_index = stream()->get_u2(CHECK_(nullHandle));
  check_property(valid_cp_range(signature_index, cp_size) &&
                 cp->tag_at(signature_index).is_utf8(), 
                 "Illegal constant pool index %d for method signature in class file %s", 
                 signature_index,
                 CHECK_(nullHandle));
  symbolHandle signature(THREAD, cp->symbol_at(signature_index));

  bool in_clinit = false;
  AccessFlags access_flags;  
  if (name == vmSymbols::class_initializer_name()) {
    // We ignore the access flags for a class initializer. (JVM Spec. p. 116)
    flags = JVM_ACC_STATIC;
    in_clinit = true;
  } else {
    verify_legal_method_modifiers(flags, is_interface, name, CHECK_(nullHandle));
  }

  int args_size = 0;  // only used when _need_verify is true
  if (_need_verify) {
    args_size = ((flags & JVM_ACC_STATIC) ? 0 : 1) + 
                    verify_legal_method_signature(name, signature, CHECK_(nullHandle));
    if (args_size > MAX_ARGS_SIZE) {
      classfile_parse_error("Too many arguments in method signature in class file %s", CHECK_(nullHandle));
    }
  }
        
  access_flags.set_flags(flags & JVM_RECOGNIZED_METHOD_MODIFIERS);
  
  // Default values for code and exceptions attribute elements
  u2 max_stack = 0;
  u2 max_locals = 0;
  u4 code_length = 0;
  u1* code_start = 0;
  u2 exception_table_length = 0;
  typeArrayHandle exception_handlers(THREAD, Universe::the_empty_int_array());
  u2 checked_exceptions_length = 0;
  u2* checked_exceptions_start = NULL;
  int compressed_linenumber_table_size = 0;
  u_char* compressed_linenumber_table = NULL;
  int total_lvt_length = 0;
  u2 lvt_cnt = 0;
  u2 lvtt_cnt = 0;
  u2 max_lvt_cnt = INITIAL_MAX_LVT_NUMBER;
  u2 max_lvtt_cnt = INITIAL_MAX_LVT_NUMBER;
  u2* localvariable_table_length = NEW_RESOURCE_ARRAY(u2,  INITIAL_MAX_LVT_NUMBER);
  u2** localvariable_table_start = NEW_RESOURCE_ARRAY(u2*, INITIAL_MAX_LVT_NUMBER);
  u2* localvariable_type_table_length = NEW_RESOURCE_ARRAY(u2,  INITIAL_MAX_LVT_NUMBER);
  u2** localvariable_type_table_start = NEW_RESOURCE_ARRAY(u2*, INITIAL_MAX_LVT_NUMBER);
  bool parsed_code_attribute = false;
  bool parsed_checked_exceptions_attribute = false;
  // stackmap attribute - JDK1.5
  typeArrayHandle stackmap_u1(THREAD, Universe::the_empty_byte_array()); 
  typeArrayHandle stackmap_u2(THREAD, Universe::the_empty_short_array()); 
  u2 generic_signature_index = 0;
  u1* runtime_visible_annotations = NULL;
  int runtime_visible_annotations_length = 0;
  u1* runtime_invisible_annotations = NULL;
  int runtime_invisible_annotations_length = 0;
  u1* runtime_visible_parameter_annotations = NULL;
  int runtime_visible_parameter_annotations_length = 0;
  u1* runtime_invisible_parameter_annotations = NULL;
  int runtime_invisible_parameter_annotations_length = 0;
  u1* annotation_default = NULL;
  int annotation_default_length = 0;

  // Parse code and exceptions attribute
  u2 method_attributes_count = stream()->get_u2(CHECK_(nullHandle));
  while (method_attributes_count--) {   
    u2 method_attribute_name_index = stream()->get_u2(CHECK_(nullHandle));
    u4 method_attribute_length = stream()->get_u4(CHECK_(nullHandle));
    check_property(valid_cp_range(method_attribute_name_index, cp_size) &&
                   cp->tag_at(method_attribute_name_index).is_utf8(), 
                  "Invalid method attribute name index %d in class file %s", 
                   method_attribute_name_index,
                   CHECK_(nullHandle));

    symbolOop method_attribute_name = cp->symbol_at(method_attribute_name_index);
    if (method_attribute_name == vmSymbols::tag_code()) {
      // Parse Code attribute
      if (_need_verify) {
        guarantee_property(!access_flags.is_native() && !access_flags.is_abstract(), 
                        "Code attribute in native or abstract methods in class file %s", 
                         CHECK_(nullHandle));
      }
      if (parsed_code_attribute) {
        classfile_parse_error("Multiple Code attributes in class file %s", CHECK_(nullHandle));
      }
      parsed_code_attribute = true;

      // Stack size, locals size, and code size
      if (_major_version == 45 && _minor_version <= 2) {
        max_stack = stream()->get_u1(CHECK_(nullHandle));
        max_locals = stream()->get_u1(CHECK_(nullHandle));
        code_length = stream()->get_u2(CHECK_(nullHandle));
      } else {
        max_stack = stream()->get_u2(CHECK_(nullHandle));
        max_locals = stream()->get_u2(CHECK_(nullHandle));
        code_length = stream()->get_u4(CHECK_(nullHandle));
      }
      if (_need_verify) {
        guarantee_property(args_size <= max_locals, 
                           "Arguments can't fit into locals in class file %s", CHECK_(nullHandle));
        guarantee_property(code_length > 0 && code_length <= MAX_CODE_SIZE, 
                           "Invalid method Code length %d in class file %s", 
                           code_length, CHECK_(nullHandle));
      }
      // Code pointer
      code_start = stream()->get_u1_buffer(code_length);
      if (code_start == NULL) truncated_class_file_error(CHECK_(nullHandle));
      stream()->skip_u1(code_length, CHECK_(nullHandle));

      // Exception handler table
      exception_table_length = stream()->get_u2(CHECK_(nullHandle));
      if (exception_table_length > 0) {
        exception_handlers = 
              parse_exception_table(code_length, exception_table_length, cp, CHECK_(nullHandle));
      }

     // Parse additional attributes in code attribute
      u2 code_attributes_count = stream()->get_u2(CHECK_(nullHandle));
      unsigned int calculated_attribute_length = sizeof(max_stack) + 
                                                 sizeof(max_locals) + 
                                                 sizeof(code_length) +
                                                 code_length + 
                                                 sizeof(exception_table_length) +
                                                 sizeof(code_attributes_count) +
                                                 exception_table_length*(sizeof(u2) /* start_pc */+
                                                                         sizeof(u2) /* end_pc */  +
                                                                         sizeof(u2) /* handler_pc */ +
                                                                         sizeof(u2) /* catch_type_index */);

      while (code_attributes_count--) {
        u2 code_attribute_name_index = stream()->get_u2(CHECK_(nullHandle));
        u4 code_attribute_length = stream()->get_u4(CHECK_(nullHandle));
        calculated_attribute_length += code_attribute_length + 
                                       sizeof(code_attribute_name_index) +
                                       sizeof(code_attribute_length);
        check_property(valid_cp_range(code_attribute_name_index, cp_size) &&
                       cp->tag_at(code_attribute_name_index).is_utf8(), 
                       "Invalid code attribute name index %d in class file %s", 
                       code_attribute_name_index,
                       CHECK_(nullHandle));
        if (LoadLineNumberTables && 
            cp->symbol_at(code_attribute_name_index) == vmSymbols::tag_line_number_table()) {
          // Parse and compress line number table
          compressed_linenumber_table = parse_linenumber_table(code_attribute_length, 
                                                               code_length,
                                                               &compressed_linenumber_table_size, 
                                                               CHECK_(nullHandle));
                                         
        } else if (LoadLocalVariableTables && 
                   cp->symbol_at(code_attribute_name_index) == vmSymbols::tag_local_variable_table()) {
          // Parse local variable table
          if (lvt_cnt == max_lvt_cnt) {
            max_lvt_cnt <<= 1;
            REALLOC_RESOURCE_ARRAY(u2, localvariable_table_length, lvt_cnt, max_lvt_cnt);
            REALLOC_RESOURCE_ARRAY(u2*, localvariable_table_start, lvt_cnt, max_lvt_cnt);
          }
          localvariable_table_start[lvt_cnt] = parse_localvariable_table(code_length,
                                                                max_locals,
                                                                code_attribute_length,
                                                                cp,
                                                                &localvariable_table_length[lvt_cnt],
                                                                false,	// is not LVTT
                                                                CHECK_(nullHandle));
          total_lvt_length += localvariable_table_length[lvt_cnt];
          lvt_cnt++;
        } else if (LoadLocalVariableTypeTables && 
                   _major_version >= JAVA_1_5_VERSION &&
                   cp->symbol_at(code_attribute_name_index) == vmSymbols::tag_local_variable_type_table()) {
          // Parse local variable type table
          if (lvtt_cnt == max_lvtt_cnt) {
            max_lvtt_cnt <<= 1;
            REALLOC_RESOURCE_ARRAY(u2, localvariable_type_table_length, lvtt_cnt, max_lvtt_cnt);
            REALLOC_RESOURCE_ARRAY(u2*, localvariable_type_table_start, lvtt_cnt, max_lvtt_cnt);
          }
          localvariable_type_table_start[lvtt_cnt] = parse_localvariable_table(code_length,
                                                                max_locals,
                                                                code_attribute_length,
                                                                cp,
                                                                &localvariable_type_table_length[lvtt_cnt],
                                                                true,	// is LVTT
                                                                CHECK_(nullHandle));
          lvtt_cnt++;
        } else if (UseSplitVerifier &&
                   _major_version >= Verifier::STACKMAP_ATTRIBUTE_MAJOR_VERSION &&
                   cp->symbol_at(code_attribute_name_index) == vmSymbols::tag_stack_map()) {
          // Stack map is only needed by the new verifier in JDK1.5.
          parse_stackmap_table(code_attribute_length, code_length, max_locals, max_stack,
                               &stackmap_u1, &stackmap_u2, cp, CHECK_(nullHandle));
        } else {
          // Skip unknown attributes
          stream()->skip_u1(code_attribute_length, CHECK_(nullHandle));
        }
      }
      // check method attribute length
      if (_need_verify) {
        guarantee_property(method_attribute_length == calculated_attribute_length,
                           "Code segment has wrong length in class file %s", CHECK_(nullHandle));
      }
    } else if (method_attribute_name == vmSymbols::tag_exceptions()) {
      // Parse Exceptions attribute
      if (parsed_checked_exceptions_attribute) {
        classfile_parse_error("Multiple Exceptions attributes in class file %s", CHECK_(nullHandle));
      }
      parsed_checked_exceptions_attribute = true;
      checked_exceptions_start =
            parse_checked_exceptions(&checked_exceptions_length, 
                                     method_attribute_length, 
                                     cp, CHECK_(nullHandle));
    } else if (method_attribute_name == vmSymbols::tag_synthetic()) {
      if (method_attribute_length != 0) {
        classfile_parse_error("Invalid Synthetic method attribute length %d in class file %s", 
                              method_attribute_length, CHECK_(nullHandle));
      }
      // Should we check that there hasn't already been a synthetic attribute?
      access_flags.set_is_synthetic();
    } else if (method_attribute_name == vmSymbols::tag_deprecated()) { // 4276120
      if (method_attribute_length != 0) {
        classfile_parse_error("Invalid Deprecated method attribute length %d in class file %s", 
                              method_attribute_length, CHECK_(nullHandle));
      }
    } else if (_major_version >= JAVA_1_5_VERSION) {
      if (method_attribute_name == vmSymbols::tag_signature()) {
        if (method_attribute_length != 2) {
          classfile_parse_error("Invalid Signature attribute length %d in class file %s", 
                                method_attribute_length, CHECK_(nullHandle));
        }
        generic_signature_index = stream()->get_u2(CHECK_(nullHandle));
      } else if (method_attribute_name == vmSymbols::tag_runtime_visible_annotations()) {
        runtime_visible_annotations_length = method_attribute_length;
        runtime_visible_annotations = stream()->get_u1_buffer(runtime_visible_annotations_length);
        if (runtime_visible_annotations == NULL) truncated_class_file_error(CHECK_(nullHandle));
        stream()->skip_u1(runtime_visible_annotations_length, CHECK_(nullHandle));
      } else if (PreserveAllAnnotations && method_attribute_name == vmSymbols::tag_runtime_invisible_annotations()) {
        runtime_invisible_annotations_length = method_attribute_length;
        runtime_invisible_annotations = stream()->get_u1_buffer(runtime_invisible_annotations_length);
        if (runtime_invisible_annotations == NULL) truncated_class_file_error(CHECK_(nullHandle));
        stream()->skip_u1(runtime_invisible_annotations_length, CHECK_(nullHandle));
      } else if (method_attribute_name == vmSymbols::tag_runtime_visible_parameter_annotations()) {
        runtime_visible_parameter_annotations_length = method_attribute_length;
        runtime_visible_parameter_annotations = stream()->get_u1_buffer(runtime_visible_parameter_annotations_length);
        if (runtime_visible_parameter_annotations == NULL) truncated_class_file_error(CHECK_(nullHandle));
        stream()->skip_u1(runtime_visible_parameter_annotations_length, CHECK_(nullHandle));
      } else if (PreserveAllAnnotations && method_attribute_name == vmSymbols::tag_runtime_invisible_parameter_annotations()) {
        runtime_invisible_parameter_annotations_length = method_attribute_length;
        runtime_invisible_parameter_annotations = stream()->get_u1_buffer(runtime_invisible_parameter_annotations_length);
        if (runtime_invisible_parameter_annotations == NULL) truncated_class_file_error(CHECK_(nullHandle));
        stream()->skip_u1(runtime_invisible_parameter_annotations_length, CHECK_(nullHandle));
      } else if (method_attribute_name == vmSymbols::tag_annotation_default()) {
        annotation_default_length = method_attribute_length;
        annotation_default = stream()->get_u1_buffer(annotation_default_length);
        if (annotation_default == NULL) truncated_class_file_error(CHECK_(nullHandle));
        stream()->skip_u1(annotation_default_length, CHECK_(nullHandle));
      } else {
        // Skip unknown attributes
        stream()->skip_u1(method_attribute_length, CHECK_(nullHandle));
      }
    } else {
      // Skip unknown attributes
      stream()->skip_u1(method_attribute_length, CHECK_(nullHandle));
    }      
  }
  // Make sure there's at least one Code attribute in non-native/non-abstract method
  if (_need_verify) {
    guarantee_property(in_clinit || access_flags.is_native() || access_flags.is_abstract() || parsed_code_attribute,
                      "Absent Code attribute in method that is not native or abstract in class file %s", CHECK_(nullHandle));
  }

  // All sizing information for a methodOop is finally available, now create it
  methodOop m_oop  = oopFactory::new_method(code_length, access_flags,
                               compressed_linenumber_table_size, 
                               total_lvt_length, 
                               checked_exceptions_length, 
                               CHECK_(nullHandle));
  methodHandle m (THREAD, m_oop);

  ClassLoadingService::add_class_method_size(m_oop->size()*HeapWordSize);

  // Fill in information from fixed part (access_flags already set)
  m->set_constants(cp());
  m->set_name_index(name_index);
  m->set_signature_index(signature_index);
  m->set_generic_signature_index(generic_signature_index);
#ifdef CC_INTERP
  // hmm is there a gc issue here??
  ResultTypeFinder rtf(cp->symbol_at(signature_index));
  m->set_result_index(rtf.type());
#endif
  m->compute_size_of_parameters(THREAD);
  // Fill in code attribute information
  m->set_max_stack(max_stack);
  m->set_max_locals(max_locals);
  m->set_exception_table(exception_handlers());
  m->set_stackmap_u1(stackmap_u1());

  /**
   * In non-product mode, the stackmap_u2 field is the flag used to indicate
   * that the methodOop and it's associated constMethodOop are partially 
   * initialized and thus are exempt from pre/post GC verification.  Once 
   * the field is set, the oops are considered fully initialized so make 
   * sure that the oops can pass verification when this field is set. 
   */
  m->set_stackmap_u2(stackmap_u2());

  // Copy byte codes
  if (code_length > 0) {
    memcpy(m->code_base(), code_start, code_length);
  }
  // Copy line number table
  if (compressed_linenumber_table_size > 0) {
    memcpy(m->compressed_linenumber_table(), compressed_linenumber_table, compressed_linenumber_table_size);
  }
  // Copy checked exceptions
  if (checked_exceptions_length > 0) {
    int size = checked_exceptions_length * sizeof(CheckedExceptionElement) / sizeof(u2);
    copy_u2_with_conversion((u2*) m->checked_exceptions_start(), checked_exceptions_start, size);
  }

  /* Copy class file LVT's/LVTT's into the HotSpot internal LVT.
   *
   * Rules for LVT's and LVTT's are:
   *   - There can be any number of LVT's and LVTT's.
   *   - If there are n LVT's, it is the same as if there was just
   *     one LVT containing all the entries from the n LVT's.
   *   - There may be no more than one LVT entry per local variable.
   *     Two LVT entries are 'equal' if these fields are the same:
   *        start_pc, length, name, slot
   *   - There may be no more than one LVTT entry per each LVT entry.
   *     Each LVTT entry has to match some LVT entry.
   *   - HotSpot internal LVT keeps natural ordering of class file LVT entries.
   */
  if (total_lvt_length > 0) {  
    int tbl_no, idx;

    promoted_flags->set_has_localvariable_table();

    LVT_Hash** lvt_Hash = NEW_RESOURCE_ARRAY(LVT_Hash*, HASH_ROW_SIZE);
    initialize_hashtable(lvt_Hash);

    // To fill LocalVariableTable in
    Classfile_LVT_Element*  cf_lvt;
    LocalVariableTableElement* lvt = m->localvariable_table_start();

    for (tbl_no = 0; tbl_no < lvt_cnt; tbl_no++) {
      cf_lvt = (Classfile_LVT_Element *) localvariable_table_start[tbl_no];
      for (idx = 0; idx < localvariable_table_length[tbl_no]; idx++, lvt++) {
        copy_lvt_element(&cf_lvt[idx], lvt);
        // If no duplicates, add LVT elem in hashtable lvt_Hash.
        if (LVT_put_after_lookup(lvt, lvt_Hash) == false 
          && _need_verify 
          && _major_version >= JAVA_1_5_VERSION ) {
          clear_hashtable(lvt_Hash);
          classfile_parse_error("Duplicated LocalVariableTable attribute "
                                "entry for '%s' in class file %s",
                                 cp->symbol_at(lvt->name_cp_index)->as_utf8(),
                                 CHECK_(nullHandle));
        }
      }
    }

    // To merge LocalVariableTable and LocalVariableTypeTable
    Classfile_LVT_Element* cf_lvtt;
    LocalVariableTableElement lvtt_elem;

    for (tbl_no = 0; tbl_no < lvtt_cnt; tbl_no++) {
      cf_lvtt = (Classfile_LVT_Element *) localvariable_type_table_start[tbl_no];
      for (idx = 0; idx < localvariable_type_table_length[tbl_no]; idx++) {
        copy_lvt_element(&cf_lvtt[idx], &lvtt_elem);
        int index = hash(&lvtt_elem);
        LVT_Hash* entry = LVT_lookup(&lvtt_elem, index, lvt_Hash);
        if (entry == NULL) {
          if (_need_verify) {
            clear_hashtable(lvt_Hash);
            classfile_parse_error("LVTT entry for '%s' in class file %s "
                                  "does not match any LVT entry",
                                   cp->symbol_at(lvtt_elem.name_cp_index)->as_utf8(),
                                   CHECK_(nullHandle));
          }
        } else if (entry->_elem->signature_cp_index != 0 && _need_verify) {
          clear_hashtable(lvt_Hash);
          classfile_parse_error("Duplicated LocalVariableTypeTable attribute "
                                "entry for '%s' in class file %s",
                                 cp->symbol_at(lvtt_elem.name_cp_index)->as_utf8(),
                                 CHECK_(nullHandle));
        } else {
          // to add generic signatures into LocalVariableTable
          entry->_elem->signature_cp_index = lvtt_elem.descriptor_cp_index;
        }
      }
    }
    clear_hashtable(lvt_Hash);
  }

  *method_annotations = assemble_annotations(runtime_visible_annotations,
                                             runtime_visible_annotations_length,
                                             runtime_invisible_annotations,
                                             runtime_invisible_annotations_length,
                                             CHECK_(nullHandle));
  *method_parameter_annotations = assemble_annotations(runtime_visible_parameter_annotations,
                                                       runtime_visible_parameter_annotations_length,
                                                       runtime_invisible_parameter_annotations,
                                                       runtime_invisible_parameter_annotations_length,
                                                       CHECK_(nullHandle));
  *method_default_annotations = assemble_annotations(annotation_default,
                                                     annotation_default_length,
                                                     NULL,
                                                     0,
                                                     CHECK_(nullHandle));

  return m;
}

  
// The promoted_flags parameter is used to pass relevant access_flags
// from the methods back up to the containing klass. These flag values
// are added to klass's access_flags.

objArrayHandle ClassFileParser::parse_methods(constantPoolHandle cp, bool is_interface, 
                                              AccessFlags* promoted_flags,
                                              objArrayOop* methods_annotations_oop,
                                              objArrayOop* methods_parameter_annotations_oop,
                                              objArrayOop* methods_default_annotations_oop,
                                              TRAPS) {
  objArrayHandle nullHandle;
  typeArrayHandle method_annotations;
  typeArrayHandle method_parameter_annotations;
  typeArrayHandle method_default_annotations;
  u2 length = stream()->get_u2(CHECK_(nullHandle));
  if (length == 0) {
    return objArrayHandle(THREAD, Universe::the_empty_system_obj_array());
  } else {
    objArrayOop m = oopFactory::new_system_objArray(length, CHECK_(nullHandle));
    objArrayHandle methods(THREAD, m);
    HandleMark hm(THREAD);
    objArrayHandle methods_annotations;
    objArrayHandle methods_parameter_annotations;
    objArrayHandle methods_default_annotations;
    for (int index = 0; index < length; index++) {
      methodHandle method = parse_method(cp, is_interface, 
                                         promoted_flags,
                                         &method_annotations,
                                         &method_parameter_annotations,
                                         &method_default_annotations,
                                         CHECK_(nullHandle));
      methods->obj_at_put(index, method());  
      if (method_annotations.not_null()) {
        if (methods_annotations.is_null()) {
          objArrayOop md = oopFactory::new_system_objArray(length, CHECK_(nullHandle));
          methods_annotations = objArrayHandle(THREAD, md);
        }
        methods_annotations->obj_at_put(index, method_annotations());
      }
      if (method_parameter_annotations.not_null()) {
        if (methods_parameter_annotations.is_null()) {
          objArrayOop md = oopFactory::new_system_objArray(length, CHECK_(nullHandle));
          methods_parameter_annotations = objArrayHandle(THREAD, md);
        }
        methods_parameter_annotations->obj_at_put(index, method_parameter_annotations());
      }
      if (method_default_annotations.not_null()) {
        if (methods_default_annotations.is_null()) {
          objArrayOop md = oopFactory::new_system_objArray(length, CHECK_(nullHandle));
          methods_default_annotations = objArrayHandle(THREAD, md);
        }
        methods_default_annotations->obj_at_put(index, method_default_annotations());
      }
    }
    if (_need_verify) {
      // Check duplicated methods
      ResourceMark rm(THREAD);
      NameSigHash** names_and_sigs = NEW_RESOURCE_ARRAY(NameSigHash*, HASH_ROW_SIZE);
      initialize_hashtable(names_and_sigs);
      for (int i = 0; i < length; i++) {
        methodOop m = (methodOop)methods->obj_at(i);
        // If no duplicates, add name/signature in hashtable names_and_sigs.
        if (put_after_lookup(m->name(), m->signature(), names_and_sigs) == false) {
          clear_hashtable(names_and_sigs);
          classfile_parse_error("Repetitive method name/signature in class file %s", CHECK_(nullHandle));
        }
      }
      clear_hashtable(names_and_sigs);
    }

    *methods_annotations_oop = methods_annotations();
    *methods_parameter_annotations_oop = methods_parameter_annotations();
    *methods_default_annotations_oop = methods_default_annotations();

    return methods;
  }
}


typeArrayHandle ClassFileParser::sort_methods(objArrayHandle methods,
                                              objArrayHandle methods_annotations,
                                              objArrayHandle methods_parameter_annotations,
                                              objArrayHandle methods_default_annotations,
                                              TRAPS) {
  typeArrayHandle nullHandle;
  int length = methods()->length();
  // If JVMTI original method ordering is enabled we have to 
  // remember the original class file ordering.
  // We temporarily use the vtable_index field in the methodOop to store the
  // class file index, so we can read in after calling qsort.
  if (JvmtiExport::can_maintain_original_method_order()) {
    for (int index = 0; index < length; index++) {
      methodOop m = methodOop(methods->obj_at(index));
      assert(m->vtable_index() == -1, "vtable index should not be set");
      m->set_vtable_index(index);
    }
  }
  // Sort method array by ascending method name (for faster lookups & vtable construction)
  // Note that the ordering is not alphabetical, see symbolOopDesc::fast_compare
  methodOopDesc::sort_methods(methods(),
                              methods_annotations(),
                              methods_parameter_annotations(),
                              methods_default_annotations());

  // If JVMTI original method ordering is enabled construct int array remembering the original ordering
  if (JvmtiExport::can_maintain_original_method_order()) {
    typeArrayOop new_ordering = oopFactory::new_permanent_intArray(length, CHECK_(nullHandle));
    typeArrayHandle method_ordering(THREAD, new_ordering);
    for (int index = 0; index < length; index++) {
      methodOop m = methodOop(methods->obj_at(index));
      int old_index = m->vtable_index();
      assert(old_index >= 0 && old_index < length, "invalid method index");
      method_ordering->int_at_put(index, old_index);
      m->set_vtable_index(-1);
    }
    return method_ordering;
  } else {
    return typeArrayHandle(THREAD, Universe::the_empty_int_array());
  }
}


void ClassFileParser::parse_classfile_sourcefile_attribute(constantPoolHandle cp, instanceKlassHandle k, TRAPS) {
  u2 sourcefile_index = stream()->get_u2(CHECK);
  check_property(valid_cp_range(sourcefile_index, cp->length()) &&
                 cp->tag_at(sourcefile_index).is_utf8(), 
                 "Invalid SourceFile attribute at constant pool index %d in class file %s", 
                 sourcefile_index,
                 CHECK);  
  k->set_source_file_name(cp->symbol_at(sourcefile_index));
}



void ClassFileParser::parse_classfile_source_debug_extension_attribute(constantPoolHandle cp, 
                                                                       instanceKlassHandle k, 
                                                                       int length, TRAPS) {
  u1* sde_buffer = stream()->get_u1_buffer(length);
  if (sde_buffer != NULL) {
    // Don't bother storing it if there is no way to retrieve it
    if (JvmtiExport::can_get_source_debug_extension()) {
      // Optimistically assume that only 1 byte UTF format is used
      // (common case)
      symbolOop sde_symbol = oopFactory::new_symbol((char*)sde_buffer, 
                                                    length, CHECK);
      k->set_source_debug_extension(sde_symbol);
    }
    // Got utf8 string, set stream position forward
    stream()->skip_u1(length, CHECK);
  } else {
    truncated_class_file_error(CHECK);
  }
}


// Inner classes can be static, private or protected (classic VM does this)
#define RECOGNIZED_INNER_CLASS_MODIFIERS (JVM_RECOGNIZED_CLASS_MODIFIERS | JVM_ACC_PRIVATE | JVM_ACC_PROTECTED | JVM_ACC_STATIC)

// Return number of classes in the inner classes attribute table
u2 ClassFileParser::parse_classfile_inner_classes_attribute(constantPoolHandle cp, instanceKlassHandle k, TRAPS) {  
  u2 length = stream()->get_u2(CHECK_0);

  // 4-tuples of shorts [inner_class_info_index, outer_class_info_index, inner_name_index, inner_class_access_flags]
  typeArrayOop ic = oopFactory::new_permanent_shortArray(length*4, CHECK_0);  
  typeArrayHandle inner_classes(THREAD, ic);
  int index = 0;
  int cp_size = cp->length();
  for (int n = 0; n < length; n++) {
    // Inner class index
    u2 inner_class_info_index = stream()->get_u2(CHECK_0);
    check_property(inner_class_info_index == 0 || 
                   (valid_cp_range(inner_class_info_index, cp_size) && 
                    cp->tag_at(inner_class_info_index).is_klass_reference()), 
                   "inner_class_info_index %d has bad constant type in class file %s", 
                    inner_class_info_index,
                    CHECK_0);
    // Outer class index
    u2 outer_class_info_index = stream()->get_u2(CHECK_0);
    check_property(outer_class_info_index == 0 || 
                   (valid_cp_range(outer_class_info_index, cp_size) &&
                    cp->tag_at(outer_class_info_index).is_klass_reference()), 
                   "outer_class_info_index %d has bad constant type in class file %s", 
                    outer_class_info_index,
                    CHECK_0);
    // Inner class name
    u2 inner_name_index = stream()->get_u2(CHECK_0);
    check_property(inner_name_index == 0 || 
                   (valid_cp_range(inner_name_index, cp_size) &&
                    cp->tag_at(inner_name_index).is_utf8()), 
                   "inner_name_index %d has bad constant type in class file %s", 
                    inner_name_index,
                    CHECK_0);    
    if (_need_verify) {
      guarantee_property(inner_class_info_index != outer_class_info_index, 
                         "Class is both outer and inner class in class file %s", CHECK_0);
    }
    // Access flags
    AccessFlags inner_access_flags;
    jint flags = stream()->get_u2(CHECK_0) & RECOGNIZED_INNER_CLASS_MODIFIERS;
    // 4012001: check flags before setting interface to abstract to catch non-abstract interfaces
    // Failed specjvm98 because it contains class modifier 0x0201 (public interface, non-abstract).
    if ((flags & JVM_ACC_INTERFACE) != 0) {
      // Set the abstract bit explicitly for interface classes (Classic VM does this)
      flags |= JVM_ACC_ABSTRACT;
    }
    verify_legal_class_modifiers(flags, CHECK_0);
    inner_access_flags.set_flags(flags);

    inner_classes->short_at_put(index++, inner_class_info_index);
    inner_classes->short_at_put(index++, outer_class_info_index);
    inner_classes->short_at_put(index++, inner_name_index);	
    inner_classes->short_at_put(index++, inner_access_flags.as_short());
  }

  // 4347400: make sure there's no duplicate entry in the classes array
  if (_need_verify && _major_version >= JAVA_1_5_VERSION) {
    for(int i = 0; i < inner_classes->length(); i += 4) {
      for(int j = i + 4; j < inner_classes->length(); j += 4) {
        guarantee_property((inner_classes->ushort_at(i)   != inner_classes->ushort_at(j) ||
                            inner_classes->ushort_at(i+1) != inner_classes->ushort_at(j+1) ||
                            inner_classes->ushort_at(i+2) != inner_classes->ushort_at(j+2) ||
                            inner_classes->ushort_at(i+3) != inner_classes->ushort_at(j+3)),
                            "Duplicate entry in InnerClasses in class file %s",
                            CHECK_0);
      }
    }  
  }  

  // Update instanceKlass with inner class info.  
  k->set_inner_classes(inner_classes());
  return length;  
}

void ClassFileParser::parse_classfile_synthetic_attribute(constantPoolHandle cp, instanceKlassHandle k, TRAPS) {
  k->set_is_synthetic();
}

void ClassFileParser::parse_classfile_signature_attribute(constantPoolHandle cp, instanceKlassHandle k, TRAPS) {
  u2 signature_index = stream()->get_u2(CHECK);
  check_property(valid_cp_range(signature_index, cp->length()) &&
                 cp->tag_at(signature_index).is_utf8(), 
                 "Invalid constant pool index %d in Signature attribute in class file %s", 
                 signature_index,
                 CHECK);    
  k->set_generic_signature(cp->symbol_at(signature_index));
}

void ClassFileParser::parse_classfile_attributes(constantPoolHandle cp, instanceKlassHandle k, TRAPS) {
  // Set inner classes attribute to default sentinel
  k->set_inner_classes(Universe::the_empty_short_array());
  u2 attributes_count = stream()->get_u2(CHECK);
  bool parsed_sourcefile_attribute = false;
  bool parsed_innerclasses_attribute = false;
  bool parsed_enclosingmethod_attribute = false;
  u1* runtime_visible_annotations = NULL;
  int runtime_visible_annotations_length = 0;
  u1* runtime_invisible_annotations = NULL;
  int runtime_invisible_annotations_length = 0;
  // Iterate over attributes
  while (attributes_count--) {    
    u2 attribute_name_index = stream()->get_u2(CHECK);
    u4 attribute_length = stream()->get_u4(CHECK);
    check_property(valid_cp_range(attribute_name_index, cp->length()) &&
                   cp->tag_at(attribute_name_index).is_utf8(), 
                   "Attribute name has bad constant pool index %d in class file %s", 
                   attribute_name_index,
                   CHECK);
    symbolOop tag = cp->symbol_at(attribute_name_index);
    if (tag == vmSymbols::tag_source_file()) {
      // Check for SourceFile tag
      if (_need_verify) {
        guarantee_property(attribute_length == 2, "Wrong SourceFile attribute length in class file %s", CHECK);
      }
      if (parsed_sourcefile_attribute) {
        classfile_parse_error("Multiple SourceFile attributes in class file %s", CHECK);
      } else {
        parsed_sourcefile_attribute = true;
      }
      parse_classfile_sourcefile_attribute(cp, k, CHECK);
    } else if (tag == vmSymbols::tag_source_debug_extension()) {
      // Check for SourceDebugExtension tag
      parse_classfile_source_debug_extension_attribute(cp, k, (int)attribute_length, CHECK);
    } else if (tag == vmSymbols::tag_inner_classes()) {
      // Check for InnerClasses tag
      if (parsed_innerclasses_attribute) {
        classfile_parse_error("Multiple InnerClasses attributes in class file %s", CHECK);
      } else {
        parsed_innerclasses_attribute = true;
      }
      u2 num_of_classes = parse_classfile_inner_classes_attribute(cp, k, CHECK);
      if (_need_verify && _major_version >= JAVA_1_5_VERSION) {
        guarantee_property(attribute_length == sizeof(num_of_classes) + 4 * sizeof(u2) * num_of_classes,
                          "Wrong InnerClasses attribute length in class file %s", CHECK);
      }
    } else if (tag == vmSymbols::tag_synthetic()) {
      // Check for Synthetic tag
      // Shouldn't we check that the synthetic flags wasn't already set? - not required in spec
      if (attribute_length != 0) {
        classfile_parse_error("Invalid Synthetic classfile attribute length %d in class file %s", 
                              attribute_length, CHECK);
      }
      parse_classfile_synthetic_attribute(cp, k, CHECK);
    } else if (tag == vmSymbols::tag_deprecated()) {
      // Check for Deprecatd tag - 4276120
      if (attribute_length != 0) {
        classfile_parse_error("Invalid Deprecated classfile attribute length %d in class file %s", 
                              attribute_length, CHECK);
      }
    } else if (_major_version >= JAVA_1_5_VERSION) {
      if (tag == vmSymbols::tag_signature()) {
        if (attribute_length != 2) {
          classfile_parse_error("Wrong Signature attribute length %d in class file %s", attribute_length, CHECK);
        }
        parse_classfile_signature_attribute(cp, k, CHECK);
      } else if (tag == vmSymbols::tag_runtime_visible_annotations()) {
        runtime_visible_annotations_length = attribute_length;
        runtime_visible_annotations = stream()->get_u1_buffer(runtime_visible_annotations_length);
        if (runtime_visible_annotations == NULL) truncated_class_file_error(CHECK);
        stream()->skip_u1(runtime_visible_annotations_length, CHECK);
      } else if (PreserveAllAnnotations && tag == vmSymbols::tag_runtime_invisible_annotations()) {
        runtime_invisible_annotations_length = attribute_length;
        runtime_invisible_annotations = stream()->get_u1_buffer(runtime_invisible_annotations_length);
        if (runtime_invisible_annotations == NULL) truncated_class_file_error(CHECK);
        stream()->skip_u1(runtime_invisible_annotations_length, CHECK);
      } else if (tag == vmSymbols::tag_enclosing_method()) {
        if (parsed_enclosingmethod_attribute) {
          classfile_parse_error("Multiple EnclosingMethod attributes in class file %s", CHECK);
        }   else {
          parsed_enclosingmethod_attribute = true;
        }
        u2 class_index  = stream()->get_u2(CHECK);
        u2 method_index = stream()->get_u2(CHECK);
        if (class_index == 0) {
          classfile_parse_error("Invalid class index in EnclosingMethod attribute in class file %s", CHECK);
        }
        // Validate the constant pool indices and types
        if (!cp->is_within_bounds(class_index) ||
            !cp->tag_at(class_index).is_klass_reference()) {
          classfile_parse_error("Invalid or out-of-bounds class index in EnclosingMethod attribute in class file %s", CHECK);
        }
        if (method_index != 0 &&
            (!cp->is_within_bounds(method_index) ||
             !cp->tag_at(method_index).is_name_and_type())) {
          classfile_parse_error("Invalid or out-of-bounds method index in EnclosingMethod attribute in class file %s", CHECK);
        }           
        k->set_enclosing_method_indices(class_index, method_index);
      } else {
        // Unknown attribute
        stream()->skip_u1(attribute_length, CHECK);
      }
    } else {
      // Unknown attribute
      stream()->skip_u1(attribute_length, CHECK);
    }
  }
  typeArrayHandle annotations = assemble_annotations(runtime_visible_annotations,
                                                     runtime_visible_annotations_length,
                                                     runtime_invisible_annotations,
                                                     runtime_invisible_annotations_length,
                                                     CHECK);
  k->set_class_annotations(annotations());
}


typeArrayHandle ClassFileParser::assemble_annotations(u1* runtime_visible_annotations,
                                                      int runtime_visible_annotations_length,
                                                      u1* runtime_invisible_annotations,
                                                      int runtime_invisible_annotations_length, TRAPS) {
  typeArrayHandle annotations;
  if (runtime_visible_annotations != NULL ||
      runtime_invisible_annotations != NULL) {
    typeArrayOop anno = oopFactory::new_permanent_byteArray(runtime_visible_annotations_length +
                                                            runtime_invisible_annotations_length, CHECK_(annotations));
    annotations = typeArrayHandle(THREAD, anno);
    if (runtime_visible_annotations != NULL) {
      memcpy(annotations->byte_at_addr(0), runtime_visible_annotations, runtime_visible_annotations_length);
    }
    if (runtime_invisible_annotations != NULL) {
      memcpy(annotations->byte_at_addr(runtime_visible_annotations_length), runtime_invisible_annotations, runtime_invisible_annotations_length);
    }
  }
  return annotations;
}


static void initialize_static_field(fieldDescriptor* fd, TRAPS) {
  KlassHandle h_k (THREAD, fd->field_holder());
  assert(h_k.not_null() && fd->is_static(), "just checking");
  if (fd->has_initial_value()) {
    BasicType t = fd->field_type();
    switch (t) {
      case T_BYTE:
        h_k()->byte_field_put(fd->offset(), fd->int_initial_value());
	      break;
      case T_BOOLEAN:
        h_k()->bool_field_put(fd->offset(), fd->int_initial_value());
	      break;
      case T_CHAR:
        h_k()->char_field_put(fd->offset(), fd->int_initial_value());
	      break;
      case T_SHORT:
        h_k()->short_field_put(fd->offset(), fd->int_initial_value());
	      break;
      case T_INT:
        h_k()->int_field_put(fd->offset(), fd->int_initial_value());
        break;
      case T_FLOAT:
        h_k()->float_field_put(fd->offset(), fd->float_initial_value());
        break;
      case T_DOUBLE:
        h_k()->double_field_put(fd->offset(), fd->double_initial_value());
        break;
      case T_LONG:
        h_k()->long_field_put(fd->offset(), fd->long_initial_value());
        break;
      case T_OBJECT:
        {
          #ifdef ASSERT      
          symbolOop sym = oopFactory::new_symbol("Ljava/lang/String;", CHECK);
          assert(fd->signature() == sym, "just checking");      
          #endif
          oop string = fd->string_initial_value(CHECK);
          h_k()->obj_field_put(fd->offset(), string);
        }
        break;
      default:
        THROW_MSG(vmSymbols::java_lang_ClassFormatError(), 
                  "Illegal ConstantValue attribute in class file");
    }
  }
}


void ClassFileParser::java_lang_ref_Reference_fix_pre(typeArrayHandle* fields_ptr,
  constantPoolHandle cp, FieldAllocationCount *fac_ptr, TRAPS) {
  // This code is for compatibility with earlier jdk's that do not
  // have the "discovered" field in java.lang.ref.Reference.  For 1.5
  // the check for the "discovered" field should issue a warning if
  // the field is not found.  For 1.6 this code should be issue a
  // fatal error if the "discovered" field is not found.
  //
  // Increment fac.nonstatic_oop_count so that the start of the
  // next type of non-static oops leaves room for the fake oop.
  // Do not increment next_nonstatic_oop_offset so that the
  // fake oop is place after the java.lang.ref.Reference oop
  // fields.
  //
  // Check the fields in java.lang.ref.Reference for the "discovered"
  // field.  If it is not present, artifically create a field for it.
  // This allows this VM to run on early JDK where the field is not
  // present.

  //
  // Increment fac.nonstatic_oop_count so that the start of the 
  // next type of non-static oops leaves room for the fake oop.
  // Do not increment next_nonstatic_oop_offset so that the
  // fake oop is place after the java.lang.ref.Reference oop
  // fields.
  //
  // Check the fields in java.lang.ref.Reference for the "discovered"
  // field.  If it is not present, artifically create a field for it.
  // This allows this VM to run on early JDK where the field is not
  // present.
  int reference_sig_index = 0;
  int reference_name_index = 0;
  int reference_index = 0;
  int extra = java_lang_ref_Reference::number_of_fake_oop_fields;
  const int n = (*fields_ptr)()->length();
  for (int i = 0; i < n; i += instanceKlass::next_offset ) {
    int name_index = 
    (*fields_ptr)()->ushort_at(i + instanceKlass::name_index_offset);
    int sig_index  = 
      (*fields_ptr)()->ushort_at(i + instanceKlass::signature_index_offset);
    symbolOop f_name = cp->symbol_at(name_index);
    symbolOop f_sig  = cp->symbol_at(sig_index);
    if (f_sig == vmSymbols::reference_signature() && reference_index == 0) {
      // Save the index for reference signature for later use.
      // The fake discovered field does not entries in the
      // constant pool so the index for its signature cannot
      // be extracted from the constant pool.  It will need 
      // later, however.  It's signature is vmSymbols::reference_signature()
      // so same an index for that signature.
      reference_sig_index = sig_index;
      reference_name_index = name_index;
      reference_index = i;
    }
    if (f_name == vmSymbols::reference_discovered_name() &&
      f_sig == vmSymbols::reference_signature()) {
      // The values below are fake but will force extra
      // non-static oop fields and a corresponding non-static 
      // oop map block to be allocated.
      extra = 0;
      break;
    }
  }
  if (extra != 0) { 
    fac_ptr->nonstatic_oop_count += extra;
    // Add the additional entry to "fields" so that the klass
    // contains the "discoverd" field and the field will be initialized
    // in instances of the object.
    int fields_with_fix_length = (*fields_ptr)()->length() + 
      instanceKlass::next_offset;
    typeArrayOop ff = oopFactory::new_permanent_shortArray(
                                                fields_with_fix_length, CHECK);
    typeArrayHandle fields_with_fix(THREAD, ff);

    // Take everything from the original but the length.
    for (int idx = 0; idx < (*fields_ptr)->length(); idx++) {
      fields_with_fix->ushort_at_put(idx, (*fields_ptr)->ushort_at(idx));
    }

    // Add the fake field at the end.
    int i = (*fields_ptr)->length();
    // There is no name index for the fake "discovered" field nor 
    // signature but a signature is needed so that the field will
    // be properly initialized.  Use one found for
    // one of the other reference fields. Be sure the index for the
    // name is 0.  In fieldDescriptor::initialize() the index of the
    // name is checked.  That check is by passed for the last nonstatic
    // oop field in a java.lang.ref.Reference which is assumed to be
    // this artificial "discovered" field.  An assertion checks that
    // the name index is 0.
    assert(reference_index != 0, "Missing signature for reference");

    int j;
    for (j = 0; j < instanceKlass::next_offset; j++) {
      fields_with_fix->ushort_at_put(i + j, 
	(*fields_ptr)->ushort_at(reference_index +j));
    }
    // Clear the public access flag and set the private access flag.
    short flags;
    flags = 
      fields_with_fix->ushort_at(i + instanceKlass::access_flags_offset);
    assert(!(flags & JVM_RECOGNIZED_FIELD_MODIFIERS), "Unexpected access flags set");
    flags = flags & (~JVM_ACC_PUBLIC);
    flags = flags | JVM_ACC_PRIVATE;
    AccessFlags access_flags;
    access_flags.set_flags(flags);
    assert(!access_flags.is_public(), "Failed to clear public flag");
    assert(access_flags.is_private(), "Failed to set private flag");
    fields_with_fix->ushort_at_put(i + instanceKlass::access_flags_offset, 
      flags);

    assert(fields_with_fix->ushort_at(i + instanceKlass::name_index_offset) 
      == reference_name_index, "The fake reference name is incorrect");
    assert(fields_with_fix->ushort_at(i + instanceKlass::signature_index_offset)
      == reference_sig_index, "The fake reference signature is incorrect");
    // The type of the field is stored in the low_offset entry during
    // parsing.
    assert(fields_with_fix->ushort_at(i + instanceKlass::low_offset) ==
      NONSTATIC_OOP, "The fake reference type is incorrect");

    // "fields" is allocated in the permanent generation.  Disgard
    // it and let it be collected.
    (*fields_ptr) = fields_with_fix;
  }
  return;
}


void ClassFileParser::java_lang_Class_fix_pre(objArrayHandle* methods_ptr, 
  FieldAllocationCount *fac_ptr, TRAPS) {
  // Add fake fields for java.lang.Class instances
  //
  // This is not particularly nice. We should consider adding a
  // private transient object field at the Java level to
  // java.lang.Class. Alternatively we could add a subclass of
  // instanceKlass which provides an accessor and size computer for
  // this field, but that appears to be more code than this hack.
  //
  // NOTE that we wedge these in at the beginning rather than the
  // end of the object because the Class layout changed between JDK
  // 1.3 and JDK 1.4 with the new reflection implementation; some
  // nonstatic oop fields were added at the Java level. The offsets
  // of these fake fields can't change between these two JDK
  // versions because when the offsets are computed at bootstrap
  // time we don't know yet which version of the JDK we're running in.

  // The values below are fake but will force two non-static oop fields and 
  // a corresponding non-static oop map block to be allocated.
  const int extra = java_lang_Class::number_of_fake_oop_fields;
  fac_ptr->nonstatic_oop_count += extra;
}


void ClassFileParser::java_lang_Class_fix_post(int* next_nonstatic_oop_offset_ptr) {
  // Cause the extra fake fields in java.lang.Class to show up before
  // the Java fields for layout compatibility between 1.3 and 1.4
  // Incrementing next_nonstatic_oop_offset here advances the 
  // location where the real java fields are placed.
  const int extra = java_lang_Class::number_of_fake_oop_fields;
  (*next_nonstatic_oop_offset_ptr) += (extra * wordSize);
}


instanceKlassHandle ClassFileParser::parseClassFile(symbolHandle name, 
                                                    Handle class_loader, 
                                                    Handle protection_domain, 
                                                    symbolHandle& parsed_name,
                                                    TRAPS) {
  // Timing
  PerfTraceTime vmtimer(ClassLoader::perf_accumulated_time());

  if (jvmpi::is_event_enabled(JVMPI_EVENT_CLASS_LOAD_HOOK)) {
    ClassFileStream* cfs = stream();
    unsigned char* ptr = cfs->buffer();
    unsigned char* end_ptr = cfs->buffer() + cfs->length();

    jvmpi::post_class_load_hook_event(&ptr, &end_ptr, jvmpi::jvmpi_alloc);

    if (ptr != cfs->buffer()) {
      cfs = new ClassFileStream(ptr, end_ptr - ptr, cfs->source());
      set_stream(cfs);
    }
  }

  if (JvmtiExport::should_post_class_file_load_hook()) {
    ClassFileStream* cfs = stream();
    unsigned char* ptr = cfs->buffer();
    unsigned char* end_ptr = cfs->buffer() + cfs->length();

    JvmtiExport::post_class_file_load_hook(name, class_loader, protection_domain, &ptr, &end_ptr);

    if (ptr != cfs->buffer()) {
      // JVMTI agent has modified class file data.
      // Set new class file stream using JVMTI agent modified
      // class file data.       
      cfs = new ClassFileStream(ptr, end_ptr - ptr, cfs->source());
      set_stream(cfs);
    }
  }


  instanceKlassHandle nullHandle;

  // Figure out whether we can skip format checking (matching classic VM behavior)
  _need_verify = Verifier::should_verify_for(class_loader());
  
  // Set the verify flag in stream
  stream()->set_verify(_need_verify);

  // Save the class file name for easier error message printing.
  _class_name = name.not_null()? name : vmSymbols::unknown_class_name();

  // Magic value
  u4 magic = stream()->get_u4(CHECK_(nullHandle));
  guarantee_property(magic == JAVA_CLASSFILE_MAGIC, 
                     "Incompatible magic value %d in class file %s", 
                     magic, CHECK_(nullHandle));

  // Version numbers  
  u2 minor_version = stream()->get_u2(CHECK_(nullHandle));
  u2 major_version = stream()->get_u2(CHECK_(nullHandle));

  // Check version numbers - we check this even with verifier off
  if ((major_version < JAVA_MIN_SUPPORTED_VERSION) || (major_version > JAVA_MAX_SUPPORTED_VERSION) 
   || ((major_version == JAVA_MAX_SUPPORTED_VERSION) && (minor_version > JAVA_MAX_SUPPORTED_MINOR_VERSION))) {
    THROW_MSG_(vmSymbols::java_lang_UnsupportedClassVersionError(), 
               "Bad version number in .class file", 
               nullHandle);
  }
  _major_version = major_version;
  _minor_version = minor_version;


  // Check if verification needs to be relaxed for this class file
  // Do not restrict it to jdk1.0 or jdk1.1 to maintain backward compatibility (4982376)
  _relax_verify = Verifier::relax_verify_for(class_loader());

  // Constant pool
  constantPoolHandle cp = parse_constant_pool(CHECK_(nullHandle));
  int cp_size = cp->length();

  // Access flags
  AccessFlags access_flags;
  jint flags = stream()->get_u2(CHECK_(nullHandle)) & JVM_RECOGNIZED_CLASS_MODIFIERS;
  // 4012001: check flags before setting interface to abstract to catch non-abstract interfaces
  // Failed specjvm98 because it contains class modifier 0x0201 (public interface, non-abstract).
  if ((flags & JVM_ACC_INTERFACE) != 0) {
    // Set the abstract bit explicitly for interface classes (Classic VM does this)
    flags |= JVM_ACC_ABSTRACT;
  }
  verify_legal_class_modifiers(flags, CHECK_(nullHandle));
  access_flags.set_flags(flags);

  // This class and superclass
  instanceKlassHandle super_klass;
  u2 this_class_index = stream()->get_u2(CHECK_(nullHandle));
  check_property(valid_cp_range(this_class_index, cp_size) &&
                 cp->tag_at(this_class_index).is_unresolved_klass(), 
                 "Invalid this class index %d in constant pool in class file %s", 
                 this_class_index,
                 CHECK_(nullHandle));

  symbolHandle class_name (THREAD, cp->unresolved_klass_at(this_class_index));
  assert(class_name.not_null(), "class_name can't be null");

  // It's important to set parsed_name *before* resolving the super class.
  // (it's used for cleanup by the caller if parsing fails)
  parsed_name = class_name;

  // Update _class_name which could be null previously to be class_name
  _class_name = class_name;

  // Don't need to check whether this class name is legal or not.
  // It has been checked when constant pool is parsed.
  // However, make sure it is not an array type.
  if (_need_verify) {
    guarantee_property(class_name->byte_at(0) != JVM_SIGNATURE_ARRAY, 
                       "Bad class name in class file %s", 
                       CHECK_(nullHandle));
  }
  
  klassOop preserve_this_klass;   // for storing result across HandleMark

  // release all handles when parsing is done
  { HandleMark hm(THREAD);

    // Checks if name in class file matches requested name
    if (name.not_null() && class_name() != name()) {
      ResourceMark rm(THREAD);
      Exceptions::fthrow(
        THREAD_AND_LOCATION,
        vmSymbolHandles::java_lang_NoClassDefFoundError(), 
        "%s (wrong name: %s)", 
        name->as_C_string(), 
        class_name->as_C_string()
      );
    }

    if (TraceClassLoadingPreorder) {
      tty->print("[Loading %s", name()->as_klass_external_name());
      if (stream()->source() != NULL) tty->print(" from %s", stream()->source());
      tty->print_cr("]");
    }

    u2 super_class_index = stream()->get_u2(CHECK_(nullHandle));
    if (super_class_index == 0) {
      check_property(class_name() == vmSymbols::java_lang_Object(), 
                     "Invalid superclass index %d in class file %s", 
                     super_class_index,
                     CHECK_(nullHandle));
    } else {
      check_property(valid_cp_range(super_class_index, cp_size) &&
                     cp->tag_at(super_class_index).is_unresolved_klass(), 
                     "Invalid superclass index %d in class file %s", 
                     super_class_index,
                     CHECK_(nullHandle));
      // The class name should be legal because it is checked when parsing constant pool.
      // However, make sure it is not an array type.
      if (_need_verify) {
        guarantee_property(cp->unresolved_klass_at(super_class_index)->byte_at(0) != JVM_SIGNATURE_ARRAY, 
                          "Bad superclass name in class file %s", CHECK_(nullHandle));
      }
    }

    // Interfaces
    objArrayHandle local_interfaces = parse_interfaces(cp, class_loader, protection_domain, CHECK_(nullHandle));

    // Fields (offsets are filled in later)
    struct FieldAllocationCount fac = {0,0,0,0,0,0,0,0,0,0};
    objArrayHandle fields_annotations;
    typeArrayHandle fields = parse_fields(cp, access_flags.is_interface(), &fac, &fields_annotations, CHECK_(nullHandle));
    // Methods
    AccessFlags promoted_flags;
    promoted_flags.set_flags(0);
    // These need to be oop pointers because they are allocated lazily
    // inside parse_methods inside a nested HandleMark
    objArrayOop methods_annotations_oop = NULL;
    objArrayOop methods_parameter_annotations_oop = NULL;
    objArrayOop methods_default_annotations_oop = NULL;
    objArrayHandle methods = parse_methods(cp, access_flags.is_interface(), 
                                           &promoted_flags,
                                           &methods_annotations_oop,
                                           &methods_parameter_annotations_oop,
                                           &methods_default_annotations_oop,
                                           CHECK_(nullHandle));

    objArrayHandle methods_annotations(THREAD, methods_annotations_oop);
    objArrayHandle methods_parameter_annotations(THREAD, methods_parameter_annotations_oop);
    objArrayHandle methods_default_annotations(THREAD, methods_default_annotations_oop);

    // We check super class after class file is parsed and format is checked
    if (super_class_index > 0) {
      symbolHandle sk (THREAD, cp->klass_name_at(super_class_index));
      if (access_flags.is_interface()) {
        // Before attempting to resolve the superclass, check for class format
        // errors not checked yet.
        guarantee_property(sk() == vmSymbols::java_lang_Object(),
                           "Interfaces must have java.lang.Object as superclass in class file %s",
                           CHECK_(nullHandle));
      }
      vmtimer.suspend();  // do not count recursive loading twice
      klassOop k = SystemDictionary::resolve_super_or_fail(class_name,
                                                           sk, 
                                                           class_loader, 
                                                           protection_domain, 
                                                           CHECK_(nullHandle));
      KlassHandle kh (THREAD, k);
      super_klass = instanceKlassHandle(THREAD, kh());
      vmtimer.resume();
      // Make sure super class is not final
      if (super_klass->is_final()) {
        THROW_MSG_(vmSymbols::java_lang_VerifyError(), "Cannot inherit from final class", nullHandle);
      }
    }

    // Compute the transitive list of all unique interfaces implemented by this class
    objArrayHandle transitive_interfaces = compute_transitive_interfaces(super_klass, local_interfaces, CHECK_(nullHandle));

    // sort methods
    typeArrayHandle method_ordering = sort_methods(methods,
                                                   methods_annotations,
                                                   methods_parameter_annotations,
                                                   methods_default_annotations,
                                                   CHECK_(nullHandle));

    // promote flags from parse_methods() to the klass' flags
    access_flags.add_promoted_flags(promoted_flags.as_int());

    // Size of Java vtable (in words)
    int vtable_size = 0;    
    int itable_size = 0;
    int num_miranda_methods = 0;

    klassVtable::compute_vtable_size_and_num_mirandas(vtable_size, 
  						      num_miranda_methods, 
						        super_klass(),
						        methods(),
						        access_flags,
						        class_loader(),
						        class_name(), 
						        local_interfaces());  
       
    // Size of Java itable (in words)
    itable_size = access_flags.is_interface() ? 0 : klassItable::compute_itable_size(transitive_interfaces);  
    
    // Field size and offset computation
    int nonstatic_field_size = super_klass() == NULL ? 0 : super_klass->nonstatic_field_size();
    int static_field_size = 0;
    int next_static_oop_offset;
    int next_static_double_offset;
    int next_static_word_offset;
    int next_static_short_offset;
    int next_static_byte_offset;
    int next_static_type_offset;
    int next_nonstatic_oop_offset;
    int next_nonstatic_double_offset;
    int next_nonstatic_word_offset;
    int next_nonstatic_short_offset;
    int next_nonstatic_byte_offset;
    int next_nonstatic_type_offset;
    int first_nonstatic_oop_offset;

    // Calculate the starting byte offsets
    next_static_oop_offset      = (instanceKlass::header_size() + 
		 		  align_object_offset(vtable_size) + 
				  align_object_offset(itable_size)) * wordSize;
    next_static_double_offset   = next_static_oop_offset + 
			 	  (fac.static_oop_count * oopSize);
    if ( fac.static_double_count && 
	 (Universe::field_type_should_be_aligned(T_DOUBLE) || 
 	  Universe::field_type_should_be_aligned(T_LONG)) ) {
      next_static_double_offset = align_size_up(next_static_double_offset, BytesPerLong);
    }

    next_static_word_offset     = next_static_double_offset + 
				  (fac.static_double_count * BytesPerLong);
    next_static_short_offset    = next_static_word_offset + 
				  (fac.static_word_count * BytesPerInt);
    next_static_byte_offset     = next_static_short_offset + 
				  (fac.static_short_count * BytesPerShort);
    next_static_type_offset     = align_size_up((next_static_byte_offset +
			          fac.static_byte_count ), wordSize );
    static_field_size 	        = (next_static_type_offset - 
			          next_static_oop_offset) / wordSize;
    next_nonstatic_oop_offset   = (instanceOopDesc::header_size() + 
				  nonstatic_field_size) * wordSize;

    // Add fake fields for java.lang.Class instances (also see below)
    if (class_name() == vmSymbols::java_lang_Class() && class_loader.is_null()) {
      java_lang_Class_fix_pre(&methods, &fac, CHECK_(nullHandle));
    }

    // Add a fake "discovered" field if it is not present 
    // for compatibility with earlier jdk's.
    if (class_name() == vmSymbols::java_lang_ref_Reference() 
      && class_loader.is_null()) {
      java_lang_ref_Reference_fix_pre(&fields, cp, &fac, CHECK_(nullHandle));
    }
    // end of "discovered" field compactibility fix

    next_nonstatic_double_offset= next_nonstatic_oop_offset + 
			 	  (fac.nonstatic_oop_count * oopSize);
    if ( fac.nonstatic_double_count && 
         (Universe::field_type_should_be_aligned(T_DOUBLE) || 
          Universe::field_type_should_be_aligned(T_LONG)) ) {
      next_nonstatic_double_offset = align_size_up(next_nonstatic_double_offset, BytesPerLong); 
    }
    next_nonstatic_word_offset  = next_nonstatic_double_offset + 
				  (fac.nonstatic_double_count * BytesPerLong);
    next_nonstatic_short_offset = next_nonstatic_word_offset + 
				  (fac.nonstatic_word_count * BytesPerInt);
    next_nonstatic_byte_offset  = next_nonstatic_short_offset + 
				  (fac.nonstatic_short_count * BytesPerShort);
    next_nonstatic_type_offset  = align_size_up((next_nonstatic_byte_offset +
			          fac.nonstatic_byte_count ), wordSize );
    nonstatic_field_size = nonstatic_field_size + ((next_nonstatic_type_offset - 
			          next_nonstatic_oop_offset) / wordSize );
 
    first_nonstatic_oop_offset  = next_nonstatic_oop_offset;

    // Add fake fields for java.lang.Class instances (also see above)
    if (class_name() == vmSymbols::java_lang_Class() && class_loader.is_null()) {
      java_lang_Class_fix_post(&next_nonstatic_oop_offset);
    }

    // Iterate over fields again and compute correct offsets.
    // The field allocation type was temporarily stored in the offset slot.
    // oop fields are located before non-oop fields (static and non-static).
    int len = fields->length();
    for (int i = 0; i < len; i += instanceKlass::next_offset) {
      int real_offset;
      FieldAllocationType atype = (FieldAllocationType) fields->ushort_at(i+4);
      switch (atype) {
        case STATIC_OOP:
          real_offset = next_static_oop_offset;
	        next_static_oop_offset += oopSize;
          break;
        case STATIC_BYTE:
          real_offset = next_static_byte_offset;
	        next_static_byte_offset += 1;
          break;
        case STATIC_SHORT:
          real_offset = next_static_short_offset;
	        next_static_short_offset += BytesPerShort;
          break;
        case STATIC_WORD:
          real_offset = next_static_word_offset;
	        next_static_word_offset += BytesPerInt;
          break;
        case STATIC_ALIGNED_DOUBLE:
        case STATIC_DOUBLE:
          real_offset = next_static_double_offset;
          next_static_double_offset += BytesPerLong;
          break;
        case NONSTATIC_OOP:
          real_offset = next_nonstatic_oop_offset;
	        next_nonstatic_oop_offset += oopSize;
          break;
        case NONSTATIC_BYTE:
          real_offset = next_nonstatic_byte_offset;
	        next_nonstatic_byte_offset += 1;
          break;
        case NONSTATIC_SHORT:
          real_offset = next_nonstatic_short_offset;
	        next_nonstatic_short_offset += BytesPerShort;
          break;
        case NONSTATIC_WORD:
          real_offset = next_nonstatic_word_offset;
	        next_nonstatic_word_offset += BytesPerInt;
          break;
        case NONSTATIC_ALIGNED_DOUBLE:
        case NONSTATIC_DOUBLE:
          real_offset = next_nonstatic_double_offset;
          next_nonstatic_double_offset += BytesPerLong;
          break;
        default:
          ShouldNotReachHere();
      }
      fields->short_at_put(i+4, extract_low_short_from_int(real_offset) );
      fields->short_at_put(i+5, extract_high_short_from_int(real_offset) ); 
    }

    // Size of instances
    int instance_size;

    instance_size = align_object_size(next_nonstatic_type_offset / wordSize);

    // Size of non-static oop map blocks (in words) allocated at end of klass
    int nonstatic_oop_map_size = compute_oop_map_size(super_klass, fac.nonstatic_oop_count, first_nonstatic_oop_offset);

    // Compute reference type
    ReferenceType rt;
    if (super_klass() == NULL) {
      rt = REF_NONE;
    } else {
      rt = super_klass->reference_type();
    }

    // We can now create the basic klassOop for this klass    
    klassOop ik = oopFactory::new_instanceKlass(
                                    vtable_size, itable_size, 
                                    static_field_size, nonstatic_oop_map_size, 
                                    rt, CHECK_(nullHandle));
    instanceKlassHandle this_klass (THREAD, ik); 

    assert(this_klass->static_field_size() == static_field_size && 
           this_klass->nonstatic_oop_map_size() == nonstatic_oop_map_size, "sanity check");
    
    // Fill in information already parsed
    this_klass->set_access_flags(access_flags);
    this_klass->set_size_helper(instance_size);
    // Not yet: supers are done below to support the new subtype-checking fields
    //this_klass->set_super(super_klass());  
    this_klass->set_class_loader(class_loader());    
    this_klass->set_nonstatic_field_size(nonstatic_field_size);
    this_klass->set_static_oop_field_size(fac.static_oop_count);       
    cp->set_pool_holder(this_klass());
    this_klass->set_constants(cp());
    this_klass->set_local_interfaces(local_interfaces());
    this_klass->set_fields(fields());
    this_klass->set_methods(methods());
    this_klass->set_method_ordering(method_ordering());
    this_klass->set_name(cp->klass_name_at(this_class_index));
    this_klass->set_protection_domain(protection_domain());
    this_klass->set_fields_annotations(fields_annotations());
    this_klass->set_methods_annotations(methods_annotations());
    this_klass->set_methods_parameter_annotations(methods_parameter_annotations());
    this_klass->set_methods_default_annotations(methods_default_annotations());

    this_klass->set_major_version(major_version);
      
    // Miranda methods
    if ((num_miranda_methods > 0) || 
	// if this class introduced new miranda methods or
	(super_klass.not_null() && (super_klass->has_miranda_methods()))
	// super class exists and this class inherited miranda methods
	) {
      this_klass->set_has_miranda_methods(); // then set a flag
    }

    // Additional attributes
    parse_classfile_attributes(cp, this_klass, CHECK_(nullHandle));

    // Make sure this is the end of class file stream
    if (_need_verify && !_relax_verify) {
      guarantee_property(stream()->at_eos(), "Extra bytes at the end of class file %s", CHECK_(nullHandle));
    }

    // Initialize static fields
    this_klass->do_local_static_fields(&initialize_static_field, CHECK_(nullHandle));

    // VerifyOops believes that once this has been set, the object is completely loaded.
    // Compute transitive closure of interfaces this class implements
    this_klass->set_transitive_interfaces(transitive_interfaces());    

    // Fill in information needed to compute superclasses.
    this_klass->initialize_supers(super_klass(), CHECK_(nullHandle));

    // Initialize itable offset tables
    klassItable::setup_itable_offset_table(this_klass);

    // Do final class setup
    fill_oop_maps(this_klass, fac.nonstatic_oop_count, first_nonstatic_oop_offset);

    set_precomputed_flags(this_klass);

    // reinitialize modifiers, using the InnerClasses attribute
    int computed_modifiers = this_klass->compute_modifier_flags(CHECK_(nullHandle));
    this_klass->set_modifier_flags(computed_modifiers);

    // check if this class can access its super class
    check_super_class_access(this_klass, CHECK_(nullHandle));

    // check if this class can access its superinterfaces
    check_super_interface_access(this_klass, CHECK_(nullHandle));

    // check if this class overrides any final method
    check_final_method_override(this_klass, CHECK_(nullHandle));

    // check that if this class is an interface then it doesn't have static methods
    if (this_klass->is_interface()) {
      check_illegal_static_method(this_klass, CHECK_(nullHandle));
    }

    ClassLoadingService::notify_class_loaded(instanceKlass::cast(this_klass()), 
                                             false /* not shared class */);
	  
    if (TraceClassLoading) {
      // print in a single call to reduce interleaving of output
      if (stream()->source() != NULL) {
        tty->print("[Loaded %s from %s]\n", this_klass->external_name(),
                   stream()->source());
      } else if (class_loader.is_null()) {
        if (THREAD->is_Java_thread()) {
          klassOop caller = ((JavaThread*)THREAD)->security_get_caller_class(1);
          tty->print("[Loaded %s by instance of %s]\n",
                     this_klass->external_name(),
                     instanceKlass::cast(caller)->external_name());
        } else {
          tty->print("[Loaded %s]\n", this_klass->external_name());
        }
      } else {
        ResourceMark rm;
        tty->print("[Loaded %s from %s]\n", this_klass->external_name(),
                   instanceKlass::cast(class_loader->klass())->external_name());
      }
    }

    if (TraceClassResolution) {
      // print out the superclass.
      const char * from = Klass::cast(this_klass())->external_name();
      if (this_klass->java_super() != NULL) {
        tty->print("RESOLVE %s %s\n", from, instanceKlass::cast(this_klass->java_super())->external_name());
      }
      // print out each of the interface classes referred to by this class.
      objArrayHandle local_interfaces = this_klass->local_interfaces();
      if (!local_interfaces.is_null()) {
        int length = local_interfaces->length();
        for (int i = 0; i < length; i++) {
          klassOop k = klassOop(local_interfaces->obj_at(i)); 
          instanceKlass* to_class = instanceKlass::cast(k);
          const char * to = to_class->external_name();
          tty->print("RESOLVE %s %s\n", from, to);
        }
      }
    }

    // preserve result across HandleMark  
    preserve_this_klass = this_klass();    
  }

  // Create new handle outside HandleMark
  instanceKlassHandle this_klass (THREAD, preserve_this_klass);
  debug_only(this_klass->as_klassOop()->verify();)

  return this_klass;
}


int ClassFileParser::compute_oop_map_size(instanceKlassHandle super, int nonstatic_oop_count, int first_nonstatic_oop_offset) {
  int map_size = super.is_null() ? 0 : super->nonstatic_oop_map_size();
  if (nonstatic_oop_count > 0) {
    // We have oops to add to map
    if (map_size == 0) {
      map_size++;
    } else {
      // Check whether we should add a new map block or whether the last one can be extended
      OopMapBlock* first_map = super->start_of_nonstatic_oop_maps();
      OopMapBlock* last_map = first_map + map_size - 1;

      int next_offset;
      next_offset = last_map->offset() + (last_map->length() * oopSize);

      if (next_offset != first_nonstatic_oop_offset) {
        // Superklass didn't end with a oop field, add extra map
        assert(next_offset<first_nonstatic_oop_offset, "just checking");
        map_size++;
      }
    }
  }
  return map_size;
}


void ClassFileParser::fill_oop_maps(instanceKlassHandle k, int nonstatic_oop_count, int first_nonstatic_oop_offset) {
  OopMapBlock* this_oop_map = k->start_of_nonstatic_oop_maps();
  OopMapBlock* last_oop_map = this_oop_map + k->nonstatic_oop_map_size() - 1;
  instanceKlass* super = k->superklass();
  if (super != NULL) {
    int super_oop_map_size     = super->nonstatic_oop_map_size();
    OopMapBlock* super_oop_map = super->start_of_nonstatic_oop_maps();
    // Copy maps from superklass
    while (super_oop_map_size-- > 0) {
      *this_oop_map++ = *super_oop_map++;
    }
  }
  if (nonstatic_oop_count > 0) {
    if (this_oop_map == last_oop_map) {
      // We added a new map block, fill it
      last_oop_map->set_offset(first_nonstatic_oop_offset);
      last_oop_map->set_length(nonstatic_oop_count);
    } else {
      // We should extend the last map block copied from the superklass
      assert(this_oop_map == last_oop_map + 1, "just checking");
      last_oop_map->set_length(last_oop_map->length() + nonstatic_oop_count);
    }
  }
}


void ClassFileParser::set_precomputed_flags(instanceKlassHandle k) {
  // Check if this klass has an empty finalize method (i.e. one with return bytecode only),
  // in which case we don't have to register objects as finalizable
  methodOop m = k->lookup_method(vmSymbols::finalize_method_name(), 
                                 vmSymbols::void_method_signature());
  if (m != NULL && !m->is_empty_method()) {
    k->set_has_finalizer();
  }

  // Check if this klass supports the java.lang.Cloneable interface
  if (SystemDictionary::cloneable_klass_loaded()) {
    if (k->is_subtype_of(SystemDictionary::cloneable_klass())) {
      k->set_is_cloneable();
    }
  }

  // Check if this klass has a vanilla default constructor
  klassOop super = k->super();
  if (super == NULL) {
    // java.lang.Object has empty default constructor
    k->set_has_vanilla_constructor();
  } else {
    if (Klass::cast(super)->has_vanilla_constructor()) {
      methodOop constructor = k->find_method(vmSymbols::object_initializer_name(), vmSymbols::void_method_signature());
      if (constructor != NULL && constructor->is_vanilla_constructor()) {
        k->set_has_vanilla_constructor();
      }
    }
  }

  if (!k->has_finalizer() && !k->is_abstract() && !k->is_interface()) {
    k->set_can_be_fastpath_allocated(); // Allow fast-path allocation
  }
}


// utility method for appending and array with check for duplicates

void append_interfaces(objArrayHandle result, int& index, objArrayOop ifs) {
  // iterate over new interfaces
  for (int i = 0; i < ifs->length(); i++) {
    oop e = ifs->obj_at(i);
    assert(e->is_klass() && instanceKlass::cast(klassOop(e))->is_interface(), "just checking");
    // check for duplicates
    bool duplicate = false;
    for (int j = 0; j < index; j++) {
      if (result->obj_at(j) == e) {
        duplicate = true;
        break;
      }
    }
    // add new interface
    if (!duplicate) {
      result->obj_at_put(index++, e);
    }
  }
}

objArrayHandle ClassFileParser::compute_transitive_interfaces(instanceKlassHandle super, objArrayHandle local_ifs, TRAPS) {
  // Compute maximum size for transitive interfaces
  int max_transitive_size = 0;
  int super_size = 0;
  // Add superclass transitive interfaces size
  if (super.not_null()) {
    super_size = super->transitive_interfaces()->length();
    max_transitive_size += super_size;
  }
  // Add local interfaces' super interfaces  
  int local_size = local_ifs->length();
  for (int i = 0; i < local_size; i++) {
    klassOop l = klassOop(local_ifs->obj_at(i));
    max_transitive_size += instanceKlass::cast(l)->transitive_interfaces()->length();
  }
  // Finally add local interfaces
  max_transitive_size += local_size;
  // Construct array
  objArrayHandle result;
  if (max_transitive_size == 0) {
    // no interfaces, use canonicalized array
    result = objArrayHandle(THREAD, Universe::the_empty_system_obj_array());
  } else if (max_transitive_size == super_size) {
    // no new local interfaces added, share superklass' transitive interface array
    result = objArrayHandle(THREAD, super->transitive_interfaces());
  } else if (max_transitive_size == local_size) {
    // only local interfaces added, share local interface array
    result = local_ifs;
  } else {
    objArrayHandle nullHandle;
    objArrayOop new_objarray = oopFactory::new_system_objArray(max_transitive_size, CHECK_(nullHandle));
    result = objArrayHandle(THREAD, new_objarray);
    int index = 0;
    // Copy down from superclass
    if (super.not_null()) {
      append_interfaces(result, index, super->transitive_interfaces());
    }    
    // Copy down from local interfaces' superinterfaces
    for (int i = 0; i < local_ifs->length(); i++) {
      klassOop l = klassOop(local_ifs->obj_at(i));
      append_interfaces(result, index, instanceKlass::cast(l)->transitive_interfaces());
    }
    // Finally add local interfaces
    append_interfaces(result, index, local_ifs());

    // Check if duplicates were removed
    if (index != max_transitive_size) {
      assert(index < max_transitive_size, "just checking");
      objArrayOop new_result = oopFactory::new_system_objArray(index, CHECK_(nullHandle));
      for (int i = 0; i < index; i++) {
        oop e = result->obj_at(i);
        assert(e != NULL, "just checking");
        new_result->obj_at_put(i, e);
      }
      result = objArrayHandle(THREAD, new_result);
    }
  }
  return result;  
}


void ClassFileParser::check_super_class_access(instanceKlassHandle this_klass, TRAPS) {
  klassOop super = this_klass->super();
  if ((super != NULL) &&
      (!Reflection::verify_class_access(this_klass->as_klassOop(), super, false))) {
    ResourceMark rm(THREAD);
    Exceptions::fthrow(  
      THREAD_AND_LOCATION,
      vmSymbolHandles::java_lang_IllegalAccessError(),
      "class %s cannot access its superclass %s",
      this_klass->external_name(),
      instanceKlass::cast(super)->external_name()
      );
  }
}


void ClassFileParser::check_super_interface_access(instanceKlassHandle this_klass, TRAPS) {
  objArrayHandle local_interfaces (THREAD, this_klass->local_interfaces());
  int lng = local_interfaces->length();
  for (int i = lng - 1; i >= 0; i--) {
    klassOop k = klassOop(local_interfaces->obj_at(i)); 
    assert (k != NULL && Klass::cast(k)->is_interface(), "invalid interface");
    if (!Reflection::verify_class_access(this_klass->as_klassOop(), k, false)) {
      ResourceMark rm(THREAD);
      Exceptions::fthrow(  
        THREAD_AND_LOCATION,
        vmSymbolHandles::java_lang_IllegalAccessError(),
        "class %s cannot access its superinterface %s",
        this_klass->external_name(),
        instanceKlass::cast(k)->external_name()
        );
    }
  }
}


void ClassFileParser::check_final_method_override(instanceKlassHandle this_klass, TRAPS) {
  objArrayHandle methods (THREAD, this_klass->methods());
  int num_methods = methods->length();
  
  // go thru each method and check if it overrides a final method
  for (int index = 0; index < num_methods; index++) {
    methodOop m = (methodOop)methods->obj_at(index);

    // skip private, static and <init> methods
    if ((!m->is_private()) &&
	      (!m->is_static()) &&
	      (m->name() != vmSymbols::object_initializer_name())) {
	
      symbolOop name = m->name();
      symbolOop signature = m->signature();
      klassOop k = this_klass->super();
      methodOop super_m = NULL;
      while (k != NULL) {
	      // lookup a matching method in the super class hierarchy
        super_m = instanceKlass::cast(k)->lookup_method(name, signature); 
	      if (super_m == NULL) {
	        break; // didn't find any match; get out
	      }

	      if (super_m->is_final() &&
	        // matching method in super is final
	        (Reflection::verify_field_access(this_klass->as_klassOop(), 
					                                 super_m->method_holder(),
					                                 super_m->method_holder(),
					                                 super_m->access_flags(),
					                                 false))
	        // this class can access super final method and therefore override
	        ) {
	        ResourceMark rm(THREAD);
	        Exceptions::fthrow(  
	          THREAD_AND_LOCATION,
	          vmSymbolHandles::java_lang_VerifyError(),
	          "class %s overrides final method %s.%s",
	          this_klass->external_name(),
	          name,
	          signature
	        );
	      }

        k = instanceKlass::cast(super_m->method_holder())->super(); // continue to look
      }
    }
  }
}


// assumes that this_klass is an interface
void ClassFileParser::check_illegal_static_method(instanceKlassHandle this_klass, TRAPS) {
  assert(this_klass->is_interface(), "not an interface");
  objArrayHandle methods (THREAD, this_klass->methods());
  int num_methods = methods->length();

  for (int index = 0; index < num_methods; index++) {
    methodOop m = (methodOop)methods->obj_at(index);
    // if m is static and not the init method, throw a verify error
    if ((m->is_static()) && (m->name() != vmSymbols::class_initializer_name())) {
      ResourceMark rm(THREAD);
      Exceptions::fthrow(  
	      THREAD_AND_LOCATION,
	      vmSymbolHandles::java_lang_VerifyError(),
	      "Illegal static method %s in interface %s",
	      m->name(),
	      this_klass->external_name()
	    );
    }
  }
}

// utility methods for format checking 

void ClassFileParser::verify_legal_class_modifiers(jint flags, TRAPS) {
  if (!_need_verify) { return; }

  if ( ((flags & JVM_ACC_INTERFACE) 
     && (((flags & JVM_ACC_ABSTRACT) == 0) || (flags & JVM_ACC_FINAL) 
          /* 4012001: rejects super interfaces - commented out until JCK fixes bug 4895745
       || (flags & JVM_ACC_SUPER) */
       || (_major_version >= JAVA_1_5_VERSION && (flags & JVM_ACC_SYNTHETIC)) ))
   || (((flags & JVM_ACC_INTERFACE) == 0) 
     && (((flags & JVM_ACC_FINAL) && (flags & JVM_ACC_ABSTRACT)) || ((_major_version >= JAVA_1_5_VERSION) && (flags & JVM_ACC_ANNOTATION)))) ) {
    ResourceMark rm(THREAD);
    Exceptions::fthrow(THREAD_AND_LOCATION, vmSymbols::java_lang_ClassFormatError(),
                       "Illegal class modifiers in class %s: 0x%X", _class_name->as_C_string(), flags);
  }
}

void ClassFileParser::verify_legal_field_modifiers(jint flags, bool is_interface, TRAPS) {
  if (!_need_verify) { return; }

  if (!is_interface) {   // class fields
    if ( ((flags & JVM_ACC_PUBLIC) && ((flags & JVM_ACC_PROTECTED) || (flags & JVM_ACC_PRIVATE)))
      || ((flags & JVM_ACC_PROTECTED) && ((flags & JVM_ACC_PUBLIC) || (flags & JVM_ACC_PRIVATE)))
      || ((flags & JVM_ACC_PRIVATE) && ((flags & JVM_ACC_PROTECTED) || (flags & JVM_ACC_PUBLIC))) 
      || ((flags & JVM_ACC_FINAL) && (flags & JVM_ACC_VOLATILE)) ) {
      ResourceMark rm(THREAD);
      Exceptions::fthrow(THREAD_AND_LOCATION, vmSymbols::java_lang_ClassFormatError(),
                         "Illegal field modifiers in class %s: 0x%X", _class_name->as_C_string(), flags);
    }
  } else {    // interface fields
    if ( !(flags & JVM_ACC_PUBLIC) || !(flags & JVM_ACC_STATIC) || !(flags & JVM_ACC_FINAL)
      || (flags & JVM_ACC_PRIVATE) || (flags & JVM_ACC_PROTECTED) 
      || (flags & JVM_ACC_VOLATILE) || (flags & JVM_ACC_TRANSIENT)
      || (_major_version >= JAVA_1_5_VERSION && (flags & JVM_ACC_ENUM)) ) {
      ResourceMark rm(THREAD);
      Exceptions::fthrow(THREAD_AND_LOCATION, vmSymbols::java_lang_ClassFormatError(),
                         "Illegal field modifiers in class %s: 0x%X", _class_name->as_C_string(), flags);
    }
  }
}

void ClassFileParser::verify_legal_method_modifiers(jint flags, bool is_interface, symbolHandle name, TRAPS) {
  if (!_need_verify) { return; }

  bool legal = true;
  if (!is_interface) {
    // class or instance methods
    assert(!name.is_null(), "method name is null");
    if (name == vmSymbols::object_initializer_name()) {
      if ((flags & JVM_ACC_STATIC) || (flags & JVM_ACC_FINAL) || (flags & JVM_ACC_SYNCHRONIZED) 
       || (flags & JVM_ACC_NATIVE) || (flags & JVM_ACC_ABSTRACT) 
       || (_major_version >= JAVA_1_5_VERSION && (flags & JVM_ACC_BRIDGE)) ) {
        legal = false;
      }
    } else {
      if (flags & JVM_ACC_ABSTRACT) {
        if ((flags & JVM_ACC_FINAL) || (flags & JVM_ACC_NATIVE)
         || (flags & JVM_ACC_PRIVATE) || (flags & JVM_ACC_STATIC) 
         || (_major_version >= JAVA_1_5_VERSION && (flags & JVM_ACC_SYNCHRONIZED) 
           // 4012001: rejects abstract strict and abstract synchronized methods
           /* This is commented out until javac fixes bug 4890688
           || (flags & JVM_ACC_STRICT)) */ ) ) {
          legal = false;
        }
      }
    }
  } else {
    // interface methods
    if (!(flags & JVM_ACC_ABSTRACT) || !(flags & JVM_ACC_PUBLIC)
      || (flags & JVM_ACC_STATIC) || (flags & JVM_ACC_FINAL) 
      || (flags & JVM_ACC_NATIVE) 
      || (_major_version >= JAVA_1_5_VERSION && (flags & JVM_ACC_SYNCHRONIZED)
        // 4012001: rejects abstract strict and abstract synchronized methods
        /* This is commented out until javac fixes bug 4890688
        || (flags & JVM_ACC_STRICT) */ ) ) {
      legal = false;
    }
  }
  if ((flags & JVM_ACC_PUBLIC) && ((flags & JVM_ACC_PROTECTED) || (flags & JVM_ACC_PRIVATE))) {
    legal = false;
  }
  if ((flags & JVM_ACC_PROTECTED) && ((flags & JVM_ACC_PUBLIC) || (flags & JVM_ACC_PRIVATE))) {
    legal = false;
  }
  if ((flags & JVM_ACC_PRIVATE) && ((flags & JVM_ACC_PROTECTED) || (flags & JVM_ACC_PUBLIC))) {
    legal = false;
  }
  if (!legal) {
    ResourceMark rm(THREAD);
    Exceptions::fthrow(THREAD_AND_LOCATION, vmSymbols::java_lang_ClassFormatError(),
                       "Method %s in class %s has illegal modifiers: 0x%X", 
                       name->as_C_string(), _class_name->as_C_string(), flags);
  }
}

void ClassFileParser::verify_legal_utf8(const unsigned char* buffer, int length, TRAPS) {
  if (!_need_verify) { return; }

  for(int i = 0; i < length; i++) {
    unsigned short c;
    // no embedded zeros
    guarantee_property((buffer[i] != 0), "Illegal UTF8 string in constant pool in class file %s", CHECK);
    if(buffer[i] < 128) {
      continue;
    }
    if ((i + 5) < length) { // see if it's legal supplementary character
      if (UTF8::is_supplementary_character(&buffer[i])) {
        c = UTF8::get_supplementary_character(&buffer[i]);
        i += 5;
        continue;
      } 
    }
    switch (buffer[i] >> 4) {
      default: break;
      case 0x8: case 0x9: case 0xA: case 0xB: case 0xF:
        classfile_parse_error("Illegal UTF8 string in constant pool in class file %s", CHECK);
      case 0xC: case 0xD:  // 110xxxxx  10xxxxxx
        c = (buffer[i] & 0x1F) << 6;
        i++;
        if ((i < length) && ((buffer[i] & 0xC0) == 0x80)) {
          c += buffer[i] & 0x3F;
          if (_major_version <= 47 || c == 0 || c >= 0x80) {
            // for classes with major > 47, c must a null or a character in its shortest form
            break;
          }
        } 
        classfile_parse_error("Illegal UTF8 string in constant pool in class file %s", CHECK);
      case 0xE:  // 1110xxxx 10xxxxxx 10xxxxxx
        c = (buffer[i] & 0xF) << 12;
        i += 2;
        if ((i < length) && ((buffer[i-1] & 0xC0) == 0x80) && ((buffer[i] & 0xC0) == 0x80)) {
          c += ((buffer[i-1] & 0x3F) << 6) + (buffer[i] & 0x3F);
          if (_major_version <= 47 || c >= 0x800) {
            // for classes with major > 47, c must be in its shortest form
            break;
          }
        }
        classfile_parse_error("Illegal UTF8 string in constant pool in class file %s", CHECK);
    }  // end of switch
  } // end of for
}

// Checks if name is a legal class name.
void ClassFileParser::verify_legal_class_name(symbolHandle name, TRAPS) {
  if (!_need_verify || _relax_verify) { return; }

  char* bytes = name->as_utf8();
  unsigned int length = name->utf8_length();
  bool legal = false;

  if (length > 0) {
    char* p;
    if (bytes[0] == JVM_SIGNATURE_ARRAY) {
      p = skip_over_field_signature(bytes, false, length, CHECK);
      legal = (p != NULL) && ((p - bytes) == (int)length);
    } else if (_major_version < JAVA_1_5_VERSION) {
      if (bytes[0] != '<') {
        p = skip_over_field_name(bytes, true, length);
        legal = (p != NULL) && ((p - bytes) == (int)length);
      }
    } else {
      // 4900761: relax the constraints based on JSR202 spec
      // Class names may be drawn from the entire Unicode character set.
      // Identifiers between '/' must be unqualified names.
      // The utf8 string has been verified when parsing cpool entries.
      legal = verify_unqualified_name(bytes, length, LegalClass);  
    }
  } 
  if (!legal) {
    ResourceMark rm(THREAD);
    Exceptions::fthrow(THREAD_AND_LOCATION, vmSymbols::java_lang_ClassFormatError(),
                       "Illegal class name \"%s\" in class file %s", bytes, _class_name->as_C_string());
  }
}

// Checks if name is a legal field name.
void ClassFileParser::verify_legal_field_name(symbolHandle name, TRAPS) {
  if (!_need_verify || _relax_verify) { return; }

  char* bytes = name->as_utf8();
  unsigned int length = name->utf8_length();
  bool legal = false;

  if (length > 0) {
    if (_major_version < JAVA_1_5_VERSION) {
      if (bytes[0] != '<') { 
        char* p = skip_over_field_name(bytes, false, length);
        legal = (p != NULL) && ((p - bytes) == (int)length);
      }
    } else {
      // 4881221: relax the constraints based on JSR202 spec
      legal = verify_unqualified_name(bytes, length, LegalField);
    }
  }

  if (!legal) {
    ResourceMark rm(THREAD);
    Exceptions::fthrow(THREAD_AND_LOCATION, vmSymbols::java_lang_ClassFormatError(),
                       "Illegal field name \"%s\" in class %s", bytes, _class_name->as_C_string());
  }
}

// Checks if name is a legal method name.
void ClassFileParser::verify_legal_method_name(symbolHandle name, TRAPS) {
  if (!_need_verify || _relax_verify) { return; }

  assert(!name.is_null(), "method name is null");
  char* bytes = name->as_utf8();
  unsigned int length = name->utf8_length();
  bool legal = false;

  if (length > 0) {
    if (bytes[0] == '<') {
      if (name == vmSymbols::object_initializer_name() || name == vmSymbols::class_initializer_name()) {
        legal = true;
      }
    } else if (_major_version < JAVA_1_5_VERSION) {
      char* p;
      p = skip_over_field_name(bytes, false, length);
      legal = (p != NULL) && ((p - bytes) == (int)length);
    } else {
      // 4881221: relax the constraints based on JSR202 spec
      legal = verify_unqualified_name(bytes, length, LegalMethod);
    }
  }

  if (!legal) {
    ResourceMark rm(THREAD);
    Exceptions::fthrow(THREAD_AND_LOCATION, vmSymbols::java_lang_ClassFormatError(),
                       "Illegal method name \"%s\" in class %s", bytes, _class_name->as_C_string());
  }
}


// Checks if signature is a legal field signature.
void ClassFileParser::verify_legal_field_signature(symbolHandle name, symbolHandle signature, TRAPS) {
  if (!_need_verify) { return; }

  char* bytes = signature->as_utf8();
  unsigned int length = signature->utf8_length();
  char* p = skip_over_field_signature(bytes, false, length, CHECK);

  if (p == NULL || (p - bytes) != (int)length) {
    ResourceMark rm(THREAD);
    Exceptions::fthrow(THREAD_AND_LOCATION, vmSymbols::java_lang_ClassFormatError(),
                       "Field \"%s\" in class %s has illegal signature \"%s\"", 
                       name->as_C_string(), _class_name->as_C_string(), bytes);
  }
}

// Checks if signature is a legal method signature.
// Returns number of parameters
int ClassFileParser::verify_legal_method_signature(symbolHandle name, symbolHandle signature, TRAPS) {
  if (!_need_verify) { return 0; }

  unsigned int args_size = 0;
  char* p = signature->as_utf8();
  unsigned int length = signature->utf8_length();
  char* nextp;

  // The first character must be a '('
  if ((length > 0) && (*p++ == JVM_SIGNATURE_FUNC)) {
    length--;
    // Skip over legal field signatures
    nextp = skip_over_field_signature(p, false, length, CHECK_0);
    while ((length > 0) && (nextp != NULL)) {
      args_size++;
      if (p[0] == 'J' || p[0] == 'D') {
        args_size++;
      }
      length -= nextp - p;
      p = nextp;
      nextp = skip_over_field_signature(p, false, length, CHECK_0);
    }
    // The first non-signature thing better be a ')'
    if ((length > 0) && (*p++ == JVM_SIGNATURE_ENDFUNC)) {
      length--;
      if (name->utf8_length() > 0 && name->byte_at(0) == '<') {
        // All internal methods must return void
        if ((length == 1) && (p[0] == JVM_SIGNATURE_VOID)) {
          return args_size;
        }
      } else {
        // Now we better just have a return value
        nextp = skip_over_field_signature(p, true, length, CHECK_0);
        if (nextp && ((int)length == (nextp - p))) {
          return args_size;
        }
      }
    }
  }
  // Report error
  ResourceMark rm(THREAD);
  Exceptions::fthrow(THREAD_AND_LOCATION, vmSymbols::java_lang_ClassFormatError(),
                     "Method \"%s\" in class %s has illegal signature \"%s\"", 
                     name->as_C_string(),  _class_name->as_C_string(), p);
  return 0;
}


// Unqualified names may not contain the characters '.', ';', or '/'.
// Method names also may not contain the characters '<' or '>', unless <init> or <clinit>.
// Note that method names may not be <init> or <clinit> in this method.
// Because these names have been checked as special cases before calling this method
// in verify_legal_method_name.
bool ClassFileParser::verify_unqualified_name(char* name, unsigned int length, int type) {
  jchar ch;

  for (char* p = name; p != name + length; ) {
    ch = *p;
    if (ch < 128) {
      p++;
      if (ch == '.' || ch == ';') {
        return false;   // do not permit '.' or ';'
      }
      if (type != LegalClass && ch == '/') {
        return false;   // do not permit '/' unless it's class name
      }
      if (type == LegalMethod && (ch == '<' || ch == '>')) {
        return false;   // do not permit '<' or '>' in method names
      }
    } else {
      char* tmp_p = UTF8::next(p, &ch);
      p = tmp_p;
    }
  }
  return true;
}


// Take pointer to a string. Skip over the longest part of the string that could 
// be taken as a fieldname. Allow '/' if slash_ok is true.
// Return a pointer to just past the fieldname. 
// Return NULL if no fieldname at all was found, or in the case of slash_ok 
// being true, we saw consecutive slashes (meaning we were looking for a 
// qualified path but found something that was badly-formed).
char* ClassFileParser::skip_over_field_name(char* name, bool slash_ok, unsigned int length) {
  char* p;
  jchar ch;                     
  jboolean last_is_slash = false;            
  jboolean not_first_ch = false; 

  for (p = name; p != name + length; not_first_ch = true) {
    char* old_p = p;
    ch = *p;
    if (ch < 128) {
      p++;
      // quick check for ascii
      if ((ch >= 'a' && ch <= 'z') ||
          (ch >= 'A' && ch <= 'Z') ||
          (ch == '_' || ch == '$') ||
          (not_first_ch && ch >= '0' && ch <= '9')) {
        last_is_slash = false;
        continue;
      }
      if (slash_ok && ch == '/') {
        if (last_is_slash) {
          return NULL;  // Don't permit consecutive slashes
        }
        last_is_slash = true;
        continue;
      }
    } else {
      jint unicode_ch;
      char* tmp_p = UTF8::next_character(p, &unicode_ch);
      p = tmp_p;
      last_is_slash = false;
      // Check if ch is Java identifier start or is Java identifier part
      // 4672820: call java.lang.Character methods directly without generating separate tables.
      EXCEPTION_MARK;
      instanceKlassHandle klass (THREAD, SystemDictionary::char_klass());

      // return value
      JavaValue result(T_BOOLEAN);
      // Set up the arguments to isJavaIdentifierStart and isJavaIdentifierPart
      JavaCallArguments args;
      args.push_int(unicode_ch);

      // public static boolean isJavaIdentifierStart(char ch);
      JavaCalls::call_static(&result,
                             klass,
                             vmSymbolHandles::isJavaIdentifierStart_name(), 
                             vmSymbolHandles::int_bool_signature(),
                             &args,
                             THREAD);
         
      if (HAS_PENDING_EXCEPTION) {      
        CLEAR_PENDING_EXCEPTION;
        return 0;
      }
      if (result.get_jboolean()) {
        continue;
      }
        
      if (not_first_ch) {
        // public static boolean isJavaIdentifierPart(char ch);
        JavaCalls::call_static(&result,
                               klass,
                               vmSymbolHandles::isJavaIdentifierPart_name(), 
                               vmSymbolHandles::int_bool_signature(),
                               &args,
                               THREAD);
     
        if (HAS_PENDING_EXCEPTION) {    
          CLEAR_PENDING_EXCEPTION;
          return 0;
        }

        if (result.get_jboolean()) {
          continue;
        }
      }
    }
    return (not_first_ch) ? old_p : NULL;
  }
  return (not_first_ch) ? p : NULL;
}


// Take pointer to a string. Skip over the longest part of the string that could
// be taken as a field signature. Allow "void" if void_ok.
// Return a pointer to just past the signature. 
// Return NULL if no legal signature is found.
char* ClassFileParser::skip_over_field_signature(char* signature, 
                                                 bool void_ok, 
                                                 unsigned int length,
                                                 TRAPS) {
  unsigned int array_dim = 0;
  while (length > 0) {
    switch (signature[0]) {
      case JVM_SIGNATURE_VOID: if (!void_ok) { return NULL; }
      case JVM_SIGNATURE_BOOLEAN:
      case JVM_SIGNATURE_BYTE:
      case JVM_SIGNATURE_CHAR:
      case JVM_SIGNATURE_SHORT:
      case JVM_SIGNATURE_INT:
      case JVM_SIGNATURE_FLOAT:
      case JVM_SIGNATURE_LONG:
      case JVM_SIGNATURE_DOUBLE:
        return signature + 1;
      case JVM_SIGNATURE_CLASS: {
        if (_major_version < JAVA_1_5_VERSION) {
          // Skip over the class name if one is there
          char* p = skip_over_field_name(signature + 1, true, --length);
        
          // The next character better be a semicolon
          if (p && (p - signature) > 1 && p[0] == ';') {
            return p + 1;
          }
        } else {
          // 4900761: For class version > 48, any unicode is allowed in class name.
          length--; 
          signature++; 
          while (length > 0 && signature[0] != ';') {
            if (signature[0] == '.') {
              classfile_parse_error("Class name contains illegal character '.' in descriptor in class file %s", CHECK_0);
            }
            length--; 
            signature++; 
          }            
          if (signature[0] == ';') { return signature + 1; }
        }
            
        return NULL;
      }
      case JVM_SIGNATURE_ARRAY:
        array_dim++;
        if (array_dim > 255) {
          // 4277370: array descriptor is valid only if it represents 255 or fewer dimensions.
          classfile_parse_error("Array type descriptor has more than 255 dimensions in class file %s", CHECK_0);
        }
        // The rest of what's there better be a legal signature
        signature++;
        length--;
        void_ok = false;
        break;

      default:
        return NULL;
    }
  }
  return NULL;
}
