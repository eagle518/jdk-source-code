/*
 * @(#)CustomUIDefault.java	1.4 10/03/23
 *
 * Copyright (c) 2006, 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package org.jdesktop.synthdesigner.synthmodel;

/**
 * CustomUIDefault
 *
 * @author  Richard Bair
 * @author  Jasper Potts
 */
public class CustomUIDefault<T> extends UIDefault<T> {
    private static int counter = -1;

    public CustomUIDefault() {
        super("Unnamed" + (++counter == 0 ? "" : counter), null);
    }

    public void setName(String id) {
        super.setName(id);
    }
}
