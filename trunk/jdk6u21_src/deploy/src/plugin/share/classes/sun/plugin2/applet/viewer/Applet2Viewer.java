/*
 * @(#)Applet2Viewer.java	1.15 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.applet.viewer;

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

import com.sun.deploy.util.JVMParameters;

import sun.plugin2.applet.viewer.util.*;

public class Applet2Viewer {

    public static void main(String[] args) throws Exception {
        if (args.length != 1) {
            System.out.println("Usage: Applet2Viewer [url to HTML page containing <applet> tag]");
            System.out.println("Views the first applet on the specified HTML page.");
            System.exit(1);
        }

        URL url = new URL(args[0]);
        InputStream stream = url.openStream();
        if (stream == null) {
            throw new RuntimeException("Error opening URL " + url);
        }
        AppletTagParser finder = new AppletTagParser();
        new ParserDelegator().parse(new InputStreamReader(stream), finder, true);
        stream.close();

        if (!finder.foundApplet()) {
            System.out.println("No applet found on web page");
            System.exit(1);
        }

        Map params = finder.getParameters();
        int width  = 512;
        int height = 512;
        try {
            width = Integer.parseInt((String) params.get("width"));
        } catch (Exception e) {
        }
        try {
            height = Integer.parseInt((String) params.get("height"));
        } catch (Exception e) {
        }

        final int fw = width;
        final int fh = height;

        // make this JVM's parameter accessible deployment wide
        JVMParameters jvmParams = new JVMParameters();
        jvmParams.parseBootClassPath(JVMParameters.getPlugInDependentJars());
        jvmParams.setDefault(true);
        JVMParameters.setRunningJVMParameters(jvmParams);

        System.out.println("Initializing Applet2Environment");
        try {
            sun.net.ProgressMonitor.setDefault(new sun.plugin.util.ProgressMonitor());
        } catch (Throwable e) {
            // This is expected on JDK 1.4.2; don't display any dialog box
        }
        Applet2Environment.initialize(null, true, false,
                                      new Plugin2ConsoleController(null, null),
                                      null, null);
        final Applet2Manager manager = new Applet2Manager(null, null, false); // not relaunched
        manager.setAppletExecutionContext(new NoopExecutionContext(params,
                                                                   url.toExternalForm()));
        System.out.println("Starting applet with parameters:");
        for (Iterator iter = params.keySet().iterator(); iter.hasNext(); ) {
            String key = (String) iter.next();
            String val = (String) params.get(key);
            System.out.println("  " + key + " = " + val);
        }
        AppContext appContext = manager.getAppletAppContext();
        DeployAWTUtil.invokeLater(appContext, new Runnable() {
                public void run() {
                    try {
                        manager.initialize();
                    } catch (Exception e) {
                        e.printStackTrace();
                        System.err.println("Error while initializing manager: "+e+", bail out");
                        return;
                    }

                    Frame f = new Frame("AppletViewer");
                    f.addWindowListener(new WindowAdapter() {
                            public void windowClosing(WindowEvent e) {
                                System.exit(0);
                            }
                        });
                    f.setLayout(new BorderLayout());
                    manager.setAppletParentContainer(f);
                    // This is a hack to get the Frame to size itself
                    // appropriately for the component it contains
                    Canvas c = new Canvas();
                    c.setSize(fw, fh);
                    f.add(c, BorderLayout.CENTER);
                    f.pack();
                    f.setVisible(true);
                    f.remove(c);
                    manager.start();
                }
            });
    }
}
