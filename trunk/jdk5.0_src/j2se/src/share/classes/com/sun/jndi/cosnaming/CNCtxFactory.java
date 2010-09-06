/*
 * @(#)CNCtxFactory.java	1.8 04/07/16
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jndi.cosnaming;

import javax.naming.spi.InitialContextFactory;
import javax.naming.*;

import java.util.Hashtable;

/**
  * Implements the JNDI SPI InitialContextFactory interface used to
  * create  the InitialContext objects.
  *
  * @author Raj Krishnamurthy
  * @version 1.8 04/07/16
  */

public class CNCtxFactory implements InitialContextFactory {

  /**
    * Creates the InitialContext object. Properties parameter should
    * should contain the ORB object for the value jndi.corba.orb.
    * @param env Properties object
    */

  public Context getInitialContext(Hashtable<?,?> env) throws NamingException {
      return new CNCtx(env);
  }
}

