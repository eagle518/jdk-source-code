/*
 * @(#)NativeGen.java	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.corba.se.idl;

// NOTES:

import java.io.PrintWriter;
import java.util.Hashtable;

public interface NativeGen extends Generator
{
  void generate (Hashtable symbolTable, NativeEntry entry, PrintWriter stream);
} // interface NativeGen

