/*
 * Copyright (c) 2000, 2008, Oracle and/or its affiliates. All rights reserved.
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

package sun.jvm.hotspot.oops;

import java.io.*;
import java.util.*;
import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.runtime.*;
import sun.jvm.hotspot.types.*;
import sun.jvm.hotspot.utilities.*;

//  ConstantPoolCache : A constant pool cache (constantPoolCacheOopDesc).
//  See cpCacheOop.hpp for details about this class.
//
public class ConstantPoolCache extends Oop {
  static {
    VM.registerVMInitializedObserver(new Observer() {
        public void update(Observable o, Object data) {
          initialize(VM.getVM().getTypeDataBase());
        }
      });
  }

  private static synchronized void initialize(TypeDataBase db) throws WrongTypeException {
    Type type      = db.lookupType("constantPoolCacheOopDesc");
    constants      = new OopField(type.getOopField("_constant_pool"), 0);
    baseOffset     = type.getSize();
    Type elType    = db.lookupType("ConstantPoolCacheEntry");
    elementSize    = elType.getSize();
    length         = new CIntField(type.getCIntegerField("_length"), 0);
  }

  ConstantPoolCache(OopHandle handle, ObjectHeap heap) {
    super(handle, heap);
  }

  public boolean isConstantPoolCache() { return true; }

  private static OopField constants;

  private static long baseOffset;
  private static long elementSize;
  private static CIntField length;


  public ConstantPool getConstants() { return (ConstantPool) constants.getValue(this); }

  public long getObjectSize() {
    return alignObjectSize(baseOffset + getLength() * elementSize);
  }

  public ConstantPoolCacheEntry getEntryAt(int i) {
    if (Assert.ASSERTS_ENABLED) {
      Assert.that(0 <= i && i < getLength(), "index out of bounds");
    }
    return new ConstantPoolCacheEntry(this, i);
  }

  public int getIntAt(int entry, int fld) {
    //alignObjectSize ?
    long offset = baseOffset + /*alignObjectSize*/entry * elementSize + fld* getHeap().getIntSize();
    return (int) getHandle().getCIntegerAt(offset, getHeap().getIntSize(), true );
  }


  public void printValueOn(PrintStream tty) {
    tty.print("ConstantPoolCache for " + getConstants().getPoolHolder().getName().asString());
  }

  public int getLength() {
    return (int) length.getValue(this);
  }

  public void iterateFields(OopVisitor visitor, boolean doVMFields) {
    super.iterateFields(visitor, doVMFields);
    if (doVMFields) {
      visitor.doOop(constants, true);
      for (int i = 0; i < getLength(); i++) {
        ConstantPoolCacheEntry entry = getEntryAt(i);
        entry.iterateFields(visitor);
      }
    }
  }
};
