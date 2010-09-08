/*
 * @(#)UnsafeXDisposerRecord.java	1.4 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.awt.X11;

import sun.misc.Unsafe;
import java.util.logging.*;

class UnsafeXDisposerRecord implements sun.java2d.DisposerRecord {
    private static final Logger log = Logger.getLogger("sun.awt.X11.UnsafeXDisposerRecord");
    private static Unsafe unsafe = XlibWrapper.unsafe;
    final long[] unsafe_ptrs, x_ptrs;
    final String name;
    volatile boolean disposed;
    final Throwable place;
    public UnsafeXDisposerRecord(String name, long[] unsafe_ptrs, long[] x_ptrs) {
        this.unsafe_ptrs = unsafe_ptrs;
        this.x_ptrs = x_ptrs;
        this.name = name;
        if (XlibWrapper.isBuildInternal) {
            place = new Throwable();
        } else {
            place = null;
        }
    }
    public UnsafeXDisposerRecord(String name, long ... unsafe_ptrs) {
        this.unsafe_ptrs = unsafe_ptrs;
        this.x_ptrs = null;
        this.name = name;
        if (XlibWrapper.isBuildInternal) {
            place = new Throwable();
        } else {
            place = null;
        }
    }

    public void dispose() {
        XToolkit.awtLock();
        try {
            if (!disposed) {
                if (XlibWrapper.isBuildInternal && "Java2D Disposer".equals(Thread.currentThread().getName()) && log.isLoggable(Level.WARNING)) {
                    if (place != null) {
                        log.log(Level.WARNING, name + " object was not disposed before finalization!", place);
                    } else {
                        log.log(Level.WARNING, name + " object was not disposed before finalization!");
                    }
                }
                
                if (unsafe_ptrs != null) {
                    for (long l : unsafe_ptrs) {
                        if (l != 0) {
                            unsafe.freeMemory(l);
                        }
                    }
                }
                if (x_ptrs != null) {
                    for (long l : x_ptrs) {                    
                        if (l != 0) {
                            if (Native.getLong(l) != 0) {
                                XlibWrapper.XFree(Native.getLong(l));
                            }
                            unsafe.freeMemory(l);
                        }
                    }
                }
                disposed = true;
            }
        } finally {
            XToolkit.awtUnlock();
        }
    }
}
