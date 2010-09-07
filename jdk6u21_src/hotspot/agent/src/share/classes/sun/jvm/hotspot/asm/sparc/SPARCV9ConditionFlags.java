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

class SPARCV9ConditionFlags {
    private static final String ccFlagNames[] = {
        "%fcc0", "%fcc1", "%fcc2", "%fcc3", "%icc", null, "%xcc", null
    };

    public static String getFlagName(int index) {
        return ccFlagNames[index];
    }
}
