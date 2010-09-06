#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)ciMethodData.cpp	1.17 04/03/02 02:08:48 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_ciMethodData.cpp.incl"

// ciMethodData

// ------------------------------------------------------------------
// ciMethodData::ciMethodData
//
ciMethodData::ciMethodData(methodDataHandle h_md) : ciObject(h_md) {
  assert(h_md() != NULL, "no null method data");
  Copy::zero_to_words((HeapWord*) &_orig, sizeof(_orig) / sizeof(HeapWord));
  _data = NULL;
  _data_size = 0;
  _extra_data_size = 0;
  _current_mileage = 0;
  _state = empty_state;
  _saw_free_extra_data = false;
  // Set an initial hint. Don't use set_hint_di() because
  // first_di() may be out of bounds if data_size is 0.
  _hint_di = first_di();
}

// ------------------------------------------------------------------
// ciMethodData::ciMethodData
//
// No methodDataOop.
ciMethodData::ciMethodData() : ciObject() {
  Copy::zero_to_words((HeapWord*) &_orig, sizeof(_orig) / sizeof(HeapWord));
  _data = NULL;
  _data_size = 0;
  _extra_data_size = 0;
  _current_mileage = 0;
  _state = empty_state;
  _saw_free_extra_data = false;
  // Set an initial hint. Don't use set_hint_di() because
  // first_di() may be out of bounds if data_size is 0.
  _hint_di = first_di();
}

void ciMethodData::load_data() {
  methodDataOop mdo = get_methodDataOop();
  if (mdo == NULL) return;

  // To do: don't copy the data if it is not "ripe" -- require a minimum # 
  // of invocations.

  // Snapshot the data -- actually, take an approximate snapshot of
  // the data.  Any concurrently executing threads may be changing the 
  // data as we copy it.
  int skip_header = oopDesc::header_size();
  Copy::disjoint_words((HeapWord*) mdo              + skip_header,
                       (HeapWord*) &_orig           + skip_header,
                       sizeof(_orig) / HeapWordSize - skip_header);
  DEBUG_ONLY(*_orig.adr_method() = NULL);  // no dangling oops, please
  Arena* arena = CURRENT_ENV->arena();
  _data_size = mdo->data_size();
  _extra_data_size = mdo->extra_data_size();
  int total_size = _data_size + _extra_data_size;
  _data = (intptr_t *) arena->Amalloc(total_size);
  Copy::disjoint_words((HeapWord*) mdo->data_base(), (HeapWord*) _data, total_size / HeapWordSize);

  // Traverse the profile data, translating any oops into their 
  // ci equivalents.
  ResourceMark rm;
  ciProfileData* ci_data = first_data();
  ProfileData* data = mdo->first_data();
  while (is_valid(ci_data)) {
    ci_data->translate_from(data);
    ci_data = next_data(ci_data);
    data = mdo->next_data(data);
  }
  // Note:  Extra data are all BitData, and do not need translation.
  _current_mileage = methodDataOopDesc::mileage_of(mdo->method());
  _state = mdo->is_mature()? mature_state: immature_state;
}

void ciVirtualCallData::translate_from(ProfileData* data) {
  for (uint row = 0; row < row_limit(); row++) {
    klassOop k = data->as_VirtualCallData()->receiver(row);
    if (k) {
      ciKlass* klass = CURRENT_ENV->get_object(k)->as_klass();
      set_receiver(row, klass);
    }
  }
};


// Get the data at an arbitrary (sort of) data index.
ciProfileData* ciMethodData::data_at(int data_index) {
  if (out_of_bounds(data_index)) {
    return NULL;
  }
  DataLayout* data_layout = data_layout_at(data_index);
  
  switch (data_layout->tag()) {
  case DataLayout::no_tag:
  default:
    ShouldNotReachHere();
    return NULL;
  case DataLayout::bit_data_tag:
    return new ciBitData(data_layout);
  case DataLayout::counter_data_tag:
    return new ciCounterData(data_layout);
  case DataLayout::jump_data_tag:
    return new ciJumpData(data_layout);
  case DataLayout::virtual_call_data_tag:
    return new ciVirtualCallData(data_layout);
  case DataLayout::ret_data_tag:
    return new ciRetData(data_layout);
  case DataLayout::branch_data_tag:
    return new ciBranchData(data_layout);
  case DataLayout::multi_branch_data_tag:
    return new ciMultiBranchData(data_layout);
  };
}

// Iteration over data.
ciProfileData* ciMethodData::next_data(ciProfileData* current) {
  int current_index = dp_to_di(current->dp());
  int next_index = current_index + current->size_in_bytes();
  ciProfileData* next = data_at(next_index);
  return next;
}

// Translate a bci to its corresponding data, or NULL.
ciProfileData* ciMethodData::bci_to_data(int bci) {
  ciProfileData* data = data_before(bci);
  for ( ; is_valid(data); data = next_data(data)) {
    if (data->bci() == bci) {
      set_hint_di(dp_to_di(data->dp()));
      return data;
    } else if (data->bci() > bci) {
      break;
    }
  }
  // bci_to_extra_data(bci) ...
  DataLayout* dp  = data_layout_at(data_size());
  DataLayout* end = data_layout_at(data_size() + extra_data_size());
  for (; dp < end; dp = methodDataOopDesc::next_extra(dp)) {
    if (dp->tag() == DataLayout::no_tag) {
      _saw_free_extra_data = true;  // observed an empty slot (common case)
      return NULL;
    }
    if (dp->bci() == bci) {
      assert(dp->tag() == DataLayout::bit_data_tag, "sane");
      return new ciBitData(dp);
    }
  }
  return NULL;
}

// Conservatively decode the trap_state of a ciProfileData.
int ciMethodData::has_trap_at(ciProfileData* data, int reason) {
  if (trap_count(reason) == 0) {
    // Impossible for this trap to have occurred, regardless of trap_state.
    // Note:  This happens if the MDO is empty.
    return 0;
  } else if (data == NULL) {
    // No profile here, not even an extra_data record allocated on the fly.
    // If there are empty extra_data records, and there had been a trap,
    // there would have been a non-null data pointer.  If there are no
    // free extra_data records, we must return a conservative -1.
    return (_saw_free_extra_data? 0: -1);
  } else {
    return Deoptimization::trap_state_has_reason(data->trap_state(), reason);
  }
}

int ciMethodData::trap_recompiled_at(ciProfileData* data) {
  if (data == NULL) {
    return (_saw_free_extra_data? 0: -1);  // (see previous method)
  } else {
    return Deoptimization::trap_state_is_recompiled(data->trap_state())? 1: 0;
  }
}

// Implementation of the print method.
void ciMethodData::print_impl() {
  ciObject::print_impl();
}

#ifndef PRODUCT
void ciMethodData::print() {
  print_data_on(tty); 
}

void ciMethodData::print_data_on(outputStream* st) {
  ResourceMark rm;
  ciProfileData* data;
  for (data = first_data(); is_valid(data); data = next_data(data)) {
    st->print("%d", dp_to_di(data->dp()));
    st->fill_to(6);
    data->print_data_on(st);
  }
}

void ciVirtualCallData::print_data_on(outputStream* st) {
  print_shared(st, "ciVirtualCallData");
  uint row;
  int entries = 0;
  for (row = 0; row < row_limit(); row++) {
    if (receiver(row) != NULL)  entries++;
  }
  st->print_cr("count(%u) entries(%u)", count(), entries);
  for (row = 0; row < row_limit(); row++) {
    if (receiver(row) != NULL) {
      tab(st);
      receiver(row)->print_value_on(st);
      st->print_cr("(%u)", receiver_count(row));
    }
  }
}
#endif
