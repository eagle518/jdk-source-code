/*
 * @(#)ThreadTab.java	1.25 04/06/11
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

import java.util.*;
import java.util.List;

import sun.awt.*;

class ThreadTab extends Tab implements DocumentListener, ListSelectionListener {
    PlotterPanel threadMeter;
    TimeComboBox timeComboBox;
    DefaultListModel listModel;
    JList list;
    JTextArea textArea;
    JTextField filterTF;
    HashMap<Long, String> nameCache = new HashMap<Long, String>();

    private static final String threadCountKey   = "threadCount";
    private static final String peakKey          = "peak";
    private static final String totalKey         = "total";

    private static final String threadCountName   = Resources.getText("Live Threads");
    private static final String peakName          = Resources.getText("Peak");
    private static final String totalName         = Resources.getText("Total Started");

    private static final Color  threadCountColor = Plotter.defaultColor;
    private static final Color  peakColor        = Color.red;
    private static final Color  totalColor       = Color.magenta;

    /*
      Hierarchy of panels and layouts for this tab:

	ThreadTab (BorderLayout)

	    North:  topPanel (BorderLayout)

			Center: controlPanel (FlowLayout)
				    timeComboBox

	    Center: plotterPanel (BorderLayout)

			Center: plotter

	    South:  bottomPanel (BorderLayout)

			Center: details
    */


    public static String getTabName() {
	return Resources.getText("Threads");
    }

    public ThreadTab(VMPanel vmPanel) {
	super(vmPanel, getTabName());

	setLayout(new BorderLayout(4, 4));
	setBorder(new EmptyBorder(4, 4, 3, 4));

	JPanel topPanel     = new JPanel(new BorderLayout());
	JPanel plotterPanel = new JPanel(new VariableGridLayout(0, 1, 4, 4, true, true));
	JPanel bottomPanel  = new JPanel(new BorderLayout(10, 10));

	add(topPanel, BorderLayout.NORTH);
	add(plotterPanel,  BorderLayout.CENTER);
	add(bottomPanel,  BorderLayout.SOUTH);

	JPanel controlPanel = new JPanel(new FlowLayout(FlowLayout.CENTER, 20, 5));
	topPanel.add(controlPanel, BorderLayout.CENTER);

        threadMeter = new PlotterPanel(Resources.getText("Number of Threads"), false, true);
	threadMeter.plotter.createSequence(threadCountKey, threadCountName,  threadCountColor, true);
	threadMeter.plotter.createSequence(peakKey,        peakName,         peakColor,        true);
	threadMeter.plotter.createSequence(totalKey,       totalName,        totalColor,       true);

	plotterPanel.add(threadMeter);

	timeComboBox = new TimeComboBox(threadMeter.plotter);
	controlPanel.add(new LabeledComponent(Resources.getText("Time Range:"),
					      timeComboBox));

	listModel = new DefaultListModel();

	list = new JList(listModel) {
	    {
		addListSelectionListener(ThreadTab.this);
		setCellRenderer(new DefaultListCellRenderer() {
		    public Component getListCellRendererComponent(JList list, Object value, int index,
								  boolean isSelected, boolean cellHasFocus) {
			super.getListCellRendererComponent(list, value, index, isSelected, cellHasFocus);

			if (value != null) {
			    String name = nameCache.get(value);
			    if (name == null) {
				name = value.toString();
			    }
			    setText(name);
			}
			return this;
		    }
		});
	    }

	    public Dimension getPreferredSize() {
		Dimension d = super.getPreferredSize();
		d.width = Math.max(d.width, 100);
		return d;
	    }
	};
	textArea = new JTextArea();
	bottomPanel.add(new JScrollPane(list), BorderLayout.WEST);
	bottomPanel.add(new JScrollPane(textArea), BorderLayout.CENTER);

	filterTF = new JTextField(20);
	filterTF.getDocument().addDocumentListener(this);
	JPanel p = new JPanel(new FlowLayout(FlowLayout.LEFT));
        p.add(new LabeledComponent(Resources.getText("Filter: "), filterTF));
	bottomPanel.add(p, BorderLayout.SOUTH);

        BorderedComponent c = new BorderedComponent(Resources.getText("Live Threads"), bottomPanel, true);
	plotterPanel.add(c);

	LabeledComponent.layout(plotterPanel);

    }

    private long oldThreads[] = new long[0];

    public void update() {
	ProxyClient proxyClient = vmPanel.getProxyClient();
	try {
	    final ThreadMXBean threadMBean = proxyClient.getThreadMXBean();

	    final int tlCount = threadMBean.getThreadCount();
	    threadMeter.plotter.addValue(threadCountKey, tlCount);

	    int tpCount = threadMBean.getPeakThreadCount();
	    threadMeter.plotter.addValue(peakKey, tpCount);

	    long ttCount = threadMBean.getTotalStartedThreadCount();
	    threadMeter.plotter.addValue(totalKey, ttCount);

	    final long[] threads = threadMBean.getAllThreadIds();

	    SwingUtilities.invokeAndWait(new Runnable() {
		public void run () {
		    threadMeter.setValueLabel(tlCount+"");

		    String filter = filterTF.getText();
		    boolean doFilter = (filter.length() > 0);

		    ArrayList<Long> l = new ArrayList<Long>();
		    for (long t : threads) {
			l.add(t);
		    }
		    Iterator<Long> iterator = l.iterator();
		    while (iterator.hasNext()) {
			long newThread = iterator.next();
			String name = nameCache.get(newThread);
			if (name == null) {
			    ThreadInfo ti = threadMBean.getThreadInfo(newThread);
			    if (ti != null) {
				name = ti.getThreadName();
				if (name != null) {
				    nameCache.put(newThread, name);
				}
			    }
			}
			if (doFilter && name != null && name.indexOf(filter) < 0) {
			    iterator.remove();
			}
		    }
		    long[] newThreads = threads;
		    if (l.size() < threads.length) {
			newThreads = new long[l.size()];
			for (int i = 0; i < newThreads.length; i++) {
			    newThreads[i] = l.get(i);
			}
		    }


		    for (long oldThread : oldThreads) {
			boolean found = false;
			for (long newThread : newThreads) {
			    if (newThread == oldThread) {
				found = true;
				break;
			    }
			}
			if (!found) {
			    listModel.removeElement(oldThread);
			    if (!doFilter) {
				nameCache.remove(oldThread);
			    }
			}
		    }

		    // Threads are in reverse chronological order
		    for (int i = newThreads.length - 1; i >= 0; i--) {
			long newThread = newThreads[i];
			boolean found = false;
			for (long oldThread : oldThreads) {
			    if (newThread == oldThread) {
				found = true;
				break;
			    }
			}
			if (!found) {
			    listModel.addElement(newThread);
			}
		    }
		    oldThreads = newThreads;
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

    long lastSelected = -1;

    public void valueChanged(ListSelectionEvent ev) {
	Long selected = (Long)list.getSelectedValue();
	if (selected == null) {
	    if (lastSelected != -1) {
		selected = lastSelected;
	    }
	} else {
	    lastSelected = selected;
	}
	textArea.setText("");
	if (selected != null) {
	    final long threadID = selected;
	    workerAdd(new Runnable() {
		public void run() {
		    ProxyClient proxyClient = vmPanel.getProxyClient();
		    StringBuilder sb = new StringBuilder();
		    try {
			ThreadMXBean threadMBean = proxyClient.getThreadMXBean();
			ThreadInfo ti = threadMBean.getThreadInfo(threadID, Integer.MAX_VALUE);
			if (ti != null) {
                            if (ti.getLockName() == null) {
                                sb.append(Resources.getText("Name State",
                                              ti.getThreadName(),
                                              ti.getThreadState().toString()));
                            } else if (ti.getLockOwnerName() == null) {
                                sb.append(Resources.getText("Name State LockName",
                                              ti.getThreadName(),
                                              ti.getThreadState().toString(),
                                              ti.getLockName()));
                            } else {
                                sb.append(Resources.getText("Name State LockName LockOwner",
                                              ti.getThreadName(),
                                              ti.getThreadState().toString(),
                                              ti.getLockName(),
                                              ti.getLockOwnerName()));
                            }
                            sb.append(Resources.getText("BlockedCount WaitedCount",
                                              ti.getBlockedCount(),
                                              ti.getWaitedCount()));
                            sb.append(Resources.getText("Stack trace"));
			    for (StackTraceElement e : ti.getStackTrace()) {
				sb.append(e.toString()+"\n");
			    }
			}
		    } catch (IOException ex) {
			// Ignore
		    } catch (UndeclaredThrowableException e) {
			proxyClient.markAsDead();
		    }
		    final String text = sb.toString();
		    SwingUtilities.invokeLater(new Runnable() {
			public void run() {
			    textArea.setText(text);
			    textArea.setCaretPosition(0);
			}
		    });
		}
	    });
	}
    }

    private void doUpdate() {
	workerAdd(new Runnable() {
	    public void run() {
		update();
	    }
	});
    }

    // DocumentListener interface

    public void insertUpdate(DocumentEvent e) {
	doUpdate();
    }

    public void removeUpdate(DocumentEvent e) {
	doUpdate();
    }

    public void changedUpdate(DocumentEvent e) {
	doUpdate();
    }
}
