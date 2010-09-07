/*
 * @(#)HasUIDefaults.java	1.3 10/03/23
 *
 * Copyright (c) 2006, 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package org.jdesktop.swingx.designer.utils;

import javax.swing.UIDefaults;

/**
 * HasUIDefaults - A tagging interface for any class that has UIDefaults
 *
 * @author Created by Jasper Potts (Jun 22, 2007)
 * @version 1.0
 */
public interface HasUIDefaults {
    public UIDefaults getUiDefaults();
}
