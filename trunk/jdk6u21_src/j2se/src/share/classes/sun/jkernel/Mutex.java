/*
 * @(#)Mutex.java	1.8 10/03/23
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.jkernel;

/**
 * A mutex which works even between different processes.  Currently implemented
 * only on Win32.
 *
 *@author Ethan Nicholas
 */
public class Mutex {
    static {
        try {
            System.loadLibrary("jkernel");
        }
        catch (Exception e) { 
            throw new Error(e); 
        }
    }
    
    private String uniqueId;
    private long handle;
    
    public static Mutex create(String uniqueId) {
        return new Mutex(uniqueId);
    }
    
        
    private Mutex(String uniqueId) {
        this.uniqueId = uniqueId;
        this.handle = createNativeMutex(uniqueId);
    }
    
    
    private static native long createNativeMutex(String uniqueId);
    
    
    public native void acquire();
    
    
    public native boolean acquire(int timeout);
    
    
    public native void release();
    
    
    public native void destroyNativeMutex();
    
    
    public void dispose() {
        destroyNativeMutex();
        handle = 0;
    }
    
    
    public void finalize() {
        dispose();
    }
    
    
    public String toString() {
        return "Mutex[" + uniqueId + "]";
    }
}
