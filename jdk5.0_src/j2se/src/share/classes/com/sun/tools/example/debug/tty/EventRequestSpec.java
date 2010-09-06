/*
 * @(#)EventRequestSpec.java	1.19 03/12/19
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
import com.sun.jdi.request.ExceptionRequest;
import com.sun.jdi.request.ClassPrepareRequest;
import com.sun.jdi.event.ClassPrepareEvent;
import java.util.List;
import java.util.ArrayList;
import java.util.Iterator;

abstract class EventRequestSpec {

    final ReferenceTypeSpec refSpec;

    int suspendPolicy = EventRequest.SUSPEND_ALL;

    EventRequest resolved = null;
    ClassPrepareRequest prepareRequest = null;

    EventRequestSpec(ReferenceTypeSpec refSpec) {
        this.refSpec = refSpec;
    }

    /**
     * The 'refType' is known to match, return the EventRequest.
     */
    abstract EventRequest resolveEventRequest(ReferenceType refType) 
                                           throws Exception;

    /**
     * @return If this EventRequestSpec matches the 'refType'
     * return the cooresponding EventRequest.  Otherwise
     * return null.
     */
    synchronized EventRequest resolve(ClassPrepareEvent event) throws Exception {
        if ((resolved == null) && 
            (prepareRequest != null) &&
            prepareRequest.equals(event.request())) {

            resolved = resolveEventRequest(event.referenceType());
            prepareRequest.disable();
            Env.vm().eventRequestManager().deleteEventRequest(prepareRequest);
            prepareRequest = null;

            if (refSpec instanceof PatternReferenceTypeSpec) {
                PatternReferenceTypeSpec prs = (PatternReferenceTypeSpec)refSpec;
                if (! prs.isUnique()){
                    /* 
                     * Class pattern event requests are never
                     * considered "resolved", since future class loads
                     * might also match.
                     * Create and enable a new ClassPrepareRequest to
                     * keep trying to resolve.
                     */
                    resolved = null;
                    prepareRequest = refSpec.createPrepareRequest();
                    prepareRequest.enable();
                }
            }
        }
        return resolved;
    }   

    synchronized void remove() {
        if (isResolved()) {
            Env.vm().eventRequestManager().deleteEventRequest(resolved());
        }
        if (refSpec instanceof PatternReferenceTypeSpec) {
            PatternReferenceTypeSpec prs = (PatternReferenceTypeSpec)refSpec;
            if (! prs.isUnique()){
                /*
                 * This is a class pattern.  Track down and delete
                 * all EventRequests matching this spec.
                 * Note: Class patterns apply only to ExceptionRequests,
                 * so that is all we need to examine.
                 */
                ArrayList deleteList = new ArrayList();
                Iterator iter =
                    Env.vm().eventRequestManager().exceptionRequests().iterator();
                while (iter.hasNext()) {
                    ExceptionRequest er = (ExceptionRequest)iter.next();
                    if (prs.matches(er.exception())) {
                        deleteList.add (er);
                    }
                }
                Env.vm().eventRequestManager().deleteEventRequests(deleteList);
            }
        }
    }
    
    private EventRequest resolveAgainstPreparedClasses() throws Exception {
        Iterator iter = Env.vm().allClasses().iterator();
        while (iter.hasNext()) {
            ReferenceType refType = (ReferenceType)iter.next();
            if (refType.isPrepared() && refSpec.matches(refType)) {
                resolved = resolveEventRequest(refType);
            }
        }
        return resolved;
    }

    synchronized EventRequest resolveEagerly() throws Exception {
        try {
            if (resolved == null) {
                /*
                 * Not resolved.  Schedule a prepare request so we
                 * can resolve later.
                 */
                prepareRequest = refSpec.createPrepareRequest();
                prepareRequest.enable();
    
                // Try to resolve in case the class is already loaded.
                resolveAgainstPreparedClasses();
                if (resolved != null) {
                    prepareRequest.disable();
                    Env.vm().eventRequestManager().deleteEventRequest(prepareRequest);
                    prepareRequest = null;
                }
            }
            if (refSpec instanceof PatternReferenceTypeSpec) {
                PatternReferenceTypeSpec prs = (PatternReferenceTypeSpec)refSpec;
                if (! prs.isUnique()){
                    /* 
                     * Class pattern event requests are never
                     * considered "resolved", since future class loads
                     * might also match.  Create a new
                     * ClassPrepareRequest if necessary and keep
                     * trying to resolve.
                     */
                    resolved = null;
                    if (prepareRequest == null) {
                        prepareRequest = refSpec.createPrepareRequest();
                        prepareRequest.enable();
                    }
                }            
            }
        } catch (VMNotConnectedException e) {
            // Do nothing. Another resolve will be attempted when the 
            // VM is started.
        }
        return resolved;
    }

    /**
     * @return the eventRequest this spec has been resolved to,
     * null if so far unresolved.
     */
    EventRequest resolved() {
        return resolved;
    }

    /**
     * @return true if this spec has been resolved.
     */
    boolean isResolved() {
        return resolved != null;
    }

    protected boolean isJavaIdentifier(String s) {
        if (s.length() == 0) {                              
            return false;
        }

        int cp = s.codePointAt(0);
        if (! Character.isJavaIdentifierStart(cp)) {
            return false;
        }

        for (int i = Character.charCount(cp); i < s.length(); i += Character.charCount(cp)) {
            cp = s.codePointAt(i);
            if (! Character.isJavaIdentifierPart(cp)) {
                return false;
            }
        }

        return true;
    }

    String errorMessageFor(Exception e) { 
        if (e instanceof IllegalArgumentException) {
            return (MessageOutput.format("Invalid command syntax"));
        } else if (e instanceof RuntimeException) {
            // A runtime exception that we were not expecting
            throw (RuntimeException)e;
        } else {
            return (MessageOutput.format("Internal error; unable to set",
                                         this.refSpec.toString()));
        } 
    }
}


