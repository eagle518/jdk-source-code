/*
 * @(#)COMEventListener.java	1.5 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.com.event;

import java.lang.reflect.Method;

public interface COMEventListener {
    public void notify(Object event, Method method) throws Throwable;
}


