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

package sun.jvm.hotspot.asm.sparc;

import sun.jvm.hotspot.asm.*;

public class SPARCV9IlltrapInstruction extends SPARCUnimpInstruction
                    implements SPARCV9Instruction {
    public SPARCV9IlltrapInstruction(int const22) {
        super("illtrap", const22);
    }
}
