#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)bytecodeTracer.cpp	1.41 03/12/23 16:40:34 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_bytecodeTracer.cpp.incl"


#ifndef PRODUCT

// Standard closure for BytecodeTracer: prints the current bytecode
// and its attributes using bytecode-specific information.

class BytecodePrinter: public BytecodeClosure {
 private:
  methodOop _current_method;
  bool      _is_wide;
  address   _next_pc;                // current decoding position

  void      align()                  { _next_pc = (address)round_to((intptr_t)_next_pc, sizeof(jint)); }  
  int       get_byte()               { return *(jbyte*) _next_pc++; }  // signed  
  short     get_short()              { short i=Bytes::get_Java_u2(_next_pc); _next_pc+=2; return i; }
  int       get_int()                { int i=Bytes::get_Java_u4(_next_pc); _next_pc+=4; return i; }                                                   

  int       get_index()              { return *(address)_next_pc++; }
  int       get_big_index()          { int i=Bytes::get_Java_u2(_next_pc); _next_pc+=2; return i; }
  int       get_index_special()      { return (is_wide()) ? get_big_index() : get_index(); }
  methodOop method()                 { return _current_method; }
  bool      is_wide()                { return _is_wide; }


  void      print_constant(int i);
  void      print_attributes(Bytecodes::Code code, int bci);
  void      bytecode_epilog(int bci);

 public:
  BytecodePrinter() {
    _is_wide = false;
  }

  // This method is called while executing the raw bytecodes, so none of
  // the adjustments that BytecodeStream performs applies.
  void trace(methodHandle method, address bcp, uintptr_t tos, uintptr_t tos2) {
    MutexLocker ml(BytecodeTrace_lock);
    ResourceMark rm;
    if (_current_method != method()) {
      // Note 1: This code will not work as expected with true MT/MP.
      //         Need an explicit lock or a different solution.
      tty->cr();
      tty->print("[%d] ", (int) Thread::current()->osthread()->thread_id());
      method->print_name(tty);
      tty->cr();
      _current_method = method();
    }
    Bytecodes::Code code;
    if (is_wide()) {
      // bcp wasn't advanced if previous bytecode was _wide.
      code = Bytecodes::cast(*(bcp+1));
    } else {
      code = Bytecodes::cast(*bcp);
    }
    int bci = bcp - method->code_base();
    tty->print("[%d] ", (int) Thread::current()->osthread()->thread_id());
    if (Verbose) {
      tty->print("%8d  %4d  " INTPTR_FORMAT " " INTPTR_FORMAT " %s", 
	   BytecodeCounter::counter_value(), bci, tos, tos2, Bytecodes::name(code));
    } else {
      tty->print("%8d  %4d  %s", 
	   BytecodeCounter::counter_value(), bci, Bytecodes::name(code));
    }
    _next_pc = is_wide() ? bcp+2 : bcp+1;
    print_attributes(code, bci);
    // Set is_wide for the next one, since the caller of this doesn't skip
    // the next bytecode.
    _is_wide = (code == Bytecodes::_wide);
  }

  // Used for methodOop::print_codes().  The input bcp comes from
  // BytecodeStream, which will skip wide bytecodes.
  void trace(methodHandle method, address bcp) {
    _current_method = method();
    ResourceMark rm;
    Bytecodes::Code code = Bytecodes::cast(*bcp);
    // Set is_wide 
    _is_wide = (code == Bytecodes::_wide);
    if (is_wide()) {
      code = Bytecodes::cast(*(bcp+1));
    }
    int bci = bcp - method->code_base();
    // Print bytecode index and name
    if (is_wide()) {
      tty->print("%d %s_w", bci, Bytecodes::name(code));
    } else {
      tty->print("%d %s", bci, Bytecodes::name(code));
    }
    _next_pc = is_wide() ? bcp+2 : bcp+1;
    print_attributes(code, bci);
    bytecode_epilog(bci);
  }
};


// Implementation of BytecodeTracer

BytecodeClosure* BytecodeTracer::_closure = NULL;

static BytecodePrinter std_closure;
BytecodeClosure* BytecodeTracer::std_closure() {
  return &::std_closure;
}


void BytecodeTracer::trace(methodHandle method, address bcp, uintptr_t tos, uintptr_t tos2 ) {
  if (TraceBytecodes && BytecodeCounter::counter_value() >= TraceBytecodesAt) _closure->trace(method, bcp, tos, tos2);
}

void BytecodeTracer::trace(methodHandle method, address bcp) {
  _closure->trace(method, bcp);
}

void print_oop(oop value) {
  if (value == NULL) {
    tty->print_cr(" NULL");
  } else {
    EXCEPTION_MARK;
    Handle h_value (THREAD, value);
    symbolHandle sym = java_lang_String::as_symbol(h_value, CATCH);
    if (sym->utf8_length() > 32) {
      tty->print_cr(" ....");
    } else {
      sym->print(); tty->cr();
    }
  }
}

void BytecodePrinter::print_constant(int i) {
  constantPoolOop constants = method()->constants();
  constantTag tag = constants->tag_at(i);

  if (tag.is_int()) { 
    tty->print_cr(" " INT32_FORMAT, constants->int_at(i));
  } else if (tag.is_long()) {
    tty->print_cr(" " INT64_FORMAT, constants->long_at(i));
  } else if (tag.is_float()) { 
    tty->print_cr(" %f", constants->float_at(i));
  } else if (tag.is_double()) {
    tty->print_cr(" %f", constants->double_at(i));
  } else if (tag.is_string()) { 
    oop string = constants->resolved_string_at(i);
    print_oop(string);
  } else if (tag.is_unresolved_string()) { 
    tty->print_cr(" <unresolved string at %d>", i);  
  } else if (tag.is_klass()) { 
    tty->print_cr(" %s", constants->resolved_klass_at(i)->klass_part()->external_name());
  } else if (tag.is_unresolved_klass()) { 
    tty->print_cr(" <unresolved klass at %d>", i);  
  } else ShouldNotReachHere();  
}


void BytecodePrinter::print_attributes(Bytecodes::Code code, int bci) {
  // Show attributes of pre-rewritten codes
  code = Bytecodes::java_code(code);
  // If the code doesn't have any fields there's nothing to print.
  // note this is ==1 because the tableswitch and lookupswitch are
  // zero size (for some reason) and we want to print stuff out for them.
  if (Bytecodes::length_for(code) == 1) {
    tty->cr();
    return;
  }

  switch(code) {
    // Java specific bytecodes only matter.
    case Bytecodes::_bipush:
      tty->print_cr(" " INT32_FORMAT, get_byte());
      break;
    case Bytecodes::_sipush: 
      tty->print_cr(" " INT32_FORMAT, get_short());
      break;
    case Bytecodes::_ldc:
      print_constant(get_index());
      break;

    case Bytecodes::_ldc_w:
    case Bytecodes::_ldc2_w:
      print_constant(get_big_index());
      break;

    case Bytecodes::_iload:
    case Bytecodes::_lload:
    case Bytecodes::_fload:
    case Bytecodes::_dload:
    case Bytecodes::_aload:
    case Bytecodes::_istore:
    case Bytecodes::_lstore:
    case Bytecodes::_fstore:
    case Bytecodes::_dstore:
    case Bytecodes::_astore:
      tty->print_cr(" #%d", get_index_special());
      break;

    case Bytecodes::_iinc:
      { int index = get_index_special();
        jint offset = is_wide() ? get_short(): get_byte();    
        tty->print_cr(" #%d " INT32_FORMAT, index, offset);
      }
      break;    

    case Bytecodes::_newarray: {
        BasicType atype = (BasicType)get_index();
        const char* str = type2name(atype);
        if (str == NULL || atype == T_OBJECT || atype == T_ARRAY) {
          assert(false, "Unidentified basic type");
        }
        tty->print_cr(" %s", str);
      }
      break;
    case Bytecodes::_anewarray: {
        int klass_index = get_big_index();
        constantPoolOop constants = method()->constants();
        symbolOop name = constants->klass_name_at(klass_index);
        tty->print_cr(" %s ", name->as_C_string());
      }
      break;
    case Bytecodes::_multianewarray: {
        int klass_index = get_big_index();
        int nof_dims = get_index();
        constantPoolOop constants = method()->constants();
        symbolOop name = constants->klass_name_at(klass_index);
        tty->print_cr(" %s %d", name->as_C_string(), nof_dims);
      }
      break;

    case Bytecodes::_ifeq:
    case Bytecodes::_ifnull:
    case Bytecodes::_iflt:
    case Bytecodes::_ifle:
    case Bytecodes::_ifne:
    case Bytecodes::_ifnonnull:
    case Bytecodes::_ifgt:
    case Bytecodes::_ifge:
    case Bytecodes::_if_icmpeq:
    case Bytecodes::_if_icmpne:
    case Bytecodes::_if_icmplt:
    case Bytecodes::_if_icmpgt:
    case Bytecodes::_if_icmple:
    case Bytecodes::_if_icmpge:
    case Bytecodes::_if_acmpeq:
    case Bytecodes::_if_acmpne:
    case Bytecodes::_goto:
    case Bytecodes::_jsr:
      tty->print_cr(" %d", bci + get_short());
      break;

    case Bytecodes::_goto_w:
    case Bytecodes::_jsr_w:
      tty->print_cr(" %d", bci + get_int());
      break;

    case Bytecodes::_ret: tty->print_cr(" %d", get_index_special()); break;

    case Bytecodes::_tableswitch:
      { align();
        int  default_dest = bci + get_int();
        int  lo           = get_int();
        int  hi           = get_int();
        int  len          = hi - lo + 1;
        jint* dest        = NEW_RESOURCE_ARRAY(jint, len);
        for (int i = 0; i < len; i++) {
          dest[i] = bci + get_int();
        }
        tty->print(" %d " INT32_FORMAT " " INT32_FORMAT " ",
                      default_dest, lo, hi); 
        int first = true;
        for (int ll = lo; ll <= hi; ll++, first = false)  {
          int idx = ll - lo;
          const char *format = first ? " %d:" INT32_FORMAT " (delta: %d)" :
                                       ", %d:" INT32_FORMAT " (delta: %d)";
          tty->print(format, ll, dest[idx], dest[idx]-bci);
        }
        tty->cr();
      }
      break;
    case Bytecodes::_lookupswitch:
    case Bytecodes::_fast_linearswitch:
    case Bytecodes::_fast_binaryswitch:
      { align();
        int  default_dest = bci + get_int();
        int  len          = get_int();
        jint* key         = NEW_RESOURCE_ARRAY(jint, len);
        jint* dest        = NEW_RESOURCE_ARRAY(jint, len);
        for (int i = 0; i < len; i++) {
          key [i] = get_int();
          dest[i] = bci + get_int();
        };
        tty->print(" %d %d ", default_dest, len); 
        bool first = true;
        for (int ll = 0; ll < len; ll++, first = false)  {
          const char *format = first ? " " INT32_FORMAT ":" INT32_FORMAT :
                                       ", " INT32_FORMAT ":" INT32_FORMAT ;
          tty->print(format, key[ll], dest[ll]);
        }
        tty->cr();
      }
      break;

    case Bytecodes::_putfield:
    case Bytecodes::_getfield: {
        int i = get_big_index();
        constantPoolOop constants = method()->constants();
        symbolOop field = constants->name_ref_at(i);
        tty->print_cr(" %d <%s>", i, field->as_C_string()); 
      }
      break;

    case Bytecodes::_putstatic:
    case Bytecodes::_getstatic: {
        // the name would be nice here.
        tty->print_cr(" %d", get_big_index());
      }
      break;

    case Bytecodes::_invokevirtual:
    case Bytecodes::_invokespecial:
    case Bytecodes::_invokestatic:
      { int i = get_big_index();
        constantPoolOop constants = method()->constants();
        symbolOop name = constants->name_ref_at(i);
        symbolOop signature = constants->signature_ref_at(i);
        tty->print_cr(" %d <%s> <%s> ", i, name->as_C_string(), signature->as_C_string()); 
      }
      break;

    case Bytecodes::_invokeinterface:
      { int i = get_big_index();
        int n = get_index();
        get_index();
        constantPoolOop constants = method()->constants();
        symbolOop name = constants->name_ref_at(i);
        symbolOop signature = constants->signature_ref_at(i);
        tty->print_cr(" %d <%s> <%s> %d", i, name->as_C_string(), signature->as_C_string(), n);
      }
      break;

    case Bytecodes::_new:
    case Bytecodes::_checkcast:
    case Bytecodes::_instanceof:
      tty->print_cr(" %d", get_big_index());
      break;

    case Bytecodes::_wide: 
      // length is zero not one, but printed with no more info.
      break;
    
    default:
      ShouldNotReachHere();
      break;
  }
}


void BytecodePrinter::bytecode_epilog(int bci) {
#ifndef CORE
  methodDataOop mdo = method()->method_data();
  if (mdo != NULL) {
    ProfileData* data = mdo->bci_to_data(bci);
    if (data != NULL) {
      tty->print("  %d", mdo->dp_to_di(data->dp()));
      tty->fill_to(6);
      data->print_data_on(tty);
    }
  }
#endif
}
#endif // PRODUCT
