/*
 * Copyright (c) 2001, 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */

# include "incls/_precompiled.incl"
# include "incls/_intHisto.cpp.incl"

IntHistogram::IntHistogram(int est, int max) : _max(max), _tot(0) {
  assert(0 <= est && est <= max, "Preconditions");
  _elements = new (ResourceObj::C_HEAP) GrowableArray<int>(est, true);
  guarantee(_elements != NULL, "alloc failure");
}

void IntHistogram::add_entry(int outcome) {
  if (outcome > _max) outcome = _max;
  int new_count = _elements->at_grow(outcome) + 1;
  _elements->at_put(outcome, new_count);
  _tot++;
}

int IntHistogram::entries_for_outcome(int outcome) {
  return _elements->at_grow(outcome);
}

void IntHistogram::print_on(outputStream* st) const {
  double tot_d = (double)_tot;
  st->print_cr("Outcome     # of occurrences   %% of occurrences");
  st->print_cr("-----------------------------------------------");
  for (int i=0; i < _elements->length()-2; i++) {
    int cnt = _elements->at(i);
    if (cnt != 0) {
      st->print_cr("%7d        %10d         %8.4f",
                   i, cnt, (double)cnt/tot_d);
    }
  }
  // Does it have any max entries?
  if (_elements->length()-1 == _max) {
    int cnt = _elements->at(_max);
    st->print_cr(">= %4d        %10d         %8.4f",
                 _max, cnt, (double)cnt/tot_d);
  }
  st->print_cr("-----------------------------------------------");
  st->print_cr("    All        %10d         %8.4f", _tot, 1.0);
}
