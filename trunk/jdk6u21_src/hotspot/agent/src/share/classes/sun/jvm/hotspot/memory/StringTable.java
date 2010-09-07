/*
 * Copyright (c) 2005, Oracle and/or its affiliates. All rights reserved.
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

package sun.jvm.hotspot.memory;

import java.io.*;
import java.util.*;
import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.oops.*;
import sun.jvm.hotspot.types.*;
import sun.jvm.hotspot.runtime.*;
import sun.jvm.hotspot.utilities.*;

public class StringTable extends sun.jvm.hotspot.utilities.Hashtable {
  static {
    VM.registerVMInitializedObserver(new Observer() {
        public void update(Observable o, Object data) {
          initialize(VM.getVM().getTypeDataBase());
        }
      });
  }

  private static synchronized void initialize(TypeDataBase db) {
    Type type = db.lookupType("StringTable");
    theTableField  = type.getAddressField("_the_table");
    stringTableSize = db.lookupIntConstant("StringTable::string_table_size").intValue();
  }

  // Fields
  private static AddressField theTableField;
  private static int stringTableSize;

  // Accessors
  public static StringTable getTheTable() {
    Address tmp = theTableField.getValue();
    return (StringTable) VMObjectFactory.newObject(StringTable.class, tmp);
  }

  public static int getStringTableSize() {
    return stringTableSize;
  }

  public StringTable(Address addr) {
    super(addr);
  }

  public interface StringVisitor {
    public void visit(Instance string);
  }

  public void stringsDo(StringVisitor visitor) {
    int numBuckets = tableSize();
    for (int i = 0; i < numBuckets; i++) {
      for (HashtableEntry e = (HashtableEntry) bucket(i); e != null;
           e = (HashtableEntry) e.next()) {
        visitor.visit((Instance) e.literal());
      }
    }
  }
}
