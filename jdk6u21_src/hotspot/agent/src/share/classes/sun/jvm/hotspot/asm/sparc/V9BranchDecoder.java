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

abstract class V9BranchDecoder extends BranchDecoder
                       implements /* imports */ V9InstructionDecoder {
    static boolean getPredictTaken(int instruction) {
        return (PREDICTION_MASK & instruction) != 0;
    }
}
