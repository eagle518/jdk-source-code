/*
 * Copyright (c) 2001, 2005, Oracle and/or its affiliates. All rights reserved.
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

class AdaptivePaddedAverage;

class PSPermGen : public PSOldGen {
  friend class VMStructs;
 protected:
  AdaptivePaddedAverage* _avg_size;  // Used for sizing
  size_t _last_used;                 // Amount used at last GC, used for sizing

 public:
  // Initialize the generation.
  PSPermGen(ReservedSpace rs, size_t alignment, size_t initial_byte_size,
            size_t minimum_byte_size, size_t maximum_byte_size,
            const char* gen_name, int level);

  // Permanent Gen special allocation. Uses the OldGen allocation
  // routines, which should not be directly called on this generation.
  HeapWord* allocate_permanent(size_t word_size);

  // Size calculation.
  void compute_new_size(size_t used_before_collection);

  // MarkSweep code
  virtual void precompact();

  // Parallel old
  virtual void move_and_update(ParCompactionManager* cm);

  virtual const char* name() const { return "PSPermGen"; }
};
