/*
 * Copyright (c) 2006, 2007, Oracle and/or its affiliates. All rights reserved.
 * Copyright 2007 Red Hat, Inc.
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

inline VMReg RegisterImpl::as_VMReg() {
  return VMRegImpl::as_VMReg(encoding());
}

inline VMReg FloatRegisterImpl::as_VMReg() {
  return VMRegImpl::as_VMReg(encoding() + ConcreteRegisterImpl::max_gpr);
}
