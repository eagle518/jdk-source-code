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

/**
 *
 * @author Thomas Wuerthinger
 */
public interface FilterChainProvider {

    public FilterChain getFilterChain();

    public FilterChain getSequence();
}
