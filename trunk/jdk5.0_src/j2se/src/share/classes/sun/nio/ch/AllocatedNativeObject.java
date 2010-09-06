/*
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)AllocatedNativeObject.java	1.9 03/12/19
 */

package sun.nio.ch;					// Formerly in sun.misc


// ## In the fullness of time, this class will be eliminated

class AllocatedNativeObject				// package-private
    extends NativeObject
{

    /**
     * Allocates a memory area of at least <tt>size</tt> bytes outside of the
     * Java heap and creates a native object for that area.
     *
     * @param  size
     *         Number of bytes to allocate
     *
     * @param  pageAligned
     *         If <tt>true</tt> then the area will be aligned on a hardware
     *         page boundary
     *
     * @throws OutOfMemoryError
     *         If the request cannot be satisfied
     */
    AllocatedNativeObject(int size, boolean pageAligned) {
	super(size, pageAligned);
    }

    /**
     * Frees the native memory area associated with this object.
     */
    synchronized void free() {
	if (allocationAddress != 0) {
	    unsafe.freeMemory(allocationAddress);
	    allocationAddress = 0;
	}
    }

}
