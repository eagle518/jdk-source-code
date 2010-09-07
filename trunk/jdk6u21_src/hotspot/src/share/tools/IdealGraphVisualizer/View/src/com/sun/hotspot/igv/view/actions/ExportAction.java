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
package com.sun.hotspot.igv.view.actions;

import com.sun.hotspot.igv.settings.Settings;
import com.sun.hotspot.igv.view.ExportCookie;
import java.awt.event.InputEvent;
import java.awt.event.KeyEvent;
import java.io.File;
import javax.swing.Action;
import javax.swing.JFileChooser;
import javax.swing.KeyStroke;
import javax.swing.filechooser.FileFilter;
import org.openide.util.HelpCtx;
import org.openide.util.Lookup;
import org.openide.util.LookupEvent;
import org.openide.util.LookupListener;
import org.openide.util.NbBundle;
import org.openide.util.Utilities;
import org.openide.util.actions.CallableSystemAction;

/**
 *
 * @author Thomas Wuerthinger
 */
public final class ExportAction extends CallableSystemAction implements LookupListener {

    private final Lookup lookup;
    private final Lookup.Result<ExportCookie> result;

    public ExportAction() {
        putValue(Action.SHORT_DESCRIPTION, "Export current graph as an SVG file");
        putValue(Action.ACCELERATOR_KEY, KeyStroke.getKeyStroke(KeyEvent.VK_E, InputEvent.CTRL_MASK));
        lookup = Utilities.actionsGlobalContext();
        result = lookup.lookup(new Lookup.Template<ExportCookie>(ExportCookie.class));
        result.addLookupListener(this);
        resultChanged(null);
    }

    public void resultChanged(LookupEvent e) {
        super.setEnabled(result.allInstances().size() > 0);
    }

    public void performAction() {

        JFileChooser fc = new JFileChooser();
        fc.setFileFilter(new FileFilter() {

            public boolean accept(File f) {
                return true;
            }

            public String getDescription() {
                return "SVG files (*.svg)";
            }
        });
        fc.setCurrentDirectory(new File(Settings.get().get(Settings.DIRECTORY, Settings.DIRECTORY_DEFAULT)));


        if (fc.showSaveDialog(null) == JFileChooser.APPROVE_OPTION) {
            File file = fc.getSelectedFile();
            if (!file.getName().contains(".")) {
                file = new File(file.getAbsolutePath() + ".svg");
            }

            File dir = file;
            if (!dir.isDirectory()) {
                dir = dir.getParentFile();
            }

            Settings.get().put(Settings.DIRECTORY, dir.getAbsolutePath());
            ExportCookie cookie = Utilities.actionsGlobalContext().lookup(ExportCookie.class);
            if (cookie != null) {
                cookie.export(file);
            }
        }
    }

    public String getName() {
        return NbBundle.getMessage(ExportAction.class, "CTL_ExportAction");
    }

    @Override
    protected String iconResource() {
        return "com/sun/hotspot/igv/view/images/export.gif";
    }

    public HelpCtx getHelpCtx() {
        return HelpCtx.DEFAULT_HELP;
    }

    @Override
    protected boolean asynchronous() {
        return false;
    }
}
