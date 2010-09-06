/*
 * @(#)Session.java	1.13 03/12/19
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

import com.sun.jdi.VirtualMachine;
import com.sun.jdi.VMDisconnectedException;
import com.sun.jdi.event.EventSet;

/**
 * Our repository of what we know about the state of one
 * running VM.
 */
class Session {

    final VirtualMachine vm;    
    final ExecutionManager runtime;
    final OutputListener diagnostics;

    boolean running = true;  // Set false by JDIEventSource
    boolean interrupted = false;  // Set false by JDIEventSource

    private JDIEventSource eventSourceThread = null;
    private int traceFlags;
    private boolean dead = false;

    public Session(VirtualMachine vm, ExecutionManager runtime,
		   OutputListener diagnostics) {
	this.vm = vm;
	this.runtime = runtime;
	this.diagnostics = diagnostics;
	this.traceFlags = VirtualMachine.TRACE_NONE;
    }

    /**
     * Determine if VM is interrupted, i.e, present and not running.
     */
    public boolean isInterrupted() {
	return interrupted;
    }
    
    public void setTraceMode(int traceFlags) {
        this.traceFlags = traceFlags;
        if (!dead) {
            vm.setDebugTraceMode(traceFlags);
        }
    }

    public boolean attach() {
        vm.setDebugTraceMode(traceFlags);
        diagnostics.putString("Connected to VM");
	eventSourceThread = new JDIEventSource(this);
	eventSourceThread.start();
        return true;
    }
    
    public void detach() {
        if (!dead) {
	    eventSourceThread.interrupt();
	    eventSourceThread = null;
	    //### The VM may already be disconnected
	    //### if the debuggee did a System.exit().
	    //### Exception handler here is a kludge,
	    //### Rather, there are many other places
	    //### where we need to handle this exception,
	    //### and initiate a detach due to an error
	    //### condition, e.g., connection failure.
	    try {
		vm.dispose();
	    } catch (VMDisconnectedException ee) {}
            dead = true;
	    diagnostics.putString("Disconnected from VM");
        }
    }
}
