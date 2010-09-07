/*
 * @(#)UnixBrowserSupport.java	1.14 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws;
import java.net.URL;
import com.sun.deploy.config.Config;
import java.io.*;

/** Concrete implementation of the BrowserSupport class for Unix */
public class UnixBrowserSupport extends BrowserSupport  {

    /** Platform dependent */
    // mailcap info for NS6
    public String getNS6MailCapInfo() {
	return  "user_pref(\"helpers.private_mailcap_file\", \"" +
           System.getProperty("user.home") +
           "/.mailcap\");\nuser_pref(\"helpers.private_mime_types_file\", \"" +           System.getProperty("user.home") + "/.mime.types\");\n";
    }

    /**
     Gets an <code>OperaSupport</code> object for use on Unix/Linux platforms.

     @return An <code>OperaSupport</code> object for use on Unix/Linux
             platforms.
     */
    public OperaSupport getOperaSupport()
    {
        return (new UnixOperaSupport());
    }

    /** Platform dependent */
    public boolean isWebBrowserSupportedImpl() { return true; }


    /** This code is heavily inspired by the JavaWorld article:
     * http://www.javaworld.com/javaworld/javatips/jw-javatip66.html
     */
    public boolean showDocumentImpl(URL url) {
        return (Config.getInstance().showDocument(url.toString()));
    }
}
