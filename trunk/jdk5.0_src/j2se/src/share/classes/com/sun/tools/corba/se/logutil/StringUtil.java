/*
 * @(#)StringUtil.java	1.3 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.corba.se.logutil;

public abstract class StringUtil {
    /** Take a string containing underscores, and return a string
    * with the underscore removed, and all characters exception in lower
    * case except the characters after the underscores.
    */
    public static String toMixedCase( String str )
    {
	StringBuffer sbuf = new StringBuffer( str.length() ) ;
	boolean uppercaseNext = false ;
	for (int ctr=0; ctr<str.length(); ctr++) {
	    char ch = str.charAt( ctr ) ;

	    if (ch == '_') {
		uppercaseNext = true ;
	    } else if (uppercaseNext) {
		sbuf.append( Character.toUpperCase( ch ) ) ;
		uppercaseNext = false ;
	    } else {
		sbuf.append( Character.toLowerCase( ch ) ) ;
	    }
	}

	return sbuf.toString() ;
    }

    public static int countArgs( String str )
    {
	int result = 0 ;
	for( int ctr = 0; ctr<str.length(); ctr++ ) 
	    if (str.charAt(ctr) == '{')
		result++ ;

	return result ;
    }
}
