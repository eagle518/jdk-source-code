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
package com.sun.hotspot.igv.filterwindow;

import com.sun.hotspot.igv.filter.FilterChain;
import com.sun.hotspot.igv.filter.FilterChainProvider;

/**
 *
 * @author Thomas Wuerthinger
 */
public class FilterChainProviderImplementation implements FilterChainProvider {

    public FilterChain getFilterChain() {
        return FilterTopComponent.findInstance().getFilterChain();
    }

    public FilterChain getSequence() {
        return FilterTopComponent.findInstance().getSequence();
    }
}
