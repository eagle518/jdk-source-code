/*
 * @(#)ByteArrayCounter.java	1.3 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.management.counter;

/**
 * Interface for performance counter wrapping <code>byte[]</code> objects.
 *
 * @author   Brian Doherty
 * @version  1.3, 12/19/03
 */
public interface ByteArrayCounter extends Counter {

    /**
     * Get a copy of the elements of the ByteArrayCounter.
     */
    public byte[] byteArrayValue();
  
    /**
     * Get the value of an element of the ByteArrayCounter object.
     */
    public byte byteAt(int index);
}
