#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)fieldType.cpp	1.21 03/12/23 16:43:38 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_fieldType.cpp.incl"

void FieldType::skip_optional_size(symbolOop signature, int* index) {
  jchar c = signature->byte_at(*index);
  while (c >= '0' && c <= '9') {
    *index = *index + 1;
    c = signature->byte_at(*index);
  }
}

BasicType FieldType::basic_type(symbolOop signature) {
  return char2type(signature->byte_at(0));
}

// Check if it is a valid array signature
bool FieldType::is_valid_array_signature(symbolOop sig) {
  assert(sig->utf8_length() > 1, "this should already have been checked");
  assert(sig->byte_at(0) == '[', "this should already have been checked");
  // The first character is already checked
  int i = 1; 
  int len = sig->utf8_length();   
  // First skip all '['s
  while(i < len - 1 && sig->byte_at(i) == '[') i++;
  
  // Check type
  switch(sig->byte_at(i)) {
    case 'B': // T_BYTE
    case 'C': // T_CHAR
    case 'D': // T_DOUBLE
    case 'F': // T_FLOAT
    case 'I': // T_INT
    case 'J': // T_LONG
    case 'S': // T_SHORT
    case 'Z': // T_BOOLEAN 
      // If it is an array, the type is the last character
      return (i + 1 == len);
    case 'L':
      // If it is an object, the last character must be a ';'
      return sig->byte_at(len - 1) == ';';
  }
  
  return false;
}
  

BasicType FieldType::get_array_info(symbolOop signature, jint* dimension, symbolOop* object_key, TRAPS) {
  assert(basic_type(signature) == T_ARRAY, "must be array");  
  int index = 1;
  int dim   = 1;
  skip_optional_size(signature, &index);
  while (signature->byte_at(index) == '[') {
    index++;
    dim++;
    skip_optional_size(signature, &index);
  }
  ResourceMark rm;
  symbolOop element = oopFactory::new_symbol(signature->as_C_string() + index, CHECK_(T_BYTE));  
  BasicType element_type = FieldType::basic_type(element);
  if (element_type == T_OBJECT) {
    char* object_type = element->as_C_string();
    object_type[element->utf8_length() - 1] = '\0';
    *object_key = oopFactory::new_symbol(object_type + 1, CHECK_(T_BYTE));                   
  }
  // Pass dimension back to caller
  *dimension = dim;
  return element_type;
}

