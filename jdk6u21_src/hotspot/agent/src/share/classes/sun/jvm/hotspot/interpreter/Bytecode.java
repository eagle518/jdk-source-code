/*
 * Copyright (c) 2001, 2004, Oracle and/or its affiliates. All rights reserved.
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

package sun.jvm.hotspot.interpreter;

import sun.jvm.hotspot.oops.*;
import sun.jvm.hotspot.utilities.*;

public class Bytecode {
  Method method;
  int bci;
  static final int jintSize = 4;
  static final String spaces = " ";
  static final String comma  = ", ";

  Bytecode(Method method, int bci) {
    this.method = method;
    this.bci    = bci;
  }

  // Address computation
  // NOTE: assumes that the start of the method's bytecodes is 4-byte aligned
  int alignedOffset(int offset) {
    return Bits.roundTo(bci + offset, jintSize) - bci;
  }

  int javaSignedWordAt(int offset) {
    return method.getBytecodeIntArg(bci + offset);
  }

  short javaShortAt(int offset) {
    return method.getBytecodeShortArg(bci + offset);
  }

  byte javaByteAt(int offset) {
    return method.getBytecodeByteArg(bci + offset);
  }

  public Method method() { return method; }
  public int    bci()    { return bci;    }

  // hotspot byte code
  public int code() {
    return Bytecodes.codeAt(method(), bci());
  }

  // jvm byte code
  public int javaCode() {
    return Bytecodes.javaCode(code());
  }

  public String getBytecodeName() {
    return Bytecodes.name(code());
  }

  public String getJavaBytecodeName() {
    return Bytecodes.name(javaCode());
  }

  public int getLength() {
    return Bytecodes.lengthAt(method(), bci());
  }

  public int getJavaLength() {
    return Bytecodes.javaLengthAt(method(), bci());
  }

  public String toString() {
    StringBuffer buf = new StringBuffer(getJavaBytecodeName());
    if (code() != javaCode()) {
       buf.append(spaces);
       buf.append('[');
       buf.append(getBytecodeName());
       buf.append(']');
    }
    return buf.toString();
  }
}
