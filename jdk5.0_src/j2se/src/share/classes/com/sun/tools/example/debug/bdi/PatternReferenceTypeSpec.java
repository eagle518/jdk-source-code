/*
 * @(#)PatternReferenceTypeSpec.java	1.9 03/12/19
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
import java.util.StringTokenizer;

class PatternReferenceTypeSpec implements ReferenceTypeSpec {
    final boolean isWild;
    final String classId;

    PatternReferenceTypeSpec(String classId) 
//                             throws ClassNotFoundException 
    {
//        checkClassName(classId);
        isWild = classId.startsWith("*.");
        if (isWild) {
            this.classId = classId.substring(1);
        } else {
            this.classId = classId;
        }
    }

    /**
     * Does the specified ReferenceType match this spec.
     */
    public boolean matches(ReferenceType refType) {
        if (isWild) {
            return refType.name().endsWith(classId);
        } else {
            return refType.name().equals(classId);
        }
    }

    public int hashCode() {
        return classId.hashCode();
    }

    public boolean equals(Object obj) {
        if (obj instanceof PatternReferenceTypeSpec) {
            PatternReferenceTypeSpec spec = (PatternReferenceTypeSpec)obj;

            return classId.equals(spec.classId) && (isWild == spec.isWild);
        } else {
            return false;
        }
    }

    private void checkClassName(String className) throws ClassNotFoundException {
        // Do stricter checking of class name validity on deferred
        //  because if the name is invalid, it will
        // never match a future loaded class, and we'll be silent
        // about it.
        StringTokenizer tokenizer = new StringTokenizer(className, ".");
        boolean first = true;
        while (tokenizer.hasMoreTokens()) {
            String token = tokenizer.nextToken();
            // Each dot-separated piece must be a valid identifier
            // and the first token can also be "*". (Note that 
            // numeric class ids are not permitted. They must
            // match a loaded class.)
            if (!Utils.isJavaIdentifier(token) && !(first && token.equals("*"))) {
                throw new ClassNotFoundException();
            }
            first = false;
        }
    }

    public String toString() {
        return isWild? "*" + classId : classId;
    }
}


