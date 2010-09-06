/*
 * @(#)LocaleElements_fi.java	1.18 04/01/26
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

import java.util.ListResourceBundle;

public class LocaleElements_fi extends ListResourceBundle {
    /**
     * Overrides ListResourceBundle
     */
    public Object[][] getContents() {
        return new Object[][] {
            { "Languages", // language names
                new String[][] {
		    { "ar", "arabia" },
		    { "ba", "baski" },
		    { "bg", "bulgaria" },
		    { "ca", "katalaani" },
		    { "cs", "tsekki" },
		    { "da", "danska" },
		    { "de", "saksa" },
		    { "el", "kreikka" },
		    { "en", "englanti" },
		    { "es", "espanja" },
		    { "fi", "suomi" },
		    { "fr", "franska" },
		    { "he", "heprea" },
                    { "iw", "heprea" },
		    { "hi", "hindi" },
		    { "it", "italia" },
		    { "ja", "japani" },
		    { "lt", "liettua" },
		    { "lv", "latvia" },
		    { "nl", "hollanti" },
		    { "no", "norja" },
		    { "pl", "puola" },
		    { "pt", "portugali" },
		    { "ru", "ven\u00e4j\u00e4" },
		    { "sv", "ruotsi" },
		    { "th", "thai" },
		    { "tr", "turkki" },
		    { "zh", "kiina" }
                }
            },
            { "Countries", // country names
                new String[][] {
		    { "BE", "Belgia" },
		    { "BR", "Brasilia" },
		    { "CA", "Kanada" },
		    { "CH", "Sveitsi" },
		    { "CN", "Kiina" },
		    { "CZ", "Tsekin tasavalta" },
		    { "DE", "Saksa" },
		    { "DK", "Danska" },
		    { "ES", "Espanja" },
		    { "FI", "Suomi" },
		    { "FR", "Franska" },
		    { "GB", "Iso-Britannia" },
		    { "GR", "Kreikka" },
		    { "IE", "Irlanti" },
		    { "IT", "Italia" },
		    { "JP", "Japani" },
		    { "KR", "Korea" },
		    { "NL", "Alankomaat" },
		    { "NO", "Norja" },
		    { "PL", "Puola" },
		    { "PT", "Portugali" },
		    { "RU", "Ven\u00e4j\u00e4" },
		    { "SE", "Ruotsi" },
		    { "TR", "Turkki" },
		    { "US", "Yhdysvallat" }
                }
            },
            { "MonthNames",
                new String[] {
                    "tammikuu", // january
                    "helmikuu", // february
                    "maaliskuu", // march
                    "huhtikuu", // april
                    "toukokuu", // may
                    "kes\u00e4kuu", // june
                    "hein\u00e4kuu", // july
                    "elokuu", // august
                    "syyskuu", // september
                    "lokakuu", // october
                    "marraskuu", // november
                    "joulukuu", // december
                    "" // month 13 if applicable
                }
            },
            { "MonthAbbreviations",
                new String[] {
                    "tammi", // abb january
                    "helmi", // abb february
                    "maalis", // abb march
                    "huhti", // abb april
                    "touko", // abb may
                    "kes\u00e4", // abb june
                    "hein\u00e4", // abb july
                    "elo", // abb august
                    "syys", // abb september
                    "loka", // abb october
                    "marras", // abb november
                    "joulu", // abb december
                    "" // abb month 13 if applicable
                }
            },
            { "DayNames",
                new String[] {
                    "sunnuntai", // Sunday
                    "maanantai", // Monday
                    "tiistai", // Tuesday
                    "keskiviikko", // Wednesday
                    "torstai", // Thursday
                    "perjantai", // Friday
                    "lauantai" // Saturday
                }
            },
            { "DayAbbreviations",
                new String[] {
                    "su", // abb Sunday
                    "ma", // abb Monday
                    "ti", // abb Tuesday
                    "ke", // abb Wednesday
                    "to", // abb Thursday
                    "pe", // abb Friday
                    "la" // abb Saturday
                }
            },
            { "NumberElements",
                new String[] {
                    ",", // decimal separator
                    "\u00a0", // group (thousands) separator
                    ";", // list separator
                    "%", // percent sign
                    "0", // native 0 digit
                    "#", // pattern digit
                    "-", // minus sign
                    "E", // exponential
                    "\u2030", // per mille
                    "\u221e", // infinity
                    "\ufffd" // NaN
                }
            },
            { "DateTimePatterns",
                new String[] {
                    "H:mm:ss z", // full time pattern
                    "H:mm:ss z", // long time pattern
                    "H:mm:ss", // medium time pattern
                    "H:mm", // short time pattern
                    "d. MMMM'ta 'yyyy", // full date pattern
                    "d. MMMM'ta 'yyyy", // long date pattern
                    "d.M.yyyy", // medium date pattern
                    "d.M.yyyy", // short date pattern
                    "{1} {0}" // date-time pattern
                }
            },
            { "DateTimeElements",
                new String[] {
                    "2", // first day of week
                    "4" // min days in first week
                }
            },
            { "CollationElements",
                "& V ; w , W "
                + "& Z < a\u030a , A\u030a"   // Z < a-ring
                + "< a\u0308 , A\u0308 < o\u0308 , O\u0308"
                + "< o\u030b , O\u030b ; \u00f8 , \u00d8"  // o-double-acute ; o-stroke
                + "&  Y ; u\u030b, U\u030b "       // nt : y ; u-double-acute
                + "; u\u0308 , U\u0308"   // nt & tal : y ; u-umlaut
            }
        };
    }
}
