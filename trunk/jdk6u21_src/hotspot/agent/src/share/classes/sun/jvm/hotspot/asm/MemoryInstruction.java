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

package sun.jvm.hotspot.asm;

public interface MemoryInstruction extends RTLDataTypes {
   public int getDataType(); // one of the RTLDataTypes.
   public boolean isConditional(); // conditional store like swap or v9 like non-faulting loads
}
