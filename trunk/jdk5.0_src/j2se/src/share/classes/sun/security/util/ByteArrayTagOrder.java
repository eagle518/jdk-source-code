/*
 * @(#)ByteArrayTagOrder.java	1.12 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


/**
 * ByteArrayTagOrder: a class for comparing two DER encodings by the 
 * order of their tags.
 *
 * @version 1.12 12/19/03
 * @author D. N. Hoover
 */

package sun.security.util;

import java.util.Comparator;

public class ByteArrayTagOrder implements Comparator {

    /**
     * Compare two byte arrays, by the order of their tags,
     * as defined in ITU-T X.680, sec. 6.4.  (First compare
     *  tag classes, then tag numbers, ignoring the constructivity bit.)
     *
     * @param  obj1 first byte array to compare.
     * @param  obj2 second byte array to compare.
     * @return negative number if obj1 < obj2, 0 if obj1 == obj2, 
     * positive number if obj1 > obj2.  
     *
     * @exception <code>ClassCastException</code> 
     * if either argument is not a byte array.
     */

    public final int compare(Object obj1, Object obj2) {

	byte[] bytes1 = (byte[]) obj1;
	byte[] bytes2 = (byte[]) obj2;

	// tag order is same as byte order ignoring any difference in 
	// the constructivity bit (0x02)
	return (bytes1[0] | 0x20) - (bytes2[0] | 0x20);
    }
	

}
