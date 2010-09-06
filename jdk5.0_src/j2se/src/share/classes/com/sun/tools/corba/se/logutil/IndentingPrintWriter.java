/*
 * @(#)IndentingPrintWriter.java	1.3 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.corba.se.logutil;

import java.io.PrintWriter ;
import java.io.Writer ;
import java.io.OutputStream ;
import java.io.BufferedWriter ;
import java.io.OutputStreamWriter ;
import jsint.Pair ;
import java.util.StringTokenizer ;

public class IndentingPrintWriter extends PrintWriter {
    private int level = 0 ;
    private int indentWidth = 4 ;
    private String indentString = "" ;

    public void printMsg( String msg, Pair data )
    {
	// System.out.println( "printMsg called with msg=" + msg + " data=" + data ) ;
	StringTokenizer st = new StringTokenizer( msg, "@", true ) ;
	StringBuffer result = new StringBuffer() ;
	Object head = data.first ;
	Pair tail = (Pair)data.rest ;
	String token = null ;

	while (st.hasMoreTokens()) {
	    token = st.nextToken() ;
	    if (token.equals("@")) {
		if (head != null) {
		    result.append( head ) ;
		    head = tail.first ;
		    tail = (Pair)tail.rest ;
		} else {
		    throw new Error( "List too short for message" ) ;
		}
	    } else {
		result.append( token ) ;
	    }
	}

	// System.out.println( "Printing result " + result + " to file" ) ;
	print( result ) ;
	println() ;
    }

    public IndentingPrintWriter (Writer out) {
	super( out, true ) ;
	// System.out.println( "Constructing a new IndentingPrintWriter with Writer " + out ) ;
    }

    public IndentingPrintWriter(Writer out, boolean autoFlush) {
	super( out, autoFlush ) ;
	// System.out.println( "Constructing a new IndentingPrintWriter with Writer " + out ) ;
    }

    public IndentingPrintWriter(OutputStream out) {
	super(out, true);
	// System.out.println( "Constructing a new IndentingPrintWriter with OutputStream " + out ) ;
    }

    public IndentingPrintWriter(OutputStream out, boolean autoFlush) {
	super(new BufferedWriter(new OutputStreamWriter(out)), autoFlush);
	// System.out.println( "Constructing a new IndentingPrintWriter with OutputStream " + out ) ;
    }

    public void setIndentWidth( int indentWidth )
    {
	this.indentWidth = indentWidth ;
	updateIndentString() ;
    }

    public void indent()
    {
	level++ ;
	updateIndentString() ;
    }

    public void undent()
    {
	if (level > 0) {
	    level-- ;
	    updateIndentString() ;
	}
    }

    private void updateIndentString()
    {
	int size = level * indentWidth ;
	StringBuffer sbuf = new StringBuffer( size ) ;
	for (int ctr = 0; ctr<size; ctr++ )
	    sbuf.append( " " ) ;
	indentString = sbuf.toString() ;
    }

    // overridden from PrintWriter
    public void println() 
    {
	super.println() ;

	print( indentString ) ;
    }
}
