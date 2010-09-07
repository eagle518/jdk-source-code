/*
 * Copyright (c) 1998, 2007, Oracle and/or its affiliates. All rights reserved.
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

// machine-dependent implemention for register maps
  friend class frame;

 private:
  intptr_t* _window;         // register window save area (for L and I regs)
  intptr_t* _younger_window; // previous save area (for O regs, if needed)

  address pd_location(VMReg reg) const;
  void pd_clear();
  void pd_initialize_from(const RegisterMap* map) {
    _window         = map->_window;
    _younger_window = map->_younger_window;
    _location_valid[0] = 0;  // avoid the shift_individual_registers game
  }
  void pd_initialize() {
    _window = NULL;
    _younger_window = NULL;
    _location_valid[0] = 0;  // avoid the shift_individual_registers game
  }
  void shift_window(intptr_t* sp, intptr_t* younger_sp) {
    _window         = sp;
    _younger_window = younger_sp;
    // Throw away locations for %i, %o, and %l registers:
    // But do not throw away %g register locs.
    if (_location_valid[0] != 0)  shift_individual_registers();
  }
  void shift_individual_registers();
  // When popping out of compiled frames, we make all IRegs disappear.
  void make_integer_regs_unsaved() { _location_valid[0] = 0; }
