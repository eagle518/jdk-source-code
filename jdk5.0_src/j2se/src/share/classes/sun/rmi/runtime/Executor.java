/*
 * @(#)Executor.java	1.8 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.rmi.runtime;

/**
 * Executor is an abstraction for a thread factory or thread pool for
 * executing actions asynchronously.
 *
 * @author	Peter Jones
 * @version	1.8, 03/12/19
 */
public interface Executor {

    /**
     * Executes the given Runnable action asynchronously in some thread.
     *
     * The implemention may create a new thread to execute the action,
     * or it may execute the action in an existing thread.
     *
     * The execution of a given action must not be delayed indefinitely
     * in order to complete execution of a different action passed to a
     * different invocation of this method.  In other words, the
     * implementation must assume that there may be arbitrary dependencies
     * between actions passed to this method, so it needs to be careful
     * to avoid potential deadlock by delaying execution of one action
     * indefinitely until another completes.
     *
     * Also, this method itself must not block, because it may be invoked
     * by code that is serially processing data to produce multiple such
     * arbitrarily-dependent actions that need to be executed.
     *
     * @param	runnable the Runnable action to execute
     *
     * @name	name string to include in the name of the thread used
     * to execute the action
     */
    void execute(Runnable runnable, String name);
}
