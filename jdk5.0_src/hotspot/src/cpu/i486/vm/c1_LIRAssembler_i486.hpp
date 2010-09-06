#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)c1_LIRAssembler_i486.hpp	1.10 03/12/23 16:36:07 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

 private:

  Address::ScaleFactor array_element_size(BasicType type) const;

  void monitorenter(RInfo obj_, RInfo lock_, Register hdr, int monitor_no, CodeEmitInfo* info);
  void monitorexit(RInfo obj_reg, RInfo lock_reg, Register new_hdr, int monitor_no, Register exception);

  Address as_Address_lo(LIR_Address* addr);
  Address as_Address_hi(LIR_Address* addr);

  void maybe_adjust_stack_alignment(ciMethod* method);

public:
  bool is_empty_fpu_stack() const                { return frame_map()->fpu_stack()->is_empty(); }
  int  fpu_stack_size() const                    { return frame_map()->fpu_stack()->stack_size(); }

