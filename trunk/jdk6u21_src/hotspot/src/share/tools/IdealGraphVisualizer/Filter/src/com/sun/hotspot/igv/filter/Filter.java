/*
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
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
 */
package com.sun.hotspot.igv.filter;

import com.sun.hotspot.igv.data.ChangedEvent;
import com.sun.hotspot.igv.data.ChangedEventProvider;
import com.sun.hotspot.igv.data.Properties;
import com.sun.hotspot.igv.graph.Diagram;
import org.openide.cookies.OpenCookie;

/**
 *
 * @author Thomas Wuerthinger
 */
public interface Filter extends Properties.Provider, ChangedEventProvider<Filter> {

    public String getName();

    public void apply(Diagram d);

    OpenCookie getEditor();

    ChangedEvent<Filter> getChangedEvent();
}
