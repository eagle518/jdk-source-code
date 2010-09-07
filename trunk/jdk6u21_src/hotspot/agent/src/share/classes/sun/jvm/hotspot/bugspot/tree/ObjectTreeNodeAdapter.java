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

import java.io.*;
import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.debugger.cdbg.*;
import sun.jvm.hotspot.ui.tree.SimpleTreeNode;

/** An adapter class which allows C/C++ objects to be displayed in a
    tree via the SimpleTreeNode interface. */

public class ObjectTreeNodeAdapter extends FieldTreeNodeAdapter {
  // Address of object
  private Address addr;

  /** The address may be null (for object fields of objcets which are
      null). The FieldIdentifier should not be null. treeTableMode
      defaults to false. */
  public ObjectTreeNodeAdapter(Address addr, FieldIdentifier id) {
    this(addr, id, false);
  }

  /** The address may be null (for object fields of objcets which are
      null). The FieldIdentifier should not be null. */
  public ObjectTreeNodeAdapter(Address addr, FieldIdentifier id, boolean treeTableMode) {
    super(id, treeTableMode);
    this.addr = addr;
  }

  public int getChildCount() {
    if (addr == null) {
      return 0;
    }

    Counter c = new Counter();
    getType().iterateObject(addr, c);
    return c.getNumFields();
  }

  public SimpleTreeNode getChild(int index) {
    if (addr == null) {
      return null;
    }

    Fetcher f = new Fetcher(index);
    getType().iterateObject(addr, f);
    return f.getChild();
  }

  public boolean isLeaf() {
    return (addr == null);
  }

  public int getIndexOfChild(SimpleTreeNode child) {
    FieldIdentifier id = ((FieldTreeNodeAdapter) child).getID();
    Finder f = new Finder(id);
    getType().iterateObject(addr, f);
    return f.getIndex();
  }

  public String getValue() {
    if (addr != null) {
      return addr.toString();
    }
    return "NULL";
  }

  /** Should be used only once, then have the number of fields
      fetched. */
  static class Counter extends DefaultObjectVisitor {
    private int numFields;

    public int getNumFields() {
      return numFields;
    }

    public void doBit(FieldIdentifier f, long val)                   { ++numFields; }
    public void doInt(FieldIdentifier f, long val)                   { ++numFields; }
    public void doEnum(FieldIdentifier f, long val, String enumName) { ++numFields; }
    public void doFloat(FieldIdentifier f, float val)                { ++numFields; }
    public void doDouble(FieldIdentifier f, double val)              { ++numFields; }
    public void doPointer(FieldIdentifier f, Address val)            { ++numFields; }
    public void doArray(FieldIdentifier f, Address val)              { ++numFields; }
    public void doRef(FieldIdentifier f, Address val)                { ++numFields; }
    public void doCompound(FieldIdentifier f, Address addr)          { ++numFields; }
  }

  /** Creates a new SimpleTreeNode for the given field. */
  class Fetcher extends DefaultObjectVisitor {
    private int index;
    private int curField;
    private SimpleTreeNode child;

    public Fetcher(int index) {
      this.index = index;
    }

    public SimpleTreeNode getChild() {
      return child;
    }

    public void doBit(FieldIdentifier f, long val) {
      if (curField == index) {
        child = new LongTreeNodeAdapter(val, f, getTreeTableMode());
      }
      ++curField;
    }

    public void doInt(FieldIdentifier f, long val) {
      if (curField == index) {
        child = new LongTreeNodeAdapter(val, f, getTreeTableMode());
      }
      ++curField;
    }

    public void doEnum(FieldIdentifier f, long val, String enumName) {
      if (curField == index) {
        child = new EnumTreeNodeAdapter(enumName, val, f, getTreeTableMode());
      }
      ++curField;
    }

    public void doFloat(FieldIdentifier f, float val) {
      if (curField == index) {
        child = new FloatTreeNodeAdapter(val, f, getTreeTableMode());
      }
      ++curField;
    }

    public void doDouble(FieldIdentifier f, double val) {
      if (curField == index) {
        child = new DoubleTreeNodeAdapter(val, f, getTreeTableMode());
      }
      ++curField;
    }

    public void doPointer(FieldIdentifier f, Address val) {
      if (curField == index) {
        child = new AddressTreeNodeAdapter(val, f, getTreeTableMode());
      }
      ++curField;
    }

    public void doArray(FieldIdentifier f, Address val) {
      if (curField == index) {
        child = new AddressTreeNodeAdapter(val, f, getTreeTableMode());
      }
      ++curField;
    }

    public void doRef(FieldIdentifier f, Address val) {
      if (curField == index) {
        child = new AddressTreeNodeAdapter(val, f, getTreeTableMode());
      }
      ++curField;
    }

    public void doCompound(FieldIdentifier f, Address val) {
      if (curField == index) {
        child = new ObjectTreeNodeAdapter(val, f, getTreeTableMode());
      }
      ++curField;
    }
  }

  /** Finds the index of the given FieldIdentifier. */
  static class Finder extends DefaultObjectVisitor {
    private FieldIdentifier id;
    private int curField;
    private int index = -1;

    public Finder(FieldIdentifier id) {
      this.id = id;
    }

    /** Returns -1 if not found */
    public int getIndex() {
      return index;
    }

    public void doBit(FieldIdentifier f, long val)        { if (f.equals(id)) { index = curField; } ++curField; }
    public void doInt(FieldIdentifier f, long val)        { if (f.equals(id)) { index = curField; } ++curField; }
    public void doEnum(FieldIdentifier f, long val,
                       String enumName)                   { if (f.equals(id)) { index = curField; } ++curField; }
    public void doFloat(FieldIdentifier f, float val)     { if (f.equals(id)) { index = curField; } ++curField; }
    public void doDouble(FieldIdentifier f, double val)   { if (f.equals(id)) { index = curField; } ++curField; }
    public void doPointer(FieldIdentifier f, Address val) { if (f.equals(id)) { index = curField; } ++curField; }
    public void doArray(FieldIdentifier f, Address val)   { if (f.equals(id)) { index = curField; } ++curField; }
    public void doRef(FieldIdentifier f, Address val)     { if (f.equals(id)) { index = curField; } ++curField; }
    public void doCompound(FieldIdentifier f,
                           Address val)                   { if (f.equals(id)) { index = curField; } ++curField; }
  }
}
