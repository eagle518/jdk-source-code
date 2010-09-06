/*
 * @(#)StringFlag.java	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.management;

public class StringFlag extends Flag {
    StringFlag(String name, String value, boolean writeable, 
                      Flag.FlagSource source) {
        super(name, value, writeable, source);
    }


    /**
     * Returns the String value of this flag.
     *
     * @return the String value of this flag.
     */
    public String stringValue() {
        return (String) getValue();
    }

    /**
     * Returns the string representation of this flag.
     *
     * @return the string representation of this flag.
     */
    public String toString() {
        return "String " + getName() + " = " + stringValue() +
               " [" + getSource() + "]";
    }
}
