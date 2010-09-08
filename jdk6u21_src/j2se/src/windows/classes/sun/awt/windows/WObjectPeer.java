/*
 * @(#)WObjectPeer.java	1.23 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.awt.windows;

import java.awt.Component;

abstract class WObjectPeer {

    static {
        initIDs();
    }

    // The Windows handle for the native widget.
    long pData;
    // if the native peer has been destroyed
    boolean destroyed = false;
    // The associated AWT object.
    Object target;

    private volatile boolean disposed;

    // set from JNI if any errors in creating the peer occur
    protected Error createError = null;

    // used to synchronize the state of this peer
    private final Object stateLock = new Object();

    public static WObjectPeer getPeerForTarget(Object t) {
	WObjectPeer peer = (WObjectPeer) WToolkit.targetToPeer(t);
	return peer;
    }

    public long getData() {
        return pData;
    }

    public Object getTarget() {
        return target;
    }

    public final Object getStateLock() {
        return stateLock;
    }

    /*
     * Subclasses should override disposeImpl() instead of dispose(). Client
     * code should always invoke dispose(), never disposeImpl().
     */
    abstract protected void disposeImpl();
    public final void dispose() {
        boolean call_disposeImpl = false;

        synchronized (this) {
            if (!disposed) {
                disposed = call_disposeImpl = true;
            }
        }    

	if (call_disposeImpl) {
	    disposeImpl();
	}
    }
    protected final boolean isDisposed() {
        return disposed;
    }

    /**
     * Initialize JNI field and method IDs
     */
    private static native void initIDs();
}
