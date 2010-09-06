/*
 * @(#)JDIEventSource.java	1.9 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*
 * Copyright (c) 1997-1999 by Sun Microsystems, Inc. All Rights Reserved.
 * 
 * Sun grants you ("Licensee") a non-exclusive, royalty free, license to use,
 * modify and redistribute this software in source and binary code form,
 * provided that i) this copyright notice and license appear on all copies of
 * the software; and ii) Licensee does not utilize the software in a manner
 * which is disparaging to Sun.
 * 
 * This software is provided "AS IS," without a warranty of any kind. ALL
 * EXPRESS OR IMPLIED CONDITIONS, REPRESENTATIONS AND WARRANTIES, INCLUDING ANY
 * IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NON-INFRINGEMENT, ARE HEREBY EXCLUDED. SUN AND ITS LICENSORS SHALL NOT BE
 * LIABLE FOR ANY DAMAGES SUFFERED BY LICENSEE AS A RESULT OF USING, MODIFYING
 * OR DISTRIBUTING THE SOFTWARE OR ITS DERIVATIVES. IN NO EVENT WILL SUN OR ITS
 * LICENSORS BE LIABLE FOR ANY LOST REVENUE, PROFIT OR DATA, OR FOR DIRECT,
 * INDIRECT, SPECIAL, CONSEQUENTIAL, INCIDENTAL OR PUNITIVE DAMAGES, HOWEVER
 * CAUSED AND REGARDLESS OF THE THEORY OF LIABILITY, ARISING OUT OF THE USE OF
 * OR INABILITY TO USE SOFTWARE, EVEN IF SUN HAS BEEN ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGES.
 * 
 * This software is not designed or intended for use in on-line control of
 * aircraft, air traffic, aircraft navigation or aircraft communications; or in
 * the design, construction, operation or maintenance of any nuclear
 * facility. Licensee represents and warrants that it will not use or
 * redistribute the Software for such purposes.
 */
package com.sun.tools.example.debug.bdi;

import com.sun.jdi.*;
import com.sun.jdi.event.*;

import java.util.*;

import com.sun.tools.example.debug.event.*;

import javax.swing.SwingUtilities;

/**
 */
class JDIEventSource extends Thread {

    private /*final*/ EventQueue queue;
    private /*final*/ Session session;
    private /*final*/ ExecutionManager runtime;
    private final JDIListener firstListener = new FirstListener();

    private boolean wantInterrupt;  //### Hack

    /**
     * Create event source.
     */
    JDIEventSource(Session session) {
        super("JDI Event Set Dispatcher");
	this.session = session;
        this.runtime = session.runtime;
        this.queue = session.vm.eventQueue();
    }
    
    public void run() {
        try {
            runLoop();
        } catch (Exception exc) {
            //### Do something different for InterruptedException???
            // just exit
        }
        session.running = false;
    }

    private void runLoop() throws InterruptedException {
        AbstractEventSet es;
        do {
            EventSet jdiEventSet = queue.remove();
            es = AbstractEventSet.toSpecificEventSet(jdiEventSet);
	    session.interrupted = es.suspendedAll();
	    dispatchEventSet(es);
        } while(!(es instanceof VMDisconnectEventSet));
    }

    //### Gross foul hackery!
    private void dispatchEventSet(final AbstractEventSet es) {
        SwingUtilities.invokeLater(new Runnable() {
            public void run() {
                boolean interrupted = es.suspendedAll();
                es.notify(firstListener);
                boolean wantInterrupt = JDIEventSource.this.wantInterrupt;
                for (Iterator it = session.runtime.jdiListeners.iterator(); 
                     it.hasNext(); ) {
                    JDIListener jl = (JDIListener)it.next();
                    es.notify(jl);
                }
                if (interrupted && !wantInterrupt) {
                    session.interrupted = false;
                    //### Catch here is a hack
                    try {
                        session.vm.resume();
                    } catch (VMDisconnectedException ee) {}
                }
                if (es instanceof ThreadDeathEventSet) {
                    ThreadReference t = ((ThreadDeathEventSet)es).getThread();
                    session.runtime.removeThreadInfo(t);
                }
            }
        });
    }

    private void finalizeEventSet(AbstractEventSet es) {
        if (session.interrupted && !wantInterrupt) {
            session.interrupted = false;
            //### Catch here is a hack
            try {
                session.vm.resume();
            } catch (VMDisconnectedException ee) {}
        }
        if (es instanceof ThreadDeathEventSet) {
            ThreadReference t = ((ThreadDeathEventSet)es).getThread();
            session.runtime.removeThreadInfo(t);
        }
    }

    //### This is a Hack, deal with it
    private class FirstListener implements JDIListener {

        public void accessWatchpoint(AccessWatchpointEventSet e) {
            session.runtime.validateThreadInfo();
            wantInterrupt = true;
        }

        public void classPrepare(ClassPrepareEventSet e)  {
            wantInterrupt = false;
            runtime.resolve(e.getReferenceType());
        }

        public void classUnload(ClassUnloadEventSet e)  {
            wantInterrupt = false;
        }

        public void exception(ExceptionEventSet e)  {
            wantInterrupt = true;
        }

        public void locationTrigger(LocationTriggerEventSet e)  {
            session.runtime.validateThreadInfo();
            wantInterrupt = true;
        }

        public void modificationWatchpoint(ModificationWatchpointEventSet e)  {
            session.runtime.validateThreadInfo();
            wantInterrupt = true;
        }

        public void threadDeath(ThreadDeathEventSet e)  {
            wantInterrupt = false;
        }

        public void threadStart(ThreadStartEventSet e)  {
            wantInterrupt = false;
        }

        public void vmDeath(VMDeathEventSet e)  {
            //### Should have some way to notify user
            //### that VM died before the session ended.
            wantInterrupt = false;
        }

        public void vmDisconnect(VMDisconnectEventSet e)  {
            //### Notify user?
            wantInterrupt = false;
            session.runtime.endSession();
        }

        public void vmStart(VMStartEventSet e)  {
            //### Do we need to do anything with it?
            wantInterrupt = false;
        }
    }
}
