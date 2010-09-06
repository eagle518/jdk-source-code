/*
 * @(#)ContextIdentifierHelper.java	1.10 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.org.omg.CORBA;


/**
* com/sun/org/omg/CORBA/ContextIdentifierHelper.java
* Generated by the IDL-to-Java compiler (portable), version "3.0"
* from ir.idl
* Thursday, May 6, 1999 1:51:50 AM PDT
*/

public final class ContextIdentifierHelper
{
    private static String  _id = "IDL:omg.org/CORBA/ContextIdentifier:1.0";

    public ContextIdentifierHelper()
    {
    }

    public static void insert (org.omg.CORBA.Any a, String that)
    {
	org.omg.CORBA.portable.OutputStream out = a.create_output_stream ();
	a.type (type ());
	write (out, that);
	a.read_value (out.create_input_stream (), type ());
    }

    public static String extract (org.omg.CORBA.Any a)
    {
	return read (a.create_input_stream ());
    }

    private static org.omg.CORBA.TypeCode __typeCode = null;
    synchronized public static org.omg.CORBA.TypeCode type ()
    {
	if (__typeCode == null)
	    {
		__typeCode = org.omg.CORBA.ORB.init ().create_string_tc (0);
		__typeCode = org.omg.CORBA.ORB.init ().create_alias_tc (com.sun.org.omg.CORBA.IdentifierHelper.id (), "Identifier", __typeCode);
		__typeCode = org.omg.CORBA.ORB.init ().create_alias_tc (com.sun.org.omg.CORBA.ContextIdentifierHelper.id (), "ContextIdentifier", __typeCode);
	    }
	return __typeCode;
    }

    public static String id ()
    {
	return _id;
    }

    public static String read (org.omg.CORBA.portable.InputStream istream)
    {
	String value = null;
	value = istream.read_string ();
	return value;
    }

    public static void write (org.omg.CORBA.portable.OutputStream ostream, String value)
    {
	ostream.write_string (value);
    }

}