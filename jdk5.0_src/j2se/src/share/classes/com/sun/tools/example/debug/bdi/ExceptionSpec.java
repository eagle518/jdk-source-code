/*
 * @(#)ExceptionSpec.java	1.8 03/12/19
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

import com.sun.jdi.ReferenceType;
import com.sun.jdi.request.*;

import java.util.ArrayList;
import java.util.List;
import java.util.Iterator;

public class ExceptionSpec extends EventRequestSpec {

    boolean notifyCaught;
    boolean notifyUncaught;

    ExceptionSpec(EventRequestSpecList specs, ReferenceTypeSpec refSpec,
                  boolean notifyCaught, boolean notifyUncaught)
    {
        super(specs, refSpec);
        this.notifyCaught = notifyCaught;
        this.notifyUncaught = notifyUncaught;
    }

    void notifySet(SpecListener listener, SpecEvent evt) {
        listener.exceptionInterceptSet(evt);
    }

    void notifyDeferred(SpecListener listener, SpecEvent evt) {
        listener.exceptionInterceptDeferred(evt);
    }

    void notifyResolved(SpecListener listener, SpecEvent evt) {
        listener.exceptionInterceptResolved(evt);
    }

    void notifyDeleted(SpecListener listener, SpecEvent evt) {
        listener.exceptionInterceptDeleted(evt);
    }

    void notifyError(SpecListener listener, SpecErrorEvent evt) {
        listener.exceptionInterceptError(evt);
    }

    /**
     * The 'refType' is known to match.
     */
    void resolve(ReferenceType refType) {
        setRequest(refType.virtualMachine().eventRequestManager()
                   .createExceptionRequest(refType, 
                                           notifyCaught, notifyUncaught));
    }

    public int hashCode() {
        return refSpec.hashCode();
    }

    public boolean equals(Object obj) {
        if (obj instanceof ExceptionSpec) {
            ExceptionSpec es = (ExceptionSpec)obj;

            return refSpec.equals(es.refSpec);
        } else {
            return false;
        }
    }

    public String toString() {
        StringBuffer buffer = new StringBuffer("exception catch ");
        buffer.append(refSpec.toString());
        return buffer.toString();
    }
}
