#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)compilerOracle.cpp	1.23 03/12/23 16:40:03 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_compilerOracle.cpp.incl"

class MethodMatcher : public CHeapObj {
 private:
  jobject        _class_name;
  jobject        _method_name;
  MethodMatcher* _next;
 public:
  MethodMatcher(symbolHandle class_name, symbolHandle method_name, MethodMatcher* next);
  bool match(methodHandle method);
};

MethodMatcher::MethodMatcher(symbolHandle class_name, symbolHandle method_name, MethodMatcher* next) {
  _class_name  = JNIHandles::make_global(class_name,  false);
  _method_name = JNIHandles::make_global(method_name, false);
  _next        = next;
}


bool MethodMatcher::match(methodHandle method) {
  symbolOop class_name  = Klass::cast(method->method_holder())->name();
  symbolOop method_name = method->name();
  for (MethodMatcher* current = this; current != NULL; current = current->_next) {
    if ( JNIHandles::resolve(current->_class_name)  == class_name
      && JNIHandles::resolve(current->_method_name) == method_name) {
      return true;
    }
  }
  return false;
}

static MethodMatcher* should_exclude_list  = NULL;
static MethodMatcher* should_inline_list  = NULL;
static MethodMatcher* should_break_at_list = NULL;
static MethodMatcher* should_print_list = NULL;
static MethodMatcher* should_log_list = NULL;

void CompilerOracle::add_should_break_at(symbolHandle class_name, symbolHandle method_name) {
  should_break_at_list = new MethodMatcher(class_name, method_name, should_break_at_list);
}

void CompilerOracle::add_should_exclude(symbolHandle class_name, symbolHandle method_name) {
  should_exclude_list = new MethodMatcher(class_name, method_name, should_exclude_list);
}

void CompilerOracle::add_should_inline(symbolHandle class_name, symbolHandle method_name) {
  should_inline_list = new MethodMatcher(class_name, method_name, should_inline_list);
}

void CompilerOracle::add_should_print(symbolHandle class_name, symbolHandle method_name) {
  should_print_list = new MethodMatcher(class_name, method_name, should_print_list);
}

void CompilerOracle::add_should_log(symbolHandle class_name, symbolHandle method_name) {
  if (!LogCompilation && should_log_list == NULL)
    tty->print_cr("Warning:  +LogCompilation must be enabled in order for individual methods to be logged.");
  should_log_list = new MethodMatcher(class_name, method_name, should_log_list);
}

bool CompilerOracle::should_exclude(methodHandle method) {
  return (should_exclude_list != NULL)
      && should_exclude_list->match(method);
}

bool CompilerOracle::should_inline(methodHandle method) {
  return (should_inline_list != NULL)
      && should_inline_list->match(method);
}

bool CompilerOracle::should_print(methodHandle method) {
  return (should_print_list != NULL) && (method() != NULL)
      && should_print_list->match(method);
}

bool CompilerOracle::should_log(methodHandle method) {
  if (!LogCompilation)          return false;
  if (should_log_list == NULL)  return true;  // by default, log all
  return (method() != NULL) && should_log_list->match(method);
}

bool CompilerOracle::should_break_at(methodHandle method) {
  return (should_break_at_list != NULL)
      && should_break_at_list->match(method);
}

void CompilerOracle::read_from_line(char* line) {
  if (line[0] == '\0') return;
  if (line[0] == '#')  return;

  for (char* lp = line; *lp != '\0'; lp++) {
    // Allow '.' to separate the class name from the method name.
    // This is the preferred spelling of methods:
    //	    exclude java/lang/String.indexOf(I)I
    // Allow ',' for spaces (eases command line quoting).
    //	    exclude,java/lang/String.indexOf
    // For backward compatibility, allow space as separator also.
    //	    exclude java/lang/String indexOf
    //	    exclude,java/lang/String,indexOf
    // For simple implementation convenience here, convert them all to space.
    if (*lp == ',' || *lp == '.')  *lp = ' ';
  }

  // The characters allowed in a class or method name.  All characters > 0x7f
  // are allowed in order to handle obfuscated class files (e.g. Volano)
  #define RANGE "[abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789/$_<>" \
	"\x80\x81\x82\x83\x84\x85\x86\x87\x88\x89\x8a\x8b\x8c\x8d\x8e\x8f" \
	"\x90\x91\x92\x93\x94\x95\x96\x97\x98\x99\x9a\x9b\x9c\x9d\x9e\x9f" \
	"\xa0\xa1\xa2\xa3\xa4\xa5\xa6\xa7\xa8\xa9\xaa\xab\xac\xad\xae\xaf" \
	"\xb0\xb1\xb2\xb3\xb4\xb5\xb6\xb7\xb8\xb9\xba\xbb\xbc\xbd\xbe\xbf" \
	"\xc0\xc1\xc2\xc3\xc4\xc5\xc6\xc7\xc8\xc9\xca\xcb\xcc\xcd\xce\xcf" \
	"\xd0\xd1\xd2\xd3\xd4\xd5\xd6\xd7\xd8\xd9\xda\xdb\xdc\xdd\xde\xdf" \
	"\xe0\xe1\xe2\xe3\xe4\xe5\xe6\xe7\xe8\xe9\xea\xeb\xec\xed\xee\xef" \
	"\xf0\xf1\xf2\xf3\xf4\xf5\xf6\xf7\xf8\xf9\xfa\xfb\xfc\xfd\xfe\xff]"

  char class_name[256];
  char method_name[256];
  int result = sscanf(line, "exclude %" RANGE " %"  RANGE, class_name, method_name);
  if (result == 2) {
    EXCEPTION_MARK;
    symbolHandle c_name = oopFactory::new_symbol_handle(class_name, CATCH);
    symbolHandle m_name = oopFactory::new_symbol_handle(method_name, CATCH);
    add_should_exclude(c_name, m_name);
    tty->print_cr("CompilerOracle: exclude %s %s", class_name, method_name);
    return;
  }

  if (sscanf(line, "break %" RANGE " %"  RANGE, class_name, method_name) == 2) {
    EXCEPTION_MARK;
    symbolHandle c_name = oopFactory::new_symbol_handle(class_name, CATCH);
    symbolHandle m_name = oopFactory::new_symbol_handle(method_name, CATCH);
    add_should_break_at(c_name, m_name);
    tty->print_cr("CompilerOracle: break %s %s", class_name, method_name);
    return;
  }

  if (sscanf(line, "inline %" RANGE " %"  RANGE, class_name, method_name) == 2) {
    EXCEPTION_MARK;
    symbolHandle c_name = oopFactory::new_symbol_handle(class_name, CATCH);
    symbolHandle m_name = oopFactory::new_symbol_handle(method_name, CATCH);
    add_should_inline(c_name, m_name);
    tty->print_cr("CompilerOracle: inline %s %s", class_name, method_name);
    return;
  }

  if (sscanf(line, "print %" RANGE " %"  RANGE, class_name, method_name) == 2) {
    EXCEPTION_MARK;
    symbolHandle c_name = oopFactory::new_symbol_handle(class_name, CATCH);
    symbolHandle m_name = oopFactory::new_symbol_handle(method_name, CATCH);
    add_should_print(c_name, m_name);
    tty->print_cr("CompilerOracle: print %s %s", class_name, method_name);
    return;
  }

  if (sscanf(line, "log %" RANGE " %"  RANGE, class_name, method_name) == 2) {
    EXCEPTION_MARK;
    symbolHandle c_name = oopFactory::new_symbol_handle(class_name, CATCH);
    symbolHandle m_name = oopFactory::new_symbol_handle(method_name, CATCH);
    add_should_log(c_name, m_name);
    tty->print_cr("CompilerOracle: log %s %s", class_name, method_name);
    return;
  }

  tty->print_cr("CompilerOracle: unrecognized line");
  tty->print_cr("\"%s\"", line);
}

static const char* cc_file() {
  if (CompileCommandFile[0] == '\0')
    return ".hotspot_compiler";
  return CompileCommandFile;
}

void CompilerOracle::parse_from_file() {
  FILE* stream = fopen(cc_file(), "rt");
  if (stream == NULL) return;

  char token[1024];
  int  pos = 0;
  int  c = getc(stream);
  while(c != EOF) {
    if (c == '\n') {
      token[pos++] = '\0';
      read_from_line(token);
      pos = 0;
    } else {
      token[pos++] = c;
    }
    c = getc(stream);
  }   
  token[pos++] = '\0';
  read_from_line(token);

  fclose(stream);
}

void CompilerOracle::parse_from_string(const char* str) {
  char token[1024];
  int  pos = 0;
  const char* sp = str;
  int  c = *sp++;
  while (c != '\0') {
    if (c == '\n') {
      token[pos++] = '\0';
      read_from_line(token);
      pos = 0;
    } else {
      token[pos++] = c;
    }
    c = *sp++;
  }
  token[pos++] = '\0';
  read_from_line(token);
}

void CompilerOracle::append_comment_to_file(const char* message) {
  fileStream stream(fopen(cc_file(), "at"));
  stream.print("# ");
  for (int index = 0; message[index] != '\0'; index++) {
    stream.put(message[index]);
    if (message[index] == '\n') stream.print("# ");
  }
  stream.cr();
}

void CompilerOracle::append_exclude_to_file(methodHandle method) {
  fileStream stream(fopen(cc_file(), "at"));
  stream.print("exclude ");
  Klass::cast(method->method_holder())->name()->print_symbol_on(&stream);
  stream.print(".");
  method->name()->print_symbol_on(&stream);
  method->signature()->print_symbol_on(&stream);
  stream.cr();
  stream.cr();
}


void compilerOracle_init() {
  CompilerOracle::parse_from_string(CompileCommand);
  CompilerOracle::parse_from_file();
}


