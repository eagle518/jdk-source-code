/*
 * @(#)LocaleElements.java	1.38 04/03/09
 */

/*
 * Portions Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * (C) Copyright Taligent, Inc. 1996, 1997 - All Rights Reserved
 * (C) Copyright IBM Corp. 1996 - 1999 - All Rights Reserved
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

public class LocaleElements extends ListResourceBundle {
    /**
     * Overrides ListResourceBundle
     */
    public Object[][] getContents() {
        return new Object[][] {
            { "Languages", // language names
                new String[][] {
                    { "aa", "Afar" },
                    { "ab", "Abkhazian" },
                    { "ae", "Avestan" },
                    { "af", "Afrikaans" },
                    { "ak", "Akan" },
                    { "am", "Amharic" },
                    { "an", "Aragonese" },
                    { "ar", "Arabic" },
                    { "as", "Assamese" },
                    { "av", "Avaric" },
                    { "ay", "Aymara" },
                    { "az", "Azerbaijani" },
                    { "ba", "Bashkir" },
                    { "be", "Belarusian" },
                    { "bg", "Bulgarian" },
                    { "bh", "Bihari" },
                    { "bi", "Bislama" },
                    { "bm", "Bambara" },
                    { "bn", "Bengali" },
                    { "bo", "Tibetan" },
                    { "br", "Breton" },
                    { "bs", "Bosnian" },
                    { "ca", "Catalan" },
                    { "ce", "Chechen" },
                    { "ch", "Chamorro" },
                    { "co", "Corsican" },
                    { "cr", "Cree" },
                    { "cs", "Czech" },
                    { "cu", "Church Slavic" },
                    { "cv", "Chuvash" },
                    { "cy", "Welsh" },
                    { "da", "Danish" },
                    { "de", "German" },
                    { "dv", "Divehi" },
                    { "dz", "Dzongkha" },
                    { "ee", "Ewe" },
                    { "el", "Greek" },
                    { "en", "English" },
                    { "eo", "Esperanto" },
                    { "es", "Spanish" },
                    { "et", "Estonian" },
                    { "eu", "Basque" },
                    { "fa", "Persian" },
                    { "ff", "Fulah" },
                    { "fi", "Finnish" },
                    { "fj", "Fijian" },
                    { "fo", "Faroese" },
                    { "fr", "French" },
                    { "fy", "Frisian" },
                    { "ga", "Irish" },
                    { "gd", "Scottish Gaelic" },
                    { "gl", "Gallegan" },
                    { "gn", "Guarani" },
                    { "gu", "Gujarati" },
                    { "gv", "Manx" },
                    { "ha", "Hausa" },
                    { "he", "Hebrew" },
                    { "hi", "Hindi" },
                    { "ho", "Hiri Motu" },
                    { "hr", "Croatian" },
                    { "ht", "Haitian" },
                    { "hu", "Hungarian" },
                    { "hy", "Armenian" },
                    { "hz", "Herero" },
                    { "ia", "Interlingua" },
                    { "id", "Indonesian" },
                    { "ie", "Interlingue" },
                    { "ig", "Igbo" },
                    { "ii", "Sichuan Yi" },
                    { "ik", "Inupiaq" },
                    { "in", "Indonesian" },
                    { "io", "Ido" },
                    { "is", "Icelandic" },
                    { "it", "Italian" },
                    { "iu", "Inuktitut" },
                    { "iw", "Hebrew" },
                    { "ja", "Japanese" },
                    { "ji", "Yiddish" },
                    { "jv", "Javanese" },
                    { "ka", "Georgian" },
                    { "kg", "Kongo" },
                    { "ki", "Kikuyu" },
                    { "kj", "Kwanyama" },
                    { "kk", "Kazakh" },
                    { "kl", "Greenlandic" },
                    { "km", "Khmer" },
                    { "kn", "Kannada" },
                    { "ko", "Korean" },
                    { "kr", "Kanuri" },
                    { "ks", "Kashmiri" },
                    { "ku", "Kurdish" },
                    { "kv", "Komi" },
                    { "kw", "Cornish" },
                    { "ky", "Kirghiz" },
                    { "la", "Latin" },
                    { "lb", "Luxembourgish" },
                    { "lg", "Ganda" },
                    { "li", "Limburgish" },
                    { "ln", "Lingala" },
                    { "lo", "Lao" },
                    { "lt", "Lithuanian" },
                    { "lu", "Luba-Katanga" },
                    { "lv", "Latvian" },
                    { "mg", "Malagasy" },
                    { "mh", "Marshallese" },
                    { "mi", "Maori" },
                    { "mk", "Macedonian" },
                    { "ml", "Malayalam" },
                    { "mn", "Mongolian" },
                    { "mo", "Moldavian" },
                    { "mr", "Marathi" },
                    { "ms", "Malay" },
                    { "mt", "Maltese" },
                    { "my", "Burmese" },
                    { "na", "Nauru" },
                    { "nb", "Norwegian Bokm\u00e5l" },
                    { "nd", "North Ndebele" },
                    { "ne", "Nepali" },
                    { "ng", "Ndonga" },
                    { "nl", "Dutch" },
                    { "nn", "Norwegian Nynorsk" },
                    { "no", "Norwegian" },
                    { "nr", "South Ndebele" },
                    { "nv", "Navajo" },
                    { "ny", "Nyanja" },
                    { "oc", "Occitan" },
                    { "oj", "Ojibwa" },
                    { "om", "Oromo" },
                    { "or", "Oriya" },
                    { "os", "Ossetian" },
                    { "pa", "Panjabi" },
                    { "pi", "Pali" },
                    { "pl", "Polish" },
                    { "ps", "Pushto" },
                    { "pt", "Portuguese" },
                    { "qu", "Quechua" },
                    { "rm", "Raeto-Romance" },
                    { "rn", "Rundi" },
                    { "ro", "Romanian" },
                    { "ru", "Russian" },
                    { "rw", "Kinyarwanda" },
                    { "sa", "Sanskrit" },
                    { "sc", "Sardinian" },
                    { "sd", "Sindhi" },
                    { "se", "Northern Sami" },
                    { "sg", "Sango" },
                    { "si", "Sinhalese" },
                    { "sk", "Slovak" },
                    { "sl", "Slovenian" },
                    { "sm", "Samoan" },
                    { "sn", "Shona" },
                    { "so", "Somali" },
                    { "sq", "Albanian" },
                    { "sr", "Serbian" },
                    { "ss", "Swati" },
                    { "st", "Southern Sotho" },
                    { "su", "Sundanese" },
                    { "sv", "Swedish" },
                    { "sw", "Swahili" },
                    { "ta", "Tamil" },
                    { "te", "Telugu" },
                    { "tg", "Tajik" },
                    { "th", "Thai" },
                    { "ti", "Tigrinya" },
                    { "tk", "Turkmen" },
                    { "tl", "Tagalog" },
                    { "tn", "Tswana" },
                    { "to", "Tonga" },
                    { "tr", "Turkish" },
                    { "ts", "Tsonga" },
                    { "tt", "Tatar" },
                    { "tw", "Twi" },
                    { "ty", "Tahitian" },
                    { "ug", "Uighur" },
                    { "uk", "Ukrainian" },
                    { "ur", "Urdu" },
                    { "uz", "Uzbek" },
                    { "ve", "Venda" },
                    { "vi", "Vietnamese" },
                    { "vo", "Volap\u00fck" },
                    { "wa", "Walloon" },
                    { "wo", "Wolof" },
                    { "xh", "Xhosa" },
                    { "yi", "Yiddish" },
                    { "yo", "Yoruba" },
                    { "za", "Zhuang" },
                    { "zh", "Chinese" },
                    { "zu", "Zulu" },
                }
            },
            { "Countries", // country names
                new String[][] {
                    { "AD", "Andorra" },
                    { "AE", "United Arab Emirates" },
                    { "AF", "Afghanistan" },
                    { "AG", "Antigua and Barbuda" },
                    { "AI", "Anguilla" },
                    { "AL", "Albania" },
                    { "AM", "Armenia" },
                    { "AN", "Netherlands Antilles" },
                    { "AO", "Angola" },
                    { "AQ", "Antarctica" },
                    { "AR", "Argentina" },
                    { "AS", "American Samoa" },
                    { "AT", "Austria" },
                    { "AU", "Australia" },
                    { "AW", "Aruba" },
                    { "AX", "\u00c5land Islands" },
                    { "AZ", "Azerbaijan" },
                    { "BA", "Bosnia and Herzegovina" },
                    { "BB", "Barbados" },
                    { "BD", "Bangladesh" },
                    { "BE", "Belgium" },
                    { "BF", "Burkina Faso" },
                    { "BG", "Bulgaria" },
                    { "BH", "Bahrain" },
                    { "BI", "Burundi" },
                    { "BJ", "Benin" },
                    { "BM", "Bermuda" },
                    { "BN", "Brunei" },
                    { "BO", "Bolivia" },
                    { "BR", "Brazil" },
                    { "BS", "Bahamas" },
                    { "BT", "Bhutan" },
                    { "BV", "Bouvet Island" },
                    { "BW", "Botswana" },
                    { "BY", "Belarus" },
                    { "BZ", "Belize" },
                    { "CA", "Canada" },
                    { "CC", "Cocos Islands" },
                    { "CD", "The Democratic Republic Of Congo" },
                    { "CF", "Central African Republic" },
                    { "CG", "Congo" },
                    { "CH", "Switzerland" },
                    // Ivory Coast is older usage; Cd'I is now in common use in English
                    { "CI", "C\u00F4te d'Ivoire" },
                    { "CK", "Cook Islands" },
                    { "CL", "Chile" },
                    { "CM", "Cameroon" },
                    { "CN", "China" },
                    { "CO", "Colombia" },
                    { "CR", "Costa Rica" },
                    { "CS", "Serbia and Montenegro" },
                    { "CU", "Cuba" },
                    { "CV", "Cape Verde" },
                    { "CX", "Christmas Island" },
                    { "CY", "Cyprus" },
                    { "CZ", "Czech Republic" },
                    { "DE", "Germany" },
                    { "DJ", "Djibouti" },
                    { "DK", "Denmark" },
                    { "DM", "Dominica" },
                    { "DO", "Dominican Republic" },
                    { "DZ", "Algeria" },
                    { "EC", "Ecuador" },
                    { "EE", "Estonia" },
                    { "EG", "Egypt" },
                    { "EH", "Western Sahara" },
                    { "ER", "Eritrea" },
                    { "ES", "Spain" },
                    { "ET", "Ethiopia" },
                    { "FI", "Finland" },
                    { "FJ", "Fiji" },
                    { "FK", "Falkland Islands" },
                    { "FM", "Micronesia" },
                    { "FO", "Faroe Islands" },
                    { "FR", "France" },
                    { "GA", "Gabon" },
                    { "GB", "United Kingdom" },
                    { "GD", "Grenada" },
                    { "GE", "Georgia" },
                    { "GF", "French Guiana" },
                    { "GH", "Ghana" },
                    { "GI", "Gibraltar" },
                    { "GL", "Greenland" },
                    { "GM", "Gambia" },
                    { "GN", "Guinea" },
                    { "GP", "Guadeloupe" },
                    { "GQ", "Equatorial Guinea" },
                    { "GR", "Greece" },
                    { "GS", "South Georgia And The South Sandwich Islands" },
                    { "GT", "Guatemala" },
                    { "GU", "Guam" },
                    { "GW", "Guinea-Bissau" },
                    { "GY", "Guyana" },
                    { "HK", "Hong Kong" },
                    { "HM", "Heard Island And McDonald Islands" },
                    { "HN", "Honduras" },
                    { "HR", "Croatia" },
                    { "HT", "Haiti" },
                    { "HU", "Hungary" },
                    { "ID", "Indonesia" },
                    { "IE", "Ireland" },
                    { "IL", "Israel" },
                    { "IN", "India" },
                    { "IO", "British Indian Ocean Territory" },
                    { "IQ", "Iraq" },
                    { "IR", "Iran" },
                    { "IS", "Iceland" },
                    { "IT", "Italy" },
                    { "JM", "Jamaica" },
                    { "JO", "Jordan" },
                    { "JP", "Japan" },
                    { "KE", "Kenya" },
                    { "KG", "Kyrgyzstan" },
                    { "KH", "Cambodia" },
                    { "KI", "Kiribati" },
                    { "KM", "Comoros" },
                    { "KN", "Saint Kitts And Nevis" },
                    { "KP", "North Korea" },
                    { "KR", "South Korea" },
                    { "KW", "Kuwait" },
                    { "KY", "Cayman Islands" },
                    { "KZ", "Kazakhstan" },
                    { "LA", "Laos" },
                    { "LB", "Lebanon" },
                    { "LC", "Saint Lucia" },
                    { "LI", "Liechtenstein" },
                    { "LK", "Sri Lanka" },
                    { "LR", "Liberia" },
                    { "LS", "Lesotho" },
                    { "LT", "Lithuania" },
                    { "LU", "Luxembourg" },
                    { "LV", "Latvia" },
                    { "LY", "Libya" },
                    { "MA", "Morocco" },
                    { "MC", "Monaco" },
                    { "MD", "Moldova" },
                    { "MG", "Madagascar" },
                    { "MH", "Marshall Islands" },
                    { "MK", "Macedonia" },
                    { "ML", "Mali" },
                    { "MM", "Myanmar" },
                    { "MN", "Mongolia" },
                    { "MO", "Macao" },
                    { "MP", "Northern Mariana Islands" },
                    { "MQ", "Martinique" },
                    { "MR", "Mauritania" },
                    { "MS", "Montserrat" },
                    { "MT", "Malta" },
                    { "MU", "Mauritius" },
                    { "MV", "Maldives" },
                    { "MW", "Malawi" },
                    { "MX", "Mexico" },
                    { "MY", "Malaysia" },
                    { "MZ", "Mozambique" },
                    { "NA", "Namibia" },
                    { "NC", "New Caledonia" },
                    { "NE", "Niger" },
                    { "NF", "Norfolk Island" },
                    { "NG", "Nigeria" },
                    { "NI", "Nicaragua" },
                    { "NL", "Netherlands" },
                    { "NO", "Norway" },
                    { "NP", "Nepal" },
                    { "NR", "Nauru" },
                    { "NU", "Niue" },
                    { "NZ", "New Zealand" },
                    { "OM", "Oman" },
                    { "PA", "Panama" },
                    { "PE", "Peru" },
                    { "PF", "French Polynesia" },
                    { "PG", "Papua New Guinea" },
                    { "PH", "Philippines" },
                    { "PK", "Pakistan" },
                    { "PL", "Poland" },
                    { "PM", "Saint Pierre And Miquelon" },
                    { "PN", "Pitcairn" },
                    { "PR", "Puerto Rico" },
                    { "PS", "Palestine" },
                    { "PT", "Portugal" },
                    { "PW", "Palau" },
                    { "PY", "Paraguay" },
                    { "QA", "Qatar" },
                    { "RE", "Reunion" },
                    { "RO", "Romania" },
                    { "RU", "Russia" },
                    { "RW", "Rwanda" },
                    { "SA", "Saudi Arabia" },
                    { "SB", "Solomon Islands" },
                    { "SC", "Seychelles" },
                    { "SD", "Sudan" },
                    { "SE", "Sweden" },
                    { "SG", "Singapore" },
                    { "SH", "Saint Helena" },
                    { "SI", "Slovenia" },
                    { "SJ", "Svalbard And Jan Mayen" },
                    { "SK", "Slovakia" },
                    { "SL", "Sierra Leone" },
                    { "SM", "San Marino" },
                    { "SN", "Senegal" },
                    { "SO", "Somalia" },
                    { "SR", "Suriname" },
                    { "ST", "Sao Tome And Principe" },
                    { "SV", "El Salvador" },
                    { "SY", "Syria" },
                    { "SZ", "Swaziland" },
                    { "TC", "Turks And Caicos Islands" },
                    { "TD", "Chad" },
                    { "TF", "French Southern Territories" },
                    { "TG", "Togo" },
                    { "TH", "Thailand" },
                    { "TJ", "Tajikistan" },
                    { "TK", "Tokelau" },
                    { "TL", "Timor-Leste" },
                    { "TM", "Turkmenistan" },
                    { "TN", "Tunisia" },
                    { "TO", "Tonga" },
                    { "TR", "Turkey" },
                    { "TT", "Trinidad and Tobago" },
                    { "TV", "Tuvalu" },
                    { "TW", "Taiwan" },
                    { "TZ", "Tanzania" },
                    { "UA", "Ukraine" },
                    { "UG", "Uganda" },
                    { "UM", "United States Minor Outlying Islands" },
                    { "US", "United States" },
                    { "UY", "Uruguay" },
                    { "UZ", "Uzbekistan" },
                    { "VA", "Vatican" },
                    { "VC", "Saint Vincent And The Grenadines" },
                    { "VE", "Venezuela" },
                    { "VG", "British Virgin Islands" },
                    { "VI", "U.S. Virgin Islands" },
                    { "VN", "Vietnam" }, // One word
                    { "VU", "Vanuatu" },
                    { "WF", "Wallis And Futuna" },
                    { "WS", "Samoa" },
                    { "YE", "Yemen" },
                    { "YT", "Mayotte" },
                    { "ZA", "South Africa" },
                    { "ZM", "Zambia" },
                    { "ZW", "Zimbabwe" },
                }
            },
            { "%%EURO", "Euro" }, // Euro variant display name
            { "%%B", "Bokm\u00e5l" }, // Norwegian variant display name
            { "%%NY", "Nynorsk" },  // Norwegian variant display name
            { "LocaleNamePatterns",
                /* Formats for the display name of a locale, for a list of
                 * items, and for composing two items in a list into one item.
                 * The list patterns are used in the variant name and in the
                 * full display name.
                 *
                 * This is the language-neutral form of this resource.
                 */
                new String[] {
                    "{0,choice,0#|1#{1}|2#{1} ({2})}", // Display name
                    "{0,choice,0#|1#{1}|2#{1},{2}|3#{1},{2},{3}}", // List
                    "{0},{1}" // List composition
                }
            },
            { "MonthNames",
                new String[] {
                    "January", // january
                    "February", // february
                    "March", // march
                    "April", // april
                    "May", // may
                    "June", // june
                    "July", // july
                    "August", // august
                    "September", // september
                    "October", // october
                    "November", // november
                    "December", // december
                    "" // month 13 if applicable
                }
            },
            { "MonthAbbreviations",
                new String[] {
                    "Jan", // abb january
                    "Feb", // abb february
                    "Mar", // abb march
                    "Apr", // abb april
                    "May", // abb may
                    "Jun", // abb june
                    "Jul", // abb july
                    "Aug", // abb august
                    "Sep", // abb september
                    "Oct", // abb october
                    "Nov", // abb november
                    "Dec", // abb december
                    "" // abb month 13 if applicable
                }
            },
            { "DayNames",
                new String[] {
                    "Sunday", // Sunday
                    "Monday", // Monday
                    "Tuesday", // Tuesday
                    "Wednesday", // Wednesday
                    "Thursday", // Thursday
                    "Friday", // Friday
                    "Saturday" // Saturday
                }
            },
            { "DayAbbreviations",
                new String[] {
                    "Sun", // abb Sunday
                    "Mon", // abb Monday
                    "Tue", // abb Tuesday
                    "Wed", // abb Wednesday
                    "Thu", // abb Thursday
                    "Fri", // abb Friday
                    "Sat" // abb Saturday
                }
            },
            { "AmPmMarkers",
                new String[] {
                    "AM", // am marker
                    "PM" // pm marker
                }
            },
            { "Eras",
                new String[] { // era strings
                    "BC",
                    "AD"
                }
            },
            { "NumberPatterns",
                new String[] {
                    "#,##0.###;-#,##0.###", // decimal pattern
                    "\u00a4 #,##0.00;-\u00a4 #,##0.00", // currency pattern
                    "#,##0%" // percent pattern
                }
            },
            { "NumberElements",
                new String[] {
                    ".", // decimal separator
                    ",", // group (thousands) separator
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
            { "CurrencySymbols",
                new String[][] {
                   // localized versions should have entries of the form
                   // {<ISO 4217 currency code>, <localized currency symbol>}
                   // e.g., {"USD", "US$"}
                   // There are no entries for the root locale, so we fall
                   // back onto the ISO 4217 currency code.
                }
            },
            { "DateTimePatterns",
                new String[] {
                    "h:mm:ss a z", // full time pattern
                    "h:mm:ss a z", // long time pattern
                    "h:mm:ss a", // medium time pattern
                    "h:mm a", // short time pattern
                    "EEEE, MMMM d, yyyy", // full date pattern
                    "MMMM d, yyyy", // long date pattern
                    "MMM d, yyyy", // medium date pattern
                    "M/d/yy", // short date pattern
                    "{1} {0}" // date-time pattern
                }
            },
            { "DateTimeElements",
                new String[] {
                    "1", // first day of week
                    "1" // min days in first week
                }
            },
            { "CollationElements", "" },
        };
    }
}
