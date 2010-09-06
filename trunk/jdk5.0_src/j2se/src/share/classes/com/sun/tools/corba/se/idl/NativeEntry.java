/*
 * @(#)NativeEntry.java	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.corba.se.idl;

// NOTES:

import java.io.PrintWriter;
import java.util.Hashtable;

/**
 * This is the symbol table entry for constants.
 **/
public class NativeEntry extends SymtabEntry
{
  protected NativeEntry ()
  {
    super ();
    repositoryID (Util.emptyID);
  } // ctor

  protected NativeEntry (SymtabEntry that, IDLID clone)
  {
    super (that, clone);
    if (module ().equals (""))
      module (name ());
    else if (!name ().equals (""))
      module (module () + "/" + name ());
  } // ctor

  protected NativeEntry (NativeEntry that)
  {
    super (that);
  } // ctor

  /** This is a shallow copy clone. */
  public Object clone ()
  {
    return new NativeEntry (this);
  } // clone

  /** Invoke the constant generator.
      @param symbolTable the symbol table is a hash table whose key is
       a fully qualified type name and whose value is a SymtabEntry or
       a subclass of SymtabEntry.
      @param stream the stream to which the generator should sent its output.
      @see SymtabEntry */
  public void generate (Hashtable symbolTable, PrintWriter stream)
  {
    nativeGen.generate(symbolTable, this, stream);
  } // generate

  /** Access the constant generator.
      @returns an object which implements the ConstGen interface.
      @see ConstGen */
  public Generator generator ()
  {
    return nativeGen;
  } // generator

  static NativeGen nativeGen;
} // class NativeEntry
