/*
 * @(#)Month.java	1.8 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.javazic;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * Month enum handles month related manipulation.
 *
 * @since 1.4
 */
enum Month {
    JANUARY("Jan"),
    FEBRUARY("Feb"),
    MARCH("Mar"),
    APRIL("Apr"),
    MAY("May"),
    JUNE("Jun"),
    JULY("Jul"),
    AUGUST("Aug"),
    SEPTEMBER("Sep"),
    OCTOBER("Oct"),
    NOVEMBER("Nov"),
    DECEMBER("Dec");

    private final String abbr;

    private static final Map<String,Month> abbreviations
				= new HashMap<String,Month>(12);

    static {
	for (Month m : Month.values()) {
	    abbreviations.put(m.abbr, m);
	}
    }

    private Month(String abbr) {
	this.abbr = abbr;
    }

    int value() {
	return ordinal() + 1;
    }

    /**
     * Parses the specified string as a month abbreviation.
     * @param name the month abbreviation
     * @return the Month value
     */
    static Month parse(String name) {
	Month m = abbreviations.get(name);
	if (m != null) {
	    return m;
	}
	return null;
    }

    /**
     * @param month the nunmth number (1-based)
     * @return the month name in uppercase of the specified month
     */
    static String toString(int month) {
	if (month >= JANUARY.value() && month <= DECEMBER.value()) {
	    return "Calendar." + Month.values()[month - 1];
	}
	throw new IllegalArgumentException("wrong month number: " + month);
    }
}
