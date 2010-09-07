/*
 * Copyright (c) 2003, 2007, Oracle and/or its affiliates. All rights reserved.
 * Copyright 2008, 2009 Red Hat, Inc.
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

 protected:
  MacroAssembler* assembler() const {
    return _masm;
  }

 protected:
  address generate_entry(address entry_point) {
    ZeroEntry *entry = (ZeroEntry *) assembler()->pc();
    assembler()->advance(sizeof(ZeroEntry));
    entry->set_entry_point(entry_point);
    return (address) entry;
  }
