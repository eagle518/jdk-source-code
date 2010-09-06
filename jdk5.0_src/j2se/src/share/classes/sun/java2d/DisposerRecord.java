/*
 * @(#)DisposerRecord.java	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.java2d;

/**
 * This class is used to hold the resource to be 
 * disposed.
 */
public abstract class DisposerRecord {
    public abstract void dispose();
}
