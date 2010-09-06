/*
 * @(#)DisposerTarget.java	1.3 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.java2d;

/**
 * This is an interface which should be implemented by
 * the classes which use Disposer.
 */
public interface DisposerTarget {
    /**
     * Returns an object which will be
     * used as the referent in the ReferenceQueue
     */
    public Object getDisposerReferent();
}
