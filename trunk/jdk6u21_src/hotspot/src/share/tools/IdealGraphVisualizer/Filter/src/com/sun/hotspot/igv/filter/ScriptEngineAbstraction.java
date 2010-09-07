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

import com.sun.hotspot.igv.graph.Diagram;

/**
 *
 * @author Thomas Wuerthinger
 */
public interface ScriptEngineAbstraction {

    public boolean initialize(String jsHelperText);

    public void execute(Diagram d, String code);
}
