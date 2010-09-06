/*
 * @(#)FontSupport.java	1.9 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.java2d;

import sun.awt.FontConfiguration;

/**
 * Font support for graphics environment
 */
public interface FontSupport {

    /**
     * Returns the current font configuration.
     */
    public FontConfiguration getFontConfiguration();
}
