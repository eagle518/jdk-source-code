/*
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.association.utility;


/**
 * Create an instance of GnomeAppAssociationReader
 *
 * @version 1.0
 */
public class AppAssociationReaderFactory {
    public static AppAssociationReader newInstance() {
        return new GnomeAppAssociationReader();
    }
}
