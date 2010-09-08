/*
 * @(#)WBufferStrategy.java	1.6 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.awt.windows;

import java.awt.Image;
import java.awt.Component;

/**
 * This sun-private class exists solely to get a handle to 
 * the back buffer associated with a Component.  If that 
 * Component has a BufferStrategy with >1 buffer, then the
 * Image subclass associated with that buffer will be returned.
 */
public class WBufferStrategy {

    private static native void initIDs(Class componentClass);

    static {
	initIDs(Component.class);
    }
    
    public static native Image getDrawBuffer(Component comp);

}
