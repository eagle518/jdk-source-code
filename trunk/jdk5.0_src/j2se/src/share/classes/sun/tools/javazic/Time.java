/*
 * @(#)Time.java	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 * 
 */

package sun.tools.javazic;

import java.util.ArrayList;
import sun.util.calendar.CalendarDate;
import sun.util.calendar.CalendarSystem;
import sun.util.calendar.Gregorian;

/**
 * Time class represents the "AT" field and other time related information.
 *
 * @since 1.4
 */
class Time {

    static final Gregorian gcal = CalendarSystem.getGregorianCalendar();

    // type is wall clock time
    private static final int WALL = 1;

    // type is standard time
    private static final int STD = 2;

    // type is UTC
    private static final int UTC = 3;

    // type of representing time
    private int type;

    /**
     * Time from the EPOCH in milliseconds
     */
    private long time;

    /**
     * Current time in milliseconds
     */
    private static long currentTime = System.currentTimeMillis();

    Time() {
	time = 0L;
    }

    Time(long time) {
	this.time = time;
    }

    void setType(int type) {
	this.type = type;
    }

    long getTime() {
	return time;
    }

    int getType() {
	return type;
    }

    static long getCurrentTime() {
	return currentTime;
    }

    /**
     * @return true if the time is represented in wall-clock time.
     */
    boolean isWall() {
	return type == WALL;
    }

    /**
     * @return true if the time is represented in standard time.
     */
    boolean isSTD() {
	return type == STD;
    }

    /**
     * @return true if the time is represented in UTC time.
     */
    boolean isUTC() {
	return type == UTC;
    }

    /**
     * Converts the type to a string that represents the type in the
     * SimpleTimeZone time mode. (e.g., "SimpleTimeZone.WALL_TIME").
     * @return the converted string or null if the type is undefined.
     */
    String getTypeForSimpleTimeZone() {
	String	stz = "SimpleTimeZone.";
	if (isWall()) {
	    return stz+"WALL_TIME";
	}
	else if (isSTD()) {
	    return stz+"STANDARD_TIME";
	}
	else if (isUTC()) {
	    return stz+"UTC_TIME";
	}
	else {
	    return null;
	}
    }

    /**
     * Converts the given Gregorian calendar field values to local time.
     * Local time is represented by the amount of milliseconds from
     * January 1, 1970 0:00 GMT.
     * @param year the year value
     * @param month the month value
     * @param day the day represented by {@link RuleDay}
     * @param save the amount of daylight time in milliseconds
     * @param gmtOffset the GMT offset in milliseconds
     * @param time the time of the day represented by {@link Time}
     * @return local time
     */
    static long getLocalTime(int year, int month, RuleDay day, int save,
			     int gmtOffset, Time time) {
	long	t = time.getTime();

	if (time.isSTD())
	    t = time.getTime() + save;
	else if (time.isUTC())
	    t = time.getTime() + save + gmtOffset;

	return getLocalTime(year, month, day, t);
    }

    /**
     * Converts the given Gregorian calendar field values to local time.
     * Local time is represented by the amount of milliseconds from
     * January 1, 1970 0:00 GMT.
     * @param year the year value
     * @param month the month value
     * @param day the day value
     * @param time the time of the day in milliseconds
     * @return local time
     */
    static long getLocalTime(int year, int month, int day, long time) {
	CalendarDate date = gcal.newCalendarDate(null);
	date.setDate(year, month, day);
	long millis = gcal.getTime(date);
	return millis + time;
    }

    /**
     * Equivalent to <code>getLocalTime(year, month, day, (long)time)</code>.
     * @param year the year value
     * @param month the month value
     * @param day the day value
     * @param time the time of the day in milliseconds
     * @return local time
     */
    static long getLocalTime(int year, int month, int day, int time) {
	return getLocalTime(year, month, day, (long)time);
    }

    /**
     * Equivalent to {@link #getLocalTime(int, int, RuleDay, int)
     * getLocalTime(year, month, day, (int) time)}.
     * @param year the year value
     * @param month the month value
     * @param day the day represented by {@link RuleDay}
     * @param time the time of the day represented by {@link Time}
     * @return local time
     */
    static long getLocalTime(int year, int month, RuleDay day, long time) {
	return getLocalTime(year, month, day, (int) time);
    }

    /**
     * Converts the given Gregorian calendar field values to local time.
     * Local time is represented by the amount of milliseconds from
     * January 1, 1970 0:00 GMT.
     * @param year the year value
     * @param month the month value
     * @param day the day represented by {@link RuleDay}
     * @param time the time of the day represented by {@link Time}
     * @return local time
     */
    static long getLocalTime(int year, int month, RuleDay day, int time) {
	CalendarDate cdate = gcal.newCalendarDate(null);

	if (day.isLast()) {	// e.g., "lastSun"
	    cdate.setDate(year, month, 1);
	    cdate.setDayOfMonth(gcal.getMonthLength(cdate));
	    cdate = gcal.getNthDayOfWeek(-1, day.getDayOfWeek(), cdate);
	} else if (day.isLater()) { // e.g., "Sun>=1"
	    cdate.setDate(year, month, day.getDay());
	    cdate = gcal.getNthDayOfWeek(1, day.getDayOfWeek(), cdate);
	} else if (day.isExact()) {
	    cdate.setDate(year, month, day.getDay());
	} else if (day.isEarlier()) {	// e.g., "Sun<=15"
	    cdate.setDate(year, month, day.getDay());
	    cdate = gcal.getNthDayOfWeek(-1, day.getDayOfWeek(), cdate);
	} else {
	    Main.panic("invalid day type: " + day);
	}
	return gcal.getTime(cdate) + time;
    }

    /**
     * Parses the given "AT" field and constructs a Time object.
     * @param the "AT" field string
     * @return the Time object
     */
    static Time parse(String time) {
	int sign;
	int index = 0;
	Time tm;

	if (time.charAt(0) == '-') {
	    sign = -1;
	    index++;
	} else {
	    sign = 1;
	}
	int val = 0;
	int num = 0;
	int countDelim = 0;
	while (index < time.length()) {
	    char c = time.charAt(index++);
	    if (c == ':') {
		val = val * 60 + num;
		countDelim++;
		num = 0;
		continue;
	    }
	    int d = Character.digit(c, 10);
	    if (d == -1) {
		--index;
		break;
	    }
	    num = num * 10 + d;
	}
	val = val * 60 + num;
	// convert val to second
	for (; countDelim < 2; countDelim++) {
	    val *= 60;
	}
	tm = new Time((long)val * 1000 * sign);
	if (index < time.length()) {
	    char c = time.charAt(index++);
	    if (c == 's') {
		tm.setType(tm.STD);
	    } else if (c == 'u' || c == 'g' || c == 'z') {
		tm.setType(tm.UTC);
	    } else if (c == 'w') {
		tm.setType(tm.WALL);
	    } else {
		Main.panic("unknown time mode: "+c);
	    }
	} else {
	    tm.setType(tm.WALL);
	}
	return tm;
    }

    /**
     * Converts the given milliseconds string to a "GMT[+-]hh:mm" string.
     * @param ms the milliseconds string
     */
    static String toGMTFormat(String ms) {
	long	l = Long.parseLong(ms) / 1000;
	String	s;

	if (l < 0) {
	    s = "-";
	    l = -l;
	} 
	else {
	    s = "+";
	} 

	if (l < 36000) {
	    s += "0";
	}

	/* Get hour */
	s += l / 3600; 
	l %= 3600;
	
	s += ":";

	/* Get minute */
	if (l < 600) {
	    s += "0"; 
	}

	s += l / 60;

	return s;
    }

    /**
     * Converts the given millisecond value to a string for a
     * SimpleTimeZone parameter.
     * @param ms the millisecond value
     * @return the string in a human readable form
     */
    static String toFormedString(int ms) {
	StringBuffer s = new StringBuffer();
	boolean minus = false;

	if (ms < 0) {
	    s.append("-");
	    minus = true;
	    ms = -ms;
	} else if (ms == 0) {
	    return "0";
	}

	int hour = ms / (60 * 60 * 1000);
	ms %= (60 * 60 * 1000);
	int minute = ms / (60 * 1000);

	if (hour != 0) {
	    if (minus && minute != 0) {
		s.append("(");
	    }
	    s.append(Integer.toString(hour) + "*ONE_HOUR");
	}

	if (minute != 0) {
	    if (hour != 0) {
		s.append("+");
	    }
	    s.append(Integer.toString(minute) + "*ONE_MINUTE");
	    if (minus && hour != 0) {
		s.append(")");
	    }
	}

	return s.toString();
    }
}
