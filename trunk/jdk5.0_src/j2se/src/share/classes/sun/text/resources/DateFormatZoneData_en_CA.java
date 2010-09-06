/*
 * @(#)DateFormatZoneData_en_CA.java	1.19 03/12/19
 */

/*
 * Portions Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * (C) Copyright Taligent, Inc. 1996, 1997 - All Rights Reserved
 * (C) Copyright IBM Corp. 1996 - 1998 - All Rights Reserved
 *
 * The original version of this source code and documentation
 * is copyrighted and owned by Taligent, Inc., a wholly-owned
 * subsidiary of IBM. These materials are provided under terms
 * of a License Agreement between Taligent and Sun. This technology
 * is protected by multiple US and International patents.
 *
 * This notice and attribution to Taligent may not be removed.
 * Taligent is a registered trademark of Taligent, Inc.
 *
 */

package sun.text.resources;

/**
 * Supplement package private date-time formatting zone data for DateFormat.
 * DateFormatData used in DateFormat will be initialized by loading the data
 * from LocaleElements and DateFormatZoneData resources. The zone data are
 * represented with the following form:
 * {ID, new String[] {ID, long zone string, short zone string, long daylight 
 * string, short daylight string, representative city of zone}}, where ID is
 * NOT localized, but is used to look up the localized timezone data
 * internally. Localizers can localize any zone strings except
 * for the ID of the timezone.
 * Also, localizer should not touch "localPatternChars" entry.
 *
 * @see          Format
 * @see          DateFormatData
 * @see          LocaleElements
 * @see          SimpleDateFormat
 * @see          TimeZone
 * @version      1.19 12/19/03
 * @author       Chen-Lieh Huang
 * @author       Alan Liu
 */
//  Canada DateFormatZoneData
//
public final class DateFormatZoneData_en_CA extends DateFormatZoneData
{
    /**
     * Overrides DateFormatZoneData
     */
    public Object[][] getContents() {
        return new Object[][] {
	    // Zones should have unique names and abbreviations within
	    // this locale.  Names and abbreviations may be identical
	    // if the corresponding zones really are identical.  E.g.:
	    // America/Phoenix and America/Denver both use MST; these
	    // zones differ only in that America/Denver uses MDT as well.
	    //
	    // We list both short and long IDs.  Short IDs come first so that they
	    // are chosen preferentially during parsing of zone names.
	    //
	    {"PST", new String[] {/*--America/Los_Angeles--*/ "Pacific Standard Time", "PST",
				  "Pacific Daylight Time", "PDT" /*San Francisco*/}},
	    {"MST", new String[] {/*--America/Denver--*/ "Mountain Standard Time", "MST",
				  "Mountain Daylight Time", "MDT" /*Denver*/}},
	    {"PNT", new String[] {/*--America/Phoenix--*/ "Mountain Standard Time", "MST",
				  "Mountain Standard Time", "MST" /*Phoenix*/}},
	    {"CST", new String[] {/*--America/Chicago--*/ "Central Standard Time", "CST",
				  "Central Daylight Time", "CDT" /*Chicago*/}},
	    {"EST", new String[] {/*--America/New_York--*/ "Eastern Standard Time", "EST",
				  "Eastern Daylight Time", "EDT" /*New York*/}},
	    {"IET", new String[] {/*--America/Indianapolis--*/ "Eastern Standard Time", "EST",
				  "Eastern Standard Time", "EST" /*Indianapolis*/}},
	    {"HST", new String[] {/*--Pacific/Honolulu--*/ "Hawaii Standard Time", "HST",
				  "Hawaii Standard Time", "HST" /*Honolulu*/}},
	    {"AST", new String[] {/*--America/Anchorage--*/ "Alaska Standard Time", "AKST",
				  "Alaska Daylight Time", "AKDT" /*Anchorage*/}},
	    {"CNT", new String[] {/*--America/St_Johns--*/ "Newfoundland Standard Time",
				  "NST", "Newfoundland Daylight Time", "NDT" /*St. John's*/}},
            
	    {"America/Los_Angeles", new String[] {"Pacific Standard Time", "PST",
						  "Pacific Daylight Time", "PDT" /*San Francisco*/}},
	    {"America/Denver", new String[] {"Mountain Standard Time", "MST",
					     "Mountain Daylight Time", "MDT" /*Denver*/}},
	    {"America/Phoenix", new String[] {"Mountain Standard Time", "MST",
					      "Mountain Standard Time", "MST" /*Phoenix*/}},
	    {"America/Chicago", new String[] {"Central Standard Time", "CST",
					      "Central Daylight Time", "CDT" /*Chicago*/}},
	    {"America/New_York", new String[] {"Eastern Standard Time", "EST",
					       "Eastern Daylight Time", "EDT" /*New York*/}},
	    {"America/Indianapolis", new String[] {"Eastern Standard Time", "EST",
						   "Eastern Standard Time", "EST" /*Indianapolis*/}},
	    {"Pacific/Honolulu", new String[] {"Hawaii Standard Time", "HST",
					       "Hawaii Standard Time", "HST" /*Honolulu*/}},
	    {"America/Anchorage", new String[] {"Alaska Standard Time", "AKST",
						"Alaska Daylight Time", "AKDT" /*Anchorage*/}},
	    {"America/Halifax", new String[] {"Atlantic Standard Time", "AST",
					      "Atlantic Daylight Time", "ADT" /*Halifax*/}},
	    {"America/St_Johns", new String[] {"Newfoundland Standard Time",
					       "NST", "Newfoundland Daylight Time", "NDT" /*St. John's*/}},

            {"localPatternChars", "GyMdkHmsSEDFwWahKzZ"},
        };
    }
}
