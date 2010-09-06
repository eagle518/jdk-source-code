/*
 * @(#)WatchpointEventSet.java	1.8 03/12/19
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

public abstract class WatchpointEventSet extends LocatableEventSet {
    
    WatchpointEventSet(EventSet jdiEventSet) {
        super(jdiEventSet);
    }

    /**
     * Returns the field that is about to be accessed/modified. 
     *
     * @return a {@link Field} which mirrors the field
     * in the target VM.
     */
    public Field getField() {
        return ((WatchpointEvent)oneEvent).field();
    }

    /**
     * Returns the object whose field is about to be accessed/modified. 
     * Return null is the access is to a static field.
     *
     * @return a {@link ObjectReference} which mirrors the event's 
     * object in the target VM.
     */
    public ObjectReference getObject() {
        return ((WatchpointEvent)oneEvent).object();
    }

    /**
     * Current value of the field.
     */
    public Value getValueCurrent() {
        return ((WatchpointEvent)oneEvent).valueCurrent();
    }
}

