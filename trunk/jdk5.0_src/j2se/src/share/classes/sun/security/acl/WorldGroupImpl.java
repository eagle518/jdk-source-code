/*
 * @(#)WorldGroupImpl.java	1.11 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.security.acl;

import java.security.*;

/**
 * This class implements a group of principals.
 * @author Satish Dharmaraj
 */
public class WorldGroupImpl extends GroupImpl {

    public WorldGroupImpl(String s) {
	super(s);
    }

    /**
     * returns true for all passed principals
     * @param member The principal whose membership must be checked in this Group.
     * @return true always since this is the "world" group.
     */
    public boolean isMember(Principal member) {
	return true;
    }
}
