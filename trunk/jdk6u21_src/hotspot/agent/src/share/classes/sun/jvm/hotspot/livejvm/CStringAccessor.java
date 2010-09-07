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

package sun.jvm.hotspot.livejvm;

import java.io.UnsupportedEncodingException;
import sun.jvm.hotspot.debugger.*;

class CStringAccessor {
  private Address addr;
  private int bufLen;

  CStringAccessor(Address addr, int bufLen) {
    this.addr = addr;
    this.bufLen = bufLen;
  }

  String getValue() throws DebuggerException {
    int len = 0;
    while ((addr.getCIntegerAt(len, 1, true) != 0) && (len < bufLen)) {
      ++len;
    }
    byte[] res = new byte[len];
    for (int i = 0; i < len; i++) {
      res[i] = (byte) addr.getCIntegerAt(i, 1, true);
    }
    try {
      return new String(res, "US-ASCII");
    } catch (UnsupportedEncodingException e) {
      throw new DebuggerException("Unable to use US-ASCII encoding");
    }
  }

  void setValue(String value) throws DebuggerException {
    try {
      byte[] data = value.getBytes("US-ASCII");
      if (data.length >= bufLen) {
        throw new DebuggerException("String too long");
      }
      for (int i = 0; i < data.length; i++) {
        addr.setCIntegerAt(i, 1, data[i]);
      }
      addr.setCIntegerAt(data.length, 1, 0);
    } catch (UnsupportedEncodingException e) {
      throw new DebuggerException("Unable to use US-ASCII encoding");
    }
  }
}
