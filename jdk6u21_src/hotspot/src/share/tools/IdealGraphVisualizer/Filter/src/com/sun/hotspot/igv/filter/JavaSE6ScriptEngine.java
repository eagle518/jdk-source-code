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
import javax.script.Bindings;
import javax.script.ScriptContext;
import javax.script.ScriptEngine;
import javax.script.ScriptEngineManager;
import javax.script.ScriptException;
import org.openide.util.Exceptions;

/**
 *
 * @author Thomas Wuerthinger
 */
public class JavaSE6ScriptEngine implements ScriptEngineAbstraction {

    private ScriptEngine engine;
    private Bindings bindings;

    public boolean initialize(String jsHelperText) {
        try {
            ScriptEngineManager sem = new ScriptEngineManager();
            ScriptEngine e = sem.getEngineByName("ECMAScript");
            engine = e;
            e.eval(jsHelperText);
            Bindings b = e.getContext().getBindings(ScriptContext.ENGINE_SCOPE);
            b.put("IO", System.out);
            bindings = b;
            return true;
        } catch (Exception e) {
            return false;
        }
    }

    public void execute(Diagram d, String code) {
        try {
            Bindings b = bindings;
            b.put("graph", d);
            engine.eval(code, b);
        } catch (ScriptException ex) {
            Exceptions.printStackTrace(ex);
        }
    }
}
