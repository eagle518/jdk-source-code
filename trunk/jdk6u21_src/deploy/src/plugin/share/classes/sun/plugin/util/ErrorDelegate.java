/*
 * @(#)ErrorDelegate.java	1.3 10/03/24
 *
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.util;

import java.util.*;

/** The ErrorDelegate helps abstract the "reload applet" operation
    from the ErrorPanel to break a dependence on the AppletViewer
    class. */
public interface ErrorDelegate {
    public void handleReloadApplet();
    public String getCodeBase();
	
    // This method adds CodeSources Locations to a set
    // Locations include jars and jnlp (if JNLP applets).
    public void addJarFilesToSet(Set/*<String>*/ jarSet);
}
