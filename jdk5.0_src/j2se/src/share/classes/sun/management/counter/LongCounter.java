/*
 * @(#)LongCounter.java	1.3 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.management.counter;

/**
 * Interface for a performance counter wrapping a
 * <code>long</code> basic type.
 *
 * @author   Brian Doherty
 * @version  1.3, 12/19/03
 */
public interface LongCounter extends Counter {

    /**
     * Get the value of this Long performance counter
     */
    public long longValue();
}
