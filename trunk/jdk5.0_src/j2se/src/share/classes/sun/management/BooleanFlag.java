/*
 * @(#)BooleanFlag.java	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.management;

public class BooleanFlag extends Flag {
    BooleanFlag(String name, Boolean value, boolean writeable, 
                       Flag.FlagSource source) {
        super(name, value, writeable, source);
    }

    /**
     * Returns the integer value of this flag.
     *
     * @return the integer value of this flag.
     */
    public boolean booleanValue() {
        Boolean value = (Boolean) getValue();
        return value.booleanValue();
    }

    /**
     * Returns the string representation of this flag.
     *
     * @return the string representation of this flag.
     */
    public String toString() {
        return "boolean " + getName() + " = " + booleanValue() +
               " [" + getSource() + "]";
    }

}
