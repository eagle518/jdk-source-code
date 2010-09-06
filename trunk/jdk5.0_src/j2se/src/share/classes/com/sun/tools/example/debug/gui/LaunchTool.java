/*
 * @(#)LaunchTool.java	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*
 * Copyright (c) 1997-1999 by Sun Microsystems, Inc. All Rights Reserved.
 * 
 * Sun grants you ("Licensee") a non-exclusive, royalty free, license to use,
 * modify and redistribute this software in source and binary code form,
 * provided that i) this copyright notice and license appear on all copies of
 * the software; and ii) Licensee does not utilize the software in a manner
 * which is disparaging to Sun.
 * 
 * This software is provided "AS IS," without a warranty of any kind. ALL
 * EXPRESS OR IMPLIED CONDITIONS, REPRESENTATIONS AND WARRANTIES, INCLUDING ANY
 * IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NON-INFRINGEMENT, ARE HEREBY EXCLUDED. SUN AND ITS LICENSORS SHALL NOT BE
 * LIABLE FOR ANY DAMAGES SUFFERED BY LICENSEE AS A RESULT OF USING, MODIFYING
 * OR DISTRIBUTING THE SOFTWARE OR ITS DERIVATIVES. IN NO EVENT WILL SUN OR ITS
 * LICENSORS BE LIABLE FOR ANY LOST REVENUE, PROFIT OR DATA, OR FOR DIRECT,
 * INDIRECT, SPECIAL, CONSEQUENTIAL, INCIDENTAL OR PUNITIVE DAMAGES, HOWEVER
 * CAUSED AND REGARDLESS OF THE THEORY OF LIABILITY, ARISING OUT OF THE USE OF
 * OR INABILITY TO USE SOFTWARE, EVEN IF SUN HAS BEEN ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGES.
 * 
 * This software is not designed or intended for use in on-line control of
 * aircraft, air traffic, aircraft navigation or aircraft communications; or in
 * the design, construction, operation or maintenance of any nuclear
 * facility. Licensee represents and warrants that it will not use or
 * redistribute the Software for such purposes.
 */

package com.sun.tools.example.debug.gui;

import java.util.List;
import java.util.ArrayList;
import java.util.Map;
import java.util.HashMap;
import java.util.Iterator;

import java.io.IOException;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Container;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;

import javax.swing.*;
import javax.swing.border.Border;
import javax.swing.border.TitledBorder;

import com.sun.jdi.*;
import com.sun.jdi.connect.*;

import com.sun.tools.example.debug.bdi.*;

class LaunchTool {

    private final ExecutionManager runtime;

    private abstract class ArgRep {
        final Connector.Argument arg;
        final JPanel panel;

        ArgRep(Connector.Argument arg) {
            this.arg = arg;
            panel = new JPanel();
            Border etched = BorderFactory.createEtchedBorder();
            Border titled = BorderFactory.createTitledBorder(etched, 
                                      arg.description(),
                                      TitledBorder.LEFT, TitledBorder.TOP); 
            panel.setBorder(titled);
        }

        abstract String getText();

        boolean isValid() {
            return arg.isValid(getText());
        }

        boolean isSpecified() {
            String value = getText();
            return (value != null && value.length() > 0) || 
                !arg.mustSpecify();
        }

        void install() {
            arg.setValue(getText());
        }
    }

    private class StringArgRep extends ArgRep {
        final JTextField textField;

        StringArgRep(Connector.Argument arg, JPanel comp) {
            super(arg);
            textField = new JTextField(arg.value(), 50 );
            textField.setBorder(BorderFactory.createLoweredBevelBorder());

            panel.add(new JLabel(arg.label(), SwingConstants.RIGHT));
            panel.add(textField); // , BorderLayout.CENTER);
            comp.add(panel);
        }

        String getText() {
            return textField.getText();
        }
    }

    private class BooleanArgRep extends ArgRep {
        final JCheckBox check;

        BooleanArgRep(Connector.BooleanArgument barg, JPanel comp) {
            super(barg);
            check = new JCheckBox(barg.label());
            check.setSelected(barg.booleanValue());
            panel.add(check);
            comp.add(panel);
        }

        String getText() {
            return ((Connector.BooleanArgument)arg)
                           .stringValueOf(check.getModel().isSelected());
        }
    }
        

    private LaunchTool(ExecutionManager runtime) {
        this.runtime = runtime;
    }

    private Connector selectConnector() {
        final JDialog dialog = new JDialog();
        Container content = dialog.getContentPane();
        final JPanel radioPanel = new JPanel();
        final ButtonGroup radioGroup = new ButtonGroup();
        VirtualMachineManager manager = Bootstrap.virtualMachineManager();
        List all = manager.allConnectors();
        Map modelToConnector = new HashMap(all.size(), 0.5f);

        dialog.setModal(true);
        dialog.setTitle("Select Connector Type");
        radioPanel.setLayout(new BoxLayout(radioPanel, BoxLayout.Y_AXIS));
        for (Iterator it = all.iterator(); it.hasNext(); ) {
            Connector connector = (Connector)it.next();
            JRadioButton radio = new JRadioButton(connector.description());
            modelToConnector.put(radio.getModel(), connector);
            radioPanel.add(radio);
            radioGroup.add(radio);
        }
        content.add(radioPanel);

        final boolean[] oked = {false};
        JPanel buttonPanel = okCancel( dialog, new ActionListener() { 
            public void actionPerformed(ActionEvent event) { 
                if (radioGroup.getSelection() == null) {
                    JOptionPane.showMessageDialog(dialog, 
                                    "Please select a connector type",
                                    "No Selection", 
                                     JOptionPane.ERROR_MESSAGE);
                } else {
                    oked[0] = true;
                    dialog.setVisible(false);
                    dialog.dispose();
                }
            } 
        } );
        content.add(BorderLayout.SOUTH, buttonPanel);
        dialog.pack();
        dialog.show();    

        return oked[0] ? 
            (Connector)(modelToConnector.get(radioGroup.getSelection())) : 
            null;
    }

    private void configureAndConnect(final Connector connector) {
        final JDialog dialog = new JDialog();
        final Map args = connector.defaultArguments();
        
        dialog.setModal(true);
        dialog.setTitle("Connector Arguments");
        Container content = dialog.getContentPane();
        JPanel guts = new JPanel();
        Border etched = BorderFactory.createEtchedBorder();
        Border titled = BorderFactory.createTitledBorder(etched, 
                                connector.description(),
                                TitledBorder.LEFT, TitledBorder.TOP); 
        guts.setBorder(etched);
        guts.setLayout(new BoxLayout(guts, BoxLayout.Y_AXIS));

        //        guts.add(new JLabel(connector.description()));

        final List argReps = new ArrayList(args.size());
        for (Iterator it = args.values().iterator(); it.hasNext(); ) {
            Object arg = it.next();
            ArgRep ar;
            if (arg instanceof Connector.BooleanArgument) {
                ar = new BooleanArgRep((Connector.BooleanArgument)arg, guts);
            } else {
                ar = new StringArgRep((Connector.Argument)arg, guts);
            }
            argReps.add(ar);
        }
        content.add(guts);

        JPanel buttonPanel = okCancel( dialog, new ActionListener() { 
            public void actionPerformed(ActionEvent event) { 
                for (Iterator it = argReps.iterator(); it.hasNext(); ) {
                    ArgRep ar = (ArgRep)it.next();
                    if (!ar.isSpecified()) {
                        JOptionPane.showMessageDialog(dialog, 
                                    ar.arg.label() + 
                                         ": Argument must be specified",
                                    "No argument", JOptionPane.ERROR_MESSAGE);
                        return;
                    }                        
                    if (!ar.isValid()) {
                        JOptionPane.showMessageDialog(dialog, 
                                    ar.arg.label() + 
                                         ": Bad argument value: " + 
                                         ar.getText(),
                                    "Bad argument", JOptionPane.ERROR_MESSAGE);
                        return;
                    }                        
                    ar.install();
                }
                try {
                    if (runtime.explictStart(connector, args)) {
                        dialog.setVisible(false);
                        dialog.dispose();
                    } else {
                        JOptionPane.showMessageDialog(dialog, 
                           "Bad arguments values: See diagnostics window.",
                           "Bad arguments", JOptionPane.ERROR_MESSAGE);
                    }
                } catch (VMLaunchFailureException exc) {
                        JOptionPane.showMessageDialog(dialog, 
                           "Launch Failure: " + exc,
                           "Launch Failed",JOptionPane.ERROR_MESSAGE);
                }
            } 
        } );
        content.add(BorderLayout.SOUTH, buttonPanel);
        dialog.pack();
        dialog.show();    
    }

    private JPanel okCancel(final JDialog dialog, ActionListener okListener) {
        JPanel buttonPanel = new JPanel();
        JButton ok = new JButton("OK");
        JButton cancel = new JButton("Cancel");
        buttonPanel.add(ok);
        buttonPanel.add(cancel);
        ok.addActionListener(okListener);
        cancel.addActionListener( new ActionListener() { 
            public void actionPerformed(ActionEvent event) { 
                dialog.setVisible(false);
                dialog.dispose();
            } 
        } );
        return buttonPanel;
    }

    static void queryAndLaunchVM(ExecutionManager runtime)
                                         throws VMLaunchFailureException {
        LaunchTool lt = new LaunchTool(runtime);
        Connector connector = lt.selectConnector();
        if (connector != null) {
            lt.configureAndConnect(connector);
        }
    }
}
