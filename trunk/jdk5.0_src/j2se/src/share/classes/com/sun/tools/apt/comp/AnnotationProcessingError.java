/**
 * @(#)AnnotationProcessingError.java	1.1 04/03/06
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.apt.comp;

public class AnnotationProcessingError extends Error {
    AnnotationProcessingError(Throwable cause) {
	super(cause);
    }
}
