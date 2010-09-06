#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)c1_InstructionPrinter.cpp	1.109 04/03/31 18:13:07 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

#include "incls/_precompiled.incl"
#include "incls/_c1_InstructionPrinter.cpp.incl"


#ifndef PRODUCT

const char* InstructionPrinter::basic_type_name(BasicType type) {
  switch (type) {
    case T_BOOLEAN: return "boolean";
    case T_BYTE   : return "byte";
    case T_CHAR   : return "char";
    case T_SHORT  : return "short";
    case T_INT    : return "int";
    case T_LONG   : return "long";
    case T_FLOAT  : return "float";
    case T_DOUBLE : return "double";
    case T_ARRAY  : return "array";
    case T_OBJECT : return "object";
    default       : return "???";
  }
}


const char* InstructionPrinter::cond_name(If::Condition cond) {
  switch (cond) {
    case If::eql: return "==";
    case If::neq: return "!=";
    case If::lss: return "<";
    case If::leq: return "<=";
    case If::gtr: return ">";
    case If::geq: return ">=";
  }
  ShouldNotReachHere();
  return NULL;
}


const char* InstructionPrinter::op_name(Bytecodes::Code op) {
  switch (op) {
    // arithmetic ops
    case Bytecodes::_iadd : // fall through
    case Bytecodes::_ladd : // fall through
    case Bytecodes::_fadd : // fall through
    case Bytecodes::_dadd : return "+";
    case Bytecodes::_isub : // fall through
    case Bytecodes::_lsub : // fall through
    case Bytecodes::_fsub : // fall through
    case Bytecodes::_dsub : return "-";
    case Bytecodes::_imul : // fall through
    case Bytecodes::_lmul : // fall through
    case Bytecodes::_fmul : // fall through
    case Bytecodes::_dmul : return "*";
    case Bytecodes::_idiv : // fall through
    case Bytecodes::_ldiv : // fall through
    case Bytecodes::_fdiv : // fall through
    case Bytecodes::_ddiv : return "/";
    case Bytecodes::_irem : // fall through
    case Bytecodes::_lrem : // fall through
    case Bytecodes::_frem : // fall through
    case Bytecodes::_drem : return "%";
    // shift ops
    case Bytecodes::_ishl : // fall through
    case Bytecodes::_lshl : return "<<";
    case Bytecodes::_ishr : // fall through
    case Bytecodes::_lshr : return ">>";
    case Bytecodes::_iushr: // fall through
    case Bytecodes::_lushr: return ">>>";
    // logic ops
    case Bytecodes::_iand : // fall through
    case Bytecodes::_land : return "&";
    case Bytecodes::_ior  : // fall through
    case Bytecodes::_lor  : return "|";
    case Bytecodes::_ixor : // fall through
    case Bytecodes::_lxor : return "^";
  }
  return Bytecodes::name(op);
}


void InstructionPrinter::print_klass(ciKlass* klass) {
  tty->print("<klass 0x%x>", klass);
}


void InstructionPrinter::print_object(Value obj) {
  ValueType* type = obj->type();
  if (type->as_ObjectConstant() != NULL) {
    ciObject* value = type->as_ObjectConstant()->value();
    if (value->is_null_object()) {
      tty->print("null");
    } else if (!value->is_loaded()) {
      tty->print("<unloaded object 0x%x>", value);
    } else {
      tty->print("<object 0x%x>", value->encoding());
    }
  } else if (type->as_InstanceConstant() != NULL) {
    tty->print("<instance 0x%x>", type->as_InstanceConstant()->value()->encoding());
  } else if (type->as_ArrayConstant() != NULL) {
    tty->print("<array 0x%x>", type->as_ArrayConstant()->value()->encoding());
  } else if (type->as_ClassConstant() != NULL) {
    ciInstanceKlass* klass = type->as_ClassConstant()->value();
    if (!klass->is_loaded()) {
      tty->print("<unloaded class>");
    } else {
      tty->print("<class 0x%x>", klass->encoding());
    }
  } else {
    tty->print("???");
  }
}


void InstructionPrinter::print_temp(Value value) {
  tty->print("%c%d", value->type()->tchar(), value->id());
}


void InstructionPrinter::print_local(AccessLocal* local) {
  tty->print("L%d/name %d", local->java_index(), local->local_name());
}


void InstructionPrinter::print_field(AccessField* field) {
  print_value(field->obj());
  tty->print("._%d", field->offset());
}


void InstructionPrinter::print_indexed(AccessIndexed* indexed) {
  print_value(indexed->array());
  tty->put('[');
  print_value(indexed->index());
  tty->put(']');
}


void InstructionPrinter::print_monitor(AccessMonitor* monitor) {
  tty->print("monitor[%d](", monitor->monitor_no());
  print_value(monitor->obj());
  tty->put(')');
}


void InstructionPrinter::print_op2(Op2* instr) {
  print_value(instr->x());
  tty->print(" %s ", op_name(instr->op()));
  print_value(instr->y());
}


void InstructionPrinter::print_value(Value value) {
  if (value == NULL) {
    tty->print("NULL");
  } else {
    print_temp(value);
  }
}


void InstructionPrinter::print_instr(Instruction* instr) {
  instr->visit(this);
}


void InstructionPrinter::print_stack(ValueStack* stack) {
  int start_position = tty->position();
  if (stack->stack_is_empty()) {
    tty->print("empty stack");
  } else {
    tty->print("stack [");
    for (int i = 0; i < stack->stack_size();) {
      if (i > 0) tty->print(", ");
      tty->print("%d:", i);
      print_value(stack->stack_at_inc(i));
    }
    tty->put(']');
  }

  if (!stack->no_active_locks()) {
    // print out the lines on the line below this
    // one at the same indentation level.
    tty->cr();
    fill_to(start_position, ' ');
    tty->print("locks [");
    for (int i = i = 0; i < stack->locks_size(); i++) {
      Value t = stack->locks().at(i);
      if (i > 0) tty->print(", ");
      tty->print("%d:", i);
      if (t == NULL) {
        // synchronized methods push null on the lock stack
        tty->print("this");
      } else {
        print_value(t);
      }
    }
    tty->print("]");
  }
}


void InstructionPrinter::print_inline_level(BlockBegin* block) {
  tty->print_cr("inlining depth %d", block->scope()->level());
}


void InstructionPrinter::print_unsafe_op(UnsafeOp* op, const char* name) {
  tty->print(name);
  tty->put(' ');
  print_value(op->unsafe());
  tty->print(".(");
}

void InstructionPrinter::print_unsafe_raw_op(UnsafeRawOp* op, const char* name) {
  print_unsafe_op(op, name);
  tty->print("base ");
  print_value(op->base());
  if (op->has_index()) {
    tty->print(", index "); print_value(op->index());
    tty->print(", log2_scale %d", op->log2_scale());
  }
}


void InstructionPrinter::print_unsafe_object_op(UnsafeObjectOp* op, const char* name) {
  print_unsafe_op(op, name);
  print_value(op->object());
}


void InstructionPrinter::fill_to(int pos, char filler) {
  while (tty->position() < pos) tty->put(filler);
}


void InstructionPrinter::print_head() {
  const char filler = '_';
  fill_to(bci_pos  , filler); tty->print("bci"  );
  fill_to(use_pos  , filler); tty->print("use"  );
  fill_to(temp_pos , filler); tty->print("tid"  );
  fill_to(instr_pos, filler); tty->print("instr");
  fill_to(end_pos  , filler);
  tty->cr();
}


void InstructionPrinter::print_line(Instruction* instr) {
  // print instruction data on one line
  if (instr->is_pinned()) tty->put('.');
  fill_to(bci_pos  ); tty->print("%d", instr->bci());
  fill_to(use_pos  ); tty->print("%d", instr->use_count());
  fill_to(temp_pos ); print_temp(instr);
  fill_to(instr_pos); print_instr(instr);
  tty->cr();
  // add a line for StateSplit instructions w/ non-empty stacks
  // (make it robust so we can print incomplete instructions)
  StateSplit* split = instr->as_StateSplit();
  if (split != NULL && split->state() != NULL && !split->state()->stack_is_empty()) {
    fill_to(instr_pos); print_stack(split->state());
    tty->cr();
  }
}


void InstructionPrinter::do_Phi(Phi* x) {
  tty->print("phi-%c%d", x->type()->tchar(), x->id());
}


void InstructionPrinter::do_Local(Local* x) {
  tty->print("local[index %d, name %d]", x->java_index(), x->local_name());
}


void InstructionPrinter::do_Constant(Constant* x) {
  ValueType* t = x->type();
  switch (t->tag()) {
    case intTag    : tty->print("%d"  , t->as_IntConstant   ()->value());    break;
    case longTag   : tty->print(os::jlong_format_specifier(), t->as_LongConstant()->value()); tty->print("L"); break;
    case floatTag  : tty->print("%g"  , t->as_FloatConstant ()->value());    break;
    case doubleTag : tty->print("%gD" , t->as_DoubleConstant()->value());    break;
    case objectTag : print_object(x);                                        break;
    case addressTag: tty->print("bci:%d", t->as_AddressConstant()->value()); break;
    default        : tty->print("???");                                      break;
  }
}


void InstructionPrinter::do_LoadLocal(LoadLocal* x) {
  print_local(x);
}


void InstructionPrinter::do_StoreLocal(StoreLocal* x) {
  print_local(x);
  tty->print(" := ");
  print_value(x->value());
  if (x->is_eliminated()) {
    tty->print(" (eliminated)");
  }
}


void InstructionPrinter::do_LoadField(LoadField* x) {
  print_field(x);
}


void InstructionPrinter::do_StoreField(StoreField* x) {
  print_field(x);
  tty->print(" := ");
  print_value(x->value());
}


void InstructionPrinter::do_ArrayLength(ArrayLength* x) {
  print_value(x->array());
  tty->print(".length");
}


void InstructionPrinter::do_LoadIndexed(LoadIndexed* x) {
  print_indexed(x);
}


void InstructionPrinter::do_StoreIndexed(StoreIndexed* x) {
  print_indexed(x);
  tty->print(" := ");
  print_value(x->value());
}

void InstructionPrinter::do_CachingChange(CachingChange* x) {
  tty->print("caching change ");
  if (x->pred_block()->is_set(BlockBegin::single_precision_flag) != x->sux_block()->is_set(BlockBegin::single_precision_flag)) {
    tty->print("; precision change ");
  }
}


void InstructionPrinter::do_NegateOp(NegateOp* x) {
  tty->put('-');
  print_value(x->x());
}


void InstructionPrinter::do_ArithmeticOp(ArithmeticOp* x) {
  print_op2(x);
}


void InstructionPrinter::do_ShiftOp(ShiftOp* x) {
  print_op2(x);
}


void InstructionPrinter::do_LogicOp(LogicOp* x) {
  print_op2(x);
}


void InstructionPrinter::do_CompareOp(CompareOp* x) {
  print_op2(x);
}


void InstructionPrinter::do_IfOp(IfOp* x) {
  print_value(x->x());
  tty->print(" %s ", cond_name(x->cond()));
  print_value(x->y());
  tty->print(" ? ");
  print_value(x->tval());
  tty->print(" : ");
  print_value(x->fval());
}


void InstructionPrinter::do_Convert(Convert* x) {
  tty->print("%s(", Bytecodes::name(x->op()));
  print_value(x->value());
  tty->put(')');
}


void InstructionPrinter::do_NullCheck(NullCheck* x) {
  tty->print("null_check(");
  print_value(x->obj());
  tty->put(')');
  if (!x->can_trap()) {
    tty->print(" (eliminated)");
  }
}


void InstructionPrinter::do_Invoke(Invoke* x) {
  if (x->receiver() != NULL) {
    print_value(x->receiver());
    tty->print(".");
  }
  tty->print("%s(", Bytecodes::name(x->code()));
  for (int i = 0; i < x->number_of_arguments(); i++) {
    if (i > 0) tty->print(", ");
    print_value(x->argument_at(i));
  }
  tty->put(')');
}


void InstructionPrinter::do_NewInstance(NewInstance* x) {
  tty->print("new instance ");
  print_klass(x->klass());
}


void InstructionPrinter::do_NewTypeArray(NewTypeArray* x) {
  tty->print("new %s array [", basic_type_name(x->elt_type()));
  print_value(x->length());
  tty->put(']');
}


void InstructionPrinter::do_NewObjectArray(NewObjectArray* x) {
  tty->print("new object array [");
  print_value(x->length());
  tty->print("] ");
  print_klass(x->klass());
}


void InstructionPrinter::do_NewMultiArray(NewMultiArray* x) {
  tty->print("new multi array [");
  Values* dims = x->dims();
  for (int i = 0; i < dims->length(); i++) {
    if (i > 0) tty->print(", ");
    print_value(dims->at(i));
  }
  tty->print("] ");
  print_klass(x->klass());
}


void InstructionPrinter::do_MonitorEnter(MonitorEnter* x) {
  tty->print("enter ");
  print_monitor(x);
}


void InstructionPrinter::do_MonitorExit(MonitorExit* x) {
  tty->print("exit ");
  print_monitor(x);
}


void InstructionPrinter::do_Intrinsic(Intrinsic* x) {
  const char* name = "<unknown intrinsic>";
  switch (x->id()) {
    case methodOopDesc::_getClass      : name = "getClass";       break;
    case methodOopDesc::_currentThread : name = "currentThread";  break;
    case methodOopDesc::_dsin          : name = "dsin";           break;
    case methodOopDesc::_dcos          : name = "dcos";           break;
    case methodOopDesc::_dsqrt         : name = "dsqrt";          break;
    case methodOopDesc::_arraycopy     : name = "arraycopy";      break;
    case methodOopDesc::_compareTo     : name = "compareto";      break;

    case methodOopDesc::_currentTimeMillis   : name = "System.currentTimeMillis";   break;
    case methodOopDesc::_nanoTime            : name = "System.nanoTime";            break;

    case methodOopDesc::_intBitsToFloat      : name = "Float.intBitsToFloat";       break;
    case methodOopDesc::_floatToRawIntBits   : name = "Float.floatToRawIntBits";    break;
    case methodOopDesc::_longBitsToDouble    : name = "Double.longBitsToDouble";    break;
    case methodOopDesc::_doubleToRawLongBits : name = "Double.doubleToRawLongBits"; break;

    // %%% the following xxx_obj32 are temporary until the 1.4.0 sun.misc.Unsafe goes away
    case methodOopDesc::_getObject_obj32: name = "getObject_obj32"; break;
    case methodOopDesc::_getBoolean_obj32:name = "getBoolean_obj32";break;
    case methodOopDesc::_getByte_obj32  : name = "getByte_obj32";   break;
    case methodOopDesc::_getShort_obj32 : name = "getShort_obj32";  break;
    case methodOopDesc::_getChar_obj32  : name = "getChar_obj32";   break;
    case methodOopDesc::_getInt_obj32   : name = "getInt_obj32";    break;
    case methodOopDesc::_getLong_obj32  : name = "getLong_obj32";   break;
    case methodOopDesc::_getFloat_obj32 : name = "getFloat_obj32";  break;
    case methodOopDesc::_getDouble_obj32: name = "getDouble_obj32"; break;

    case methodOopDesc::_putObject_obj32: name = "putObject_obj32"; break;
    case methodOopDesc::_putBoolean_obj32:name = "putBoolean_obj32";break;
    case methodOopDesc::_putByte_obj32  : name = "putByte_obj32";   break;
    case methodOopDesc::_putShort_obj32 : name = "putShort_obj32";  break;
    case methodOopDesc::_putChar_obj32  : name = "putChar_obj32";   break;
    case methodOopDesc::_putInt_obj32   : name = "putInt_obj32";    break;
    case methodOopDesc::_putLong_obj32  : name = "putLong_obj32";   break;
    case methodOopDesc::_putFloat_obj32 : name = "putFloat_obj32";  break;
    case methodOopDesc::_putDouble_obj32: name = "putDouble_obj32"; break;

    case methodOopDesc::_getObject_obj : name = "getObject_obj";  break;
    case methodOopDesc::_getBoolean_obj: name = "getBoolean_obj"; break;
    case methodOopDesc::_getByte_obj   : name = "getByte_obj";    break;
    case methodOopDesc::_getShort_obj  : name = "getShort_obj";   break;
    case methodOopDesc::_getChar_obj   : name = "getChar_obj";    break;
    case methodOopDesc::_getInt_obj    : name = "getInt_obj";     break;
    case methodOopDesc::_getLong_obj   : name = "getLong_obj";    break;
    case methodOopDesc::_getFloat_obj  : name = "getFloat_obj";   break;
    case methodOopDesc::_getDouble_obj : name = "getDouble_obj";  break;

    case methodOopDesc::_putObject_obj : name = "putObject_obj";  break;
    case methodOopDesc::_putBoolean_obj: name = "putBoolean_obj"; break;
    case methodOopDesc::_putByte_obj   : name = "putByte_obj";    break;
    case methodOopDesc::_putShort_obj  : name = "putShort_obj";   break;
    case methodOopDesc::_putChar_obj   : name = "putChar_obj";    break;
    case methodOopDesc::_putInt_obj    : name = "putInt_obj";     break;
    case methodOopDesc::_putLong_obj   : name = "putLong_obj";    break;
    case methodOopDesc::_putFloat_obj  : name = "putFloat_obj";   break;
    case methodOopDesc::_putDouble_obj : name = "putDouble_obj";  break;

    case methodOopDesc::_getObjectVolatile_obj : name = "getObjectVolatile_obj";  break;
    case methodOopDesc::_getBooleanVolatile_obj: name = "getBooleanVolatile_obj"; break;
    case methodOopDesc::_getByteVolatile_obj   : name = "getByteVolatile_obj";    break;
    case methodOopDesc::_getShortVolatile_obj  : name = "getShortVolatile_obj";   break;
    case methodOopDesc::_getCharVolatile_obj   : name = "getCharVolatile_obj";    break;
    case methodOopDesc::_getIntVolatile_obj    : name = "getIntVolatile_obj";     break;
    case methodOopDesc::_getLongVolatile_obj   : name = "getLongVolatile_obj";    break;
    case methodOopDesc::_getFloatVolatile_obj  : name = "getFloatVolatile_obj";   break;
    case methodOopDesc::_getDoubleVolatile_obj : name = "getDoubleVolatile_obj";  break;

    case methodOopDesc::_putObjectVolatile_obj : name = "putObjectVolatile_obj";  break;
    case methodOopDesc::_putBooleanVolatile_obj: name = "putBooleanVolatile_obj"; break;
    case methodOopDesc::_putByteVolatile_obj   : name = "putByteVolatile_obj";    break;
    case methodOopDesc::_putShortVolatile_obj  : name = "putShortVolatile_obj";   break;
    case methodOopDesc::_putCharVolatile_obj   : name = "putCharVolatile_obj";    break;
    case methodOopDesc::_putIntVolatile_obj    : name = "putIntVolatile_obj";     break;
    case methodOopDesc::_putLongVolatile_obj   : name = "putLongVolatile_obj";    break;
    case methodOopDesc::_putFloatVolatile_obj  : name = "putFloatVolatile_obj";   break;
    case methodOopDesc::_putDoubleVolatile_obj : name = "putDoubleVolatile_obj";  break;

    case methodOopDesc::_getByte_raw   : name = "getByte_raw";    break;
    case methodOopDesc::_getShort_raw  : name = "getShort_raw";   break;
    case methodOopDesc::_getChar_raw   : name = "getChar_raw";    break;
    case methodOopDesc::_getInt_raw    : name = "getInt_raw";     break;
    case methodOopDesc::_getLong_raw   : name = "getLong_raw";    break;
    case methodOopDesc::_getFloat_raw  : name = "getFloat_raw";   break;
    case methodOopDesc::_getDouble_raw : name = "getDouble_raw";  break;

    case methodOopDesc::_putByte_raw   : name = "putByte_raw";    break;
    case methodOopDesc::_putShort_raw  : name = "putShort_raw";   break;
    case methodOopDesc::_putChar_raw   : name = "putChar_raw";    break;
    case methodOopDesc::_putInt_raw    : name = "putInt_raw";     break;
    case methodOopDesc::_putLong_raw   : name = "putLong_raw";    break;
    case methodOopDesc::_putFloat_raw  : name = "putFloat_raw";   break;
    case methodOopDesc::_putDouble_raw : name = "putDouble_raw";  break;

    case methodOopDesc::_checkIndex    : name = "NIO_checkIndex";  break;
    case methodOopDesc::_attemptUpdate : name = "AtomicLong_attemptUpdate";  break;
    case methodOopDesc::_compareAndSwapLong_obj: name = "compareAndSwapLong_obj";  break;
    case methodOopDesc::_compareAndSwapObject_obj: name = "compareAndSwapObject_obj";  break;
    case methodOopDesc::_compareAndSwapInt_obj: name = "compareAndSwapInt_obj";  break;
  }
  tty->print("%s(", name);
  for (int i = 0; i < x->number_of_arguments(); i++) {
    if (i > 0) tty->print(", ");
    print_value(x->argument_at(i));
  }
  tty->put(')');
}


void InstructionPrinter::do_BlockBegin(BlockBegin* x) {
  // print block id
  BlockEnd* end = x->end();
  tty->print("B%d ", x->block_id());

  // print flags
  bool printed_flag = false;
  if (x->is_set(BlockBegin::std_entry_flag)) {
    if (!printed_flag) tty->print("(");
    tty->print("S"); printed_flag = true;
  }
  if (x->is_set(BlockBegin::osr_entry_flag)) {
    if (!printed_flag) tty->print("(");
    tty->print("O"); printed_flag = true;
  }
  if (x->is_set(BlockBegin::exception_entry_flag)) {
    if (!printed_flag) tty->print("(");
    tty->print("E"); printed_flag = true;
  }
  if (x->is_set(BlockBegin::subroutine_entry_flag)) {
    if (!printed_flag) tty->print("(");
    tty->print("s"); printed_flag = true;
  }
  if (x->is_set(BlockBegin::backward_branch_target_flag)) {
    if (!printed_flag) tty->print("(");
    tty->print("b"); printed_flag = true;
  }
  if (x->is_set(BlockBegin::was_visited_flag)) {
    if (!printed_flag) tty->print("(");
    tty->print("V"); printed_flag = true;
  }
  if (x->is_set(BlockBegin::single_precision_flag)) {
    if (!printed_flag) tty->print("(");
    tty->print("F24"); printed_flag = true;
  }
  if (printed_flag) tty->print(") ");

  // print block bci range
  tty->print("[%d, %d]", x->bci(), (end == NULL ? -1 : end->bci()));

  // print block successors
  if (end != NULL && end->number_of_sux() > 0) {
    tty->print(" ->");
    for (int i = 0; i < end->number_of_sux(); i++) {
      tty->print(" B%d", end->sux_at(i)->block_id());
    }
  }
  // print exception handlers
  if (x->number_of_exception_handlers() > 0) {
    tty->print(" (xhandlers ");
    for (int i = 0; i < x->number_of_exception_handlers();  i++) {
      if (i > 0) tty->print(" ");
      tty->print("B%d", x->exception_handler_at(i)->block_id());
    }
    tty->put(')');
  }
}


void InstructionPrinter::do_CheckCast(CheckCast* x) {
  tty->print("checkcast(");
  print_value(x->obj());
  tty->print(") ");
  print_klass(x->klass());
}


void InstructionPrinter::do_InstanceOf(InstanceOf* x) {
  tty->print("instanceof(");
  print_value(x->obj());
  tty->print(") ");
  print_klass(x->klass());
}


void InstructionPrinter::do_Goto(Goto* x) {
  tty->print("goto B%d", x->default_sux()->block_id());
}


void InstructionPrinter::do_If(If* x) {
  tty->print("if ");
  print_value(x->x());
  tty->print(" %s ", cond_name(x->cond()));
  print_value(x->y());
  tty->print(" then B%d else B%d", x->sux_at(0)->block_id(), x->sux_at(1)->block_id());
}


void InstructionPrinter::do_IfInstanceOf(IfInstanceOf* x) {
  tty->print("<IfInstanceOf>");
}


void InstructionPrinter::do_TableSwitch(TableSwitch* x) {
  tty->print("tableswitch ");
  print_value(x->tag());
  tty->cr();
  int l = x->length();
  for (int i = 0; i < l; i++) {
    fill_to(instr_pos);
    tty->print_cr("case %5d: B%d", x->lo_key() + i, x->sux_at(i)->block_id());
  }
  fill_to(instr_pos);
  tty->print("default   : B%d", x->default_sux()->block_id());
}


void InstructionPrinter::do_LookupSwitch(LookupSwitch* x) {
  tty->print("lookupswitch ");
  print_value(x->tag());
  tty->cr();
  int l = x->length();
  for (int i = 0; i < l; i++) {
    fill_to(instr_pos);
    tty->print_cr("case %5d: B%d", x->key_at(i), x->sux_at(i)->block_id());
  }
  fill_to(instr_pos);
  tty->print("default   : B%d", x->default_sux()->block_id());
}


void InstructionPrinter::do_Return(Return* x) {
  if (x->result() == NULL) {
    tty->print("return");
  } else {
    tty->print("%creturn ", x->type()->tchar());
    print_value(x->result());
  }
}


void InstructionPrinter::do_Throw(Throw* x) {
  tty->print("throw ");
  print_value(x->exception());
}


void InstructionPrinter::do_Base(Base* x) {
  tty->print("std entry B%d", x->std_entry()->block_id());
  if (x->number_of_sux() > 1) {
    tty->print(" osr entry B%d", x->osr_entry()->block_id());
  }
}


void InstructionPrinter::do_UnsafeGetRaw(UnsafeGetRaw* x) {
  print_unsafe_raw_op(x, "UnsafeGetRaw");
  tty->put(')');
}


void InstructionPrinter::do_UnsafePutRaw(UnsafePutRaw* x) {
  print_unsafe_raw_op(x, "UnsafePutRaw");
  tty->print(", value ");
  print_value(x->value());
  tty->put(')');
}


void InstructionPrinter::do_UnsafeGetObject(UnsafeGetObject* x) {
  print_unsafe_object_op(x, "UnsafeGetObject");
  tty->put(')');
}


void InstructionPrinter::do_UnsafePutObject(UnsafePutObject* x) {
  print_unsafe_object_op(x, "UnsafePutObject");
  tty->print(", value ");
  print_value(x->value());
  tty->put(')');
}


#endif // PRODUCT

