/*
 * @(#)IllegalConnectorArgumentsException.java	1.12 04/05/05
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jdi.connect;

import java.util.List;
import java.util.ArrayList;
import java.util.Collections;

/**
 * Thrown to indicate an invalid argument or 
 * inconsistent passed to a {@link Connector}.
 *
 * @author Gordon Hirsch
 * @since  1.3
 */
public class IllegalConnectorArgumentsException extends Exception
{
    List names;

    /**
     * Construct an <code>IllegalConnectorArgumentsException</code>
     * with the specified detail message and the name of the argument
     * which is invalid or inconsistent.
     * @param s the detailed message.
     * @param name the name of the invalid or inconsistent argument.
     */    
    public IllegalConnectorArgumentsException(String s,
                                              String name) {
        super(s);
        names = new ArrayList(1);
        names.add(name);
    }

    /**
     * Construct an <code>IllegalConnectorArgumentsException</code>
     * with the specified detail message and a <code>List</code> of
     * names of arguments which are invalid or inconsistent.
     * @param s the detailed message.
     * @param names a <code>List</code> containing the names of the
     * invalid or inconsistent argument.
     */
    public IllegalConnectorArgumentsException(String s, List<String> names) {
        super(s);

        this.names = new ArrayList(names);
    }

    /**
     * Return a <code>List</code> containing the names of the
     * invalid or inconsistent arguments.
     * @return a <code>List</code> of argument names.
     */
    public List<String> argumentNames() {
        return Collections.unmodifiableList(names);
    }
}
