/*
 * @(#)HasUIStyle.java	1.4 10/03/23
 *
 * Copyright (c) 2006, 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package org.jdesktop.synthdesigner.synthmodel;

/**
 * HasUIStyle - A marker interface for all classes that have a UIStyle
 *
 * @author  Richard Bair
 * @author  Jasper Potts
 */
public interface HasUIStyle {

    public UIStyle getStyle();
}
