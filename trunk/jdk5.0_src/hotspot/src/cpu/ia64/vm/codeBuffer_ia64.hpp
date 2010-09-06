#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)codeBuffer_ia64.hpp	1.17 03/12/23 16:36:37 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

//=============================================================================
// Support for IPF-specific support for code buffer. This specifically
// includes support for bundling.
//=============================================================================

  // This subclass describes one instruction to be emitted in an instruction
  // group. An array of instructions is used in the IPF-specific code to
  // represent the accumulated instructions to be placed in an instruction
  // group.

  class BundleInst {
  private:
    uint41_t             _inst;  // Buffered instruction bits
    Inst::Kind           _kind;  // Corresponding instruction kind
    Label *              _label; // Corresponding Label of target for branch
    address              _addr;  // Corresponding Address
    relocInfo::relocType _reloc; // Corresponding Relocation Type

    bool                 _is_ordered  : 1,
                         _has_label   : 1,
                         _has_address : 1,
                         _has_reloc   : 1;

    // This is only needed for debug purposes
    uint8_t              _order; // Original order in the sequence of instructions

  public:
    // Accessor methods
    uint41_t&            inst()        { return _inst; }
    Inst::Kind           kind()  const { return _kind; }
    Label*               label() const { assert(_has_label,   "no label");   return _label; }
    address              addr()  const { assert(_has_address, "no address"); return _addr; }
    relocInfo::relocType reloc() const { assert(_has_reloc,   "no reloc");   return _reloc; }
    uint8_t              order() const { return _order; }

    // indicate if optional data exists
    bool           is_ordered()  const { return _is_ordered; }
    bool           has_label()   const { return _has_label; }
    bool           has_address() const { return _has_address; }
    bool           has_reloc()   const { return _has_reloc; }

    void set(uint41_t inst, Inst::Kind kind, bool is_ordered, uint order) {
      _inst       = inst;
      _kind       = kind;
      _is_ordered = is_ordered;
      _order      = order;
      _has_label  = _has_address = _has_reloc = false;
    }

    void set_inst(uint41_t inst) {
      _inst = inst;
    }

    void set_label(Label *label) {
      _label = label;
      _has_label = true;
    }

    void set_addr(address addr) {
      _addr = addr;
      _has_address = true;
    }

    void set_reloc(relocInfo::relocType reloc) {
      _reloc = reloc;
      _has_reloc = true;
    }
  };

private:

  // This describes the maximum number of instructions allowed in an
  // instruction group, and the maximum number of bundles
  enum {
    MaxInsts      = 6,
    MaxSetOfInsts = 2
  };

  // Information to maintain the state of the current
  // registers read and written in this bundle

  RegisterState _current_read;
  RegisterState _current_write;

  BundleInst    _element[MaxInsts]; // Information about each instruction

  // Number of each kind of buffered instruction in _accum_insts.
  // Sum of the elements == _accum_count.
  uint8_t       _accum_kind_count[Inst::Kind_count];

  // Current instruction state
  uint8_t       _inst_count;
  uint8_t       _slot;
  bool          _is_ordered;
  bool          _explicit_bundling;
  bool          _pr_fast_write_only;
  bool          _br_fast_write_only;

  // Previous bundle information, supporting split bundles
  // This really contains a format that may be split, iff it is possible
  IPF_Bundle::Template _previous_bundle_format;

  // Reset the bundle state, usually after emitting a bundle
  void reset_bundle_state() {
    // No buffered instructions
    _accum_kind_count[Inst::M_inst] = 0;
    _accum_kind_count[Inst::F_inst] = 0;
    _accum_kind_count[Inst::I_inst] = 0;
    _accum_kind_count[Inst::B_inst] = 0;
    _accum_kind_count[Inst::L_inst] = 0;
    _accum_kind_count[Inst::X_inst] = 0;
    _accum_kind_count[Inst::A_inst] = 0;

    _inst_count = 0;

    _current_read .clear();
    _current_write.clear();

    _previous_bundle_format = IPF_Bundle::none;

    // Instructions in the next bundle are not a priori ordered
    // until we see an instruction that makes them so.
    clear_is_ordered();

    // Until a branch register is written, assume fast
    // branch register reads and writes
    set_is_br_fast_write_only();

    // Until a predicate register is written, assume fast
    // predicate register reads and writes
    set_is_pr_fast_write_only();
  }

  void pd_initialize() {
    reset_bundle_state();
    check_bundling();
  }

  // Indicate if the bundle contains any ordered instructions
  // (i.e., instructions that force ordering within a bundle)
  bool is_ordered() const { return (bool)_is_ordered; }
  void set_is_ordered()   { _is_ordered = true; }
  void clear_is_ordered() { _is_ordered = false; }

  // Indicate if all branch register writes can be read by
  // branches in the same cycle
  bool is_br_fast_write_only() const { return (bool)_br_fast_write_only; }
  void set_is_br_fast_write_only()   { _br_fast_write_only = true; }
  void clear_is_br_fast_write_only() { _br_fast_write_only = false; }

  // Indicate if all predicate register writes can be read by
  // branches in the same cycle
  bool is_pr_fast_write_only() const { return (bool)_pr_fast_write_only; }
  void set_is_pr_fast_write_only()   { _pr_fast_write_only = true; }
  void clear_is_pr_fast_write_only() { _pr_fast_write_only = false; }

  uint FindBundleSet(Inst &inst);

public:

  // Add an instruction to the code stream, starting a new bundle if necessary.
  void emit(Inst&   inst);
  void emit(Inst&   inst, Label*  target);
  void emit(Inst&   inst, address target, relocInfo::relocType rtype);
  void emit(X_Inst& inst);
  void emit(X_Inst& inst, Label* target);
  void emit(X_Inst& inst, address target, relocInfo::relocType rtype);

  // Flush/freeze the bundle cache
  void flush_bundle(bool start_new_bundle = true);

  // Set explicit bundling (manually flush bundles) or use checking
  void explicit_bundling() { _explicit_bundling = true; }
  void check_bundling()    { _explicit_bundling = false; }

  // Get or set the slot number
  void set_code_slot(uint slot) { _slot = slot; }
  uint code_slot() const { return _slot; }

  // Compute the encoded value for a Label
  int encoded_target(Label *L);

  // Attempt to insert an instruction into the bundle
  uint insert(X_Inst& inst);
  uint insert(Inst& inst);

  // Copy the CPU-specific data from another CodeBuffer
  void getCpuData(const CodeBuffer * const cb) {
    _current_read           = cb->_current_read;
    _current_write          = cb->_current_write;

    for (int i1 = 0; i1 < (int)MaxInsts; i1++)
      _element[i1]          = cb->_element[i1];

    for (int i2 = 0; i2 < (int)Inst::Kind_count; i2++)
      _accum_kind_count[i2] = cb->_accum_kind_count[i2];

    _inst_count             = cb->_inst_count;
    _slot                   = cb->_slot;
    _is_ordered             = cb->_is_ordered;
    _explicit_bundling      = cb->_explicit_bundling;
    _pr_fast_write_only     = cb->_pr_fast_write_only;
    _br_fast_write_only     = cb->_br_fast_write_only;

    _previous_bundle_format = cb->_previous_bundle_format;
  }

#ifndef PRODUCT
  void print_state();
#endif
