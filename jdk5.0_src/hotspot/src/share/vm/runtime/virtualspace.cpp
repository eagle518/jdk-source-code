#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)virtualspace.cpp	1.51 03/12/23 16:44:27 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_virtualspace.cpp.incl"


// ReservedSpace
ReservedSpace::ReservedSpace(size_t size) {
  initialize(size, 0, false, NULL);
}

ReservedSpace::ReservedSpace(size_t size, size_t forced_base_alignment,
                             bool ism, char* requested_address) {
  initialize(size, forced_base_alignment, ism, requested_address);
}

void ReservedSpace::initialize(size_t size, size_t forced_base_alignment,
                               bool ism, char* requested_address) {

  assert(size % os::vm_allocation_granularity() == 0,
         "size not allocation aligned");
  assert((forced_base_alignment % os::vm_allocation_granularity()) == 0,
         "size not allocation aligned");
  _base = NULL;
  _size = 0;
  if (size == 0) {
    return;
  }

  char* base;

  // ISM constructor 
  if (ism) { 
    assert(UseISM || UsePermISM, 
	   "ISM constructor used although UseISM or UsePermISM is not on."); 
    base = os::reserve_memory_special(size); 
    
    if (base == NULL) return;
    
    // Check alignment constraints
    if (forced_base_alignment > 0) {
      assert((uintptr_t) base % forced_base_alignment == 0, 
	     "Large pages returned a non-aligned address"); 
    }
  } else {
    // Optimistically assume that the OSes returns an aligned base pointer.
    // When reserving a large address range, most OSes seem to align to at 
    // least 64K.

    // If the memory was requested at a particular address, use
    // os::attempt_reserve_memory_at() to avoid over mapping something
    // important.  If available space is not detected, return NULL.

    if (requested_address != 0) {
      base = os::attempt_reserve_memory_at(size, requested_address);
    } else {
      base = os::reserve_memory(size, NULL);
    }

    if (base == NULL) return;

    // Check alignment constraints
    if (forced_base_alignment > 0 && ((uintptr_t) base % forced_base_alignment != 0)) {
      // Base not aligned, retry
      if (!os::release_memory(base, size)) fatal("os::release_memory failed");
      // Reserve size large enough to do manual alignment
      size_t extra_size = size + forced_base_alignment;
      char* extra_base = os::reserve_memory(extra_size);
      if (extra_base == NULL) return;
      // Do manual alignement
      base = (char*) align_size_up((uintptr_t) extra_base, forced_base_alignment);
      assert(base >= extra_base, "just checking");
      // Release unused areas
      size_t unused_bottom_size = base - extra_base;
      size_t unused_top_size = extra_size - size - unused_bottom_size;
      assert(unused_bottom_size % os::vm_allocation_granularity() == 0,
             "size not allocation aligned");
      assert(unused_top_size % os::vm_allocation_granularity() == 0,
             "size not allocation aligned");
      if (unused_bottom_size > 0) {
	os::release_memory(extra_base, unused_bottom_size);
      }
      if (unused_top_size > 0) {
	os::release_memory(base + size, unused_top_size);
      }
    }
  }
  // Done
  _base = base;
  _size = size;
  assert(markOopDesc::encode_pointer_as_mark(_base)->decode_pointer() == _base,
	 "area must be distinguisable from marks for mark-sweep");
  assert(markOopDesc::encode_pointer_as_mark(&_base[size])->decode_pointer() == &_base[size],
	 "area must be distinguisable from marks for mark-sweep");
}


ReservedSpace::ReservedSpace(char* base, size_t size) {
  assert((size % os::vm_allocation_granularity()) == 0,
         "size not allocation aligned");
  _base = base;
  _size = size;
}


ReservedSpace ReservedSpace::first_part(size_t partition_size,
                                        bool split, bool realloc) {
  assert(partition_size <= size(), "partition failed");
  if (split) {
    os::split_reserved_memory(_base, _size, partition_size, realloc);
  }
  ReservedSpace result(base(), partition_size);
  return result;
}


ReservedSpace ReservedSpace::last_part(size_t partition_size) {
  assert(partition_size <= size(), "partition failed");
  ReservedSpace result(base() + partition_size, size() - partition_size);
  return result;
}


size_t ReservedSpace::page_align_size_up(size_t size) {
  return align_size_up(size, os::vm_page_size());
}


size_t ReservedSpace::page_align_size_down(size_t size) {
  return align_size_down(size, os::vm_page_size());
}


size_t ReservedSpace::allocation_align_size_up(size_t size) {
  return align_size_up(size, os::vm_allocation_granularity());
}


size_t ReservedSpace::allocation_align_size_down(size_t size) {
  return align_size_down(size, os::vm_allocation_granularity());
}


void ReservedSpace::release() {
  if (is_reserved()) {
    os::release_memory(_base, _size);
    _base = NULL;
    _size = 0;
  }
}


// VirtualSpace

VirtualSpace::VirtualSpace() {
  _low_boundary           = NULL;
  _high_boundary          = NULL;
  _low                    = NULL;
  _high                   = NULL;
  _lower_high             = NULL;
  _middle_high            = NULL;
  _upper_high             = NULL;
  _lower_high_boundary    = NULL;
  _middle_high_boundary   = NULL;
  _upper_high_boundary    = NULL;
  _lower_alignment        = 0;
  _middle_alignment       = 0;
  _upper_alignment        = 0;
}


bool VirtualSpace::initialize(ReservedSpace rs, size_t committed_size) {
  if(!rs.is_reserved()) return false;  // allocation failed.
  assert(_low_boundary == NULL, "VirtualSpace already initialized");
  _low_boundary  = rs.base();
  _high_boundary = low_boundary() + rs.size();

  _low = low_boundary();
  _high = low();
  
  // When a VirtualSpace begins life at a large size, make all future expansion
  // and shrinking occur aligned to a granularity of large pages.  This avoids
  // fragmentation of physical addresses that inhibits the use of large pages
  // by the OS virtual memory system.  Empirically,  we see that with a 4MB 
  // page size, the only spaces that get handled this way are codecache and 
  // the heap itself, both of which provide a substantial performance 
  // boost in many benchmarks when covered by large pages.
  //
  // No attempt is made to force large page alignment at the very top and 
  // bottom of the space if they are not aligned so already. 
  _lower_alignment = os::vm_page_size();
  if (UseMPSS && rs.size() >= (size_t) LargePageSizeInBytes) {
    _middle_alignment = LargePageSizeInBytes;
  } else {
    _middle_alignment = os::vm_page_size();
  }
  _upper_alignment = os::vm_page_size();

  // End of each region
  _lower_high_boundary = (char*) round_to((intptr_t) low_boundary(), middle_alignment());
  _middle_high_boundary = (char*) round_down((intptr_t) high_boundary(), middle_alignment());
  _upper_high_boundary = high_boundary();

  // High address of each region
  _lower_high = low_boundary();
  _middle_high = lower_high_boundary();
  _upper_high = middle_high_boundary();

  // commit to initial size
  if (committed_size > 0) {
    if (!expand_by(committed_size)) {
      return false;
    }
  }
  return true;
}


VirtualSpace::~VirtualSpace() { 
  release();
}


void VirtualSpace::release() {
  (void)os::release_memory(low_boundary(), reserved_size());
  _low_boundary           = NULL;
  _high_boundary          = NULL;
  _low                    = NULL;
  _high                   = NULL;
  _lower_high             = NULL;
  _middle_high            = NULL;
  _upper_high             = NULL;
  _lower_high_boundary    = NULL;
  _middle_high_boundary   = NULL;
  _upper_high_boundary    = NULL;
  _lower_alignment        = NULL;
  _middle_alignment       = NULL;
  _upper_alignment        = NULL;
}


size_t VirtualSpace::committed_size() const { 
  return pointer_delta(high(), low(), sizeof(char));
}


size_t VirtualSpace::reserved_size() const {
  return pointer_delta(high_boundary(), low_boundary(), sizeof(char));
}


size_t VirtualSpace::uncommitted_size()  const {
  return reserved_size() - committed_size();
}


bool VirtualSpace::contains(const void* p) const {
  return low() <= (const char*) p && (const char*) p < high();
}

/* 
   First we need to determine if a particular virtual space is using large
   pages.  This is done at the initialize function and only virtual spaces
   that are larger than LargePageSizeInBytes use large pages.  Once we
   have determined this, all expand_by and shrink_by calls must grow and
   shrink by large page size chunks.  If a particular request 
   is within the current large page, the call to commit and uncommit memory
   can be ignored.  In the case that the low and high boundaries of this
   space is not large page aligned, the pages leading to the first large
   page address and the pages after the last large page address must be
   allocated with default pages.
*/
bool VirtualSpace::expand_by(size_t bytes) {
  if (uncommitted_size() < bytes) return false;

  char* unaligned_new_high = high() + bytes;
  assert(unaligned_new_high <= high_boundary(), 
	 "cannot expand by more than upper boundary");

  // Calculate where the new high for each of the regions should be.  If
  // the low_boundary() and high_boundary() are LargePageSizeInBytes aligned
  // then the unaligned lower and upper new highs would be the 
  // lower_high() and upper_high() respectively.
  char* unaligned_lower_new_high = 
    MIN2(unaligned_new_high, lower_high_boundary());
  char* unaligned_middle_new_high = 
    MIN2(unaligned_new_high, middle_high_boundary());
  char* unaligned_upper_new_high = 
    MIN2(unaligned_new_high, upper_high_boundary());

  // Align the new highs based on the regions alignment.  lower and upper 
  // alignment will always be default page size.  middle alignment will be
  // LargePageSizeInBytes if the actual size of the virtual space is in
  // fact larger than LargePageSizeInBytes.
  char* aligned_lower_new_high = 
    (char*) round_to((intptr_t) unaligned_lower_new_high, lower_alignment());
  char* aligned_middle_new_high =
    (char*) round_to((intptr_t) unaligned_middle_new_high, middle_alignment());
  char* aligned_upper_new_high =
    (char*) round_to((intptr_t) unaligned_upper_new_high, upper_alignment());

  // Determine which regions need to grow in this expand_by call.
  // If you are growing in the lower region, high() must be in that
  // region so calcuate the size based on high().  For the middle and
  // upper regions, determine the starting point of growth based on the 
  // location of high().  By getting the MAX of the region's low address 
  // (or the prevoius region's high address) and high(), we can tell if it
  // is an intra or inter region growth.
  size_t lower_needs = 0;
  if (aligned_lower_new_high > lower_high()) {
    lower_needs =
      pointer_delta(aligned_lower_new_high, lower_high(), sizeof(char));
  }
  size_t middle_needs = 0;
  if (aligned_middle_new_high > middle_high()) {
    middle_needs =
      pointer_delta(aligned_middle_new_high, middle_high(), sizeof(char));
  }
  size_t upper_needs = 0;
  if (aligned_upper_new_high > upper_high()) {
    upper_needs =
      pointer_delta(aligned_upper_new_high, upper_high(), sizeof(char));
  }

  // Check contiguity.
  assert(low_boundary() <= lower_high() && 
	 lower_high() <= lower_high_boundary(),
	 "high address must be contained within the region");
  assert(lower_high_boundary() <= middle_high() && 
	 middle_high() <= middle_high_boundary(),
	 "high address must be contained within the region");
  assert(middle_high_boundary() <= upper_high() &&
	 upper_high() <= upper_high_boundary(), 
	 "high address must be contained within the region");

  // Commit regions
  if (lower_needs > 0) {
    assert(low_boundary() <= lower_high() &&
	   lower_high() + lower_needs <= lower_high_boundary(), 
	   "must not expand beyond region");
    if (!os::commit_memory(lower_high(), lower_needs)) {
      debug_only(warning("os::commit_memory failed"));
      return false;
    } else {
      _lower_high += lower_needs;
    }
  }
  if (middle_needs > 0) {
    assert(lower_high_boundary() <= middle_high() &&
	   middle_high() + middle_needs <= middle_high_boundary(), 
	   "must not expand beyond region");
    if (!os::commit_memory(middle_high(), middle_needs, middle_alignment())) {
      debug_only(warning("os::commit_memory failed"));
      return false;
    } 
    _middle_high += middle_needs;
  }
  if (upper_needs > 0) {
    assert(middle_high_boundary() <= upper_high() &&
	   upper_high() + upper_needs <= upper_high_boundary(), 
	   "must not expand beyond region");
    if (!os::commit_memory(upper_high(), upper_needs)) {
      debug_only(warning("os::commit_memory failed"));
      return false;
    } else {
      _upper_high += upper_needs;
    }
  }
  
  _high += bytes;
  return true;
}

// A page is uncommitted if the contents of the entire page is deemed unusable.
// Continue to decrement the high() pointer until it reaches a page boundary
// in which case that particular page can now be uncommitted.
void VirtualSpace::shrink_by(size_t size) {
  if (committed_size() < size) 
    fatal("Cannot shrink virtual space to negative size");

  char* unaligned_new_high = high() - size;
  assert(unaligned_new_high >= low_boundary(), "cannot shrink past lower boundary");

  // Calculate new unaligned address
  char* unaligned_upper_new_high = 
    MAX2(unaligned_new_high, middle_high_boundary());
  char* unaligned_middle_new_high = 
    MAX2(unaligned_new_high, lower_high_boundary());
  char* unaligned_lower_new_high = 
    MAX2(unaligned_new_high, low_boundary());

  // Align address to region's alignment
  char* aligned_upper_new_high = 
    (char*) round_to((intptr_t) unaligned_upper_new_high, upper_alignment());
  char* aligned_middle_new_high = 
    (char*) round_to((intptr_t) unaligned_middle_new_high, middle_alignment());
  char* aligned_lower_new_high = 
    (char*) round_to((intptr_t) unaligned_lower_new_high, lower_alignment());
  
  // Determine which regions need to shrink
  size_t upper_needs = 0;
  if (aligned_upper_new_high < upper_high()) {
    upper_needs = 
      pointer_delta(upper_high(), aligned_upper_new_high, sizeof(char));
  }
  size_t middle_needs = 0;
  if (aligned_middle_new_high < middle_high()) {
    middle_needs = 
      pointer_delta(middle_high(), aligned_middle_new_high, sizeof(char));
  }
  size_t lower_needs = 0;
  if (aligned_lower_new_high < lower_high()) {
    lower_needs = 
      pointer_delta(lower_high(), aligned_lower_new_high, sizeof(char));
  }
  
  // Check contiguity.
  assert(middle_high_boundary() <= upper_high() &&
	 upper_high() <= upper_high_boundary(), 
	 "high address must be contained within the region");
  assert(lower_high_boundary() <= middle_high() && 
	 middle_high() <= middle_high_boundary(),
	 "high address must be contained within the region");
  assert(low_boundary() <= lower_high() && 
	 lower_high() <= lower_high_boundary(),
	 "high address must be contained within the region");

  // Uncommit
  if (upper_needs > 0) {
    assert(middle_high_boundary() <= aligned_upper_new_high &&
	   aligned_upper_new_high + upper_needs <= upper_high_boundary(), 
	   "must not shrink beyond region");
    if (!os::uncommit_memory(aligned_upper_new_high, upper_needs)) {
      debug_only(warning("os::uncommit_memory failed"));
      return;
    } else {
      _upper_high -= upper_needs;
    }
  } 
  if (middle_needs > 0) {
    assert(lower_high_boundary() <= aligned_middle_new_high &&
	   aligned_middle_new_high + middle_needs <= middle_high_boundary(), 
	   "must not shrink beyond region");
    if (!os::uncommit_memory(aligned_middle_new_high, middle_needs)) {
      debug_only(warning("os::uncommit_memory failed"));
      return;
    } else {
      _middle_high -= middle_needs;
    }
  }
  if (lower_needs > 0) {
    assert(low_boundary() <= aligned_lower_new_high &&
	   aligned_lower_new_high + lower_needs <= lower_high_boundary(), 
	   "must not shrink beyond region");
    if (!os::uncommit_memory(aligned_lower_new_high, lower_needs)) {
      debug_only(warning("os::uncommit_memory failed"));
      return;
    } else {
      _lower_high -= lower_needs;
    }
  }
  
  _high -= size;
}

#ifndef PRODUCT
void VirtualSpace::check_for_contiguity() {
  // Check contiguity.
  assert(low_boundary() <= lower_high() && 
         lower_high() <= lower_high_boundary(),
         "high address must be contained within the region");
  assert(lower_high_boundary() <= middle_high() && 
         middle_high() <= middle_high_boundary(),
         "high address must be contained within the region");
  assert(middle_high_boundary() <= upper_high() &&
         upper_high() <= upper_high_boundary(),
         "high address must be contained within the region");
  assert(low() >= low_boundary(), "low");
  assert(low_boundary() <= lower_high_boundary(), "lower high boundary");
  assert(upper_high_boundary() <= high_boundary(), "upper high boundary");
  assert(high() <= upper_high(), "upper high");
}

void VirtualSpace::print() {  
  tty->print_cr("Virtual space:");
  tty->print_cr(" - committed: %ld", committed_size());
  tty->print_cr(" - reserved:  %ld", reserved_size());
  tty->print_cr(" - [low, high]:     [" INTPTR_FORMAT ", " INTPTR_FORMAT "]",  low(), high());
  tty->print_cr(" - [low_b, high_b]: [" INTPTR_FORMAT ", " INTPTR_FORMAT "]",  low_boundary(), high_boundary());
}

#endif

