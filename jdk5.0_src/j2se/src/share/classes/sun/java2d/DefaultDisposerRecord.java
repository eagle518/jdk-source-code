/*
 * @(#)DefaultDisposerRecord.java	1.3 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.java2d;

/**
 * This class is the default DisposerRecord implementation which
 * holds pointers to the native disposal method and the data to be disposed.
 */
public class DefaultDisposerRecord extends DisposerRecord {
    private long dataPointer;
    private long disposerMethodPointer;

    public DefaultDisposerRecord(long  disposerMethodPointer, long dataPointer) {
        this.disposerMethodPointer = disposerMethodPointer;
        this.dataPointer = dataPointer;
    }
    
    public void dispose() {
	invokeNativeDispose(disposerMethodPointer, 
			    dataPointer);
    }

    public long getDataPointer() {
	return dataPointer;
    }

    public long getDisposerMethodPointer() {
	return disposerMethodPointer;
    }

    public static native void invokeNativeDispose(long disposerMethodPointer, 
						  long dataPointer);
}
