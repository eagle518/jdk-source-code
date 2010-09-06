/*
 * @(#)Gregorian.java	1.5 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.util.calendar;

import java.util.TimeZone;

/**
 * Gregorian calendar implementation.
 *
 * @author Masayoshi Okutsu
 * @since 1.5
 */

public class Gregorian extends BaseCalendar {

    static class Date extends BaseCalendar.Date {
	protected Date() {
	    super();
	}

	protected Date(TimeZone zone) {
	    super(zone);
	}

	public int getNormalizedYear() {
	    return getYear();
	}

	public void setNormalizedYear(int normalizedYear) {
	    setYear(normalizedYear);
	}
    }

    Gregorian() {
    }

    public String getName() {
	return "gregorian";
    }

    public CalendarDate newCalendarDate() {
	return new Date();
    }

    public CalendarDate newCalendarDate(TimeZone zone) {
	return new Date(zone);
    }
}
