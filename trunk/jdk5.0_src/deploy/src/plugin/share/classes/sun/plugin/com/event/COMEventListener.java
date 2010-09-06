/*
 * @(#)COMEventListener.java	1.3 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.com.event;

import java.lang.reflect.Method;

public interface COMEventListener {
    public void notify(Object event, Method method) throws Throwable;
}


