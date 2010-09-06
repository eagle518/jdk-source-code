/*
 * @(#)ClassTab.java	1.16 04/06/11
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.jconsole;

import java.awt.*;
import java.awt.event.*;
import java.io.*;
import java.lang.management.*;
import java.lang.reflect.*;

import javax.swing.*;
import javax.swing.border.*;
import javax.swing.event.*;
import javax.swing.text.*;

import java.util.*;
import java.util.List;

import sun.awt.*;

import static sun.tools.jconsole.Formatter.*;

class ClassTab extends Tab implements ActionListener {
    PlotterPanel loadedClassesMeter;
    TimeComboBox timeComboBox;
    private JCheckBox verboseCheckBox;
    private JEditorPane details;

    private static final String loadedPlotterKey        = "loaded";
    private static final String totalLoadedPlotterKey   = "totalLoaded";
    private static final String loadedPlotterName       = Resources.getText("Loaded");
    private static final String totalLoadedPlotterName  = Resources.getText("Total Loaded");
    private static final Color  loadedPlotterColor      = Plotter.defaultColor;
    private static final Color  totalLoadedPlotterColor = Color.red;

    /*
      Hierarchy of panels and layouts for this tab:

	ClassTab (BorderLayout)

	    North:  topPanel (BorderLayout)

			Center: controlPanel (FlowLayout)
				    timeComboBox

			East:   topRightPanel (FlowLayout)
				    verboseCheckBox

	    Center: plotterPanel (BorderLayout)

			Center: plotter

	    South:  bottomPanel (BorderLayout)

			Center: details
    */

    public static String getTabName() {
	return Resources.getText("Classes");
    }

    public ClassTab(VMPanel vmPanel) {
	super(vmPanel, getTabName());

	setLayout(new BorderLayout(4, 4));
	setBorder(new EmptyBorder(4, 4, 3, 4));

	JPanel topPanel     = new JPanel(new BorderLayout());
	JPanel plotterPanel = new JPanel(new BorderLayout());
	JPanel bottomPanel  = new JPanel(new BorderLayout());

	add(topPanel,     BorderLayout.NORTH);
	add(plotterPanel, BorderLayout.CENTER);
	add(bottomPanel,  BorderLayout.SOUTH);

	JPanel controlPanel = new JPanel(new FlowLayout(FlowLayout.CENTER, 20, 5));
	topPanel.add(controlPanel, BorderLayout.CENTER);

	verboseCheckBox = new JCheckBox(Resources.getText("Verbose Output"));
	verboseCheckBox.addActionListener(this);
	JPanel topRightPanel = new JPanel();
	topRightPanel.setBorder(new EmptyBorder(0, 65-8, 0, 70));
	topRightPanel.add(verboseCheckBox);
	topPanel.add(topRightPanel, BorderLayout.AFTER_LINE_ENDS);

	loadedClassesMeter = new PlotterPanel(Resources.getText("Number of Loaded Classes"), false, false);
	loadedClassesMeter.plotter.createSequence(loadedPlotterKey,
						  loadedPlotterName,
						  loadedPlotterColor,
						  true);
	loadedClassesMeter.plotter.createSequence(totalLoadedPlotterKey,
						  totalLoadedPlotterName,
						  totalLoadedPlotterColor,
						  true);
	plotterPanel.add(loadedClassesMeter);

	timeComboBox = new TimeComboBox(loadedClassesMeter.plotter);
	controlPanel.add(new LabeledComponent(Resources.getText("Time Range:"),
					      timeComboBox));

	LabeledComponent.layout(plotterPanel);

	bottomPanel.setBorder(new CompoundBorder(new TitledBorder(Resources.getText("Details")),
						  new EmptyBorder(10, 10, 10, 10)));

	details = new JEditorPane();
	details.setContentType("text/html");
	details.setEditable(false);
	//details.addHyperlinkListener(this);
	((DefaultCaret)details.getCaret()).setUpdatePolicy(DefaultCaret.NEVER_UPDATE);
	JScrollPane scrollPane = new JScrollPane(details);
	scrollPane.setPreferredSize(new Dimension(0, 150));
	bottomPanel.add(scrollPane, BorderLayout.SOUTH);
	
    }

    public void actionPerformed(ActionEvent ev) {
	final boolean b = verboseCheckBox.isSelected();
	workerAdd(new Runnable() {
	    public void run() {
		ProxyClient proxyClient = vmPanel.getProxyClient();
		try {
		    proxyClient.getClassLoadingMXBean().setVerbose(b);
		} catch (UndeclaredThrowableException e) {
		    proxyClient.markAsDead();
		} catch (IOException ex) {
		    // Ignore
		}
	    }
	});
    }


    public void update() {
	ProxyClient proxyClient = vmPanel.getProxyClient();
	try {
	    final ClassLoadingMXBean classLoadingMBean = proxyClient.getClassLoadingMXBean();

	    final long v1 = classLoadingMBean.getLoadedClassCount();
	    loadedClassesMeter.plotter.addValue(loadedPlotterKey, v1);

	    final long v2 = classLoadingMBean.getTotalLoadedClassCount();
	    loadedClassesMeter.plotter.addValue(totalLoadedPlotterKey, v2);

	    final boolean isVerbose = classLoadingMBean.isVerbose();
	    final String detailsStr = formatDetails(classLoadingMBean);

	    SwingUtilities.invokeAndWait(new Runnable() {
		public void run () {
		    loadedClassesMeter.setValueLabel(v1+"");
		    verboseCheckBox.setSelected(isVerbose);
		    details.setText(detailsStr);
		}
	    });
	} catch (IOException e) {
	    // Ignore
	} catch (UndeclaredThrowableException e) {
	    proxyClient.markAsDead();
	} catch (InvocationTargetException ex) {
	    // Ignore
	} catch (InterruptedException ex) {
	    // Ignore
	}
    }

    private String formatDetails(ClassLoadingMXBean clMBean) {
	String text = "<html><table cellspacing=0 cellpadding=0>";

	//long time = plotter.getLastTimeStamp();
	long time = System.currentTimeMillis();
	String timeStamp = formatDateTime(time);
	text += newRow(Resources.getText("Time"), timeStamp);

	long clCount = clMBean.getLoadedClassCount();
	long cuCount = clMBean.getUnloadedClassCount();
	long ctCount = clMBean.getTotalLoadedClassCount();
	text += newRow(Resources.getText("Current classes loaded"), justify(clCount, 5));
	text += newRow(Resources.getText("Total classes loaded"),   justify(ctCount, 5));
	text += newRow(Resources.getText("Total classes unloaded"), justify(cuCount, 5));

	return text;
    }

}
