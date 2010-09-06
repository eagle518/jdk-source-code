/*
 * @(#)AuxGen.java	1.12 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*
 * COMPONENT_NAME: idl.toJava
 *
 * ORIGINS: 27
 *
 * Licensed Materials - Property of IBM
 * 5639-D57 (C) COPYRIGHT International Business Machines Corp. 1997, 1999
 * RMI-IIOP v1.0
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * @(#)AuxGen.java	1.12 03/12/19
 */

package com.sun.tools.corba.se.idl.toJavaPortable;

// NOTES:

/**
 *
 **/
public interface AuxGen
{
  void generate (java.util.Hashtable symbolTable, com.sun.tools.corba.se.idl.SymtabEntry entry);
} // interface AuxGen
