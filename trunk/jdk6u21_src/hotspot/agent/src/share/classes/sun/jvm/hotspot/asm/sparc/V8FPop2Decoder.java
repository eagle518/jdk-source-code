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
import java.util.*;

class V8FPop2Decoder extends FPopDecoder {
    static Map fpop2Decoders = new HashMap(); // Map<Integer, InstructionDecoder>
    static void addFPop2Decoder(int fpOpcode, InstructionDecoder decoder) {
        fpop2Decoders.put(new Integer(fpOpcode), decoder);
    }

    // opf (op=2, op3=0x35=FPop2 - Table F-6 page 231
    static {
        addFPop2Decoder(FCMPs, new FP2RegisterDecoder(FCMPs, "fcmps", RTLDT_FL_SINGLE, RTLDT_FL_SINGLE));
        addFPop2Decoder(FCMPd, new FP2RegisterDecoder(FCMPd, "fcmpd", RTLDT_FL_DOUBLE, RTLDT_FL_DOUBLE));
        addFPop2Decoder(FCMPq, new FP2RegisterDecoder(FCMPq, "fcmpq", RTLDT_FL_QUAD, RTLDT_FL_QUAD));
        addFPop2Decoder(FCMPEs, new FP2RegisterDecoder(FCMPEs, "fcmpes", RTLDT_FL_SINGLE, RTLDT_FL_SINGLE));
        addFPop2Decoder(FCMPEd, new FP2RegisterDecoder(FCMPEd, "fcmped", RTLDT_FL_DOUBLE, RTLDT_FL_DOUBLE));
        addFPop2Decoder(FCMPEq, new FP2RegisterDecoder(FCMPEq, "fcmpeq", RTLDT_FL_QUAD, RTLDT_FL_QUAD));
    }

    InstructionDecoder getOpfDecoder(int opf) {
        return (InstructionDecoder) fpop2Decoders.get(new Integer(opf));
    }
}
