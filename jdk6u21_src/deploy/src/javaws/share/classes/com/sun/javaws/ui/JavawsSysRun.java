/*
 * 10/03/24 @(#)JavawsSysRun.java	1.9
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws.ui;

import java.awt.event.WindowEvent;
import java.awt.event.WindowAdapter;
import java.awt.Point;
import java.awt.Frame;
import java.awt.Rectangle;
import java.awt.Toolkit;
import javax.swing.SwingUtilities;
import java.security.AccessController;
import java.security.PrivilegedAction;

import com.sun.javaws.Main;
import com.sun.javaws.security.AppContextUtil;

import com.sun.deploy.util.DeploySysAction;
import com.sun.deploy.util.DeploySysRun;
import com.sun.deploy.util.Trace;
import com.sun.deploy.config.Config;
import java.util.LinkedList;
import javax.swing.JDialog;

public final class JavawsSysRun extends DeploySysRun 
{
    private final SecureThread t = new SecureThread();
         
    class Job {
        DeploySysAction action;
        DummyDialog dialog;
        Object result;
        Exception exception;
        boolean done;
        
        Job(DeploySysAction a) {
            action = a;
            done = false;
        }
        
        void setDialog(DummyDialog d) {
            dialog = d;
        }
    }
        
    private void delegateFromEDT(final Job job) throws Exception 
    {
        /* 
         * The problem:
         *    delegate() may be called from the EDT of application appContext
         *    and it may take a while to complete (e.g. require user 
         *    confirmation or downloading something from the network).
         *    In such case application EDT will be blocked and it will 
         *    be completely irresponsive.
         * 
         * Creation of modal dialog leads to creation of extra event pump 
         * and therefore application will be able to process events even 
         * while EDT is blocked. Obviously we do not want dialog to be actually 
         * visible on the screen (see 4516924)
         * 
         * Event pump will be actually created in the setVisible(true) but 
         * it will also block execution until dialog is hidden. 
         * Given that dialog is hidden by worker thread when job js complete 
         * we can safely use setVisible() as sync mechanism here.
         * 
         * Technically setVisible() implements blocking as a recursive call
         * that starts new event pump. As a result we may have a situation when
         * we have another call to delegate() from the same EDT while first call 
         * was in progress. 
         * 
         * In such case synchronized statements will not help because 
         * it will be the same thread.
         * However, first setVisible() will not exit until additional event pump
         * finishes processing of the current event (even if dialog was hidden!).
         * Therefore setVisible() also prevents potential problems if we have 
         * such "recursive" calls to delegate().
         * 
         * Note that we may have another non-EDT action in progress when first 
         * action from EDT comes. In such case we will create and show dialog.
         * And then wait in the listener until we can add action to queue.
         * This way we will have event pump started and will not interfere with
         * action in progress.
         * 
         * The drawback of this approach is that "recursive" actions will be 
         * actually complete in reverse order. I.e. if we have event queue with 
         * 2 events that trigger time consuming delegate() calls - A and B (in this order). 
         * Then execution sequence will be following:
         *  - create and show new dialog d1 that starts  new event pump P1 and 
         *    block execution of delegate() (with recursive call)
         *  - queue A for execution and trigger its execution in the secureThread
         *  - start processing event B: create and show dialog d2 and new event pump P2.
         *    Block second delegate call(). 
         *  - Wait till action A is completed before we can add new action B to queue
         *    (Meanwhile P2 keeps processing other events)
         *  - When A is complete, d1 is hidden and mutex is released.
         *    We add B to queue and secure thread starts working on it. 
         *    Results of A are kept in the job structure.
         *  - When B is complete, d2 is hidden. P2 is shut and second 
         *    setVisible call returns. We may pick and return to callee 
         *    results of B call.
         *  - "Recursive" delegate() call is over and P1 can be shut down.
         *    Results of execution of A are returned.
         * 
         * I.e. events will be processed in the order of submission (A, B) but results 
         * will be returned in the opposite order (B, A). 
         * Technically, this may be unexpected behavior but it is very unlikely 
         * anyone ever will rely on this.
         */

         /* NB: consider removal of DummyDialog class and 
          *     usage of plain dialog and direct call to postEvent() 
          *     with specific target */
         final DummyDialog dummyDialog = new DummyDialog();
         job.setDialog(dummyDialog);
              
         if (Config.getOSName().equals("Windows")) {
            // make the dummy dialog off screen
            dummyDialog.setLocation(-200, -200);
         } else {
            Rectangle rect = new Rectangle(new Point(0, 0),
                            Toolkit.getDefaultToolkit().getScreenSize());
            dummyDialog.setLocation(rect.x + rect.width/2 - 50,
            rect.y + rect.height/2);
         }
         dummyDialog.setResizable(false);
         dummyDialog.toBack();

         dummyDialog.addWindowListener(new WindowAdapter() {
             public void windowOpened(WindowEvent e) {
                 synchronized (t.mutex) {
                   t.addJob(job);
                   t.mutex.notifyAll();
                 }
             }
         });
         /* we will stuck here until t will close the dialog 
            AND "recursive" calls from EDT will end */
         dummyDialog.setVisible(true);
         
         dummyDialog.dispose();
    }
    
    /* This method ensures that action is executes in the context of security 
     * thread group.
     * Technically this is done by executing it in the context of thread t. */
    public Object delegate(DeploySysAction action) throws Exception
    {
	// If the calling thread is in the security thread group just call it	
	if (Thread.currentThread().getThreadGroup().equals(
		Main.getSecurityThreadGroup())) {
	    return action.execute();
	}

       // Have to execute action on the secure thread. 
        //
        // There are 2 different cases:
        //   1) callee thread is EDT from application context
        //   2) callee thread is non EDT thread
        // 
        // It is important to understand there could be multiple 
        // simultaneous calls to delegate(). This could happen 
        //   a) grom the different threads
        //   b) from the same EDT thread (see comments in the delegateFromEDT()
        //
        // Our secure worker thread can only work on one action at time.
        // So, we have list of jobs and sync access to it using t.mutex
        // Any change in the list is accompanied by notifyAll() to notify
        // all interested parties.
        Job job = new Job(action);

        if ((AppContextUtil.isApplicationAppContext()) &&
              SwingUtilities.isEventDispatchThread()) {
            delegateFromEDT(job);
        } else {
            synchronized(t.mutex) {            
                /* If we accured mutex here this means that either
                 *   a) t is waiting to do work 
                 *   b) or t is starting.
                 * In any case we can safely put our job to the queue.
                 * If t is still starting it will check queue before calling wait().
                 * If it is in the wait() state we will wake it up. */
                t.addJob(job);
                t.mutex.notifyAll();

                /* wait till job is not finished.
                 * Go back to wait() if we were woken up by mistake. */
                while (!job.done) {
                                       
                    try {
                         t.mutex.wait();
                    } catch(InterruptedException e) { }
                }
          /* we done, notify all that mutex is free */
                t.mutex.notifyAll();
            }
         
        }
	 if (job.exception != null)
            throw job.exception;
        return job.result;
    }
    
    /**
     * invokeLater
     * This method is specifically for making modifications to dialogs 
     * created in out secure app context, when modifications may be attempted
     * from the applications app context. (such as the download progress dlg)
     * Runs the given code on the event queue thread of the secure AppContext.
     */
    static void invokeLater(final Runnable runner) {
	if (Thread.currentThread().getThreadGroup().equals(
		Main.getSecurityThreadGroup())) {
	    SwingUtilities.invokeLater(runner);
	    return;
	}
	final Runnable invoker = new Runnable() {
	    public void run() {
                SwingUtilities.invokeLater( runner );
            }
        };
        AccessController.doPrivileged(new PrivilegedAction() {
            public Object run() {			
                Thread t = new Thread(Main.getSecurityThreadGroup(), invoker);
                t.setContextClassLoader(Main.getSecureContextClassLoader());
                t.start();
                return null;
            }
        });
    }
    
    /**
     * We use modal dialog to ensure application has working event pump
     * even if delegate() was called from EDT.
     * However, additional event pump is created only after setVisible(true).
     * And setVisible(true) will block thread until dialog is hidden. 
     * 
     * To workaround this we are subclassing Dialog to provide method to 
     * reliably hide dialog from arbitrary appcontext.
     * 
     * NB: consider replacing this class with plain dialog and postEvent() call
     * 
     * Note: JDialog is used here because it does have icon in the task bar
     *   (and java.awt.Dialog will create extra element in the windows taskbar!)
     */
    private class DummyDialog extends JDialog {   
        private ThreadGroup _callingTG;

        DummyDialog() {
            super((Frame)null, true);
            _callingTG = Thread.currentThread().getThreadGroup();
        }

        public void secureHide() {   
            (new Thread(_callingTG, new Runnable() {
                public void run() { 
		    DummyDialog.this.setVisible(false);
                }
            })).start();
        }
    }    

    /**
     * SecureThread is a thread that is running inside the secure thread group.
     */
    class SecureThread extends Thread {

        Object mutex = new Object();  //used to sync operations on jobList
        //and notify about jobList updates
        LinkedList jobList = new LinkedList();

        SecureThread() {
            super(Main.getSecurityThreadGroup(), "Javaws Secure Thread");
            setDaemon(true);
            setContextClassLoader(Main.getSecureContextClassLoader());
            start();
        }

        void addJob(Job job) {
            jobList.add(job);
        }

        private void doWork(Job job) {
            try {
                // also set contextClassLoader in the eventqueue thread
                SwingUtilities.invokeLater(new Runnable() {

                    public void run() {
                        Thread.currentThread().setContextClassLoader(
                                Main.getSecureContextClassLoader());
                    }
                });
                job.result = job.action.execute();
            } catch (Exception e) {
                job.exception = e;
            } finally {
                job.done = true;
                if (job.dialog != null) {
                    job.dialog.secureHide();
                }
            }
        }

        public void run() {
            synchronized (mutex) {
                /* Check for pending requests BEFORE we go wait().
                We might have got some requests during startup
                of this thread(). */
                while (true) {
                    if (!jobList.isEmpty()) {
                        Job job = (Job) jobList.removeFirst();
                        doWork(job);
                    } else {
                        mutex.notifyAll();

                        /* Our approach is to process whole queue and then
                         * notify other threads that job is done.
                         * While this may increase may increase wait time for 
                         * other threads we need to properly support "recursive"
                         * calls from EDT. 
                         * 
                         * If we go wait() here after we done "outer" job A
                         * then we may wait forever because thread waiting for A
                         * need to get results of action B first and this one 
                         * is still in queue! 
                         * (see comments in the delegateFromEDT()) */
                        try {
                            mutex.wait();
                        } catch (InterruptedException e) {
                            Trace.ignoredException(e);
                        }
                    }
                }
            }
        }
    }
}
