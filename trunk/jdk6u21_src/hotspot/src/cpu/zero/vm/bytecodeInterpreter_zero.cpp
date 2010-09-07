/*
 * Copyright (c) 2003, 2007, Oracle and/or its affiliates. All rights reserved.
 * Copyright 2008 Red Hat, Inc.
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

#include "incls/_precompiled.incl"
#include "incls/_cppInterpreter_zero.cpp.incl"

#ifdef CC_INTERP

const char *BytecodeInterpreter::name_of_field_at_address(address addr) {
#define DO(member) {if (addr == (address) &(member)) return XSTR(member);}
  DO(_thread);
  DO(_bcp);
  DO(_locals);
  DO(_constants);
  DO(_method);
  DO(_mdx);
  DO(_stack);
  DO(_msg);
  DO(_result);
  DO(_prev_link);
  DO(_oop_temp);
  DO(_stack_base);
  DO(_stack_limit);
  DO(_monitor_base);
  DO(_self_link);
#undef DO
  if (addr > (address) &_result && addr < (address) (&_result + 1))
    return "_result)";
  return NULL;
}

#endif // CC_INTERP
