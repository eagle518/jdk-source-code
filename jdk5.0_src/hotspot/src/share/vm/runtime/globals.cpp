#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)globals.cpp	1.35 03/12/23 16:43:42 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_globals.cpp.incl"


RUNTIME_FLAGS(MATERIALIZE_DEVELOPER_FLAG, MATERIALIZE_PD_DEVELOPER_FLAG, MATERIALIZE_PRODUCT_FLAG, MATERIALIZE_PD_PRODUCT_FLAG, MATERIALIZE_DIAGNOSTIC_FLAG)

bool Flag::is_unlocker() const {
  return strcmp(name, "UnlockDiagnosticVMOptions") == 0;
}

bool Flag::is_unlocked() const {
  if (strcmp(kind, "{diagnostic}") == 0) {
    return UnlockDiagnosticVMOptions;
  } else {
    return true;
  }
}

// Length of format string (e.g. "%.1234s") for printing ccstr below
#define FORMAT_BUFFER_LEN 16

void Flag::print_on(outputStream* st) {
  st->print("%5s %-35s %c= ", type, name, (was_set ? ':' : ' '));
  if (is_bool())  st->print("%-16s", get_bool() ? "true" : "false");
  if (is_intx())  st->print("%-16ld", get_intx());
  if (is_uintx()) st->print("%-16lu", get_uintx());
  if (is_ccstr()) {
    const char* cp = get_ccstr();
    const char* eol;
    while ((eol = strchr(cp, '\n')) != NULL) {
      char format_buffer[FORMAT_BUFFER_LEN];
      size_t llen = pointer_delta(eol, cp, sizeof(char));
      jio_snprintf(format_buffer, FORMAT_BUFFER_LEN, 
		   "%%." SIZE_FORMAT "s", llen);
      st->print(format_buffer, cp);
      st->cr();
      cp = eol+1;
      st->print("%5s %-35s += ", "", name);
    }
    st->print("%-16s", cp);
  }
  st->print(" %s", kind);
  st->cr();
}

void Flag::print_as_flag(outputStream* st) {
  if (is_bool()) {
    st->print("-XX:%s%s", get_bool() ? "+" : "-", name);
  } else if (is_intx()) {
    st->print("-XX:%s=" INTX_FORMAT, name, get_intx());
  } else if (is_uintx()) {
    st->print("-XX:%s=" UINTX_FORMAT, name, get_uintx());
  } else if (is_ccstr()) {
    st->print("-XX:%s=", name);
    // Need to turn embedded '\n's back into separate arguments
    // Not so efficient to print one character at a time, 
    // but the choice is to do the transformation to a buffer 
    // and print that.  And this need not be efficient.
    for (const char* cp = get_ccstr(); *cp != '\0'; cp += 1) {
      switch (*cp) {
      default:
        st->print("%c", *cp);
        break;
      case '\n': 
        st->print(" -XX:%s=", name);
        break;
      }
    }
  } else {
    ShouldNotReachHere();
  }
}

#define RUNTIME_PRODUCT_FLAG_STRUCT(type, name, value, doc) { #type, XSTR(name), &name, "{product}" },
#define RUNTIME_PD_PRODUCT_FLAG_STRUCT(type, name, doc)     { #type, XSTR(name), &name, "{pd product}" },
#define RUNTIME_DIAGNOSTIC_FLAG_STRUCT(type, name, value, doc) { #type, XSTR(name), &name, "{diagnostic}" },
#ifdef PRODUCT
  #define RUNTIME_DEVELOP_FLAG_STRUCT(type, name, value, doc) /* flag is constant */ 
  #define RUNTIME_PD_DEVELOP_FLAG_STRUCT(type, name, doc)     /* flag is constant */ 
#else
  #define RUNTIME_DEVELOP_FLAG_STRUCT(type, name, value, doc) { #type, XSTR(name), &name, "" },
  #define RUNTIME_PD_DEVELOP_FLAG_STRUCT(type, name, doc)     { #type, XSTR(name), &name, "{pd}" }, 
#endif

#define C1_PRODUCT_FLAG_STRUCT(type, name, value, doc) { #type, XSTR(name), &name, "{C1 product}" },
#define C1_PD_PRODUCT_FLAG_STRUCT(type, name, doc)     { #type, XSTR(name), &name, "{C1 pd product}" },
#ifdef PRODUCT
  #define C1_DEVELOP_FLAG_STRUCT(type, name, value, doc) /* flag is constant */ 
  #define C1_PD_DEVELOP_FLAG_STRUCT(type, name, doc)     /* flag is constant */ 
#else
  #define C1_DEVELOP_FLAG_STRUCT(type, name, value, doc) { #type, XSTR(name), &name, "{C1}" },
  #define C1_PD_DEVELOP_FLAG_STRUCT(type, name, doc)     { #type, XSTR(name), &name, "{C1 pd}" }, 
#endif


#define C2_PRODUCT_FLAG_STRUCT(type, name, value, doc) { #type, XSTR(name), &name, "{C2 product}" },
#define C2_PD_PRODUCT_FLAG_STRUCT(type, name, doc)     { #type, XSTR(name), &name, "{C2 pd product}" },
#ifdef PRODUCT
  #define C2_DEVELOP_FLAG_STRUCT(type, name, value, doc) /* flag is constant */ 
  #define C2_PD_DEVELOP_FLAG_STRUCT(type, name, doc)     /* flag is constant */ 
#else
  #define C2_DEVELOP_FLAG_STRUCT(type, name, value, doc) { #type, XSTR(name), &name, "{C2}" },
  #define C2_PD_DEVELOP_FLAG_STRUCT(type, name, doc)     { #type, XSTR(name), &name, "{C2 pd}" }, 
#endif


static Flag flagTable[] = {
 RUNTIME_FLAGS(RUNTIME_DEVELOP_FLAG_STRUCT, RUNTIME_PD_DEVELOP_FLAG_STRUCT, RUNTIME_PRODUCT_FLAG_STRUCT, RUNTIME_PD_PRODUCT_FLAG_STRUCT, RUNTIME_DIAGNOSTIC_FLAG_STRUCT)
#ifdef COMPILER1
 C1_FLAGS(C1_DEVELOP_FLAG_STRUCT, C1_PD_DEVELOP_FLAG_STRUCT, C1_PRODUCT_FLAG_STRUCT, C1_PD_PRODUCT_FLAG_STRUCT)
#endif
#ifdef COMPILER2
 C2_FLAGS(C2_DEVELOP_FLAG_STRUCT, C2_PD_DEVELOP_FLAG_STRUCT, C2_PRODUCT_FLAG_STRUCT, C2_PD_PRODUCT_FLAG_STRUCT)
#endif
 {0, NULL, NULL}
};

Flag* Flag::flags = flagTable;
size_t Flag::numFlags = (sizeof(flagTable) / sizeof(Flag));

inline bool str_equal(const char* s, char* q, size_t len) {
  // s is null terminated, q is not!
  if (strlen(s) != (unsigned int) len) return false;
  return strncmp(s, q, len) == 0;
}

Flag* Flag::find_flag(char* name, size_t length) {
  for (Flag* current = &flagTable[0]; current->name; current++) {
    if (str_equal(current->name, name, length)) {
      if (!(current->is_unlocked() || current->is_unlocker())) {
	// disable use of diagnostic flags until they are unlocked
	return NULL;
      }
      return current;
    }
  }
  return NULL;
}

// Returns the address of the index'th element
static Flag* address_of_flag(CommandLineFlagWithType flag) {
  assert((size_t)flag < Flag::numFlags, "bad command line flag index");
  return &Flag::flags[flag];
}

bool CommandLineFlagsEx::is_default(CommandLineFlag flag) {
  assert((size_t)flag < Flag::numFlags, "bad command line flag index");
  Flag* f = &Flag::flags[flag];
  return !f->was_set;
}

bool CommandLineFlags::wasSetOnCmdline(const char* name, bool* value) {
  Flag* result = Flag::find_flag((char*)name, strlen(name));
  if (result == NULL) return false;
  *value = result->was_set;
  return true;
}

bool CommandLineFlags::boolAt(char* name, size_t len, bool* value) {
  Flag* result = Flag::find_flag(name, len);
  if (result == NULL) return NULL;
  if (!result->is_bool()) return NULL;   
  *value = result->get_bool();
  return true;
}

bool CommandLineFlags::boolAtPut(char* name, size_t len, bool* value) {
  Flag* result = Flag::find_flag(name, len);
  if (result == NULL) return NULL;
  if (!result->is_bool()) return NULL;   
  bool old_value = result->get_bool();
  result->set_bool(*value); 
  *value = old_value;
  result->was_set = true;
  return true;
}

void CommandLineFlagsEx::boolAtPut(CommandLineFlagWithType flag, bool value) {
  Flag* faddr = address_of_flag(flag);
  guarantee(faddr != NULL && faddr->is_bool(), "wrong flag type");
  faddr->set_bool(value); 
  faddr->was_set = true;
}

bool CommandLineFlags::intxAt(char* name, size_t len, intx* value) {
  Flag* result = Flag::find_flag(name, len);
  if (result == NULL) return NULL;
  if (!result->is_intx()) return NULL;   
  *value = result->get_intx();
  return true;
}

bool CommandLineFlags::intxAtPut(char* name, size_t len, intx* value) {
  Flag* result = Flag::find_flag(name, len);
  if (result == NULL) return NULL;
  if (!result->is_intx()) return NULL;   
  intx old_value = result->get_intx();
  result->set_intx(*value); 
  *value = old_value;
  result->was_set = true;
  return true;
}

void CommandLineFlagsEx::intxAtPut(CommandLineFlagWithType flag, intx value) {
  Flag* faddr = address_of_flag(flag);
  guarantee(faddr != NULL && faddr->is_intx(), "wrong flag type");
  faddr->set_intx(value); 
  faddr->was_set = true;
}

bool CommandLineFlags::uintxAt(char* name, size_t len, uintx* value) {
  Flag* result = Flag::find_flag(name, len);
  if (result == NULL) return false;
  if (!result->is_uintx()) return false;
  *value = result->get_uintx();
  return true;
}

bool CommandLineFlags::uintxAtPut(char* name, size_t len, uintx* value) {
  Flag* result = Flag::find_flag(name, len);
  if (result == NULL) return false;
  if (!result->is_uintx()) return false;
  uintx old_value = result->get_uintx();
  result->set_uintx(*value);
  *value = old_value;
  result->was_set = true;
  return true;
}

void CommandLineFlagsEx::uintxAtPut(CommandLineFlagWithType flag, uintx value) {
  Flag* faddr = address_of_flag(flag);
  guarantee(faddr != NULL && faddr->is_uintx(), "wrong flag type");
  faddr->set_uintx(value);
  faddr->was_set = true;
}

bool CommandLineFlags::ccstrAt(char* name, size_t len, ccstr* value) {
  Flag* result = Flag::find_flag(name, len);
  if (result == NULL) return false;
  if (!result->is_ccstr()) return false;
  *value = result->get_ccstr();
  return true;
}

// Contract:  Flag will make private copy of the incoming value.
// Outgoing value is always malloc-ed, and caller MUST call free.
bool CommandLineFlags::ccstrAtPut(char* name, size_t len, ccstr* value) {
  Flag* result = Flag::find_flag(name, len);
  if (result == NULL) return false;
  if (!result->is_ccstr()) return false;
  ccstr old_value = result->get_ccstr();
  char* new_value = NEW_C_HEAP_ARRAY(char, strlen(*value)+1);
  strcpy(new_value, *value);
  result->set_ccstr(new_value);
  if (!result->was_set) {
    // Prior value is NOT heap allocated, but was a literal constant.
    char* old_value_to_free = NEW_C_HEAP_ARRAY(char, strlen(old_value)+1);
    strcpy(old_value_to_free, old_value);
    old_value = old_value_to_free;
  }
  *value = old_value;
  result->was_set = true;
  return true;
}

// Contract:  Flag will make private copy of the incoming value.
void CommandLineFlagsEx::ccstrAtPut(CommandLineFlagWithType flag, ccstr value) {
  Flag* faddr = address_of_flag(flag);
  guarantee(faddr != NULL && faddr->is_ccstr(), "wrong flag type");
  ccstr old_value = faddr->get_ccstr();
  char* new_value = NEW_C_HEAP_ARRAY(char, strlen(value)+1);
  strcpy(new_value, value);
  faddr->set_ccstr(new_value);
  if (faddr->was_set) {
    // Prior value is heap allocated so free it.
    FREE_C_HEAP_ARRAY(char, old_value);
  }
  faddr->was_set = true;
}

extern "C" {
  static int compare_flags(const void* void_a, const void* void_b) {
    return strcmp((*((Flag**) void_a))->name, (*((Flag**) void_b))->name);
  }
}

void CommandLineFlags::printSetFlags() {
  // Print which flags were set on the command line
  // note: this method is called before the thread structure is in place
  //       which means resource allocation cannot be used.

  // Compute size
  int length= 0;
  while (flagTable[length].name != NULL) length++;

  // Sort
  Flag** array = NEW_C_HEAP_ARRAY(Flag*, length);
  for (int index = 0; index < length; index++) {
    array[index] = &flagTable[index];
  }
  qsort(array, length, sizeof(Flag*), compare_flags);

  // Print
  for (int i = 0; i < length; i++) {
    if (array[i]->was_set /* naked field! */) {
      array[i]->print_as_flag(tty);
      tty->print(" ");
    }
  }
  tty->cr();
  FREE_C_HEAP_ARRAY(Flag*, array);
}

#ifndef PRODUCT


void CommandLineFlags::verify() {
  assert(Arguments::check_vm_args_consistency(), "Some flag settings conflict");
}

void CommandLineFlags::printFlags() {
  // Print the flags sorted by name
  // note: this method is called before the thread structure is in place
  //       which means resource allocation cannot be used.

  // Compute size
  int length= 0;
  while (flagTable[length].name != NULL) length++;

  // Sort
  Flag** array = NEW_C_HEAP_ARRAY(Flag*, length);
  for (int index = 0; index < length; index++) {
    array[index] = &flagTable[index];
  }
  qsort(array, length, sizeof(Flag*), compare_flags);

  // Print
  tty->print_cr("[Global flags]");
  for (int i = 0; i < length; i++) {
    if (array[i]->is_unlocked()) {
      array[i]->print_on(tty);
    }
  }
  FREE_C_HEAP_ARRAY(Flag*, array);
}

#endif

