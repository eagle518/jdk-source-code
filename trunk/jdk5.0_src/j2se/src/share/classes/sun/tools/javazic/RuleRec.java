/*
 * @(#)RuleRec.java	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.javazic;

import java.util.ArrayList;
import java.util.StringTokenizer;

/**
 * RuleRec class represents one record of the Rule set.
 *
 * @since 1.4
 */
class RuleRec {
    private int fromYear;
    private int toYear;
    private String type;
    private int inMonth;
    private RuleDay onDay;
    private Time atTime;
    private int save;
    private String letters;
    private String line;
    private boolean isLastRule;

    int getFromYear() {
	return fromYear;
    }

    int getToYear() {
	return toYear;
    }

    int getMonthNum() {
	return inMonth;
    }

    RuleDay getDay() {
	return onDay;
    }

    Time getTime() {
	return atTime;
    }

    int getSave() {
	return save;
    }

    String getLine() {
	return line;
    }

    /**
     * Sets the line from the text file.
     * @param line the text of the line
     */
    void setLine(String line) {
	this.line = line;
    }

    /**
     * @return true if the rule type is "odd".
     */
    boolean isOdd() {
	return "odd".equals(type);
    }

    /**
     * @return true if the rule type is "even".
     */
    boolean isEven() {
	return "even".equals(type);
    }

    /**
     * Determines if this rule record is the last DST schedule rule.
     *
     * @return true if this rule record has "max" as TO (year).
     */
    boolean isLastRule() {
	return isLastRule;
    }

    /**
     * Determines if the unadjusted until time of the specified ZoneRec
     * is the same as the transition time of this rule in the same
     * year as the ZoneRec until year.
     *
     * @param zrec ZoneRec to compare to
     * @param save the amount of daylight saving in milliseconds
     * @param gmtOffset the GMT offset value in milliseconds
     * @return true if the unadjusted until time is the same as rule's
     * transition time.
     */
    boolean isSameTransition(ZoneRec zrec, int save, int gmtOffset) {
	long	until, transition;

	if (zrec.getUntilTime().getType() != atTime.getType()) {
	    until = zrec.getLocalUntilTime(save, gmtOffset);
	    transition = Time.getLocalTime(zrec.getUntilYear(),
					   getMonthNum(),
					   getDay(),
					   save,
					   gmtOffset,
					   atTime);
	} else {
	    until = zrec.getLocalUntilTime();
	    transition = Time.getLocalTime(zrec.getUntilYear(),
					   getMonthNum(),
					   getDay(),
					   atTime.getTime());
	}

	return until == transition;
    }

    /**
     * Parses a Rule line and returns a RuleRec object.
     *
     * @param tokens a StringTokenizer object that should contain a
     * token for the "FROM" field and the rest.
     * @return a RuleRec object.
     */
    static RuleRec parse(StringTokenizer tokens) {
	RuleRec rec = new RuleRec();
	try {
	    // FROM
	    String token = tokens.nextToken();
	    try {
		rec.fromYear = Integer.parseInt(token);
	    } catch (NumberFormatException e) {
		// it's not integer
		if ("min".equals(token) || "minimum".equals(token)) {
		    rec.fromYear = Zoneinfo.getMinYear();
		} else if ("max".equals(token) || "maximum".equals(token)) {
		    rec.fromYear = Zoneinfo.getMaxYear();
		} else {
		    Main.panic("invalid year value: "+token);
		}
	    }

	    // TO
	    token = tokens.nextToken();
	    rec.isLastRule = false;
	    try {
		rec.toYear = Integer.parseInt(token);
	    } catch (NumberFormatException e) {
		// it's not integer
		if ("min".equals(token) || "minimum".equals(token)) {
		    rec.fromYear = Zoneinfo.getMinYear();
		} else if ("max".equals(token) || "maximum".equals(token)) {
		    rec.toYear = Integer.MAX_VALUE;
		    rec.isLastRule = true;
		} else if ("only".equals(token)) {
		    rec.toYear = rec.fromYear;
		} else {
		    Main.panic("invalid year value: "+token);
		}
	    }

	    // TYPE
	    rec.type = tokens.nextToken();

	    // IN
	    rec.inMonth = Month.parse(tokens.nextToken());

	    // ON
	    rec.onDay = RuleDay.parse(tokens.nextToken());

	    // AT
	    rec.atTime = Time.parse(tokens.nextToken());

	    // SAVE
	    rec.save = (int) Time.parse(tokens.nextToken()).getTime();

	    // LETTER/S
	    rec.letters = tokens.nextToken();
	} catch (Exception e) {
	    e.printStackTrace();
	}
	return rec;
    }

    /**
     * Calculates the transition time of the given year under this rule.
     * @param year the year value
     * @param gmtOffset the GMT offset value in milliseconds
     * @param save the amount of daylight save time
     * @return the transition time in milliseconds of the given year in UTC.
     */
    long getTransitionTime(int year, int gmtOffset, int save) {
	long time = Time.getLocalTime(year, getMonthNum(),
				      getDay(), atTime.getTime());
	if (atTime.isSTD()) {
	    time -= gmtOffset;
	} else if (atTime.isWall()) {
	    time -= gmtOffset + save;
	}
	return time;
    }	

    private static int getInt(StringTokenizer tokens) {
	String token = tokens.nextToken();
	return Integer.parseInt(token);
    }
}
