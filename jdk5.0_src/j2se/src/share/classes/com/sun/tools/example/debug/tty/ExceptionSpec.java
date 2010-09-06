/*
 * @(#)ExceptionSpec.java	1.16 03/12/19
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

import com.sun.jdi.ReferenceType;
import com.sun.jdi.request.*;

import java.util.ArrayList;
import java.util.List;
import java.util.Iterator;

class ExceptionSpec extends EventRequestSpec {
    private boolean notifyCaught;
    private boolean notifyUncaught;

    private ExceptionSpec(ReferenceTypeSpec refSpec) {
        this(refSpec, true, true);
    }

    ExceptionSpec(ReferenceTypeSpec refSpec,
                  boolean notifyCaught,
                  boolean notifyUncaught) {
        super(refSpec);
        this.notifyCaught = notifyCaught;
        this.notifyUncaught = notifyUncaught;
    }

    /**
     * The 'refType' is known to match, return the EventRequest.
     */
    EventRequest resolveEventRequest(ReferenceType refType) {
        EventRequestManager em = refType.virtualMachine().eventRequestManager();
        ExceptionRequest excReq = em.createExceptionRequest(refType, 
                                                            notifyCaught,
                                                            notifyUncaught);
        excReq.enable();
        return excReq;
    }

    public boolean notifyCaught() {
        return notifyCaught;
    }

    public boolean notifyUncaught() {
        return notifyUncaught;
    }

    public int hashCode() {
        //Reference: Effective Java[tm] (Bloch, 2001), Item 8
        int result = 17;
        result = (37 * result) + (notifyCaught() ? 0: 1);
        result = (37 * result) + (notifyUncaught() ? 0: 1);
        result = (37 * result) + refSpec.hashCode();
        return result;
    }

    public boolean equals(Object obj) {
        if (obj instanceof ExceptionSpec) {
            ExceptionSpec es = (ExceptionSpec)obj;

            if (refSpec.equals(es.refSpec) &&
                (this.notifyCaught() == es.notifyCaught()) &&
                (this.notifyUncaught() == es.notifyUncaught())) {
                return true;
            }
        }
        return false;
    }

    public String toString() {
        String s;
        if (notifyCaught && !notifyUncaught) {
            s = MessageOutput.format("exceptionSpec caught",
                                     refSpec.toString());
        } else if (notifyUncaught && !notifyCaught) {
            s = MessageOutput.format("exceptionSpec uncaught",
                                     refSpec.toString());
        } else {
            s = MessageOutput.format("exceptionSpec all",
                                     refSpec.toString());
        }
        return s;
    }
}
