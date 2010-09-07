/*
 * @(#)BufferUtil.java	1.3 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.util;

import java.nio.*;

public class BufferUtil {
    private BufferUtil() {}

    public static final int  KB = 1024;
    public static final int  MB = KB * KB;
    public static final int  GB = KB * MB;
    public static final long TB = KB * GB;

    public static ByteBuffer slice(ByteBuffer buf, int pos, int size) {
        int origPos = buf.position();
        int origLim = buf.limit();
        buf.clear();
        buf.position(pos);
        buf.limit(pos + size);
        ByteBuffer res = buf.slice();
        res.order(ByteOrder.nativeOrder());
        buf.clear();
        buf.position(origPos);
        buf.limit(origLim);
        return res;
    }
}
