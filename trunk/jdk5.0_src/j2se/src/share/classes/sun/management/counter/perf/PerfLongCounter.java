/*
 * @(#)PerfLongCounter.java	1.4 04/02/13
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.management.counter.perf;

import sun.management.counter.*;
import java.nio.LongBuffer;
import java.nio.ReadOnlyBufferException;

public class PerfLongCounter extends AbstractCounter
       implements LongCounter {

    LongBuffer lb;

    // package private
    PerfLongCounter(String name, Units u, Variability v, int flags,
                    LongBuffer lb) {
        super(name, u, v, flags);
        this.lb = lb;
    }
    
    public Object getValue() {
        return new Long(lb.get(0));
    }
    
    /**
     * Get the value of this Long performance counter 
     */
    public long longValue() {
        return lb.get(0);
    }

    /**
     * Serialize as a snapshot object.
     */
    protected Object writeReplace() throws java.io.ObjectStreamException {
        return new LongCounterSnapshot(getName(),
                                       getUnits(),
                                       getVariability(),
                                       getFlags(),
                                       longValue());
    }
}
