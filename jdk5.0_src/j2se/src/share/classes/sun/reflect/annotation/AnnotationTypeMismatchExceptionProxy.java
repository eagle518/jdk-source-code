/*
 * @(#)AnnotationTypeMismatchExceptionProxy.java	1.3 04/04/30
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.reflect.annotation;
import java.lang.annotation.*;
import java.lang.reflect.Method;

/**
 * ExceptionProxy for AnnotationTypeMismatchException.
 *
 * @author  Josh Bloch
 * @since   1.5
 */
class AnnotationTypeMismatchExceptionProxy extends ExceptionProxy {
    private Method member;
    private String foundType;

    /**
     * It turns out to be convenient to construct these proxies in
     * two stages.  Since this is a private implementation class, we
     * permit ourselves this liberty even though it's normally a very
     * bad idea.
     */
    AnnotationTypeMismatchExceptionProxy(String foundType) {
        this.foundType = foundType;
    }

    AnnotationTypeMismatchExceptionProxy setMember(Method member) {
        this.member = member;
        return this;
    }

    protected RuntimeException generateException() {
        return new AnnotationTypeMismatchException(member, foundType);
    }
}
