/*
 * @(#)DirectBuffer.java	1.10 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.nio.ch;

import sun.misc.Cleaner;


public interface DirectBuffer {

    public long address();

    public Object viewedBuffer();

    public Cleaner cleaner();

}
