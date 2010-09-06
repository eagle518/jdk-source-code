/*
 * @(#)MemoryTab.java	1.26 04/06/28
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
import java.util.*;

import javax.management.*;
import javax.management.openmbean.CompositeData;
import javax.swing.*;
import javax.swing.border.*;
import javax.swing.text.*;

import sun.management.*;

import static sun.tools.jconsole.Formatter.*;
import static sun.tools.jconsole.Resources.*;

class MemoryTab extends Tab implements ActionListener, ItemListener {
    JComboBox plotterChoice;
    TimeComboBox timeComboBox;
    JButton gcButton;

    JPanel plotterPanel;
    JPanel bottomPanel;
    JEditorPane details;
    PoolChart poolChart;

    ArrayList<Plotter> plotterList;
    Plotter heapPlotter, nonHeapPlotter;

    private static final String usedKey        = "used";
    private static final String committedKey   = "committed";
    private static final String maxKey         = "max";
    private static final String thresholdKey   = "threshold";

    private static final String usedName        = Resources.getText("Used");
    private static final String committedName   = Resources.getText("Committed");
    private static final String maxName         = Resources.getText("Max");
    private static final String thresholdName   = Resources.getText("Threshold");

    private static final Color  usedColor      = Plotter.defaultColor;
    private static final Color  committedColor = null;
    private static final Color  maxColor       = null;
    private static final Color  thresholdColor = Color.red;

    /*
      Hierarchy of panels and layouts for this tab:

	MemoryTab (BorderLayout)

	    North:  topPanel (BorderLayout)

			Center: controlPanel (FlowLayout)
				    plotterChoice, timeComboBox

			East:   topRightPanel (FlowLayout)
				    gcButton

	    Center: plotterPanel (BorderLayout)

			Center: plotter

	    South:  bottomPanel (BorderLayout)

			Center: details
			East:   poolChart
    */


    public static String getTabName() {
	return getText("Memory");
    }

    public MemoryTab(VMPanel vmPanel) {
	super(vmPanel, getTabName());

	setLayout(new BorderLayout(4, 4));
	setBorder(new EmptyBorder(4, 4, 3, 4));

	JPanel topPanel     = new JPanel(new BorderLayout());
	       plotterPanel = new JPanel(new BorderLayout());
	JPanel bottomPanel  = new JPanel(new BorderLayout());

	add(topPanel,     BorderLayout.NORTH);
	add(plotterPanel, BorderLayout.CENTER);
	add(bottomPanel,  BorderLayout.SOUTH);

	JPanel controlPanel = new JPanel(new FlowLayout(FlowLayout.LEADING, 20, 5));
	topPanel.add(controlPanel, BorderLayout.CENTER);

	// Plotter choice
	plotterChoice = new JComboBox();
	plotterChoice.addItemListener(this);
	controlPanel.add(new LabeledComponent(getText("Chart:"), plotterChoice));

	// Range control
	timeComboBox = new TimeComboBox();
	controlPanel.add(new LabeledComponent(getText("Time Range:"), timeComboBox));

	gcButton = new JButton(getText("Perform GC"));
	gcButton.addActionListener(this);
	JPanel topRightPanel = new JPanel();
	topRightPanel.setBorder(new EmptyBorder(0, 65-8, 0, 70));
	topRightPanel.add(gcButton);
	topPanel.add(topRightPanel, BorderLayout.AFTER_LINE_ENDS);

	bottomPanel.setBorder(new CompoundBorder(new TitledBorder(getText("Details")),
						  new EmptyBorder(10, 10, 10, 10)));

	details = new JEditorPane();
	details.setContentType("text/html");
	details.setEditable(false);
	//details.addHyperlinkListener(this);
	((DefaultCaret)details.getCaret()).setUpdatePolicy(DefaultCaret.NEVER_UPDATE);
	bottomPanel.add(new JScrollPane(details), BorderLayout.CENTER);

	poolChart = new PoolChart();
	bottomPanel.add(poolChart, BorderLayout.AFTER_LINE_ENDS);

	createPlotters();
    }


    private void createPlotters() {
	plotterList = new ArrayList<Plotter>();
	workerAdd(new Runnable() {
	    public void run() {
		heapPlotter = new Plotter(true) {
		    public String toString() {
			return Resources.getText("Heap Memory Usage");
		    }
		};

		nonHeapPlotter = new Plotter(true) {
		    public String toString() {
			return Resources.getText("Non-Heap Memory Usage");
		    }
		};

		heapPlotter.createSequence(usedKey,         usedName,      usedColor,      true);
		heapPlotter.createSequence(committedKey,    committedName, committedColor, false);
		heapPlotter.createSequence(maxKey,          maxName,       maxColor,       false);

		nonHeapPlotter.createSequence(usedKey,      usedName,      usedColor,      true);
		nonHeapPlotter.createSequence(committedKey, committedName, committedColor, false);
		nonHeapPlotter.createSequence(maxKey,       maxName,       maxColor,       false);

		plotterList.add(heapPlotter);
		plotterList.add(nonHeapPlotter);

		// Now add memory pools
		ProxyClient proxyClient = vmPanel.getProxyClient();
		try {
		    Map<ObjectName, MBeanInfo> mBeanMap = proxyClient.getMBeans("java.lang");
		    Set<ObjectName> keys = mBeanMap.keySet();
		    ObjectName[] objectNames = keys.toArray(new ObjectName[keys.size()]);
		    ArrayList<PoolPlotter> nonHeapPlotters = new ArrayList<PoolPlotter>(2);
		    for (ObjectName objectName : objectNames) {
			String type = objectName.getKeyProperty("type");
			if (type.equals("MemoryPool")) {
			    String name = getText("MemoryPoolLabel",
						  objectName.getKeyProperty("name"));
			    // Heap or non-heap?
			    boolean isHeap = false;
			    AttributeList al =
				proxyClient.getAttributes(objectName,
							  new String[] { "Type" });
			    if (al.size() > 0) {
				isHeap = MemoryType.HEAP.name().equals(((Attribute)al.get(0)).getValue());
			    }
			    PoolPlotter poolPlotter = new PoolPlotter(true, objectName,
								      name, isHeap);

			    poolPlotter.createSequence(usedKey,      usedName,      usedColor,      true);
			    poolPlotter.createSequence(committedKey, committedName, committedColor, false);
			    poolPlotter.createSequence(maxKey,       maxName,       maxColor,       false);
			    poolPlotter.createSequence(thresholdKey, thresholdName, thresholdColor, false);

			    if (isHeap) {
				plotterList.add(poolPlotter);
			    } else {
				// Will be added to plotterList below
				nonHeapPlotters.add(poolPlotter);
			    }
			}
		    }
		    // Add non-heap plotters last
		    for (PoolPlotter poolPlotter : nonHeapPlotters) {
			plotterList.add(poolPlotter);
		    }
		} catch (UndeclaredThrowableException e) {
		    proxyClient.markAsDead();
		    return;
		} catch (final IOException ex) {
		    // Oops, we'll have to ignore this
		}
		SwingUtilities.invokeLater(new Runnable() {
		    public void run() {
			for (Plotter p : plotterList) {
			    plotterChoice.addItem(p);
			    timeComboBox.addPlotter(p);
			}
		    }
		});
	    }
	});
    }

    public void itemStateChanged(ItemEvent ev) {
	if (ev.getStateChange() == ItemEvent.SELECTED) {
	    Plotter plotter = (Plotter)plotterChoice.getSelectedItem();
	    if (plotterPanel.getComponentCount() == 1) {
		plotterPanel.remove(0);
	    }
	    plotterPanel.add(plotter, BorderLayout.CENTER);
	    plotterPanel.revalidate();
	}
    }

    public void gc() {
	new Thread("MemoryPanel.gc") {
	    public void run() {
		ProxyClient proxyClient = vmPanel.getProxyClient();
		try {
		    proxyClient.getMemoryMXBean().gc();
		} catch (UndeclaredThrowableException e) {
		    proxyClient.markAsDead();
		} catch (IOException e) {
		    // Ignore
		}
	    }
	}.start();
    }

    public void update() {
	int poolCount = 0;
	ProxyClient proxyClient = vmPanel.getProxyClient();
	for (Plotter plotter : plotterList) {
	    MemoryUsage mu = null;
	    long threshold = 0;
	    try {
		if (plotter instanceof PoolPlotter) {
		    PoolPlotter poolPlotter = (PoolPlotter)plotter;
		    ObjectName objectName = poolPlotter.objectName;
		    AttributeList al =
			proxyClient.getAttributes(objectName,
						  new String[] { "Usage", "UsageThreshold" });
		    if (al.size() > 0) {
			CompositeData cd = (CompositeData)((Attribute)al.get(0)).getValue();
			mu = MemoryUsage.from(cd);

			if (al.size() > 1) {
			    threshold = (Long)((Attribute)al.get(1)).getValue();
			}
		    }
		} else if (plotter == heapPlotter) {
		    mu = proxyClient.getMemoryMXBean().getHeapMemoryUsage();
		} else if (plotter == nonHeapPlotter) {
		    mu = proxyClient.getMemoryMXBean().getNonHeapMemoryUsage();
		}
	    } catch (UndeclaredThrowableException e) {
		proxyClient.markAsDead();
		break;
	    } catch (IOException ex) {
		// Skip this plotter
	    }
	    if (mu != null) {
		long used      = mu.getUsed();
		long committed = mu.getCommitted();
		long max       = mu.getMax();
		plotter.addValue(usedKey,      used);
		plotter.addValue(committedKey, committed);
		plotter.addValue(maxKey,       max);
		if (plotter instanceof PoolPlotter) {
		    plotter.addValue(thresholdKey, threshold);
		    if (threshold > 0L) {
			plotter.setIsPlotted(thresholdKey, true);
		    }
		    poolChart.setValue(poolCount++, (PoolPlotter)plotter,
				       used, threshold, max);
		}
	    }
	}

	final String str = formatDetails();
	try {
	    SwingUtilities.invokeAndWait(new Runnable() {
		public void run() {
		    details.setText(str);
		}
	    });
	} catch (InvocationTargetException ex) {
	    // Ignore
	} catch (InterruptedException ex) {
	    // Ignore
	}

    }

    private String formatDetails() {
	ProxyClient proxyClient = vmPanel.getProxyClient();
	if (proxyClient.isDead()) {
	    return "";
	}

	String text = "<html><table cellspacing=0 cellpadding=0>";

	Plotter plotter = (Plotter)plotterChoice.getSelectedItem();
	if (plotter == null) {
	    return "";
	}

	//long time = plotter.getLastTimeStamp();
	long time = System.currentTimeMillis();
	String timeStamp = formatDateTime(time);
	text += newRow(getText("Time"), timeStamp);

	long used = plotter.getLastValue(usedKey);
	long committed = plotter.getLastValue(committedKey);
	long max = plotter.getLastValue(maxKey);
	long threshold = plotter.getLastValue(thresholdKey);

        text += newRow(getText("Used"), formatKBytes(used));
        if (committed > 0L) {
            text += newRow(getText("Committed"), formatKBytes(committed));
        }
        if (max > 0L) {
            text += newRow(getText("Max"), formatKBytes(max));
        }
        if (threshold > 0L) {
            text += newRow(getText("Usage Threshold"), formatKBytes(threshold));
        }

        try {
            Collection<GarbageCollectorMXBean> garbageCollectors =
                proxyClient.getGarbageCollectorMXBeans();
    
            boolean descPrinted = false;
            for (GarbageCollectorMXBean garbageCollectorMBean : garbageCollectors) {
                String gcName = garbageCollectorMBean.getName();
                long gcCount = garbageCollectorMBean.getCollectionCount();
                long gcTime = garbageCollectorMBean.getCollectionTime();
                String str = getText("GC time details", justify(formatTime(gcTime), 14),
                                     gcName,
                                     String.format("%,d",gcCount));
                if (!descPrinted) {
                    text += newRow(getText("GC time"), str);
                    descPrinted = true;
                } else {
                    text += newRow(null, str); 
                }
           }
        } catch (IOException e) {
        }

	return text;
    }

    public void actionPerformed(ActionEvent ev) {
	Object src = ev.getSource();
	if (src == gcButton) {
	    gc();
	}
    }

    private class PoolPlotter extends Plotter {
	ObjectName objectName;
	String name;
	boolean isHeap;
	long value, threshold, max;
	int barX;
	
	public PoolPlotter(boolean unitsBytes, ObjectName objectName,
			   String name, boolean isHeap) {
	    super(unitsBytes);

	    this.objectName = objectName;
	    this.name       = name;
	    this.isHeap     = isHeap;
	}


	public String toString() {
	    return name;
	}
    }

    private class PoolChart extends JComponent implements MouseListener {
	final int height       = 150;
	final int leftMargin   =  50;
	final int rightMargin  =  23;
	final int bottomMargin =  35;
	final int barWidth     =  22;
	final int barGap       =   3;
	final int groupGap     =   8;
	final int barHeight    = height * 2 / 3;

	final Color greenBar           = new Color(100, 255, 100);
	final Color greenBarBackground = new Color(210, 255, 210);
	final Color redBarBackground   = new Color(255, 210, 210);

	Font smallFont = null;

	ArrayList<PoolPlotter> poolPlotters = new ArrayList<PoolPlotter>(5);

	int nHeapPools    = 0;
	int nNonHeapPools = 0;
	Rectangle heapRect    = new Rectangle(leftMargin,            height - bottomMargin + 6, barWidth, 20);
	Rectangle nonHeapRect = new Rectangle(leftMargin + groupGap, height - bottomMargin + 6, barWidth, 20);

	public PoolChart() {
	    setBackground(Color.green.brighter());

	    addMouseListener(this);
	    ToolTipManager.sharedInstance().registerComponent(this);
	}

	public void setValue(int poolIndex, PoolPlotter poolPlotter,
			     long value, long threshold, long max) {
	    poolPlotter.value = value;
	    poolPlotter.threshold = threshold;
	    poolPlotter.max = max;

	    if (poolIndex == poolPlotters.size()) {
		poolPlotters.add(poolPlotter);
		if (poolPlotter.isHeap) {
		    poolPlotter.barX = nHeapPools * (barWidth + barGap);
		    nHeapPools++;
		    heapRect.width = nHeapPools * barWidth + (nHeapPools - 1) * barGap;
		    nonHeapRect.x  = leftMargin + heapRect.width + groupGap;
		} else {
		    poolPlotter.barX = nonHeapRect.x - leftMargin + nNonHeapPools * (barWidth + barGap);
		    nNonHeapPools++;
		    nonHeapRect.width = nNonHeapPools * barWidth + (nNonHeapPools - 1) * barGap;
		}
	    } else {
		poolPlotters.set(poolIndex, poolPlotter);
	    }
	    repaint();
	}

	private void paintPoolBar(Graphics g, PoolPlotter poolPlotter) {
	    Rectangle barRect = getBarRect(poolPlotter);
	    g.setColor(Color.gray);
	    g.drawRect(barRect.x, barRect.y, barRect.width, barRect.height);

	    long value = poolPlotter.value;
	    long max   = poolPlotter.max;
	    if (max > 0L) {
		g.translate(barRect.x, barRect.y);

		// Paint green background
		g.setColor(greenBarBackground);
		g.fillRect(1, 1, barRect.width - 1, barRect.height - 1);

		int greenHeight = (int)(value * barRect.height / max);
		long threshold = poolPlotter.threshold;
		if (threshold > 0L) {
		    int redHeight = (int)(threshold * barRect.height / max);

		    // Paint red background
		    g.setColor(redBarBackground);
		    g.fillRect(1, 1, barRect.width - 1, barRect.height - redHeight);

		    if (value > threshold) {
			// Over threshold, paint red bar
			g.setColor(thresholdColor);
			g.fillRect(1, barRect.height - greenHeight,
				   barRect.width - 1, greenHeight - redHeight);
			greenHeight = redHeight;
		    }
		}

		// Paint green bar
		g.setColor(greenBar);
		g.fillRect(1, barRect.height - greenHeight,
			   barRect.width - 1, greenHeight);

		g.translate(-barRect.x, -barRect.y);
	    }
	}

	final Color bg = new Color(250, 255, 250);

	public void paintComponent(Graphics g) {
	    super.paintComponent(g);

	    if (poolPlotters.size() == 0) {
		return;
	    }

	    if (smallFont == null) {
		smallFont = g.getFont().deriveFont(9.0F);
	    }

	    // Paint background for chart area
	    g.setColor(bg);
	    Rectangle r = g.getClipRect();
	    g.fillRect(r.x, r.y, r.width, r.height);

	    g.setFont(smallFont);
	    FontMetrics fm = g.getFontMetrics();
	    int fontDescent = fm.getDescent();

	    // Paint percentage axis
	    g.setColor(Color.black);
	    for (int pc : new int[] { 0, 25, 50, 75, 100 }) {
		String str = pc + "% --";
		g.drawString(str,
			     leftMargin - fm.stringWidth(str) - 4,
			     height - bottomMargin - (pc * barHeight / 100) + fontDescent + 1);
	    }

	    for (PoolPlotter poolPlotter : poolPlotters) {
		paintPoolBar(g, poolPlotter);
	    }

	    g.setColor(Color.gray);
	    g.drawRect(heapRect.x,    heapRect.y,    heapRect.width,    heapRect.height);
	    g.drawRect(nonHeapRect.x, nonHeapRect.y, nonHeapRect.width, nonHeapRect.height);

	    Color heapColor    = greenBar;
	    Color nonHeapColor = greenBar;


	    for (PoolPlotter poolPlotter : poolPlotters) {
		if (poolPlotter.threshold > 0L && poolPlotter.value > poolPlotter.threshold) {
		    if (poolPlotter.isHeap) {
			heapColor = thresholdColor;
		    } else {
			nonHeapColor = thresholdColor;
		    }
		}
	    }
	    g.setColor(heapColor);
	    g.fillRect(heapRect.x + 1,    heapRect.y + 1,    heapRect.width - 1,    heapRect.height - 1);
	    g.setColor(nonHeapColor);
	    g.fillRect(nonHeapRect.x + 1, nonHeapRect.y + 1, nonHeapRect.width - 1, nonHeapRect.height - 1);

	    String str = getText("Heap");
	    int stringWidth = fm.stringWidth(str);
	    int x = heapRect.x + (heapRect.width - stringWidth) / 2;
	    int y = heapRect.y + heapRect.height - 6;
	    g.setColor(Color.white);
	    g.drawString(str, x-1, y-1);
	    g.drawString(str, x+1, y-1);
	    g.drawString(str, x-1, y+1);
	    g.drawString(str, x+1, y+1);
	    g.setColor(Color.black);
	    g.drawString(str, x, y);

	    str = getText("Non-Heap");
	    stringWidth = fm.stringWidth(str);
	    x = nonHeapRect.x + (nonHeapRect.width - stringWidth) / 2;
	    y = nonHeapRect.y + nonHeapRect.height - 6;
	    g.setColor(Color.white);
	    g.drawString(str, x-1, y-1);
	    g.drawString(str, x+1, y-1);
	    g.drawString(str, x-1, y+1);
	    g.drawString(str, x+1, y+1);
	    g.setColor(Color.black);
	    g.drawString(str, x, y);

	    // Highlight current plotter
	    g.setColor(Color.blue);
	    r = null;
	    Plotter plotter = (Plotter)plotterChoice.getSelectedItem();
	    if (plotter == heapPlotter) {
		r = heapRect;
	    } else if (plotter == nonHeapPlotter) {
		r = nonHeapRect;
	    } else if (plotter instanceof PoolPlotter) {
		r = getBarRect((PoolPlotter)plotter);
	    }
	    if (r != null) {
		g.drawRect(r.x - 1, r.y - 1, r.width + 2, r.height + 2);
	    }
	}

	private Rectangle getBarRect(PoolPlotter poolPlotter) {
	    return new Rectangle(leftMargin + poolPlotter.barX,
				 height - bottomMargin - barHeight,
				 barWidth, barHeight);
	}

	public Dimension getPreferredSize() {
	    return new Dimension(nonHeapRect.x + nonHeapRect.width + rightMargin,
				 height);
	}

	public void mouseClicked(MouseEvent e) {
	    Plotter plotter = getPlotter(e);

	    if (plotter != null && plotter != plotterChoice.getSelectedItem()) {
		plotterChoice.setSelectedItem(plotter);
		repaint();
	    }
	}

	public String getToolTipText(MouseEvent e) {
	    Plotter plotter = getPlotter(e);

	    return (plotter != null) ? plotter.toString() : null;
	}

	private Plotter getPlotter(MouseEvent e) {
	    Point p = e.getPoint();
	    Plotter plotter = null;

	    if (heapRect.contains(p)) {
		plotter = heapPlotter;
	    } else if (nonHeapRect.contains(p)) {
		plotter = nonHeapPlotter;
	    } else {
		for (PoolPlotter poolPlotter : poolPlotters) {
		    if (getBarRect(poolPlotter).contains(p)) {
			plotter = poolPlotter;
			break;
		    }
		}
	    }
	    return plotter;
	}

	public void mousePressed(MouseEvent e) {}
	public void mouseReleased(MouseEvent e) {}
	public void mouseEntered(MouseEvent e) {}
	public void mouseExited(MouseEvent e) {}
    }
}
