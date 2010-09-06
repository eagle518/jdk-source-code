#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)adjoiningVirtualSpaces.cpp	1.7 03/12/23 16:40:06 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_adjoiningVirtualSpaces.cpp.incl"

AdjoiningVirtualSpaces::AdjoiningVirtualSpaces(ReservedSpace rs,
					       size_t min_low_byte_size,
					       size_t min_high_byte_size,
					       size_t alignment) :
  _reserved_space(rs), _min_low_byte_size(min_low_byte_size),
  _min_high_byte_size(min_high_byte_size), _low(0), _high(0),
  _alignment(alignment) {}

// The maximum byte sizes are for the initial layout of the 
// virtual spaces and are not the limit on the maximum bytes sizes.
void AdjoiningVirtualSpaces::initialize(size_t max_low_byte_size,
					size_t init_low_byte_size,
					size_t init_high_byte_size) {

  // It is not strickly necessary that this class only be used
  // for the boundary moving case but it the expectation now.
  // The more well tested code is still being used otherwise.
  assert(UseAdaptiveSizePolicy && UseAdaptiveGCBoundary, "runtime check");

  // The reserved spaces for the two parts of the virtual space.
  ReservedSpace old_rs   = _reserved_space.first_part(max_low_byte_size);
  ReservedSpace young_rs = _reserved_space.last_part(max_low_byte_size);

  _low = new PSVirtualSpace(old_rs, alignment());
  if (!_low->expand_by(init_low_byte_size)) {
    vm_exit_during_initialization("Could not reserve enough space for "
				  "object heap");
  }

  _high = new PSVirtualSpaceHighToLow(young_rs, alignment());
  if (!_high->expand_by(init_high_byte_size)) {
    vm_exit_during_initialization("Could not reserve enough space for "
				  "object heap");
  }  
}

bool AdjoiningVirtualSpaces::adjust_boundary_up(size_t change_in_bytes) {
  assert(UseAdaptiveSizePolicy && UseAdaptiveGCBoundary, "runtime check");
  size_t actual_change = low()->expand_into(high(), change_in_bytes);
  return actual_change != 0;
}

bool AdjoiningVirtualSpaces::adjust_boundary_down(size_t change_in_bytes) {
  assert(UseAdaptiveSizePolicy && UseAdaptiveGCBoundary, "runtime check");
  size_t actual_change = high()->expand_into(low(), change_in_bytes);
  return actual_change != 0;
}
