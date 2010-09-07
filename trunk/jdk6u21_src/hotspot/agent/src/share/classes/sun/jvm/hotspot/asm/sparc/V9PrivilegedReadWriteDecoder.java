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

abstract class V9PrivilegedReadWriteDecoder extends InstructionDecoder
                  implements V9InstructionDecoder {
    static boolean isLegalPrivilegedRegister(int reg) {
        return (reg > -1 && reg < 16) || reg == 31;
    }
}
