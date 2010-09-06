/*
 * @(#)EventRequestSpecList.java	1.20 03/12/19
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

package com.sun.tools.example.debug.tty;

import com.sun.jdi.*;
import com.sun.jdi.request.EventRequest;
import com.sun.jdi.event.ClassPrepareEvent;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Iterator;
import java.util.List;
import java.util.StringTokenizer;

class EventRequestSpecList {

    private static final int statusResolved = 1;
    private static final int statusUnresolved = 2;
    private static final int statusError = 3;
    
    // all specs
    private List eventRequestSpecs = Collections.synchronizedList(
                                                  new ArrayList());

    EventRequestSpecList() {
    }

    /** 
     * Resolve all deferred eventRequests waiting for 'refType'.
     * @return true if it completes successfully, false on error.
     */
    boolean resolve(ClassPrepareEvent event) {
        boolean failure = false;
        synchronized(eventRequestSpecs) {
            Iterator iter = eventRequestSpecs.iterator();
            while (iter.hasNext()) {
                EventRequestSpec spec = (EventRequestSpec)iter.next();
                if (!spec.isResolved()) {
                    try {
                        EventRequest eventRequest = spec.resolve(event);
                        if (eventRequest != null) {
                            MessageOutput.println("Set deferred", spec.toString());
                        }
                    } catch (Exception e) {
                        MessageOutput.println("Unable to set deferred",
                                              new Object [] {spec.toString(),
                                                             spec.errorMessageFor(e)});
                        failure = true;
                    }
                }
            }
        }
        return !failure;
    }

    void resolveAll() {
        Iterator iter = eventRequestSpecs.iterator();
        while (iter.hasNext()) {
            EventRequestSpec spec = (EventRequestSpec)iter.next();
            try {
                EventRequest eventRequest = spec.resolveEagerly();
                if (eventRequest != null) {
                    MessageOutput.println("Set deferred", spec.toString());
                } 
            } catch (Exception e) {
            }
        }
    }

    boolean addEagerlyResolve(EventRequestSpec spec) {
        try {
            eventRequestSpecs.add(spec);
            EventRequest eventRequest = spec.resolveEagerly();
            if (eventRequest != null) {
                MessageOutput.println("Set", spec.toString());
            } 
            return true;
        } catch (Exception exc) {
            MessageOutput.println("Unable to set",
                                  new Object [] {spec.toString(),
                                                 spec.errorMessageFor(exc)});
            return false;
        }
    }

    EventRequestSpec createBreakpoint(String classPattern, 
                                 int line) throws ClassNotFoundException {
        ReferenceTypeSpec refSpec = 
            new PatternReferenceTypeSpec(classPattern);
        return new BreakpointSpec(refSpec, line);
    }
        
    EventRequestSpec createBreakpoint(String classPattern, 
                                 String methodId, 
                                 List methodArgs) 
                                throws MalformedMemberNameException, 
                                       ClassNotFoundException {
        ReferenceTypeSpec refSpec = 
            new PatternReferenceTypeSpec(classPattern);
        return new BreakpointSpec(refSpec, methodId, methodArgs);
    }
        
    EventRequestSpec createExceptionCatch(String classPattern,
                                          boolean notifyCaught,
                                          boolean notifyUncaught)
                                            throws ClassNotFoundException {
        ReferenceTypeSpec refSpec = 
            new PatternReferenceTypeSpec(classPattern);
        return new ExceptionSpec(refSpec, notifyCaught, notifyUncaught);
    }
        
    EventRequestSpec createAccessWatchpoint(String classPattern, 
                                       String fieldId) 
                                      throws MalformedMemberNameException, 
                                             ClassNotFoundException {
        ReferenceTypeSpec refSpec = 
            new PatternReferenceTypeSpec(classPattern);
        return new AccessWatchpointSpec(refSpec, fieldId);
    }
        
    EventRequestSpec createModificationWatchpoint(String classPattern, 
                                       String fieldId) 
                                      throws MalformedMemberNameException, 
                                             ClassNotFoundException {
        ReferenceTypeSpec refSpec = 
            new PatternReferenceTypeSpec(classPattern);
        return new ModificationWatchpointSpec(refSpec, fieldId);
    }

    boolean delete(EventRequestSpec proto) {
        synchronized (eventRequestSpecs) {
            int inx = eventRequestSpecs.indexOf(proto);
            if (inx != -1) {
                EventRequestSpec spec = (EventRequestSpec)eventRequestSpecs.get(inx);
                spec.remove();
                eventRequestSpecs.remove(inx);
                return true;
            } else {
                return false;
            }
        }
    }

    List eventRequestSpecs() {
       // We need to make a copy to avoid synchronization problems
        synchronized (eventRequestSpecs) {
            return new ArrayList(eventRequestSpecs);
        }
    }
}
