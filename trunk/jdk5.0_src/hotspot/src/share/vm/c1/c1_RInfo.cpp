#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)c1_RInfo.cpp	1.21 03/12/23 16:39:16 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_c1_RInfo.cpp.incl"


//-------------------------------------------------------
//                 RInfo
//-------------------------------------------------------

bool RInfo::overlaps (const RInfo other) const {
  bool overlap = false;
  if (this->is_same(other)) {
    overlap = true;
  } else if (this->is_word()) {
    if (other.is_word()) {
      // they are not the same, so their registers must be different
      assert(this->cpu_regnr() != other.cpu_regnr(), "can't be the same");
    } else if (other.is_long()) {
      overlap = this->cpu_regnr() == other.cpu_regnrLo() ||
                this->cpu_regnr() == other.cpu_regnrHi();
    }
  } else if (this->is_long()) {
    if (other.is_long()) {
      overlap = this->cpu_regnrLo() == other.cpu_regnrLo() ||
                this->cpu_regnrLo() == other.cpu_regnrHi() ||
                this->cpu_regnrHi() == other.cpu_regnrLo() ||
                this->cpu_regnrHi() == other.cpu_regnrHi();
    } else if (other.is_word()) {
      overlap = this->cpu_regnrLo() == other.cpu_regnr() ||
                this->cpu_regnrHi() == other.cpu_regnr();
    }
  } else if (this->is_float()) {
    if (other.is_float()) {
      // they are not the same, so their registers must be different
      assert(this->fpu_regnr() != other.fpu_regnr(), "can't be the same");
    } else if (other.is_double()) {
      overlap = this->fpu_regnr() == other.fpu_regnrLo() ||
                this->fpu_regnr() == other.fpu_regnrHi();
    }
  } else if (this->is_double()) {
    if (other.is_double()) {
      overlap = this->fpu_regnrLo() == other.fpu_regnrLo() ||
                this->fpu_regnrLo() == other.fpu_regnrHi() ||
                this->fpu_regnrHi() == other.fpu_regnrLo() ||
                this->fpu_regnrHi() == other.fpu_regnrHi();
    } else if (other.is_float()) {
      overlap = this->fpu_regnrLo() == other.fpu_regnr() ||
                this->fpu_regnrHi() == other.fpu_regnr();
    }
  }
  return overlap;
}


void RInfo::set_word_reg(const Register r) {
  set_word_reg(FrameMap::cpu_reg2rnr(r));
}

void RInfo::set_long_reg(const Register lo, const Register hi) {
  set_long_reg(FrameMap::cpu_reg2rnr(lo), FrameMap::cpu_reg2rnr(hi));
}

Register RInfo::as_register() const {
  return FrameMap::cpu_rnr2reg(reg());
}

Register RInfo::as_register_lo() const {
  return FrameMap::cpu_rnr2reg(reg_lo());
}

Register RInfo::as_register_hi() const {
  return FrameMap::cpu_rnr2reg(reg_hi());
}


RInfo RInfo::as_rinfo_lo() const {
  RInfo res;
  if (is_long()) {
    res.set_word_reg(reg_lo());
  } else if (is_double()) {
    res.set_float_reg(reg1());
  } else {
    ShouldNotReachHere();
  }
  return res;
}


RInfo RInfo::as_rinfo_hi() const {
  RInfo res;
  if (is_long()) {
    res.set_word_reg(reg_hi());
  } else if (is_double()) {
    res.set_float_reg(reg2());
  } else {
    ShouldNotReachHere();
  }
  return res;
}


#ifndef PRODUCT

void RInfo::print() const {
  if (is_virtual()) {
    if (is_word()) {
      tty->print("R%d", reg1());
    } else if (is_long()) {
      tty->print("R%d,R%d", reg1(), reg2());
    } else if (is_float()) {
      tty->print("F%d", reg1());
    } else if (is_double()) {
      tty->print("F%d,F%d", reg1(), reg2());
    } else {
      ShouldNotReachHere();
    }
  } else if (is_word()) {
    tty->print(as_register()->name());
  } else if (is_long()) {
    const char* nameLO = as_register_lo()->name();
    const char* nameHI = as_register_hi()->name();
    tty->print("(%s,%s)", nameLO, nameHI);
  } else if (is_float() || is_double()) {
    pd_print_fpu();
  } else {
    assert(is_illegal(), "");
    tty->print("Illegal");
  }
}


void RInfo::print_raw() const {
  tty->print_cr(" RInfo - type:%d reg1:%d reg2:%d", type(), reg1(), reg2());
}


void RInfo::print(char* msg) const {
  tty->print("XX %s", msg); print();
}


void RInfoCollection::print() const {
  tty->print("[ ");
  for (int i = 0; i < length(); i++) {
    RInfo reg = at(i);
    if (reg.is_valid()) {
      tty->print("%d=", i); reg.print(); tty->print(" ");
    }
  }
  tty->print_cr("] ");
}


#endif


//--------------------------------------------------------
//               c1_RegMask
//--------------------------------------------------------


int              c1_RegMask::_size      = 0;
#ifdef CONST_SDM_BUG
// This should be const but the "const" provokes a gcc bug id c++/1983
c1_RegMask c1_RegMask::_empty_set = c1_RegMask ();
#else
const c1_RegMask c1_RegMask::_empty_set = c1_RegMask ();
#endif


void c1_RegMask::init_masks(int size) {
  //assert(size > 0 && size <= sizeof(c1_RegMask::_mask) * BitsPerByte, "wrong size");
  _size = size;
}


int c1_RegMask::get_first() const {
  assert(!is_empty(), "");
  for (int rnr = 0; rnr < _size; rnr++) {
    if (contains(rnr)) {
      return rnr;
    }
  }
  return -1;
}


#ifndef PRODUCT

void c1_RegMask::print() const {
  for (int rnr = 0; rnr < _size; rnr++) {
    if (contains(rnr)) {
      RInfo::word_reg(rnr).print();
      tty->print(", ");
    }
  }
}

#endif
