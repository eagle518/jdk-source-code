/*
 * @(#)Formatter.java	1.6 04/06/07
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.jconsole;

import java.text.*;

class Formatter {
    final static long SECOND = 1000;
    final static long MINUTE = 60 * SECOND;
    final static long HOUR   = 60 * MINUTE;
    final static long DAY    = 24 * HOUR;

    final static String cr = System.getProperty("line.separator");

    final static DateFormat timeDF            = new SimpleDateFormat("HH:mm");
    private final static DateFormat timeWithSecondsDF = new SimpleDateFormat("HH:mm:ss");
    private final static DateFormat dateDF            = new SimpleDateFormat("yyyy-MM-dd");

    static String formatTime(long t) {
        String str;
        if (t < 1 * MINUTE) {
            String seconds = String.format("%.3f", t / (double)SECOND);
            str = Resources.getText("DurationSeconds", seconds);
        } else {
            long remaining = t;
            long days = remaining / DAY;
            remaining %= 1 * DAY;
            long hours = remaining / HOUR;
            remaining %= 1 * HOUR;
            long minutes = remaining / MINUTE;

            if (t >= 1 * DAY) {
                str = Resources.getText("DurationDaysHoursMinutes",
                                        days, hours, minutes);
            } else if (t >= 1 * HOUR) {
                str = Resources.getText("DurationHoursMinutes",
                                        hours, minutes);
            } else {
                str = Resources.getText("DurationMinutes", minutes);
            }
        }
        return str;
    }

    static String formatNanoTime(long t) {
        long ms = t / 1000000;
        return formatTime(ms);
    }


    static String formatClockTime(long time) {
	return timeDF.format(time);
    }

    static String formatDate(long time) {
	return dateDF.format(time);
    }

    static String formatDateTime(long time) {
	return dateDF.format(time) + " " + timeWithSecondsDF.format(time);
    }

    static String formatKBytes(long bytes) {
        if (bytes == -1) {
            return "-1" + Resources.getText(" kbytes");
        }

        long kb = bytes / 1024;
        return justify(kb, 10) + Resources.getText(" kbytes");
    }

    // A poor attempt at right-justifying for numerical data
    static String justify(long value, int size) {
        String str = String.format("%,d",value);
        return justify(str, size);
    }

    static String justify(String str, int size) {
        StringBuffer buf = new StringBuffer();
	buf.append("<TT>");
        int n = size - str.length();
        for (int i = 0; i < n; i++) {
            buf.append("&nbsp;");
        }
        buf.append(str);
	buf.append("</TT>");
        return buf.toString();
    }

    static String newRow(String label, String value) {
        return newRow(label, value, 2);
    }

    static String newRowWith4Columns(String label, String value) {
        return newRow(label, value, 4);
    }

    private static String newRow(String label, String value, int columnPerRow) {
        if (label == null) {
            label = "<td nowrap align=right>";
        } else {
            label = "<td nowrap align=right><b>" + label + ": </b>";
        }
        value = "<td colspan=" + (columnPerRow-1) + "> <font size =-1>" + value;

        return "<tr>" + label + value + "</tr>";
    }

    static String newRow(String label1, String value1,
                         String label2, String value2) {
        label1 = "<td align=right><b>" + label1 + ": </b>";
        value1 = "<td><font size =-1>" + value1;
        label2 = "<td align=right><b>" + label2 + ": </b>";
        value2 = "<td><font size =-1>" + value2;

        return "<tr>" + label1 + value1 + label2 + value2;
    }

}
