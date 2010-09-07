/*
 * @(#)AppletTagParser.java	1.3 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.applet.viewer.util;

import java.awt.BorderLayout;
import java.awt.Canvas;
import java.awt.Frame;
import java.awt.event.*;
import java.io.*;
import java.net.*;
import java.util.*;
import javax.swing.text.*;
import javax.swing.text.html.*;
import javax.swing.text.html.parser.*;

import sun.awt.*;
import sun.plugin2.applet.*;
import sun.plugin2.applet.context.*;
import com.sun.deploy.util.*;

public class AppletTagParser extends HTMLEditorKit.ParserCallback {
    private Map/*<String,String>*/ parameters = new HashMap();
    private boolean foundApplet;

    private static String toStringHelper(Object obj) {
        if (obj == null)
            return null;
        return obj.toString();
    }

    public void handleStartTag(HTML.Tag t, MutableAttributeSet attributes, int pos) {
        if (t == HTML.Tag.APPLET && !foundApplet) {
            foundApplet = true;
            //                System.out.println("Applet found");
            //                    System.out.println("Applet attributes: ");
            extractAttributes(attributes, parameters);
        }
    }

    public void handleSimpleTag(HTML.Tag t, MutableAttributeSet attributes, int pos) {
        if (t == HTML.Tag.PARAM) {
            Map/*<String,String>*/ params = new HashMap();
            extractAttributes(attributes, params);
            String name  = (String) params.get("name");
            String value = (String) params.get("value");
            if (name != null && value != null) {
                parameters.put(name, value);
            }
            //                System.out.println("Param found: " + a);
        }
    }

    private void extractAttributes(MutableAttributeSet attributes, Map/*<String, String>*/ parameters) {
        for (Enumeration e = attributes.getAttributeNames(); e.hasMoreElements(); ) {
            Object cur = e.nextElement();
            String name = toStringHelper(cur);
            String val  = toStringHelper(attributes.getAttribute(cur));
            if (name != null && val != null) {
                parameters.put(name, val);
                //                        System.out.println("  " + name + "=" + val);
            }
        }
    }

    /*
      public void handleError(String errorMsg, int pos) {
      System.out.println("Error \"" + errorMsg + "\" at position " + pos);
      }
    */

    public boolean foundApplet() {
        return foundApplet;
    }

    public Map/*<String, String>*/ getParameters() {
        return parameters;
    }
}

