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

// This class implements a simple histogram.

// A histogram summarizes a series of "measurements", each of which is
// assumed (required in this implementation) to have an outcome that is a
// non-negative integer.  The histogram efficiently maps measurement outcomes
// to the number of measurements had that outcome.

// To print the results, invoke print() on your Histogram*.

// Note: there is already an existing "Histogram" class, in file
// histogram.{hpp,cpp}, but to my mind that's not a histogram, it's a table
// mapping strings to counts.  To be a histogram (IMHO) it needs to map
// numbers (in fact, integers) to number of occurrences of that number.

// ysr: (i am not sure i agree with the above note.) i suspect we want to have a
// histogram template that will map an arbitrary type (with a defined order
// relation) to a count.


class IntHistogram : public CHeapObj {
 protected:
  int _max;
  int _tot;
  GrowableArray<int>* _elements;

public:
  // Create a new, empty table.  "est" is an estimate of the maximum outcome
  // that will be added, and "max" is an outcome such that all outcomes at
  // least that large will be bundled with it.
  IntHistogram(int est, int max);
  // Add a measurement with the given outcome to the sequence.
  void add_entry(int outcome);
  // Return the number of entries recorded so far with the given outcome.
  int  entries_for_outcome(int outcome);
  // Return the total number of entries recorded so far.
  int  total_entries() { return _tot; }
  // Return the number of entries recorded so far with the given outcome as
  // a fraction of the total number recorded so far.
  double fraction_for_outcome(int outcome) {
    return
      (double)entries_for_outcome(outcome)/
      (double)total_entries();
  }
  // Print the histogram on the given output stream.
  void print_on(outputStream* st) const;
};
