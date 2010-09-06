/*
 * @(#)X509AttributeName.java	1.11 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.security.x509;

/**
 * This class is used to parse attribute names like "x509.info.extensions".
 *
 * @author Amit Kapoor
 * @author Hemma Prafullchandra
 * @version 1.11
 */
public class X509AttributeName {
    // Public members
    private static final char SEPARATOR = '.';

    // Private data members
    private String prefix = null;
    private String suffix = null;

    /**
     * Default constructor for the class. Name is of the form
     * "x509.info.extensions".
     *
     * @param name the attribute name.
     */
    public X509AttributeName(String name) {
        int i = name.indexOf(SEPARATOR);
        if (i == (-1)) {
            prefix = name;
        } else {
            prefix = name.substring(0, i);
            suffix = name.substring(i + 1);
        }
    }

    /**
     * Return the prefix of the name.
     */
    public String getPrefix() {
      return (prefix);
    }

    /**
     * Return the suffix of the name.
     */
    public String getSuffix() {
      return (suffix);
    }
}
