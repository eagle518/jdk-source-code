/*
 * @(#)RoundCompleteEventImpl.java	1.1 04/06/25
 *
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package com.sun.tools.apt.mirror.apt;

import com.sun.mirror.apt.AnnotationProcessorEnvironment;
import com.sun.mirror.apt.RoundCompleteEvent;
import com.sun.mirror.apt.RoundState;

public class RoundCompleteEventImpl extends RoundCompleteEvent {
    public RoundCompleteEventImpl(AnnotationProcessorEnvironment source,
				  RoundState rs) {
	super(source, rs);
    }
}
