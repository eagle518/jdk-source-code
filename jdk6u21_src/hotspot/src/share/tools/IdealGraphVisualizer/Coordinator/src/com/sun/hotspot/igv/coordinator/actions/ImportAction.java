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

package com.sun.hotspot.igv.coordinator.actions;

import com.sun.hotspot.igv.coordinator.OutlineTopComponent;
import com.sun.hotspot.igv.data.GraphDocument;
import com.sun.hotspot.igv.data.serialization.Parser;
import com.sun.hotspot.igv.settings.Settings;
import com.sun.hotspot.igv.data.serialization.XMLParser;
import java.awt.event.InputEvent;
import java.awt.event.KeyEvent;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import javax.swing.Action;
import javax.swing.JFileChooser;
import javax.swing.KeyStroke;
import javax.swing.filechooser.FileFilter;
import org.netbeans.api.progress.ProgressHandle;
import org.netbeans.api.progress.ProgressHandleFactory;
import org.openide.DialogDisplayer;
import org.openide.NotifyDescriptor;
import org.openide.util.HelpCtx;
import org.openide.util.NbBundle;
import org.openide.util.RequestProcessor;
import org.openide.util.actions.CallableSystemAction;
import org.openide.xml.XMLUtil;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;
import org.xml.sax.XMLReader;

/**
 *
 * @author Thomas Wuerthinger
 */
public final class ImportAction extends CallableSystemAction {

    public static FileFilter getFileFilter() {
        return new FileFilter() {

            public boolean accept(File f) {
                return f.getName().toLowerCase().endsWith(".xml") || f.isDirectory();
            }

            public String getDescription() {
                return "XML files (*.xml)";
            }
        };
    }

    public void performAction() {

        JFileChooser fc = new JFileChooser();
        fc.setFileFilter(ImportAction.getFileFilter());
        fc.setCurrentDirectory(new File(Settings.get().get(Settings.DIRECTORY, Settings.DIRECTORY_DEFAULT)));

        if (fc.showOpenDialog(null) == JFileChooser.APPROVE_OPTION) {
            File file = fc.getSelectedFile();

            File dir = file;
            if (!dir.isDirectory()) {
                dir = dir.getParentFile();
            }

            Settings.get().put(Settings.DIRECTORY, dir.getAbsolutePath());

            try {
                final XMLReader reader = XMLUtil.createXMLReader();
                final FileInputStream inputStream = new FileInputStream(file);
                final InputSource is = new InputSource(inputStream);

                final ProgressHandle handle = ProgressHandleFactory.createHandle("Opening file " + file.getName());
                final int basis = 1000;
                handle.start(basis);
                final int start = inputStream.available();

                final XMLParser.ParseMonitor parseMonitor = new XMLParser.ParseMonitor() {

                    public void setProgress(double d) {
                        try {
                            int curAvailable = inputStream.available();
                            int prog = (int) (basis * (double) (start - curAvailable) / (double) start);
                            handle.progress(prog);
                        } catch (IOException ex) {
                        }
                    }

                    public void setState(String state) {
                        setProgress(0.0);
                        handle.progress(state);
                    }
                };
                final Parser parser = new Parser();
                final OutlineTopComponent component = OutlineTopComponent.findInstance();

                component.requestActive();

                RequestProcessor.getDefault().post(new Runnable() {

                    public void run() {
                        GraphDocument document = null;
                        try {
                            document = parser.parse(reader, is, parseMonitor);
                            parseMonitor.setState("Finishing");
                            component.getDocument().addGraphDocument(document);
                        } catch (SAXException ex) {
                            String s = "Exception during parsing the XML file, could not load document!";
                            if (ex instanceof XMLParser.MissingAttributeException) {
                                XMLParser.MissingAttributeException e = (XMLParser.MissingAttributeException) ex;
                                s += "\nMissing attribute \"" + e.getAttributeName() + "\"";
                            }
                            ex.printStackTrace();
                            NotifyDescriptor d = new NotifyDescriptor.Message(s, NotifyDescriptor.ERROR_MESSAGE);
                            DialogDisplayer.getDefault().notify(d);
                        }
                        handle.finish();
                    }
                });

            } catch (SAXException ex) {
                ex.printStackTrace();
            } catch (FileNotFoundException ex) {
                ex.printStackTrace();
            } catch (IOException ex) {
                ex.printStackTrace();
            }
        }
    }

    public String getName() {
        return NbBundle.getMessage(ImportAction.class, "CTL_ImportAction");
    }

    public ImportAction() {
        putValue(Action.SHORT_DESCRIPTION, "Open an XML graph document");
        putValue(Action.ACCELERATOR_KEY, KeyStroke.getKeyStroke(KeyEvent.VK_O, InputEvent.CTRL_MASK));
    }

    @Override
    protected String iconResource() {
        return "com/sun/hotspot/igv/coordinator/images/import.gif";
    }

    public HelpCtx getHelpCtx() {
        return HelpCtx.DEFAULT_HELP;
    }

    @Override
    protected boolean asynchronous() {
        return false;
    }
}
