/*
 * Copyright (c) 2000, Oracle and/or its affiliates. All rights reserved.
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

import java.util.*;
import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.runtime.*;
import sun.jvm.hotspot.types.*;

/** <P> Class EdenSpace describes eden-space in new
    generation. (Currently it does not add any significant
    functionality beyond ContiguousSpace.) */

public class EdenSpace extends ContiguousSpace {
  public EdenSpace(Address addr) {
    super(addr);
  }
}
