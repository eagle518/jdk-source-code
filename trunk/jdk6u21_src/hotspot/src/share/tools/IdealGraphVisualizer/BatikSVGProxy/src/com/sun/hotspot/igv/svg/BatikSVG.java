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
package com.sun.hotspot.igv.svg;

import java.awt.Graphics2D;
import java.io.Writer;
import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import org.w3c.dom.DOMImplementation;

/**
 *
 * @author Thomas Wuerthinger
 */
public class BatikSVG {

    private static Constructor SVGGraphics2DConstructor;
    private static Method Method_stream;
    private static Method Method_createDefault;
    private static Method Method_getDOMImplementation;
    private static Method Method_setEmbeddedFontsOn;

    public static Graphics2D createGraphicsObject() {
        try {
            if (SVGGraphics2DConstructor == null) {
                ClassLoader cl = BatikSVG.class.getClassLoader();
                Class Class_GenericDOMImplementation = cl.loadClass("org.apache.batik.dom.GenericDOMImplementation");
                Class Class_SVGGeneratorContext = cl.loadClass("org.apache.batik.svggen.SVGGeneratorContext");
                Class Class_SVGGraphics2D = cl.loadClass("org.apache.batik.svggen.SVGGraphics2D");
                Method_getDOMImplementation = Class_GenericDOMImplementation.getDeclaredMethod("getDOMImplementation", new Class[0]);
                Method_createDefault = Class_SVGGeneratorContext.getDeclaredMethod("createDefault", new Class[]{org.w3c.dom.Document.class});
                Method_setEmbeddedFontsOn = Class_SVGGeneratorContext.getDeclaredMethod("setEmbeddedFontsOn", new Class[]{boolean.class});
                Method_stream = Class_SVGGraphics2D.getDeclaredMethod("stream", Writer.class, boolean.class);
                SVGGraphics2DConstructor = Class_SVGGraphics2D.getConstructor(Class_SVGGeneratorContext, boolean.class);
            }
            DOMImplementation dom = (DOMImplementation) Method_getDOMImplementation.invoke(null);
            org.w3c.dom.Document document = dom.createDocument("http://www.w3.org/2000/svg", "svg", null);
            Object ctx = Method_createDefault.invoke(null, document);
            Method_setEmbeddedFontsOn.invoke(ctx, true);
            Graphics2D svgGenerator = (Graphics2D) SVGGraphics2DConstructor.newInstance(ctx, true);
            return svgGenerator;
        } catch (ClassNotFoundException e) {
            return null;
        } catch (NoSuchMethodException e) {
            return null;
        } catch (IllegalAccessException e) {
            return null;
        } catch (InvocationTargetException e) {
            return null;
        } catch (InstantiationException e) {
            return null;
        }
    }

    public static void printToStream(Graphics2D svgGenerator, Writer stream, boolean useCSS) {
        try {
            Method_stream.invoke(svgGenerator, stream, useCSS);
        } catch (IllegalAccessException e) {
            assert false;
        } catch (InvocationTargetException e) {
            assert false;
        }
    }
}
