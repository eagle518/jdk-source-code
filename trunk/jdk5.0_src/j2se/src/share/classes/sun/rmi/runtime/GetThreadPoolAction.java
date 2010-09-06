/*
 * @(#)GetThreadPoolAction.java	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.rmi.runtime;

/**
 * GetThreadPoolAction provides security-checked access to the runtime's
 * internal thread pools as a java.security.PrivilegedAction, to be used
 * conveniently with a java.security.AccessController.doPrivileged
 * construct.
 *
 * There are two internal thread pools: one of threads in the system
 * thread group, for executing tasks to be guarded by the security policy
 * for the system thread group, and one of threads in a non-system thread
 * group, for executing tasks with user code that should not be restricted
 * by that policy.
 *
 * @author	Peter Jones
 * @version	1.4, 03/12/19
 */
public final class GetThreadPoolAction
    implements java.security.PrivilegedAction
{
    /** pool of threads for executing tasks in system thread group */
    private static final ThreadPool systemThreadPool =
	new ThreadPool(NewThreadAction.systemThreadGroup);

    /** pool of threads for executing tasks with user code */
    private static final ThreadPool userThreadPool =
	new ThreadPool(NewThreadAction.userThreadGroup);

    private final boolean user;

    /**
     * Creates an action that will obtain an internal thread pool.
     * When run, this action verifies that the current access control
     * context has permission to access the thread group used by the
     * indicated pool.
     *
     * @param	user if true, will obtain the non-system thread group
     * pool for executing user code; if false, will obtain the system
     * thread group pool
     */
    public GetThreadPoolAction(boolean user) {
	this.user = user;
    }

    public Object run() {
	SecurityManager sm = System.getSecurityManager();
	if (sm != null) {
	    sm.checkAccess(user ? NewThreadAction.userThreadGroup :
			   NewThreadAction.systemThreadGroup);
	}
	return user ? userThreadPool : systemThreadPool;
    }
}
