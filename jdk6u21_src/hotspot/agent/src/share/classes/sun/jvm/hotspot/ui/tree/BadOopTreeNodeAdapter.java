/*
 * Copyright (c) 2002, Oracle and/or its affiliates. All rights reserved.
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

/** Simple wrapper for displaying bad oops in the Inspector */

public class BadOopTreeNodeAdapter extends FieldTreeNodeAdapter {
  private OopHandle oop;

  public BadOopTreeNodeAdapter(OopHandle oop, FieldIdentifier id) {
    this(oop, id, false);
  }

  /** The oop may be null (for oop fields of oops which are null); the
      FieldIdentifier may also be null (for the root node). */
  public BadOopTreeNodeAdapter(OopHandle oop, FieldIdentifier id, boolean treeTableMode) {
    super(id, treeTableMode);
    this.oop = oop;
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
    return "** BAD OOP " + oop + " **";
  }
}
