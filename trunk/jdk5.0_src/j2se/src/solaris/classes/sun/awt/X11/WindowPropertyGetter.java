/*
 * @(#)WindowPropertyGetter.java	1.9 04/01/21
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.X11;

import sun.misc.Unsafe;

class WindowPropertyGetter {
    private static Unsafe unsafe = XlibWrapper.unsafe;
    private final long actual_type = unsafe.allocateMemory(8);
    private final long actual_format = unsafe.allocateMemory(4);
    private final long nitems_ptr = unsafe.allocateMemory(8);
    private final long bytes_after = unsafe.allocateMemory(8);
    private final long data = unsafe.allocateMemory(8);
    private final long window;
    private final XAtom property;
    private final long offset;
    private final long length;
    private final boolean auto_delete;
    private final long type;
    private boolean executed = false;
    private boolean disposed = false;
    public WindowPropertyGetter(long window, XAtom property, long offset, 
                                long length, boolean auto_delete, long type) 
    {
        if (property.getAtom() == 0) {
            throw new IllegalArgumentException("Property ATOM should be initialized first:" + property);
        }
        // Zero is AnyPropertyType.
        // if (type == 0) {
        //     throw new IllegalArgumentException("Type ATOM shouldn't be zero");
        // }
        if (window == 0) {
            throw new IllegalArgumentException("Window must not be zero");
        }
        this.window = window;
        this.property = property;
        this.offset = offset;
        this.length = length;
        this.auto_delete = auto_delete;
        this.type = type;
    }
    public WindowPropertyGetter(long window, XAtom property, long offset, 
                                long length, boolean auto_delete, XAtom type) 
    {
        this(window, property, offset, length, auto_delete, type.getAtom());        
    }
    public int execute() {
        return execute(null);
    }
    public int execute(XToolkit.XErrorHandler errorHandler) {
        if (disposed) {
            throw new IllegalStateException("Disposed");
        }
        try {
            XToolkit.awtLock();
            if (executed) {
                throw new IllegalStateException("Already executed");
            }
            executed = true;

            if (errorHandler != null) {
                XToolkit.WITH_XERROR_HANDLER(errorHandler);
            }
            Native.putLong(data, 0);
            int status = XlibWrapper.XGetWindowProperty(XToolkit.getDisplay(), window, property.getAtom(),
                                                        offset, length, (auto_delete?1:0), type,
                                                        actual_type, actual_format, nitems_ptr,
                                                        bytes_after, data);
            if (errorHandler != null) {
                XToolkit.RESTORE_XERROR_HANDLER();
            }
            return status;
        } finally {
            XToolkit.awtUnlock();
        }
    }

    public boolean isExecuted() {
        return executed;
    }

    public boolean isDisposed() {
        return disposed;
    }

    public int getActualFormat() {
        if (disposed) {
            throw new IllegalStateException("Disposed");
        }
        if (!executed) {
            throw new IllegalStateException("Not executed");
        }
        return unsafe.getInt(actual_format);
    }
    public long getActualType() {
        if (disposed) {
            throw new IllegalStateException("Disposed");
        }
        if (!executed) {
            throw new IllegalStateException("Not executed");
        }
        return XAtom.getAtom(actual_type);
    }
    public int getNumberOfItems() {
        if (disposed) {
            throw new IllegalStateException("Disposed");
        }
        if (!executed) {
            throw new IllegalStateException("Not executed");
        }
        return (int)Native.getLong(nitems_ptr);
    }
    public long getData() {
        if (disposed) {
            throw new IllegalStateException("Disposed");
        }
        return Native.getLong(data);
    }
    public long getBytesAfter() {
        if (disposed) {
            throw new IllegalStateException("Disposed");
        }
        if (!executed) {
            throw new IllegalStateException("Not executed");
        }
        return Native.getLong(bytes_after);
    }
    public void finalize() {
        if (XlibWrapper.isBuildInternal && !disposed) {
            System.err.println("WARNING: sun.awt.X11.WindowPropertyGetter not disposed till finalization!");
        }
    }
    // [das]REMIND: lock protection is needed!
    public void dispose() {
        synchronized(XToolkit.getAWTLock()) {
            if (disposed) {
                return;
            }
            unsafe.freeMemory(actual_type);
            unsafe.freeMemory(actual_format);
            unsafe.freeMemory(nitems_ptr);
            unsafe.freeMemory(bytes_after);
            if (executed && getData() != 0) {
                XlibWrapper.XFree(getData());
            }
            unsafe.freeMemory(data);
            disposed = true;
        }
    }
}
