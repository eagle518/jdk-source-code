/*
 * @(#)XPlottingViewer.java	1.9 04/06/22
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.tools.jconsole.inspector;

import java.awt.*;
import java.awt.event.*;
import java.io.*;
import java.util.*;
import java.util.Timer;

import javax.management.*;
import javax.swing.*;
import javax.swing.border.*;
import javax.swing.event.*;

import sun.tools.jconsole.Plotter;
import sun.tools.jconsole.Resources;
import sun.tools.jconsole.BorderedComponent;
import sun.tools.jconsole.MBeansTab;
import sun.tools.jconsole.Worker;

public class XPlottingViewer extends JPanel implements ActionListener {
    private JButton plotButton;
    // The plotter cache holds Plotter instances for the various attributes
    private static HashMap<String, XPlottingViewer> plotterCache =
	new HashMap<String, XPlottingViewer>();
     private static HashMap<String, Timer> timerCache =
	 new HashMap<String, Timer>();
    private JPanel bordered;
    private Number value;
    private MBeansTab tab;
    private XMBean mbean;
    private String attributeName;
    private Plotter plotter;
    private String key;
    private JTable table;
    private XPlottingViewer(String key,
			    XMBean mbean,
			    String attributeName, 
			    Object value,
			    JTable table,
			    MBeansTab tab) {
	this.tab = tab;
	this.key = key;
	this.mbean = mbean;
	this.table = table;
	this.attributeName = attributeName;
	plotter = createPlotter(mbean, attributeName, key, table);
	setupDisplay();
    }
    
    private Plotter getPlotter() {
	return plotter;
    }
    
    static void dispose(MBeansTab tab) {
	Iterator it = plotterCache.keySet().iterator();
	while(it.hasNext()) {
	    String key = (String) it.next();
	    if(key.startsWith(String.valueOf(tab.hashCode()))) {
		it.remove();
	    }
	}
	//plotterCache.clear();
	it = timerCache.keySet().iterator();
	while(it.hasNext()) { 
	    String key = (String) it.next();
	    if(key.startsWith(String.valueOf(tab.hashCode()))) {
		Timer t = timerCache.get(key);
		t.cancel();
		it.remove();
	    }
	}
    }
    
    public static boolean isViewableValue(Object value) {
	return (value instanceof Number);
    }
    
    //Fired by dbl click
    public  static Component loadPlotting(XMBean mbean,
					  String attributeName, 
					  Object value,
					  JTable table,
					  MBeansTab tab) {
	Component comp = null;
	if(isViewableValue(value)) {
	    String key = String.valueOf(tab.hashCode()) + " " + String.valueOf(mbean.hashCode()) + " " + mbean.getObjectName().getCanonicalName() + attributeName;
	    XPlottingViewer plotter = plotterCache.get(key);
	    if(plotter == null) {
		plotter = new XPlottingViewer(key,
					      mbean, 
					      attributeName, 
					      value, 
					      table, 
					      tab);
		plotterCache.put(key, plotter);
	    }
	    
	    comp = plotter;
	}
	return comp;
    }

    /*public void paintComponent(Graphics g) {
	super.paintComponent(g);
	setBackground(g.getColor());
	plotter.paintComponent(g);
	}*/
    public void actionPerformed(ActionEvent evt) {
	plotterCache.remove(key);
	Timer t = timerCache.remove(key);
	t.cancel();
	((XMBeanAttributes) table).collapse(attributeName, this);
    }
    
    //Create plotter instance
    public Plotter createPlotter(final XMBean xmbean, 
				 final String attributeName,
				 String key,
				 JTable table) {
	final Plotter plotter = new XPlotter(table, false) {
		Dimension prefSize = new Dimension(400, 170);
		public Dimension getPreferredSize() {
		    return prefSize;
		}
		public Dimension getMinimumSize() {
		    return prefSize;
		}
	    };
	
	plotter.createSequence(attributeName, attributeName, null, true);

	TimerTask timerTask = new TimerTask() {
		public void run() {
		    tab.workerAdd(new Runnable() {
			    public void run() {
				try {
				    Number n = 
					(Number) xmbean.getMBeanServerConnection().getAttribute(xmbean.getObjectName(), attributeName);
				    plotter.addValue(attributeName, n.longValue());
				} catch (Exception ex) {
				    // Should have a trace logged with proper
				    // trace mecchanism
				}
			    }
			});
		}
	    };
	    
	String timerName = "Timer-" + key;
	Timer timer = new Timer(timerName, true);
	timer.schedule(timerTask, 0, tab.getUpdateInterval());
	timerCache.put(key, timer);
	return plotter;
    }
    
    //Create Plotter display
    private void setupDisplay() {
	//setLayout(new GridLayout(2,0));
	GridBagLayout gbl = new GridBagLayout();
	setLayout(gbl);
    	setBackground(Color.white);
	plotButton = new JButton(Resources.getText("Discard chart"));
	plotButton.addActionListener(this);
	plotButton.setEnabled(true);
	
	// Add the display to the top four cells
	GridBagConstraints buttonConstraints = new GridBagConstraints();
	buttonConstraints.gridx = 0;
	buttonConstraints.gridy = 0;
	buttonConstraints.fill = GridBagConstraints.VERTICAL;
	buttonConstraints.anchor = GridBagConstraints.CENTER;
	gbl.setConstraints(plotButton, buttonConstraints);
	add(plotButton);
	
	GridBagConstraints plotterConstraints = new GridBagConstraints();
	plotterConstraints.gridx = 0;
	plotterConstraints.gridy = 1;
	plotterConstraints.weightx = 1;
	//plotterConstraints.gridwidth = (int) plotter.getPreferredSize().getWidth();
	//plotterConstraints.gridheight =  (int) plotter.getPreferredSize().getHeight();
	plotterConstraints.fill = GridBagConstraints.VERTICAL;
	gbl.setConstraints(plotter, plotterConstraints);
	

	//bordered = new JPanel();
	//bordered.setPreferredSize(new Dimension(400, 250));
	//bordered.add(plotButton);
	//bordered.add(plotter);
	
	//add(bordered);

	add(plotter);
	repaint();
    }
    
}
