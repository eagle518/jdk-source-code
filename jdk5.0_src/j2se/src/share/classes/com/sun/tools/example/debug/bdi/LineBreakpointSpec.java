/*
 * @(#)LineBreakpointSpec.java	1.8 03/12/19
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

import java.util.ArrayList;
import java.util.List;
import java.util.Iterator;

public class LineBreakpointSpec extends BreakpointSpec {
    int lineNumber;

    LineBreakpointSpec(EventRequestSpecList specs, 
                       ReferenceTypeSpec refSpec, int lineNumber) {
        super(specs, refSpec);
        this.lineNumber = lineNumber;
    }

    /**
     * The 'refType' is known to match.
     */
    void resolve(ReferenceType refType) throws InvalidTypeException,
                                             LineNotFoundException {
        if (!(refType instanceof ClassType)) {
            throw new InvalidTypeException();
        }
        Location location = location((ClassType)refType);
        setRequest(refType.virtualMachine().eventRequestManager()
                   .createBreakpointRequest(location));
    }

    private Location location(ClassType clazz) throws 
                                            LineNotFoundException {
        Location location = null;
        try {
            List locs = clazz.locationsOfLine(lineNumber());
            if (locs.size() == 0) {
                throw new LineNotFoundException();
            }
            // TODO handle multiple locations
            location = (Location)locs.get(0);
            if (location.method() == null) {
                throw new LineNotFoundException();
            } 
        } catch (AbsentInformationException e) {
            /*
             * TO DO: throw something more specific, or allow
             * AbsentInfo exception to pass through. 
             */
            throw new LineNotFoundException();
        }
        return location;
    }

    public int lineNumber() {
        return lineNumber;
    }

    public int hashCode() {
        return refSpec.hashCode() + lineNumber;
    }

    public boolean equals(Object obj) {
        if (obj instanceof LineBreakpointSpec) {
            LineBreakpointSpec breakpoint = (LineBreakpointSpec)obj;

            return refSpec.equals(breakpoint.refSpec) &&
                   (lineNumber == breakpoint.lineNumber);
        } else {
            return false;
        }
    }

    public String errorMessageFor(Exception e) { 
        if (e instanceof LineNotFoundException) {
            return ("No code at line " + lineNumber() + " in " + refSpec);
        } else if (e instanceof InvalidTypeException) {
            return ("Breakpoints can be located only in classes. " + 
                        refSpec + " is an interface or array");
        } else {
            return super.errorMessageFor( e);
        } 
    }

    public String toString() {
        StringBuffer buffer = new StringBuffer("breakpoint ");
        buffer.append(refSpec.toString());
        buffer.append(':');
        buffer.append(lineNumber);  
        buffer.append(" (");
        buffer.append(getStatusString());
        buffer.append(')');
        return buffer.toString();   
    }
}
