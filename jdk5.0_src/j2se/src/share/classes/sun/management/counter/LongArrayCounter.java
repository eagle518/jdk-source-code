/*
 * @(#)LongArrayCounter.java	1.3 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.management.counter;

/**
 * Interface for performance counter wrapping <code>long[]</code> objects.
 *
 */
public interface LongArrayCounter extends Counter {

    /**
     * Get a copy of the elements of the LongArrayCounter.
     */
    public long[] longArrayValue();
  
    /**
     * Get the value of an element of the LongArrayCounter object.
     */
    public long longAt(int index);
}
