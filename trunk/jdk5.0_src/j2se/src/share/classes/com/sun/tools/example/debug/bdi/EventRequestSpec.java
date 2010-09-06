/*
 * @(#)EventRequestSpec.java	1.9 03/12/19
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

import java.util.*;

import com.sun.jdi.*;
import com.sun.jdi.request.EventRequest;

abstract public class EventRequestSpec {

    static final int STATUS_UNRESOLVED = 1;
    static final int STATUS_RESOLVED = 2;
    static final int STATUS_ERROR = 3;

    static final Object specPropertyKey = "spec";

    final EventRequestSpecList specs;
    final ReferenceTypeSpec refSpec;
    EventRequest request = null;

    int status = STATUS_UNRESOLVED;

    EventRequestSpec(EventRequestSpecList specs, ReferenceTypeSpec refSpec) {
        this.specs = specs;
        this.refSpec = refSpec;
    }

    void setRequest(EventRequest request) {
        this.request = request;
        request.putProperty(specPropertyKey, this);
        request.enable();
    }

    /**
     * The 'refType' is known to match.
     */
    abstract void resolve(ReferenceType refType) throws Exception;

    abstract void notifySet(SpecListener listener, SpecEvent evt);
    abstract void notifyDeferred(SpecListener listener, SpecEvent evt);
    abstract void notifyResolved(SpecListener listener, SpecEvent evt);
    abstract void notifyDeleted(SpecListener listener, SpecEvent evt);
    abstract void notifyError(SpecListener listener, SpecErrorEvent evt);

    /**
     * The 'refType' is known to match.
     */
    void resolveNotify(ReferenceType refType) {
        try {
            resolve(refType);
            status = STATUS_RESOLVED;
            specs.notifyResolved(this);
        } catch(Exception exc) {
            status = STATUS_ERROR;
            specs.notifyError(this, exc);
        }
    }

    /**
     * See if 'refType' matches and resolve.
     */
    void attemptResolve(ReferenceType refType) {
        if (!isResolved() && refSpec.matches(refType)) {
            resolveNotify(refType);
        }
    }

    void attemptImmediateResolve(VirtualMachine vm) {
        // try to resolve immediately
        Iterator iter = vm.allClasses().iterator();
        while (iter.hasNext()) {
            ReferenceType refType = (ReferenceType)iter.next();
            if (refSpec.matches(refType)) {
                try {
                    resolve(refType);
                    status = STATUS_RESOLVED;
                    specs.notifySet(this);
                } catch(Exception exc) {
                    status = STATUS_ERROR;
                    specs.notifyError(this, exc);
                }
                return;
            }
        }
        specs.notifyDeferred(this);
    }

    public EventRequest getEventRequest() {  
        return request;
    }

    /**
     * @return true if this spec has been resolved.
     */
    public boolean isResolved() {
        return status == STATUS_RESOLVED;
    }

    /**
     * @return true if this spec has not yet been resolved.
     */
    public boolean isUnresolved() {
        return status == STATUS_UNRESOLVED;
    }

    /**
     * @return true if this spec is unresolvable due to error.
     */
    public boolean isErroneous() {
        return status == STATUS_ERROR;
    }

    public String getStatusString() {
        switch (status) {
            case STATUS_RESOLVED: 
                return "resolved";
            case STATUS_UNRESOLVED: 
                return "deferred";
            case STATUS_ERROR: 
                return "erroneous";
        }
        return "unknown";
    }

    boolean isJavaIdentifier(String s) {
        return Utils.isJavaIdentifier(s);
    }

    public String errorMessageFor(Exception e) { 
        if (e instanceof IllegalArgumentException) {
            return ("Invalid command syntax");
        } else if (e instanceof RuntimeException) {
            // A runtime exception that we were not expecting
            throw (RuntimeException)e;
        } else {
            return ("Internal error; unable to set" + this);
        } 
    }
}


