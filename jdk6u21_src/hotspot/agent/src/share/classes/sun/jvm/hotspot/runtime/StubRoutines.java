/*
 * Copyright (c) 2000, 2009, Oracle and/or its affiliates. All rights reserved.
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

package sun.jvm.hotspot.runtime;

import java.util.*;
import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.types.*;

/** Very minimal port for now to get frames working */

public class StubRoutines {
  private static AddressField      callStubReturnAddressField;
  private static AddressField      callStubCompiledReturnAddressField;

  static {
    VM.registerVMInitializedObserver(new Observer() {
        public void update(Observable o, Object data) {
          initialize(VM.getVM().getTypeDataBase());
        }
      });
  }

  private static synchronized void initialize(TypeDataBase db) {
    Type type = db.lookupType("StubRoutines");

    callStubReturnAddressField = type.getAddressField("_call_stub_return_address");
    // Only some platforms have specific return from compiled to call_stub
    try {
      type = db.lookupType("StubRoutines::x86");
      if (type != null) {
        callStubCompiledReturnAddressField = type.getAddressField("_call_stub_compiled_return");
      }
    } catch (RuntimeException re) {
      callStubCompiledReturnAddressField = null;
    }
    if (callStubCompiledReturnAddressField == null && VM.getVM().getCPU().equals("x86")) {
      throw new InternalError("Missing definition for _call_stub_compiled_return");
    }
  }

  public StubRoutines() {
  }

  public boolean returnsToCallStub(Address returnPC) {
    Address addr = callStubReturnAddressField.getValue();
    boolean result = false;
    if (addr == null) {
      result = (addr == returnPC);
    } else {
      result = addr.equals(returnPC);
    }
    if (result || callStubCompiledReturnAddressField == null ) return result;
    // Could be a return to compiled code return point
    addr = callStubCompiledReturnAddressField.getValue();
    if (addr == null) {
      return (addr == returnPC);
    } else {
      return (addr.equals(returnPC));
    }

  }
}
