/*
 * @(#)ExceptionEventSet.java	1.8 03/12/19
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

package com.sun.tools.example.debug.event;

import com.sun.jdi.*;
import com.sun.jdi.event.*;

public class ExceptionEventSet extends LocatableEventSet {
    
    ExceptionEventSet(EventSet jdiEventSet) {
        super(jdiEventSet);
    }

    /**
     * Gets the thrown exception object. The exception object is 
     * an instance of java.lang.Throwable or a subclass in the 
     * target VM.
     *
     * @return an {@link ObjectReference} which mirrors the thrown object in
     * the target VM. 
     */
    public ObjectReference getException() {
        return ((ExceptionEvent)oneEvent).exception();
    }
    
    /**
     * Gets the location where the exception will be caught. An exception
     * is considered to be caught if, at the point of the throw, the
     * current location is dynamically enclosed in a try statement that
     * handles the exception. (See the JVM specification for details).
     * If there is such a try statement, the catch location is the 
     * first code index of the appropriate catch clause.
     * <p>
     * If there are native methods in the call stack at the time of the
     * exception, there are important restrictions to note about the 
     * returned catch location. In such cases,
     * it is not possible to predict whether an exception will be handled
     * by some native method on the call stack.
     * Thus, it is possible that exceptions considered uncaught
     * here will, in fact, be handled by a native method and not cause 
     * termination of the target VM. Also, it cannot be assumed that the 
     * catch location returned here will ever be reached by the throwing 
     * thread. If there is 
     * a native frame between the current location and the catch location,
     * the exception might be handled and cleared in that native method 
     * instead.
     * 
     * @return the {@link Location} where the exception will be caught or null if
     * the exception is uncaught.
     */
    public Location getCatchLocation() {
        return ((ExceptionEvent)oneEvent).catchLocation();
    }

    public void notify(JDIListener listener) {
        listener.exception(this);
    }
}

