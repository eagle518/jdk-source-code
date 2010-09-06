#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)c1_RInfo_sparc.cpp	1.15 03/12/23 16:37:04 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_c1_RInfo_sparc.cpp.incl"


//-------------------------------------------------------
//                 RInfo
//-------------------------------------------------------

FloatRegister RInfo::as_float_reg() const {
  return FrameMap::nr2floatreg(float_reg());
}

FloatRegister RInfo::as_double_reg() const {
  return FrameMap::nr2floatreg(double_reg());
}

void RInfo::set_float_reg(const FloatRegister& f) {
  set_float_reg(f->encoding(FloatRegisterImpl::S));
}

void RInfo::set_double_reg(const FloatRegister& d) { 
  assert((d->encoding(FloatRegisterImpl::S) + 1) == d->successor()->encoding(FloatRegisterImpl::S) &&
         (d->encoding(FloatRegisterImpl::S) == d->encoding(FloatRegisterImpl::D)),
         "should be sequential");
  set_double_reg(d->encoding(FloatRegisterImpl::D));
}
