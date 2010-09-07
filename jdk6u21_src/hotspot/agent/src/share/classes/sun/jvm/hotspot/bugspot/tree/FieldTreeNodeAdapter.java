/*
 * Copyright (c) 2001, Oracle and/or its affiliates. All rights reserved.
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

package sun.jvm.hotspot.bugspot.tree;

import sun.jvm.hotspot.debugger.cdbg.*;
import sun.jvm.hotspot.ui.tree.SimpleTreeNode;

/** Abstract base class for all adapters for fields of C/C++ objects */

public abstract class FieldTreeNodeAdapter implements SimpleTreeNode {
  private FieldIdentifier id;
  private boolean         treeTableMode;

  /** The identifier may be null, i.e., for the root of the tree */
  public FieldTreeNodeAdapter(FieldIdentifier id, boolean treeTableMode) {
    this.id = id;
    this.treeTableMode = treeTableMode;
  }

  public FieldIdentifier getID() {
    return id;
  }

  /** Defaults to false in subclasses */
  public boolean getTreeTableMode() {
    return treeTableMode;
  }

  public Type getType() {
    return getID().getType();
  }

  public String getName() {
    if (getID() != null) {
      return getID().toString();
    }
    return "";
  }

  public String toString() {
    if (treeTableMode) {
      return getName();
    } else {
      if (getID() != null) {
        return getName() + ": " + getValue();
      } else {
        return getValue();
      }
    }
  }
}
