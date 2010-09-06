/*
 * @(#)ProfileActivator.java	1.6 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.color;


/**
 * An interface to allow the ProfileDeferralMgr to activate a
 * deferred profile.
 */
public interface ProfileActivator {

    /**
     * Activate a previously deferred ICC_Profile object.
     */
    public void activate();

}
