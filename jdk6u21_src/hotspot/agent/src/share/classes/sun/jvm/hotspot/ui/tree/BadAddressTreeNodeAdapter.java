/*
 * Copyright (c) 2004, Oracle and/or its affiliates. All rights reserved.
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

package sun.jvm.hotspot.ui.tree;

import java.io.*;
import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.oops.*;

/** Simple wrapper for displaying bad addresses in the Inspector */

public class BadAddressTreeNodeAdapter extends FieldTreeNodeAdapter {
  private boolean usingAddress;
  private Address addr;
  private long    addrValue;

  public BadAddressTreeNodeAdapter(Address addr, FieldIdentifier id) {
    this(addr, id, false);
  }

  /** The address may be null (for address fields of structures which
      are null); the FieldIdentifier may also be null (for the root
      node). */
  public BadAddressTreeNodeAdapter(Address addr, FieldIdentifier id, boolean treeTableMode) {
    super(id, treeTableMode);
    this.addr = addr;
    usingAddress = true;
  }

  public BadAddressTreeNodeAdapter(long addr, FieldIdentifier id) {
    this(addr, id, false);
  }

  /** He FieldIdentifier may be null (for the root node). */
  public BadAddressTreeNodeAdapter(long addrValue, FieldIdentifier id, boolean treeTableMode) {
    super(id, treeTableMode);
    this.addrValue = addrValue;
    usingAddress = false;
  }

  public int getChildCount() {
    return 0;
  }

  public SimpleTreeNode getChild(int index) {
    throw new RuntimeException("Should not call this");
  }

  public boolean isLeaf() {
    return true;
  }

  public int getIndexOfChild(SimpleTreeNode child) {
    throw new RuntimeException("Should not call this");
  }

  public String getValue() {
    // FIXME: should have this better factored to not have to replicate this code
    String addrString = null;
    if (usingAddress) {
      if (addr == null) {
        addrString = "0x0";
      } else {
        addrString = addr.toString();
      }
    } else {
      addrString = "0x" + Long.toHexString(addrValue);
    }
    return "** BAD ADDRESS " + addrString + " **";
  }
}
