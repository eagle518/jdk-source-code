/*
 * @(#)locale_str.h	1.38 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * Mappings from partial locale names to full locale names
 */
 static char *locale_aliases[] = {
    "ar", "ar_EG",
    "be", "be_BY",
    "bg", "bg_BG",
    "br", "br_FR",
    "ca", "ca_ES",
    "cs", "cs_CZ",
    "cz", "cs_CZ",
    "da", "da_DK",
    "de", "de_DE",
    "el", "el_GR",
    "en", "en_US",
    "eo", "eo",    /* no country for Esperanto */
    "es", "es_ES",
    "et", "et_EE",
    "eu", "eu_ES",
    "fi", "fi_FI",
    "fr", "fr_FR",
    "ga", "ga_IE",
    "gl", "gl_ES",
    "he", "iw_IL",
    "hr", "hr_HR",
#ifdef __linux__
    "hs", "en_US", // used on Linux, not clear what it stands for
#endif
    "hu", "hu_HU",
    "id", "in_ID",
    "in", "in_ID",
    "is", "is_IS",
    "it", "it_IT",
    "iw", "iw_IL",
    "ja", "ja_JP",
    "kl", "kl_GL",
    "ko", "ko_KR",
    "lt", "lt_LT",
    "lv", "lv_LV",
    "mk", "mk_MK",
    "nl", "nl_NL",
    "no", "no_NO",
    "pl", "pl_PL",
    "pt", "pt_PT",
    "ro", "ro_RO",
    "ru", "ru_RU",
#ifdef __linux__
    "se", "en_US", // used on Linux, not clear what it stands for
#endif
    "sk", "sk_SK",
    "sl", "sl_SI",
    "sq", "sq_AL",
    "sr", "sr_CS",
    "su", "fi_FI",
    "sv", "sv_SE",
    "th", "th_TH",
    "tr", "tr_TR",
#ifdef __linux__
    "ua", "en_US", // used on Linux, not clear what it stands for
#endif
    "uk", "uk_UA",
    "vi", "vi_VN",
#ifdef __linux__
    "wa", "en_US", // used on Linux, not clear what it stands for
#endif
    "zh", "zh_CN",
#ifdef __linux__
    "catalan", "ca_ES",
    "croatian", "hr_HR",
    "czech", "cs_CZ",
    "danish", "da_DK",
    "dansk", "da_DK",
    "deutsch", "de_DE",
    "dutch", "nl_NL",
    "finnish", "fi_FI",
    "fran\xE7\x61is", "fr_FR",
    "french", "fr_FR",
    "german", "de_DE",
    "greek", "el_GR",
    "hebrew", "iw_IL",
    "hrvatski", "hr_HR",
    "hungarian", "hu_HU",
    "icelandic", "is_IS",
    "italian", "it_IT",
    "japanese", "ja_JP",
    "norwegian", "no_NO",
    "polish", "pl_PL",
    "portuguese", "pt_PT",
    "romanian", "ro_RO",
    "russian", "ru_RU",
    "slovak", "sk_SK",
    "slovene", "sl_SI",
    "slovenian", "sl_SI",
    "spanish", "es_ES",
    "swedish", "sv_SE",
    "turkish", "tr_TR",
#else
    "big5", "zh_TW.Big5",
    "chinese", "zh_CN",
    "japanese", "ja_JP",
    "tchinese", "zh_TW",
#endif
    ""
 };

/*
 * Linux/Solaris language string to ISO639 string mapping table.
 */
static char *language_names[] = {
    "C", "en",
    "POSIX", "en",
    "ar", "ar",
    "be", "be",
    "bg", "bg",
    "br", "br",
    "ca", "ca",
    "cs", "cs",
    "cz", "cs",
    "da", "da",
    "de", "de",
    "el", "el",
    "en", "en",
    "eo", "eo",
    "es", "es",
    "et", "et",
    "eu", "eu",
    "fi", "fi",
    "fo", "fo",
    "fr", "fr",
    "ga", "ga",
    "gl", "gl",
    "hi", "hi",
    "he", "iw",
    "hr", "hr",
#ifdef __linux__
    "hs", "en", // used on Linux, not clear what it stands for
#endif
    "hu", "hu",
    "id", "in",
    "in", "in",
    "is", "is",
    "it", "it",
    "iw", "iw",
    "ja", "ja",
    "kl", "kl",
    "ko", "ko",
    "lt", "lt",
    "lv", "lv",
    "mk", "mk",
    "nl", "nl",
    "no", "no",
    "nr", "nr",
    "pl", "pl",
    "pt", "pt",
    "ro", "ro",
    "ru", "ru",
#ifdef __linux__
    "se", "en", // used on Linux, not clear what it stands for
#endif
    "sh", "sr", // sh is deprecated
    "sk", "sk",
    "sl", "sl",
    "sq", "sq",
    "sr", "sr",
    "su", "fi",
    "sv", "sv",
    "th", "th",
    "tr", "tr",
#ifdef __linux__
    "ua", "en", // used on Linux, not clear what it stands for
#endif
    "uk", "uk",
    "vi", "vi",
#ifdef __linux__
    "wa", "en", // used on Linux, not clear what it stands for
#endif
    "zh", "zh",
#ifdef __linux__
    "catalan", "ca",
    "croatian", "hr",
    "czech", "cs",
    "danish", "da",
    "dansk", "da",
    "deutsch", "de",
    "dutch", "nl",
    "finnish", "fi",
    "fran\xE7\x61is", "fr",
    "french", "fr",
    "german", "de",
    "greek", "el",
    "hebrew", "iw",
    "hrvatski", "hr",
    "hungarian", "hu",
    "icelandic", "is",
    "italian", "it",
    "japanese", "ja",
    "norwegian", "no",
    "polish", "pl",
    "portuguese", "pt",
    "romanian", "ro",
    "russian", "ru",
    "slovak", "sk",
    "slovene", "sl",
    "slovenian", "sl",
    "spanish", "es",
    "swedish", "sv",
    "turkish", "tr",
#else
    "chinese", "zh",
    "japanese", "ja",
    "korean", "ko",
#endif
    "",
};

/*
 * Linux/Solaris country string to ISO3166 string mapping table.
 */
static char *country_names[] = {
    "AT", "AT",
    "AU", "AU",
    "AR", "AR",
    "BE", "BE",
    "BR", "BR",
    "BO", "BO",
    "CA", "CA",
    "CH", "CH",
    "CL", "CL",
    "CN", "CN",
    "CO", "CO",
    "CR", "CR",
    "CZ", "CZ",
    "DE", "DE",
    "DK", "DK",
    "DO", "DO",
    "EC", "EC",
    "EE", "EE",
    "ES", "ES",
    "FI", "FI",
    "FO", "FO",
    "FR", "FR",
    "GB", "GB",
    "GR", "GR",
    "GT", "GT",
    "HN", "HN",
    "HR", "HR",
    "HU", "HU",
    "ID", "ID",
    "IE", "IE",
    "IL", "IL",
    "IN", "IN",
    "IS", "IS",
    "IT", "IT",
    "JP", "JP",
    "KR", "KR",
    "LT", "LT",
    "LU", "LU",
    "LV", "LV",
    "MX", "MX",
    "NI", "NI",
    "NL", "NL",
    "NO", "NO",
    "NZ", "NZ",
    "PA", "PA",
    "PE", "PE",
    "PL", "PL",
    "PT", "PT",
    "PY", "PY",
#ifdef __linux__
    "RN", "US", // used on Linux, not clear what it stands for
#endif
    "RO", "RO",
    "RU", "RU",
    "SE", "SE",
    "SI", "SI",
    "SK", "SK",
    "SV", "SV",
    "TH", "TH",
    "TR", "TR",
    "UA", "UA",
    "UK", "GB",
    "US", "US",
    "UY", "UY",
    "VE", "VE",
    "VN", "VN",
    "TW", "TW",
    "YU", "CS", // YU has been removed from ISO 3166
    "",
};

/*
 * Linux/Solaris variant string to Java variant name mapping table.
 */
static char *variant_names[] = {
#ifdef __linux__
    "nynorsk", "NY",
#endif
    "",
};
