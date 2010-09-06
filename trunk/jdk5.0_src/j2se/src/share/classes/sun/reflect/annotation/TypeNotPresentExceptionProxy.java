/*
 * @(#)TypeNotPresentExceptionProxy.java	1.2 04/04/20
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.reflect.annotation;
import java.lang.annotation.*;

/**
 * ExceptionProxy for TypeNotPresentException.
 *
 * @author  Josh Bloch
 * @since   1.5
 */
public class TypeNotPresentExceptionProxy extends ExceptionProxy {
    String typeName;
    Throwable cause;

    public TypeNotPresentExceptionProxy(String typeName, Throwable cause) {
        this.typeName = typeName;
        this.cause = cause;
    }

    protected RuntimeException generateException() {
        return new TypeNotPresentException(typeName, cause);
    }
}
