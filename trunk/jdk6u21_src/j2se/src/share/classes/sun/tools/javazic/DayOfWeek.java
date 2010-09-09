/*
 * @(#)DayOfWeek.java	1.4 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.javazic;

/**
 * Day of week enum.
 *
 * @since 1.6
 */

enum DayOfWeek {
    SUNDAY("Sun"),
    MONDAY("Mon"),
    TUESDAY("Tue"),
    WEDNESDAY("Wed"),
    THURSDAY("Thu"),
    FRIDAY("Fri"),
    SATURDAY("Sat");

    private final String abbr;

    private DayOfWeek(String abbr) {
	this.abbr = abbr;
    }

    String getAbbr() {
	return abbr;
    }

    int value() {
	return ordinal() + 1;
    }
}

