/*
 * @(#)InterfaceState.java	1.12 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*
 * COMPONENT_NAME: idl.parser
 *
 * ORIGINS: 27
 *
 * Licensed Materials - Property of IBM
 * 5639-D57 (C) COPYRIGHT International Business Machines Corp. 1997, 1999
 * RMI-IIOP v1.0
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * @(#)InterfaceState.java	1.12 03/12/19
 */

package com.sun.tools.corba.se.idl;

// NOTES:

/**
 * This class is only used within an InterfaceEntry.  If the interface
 * is stateful, then its state vector will contain one or more of these
 * InterfaceStates.
 **/
public class InterfaceState
{
  public static final int Private   = 0,
                          Protected = 1,
                          Public    = 2;

  public InterfaceState (int m, TypedefEntry e)
  {
    modifier = m;
    entry    = e;
    if (modifier < Private || modifier > Public)
      modifier = Public;
  } // ctor

  public int          modifier = Public;
  public TypedefEntry entry    = null;
} // class InterfaceState
