/*
 * @(#)DateFormatZoneData_nl_BE.java	1.21 03/12/19
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
 * @version      1.21 12/19/03
 * @author       Chen-Lieh Huang
 */
//  Belgium-Dutch DateFormatZoneData
//
public final class DateFormatZoneData_nl_BE extends DateFormatZoneData
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
            {"localPatternChars", "GyMdkHmsSEDFwWahKzZ"},
        };
    }
}
