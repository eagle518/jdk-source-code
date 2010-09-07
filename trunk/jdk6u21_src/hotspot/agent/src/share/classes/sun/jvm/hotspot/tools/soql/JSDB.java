/*
 * Copyright (c) 2004, Oracle and/or its affiliates. All rights reserved.
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

package sun.jvm.hotspot.tools.soql;

import sun.jvm.hotspot.tools.*;
import sun.jvm.hotspot.utilities.*;
import sun.jvm.hotspot.utilities.soql.*;

/** This is command line JavaScript debugger console */
public class JSDB extends Tool {
    public static void main(String[] args) {
        JSDB jsdb = new JSDB();
        jsdb.start(args);
        jsdb.stop();
    }

    public void run() {
        JSJavaScriptEngine engine = new JSJavaScriptEngine() {
                private ObjectReader objReader = new ObjectReader();
                private JSJavaFactory factory = new JSJavaFactoryImpl();

                public ObjectReader getObjectReader() {
                    return objReader;
                }

                public JSJavaFactory getJSJavaFactory() {
                    return factory;
                }
            };
        engine.startConsole();
    }
}
