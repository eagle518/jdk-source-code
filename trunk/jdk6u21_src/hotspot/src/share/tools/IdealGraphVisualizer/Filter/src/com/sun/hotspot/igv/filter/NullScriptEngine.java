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

import com.sun.hotspot.igv.graph.Diagram;

/**
 *
 * @author Thomas Wuerthinger
 */
public class NullScriptEngine implements ScriptEngineAbstraction {

    public boolean initialize(String jsHelperText) {
        return true;
    }

    public void execute(Diagram d, String code) {
    }
}
