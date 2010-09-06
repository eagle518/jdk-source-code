/*
 * @(#)AttrFilter.java	1.6 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.jndi.toolkit.dir;

import javax.naming.NamingException;
import javax.naming.directory.Attributes;

/**
  * Is implemented by classes that can perform filter checks on
  * an attribute set.
  */

public interface AttrFilter {

    /**
      * Determines whether an attribute passes the filter.
      */
    public boolean check(Attributes targetAttrs) throws NamingException;
}
