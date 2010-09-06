/*
 * @(#)ThreadPool.java	1.5 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.rmi.runtime;

import java.util.LinkedList;

/**
 * ThreadPool is a simple thread pool implementation of the Executor
 * interface.
 *
 * A new task is always given to an idle thread, if one is available;
 * otherwise, a new thread is always created.  There is no minimum
 * warm thread count, nor is there a maximum thread count (tasks are
 * never queued unless there are sufficient idle threads to execute
 * them).
 *
 * New threads are created as daemon threads in the thread group that
 * was passed to the ThreadPool instance's constructor.  Each thread's
 * name is the prefix "RMI " followed by the name of the task it is
 * currently executing, or "Idle" if it is currently idle.
 *
 * @author	Peter Jones
 * @version	1.5, 03/12/19
 */
final class ThreadPool implements Executor {

    /** how long a thread waits in the idle state before passing away */
    private static final long idleTimeout =		// default 5 minutes
	((Long) java.security.AccessController.doPrivileged(
	    new sun.security.action.GetLongAction(
		"sun.rmi.jrmp.idleThreadTimeout", 300000))).longValue();

    /** thread group that this pool's threads execute in */
    private final ThreadGroup threadGroup;

    /** lock guarding all mutable instance state (below) */
    private final Object lock = new Object();

    /** total threads running in this pool (not currently used) */
    private int totalThreads = 0;

    /** threads definitely available to take new tasks */
    private int idleThreads = 0;

    /** queues of tasks to execute */
    private final LinkedList queue = new LinkedList();

    /**
     * Creates a new thread group that executes tasks in threads of
     * the given thread group.
     */
    ThreadPool(ThreadGroup threadGroup) {
	this.threadGroup = threadGroup;
    }

    public void execute(Runnable runnable, String name) {
	Task task = new Task(runnable, name);
	synchronized (lock) {
	    if (queue.size() < idleThreads) {
		queue.addLast(task);
		lock.notify();
	    } else {
		Thread t = (Thread)
		    java.security.AccessController.doPrivileged(
			new NewThreadAction(threadGroup, new Worker(task),
					    name, true));
		t.start();
		totalThreads++;
	    }
	}
    }

    /**
     * Task simply encapsulates a task's Runnable object with its name.
     */
    private static class Task {

	final Runnable runnable;
	final String name;

	Task(Runnable runnable, String name) {
	    this.runnable = runnable;
	    this.name = name;
	}
    }

    /**
     * Worker executes an initial task, and then it executes tasks from the
     * queue, passing away if ever idle for more than the idle timeout value.
     */
    private class Worker implements Runnable {

	private Task first;

	Worker(Task first) {
	    this.first = first;
	}

	public void run() {
	    try {
		Task task = first;
		first = null;

		while (true) {
		    task.runnable.run();
		    /*
		     * REMIND: What if the task changed this thread's
		     * priority? or context class loader?
		     */

		    synchronized (lock) {
			if (queue.isEmpty()) {
			    Thread.currentThread().setName("RMI Idle");
			    idleThreads++;
			    try {
				lock.wait(idleTimeout);
			    } catch (InterruptedException e) {
				// ignore interrupts at this level
			    } finally {
				idleThreads--;
			    }
			    if (queue.isEmpty()) {
				break;		// timed out
			    }
			}
			task = (Task) queue.removeFirst();
			Thread.currentThread().setName("RMI " + task.name);
		    }
		};
		
	    } finally {
		synchronized (lock) {
		    totalThreads--;
		}
	    }
	}
    }
}
