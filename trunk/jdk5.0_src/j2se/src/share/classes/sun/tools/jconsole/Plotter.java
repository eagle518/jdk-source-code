/*
 * @(#)Plotter.java	1.22 04/06/11
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.jconsole;

import java.awt.*;
import java.awt.event.*;
import java.beans.*;

import java.text.*;
import java.util.*;

import javax.swing.*;
import javax.swing.border.*;
import javax.swing.event.*;

import static sun.tools.jconsole.Formatter.*;

public class Plotter extends JComponent implements ActionListener {

    static final String[] rangeNames = {
	Resources.getText(" 1 min"),
	Resources.getText(" 5 min"),
	Resources.getText("10 min"),
	Resources.getText("30 min"),
	Resources.getText(" 1 hour"),
	Resources.getText(" 2 hours"),
	Resources.getText(" 3 hours"),
	Resources.getText(" 6 hours"),
	Resources.getText("12 hours"),
	Resources.getText(" 1 day"),
	Resources.getText(" 7 days"),
	Resources.getText(" 1 month"),
	Resources.getText(" 3 months"),
	Resources.getText(" 6 months"),
	Resources.getText(" 1 year"),
	Resources.getText("All")
    };

    static final int[] rangeValues = {
	1,
	5,
	10,
	30,
	1 * 60,
	2 * 60,
	3 * 60,
	6 * 60,
	12 * 60,
	1 * 24 * 60,
	7 * 24 * 60,
	1 * 31 * 24 * 60,
	3 * 31 * 24 * 60,
	6 * 31 * 24 * 60,
	366 * 24 * 60,
	-1
    };
	    

    final static long SECOND = 1000;
    final static long MINUTE = 60 * SECOND;
    final static long HOUR   = 60 * MINUTE;
    final static long DAY    = 24 * HOUR;

    final static Color bgColor = new Color(250, 250, 250);
    final static Color defaultColor = Color.blue.darker();

    private ArrayList<Sequence> seqs = new ArrayList<Sequence>();
    private JPopupMenu popupMenu;
    private JRadioButtonMenuItem[] menuRBs;

    private int viewRange = -1;	// Minutes (value <= 0 means full range)
    private boolean unitsBytes;
    private Border border = null;
    private Rectangle r = new Rectangle(1, 1, 1, 1);
    private Font smallFont = null;

    // Initial margins, may be recalculated as needed
    private int topMargin = 10;
    private int bottomMargin = 45;
    private int leftMargin = 65;
    private int rightMargin = 70;

    public Plotter(boolean unitsBytes) {
	this.unitsBytes = unitsBytes;

	enableEvents(AWTEvent.MOUSE_EVENT_MASK);
    }

    public void createSequence(String key, String name, Color color, boolean isPlotted) {
	Sequence seq = getSequence(key);
	if (seq == null) {
	    seq = new Sequence(key);
	}
	seq.name = name;
	seq.color = (color != null) ? color : defaultColor;
	seq.isPlotted = isPlotted;

	seqs.add(seq);
    }

    public void setIsPlotted(String key, boolean isPlotted) {
	Sequence seq = getSequence(key);
	if (seq != null) {
	    seq.isPlotted = isPlotted;
	}
    }

    public void addValue(String key, long value) {
	addValue(key, System.currentTimeMillis(), value);
    }

    public void addValue(String key, long time, long value) {
	Sequence seq = getSequence(key);
	if (seq != null) {
	    seq.add(time, value);
	    repaint();
	}
    }

    private Sequence getSequence(String key) {
	for (Sequence seq : seqs) {
	    if (seq.key.equals(key)) {
		return seq;
	    }
	}
	return null;
    }

    /**
     * @return the displayed time range in minutes, or -1 for all data
     */
    public int getViewRange() {
	return viewRange;
    }

    /**
     * @param minutes the displayed time range in minutes, or -1 to diaplay all data
     */
    public void setViewRange(int minutes) {
	if (minutes != viewRange) {
	    int oldValue = viewRange;
	    viewRange = minutes;
	    /* Do not i18n this string */
	    firePropertyChange("viewRange", oldValue, viewRange);
	    if (popupMenu != null) {
		for (int i = 0; i < menuRBs.length; i++) {
		    if (rangeValues[i] == viewRange) {
			menuRBs[i].setSelected(true);
			break;
		    }
		}
	    }
	    repaint();
	}
    }

    public JPopupMenu getComponentPopupMenu() {
	if (popupMenu == null) {
	    popupMenu = new JPopupMenu(Resources.getText("Time Range:"));
	    menuRBs = new JRadioButtonMenuItem[rangeNames.length];
	    ButtonGroup rbGroup = new ButtonGroup();
	    for (int i = 0; i < rangeNames.length; i++) {
		menuRBs[i] = new JRadioButtonMenuItem(rangeNames[i]);
		rbGroup.add(menuRBs[i]);
		menuRBs[i].addActionListener(this);
		if (viewRange == rangeValues[i]) {
		    menuRBs[i].setSelected(true);
		}
		popupMenu.add(menuRBs[i]);
	    }
	}
	return popupMenu;
    }

    public void actionPerformed(ActionEvent ev) {
	int index = popupMenu.getComponentIndex((JComponent)ev.getSource());
	setViewRange(rangeValues[index]);
    }

    public void paintComponent(Graphics g) {
	super.paintComponent(g);

	Container parent = getParent();
	if (parent instanceof BorderedComponent) {
	    JButton moreOrLessButton = ((BorderedComponent)parent).moreOrLessButton;
	    if (moreOrLessButton != null) {
		rightMargin = Math.max(rightMargin, moreOrLessButton.getWidth());
	    }
	}


	Color oldColor = g.getColor();
	Font  oldFont  = g.getFont();

	((Graphics2D)g).setRenderingHint(RenderingHints.KEY_ANTIALIASING,
					 RenderingHints.VALUE_ANTIALIAS_ON);

	if (smallFont == null) {
	    smallFont = oldFont.deriveFont(9.0F);
	}

	r.x = leftMargin - 5;
	r.y = topMargin  - 8;
	r.width  = getWidth()-leftMargin-rightMargin;
	r.height = getHeight()-topMargin-bottomMargin+16;

	if (border == null) {
	    // By setting colors here, we avoid recalculating them
	    // over and over.
	    border = new BevelBorder(BevelBorder.LOWERED,
				     getBackground().brighter().brighter(),
				     getBackground().brighter(),
				     getBackground().darker().darker(),
				     getBackground().darker());
	}

	border.paintBorder(this, g, r.x, r.y, r.width, r.height);

	// Fill background color
	g.setColor(bgColor);
	g.fillRect(r.x+2, r.y+2, r.width-4, r.height-4);
	g.setColor(oldColor);

	long tMin = Long.MAX_VALUE;
	long tMax = Long.MIN_VALUE;
	long vMin = Long.MAX_VALUE;
	long vMax = 1;

	int w = getWidth()-rightMargin-leftMargin-10;
	int h = getHeight()-topMargin-bottomMargin;

	for (Sequence seq : seqs) {
	    if (seq.size > 1) {
		tMin = Math.min(tMin, seq.times[0]);
		tMax = Math.max(tMax, seq.times[seq.size-1]);
	    }
	}

	long viewRangeMS;
	if (viewRange > 0) {
	    viewRangeMS = viewRange * MINUTE;
	} else {
	    viewRangeMS = tMax - tMin;
	}

	// Calculate min/max values
	for (Sequence seq : seqs) {
	    if (seq.size > 0) {
		for (int i = 0; i < seq.size; i++) {
		    if (seq.size == 1 || seq.times[i] >= tMax - viewRangeMS) {
			vMax = Math.max(vMax, seq.values[i]);
			vMin = Math.min(vMin, seq.values[i]);
		    }
		}
	    } else {
		vMin = 0L;
	    }
	    if (unitsBytes || !seq.isPlotted) {
		// We'll scale only to the first (main) value set.
		// TODO: Use a separate property for this.
		break;
	    }
	}

	// Normalize scale
	vMax = normalizeMax(vMax);
	if (vMin > 0) {
	    if (vMax / vMin > 4) {
		vMin = 0;
	    } else {
		vMin = normalizeMin(vMin);
	    }
	}


	g.setColor(Color.black);

	// Axes
	// Draw vertical axis
	int x = leftMargin - 18;
	int y = topMargin;
	FontMetrics fm = g.getFontMetrics();

	// Draw min/max value strings
	{
	    String sMin = getSizeString(vMin, vMax);
	    String sMax = getSizeString(vMax, vMax);

	    int xMin = x - 6 - fm.stringWidth(sMin);
	    int xMax = x - 6 - fm.stringWidth(sMax);

	    if (checkLeftMargin(xMin) || checkLeftMargin(xMax)) {
		// Wait for next repaint
		return;
	    }
	    g.drawString(sMax, xMax, y+4);
	    g.drawString(sMin, xMin, y+h+4);
	}

	g.drawLine(x,   y,   x,   y+h);
	g.drawLine(x-5, y,   x+5, y);
	g.drawLine(x-5, y+h, x+5, y+h);

	int n = 5;
	if ((""+vMax).startsWith("2")) {
	    n = 4;
	} else if ((""+vMax).startsWith("3")) {
	    n = 6;
	} else if ((""+vMax).startsWith("4")) {
	    n = 4;
	} else if ((""+vMax).startsWith("6")) {
	    n = 6;
	} else if ((""+vMax).startsWith("7")) {
	    n = 7;
	} else if ((""+vMax).startsWith("8")) {
	    n = 8;
	} else if ((""+vMax).startsWith("9")) {
	    n = 3;
	}
	// Ticks
	for (int i = 0; i < n; i++) {
	    long v = i * vMax / n;
	    if (v > vMin) {
		y = topMargin+h-(int)(h * (v-vMin) / (vMax-vMin));
		g.drawLine(x-2, y, x+2, y);
		String s = getSizeString(v, vMax);
		int sx = x-6-fm.stringWidth(s);
		if (y < topMargin+h-13) {
		    if (checkLeftMargin(sx)) {
			// Wait for next repaint
			return;
		    }
		    g.drawString(s, sx, y+4);
		}
		// Draw horizontal grid line
		g.setColor(Color.lightGray);
		g.drawLine(r.x + 4, y, r.x + r.width - 4, y);
		g.setColor(Color.black);
	    }
	}

	// Draw horizontal axis
	x = leftMargin;
	y = topMargin + h + 15;
	g.drawLine(x,   y,   x+w, y);

	if (tMax > 0) {
	    long tz = timeDF.getTimeZone().getOffset(tMax);
	    long tickInterval = calculateTickInterval(w, 40, viewRangeMS);
	    if (tickInterval > 3 * HOUR) {
		tickInterval = calculateTickInterval(w, 80, viewRangeMS);
	    }
	    long t0 = tickInterval - (tMax - viewRangeMS + tz) % tickInterval;
	    while (t0 < viewRangeMS) {
		x = leftMargin + (int)(w * t0 / viewRangeMS);
		g.drawLine(x, y-2, x, y+2);

		long t = tMax-viewRangeMS+t0;
		String str = formatClockTime(t);
		g.drawString(str, x, y+16);
		//if (tickInterval > (1 * HOUR) && t % (1 * DAY) == 0) {
		if ((t + tz) % (1 * DAY) == 0) {
		    str = formatDate(t);
		    g.drawString(str, x, y+27);
		}
		// Draw vertical grid line
		g.setColor(Color.lightGray);
		g.drawLine(x, topMargin, x, topMargin + h);
		g.setColor(Color.black);
		t0 += tickInterval;
	    }
	}

	// Plot values
	int start = 0;
	int nValues = 0;
	int nLists = seqs.size();
	if (nLists > 0) {
	    nValues = seqs.get(0).size;
	}
	if (nValues == 0) {
	    g.setColor(oldColor);
	    return;
	} else {
	    Sequence seq = seqs.get(0);
	    // Find starting point
	    for (int p = 0; p < seq.size; p++) {
		if (seq.times[p] >= tMax - viewRangeMS) {
		    start = p;
		    break;
		}
	    }
	}

	//Optimization: collapse plot of more than four values per pixel
	int pointsPerPixel = (nValues - start) / w;
	if (pointsPerPixel < 4) {
	    pointsPerPixel = 1;
	}

	// Draw graphs
	// Loop backwards over sequences because the first needs to be painted on top
	for (int i = nLists-1; i >= 0; i--) {
	    int x0 = leftMargin;
	    int y0 = topMargin + h + 1;

	    Sequence seq = seqs.get(i);
	    if (seq.isPlotted && seq.size > 0) {
		// Paint twice, with white and with color
		for (int pass = 0; pass < 2; pass++) {
		    g.setColor((pass == 0) ? Color.white : seq.color);
		    int x1 = -1;
		    long v1 = -1;
		    for (int p = start; p < nValues; p += pointsPerPixel) {
			// Make sure we get the last value
			if (pointsPerPixel > 1 && p >= nValues - pointsPerPixel) {
			    p = nValues - 1;
			}
			int x2 = (viewRangeMS == 0L) ? 0 : (int)(w * (seq.times[p]-(tMax-viewRangeMS)) / viewRangeMS);
			long v2 = seq.values[p];
			if (v2 >= vMin && v2 <= vMax) {
			    int y2  = (int)(h * (v2 -vMin) / (vMax-vMin));
			    if (x1 >= 0 && v1 >= vMin && v1 <= vMax) {
				int y1 = (int)(h * (v1-vMin) / (vMax-vMin));

				if (y1 == y2) {
				    // fillrect is much faster
				    g.fillRect(x0+x1, y0-y1-pass, x2-x1, 1);
				} else {
				    g.drawLine(x0+x1, y0-y1-pass, x0+x2, y0-y2-pass);
				}
			    } else if (seq.size == 1) {
				// First value, draw a horizontal line
				g.fillRect(x0, y0-y2-pass, w, 1);
			    }
			}
			x1 = x2;
			v1 = v2;
		    }
		}

		// Current value
		long v = seq.values[seq.size - 1];
		if (v >= vMin && v <= vMax) {
		    g.setColor(seq.color);
		    x = r.x + r.width + 2;
		    y = topMargin+h-(int)(h * (v-vMin) / (vMax-vMin));
		    // a small triangle/arrow
		    g.fillPolygon(new int[] { x+2, x+6, x+6 },
				  new int[] { y,   y+3, y-3 },
				  3);
		}
		g.setColor(Color.black);
	    }
	}

	int[] valueStringSlots = new int[nLists];
	for (int i = 0; i < nLists; i++) valueStringSlots[i] = -1;
	for (int i = 0; i < nLists; i++) {
	    Sequence seq = seqs.get(i);
	    if (seq.isPlotted && seq.size > 0) {
		// Draw current value

		// TODO: collapse values if pointsPerPixel >= 4

		long v = seq.values[seq.size - 1];
		if (v >= vMin && v <= vMax) {
		    x = r.x + r.width + 2;
		    y = topMargin+h-(int)(h * (v-vMin) / (vMax-vMin));
		    int y2 = getValueStringSlot(valueStringSlots, y, 2*10, i);
		    g.setFont(smallFont);
		    g.setColor(seq.color);
		    String curValue = String.format("%,d", v);
		    int valWidth = fm.stringWidth(curValue);
		    String legend = seq.name;
		    int legendWidth = fm.stringWidth(legend);
		    if (checkRightMargin(valWidth) || checkRightMargin(legendWidth)) {
			// Wait for next repaint
			return;
		    }
		    g.drawString(legend  , x + 17, Math.min(topMargin+h,      y2 + 3 - 10));
		    g.drawString(curValue, x + 17, Math.min(topMargin+h + 10, y2 + 3));

		    // Maybe draw a short line to value
		    if (y2 > y + 3) {
			g.drawLine(x + 9, y + 2, x + 14, y2);
		    } else if (y2 < y - 3) {
			g.drawLine(x + 9, y - 2, x + 14, y2);
		    }
		}
		g.setFont(oldFont);
		g.setColor(Color.black);

	    }
	}
	g.setColor(oldColor);
    }

    private boolean checkLeftMargin(int x) {
	// Make sure leftMargin has at least 2 pixels over
	if (x < 2) {
	    leftMargin += (2 - x);
	    repaint();
	    return true;
	}
	return false;
    }

    private boolean checkRightMargin(int w) {
	// Make sure rightMargin has at least 2 pixels over
	if (w + 2 > rightMargin) {
	    rightMargin = (w + 2);
	    repaint();
	    return true;
	}
	return false;
    }

    private int getValueStringSlot(int[] slots, int y, int h, int i) {
	for (int s = 0; s < slots.length; s++) {
	    if (slots[s] >= y && slots[s] < y + h) {
		// collide below us
		if (slots[s] > h) {
		    return getValueStringSlot(slots, slots[s]-h, h, i);
		} else {
		    return getValueStringSlot(slots, slots[s]+h, h, i);
		}
	    } else if (y >= h && slots[s] > y - h && slots[s] < y) {
		// collide above us
		return getValueStringSlot(slots, slots[s]+h, h, i);
	    }
	}
	slots[i] = y;
	return y;
    }

    private long calculateTickInterval(int w, int hGap, long viewRangeMS) {
	long tickInterval = viewRangeMS * hGap / w;
	if (tickInterval < 1 * MINUTE) {
	    tickInterval = 1 * MINUTE;
	} else if (tickInterval < 5 * MINUTE) {
	    tickInterval = 5 * MINUTE;
	} else if (tickInterval < 10 * MINUTE) {
	    tickInterval = 10 * MINUTE;
	} else if (tickInterval < 30 * MINUTE) {
	    tickInterval = 30 * MINUTE;
	} else if (tickInterval < 1 * HOUR) {
	    tickInterval = 1 * HOUR;
	} else if (tickInterval < 3 * HOUR) {
	    tickInterval = 3 * HOUR;
	} else if (tickInterval < 6 * HOUR) {
	    tickInterval = 6 * HOUR;
	} else if (tickInterval < 12 * HOUR) {
	    tickInterval = 12 * HOUR;
	} else if (tickInterval < 1 * DAY) {
	    tickInterval = 1 * DAY;
	} else {
	    tickInterval = normalizeMax(tickInterval / DAY) * DAY;
	}
	return tickInterval;
    }

    private long normalizeMin(long l) {
	int exp = (int)Math.log10((double)l);
	long multiple = (long)Math.pow(10.0, exp);
	int i = (int)(l / multiple);
	return i * multiple;
    }

    private long normalizeMax(long l) {
	int exp = (int)Math.log10((double)l);
	long multiple = (long)Math.pow(10.0, exp);
	int i = (int)(l / multiple);
	l = (i+1)*multiple;
// 	switch (i) {
// 	  case 1: l =  2 * multiple; break;
// 	  case 2: l =  5 * multiple; break;
// 	  case 3: l =  5 * multiple; break;
// 	  case 4: l =  5 * multiple; break;
// 	  case 5: l = 10 * multiple; break;
// 	  case 6: l = 10 * multiple; break;
// 	  case 7: l = 10 * multiple; break;
// 	  case 8: l = 10 * multiple; break;
// 	  case 9: l = 10 * multiple; break;
// 	}
	return l;
    }


    /*
     * Return the input value rounded to one decimal place.  If after
     * rounding the string ends in the (locale-specific) decimal point
     * followed by a zero then trim that off as well.
     */
    private static final String decimalZero =
        new java.text.DecimalFormatSymbols().getDecimalSeparator() + "0";
    private String trimDouble(double d) {
        String s = String.format("%.1f", d);
        if (s.length() > 3 && s.endsWith(decimalZero)) {
            s = s.substring(0, s.length()-2);
        }
        return s;
    }

    private String getSizeString(long v, long vMax) {
	String s;

	if (unitsBytes) {
	    int exp = (int)Math.log10((double)vMax);

	    if (exp < 3) {
		s = Resources.getText("Size Bytes", v);
	    } else if (exp < 6) {
                s = Resources.getText("Size Kb", trimDouble(v / Math.pow(10.0, 3)));
	    } else if (exp < 9) {
		s = Resources.getText("Size Mb", trimDouble(v / Math.pow(10.0, 6)));
	    } else {
		s = Resources.getText("Size Gb", trimDouble(v / Math.pow(10.0, 9)));
	    }
	} else {
	    s = String.format("%,d", v);
	}
	return s;
    }

    private static long[] getLongArray(long[] a1) {
	if (a1 == null) {
	    return new long[1024];
	} else {
	    long[] a2 = new long[a1.length * 2];
	    System.arraycopy(a1, 0, a2, 0, a1.length);
	    return a2;
	}
    }

    private static class Sequence {
	String key;
	String name;
	Color color;
	boolean isPlotted;

	long[] times;
	long[] values;
	int size = 0;

	Sequence(String key) {
	    this.key = key;
	    times  = getLongArray(null);
	    values = getLongArray(null);
	}

	private void add(long time, long value) {
	    if (times.length == size) {
		times  = getLongArray(times);
		values = getLongArray(values);
	    }
	    times[size]  = time;
	    values[size] = value;
	    size++;
	}
    }

    // Can be overridden by subclasses
    long getValue() {
	return 0;
    }

    long getLastTimeStamp() {
	Sequence seq = seqs.get(0);
	return seq.times[seq.size - 1];
    }

    long getLastValue(String key) {
	Sequence seq = getSequence(key);
	return (seq != null && seq.size > 0) ? seq.values[seq.size - 1] : 0L;
    }
}
