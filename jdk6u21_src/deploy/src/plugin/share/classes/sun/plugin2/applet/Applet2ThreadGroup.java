/*
 * @(#)Applet2ThreadGroup.java	1.2 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.applet;

/**
 * This class defines an applet thread group.
 *
 * @version 	1.2, 03/24/10
 * @author 	Arthur van Hoff
 */
public class Applet2ThreadGroup extends ThreadGroup {

    /**
     * Constructs a new thread group for an applet. 
     * The parent of this new group is the thread
     * group of the currently running thread.
     *
     * @param   name   the name of the new thread group.
     */
    public Applet2ThreadGroup(String name) {
        this(Thread.currentThread().getThreadGroup(), name);
    }

    /**
     * Creates a new thread group for an applet. 
     * The parent of this new group is the specified 
     * thread group.
     *
     * @param     parent   the parent thread group.
     * @param     name     the name of the new thread group.
     * @exception  NullPointerException  if the thread group argument is
     *               <code>null</code>.
     * @exception  SecurityException  if the current thread cannot create a
     *               thread in the specified thread group.
     * @see     java.lang.SecurityException
     * @since   JDK1.1.1
     */
    public Applet2ThreadGroup(ThreadGroup parent, String name) {
	super(parent, name);
	setMaxPriority(Thread.NORM_PRIORITY - 1);
    }
}
