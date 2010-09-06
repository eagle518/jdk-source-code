#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)fieldType.hpp	1.22 03/12/23 16:43:39 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Note: FieldType should be based on the SignatureIterator (or vice versa).
//       In any case, this structure should be re-thought at some point.

// A FieldType is used to determine the type of a field from a signature string.

class FieldType: public AllStatic {
 private:  
  static void skip_optional_size(symbolOop signature, int* index);
  static bool is_valid_array_signature(symbolOop signature);
 public:
 
  // Return basic type
  static BasicType basic_type(symbolOop signature);  

  // Testing
  static bool is_array(symbolOop signature) { return signature->utf8_length() > 1 && signature->byte_at(0) == '[' && is_valid_array_signature(signature); }

  static bool is_obj(symbolOop signature) {
     int sig_length = signature->utf8_length();
     // Must start with 'L' and end with ';'
     return (sig_length >= 2 &&
             (signature->byte_at(0) == 'L') &&
             (signature->byte_at(sig_length - 1) == ';'));
  }

  // Parse field and extract array information. Works for T_ARRAY only.  
  static BasicType get_array_info(symbolOop signature, jint* dimension, symbolOop *object_key, TRAPS);
};

