/*
 * @(#)NewThreadAction.java	1.5 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.rmi.runtime;

/**
 * NewThreadAction is a java.security.PrivilegedAction for creating a new
 * thread conveniently with a java.security.AccessController.doPrivileged
 * construct.
 *
 * All constructors allow the choice of the Runnable for the new thread
 * to execute, the name of the new thread (which will be prefixed with
 * "RMI "), and whether or not it will be a daemon thread.
 *
 * The new thread may be created in the system thread group (the root of
 * the thread group tree) or an internally created non-system thread group,
 * as specified at construction of this class.
 *
 * The new thread will have the system class loader as its initial context
 * class loader (that is, its context class loader will NOT be inherited
 * from the current thread).
 *
 * @author	Peter Jones
 * @version	1.5, 03/12/19
 */
public final class NewThreadAction implements java.security.PrivilegedAction {

    /** cached reference to the system (root) thread group */
    static final ThreadGroup systemThreadGroup = (ThreadGroup)
        java.security.AccessController.doPrivileged(
	new java.security.PrivilegedAction() {
	    public Object run() {
		ThreadGroup group = Thread.currentThread().getThreadGroup();
		ThreadGroup parent;
		while ((parent = group.getParent()) != null) {
		    group = parent;
		}
		return group;
	    }
	});

    /**
     * special child of the system thread group for running tasks that
     * may execute user code, so that the security policy for threads in
     * the system thread group will not apply
     */
    static final ThreadGroup userThreadGroup = (ThreadGroup)
        java.security.AccessController.doPrivileged(
	new java.security.PrivilegedAction() {
	    public Object run() {
		return new ThreadGroup(systemThreadGroup, "RMI Runtime");
	    }
	});

    private final ThreadGroup group;
    private final Runnable runnable;
    private final String name;
    private final boolean daemon;

    NewThreadAction(ThreadGroup group, Runnable runnable,
		    String name, boolean daemon)
    {
	this.group = group;
	this.runnable = runnable;
	this.name = name;
	this.daemon = daemon;
    }

    /**
     * Creates an action that will create a new thread in the
     * system thread group.
     *
     * @param	runnable the Runnable for the new thread to execute
     *
     * @param	name the name of the new thread
     *
     * @param	daemon if true, new hread will be a daemon thread;
     * if false, new thread will not be a daemon thread
     */
    public NewThreadAction(Runnable runnable, String name, boolean daemon) {
	this(systemThreadGroup, runnable, name, daemon);
    }

    /**
     * Creates an action that will create a new thread.
     *
     * @param	runnable the Runnable for the new thread to execute
     *
     * @param	name the name of the new thread
     *
     * @param	daemon if true, new hread will be a daemon thread;
     * if false, new thread will not be a daemon thread
     *
     * @param	user if true, thread will be created in a non-system
     * thread group; if false, thread will be created in the system
     * thread group
     */
    public NewThreadAction(Runnable runnable, String name, boolean daemon,
			   boolean user)
    {
	this(user ? userThreadGroup : systemThreadGroup,
	     runnable, name, daemon);
    }

    public Object run() {
	Thread t = new Thread(group, runnable, "RMI " + name);
	t.setContextClassLoader(ClassLoader.getSystemClassLoader());
	t.setDaemon(daemon);
	return t;
    }
}
