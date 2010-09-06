#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)symbolTable.cpp	1.58 03/12/23 16:41:31 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_symbolTable.cpp.incl"

// --------------------------------------------------------------------------

SymbolTable* SymbolTable::_the_table = NULL;

// Lookup a symbol in a bucket.

symbolOop SymbolTable::lookup(int index, const char* name,
                              int len, unsigned int hash) {
  for (HashtableEntry* e = bucket(index); e != NULL; e = e->next()) {
    if (e->hash() == hash) {
      symbolOop sym = symbolOop(e->literal());
      if (sym->equals(name, len)) {
        return sym;
      }
    }
  }
  return NULL;
}


// We take care not to be blocking while holding the
// SymbolTable_lock. Otherwise, the system might deadlock, since the
// symboltable is used during compilation (VM_thread) The lock free
// synchronization is simplified by the fact that we do not delete
// entries in the symbol table during normal execution (only during
// safepoints).

symbolOop SymbolTable::lookup(const char* name, int len, TRAPS) {  
  unsigned int hashValue = hash_symbol(name, len);
  int index = the_table()->hash_to_index(hashValue);

  symbolOop s = the_table()->lookup(index, name, len, hashValue);

  // Found
  if (s != NULL) return s;
  
  // Otherwise, add to symbol to table
  return the_table()->basic_add(index, (u1*)name, len, hashValue, CHECK_0);
}


// Needed for preloading classes in signatures when compiling.

symbolOop SymbolTable::probe(const char* name, int len) {
  unsigned int hashValue = hash_symbol(name, len);
  int index = the_table()->hash_to_index(hashValue);
  return the_table()->lookup(index, name, len, hashValue);
}


symbolOop SymbolTable::basic_add(int index, u1 *name, int len,
                                 unsigned int hashValue, TRAPS) {  
  assert(!Universe::heap()->is_in_reserved(name) || GC_locker::is_active(),
         "proposed name of symbol must be stable");

  // We assume that lookup() has been called already, that it failed,
  // and symbol was not found.  We create the symbol here.
  symbolKlass* sk  = (symbolKlass*) Universe::symbolKlassObj()->klass_part();
  symbolOop s_oop = sk->allocate_symbol(name, len, CHECK_0);
  symbolHandle sym (THREAD, s_oop);

  // Allocation must be done before grapping the SymbolTable_lock lock
  MutexLocker ml(SymbolTable_lock, THREAD);

  assert(sym->equals((char*)name, len), "symbol must be properly initialized");

  // Since look-up was done lock-free, we need to check if another
  // thread beat us in the race to insert the symbol.

  symbolOop test = lookup(index, (char*)name, len, hashValue);
  if (test != NULL) {
    // A race occured and another thread introduced the symbol, this one
    // will be dropped and collected.
    return test;
  }  

  HashtableEntry* entry = new_entry(hashValue, sym());
  add_entry(index, entry);
  return sym();
}


//-----------------------------------------------------------------------------------------------------
// Non-product code

#ifndef PRODUCT

void SymbolTable::verify() {
  for (int i = 0; i < the_table()->table_size(); ++i) {
    HashtableEntry* p = the_table()->bucket(i);
    for ( ; p != NULL; p = p->next()) {
      symbolOop s = symbolOop(p->literal());
      guarantee(s != NULL, "symbol is NULL");
      s->verify();
      guarantee(s->is_perm(), "symbol not in permspace");
      unsigned int h = hash_symbol((char*)s->bytes(), s->utf8_length());
      guarantee(p->hash() == h, "broken hash in symbol table entry");
      guarantee(the_table()->hash_to_index(h) == i,
                "wrong index in symbol table");
    }
  }
}


void SymbolTable::print_histogram() {
  MutexLocker ml(SymbolTable_lock);
  const int results_length = 100;
  int results[results_length];
  int i,j;
  
  // initialize results to zero
  for (j = 0; j < results_length; j++) {
    results[j] = 0;
  }

  int total = 0;
  int max_symbols = 0;
  int out_of_range = 0;
  for (i = 0; i < the_table()->table_size(); i++) {
    HashtableEntry* p = the_table()->bucket(i);
    for ( ; p != NULL; p = p->next()) {
      int counter = symbolOop(p->literal())->utf8_length();
      total += counter;
      if (counter < results_length) {
        results[counter]++;
      } else {
        out_of_range++;
      }
      max_symbols = MAX2(max_symbols, counter);
    }
  }
  tty->print_cr("Symbol Table:");
  tty->print_cr("%8s %5d", "Total  ", total);
  tty->print_cr("%8s %5d", "Maximum", max_symbols);
  tty->print_cr("%8s %3.2f", "Average",
	  ((float) total / (float) the_table()->table_size()));
  tty->print_cr("%s", "Histogram:");
  tty->print_cr(" %s %29s", "Length", "Number chains that length");
  for (i = 0; i < results_length; i++) {
    if (results[i] > 0) {
      tty->print_cr("%6d %10d", i, results[i]);
    }
  }
  int line_length = 70;    
  tty->print_cr("%s %30s", " Length", "Number chains that length");
  for (i = 0; i < results_length; i++) {
    if (results[i] > 0) {
      tty->print("%4d", i);
      for (j = 0; (j < results[i]) && (j < line_length);  j++) {
        tty->print("%1s", "*");
      }
      if (j == line_length) {
        tty->print("%1s", "+");
      }
      tty->cr();
    }
  }  
  tty->print_cr(" %s %d: %d\n", "Number chains longer than",
	            results_length, out_of_range);
}

#endif // PRODUCT

// --------------------------------------------------------------------------

#ifdef ASSERT
class StableMemoryChecker : public StackObj {
  enum { _bufsize = wordSize*4 };

  address _region;
  jint    _size;
  u1      _save_buf[_bufsize];

  int sample(u1* save_buf) {
    if (_size <= _bufsize) {
      memcpy(save_buf, _region, _size);
      return _size;
    } else {
      // copy head and tail
      memcpy(&save_buf[0],          _region,                      _bufsize/2);
      memcpy(&save_buf[_bufsize/2], _region + _size - _bufsize/2, _bufsize/2);
      return (_bufsize/2)*2;
    }
  }

 public:
  StableMemoryChecker(const void* region, jint size) {
    _region = (address) region;
    _size   = size;
    sample(_save_buf);
  }

  bool verify() {
    u1 check_buf[sizeof(_save_buf)];
    int check_size = sample(check_buf);
    return (0 == memcmp(_save_buf, check_buf, check_size));
  }

  void set_region(const void* region) { _region = (address) region; }
};
#endif


// --------------------------------------------------------------------------


// Compute the hash value for a java.lang.String object which would
// contain the characters passed in. This hash value is used for at
// least two purposes.
//
// (a) As the hash value used by the StringTable for bucket selection
//     and comparison (stored in the HashtableEntry structures).  This
//     is used in the String.intern() method.
//
// (b) As the hash value used by the String object itself, in
//     String.hashCode().  This value is normally calculate in Java code
//     in the String.hashCode method(), but is precomputed for String
//     objects in the shared archive file.
//
//     For this reason, THIS ALGORITHM MUST MATCH String.hashCode().

int StringTable::hash_string(jchar* s, int len) {
  unsigned h = 0;
  while (len-- > 0) {
    h = 31*h + (unsigned) *s;
    s++;
  }
  return h;
}


StringTable* StringTable::_the_table = NULL;

oop StringTable::lookup(int index, jchar* name,
                        int len, unsigned int hash) {
  for (HashtableEntry* l = bucket(index); l != NULL; l = l->next()) {
    if (l->hash() == hash) {
      if (java_lang_String::equals(l->literal(), name, len)) {
        return l->literal();
      }
    }
  }
  return NULL;
}


oop StringTable::basic_add(int index, Handle string_or_null, jchar* name,
                           int len, unsigned int hashValue, TRAPS) {  
  debug_only(StableMemoryChecker smc(name, len * sizeof(name[0])));
  assert(!Universe::heap()->is_in_reserved(name) || GC_locker::is_active(),
         "proposed name of symbol must be stable");

  Handle string;
  // try to reuse the string if possible
  if (!string_or_null.is_null() && string_or_null()->is_perm()) {
    string = string_or_null;
  } else {
    string = java_lang_String::create_tenured_from_unicode(name, len, CHECK_0);
  }

  // Allocation must be done before grapping the SymbolTable_lock lock
  MutexLocker ml(StringTable_lock, THREAD);

  assert(java_lang_String::equals(string(), name, len),
         "string must be properly initialized");

  // Since look-up was done lock-free, we need to check if another
  // thread beat us in the race to insert the symbol.

  oop test = lookup(index, name, len, hashValue); // calls lookup(u1*, int)
  if (test != NULL) {
    // Entry already added
    return test;
  }  

  HashtableEntry* entry = new_entry(hashValue, string());
  add_entry(index, entry);
  return string();
}


oop StringTable::lookup(symbolOop symbol) {
  ResourceMark rm;
  int length;
  jchar* chars = symbol->as_unicode(length);
  unsigned int hashValue = hash_string(chars, length);
  int index = the_table()->hash_to_index(hashValue);
  return the_table()->lookup(index, chars, length, hashValue);
}


oop StringTable::intern(Handle string_or_null, jchar* name,
                        int len, TRAPS) {
  unsigned int hashValue = hash_string(name, len);
  int index = the_table()->hash_to_index(hashValue);
  oop string = the_table()->lookup(index, name, len, hashValue);

  // Found
  if (string != NULL) return string;
  
  // Otherwise, add to symbol to table
  return the_table()->basic_add(index, string_or_null, name, len,
                                hashValue, CHECK_0);  
}

oop StringTable::intern(symbolOop symbol, TRAPS) {
  if (symbol == NULL) return NULL;
  ResourceMark rm(THREAD);
  int length;
  jchar* chars = symbol->as_unicode(length);
  Handle string;
  oop result = intern(string, chars, length, CHECK_0);
  return result;
}


oop StringTable::intern(oop string, TRAPS)
{
  if (string == NULL) return NULL;
  ResourceMark rm(THREAD);
  int length;
  Handle h_string (THREAD, string);
  jchar* chars = java_lang_String::as_unicode_string(string, length);
  oop result = intern(h_string, chars, length, CHECK_0);
  return result;
}


oop StringTable::intern(const char* utf8_string, TRAPS) {
  if (utf8_string == NULL) return NULL;
  ResourceMark rm(THREAD);
  int length = UTF8::unicode_length(utf8_string);
  jchar* chars = NEW_RESOURCE_ARRAY(jchar, length);
  UTF8::convert_to_unicode(utf8_string, chars, length);
  Handle string;
  oop result = intern(string, chars, length, CHECK_0);
  return result;
}

//-----------------------------------------------------------------------------------------------------
// Non-product code
#ifndef PRODUCT


void StringTable::verify() {
  for (int i = 0; i < the_table()->table_size(); ++i) {
    HashtableEntry* p = the_table()->bucket(i);
    for ( ; p != NULL; p = p->next()) {
      oop s = p->literal();
      guarantee(s != NULL, "interned string is NULL");
      guarantee(s->is_perm(), "interned string not in permspace");

      int length;
      jchar* chars = java_lang_String::as_unicode_string(s, length);
      unsigned int h = hash_string(chars, length);
      guarantee(p->hash() == h, "broken hash in string table entry");
      guarantee(the_table()->hash_to_index(h) == i,
                "wrong index in string table");
    }
  }
}


#endif // PRODUCT
