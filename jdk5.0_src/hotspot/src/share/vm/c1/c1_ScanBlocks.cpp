#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)c1_ScanBlocks.cpp	1.48 03/12/23 16:39:19 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_c1_ScanBlocks.cpp.incl"


void ScanBlocks::update_type(int index, ValueTag tag) {
  ValueTag slottag = (ValueTag)_tags.at_grow(index, voidTag);
  if (slottag == voidTag) {
    // this slot is currently untyped, so type it
    _tags.at_put(index, tag);
  } else if (slottag != tag) {
    // the slot has conflicting types, so mark it illegal for now.
    _tags.at_put(index, illegalTag);
  } else {
    assert(slottag == tag, "should match");
  }
}

void ScanBlocks::increment_count(ValueTag tag, int index, int count) {
  intStack* ga = get_array(tag);
  int cnt = ga->at_grow(index, 0) + count;
  ga->at_put(index, cnt);
}


int ScanBlocks::count_at(ValueTag tag, int index) const {
  const intStack* ga = get_array(tag);
  if (index < ga->length()) {
    return ga->at(index);
  } else {
    return 0;
  }
}

intStack* ScanBlocks::get_array(ValueTag tag) {
  assert(tag >= 0 && tag < number_of_tags, "out of range");
  return _access_count + tag;
}

const intStack* ScanBlocks::get_array(ValueTag tag) const {
  assert(tag >= 0 && tag < number_of_tags, "out of range");
  return _access_count + tag;
}

// Increment count in _access_count[tag]
void ScanBlocks::accumulate_access(int index, ValueTag tag, int count) {
  increment_count(tag, index, count);
  update_type(index, tag);
  if (tag == doubleTag || tag == longTag) {
    update_type(index + 1, tag);
  }
}


// Here we set all the flags
void ScanBlocks::scan_block(BlockBegin* block, ScanResult* desc, bool live_only) {
  for (Instruction* n = block; n != NULL; n = n->next()) {
    if (live_only && !n->is_pinned() && (n->use_count() ==  0)) {
      // don't look at unused instructions because no code is emitted for them
      continue;
    }

    ValueTag tag = n->type()->tag();
    if (tag == floatTag) desc->set_has_floats(true);
    else if (tag == doubleTag) desc->set_has_doubles(true);
    if (n->as_StateSplit() != NULL) {
      if (n->as_Invoke() != NULL) {
        desc->set_has_calls(true);
      } else if (n->as_NewArray() || n->as_NewInstance() || n->as_AccessMonitor()) {
        desc->set_has_slow_cases(true);
      }  else if(n->as_Intrinsic() != NULL) {
        Intrinsic* i = n->as_Intrinsic();
        if (i->id() == methodOopDesc::_arraycopy) desc->set_has_slow_cases(true);
        if (!i->preserves_state()) desc->set_has_calls(true);
      }
    } else if (n->as_AccessField() != NULL) {
      AccessField* af = n->as_AccessField();
      if (!af->is_initialized() || !af->is_loaded()) desc->set_has_class_init(true);
    } else if (n->as_AccessLocal() != NULL) {
      AccessLocal* local = n->as_AccessLocal();
      StoreLocal* store = n->as_StoreLocal();
      int use_count = 0;
      if (store != NULL) {
        if (!store->is_eliminated()) {
          use_count = 1;
        }
      } else {
        use_count = n->use_count();
      }
      if (use_count > 0) {
        ValueType* type = local->type();
        assert(local->has_offset(), "must have had offset allocated");
        accumulate_access(in_words(local->offset()), tag, use_count);
      }
    }
#ifdef SPARC
    else {
      if (n->as_Convert() != NULL) {
        Convert* conv = n->as_Convert();
        switch (conv->op()) {
          case Bytecodes::_l2f: 
          case Bytecodes::_l2d: 
          case Bytecodes::_f2l: 
          case Bytecodes::_d2l: 
          case Bytecodes::_d2i: { desc->set_has_calls(true); break; }
        }
      } else if (n->as_ArithmeticOp() != NULL) {
        ArithmeticOp* arith = n->as_ArithmeticOp();
        switch (arith->op()) {
          case Bytecodes::_lrem:  
          case Bytecodes::_ldiv:  
          case Bytecodes::_lmul: 
          case Bytecodes::_drem: 
          case Bytecodes::_frem: { desc->set_has_calls(true); break; }
        }
      }
    }
#endif
  }
}


ScanBlocks::ScanBlocks(BlockList* blocks) {
  _blocks = blocks;
}


void ScanBlocks::scan(ScanResult* desc, bool live_only) {
  int n = _blocks->length() - 1;
  for (; n >= 0; n--) {
    scan_block(_blocks->at(n), desc, live_only);
  }
}


// Sort float locals by their usage
ALocalList* ScanBlocks::most_used_float_locals() {
  // check that no local is object and float
  int lng = MAX3(_access_count[floatTag].length(), _access_count[doubleTag].length(), _access_count[intTag].length());
  ALocalList* res = new ALocalList(lng);
  // build array of ALocal's if they are exclusively float's or double's
  for (int i = 0; i < lng; i++) {
    int fcnt = float_count_at(i);
    int dcnt = double_count_at(i);
    ALocal* alocal = NULL;
    if (fcnt > 0 && is_float_only(i)) {
      alocal = new ALocal(i, floatTag, fcnt);
    } else if (dcnt > 0 && is_double_only(i)) {
      alocal = new ALocal(i, doubleTag, dcnt);
    }
    if (alocal != NULL) res->append(alocal);
  }

  res->sort(ALocal::sort_by_access_count);
  return res;
}


int ScanBlocks::int_count_at(int index) const {
  return count_at(intTag, index);
}


int ScanBlocks::long_count_at(int index) const {
  return count_at(longTag, index);
}


int ScanBlocks::float_count_at(int index) const {
  return count_at(floatTag, index);
}


int ScanBlocks::double_count_at(int index) const {
  return count_at(doubleTag, index);
}


int ScanBlocks::obj_count_at(int index) const {
  return count_at(objectTag, index);
}


int ScanBlocks::address_count_at(int index) const {
  return count_at(addressTag, index);
}


// Sorts all CPU locals by their usage; access frequency is stored separately for
// objects and integers
ALocalList* ScanBlocks::most_used_locals() {
  // check that no local is object and integer
  int lng = MAX3(_access_count[intTag].length(), _access_count[longTag].length(), _access_count[objectTag].length());
  ALocalList* res = new ALocalList(lng);
  // build array of ALocal's if they are exclusively obj or int's
  for (int i = 0; i < lng; i++) {
    int icnt = int_count_at(i);
    int lcnt = long_count_at(i);
    int ocnt = obj_count_at(i);
    ALocal* alocal = NULL;
    if (icnt > 0 && is_int_only(i)) {
      alocal = new ALocal(i, intTag, icnt);
    } else if (lcnt > 0 && is_long_only(i)) {
      alocal = new ALocal(i, longTag, lcnt);
    } else if (ocnt > 0 && is_obj_only(i)) {
      alocal = new ALocal(i, objectTag, ocnt);
    }
    if (alocal != NULL) res->append(alocal);
  }

  res->sort(ALocal::sort_by_access_count);
  return res;
}

bool ScanBlocks::is_int_only(int index) const {
  if (index < _tags.length()) {
    return _tags.at(index) == intTag;
  }
  return false;
}


bool ScanBlocks::is_long_only(int index) const {
  if (index + 1 < _tags.length()) {
    return _tags.at(index) == longTag &&
      _tags.at(index + 1) == longTag &&
      long_count_at(index + 1) == 0;
  }
  return false;
}


bool ScanBlocks::is_float_only(int index) const {
  if (index < _tags.length()) {
    return _tags.at(index) == floatTag;
  }
  return false;
}


bool ScanBlocks::is_double_only(int index) const {
  if (index + 1 < _tags.length()) {
    return _tags.at(index) == doubleTag &&
      _tags.at(index + 1) == doubleTag &&
      double_count_at(index + 1) == 0;
  }
  return false;
}


bool ScanBlocks::is_obj_only(int index) const {
  if (index < _tags.length()) {
    return _tags.at(index) == objectTag;
  }
  return false;
}


bool ScanBlocks::is_address_only(int index) const {
  if (index < _tags.length()) {
    return _tags.at(index) == addressTag;
  }
  return false;
}


#ifndef PRODUCT
void ScanBlocks::print_access_count(const char* label, intStack* count) {
  if (count->length() > 0) {
    tty->print(" %s[", label);
    int i;
    for (i = 0; i < count->length(); i++) {
      if (count->at(i))
        tty->print("L%d:%d ", i, count->at(i));
    }
    tty->print("]");
  }
}

void ScanBlocks::print(ScanResult* r) {
  tty->print("Scan block analysis: ");
  if (r != NULL) {
    if (r->has_calls()) tty->print(" calls "); 
    if (r->has_slow_cases()) tty->print(" slow-case "); 
    if (r->has_floats()) tty->print(" floats "); 
    if (r->has_doubles()) tty->print(" doubles "); 
    if (r->has_class_init()) tty->print(" class-init ");
  }
  tty->print("; ");
  print_access_count("INT",    get_array(intTag));
  print_access_count("OBJ",    get_array(objectTag));
  print_access_count("LONG",   get_array(longTag));
  print_access_count("FLOAT",  get_array(floatTag));
  print_access_count("DOUBLE", get_array(doubleTag));
  tty->cr();
}
#endif

