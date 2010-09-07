/*
 * Copyright (c) 2000, 2004, Oracle and/or its affiliates. All rights reserved.
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

package sun.jvm.hotspot.compiler;

import java.util.*;

import sun.jvm.hotspot.code.*;
import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.runtime.*;
import sun.jvm.hotspot.types.*;

public class OopMap extends VMObject {
  private static CIntegerField pcOffsetField;
  private static CIntegerField omvCountField;
  private static CIntegerField omvDataSizeField;
  private static AddressField  omvDataField;
  private static AddressField  compressedWriteStreamField;

  // This is actually a field inside class CompressedStream
  private static AddressField  compressedStreamBufferField;

  static {
    VM.registerVMInitializedObserver(new Observer() {
        public void update(Observable o, Object data) {
          initialize(VM.getVM().getTypeDataBase());
        }
      });
  }

  private static void initialize(TypeDataBase db) {
    Type type = db.lookupType("OopMap");

    pcOffsetField              = type.getCIntegerField("_pc_offset");
    omvCountField              = type.getCIntegerField("_omv_count");
    omvDataSizeField           = type.getCIntegerField("_omv_data_size");
    omvDataField               = type.getAddressField("_omv_data");
    compressedWriteStreamField = type.getAddressField("_write_stream");

    type = db.lookupType("CompressedStream");
    compressedStreamBufferField = type.getAddressField("_buffer");
  }

  public OopMap(Address addr) {
    super(addr);
  }

  public long getOffset() {
    return pcOffsetField.getValue(addr);
  }

  //--------------------------------------------------------------------------------
  // Internals only below this point
  //

  // Accessors -- package private for now
  Address getOMVData() {
    return omvDataField.getValue(addr);
  }

  long getOMVDataSize() {
    return omvDataSizeField.getValue(addr);
  }

  long getOMVCount() {
    return omvCountField.getValue(addr);
  }

  CompressedWriteStream getWriteStream() {
    Address wsAddr = compressedWriteStreamField.getValue(addr);
    if (wsAddr == null) {
      return null;
    }
    Address bufferAddr = compressedStreamBufferField.getValue(wsAddr);
    if (bufferAddr == null) {
      return null;
    }
    return new CompressedWriteStream(bufferAddr);
  }
}
