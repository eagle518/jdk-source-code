/*
 * @(#)WDesktopPeer.java
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.windows;


import java.awt.Desktop.Action;
import java.awt.peer.DesktopPeer;
import java.io.File;
import java.io.IOException;
import java.net.URI;


/**
 * Concrete implementation of the interface <code>DesktopPeer</code> for 
 * the Windows platform.
 * 
 * @see DesktopPeer
 */
public class WDesktopPeer implements DesktopPeer {
    /* Contants for the operation verbs */
    private static String ACTION_OPEN_VERB = "open";
    private static String ACTION_EDIT_VERB = "edit";
    private static String ACTION_PRINT_VERB = "print";
    
    public boolean isSupported(Action action) {
        // OPEN, EDIT, PRINT, MAIL, BROWSE all supported on windows.
        return true;
    }

    public void open(File file) throws IOException {
        this.ShellExecute(file.toURI(), ACTION_OPEN_VERB);
    }

    public void edit(File file) throws IOException {
        this.ShellExecute(file.toURI(), ACTION_EDIT_VERB);        
    }

    public void print(File file) throws IOException {
        this.ShellExecute(file.toURI(), ACTION_PRINT_VERB);        
    }

    public void mail(URI uri) throws IOException {
        this.ShellExecute(uri, ACTION_OPEN_VERB);
    }

    public void browse(URI uri) throws IOException {
        this.ShellExecute(uri, ACTION_OPEN_VERB);
    }
    
    private void ShellExecute(URI uri, String verb) throws IOException {
        String errmsg = ShellExecute(uri.toString(), verb);
        
        if (errmsg != null) {
            throw new IOException("Failed to " + verb + " " + uri
                    + ". Error message: " + errmsg);
        }
    }
    
    private static native String ShellExecute(String uri, String verb);

}
