/*
 * @(#)ByteArrayLexOrder.java	1.12 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.security.util;

import java.util.Comparator;

/**
 * Compare two byte arrays in lexicographical order.
 *
 * @version 1.12 12/19/03
 * @author D. N. Hoover
 */
public class ByteArrayLexOrder implements Comparator {

    /**
     * Perform lexicographical comparison of two byte arrays,
     * regarding each byte as unsigned.  That is, compare array entries 
     * in order until they differ--the array with the smaller entry 
     * is "smaller". If array entries are 
     * equal till one array ends, then the longer array is "bigger".
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

	int diff;
	for (int i = 0; i < bytes1.length && i < bytes2.length; i++) {
	    diff = (bytes1[i] & 0xFF) - (bytes2[i] & 0xFF);
	    if (diff != 0) {
		return diff;
	    }
	}
	// if array entries are equal till the first ends, then the
	// longer is "bigger"
	return bytes1.length - bytes2.length;
    }
	

}
