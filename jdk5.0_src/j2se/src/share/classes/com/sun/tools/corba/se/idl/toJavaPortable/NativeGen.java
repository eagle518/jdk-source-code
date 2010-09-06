/*
 * @(#)NativeGen.java	1.5 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.corba.se.idl.toJavaPortable;

// NOTES:

import java.io.File;
import java.io.PrintWriter;
import java.util.Hashtable;
import java.util.Vector;

import com.sun.tools.corba.se.idl.GenFileStream;
import com.sun.tools.corba.se.idl.NativeEntry;
import com.sun.tools.corba.se.idl.ModuleEntry;
import com.sun.tools.corba.se.idl.PrimitiveEntry;
import com.sun.tools.corba.se.idl.StringEntry;
import com.sun.tools.corba.se.idl.SymtabEntry;
import com.sun.tools.corba.se.idl.TypedefEntry;

/**
 *
 **/
public class NativeGen implements com.sun.tools.corba.se.idl.NativeGen
{
  /**
   * Public zero-argument constructor.
   **/
  public NativeGen ()
  {
  } // ctor

  /**
   * Generate Java code for an IDL constant.  A constant is written to
   * a new class only when it is not a member of an interface; otherwise
   * it written to the interface class in which it resides.
   **/
  public void generate (Hashtable symbolTable, NativeEntry c, PrintWriter s)
  {
	// noop, do not generate anything
  } // generate

} // class NativeGen

