#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)stackMapTable.cpp	1.10 03/12/23 16:44:11 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_stackMapTable.cpp.incl"

StackMapTable::StackMapTable(StackMapReader reader, u2 max_locals, u2 max_stack, TRAPS) {
  _frame_count = reader.get_frame_count();
  if (_frame_count > 0) {
    _frame_array = NEW_C_HEAP_ARRAY(StackMapFrame*, _frame_count);
    for (u2 i = 0; i < _frame_count; i++) {
      _frame_array[i] = reader.next(max_locals, max_stack, CHECK);
    }
  }
}

StackMapTable::~StackMapTable() {
  for (u2 i = 0; i < _frame_count; i++) {
    delete _frame_array[i];
  }
  if (_frame_count > 0) {
    FREE_C_HEAP_ARRAY(StackMapFrame*, _frame_array);
  }
}

// This method is only called by method in StackMapTable.
u2 StackMapTable::get_index_from_offset(u2 offset) const {
  u2 i = 0;
  for (; i < _frame_count; i++) {
    if (_frame_array[i]->offset() == offset) {
      return i;
    }
  }
  return i;  // frame with offset doesn't exist in the array
}

bool StackMapTable::match_stackmap(StackMapFrame* frame, u2 target,
                                   bool match, bool update, TRAPS) const {
  u2 index = get_index_from_offset(target);
  bool ret = match_stackmap(frame, target, index, match, update, CHECK_0);
  return ret;
}

// Match and/or update current_frame to the frame in stackmap table with
// specified offset and frame index. Return true if the two frames match.
//
// The values of match and update are:
//                                                                     match  update
// checking a branch target/exception handler                          true   false
// linear bytecode verification following an unconditional branch      false  true
// linear bytecode verification not following an unconditional branch  true   true
bool StackMapTable::match_stackmap(StackMapFrame* frame,
                                   u2 target, u2 frame_index,
                                   bool match, bool update, TRAPS) const {
  if (frame_index >= _frame_count) {
    ResourceMark rm(THREAD);
    Exceptions::fthrow(THREAD_AND_LOCATION, vmSymbols::java_lang_VerifyError(),
                      "Expecting a stackmap frame at branch target %d in method %s", 
                       target, 
                       frame->verifier()->_method->name_and_sig_as_C_string(),
                       CHECK_0);
  }

  bool result = true;
  StackMapFrame *stackmap_frame = _frame_array[frame_index];
  if (match) {
    // Has direct control flow from last instruction, need to match the two frames.
    result = frame->is_assignable_to(stackmap_frame, CHECK_0);
  }
  if (update) { 
    // Use the frame in stackmap table as current frame
    frame->reset();
    frame->set_locals_size(stackmap_frame->locals_size());
    frame->copy_locals(stackmap_frame);
    frame->set_stack_size(stackmap_frame->stack_size());
    frame->copy_stack(stackmap_frame);
    frame->set_flags(stackmap_frame->flags());
  }
  return result;
}

void StackMapTable::check_jump_target(StackMapFrame* frame, u2 target, TRAPS) const {
  bool match = match_stackmap(frame, target, true, false, CHECK);
  if (!match) {
    ResourceMark rm(THREAD);
    Exceptions::fthrow(THREAD_AND_LOCATION, 
                       vmSymbols::java_lang_VerifyError(),
                       "Inconsistent stackmap frames at offset %d and at branch target %d in method %s", 
                       frame->offset(), 
                       target, 
                       frame->verifier()->_method->name_and_sig_as_C_string(),
                       CHECK);
  }
  // check if uninitialized objects exist on backward branches
  check_new_object(frame, target, CHECK);
}

void StackMapTable::check_new_object(const StackMapFrame* frame, u2 target, TRAPS) const {
  if (frame->offset() > target && frame->has_new_object()) {
    ResourceMark rm(THREAD);
    Exceptions::fthrow(THREAD_AND_LOCATION, 
                       vmSymbols::java_lang_VerifyError(),
                       "Uninitialized object exists on backward branch %d in method %s at offset %d", 
                       target, 
                       frame->verifier()->_method->name_and_sig_as_C_string(), 
                       frame->offset(),
                       CHECK);
  }
}

#ifndef PRODUCT

void StackMapTable::print() const {
  tty->print_cr("StackMapTable: frame_count = %d", _frame_count);
  tty->print_cr("table = { ");
  for (u2 i = 0; i < _frame_count; i++) {
    _frame_array[i]->print();
  }
  tty->print_cr(" }");
}

#endif

// We don't need to verify the stored attribute again because it's already been 
// checked during class file parsing.
StackMapFrame* StackMapReader::next(u2 max_locals, u2 max_stack, TRAPS) {
  u2 offset = get_u2();
  
  // locals_size represents number of elements in locals array (long/double occupies two slots)
  u2 locals_size = get_u2();
  u1 flags = 0;
  VerificationType** locals = parse_stackmap_variables(locals_size, &flags, CHECK_0);

  // stack_size represents number of elements in stack array (long/double occupies two slots)
  u2 stack_size = get_u2();
  VerificationType** stack = parse_stackmap_variables(stack_size, NULL, CHECK_0);
  
  return new StackMapFrame(offset, flags, locals_size, stack_size, max_locals, max_stack, locals, stack);
}

VerificationType** StackMapReader::parse_stackmap_variables(u2 len, u1* flags, TRAPS) {
  if (len == 0) { return NULL; }

  u2 index = 0;
  VerificationType** type_array = NEW_C_HEAP_ARRAY(VerificationType*, len);
  while (index < len) {
    u1 tag = get_u1();
    if (tag < ITEM_UninitializedThis) {
      type_array[index++] = VerificationType::get_primary_type((Tag)tag);
      if (tag == ITEM_Long) {
        type_array[index++] = VerificationType::_long2_type;
      } else if (tag == ITEM_Double) {
        type_array[index++] = VerificationType::_double2_type;
      }
    } else if (tag == ITEM_UninitializedThis) {
      type_array[index++] = UninitializedType::_uninitialized_this;
      if (flags != NULL) { *flags |= FLAG_THIS_UNINIT; }
    } else if (tag == ITEM_Object) {
      u2 class_index = get_u2();
      type_array[index++] = _verifier->cp_index_to_type(class_index, _cp, CHECK_0);
    } else if (tag == ITEM_Uninitialized) {
      u2 offset = get_u2();
      type_array[index++] = new UninitializedType(offset);
    } else {
      ShouldNotReachHere();
    }
  }
  return type_array;
}
