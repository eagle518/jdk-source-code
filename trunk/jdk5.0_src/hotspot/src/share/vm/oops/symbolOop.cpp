#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)symbolOop.cpp	1.20 03/12/23 16:42:08 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_symbolOop.cpp.incl"

//bool symbolOopDesc::equals(char* str, int len) const {
//  int l = utf8_length();
//  if (l != len) return false;
//  while (l-- > 0) {
//    if (str[l] != byte_at(l)) return false;
//  }
//  assert(l == -1, "we should be at the beginning");
//  return true;
//}

bool symbolOopDesc::equals(const char* str, int len) const {
  int l = utf8_length();
  if (l != len) return false;
  while (l-- > 0) {
    if (str[l] != byte_at(l)) return false;
  }
  assert(l == -1, "we should be at the beginning");
  return true;
}

char* symbolOopDesc::as_C_string(char* buf, int size) const {
  if (size > 0) {
    int len = MIN2(size - 1, utf8_length());
    for (int i = 0; i < len; i++) {
      buf[i] = byte_at(i);
    }
    buf[len] = '\0';
  }
  return buf;
}

char* symbolOopDesc::as_C_string() const {
  int len = utf8_length();
  char* str = NEW_RESOURCE_ARRAY(char, len + 1);
  return as_C_string(str, len + 1);
}


void symbolOopDesc::print_symbol_on(outputStream* st) {
  st = st ? st : tty;
  for (int index = 0; index < utf8_length(); index++)
    st->put((char)byte_at(index));
}

jchar* symbolOopDesc::as_unicode(int& length) const {
  symbolOopDesc* this_ptr = (symbolOopDesc*)this;
  length = UTF8::unicode_length((char*)this_ptr->bytes(), utf8_length());
  jchar* result = NEW_RESOURCE_ARRAY(jchar, length);
  if (length > 0) {
    UTF8::convert_to_unicode((char*)this_ptr->bytes(), result, length);
  }
  return result;
}

const char* symbolOopDesc::as_klass_external_name(char* buf, int size) const {
  if (size > 0) {
    char* str    = as_C_string(buf, size);
    int   length = (int)strlen(str);
    // Turn all '/'s into '.'s (also for array klasses)
    for (int index = 0; index < length; index++) {
      if (str[index] == '/') {
        str[index] = '.';
      }
    }
    return str;
  } else {
    return buf;
  }
}

const char* symbolOopDesc::as_klass_external_name() const {
  char* str    = as_C_string();
  int   length = (int)strlen(str);
  // Turn all '/'s into '.'s (also for array klasses)
  for (int index = 0; index < length; index++) {
    if (str[index] == '/') {
      str[index] = '.';
    }
  }
  return str;
}
