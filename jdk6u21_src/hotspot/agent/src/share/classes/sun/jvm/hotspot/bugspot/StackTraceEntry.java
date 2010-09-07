/*
 * Copyright (c) 2001, 2002, Oracle and/or its affiliates. All rights reserved.
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

package sun.jvm.hotspot.bugspot;

import sun.jvm.hotspot.debugger.cdbg.*;
import sun.jvm.hotspot.oops.*;
import sun.jvm.hotspot.runtime.*;

/** This class describes a frame in a stack trace. It abstracts over
    C/C++ and Java frames. */

public class StackTraceEntry {
  private CFrame cFrame;
  private CDebugger dbg;
  private JavaVFrame javaFrame;
  private String value; // What is displayed in a stack trace
  // For merging C and Java stack traces.
  // For more precise stack traces, should probably have a way to
  // convert a CFrame to a sun.jvm.hotspot.runtime.Frame. For now,
  // doing similar algorithm to jdbx (which does not have intimate
  // knowledge of the VM).
  private boolean isUnknownCFrame;

  public StackTraceEntry(CFrame cFrame, CDebugger dbg) {
    this.cFrame = cFrame;
    this.dbg = dbg;
    computeValue();
  }

  public StackTraceEntry(JavaVFrame javaFrame) {
    this.javaFrame = javaFrame;
    computeValue();
  }

  public boolean    isCFrame()     { return (cFrame != null);    }
  public boolean    isJavaFrame()  { return (javaFrame != null); }
  public CFrame     getCFrame()    { return cFrame;              }
  public JavaVFrame getJavaFrame() { return javaFrame;           }
  public boolean    isUnknownCFrame() { return isUnknownCFrame;  }
  public String toString() {
    return value;
  }

  private void computeValue() {
    isUnknownCFrame = true;
    value = "<unknown>";
    if (cFrame != null) {
      PCFinder.Info info = PCFinder.findPC(cFrame.pc(), cFrame.loadObjectForPC(), dbg);
      if (info.getName() != null) {
        value = "(C) " + info.getName();
        isUnknownCFrame = false;
        if (info.getConfidence() == PCFinder.LOW_CONFIDENCE) {
          value = value + " (?)";
        }
        if (info.getOffset() >= 0) {
          value = value + " + 0x" + Long.toHexString(info.getOffset());
        }
      }
    } else if (javaFrame != null) {
      isUnknownCFrame = false;
      Method m = javaFrame.getMethod();
      value = "(J) " + m.externalNameAndSignature();
    }
  }
}
