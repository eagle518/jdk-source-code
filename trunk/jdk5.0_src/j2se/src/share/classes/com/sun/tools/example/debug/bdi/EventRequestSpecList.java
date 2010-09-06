/*
 * @(#)EventRequestSpecList.java	1.9 03/12/19
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
import com.sun.jdi.request.*;

import java.util.*;

class EventRequestSpecList {
    
    // all specs
    private List eventRequestSpecs = Collections.synchronizedList(
                                                  new ArrayList());

    final ExecutionManager runtime;

    EventRequestSpecList(ExecutionManager runtime) {
	this.runtime = runtime;
    }

    /** 
     * Resolve all deferred eventRequests waiting for 'refType'.
     */
    void resolve(ReferenceType refType) {
        synchronized(eventRequestSpecs) {
            Iterator iter = eventRequestSpecs.iterator();
            while (iter.hasNext()) {
                ((EventRequestSpec)iter.next()).attemptResolve(refType);
             }
        }
    }

    void install(EventRequestSpec ers, VirtualMachine vm) {
        synchronized (eventRequestSpecs) {
            eventRequestSpecs.add(ers);
        }
        if (vm != null) {
            ers.attemptImmediateResolve(vm);
        }
    }
        
    BreakpointSpec 
    createClassLineBreakpoint(String classPattern, int line) {
        ReferenceTypeSpec refSpec = 
            new PatternReferenceTypeSpec(classPattern);
        return new LineBreakpointSpec(this, refSpec, line);
    }
        
    BreakpointSpec 
    createSourceLineBreakpoint(String sourceName, int line) {
        ReferenceTypeSpec refSpec = 
            new SourceNameReferenceTypeSpec(sourceName, line);
        return new LineBreakpointSpec(this, refSpec, line);
    }
        
    BreakpointSpec 
    createMethodBreakpoint(String classPattern, 
                           String methodId, List methodArgs) {
        ReferenceTypeSpec refSpec = 
            new PatternReferenceTypeSpec(classPattern);
        return new MethodBreakpointSpec(this, refSpec, 
                                        methodId, methodArgs);
    }
        
    ExceptionSpec 
    createExceptionIntercept(String classPattern,
                             boolean notifyCaught,
                             boolean notifyUncaught) {
        ReferenceTypeSpec refSpec = 
            new PatternReferenceTypeSpec(classPattern);
        return new ExceptionSpec(this, refSpec,
                                 notifyCaught, notifyUncaught);
    }
        
    AccessWatchpointSpec 
    createAccessWatchpoint(String classPattern, String fieldId) {
        ReferenceTypeSpec refSpec = 
            new PatternReferenceTypeSpec(classPattern);
        return new AccessWatchpointSpec(this, refSpec, fieldId);
    }
        
    ModificationWatchpointSpec 
    createModificationWatchpoint(String classPattern, String fieldId) {
        ReferenceTypeSpec refSpec = 
            new PatternReferenceTypeSpec(classPattern);
        return new ModificationWatchpointSpec(this, refSpec, fieldId);
    }

    void delete(EventRequestSpec ers) {
        EventRequest request = ers.getEventRequest();
        synchronized (eventRequestSpecs) {
            eventRequestSpecs.remove(ers);
        }
        if (request != null) {
            request.virtualMachine().eventRequestManager()
                .deleteEventRequest(request);
        }
        notifyDeleted(ers);
        //### notify delete - here?
    }

    List eventRequestSpecs() {
        // We need to make a copy to avoid synchronization problems
        synchronized (eventRequestSpecs) {
            return new ArrayList(eventRequestSpecs);
        }
    }

    // --------  notify routines --------------------

    private Vector specListeners() {
        return (Vector)runtime.specListeners.clone();
    }

    void notifySet(EventRequestSpec spec) {
	Vector l = specListeners();
	SpecEvent evt = new SpecEvent(spec);
	for (int i = 0; i < l.size(); i++) {
	    spec.notifySet((SpecListener)l.elementAt(i), evt);
	}
    }

    void notifyDeferred(EventRequestSpec spec) {
	Vector l = specListeners();
	SpecEvent evt = new SpecEvent(spec);
	for (int i = 0; i < l.size(); i++) {
	    spec.notifyDeferred((SpecListener)l.elementAt(i), evt);
	}
    }

    void notifyDeleted(EventRequestSpec spec) {
	Vector l = specListeners();
	SpecEvent evt = new SpecEvent(spec);
	for (int i = 0; i < l.size(); i++) {
	    spec.notifyDeleted((SpecListener)l.elementAt(i), evt);
	}
    }

    void notifyResolved(EventRequestSpec spec) {
	Vector l = specListeners();
	SpecEvent evt = new SpecEvent(spec);
	for (int i = 0; i < l.size(); i++) {
	    spec.notifyResolved((SpecListener)l.elementAt(i), evt);
	}
    }

    void notifyError(EventRequestSpec spec, Exception exc) {
	Vector l = specListeners();
	SpecErrorEvent evt = new SpecErrorEvent(spec, exc);
	for (int i = 0; i < l.size(); i++) {
	    spec.notifyError((SpecListener)l.elementAt(i), evt);
	}
    }
}



