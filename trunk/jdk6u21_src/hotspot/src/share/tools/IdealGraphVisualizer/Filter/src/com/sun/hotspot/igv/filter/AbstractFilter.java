/*
 * Copyright (c) 1998, 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */
package com.sun.hotspot.igv.filter;

import com.sun.hotspot.igv.data.ChangedEvent;
import com.sun.hotspot.igv.data.Properties;
import org.openide.cookies.OpenCookie;

/**
 *
 * @author Thomas Wuerthinger
 */
public abstract class AbstractFilter implements Filter {

    private ChangedEvent<Filter> changedEvent;
    private Properties properties;

    public AbstractFilter() {
        changedEvent = new ChangedEvent<Filter>(this);
        properties = new Properties();
    }

    public Properties getProperties() {
        return properties;
    }

    public OpenCookie getEditor() {
        return null;
    }

    public ChangedEvent<Filter> getChangedEvent() {
        return changedEvent;
    }

    protected void fireChangedEvent() {
        changedEvent.fire();
    }
}
