/*
 * @(#)LongFlag.java	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.management;

public class LongFlag extends Flag {
    LongFlag(String name, Long value, boolean writeable, 
                    Flag.FlagSource source) {
        super(name, value, writeable, source);
    }

    /**
     * Returns the long value of this flag.
     *
     * @return the long value of this flag.
     */
    public long longValue() {
         Long value = (Long) getValue();
         return value.longValue();
    }

    /**
     * Returns the string representation of this flag.
     *
     * @return the string representation of this flag.
     */
    public String toString() {
        return "long " + getName() + " = " + longValue() +
               " [" + getSource() + "]";
    }
}
