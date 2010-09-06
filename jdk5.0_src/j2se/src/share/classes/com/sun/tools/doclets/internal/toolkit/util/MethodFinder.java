/*
 * @(#)MethodFinder.java	1.12 04/07/27
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.doclets.internal.toolkit.util;

import com.sun.javadoc.*;

/**
 * This class is useful for searching a method which has documentation
 * comment and documentation tags. The method is searched in all the
 * superclasses and interfaces(subsequently super-interfaces also)
 * recursively.
 * 
 * This code is not part of an API.
 * It is implementation that is subject to change.
 * Do not use it as an API.
 */
public abstract class MethodFinder {

    abstract boolean isCorrectMethod(MethodDoc method);
    
    public MethodDoc search(ClassDoc cd, MethodDoc method) {
        MethodDoc meth = searchInterfaces(cd, method);
        if (meth != null) {
            return meth;
        }
        ClassDoc icd = cd.superclass();
        if (icd != null) {
            meth = Util.findMethod(icd, method);
            if (meth != null) {
            if (isCorrectMethod(meth)) {
                    return meth;
                }
            }
            return search(icd, method);
        }
        return null;
    }

    public MethodDoc searchInterfaces(ClassDoc cd, MethodDoc method) {
        MethodDoc[] implementedMethods = (new ImplementedMethods(method, null)).build();
        for (int i = 0; i < implementedMethods.length; i++) {
            if (isCorrectMethod(implementedMethods[i])) {
                return implementedMethods[i];
            }
        }
        return null;
    }
}
