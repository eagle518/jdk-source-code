/*
 * @(#)WorldGroupImpl.java	1.13 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
