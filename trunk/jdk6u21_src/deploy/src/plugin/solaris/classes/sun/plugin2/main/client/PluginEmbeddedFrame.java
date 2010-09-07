/*
 * @(#)PluginEmbeddedFrame.java	1.8 10/03/24 12:01:30
 *
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.main.client;

import java.awt.Dialog;
import sun.awt.X11.*;
import java.io.PrintStream;
import java.io.ByteArrayOutputStream;
import java.io.ByteArrayInputStream;
import sun.print.PSPrinterJob;
import java.applet.Applet;

import sun.plugin2.message.Pipe;
import sun.plugin.util.UIUtil;

/** This class overrides
    XEmbeddedFrame.notifyModalBlocked() on JREs that don't support the
    ModalityListener interface. NOTE: (FIXME) it currently appears
    that notifyModalBlocked with blocker == true is not called by the
    AWT, which would prevent this from working even if it were in
    use. */

public class PluginEmbeddedFrame extends XEmbeddedFrame {
    private ModalityInterface modality;

    public PluginEmbeddedFrame(long handle,
                               long parentConnection,
                               boolean supportsXEmbed,
                               ModalityInterface modality,
                               Pipe pipe,
                               int appletID) {
        super(handle, supportsXEmbed);
        this.modality = modality;
    }

    /** Override addNotify() method to disable background erase
     *  before the peer is visible
     */
    public void addNotify() {
	UIUtil.disableBackgroundErase(this);
	super.addNotify();
    }

    public void notifyModalBlocked(Dialog blocker, boolean blocked) {
        if (modality != null) {
            if (blocked) {
                modality.modalityPushed(blocker);
            } else {
                modality.modalityPopped(blocker);
            }
        }
    }

    public byte[] printPlugin(Applet applet, int x, int y, int w, int h) {

        ByteArrayOutputStream out = new ByteArrayOutputStream();
        PrintStream printOut = new PrintStream(out);
        
        if (applet != null) {
            PSPrinterJob.PluginPrinter pp = new
                PSPrinterJob.PluginPrinter(applet, printOut, x, y, w, h);
            try {
                pp.printAll();
            } catch (Throwable t) {
                // don't let applet errors hang/crash plugin
                t.printStackTrace();;
            }
        }

        return out.toByteArray();

    }
}
