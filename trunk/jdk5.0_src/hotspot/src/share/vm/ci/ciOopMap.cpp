#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)ciOopMap.cpp	1.17 04/03/29 14:12:28 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

#include "incls/_precompiled.incl"
#include "incls/_ciOopMap.cpp.incl"

#ifdef COMPILER1
// Implementation of ciOopMap

ciLocalMap::ciLocalMap(Arena* arena, int max_nof_locals, int code_size, int nof_gc_points)
: _arena(arena)
, _max_nof_locals(max_nof_locals)
, _code_size(code_size)
, _nof_gc_points(nof_gc_points)
{
  _nof_initialize = 0;
  _locals         = NULL;
  _nof_entries    = -1; // will be set later

  _bci_for_index = NULL;
  _index_for_bci = NULL;
  _map = NULL;
  _mask_size = max_nof_locals * bits_per_entry;
  _mask_words = (_mask_size + BitsPerInt - 1) / BitsPerInt;

  // One big block for all masks
  if (nof_gc_points > 0 ) {
    if (_mask_words > 0 ) {
      _map = (unsigned int*)arena->Amalloc(sizeof(unsigned int) * _mask_words * nof_gc_points);
    }
    _bci_for_index = (unsigned int*)arena->Amalloc(sizeof(unsigned int) * _nof_gc_points);
    _index_for_bci = (unsigned int*)arena->Amalloc(sizeof(unsigned int) * _code_size);
    for (int i = 0; i < _code_size; i++) {
      _index_for_bci[i] = -1;
    }
  }
}


int ciLocalMap::bci_for_index(int map_index) const {
  assert(0 <= map_index && map_index < _nof_gc_points, "");
  assert(_bci_for_index != NULL, "");
  return _bci_for_index[map_index];
}


int ciLocalMap::index_for_bci(int bci) const {
  assert(_index_for_bci != NULL, "");
  int new_bci = bci == SynchronizationEntryBCI ? 0 : bci;
  return _index_for_bci[new_bci];
}


void ciLocalMap::set_bci_for_index(int map_index, int bci) {
  assert(0 <= map_index && map_index < _nof_gc_points, "");
  _bci_for_index[map_index] = bci;
  _index_for_bci[bci] = map_index;
}


#ifndef PRODUCT

void ciLocalMap::print() {
  if (nof_locals_to_initialize() > 0) {
    tty->print("Locals to initialize: [");
    for (int i = 0; i < nof_locals_to_initialize(); i++) {
      tty->print(" %d", local_to_initialize(i));
    }
    tty->print_cr(" ]");
  }
  tty->print_cr("Oop Map:");
  for (int i = 0; i < _nof_gc_points; i++) {
    tty->print("  @%d: [", bci_for_index(i));
    for (int l = 0; l < _max_nof_locals; l++) {
      if (is_oop(i,l))      { tty->print("O"); }
      else if(is_addr(i,l)) { tty->print("A"); }
      else tty->print("x");
    }
    tty->print_cr("]");
  }
}
#endif // PRODUCT


void ciLocalMap::set_init_local(int index, int local_index) {
  assert (0<=index && index<_nof_initialize, "range check");
  _locals[index] = local_index;
}


void ciLocalMap::set_nof_initialize(int n) {
  assert(_locals == NULL, "already allocated _locals: memory leak");
  if (n == 0) return;
  _nof_initialize = n;
  _locals = (unsigned int*)_arena->Amalloc(sizeof(unsigned int) * n);
}


bool ciLocalMap::is_address_local(int local, int bci) {
  if (nof_gc_points() == 0) {
    return false;
  }
  int map_index = index_for_bci(bci);
  if (map_index == -1) {
    return false;
  } else {
    return is_addr(map_index, local);
  }
}

// Implementation of ciGenerateLocalMap

GrowableArray<bool>* create_gc_point_array(Arena* arena, int size) {
  return new(arena)GrowableArray<bool>(arena, size, size, false);
}


void ciGenerateLocalMap::find_jsr_return_points(methodHandle method) {
  BytecodeStream s(method);
  Bytecodes::Code code;
  while ((code = s.next()) >= 0) {
    if (code == Bytecodes::_jsr) {
      if (_is_gc_point == NULL) {
        _is_gc_point = create_gc_point_array(_arena, method->code_size());
      }
      _is_gc_point->at_put(s.next_bci(), true); // set flag at return point of jsr
    }
  }
}


ciGenerateLocalMap::ciGenerateLocalMap(Arena* arena, methodHandle method) : GenerateOopMap(method) {
  _arena        = arena;
  _oop_maps     = NULL;
  _fill_counter = 0;
  // we have to adjust the assert in jvmciMethod before 
  // allowing no oop maps at exception points without handlers.
  _has_exceptions_handlers = method->exception_table()->length() != 0;
  _is_synchronized = method->is_synchronized();
  _is_gc_point = NULL;
  if (_has_exceptions_handlers) {
    _is_gc_point = create_gc_point_array(_arena, method->code_size());
    int lng = method()->exception_table()->length() ;
    for (int i = 0; i < lng; i+=4) {
      int start = method->exception_table()->int_at(i + 2);
      _is_gc_point->at_put(start, true);
    }
  }
  find_jsr_return_points(method);
}


bool ciGenerateLocalMap::possible_gc_point(BytecodeStream *bcs) {
  Bytecodes::Code code = bcs->code();
  // all exception handler starts must have an oop map
  if (_is_gc_point != NULL && _is_gc_point->at(bcs->bci())) {
    return true;
  }
  if (_is_synchronized && bcs->bci() == 0) {
    return true;
  }
  return bytecode_is_gc_point(code, _has_exceptions_handlers, _is_synchronized);
}


bool ciGenerateLocalMap::bytecode_is_gc_point(Bytecodes::Code code, bool method_has_exception_handlers, bool method_is_synchronized) {
  switch (code) {
    // exception points only: need gc map only if there is an exception handler 
    // in the same method
    case Bytecodes::_iaload         : 
    case Bytecodes::_laload         : 
    case Bytecodes::_faload         : 
    case Bytecodes::_daload         : 
    case Bytecodes::_aaload         : 
    case Bytecodes::_baload         : 
    case Bytecodes::_caload         : 
    case Bytecodes::_saload         : 
    case Bytecodes::_iastore        : 
    case Bytecodes::_lastore        : 
    case Bytecodes::_fastore        : 
    case Bytecodes::_dastore        : 
    case Bytecodes::_bastore        : 
    case Bytecodes::_castore        : 
    case Bytecodes::_sastore        : 

    case Bytecodes::_idiv           : 
    case Bytecodes::_ldiv           : 
    case Bytecodes::_irem           : 
    case Bytecodes::_lrem           : 

    case Bytecodes::_arraylength    : return (method_has_exception_handlers || method_is_synchronized);

    case Bytecodes::_athrow         : return true;
    // possible deoptimization points
    case Bytecodes::_getstatic      :
    case Bytecodes::_putstatic      :
    case Bytecodes::_getfield       :
    case Bytecodes::_putfield       :
    case Bytecodes::_invokevirtual  : 
    case Bytecodes::_invokespecial  : 
    case Bytecodes::_invokestatic   : 
    case Bytecodes::_invokeinterface: return true;

    case Bytecodes::_xxxunusedxxx   : ShouldNotReachHere(); return false;

    // allocation
    case Bytecodes::_new            : 
    case Bytecodes::_newarray       : 
    case Bytecodes::_anewarray      : 
    case Bytecodes::_multianewarray : return true;

    // bytecodes with slow case calling into runtime
    case Bytecodes::_aastore        : 
    case Bytecodes::_checkcast      : 
    case Bytecodes::_instanceof     : 
    case Bytecodes::_monitorenter   : return true;

    // for compiler safepoints
    case Bytecodes::_jsr_w          :
    case Bytecodes::_jsr            :
    case Bytecodes::_ret            : return UseCompilerSafepoints;

    // a cmp may be folded into an if<cond>
    case Bytecodes::_dcmpg          :
    case Bytecodes::_dcmpl          :
    case Bytecodes::_fcmpg          :
    case Bytecodes::_fcmpl          :
    case Bytecodes::_lcmp           :

    case Bytecodes::_tableswitch    :
    case Bytecodes::_lookupswitch   :
    case Bytecodes::_goto_w         :
    case Bytecodes::_goto           : 
    case Bytecodes::_ifeq           :
    case Bytecodes::_ifne           :
    case Bytecodes::_iflt           :
    case Bytecodes::_ifge           :
    case Bytecodes::_ifgt           :
    case Bytecodes::_ifle           :
    case Bytecodes::_ifnull         :
    case Bytecodes::_ifnonnull      :
    case Bytecodes::_if_icmpeq      :
    case Bytecodes::_if_icmpne      :
    case Bytecodes::_if_icmplt      :
    case Bytecodes::_if_icmpge      :
    case Bytecodes::_if_icmpgt      :
    case Bytecodes::_if_icmple      :
    case Bytecodes::_if_acmpeq      :
    case Bytecodes::_if_acmpne      : return UseCompilerSafepoints; // actually only backward branches need it

    // ldc of class constants are now potential patching points
    // currently we don't discriminate between ldc of classes and
    // other ldcs, although we could
    case Bytecodes::_ldc_w          :
    case Bytecodes::_ldc            : return true;

    default                         : return false;
  }
}


void ciGenerateLocalMap::fill_stackmap_prolog(int nof_gc_points) {
  _nof_gc_points = nof_gc_points;
}


void ciGenerateLocalMap::fill_stackmap_for_opcodes(BytecodeStream *bcs, CellTypeState* vars, CellTypeState* stack, int stackTop) {
  if (_oop_maps == NULL) {
    _oop_maps     = new ciLocalMap(_arena, method()->max_locals(), method()->code_size(), _nof_gc_points);
    _fill_counter = 0;
  }
  if (possible_gc_point(bcs) && method()->max_locals() > 0) {
    assert (_fill_counter < _nof_gc_points, "too few gc points reported");
    _oop_maps->set_bci_for_index(_fill_counter, bcs->bci());

    unsigned int* bit_mask = _oop_maps->map_for_index(_fill_counter);

    // we are interested only in the vars
    int n_entries       = method()->max_locals();
    CellTypeState* cell = vars;
    unsigned int  mask  = 1;
    unsigned int  value = 0;
    int word_index      = 0;

    for (int i = 0; i < n_entries; i++, cell++,  mask <<= ciLocalMap::bits_per_entry) {
      if (mask == 0) {

        bit_mask[word_index++] = value;
        value = 0;
        mask = 1;
      }
      if (cell->is_reference()) {
        value |= (mask << ciLocalMap::oop_bit_number );
      }
      if (cell->is_address()) {
        value |= (mask << ciLocalMap::addr_bit_number);
      }
    }
    bit_mask[word_index] = value;

    _fill_counter++;
  }
}


void ciGenerateLocalMap::fill_init_vars(GrowableArray<intptr_t> *init_vars) {
  _oop_maps->set_nof_initialize(init_vars->length());
  for (int i = 0; i < init_vars->length(); i++) {
    _oop_maps->set_init_local(i, init_vars->at(i));
  }
}

#endif // COMPILER1

