#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)signature.cpp	1.32 03/12/23 16:44:10 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_signature.cpp.incl"


// Implementation of SignatureIterator

// Signature syntax:
//
// Signature  = "(" {Parameter} ")" ReturnType.
// Parameter  = FieldType.
// ReturnType = FieldType | "V".
// FieldType  = "B" | "C" | "D" | "F" | "I" | "J" | "S" | "Z" | "L" ClassName ";" | "[" FieldType.
// ClassName  = string.


SignatureIterator::SignatureIterator(symbolHandle signature) {
  assert(signature->is_symbol(), "not a symbol");
  _signature       = signature;
  _parameter_index = 0;
}

// Overloaded version called without handle
SignatureIterator::SignatureIterator(symbolOop signature) {
  symbolHandle sh(Thread::current(), signature);
  _signature       = sh;
  _parameter_index = 0;
}

SignatureIterator::SignatureIterator(Thread *thread, symbolOop signature) {
  symbolHandle sh(thread, signature);
  _signature       = sh;
  _parameter_index = 0;
}

void SignatureIterator::expect(char c) {
  if (_signature->byte_at(_index) != c) fatal1("expecting %c", c);
  _index++;
}


void SignatureIterator::skip_optional_size() {
  char c = _signature->byte_at(_index);
  while ('0' <= c && c <= '9') c = _signature->byte_at(++_index);
}


int SignatureIterator::parse_type() {
  // Note: This function could be simplified by using "return T_XXX_size;"
  //       instead of the assignment and the break statements. However, it
  //       seems that the product build for win32_i486 with MS VC++ 6.0 doesn't
  //       work (stack underflow for some tests) - this seems to be a VC++ 6.0
  //       compiler bug (was problem - gri 4/27/2000).
  int size = -1;
  switch(_signature->byte_at(_index)) {
    case 'B': do_byte  (); if (_parameter_index < 0 ) _return_type = T_BYTE;
              _index++; size = T_BYTE_size   ; break;
    case 'C': do_char  (); if (_parameter_index < 0 ) _return_type = T_CHAR;    
              _index++; size = T_CHAR_size   ; break;
    case 'D': do_double(); if (_parameter_index < 0 ) _return_type = T_DOUBLE;  
              _index++; size = T_DOUBLE_size ; break;
    case 'F': do_float (); if (_parameter_index < 0 ) _return_type = T_FLOAT;   
              _index++; size = T_FLOAT_size  ; break;
    case 'I': do_int   (); if (_parameter_index < 0 ) _return_type = T_INT;     
              _index++; size = T_INT_size    ; break;
    case 'J': do_long  (); if (_parameter_index < 0 ) _return_type = T_LONG;    
              _index++; size = T_LONG_size   ; break;
    case 'S': do_short (); if (_parameter_index < 0 ) _return_type = T_SHORT;   
              _index++; size = T_SHORT_size  ; break;
    case 'Z': do_bool  (); if (_parameter_index < 0 ) _return_type = T_BOOLEAN; 
              _index++; size = T_BOOLEAN_size; break;
    case 'V': do_void  (); if (_parameter_index < 0 ) _return_type = T_VOID;    
              _index++; size = T_VOID_size;  ; break;
    case 'L':
      { int begin = ++_index;
        while (_signature->byte_at(_index++) != ';') ;
        do_object(begin, _index);
      }
      if (_parameter_index < 0 ) _return_type = T_OBJECT;
      size = T_OBJECT_size;
      break;
    case '[':
      { int begin = ++_index;
        skip_optional_size();
        while (_signature->byte_at(_index) == '[') {
          _index++;
          skip_optional_size();
        }
        if (_signature->byte_at(_index) == 'L') {
          while (_signature->byte_at(_index++) != ';') ;
        } else {
          _index++;
        }
        do_array(begin, _index);
       if (_parameter_index < 0 ) _return_type = T_ARRAY;
      }
      size = T_ARRAY_size;
      break;
    default:
      ShouldNotReachHere();
      break;
  }
  assert(size >= 0, "size must be set");
  return size;
}


void SignatureIterator::check_signature_end() {
  if (_index < _signature->utf8_length()) {
    tty->print_cr("too many chars in signature");
    _signature->print_value_on(tty);
    tty->print_cr(" @ %d", _index);
  }
}


void SignatureIterator::dispatch_field() {
  // no '(', just one (field) type
  _index = 0;
  _parameter_index = 0;
  parse_type();
  check_signature_end();
}


void SignatureIterator::iterate_parameters() {
  // Parse parameters
  _index = 0;
  _parameter_index = 0;
  expect('(');
  while (_signature->byte_at(_index) != ')') _parameter_index += parse_type();
  expect(')');
  _parameter_index = 0;
}

// Optimized version of iterat_parameters when fingerprint is known
void SignatureIterator::iterate_parameters( uint64_t fingerprint ) {
  uint64_t saved_fingerprint = fingerprint;

  // Check for too many arguments
  if ( fingerprint == CONST64(-1) ) { 
    SignatureIterator::iterate_parameters();
    return;
  }

  assert(fingerprint, "Fingerprint should not be 0");

  _parameter_index = 0;
  fingerprint = fingerprint >> (static_feature_size + result_feature_size);
  while ( 1 ) {
    switch ( fingerprint & parameter_feature_mask ) {
      case bool_parm:
 	do_bool();
        _parameter_index += T_BOOLEAN_size;
        break;
      case byte_parm:
 	do_byte();
        _parameter_index += T_BYTE_size;
        break;
      case char_parm:
 	do_char();
        _parameter_index += T_CHAR_size;
        break;
      case short_parm:
 	do_short();
        _parameter_index += T_SHORT_size;
        break;
      case int_parm:
 	do_int();
        _parameter_index += T_INT_size;
        break;
      case obj_parm:
        do_object(0, 0);
        _parameter_index += T_OBJECT_size;
        break;
      case long_parm:
        do_long();
        _parameter_index += T_LONG_size;
        break;
      case float_parm:
        do_float();
        _parameter_index += T_FLOAT_size;
        break;
      case double_parm:
        do_double();
        _parameter_index += T_DOUBLE_size;
        break;
      case done_parm:
        return;
        break;
      default:
        tty->print_cr("*** parameter is %d", fingerprint & parameter_feature_mask);
        tty->print_cr("*** fingerprint is " PTR64_FORMAT, saved_fingerprint);
        ShouldNotReachHere();
        break;
    }
    fingerprint >>= parameter_feature_size;
  }
  _parameter_index = 0;
}


void SignatureIterator::iterate_returntype() {
  // Ignore parameters
  _index = 0;
  expect('(');
  while (_signature->byte_at(_index) != ')') _index++;
  expect(')');
  // Parse return type
  _parameter_index = -1;
  parse_type();
  check_signature_end();
  _parameter_index = 0;
}


void SignatureIterator::iterate() {
  // Parse parameters
  _parameter_index = 0;
  _index = 0;
  expect('(');
  while (_signature->byte_at(_index) != ')') _parameter_index += parse_type();
  expect(')');
  // Parse return type
  _parameter_index = -1;
  parse_type();
  check_signature_end();
  _parameter_index = 0;
}


// Implementation of SignatureStream

SignatureStream::SignatureStream(symbolHandle signature, bool is_method)
: _signature(signature), _at_return_type(false) {
  _begin = _end = (is_method ? 1 : 0);  // skip first '(' in method signatures
  next();
}


bool SignatureStream::is_done() const {
  return _end > _signature()->utf8_length();
}


void SignatureStream::next() {
  if (_end == _signature()->utf8_length()) {
    _end++;
    return;
  }

  _begin = _end;
  switch(_signature()->byte_at(_begin)) {
    case 'B': _type = T_BYTE;    _end++; break;
    case 'C': _type = T_CHAR;    _end++; break;
    case 'D': _type = T_DOUBLE;  _end++; break;
    case 'F': _type = T_FLOAT;   _end++; break;
    case 'I': _type = T_INT;     _end++; break;
    case 'J': _type = T_LONG;    _end++; break;
    case 'S': _type = T_SHORT;   _end++; break;
    case 'Z': _type = T_BOOLEAN; _end++; break;
    case 'V': _type = T_VOID;    _end++; break;
    case 'L': {
      _type = T_OBJECT;
      while (_signature()->byte_at(_end++) != ';');
      break;
    }
    case '[': {
      _type = T_ARRAY;
      char c = _signature->byte_at(_end);
      while ('0' <= c && c <= '9') c = _signature->byte_at(_end++);
      while (_signature->byte_at(_end) == '[') {
        _end++;
        c = _signature->byte_at(_end);
        while ('0' <= c && c <= '9') c = _signature->byte_at(_end++);
      }
      switch(_signature()->byte_at(_end)) {
        case 'B':
        case 'C':
        case 'D':
        case 'F':
        case 'I':
        case 'J':
        case 'S':
        case 'Z':_end++; break;
        default: {
          while (_signature()->byte_at(_end++) != ';');
          break;
        }
      }
      break;
    }
    case ')': _end++; next(); _at_return_type = true; break;
    default : ShouldNotReachHere();
  }
}


bool SignatureStream::is_object() const {
  return _type == T_OBJECT
      || _type == T_ARRAY;
}

bool SignatureStream::is_array() const {
  return _type == T_ARRAY;
}

symbolOop SignatureStream::as_symbol(TRAPS) {
  // Create a symbol from for string _begin _end
  ResourceMark rm(THREAD);
 
  int begin = _begin;
  int end   = _end;

  if (   _signature()->byte_at(_begin) == 'L'
      && _signature()->byte_at(_end-1) == ';') {
    begin++;
    end--;
  }

  char* buffer = NEW_RESOURCE_ARRAY(char, end - begin);
  for (int index = begin; index < end; index++) {
    buffer[index - begin] = _signature()->byte_at(index);
  }
  symbolOop result = oopFactory::new_symbol(buffer, end - begin, CHECK_0);
  return result;
}


symbolOop SignatureStream::as_symbol_or_null() {
  // Create a symbol from for string _begin _end
  ResourceMark rm;
 
  int begin = _begin;
  int end   = _end;

  if (   _signature()->byte_at(_begin) == 'L'
      && _signature()->byte_at(_end-1) == ';') {
    begin++;
    end--;
  }

  char* buffer = NEW_RESOURCE_ARRAY(char, end - begin);
  for (int index = begin; index < end; index++) {
    buffer[index - begin] = _signature()->byte_at(index);
  }
  symbolOop result = SymbolTable::probe(buffer, end - begin);
  return result;
}
