/*
 * @(#)locale_str.h	1.6 05/11/17
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


/*
 * List mapping from Windows LCID values to Java locale IDs.  Although this table
 * can (and should) include locale IDs with variant codes when appropriate, the
 * code that actually uses this table doesn't currently have a way to support the
 * variant code and will ignore it.  This will have to be changed in the future.
 * [This isn't a regression; the old code we're replacing didn't have this feature either.]
 *
 * The entries in this list should not be construed to suggest we actually have
 * full locale-data and other support for all of these locales; these are
 * merely all of the Windows locales for which we could construct an accurate
 * locale ID.  --rtg 5/4/98
 */
typedef struct LCIDtoLocale {
    LCID    winID;
    char*   javaID;
} LCIDtoLocale;

static LCIDtoLocale localeIDMap[] = {
    /* fallback locales to use when the country code doesn't match anything we have */
    0x01,   "ar",
    0x02,   "bg",
    0x03,   "ca",
    0x04,   "zh",
    0x05,   "cs",
    0x06,   "da",
    0x07,   "de",
    0x08,   "el",
    0x09,   "en",
    0x0a,   "es",
    0x0b,   "fi",
    0x0c,   "fr",
    0x0d,   "iw",
    0x0e,   "hu",
    0x0f,   "is",
    0x10,   "it",
    0x11,   "ja",
    0x12,   "ko",
    0x13,   "nl",
    0x14,   "no",
    0x15,   "pl",
    0x16,   "pt",
    0x17,   "rm",
    0x18,   "ro",
    0x19,   "ru",
    0x1a,   "sh",
    0x1b,   "sk",
    0x1c,   "sq",
    0x1d,   "sv",
    0x1e,   "th",
    0x1f,   "tr",
    0x20,   "ur",
    0x21,   "in",
    0x22,   "uk",
    0x23,   "be",
    0x24,   "sl",
    0x25,   "et",
    0x26,   "lv",
    0x27,   "lt",
    0x29,   "fa",
    0x2a,   "vi",
    0x2d,   "eu",
    0x2f,   "mk",
    0x32,   "tn",
    0x34,   "xh",
    0x35,   "zu",
    0x36,   "af",
    0x38,   "fo",
    0x39,   "hi",
    0x3a,   "mt",
    0x3c,   "gd",
    /* mappings for real Windows LCID values */
    0x0401, "ar_SA",
    0x0402, "bg_BG",
    0x0403, "ca_ES",
    0x0404, "zh_TW",
    0x0405, "cs_CZ",
    0x0406, "da_DK",
    0x0407, "de_DE",
    0x0408, "el_GR",
    0x0409, "en_US",
    0x040a, "es_ES", /* (traditional sort) */
    0x040b, "fi_FI",
    0x040c, "fr_FR",
    0x040d, "iw_IL",
    0x040e, "hu_HU",
    0x040f, "is_IS",
    0x0410, "it_IT",
    0x0411, "ja_JP",
    0x0412, "ko_KR",
    0x0413, "nl_NL",
    0x0414, "no_NO",
    0x0415, "pl_PL",
    0x0416, "pt_BR",
    0x0417, "rm_CH",
    0x0418, "ro_RO",
    0x0419, "ru_RU",
    0x041a, "hr_HR",
    0x041b, "sk_SK",
    0x041c, "sq_AL",
    0x041d, "sv_SE",
    0x041e, "th_TH",
    0x041f, "tr_TR",
    0x0420, "ur",    /* (country?) */
    0x0421, "in_ID",
    0x0422, "uk_UA",
    0x0423, "be_BY",
    0x0424, "sl_SI",
    0x0425, "et_EE",
    0x0426, "lv_LV",
    0x0427, "lt_LT",
    0x0429, "fa_IR",
    0x042a, "vi_VN",
    0x042d, "eu_ES",
/*  0x042e, "??",       no ISO-639 abbreviation for Sorbian */
    0x042f, "mk_MK",
/*  0x0430, "??",       no ISO-639 abbreviation for Sutu */
    0x0431, "ts",    /* (country?) */
    0x0432, "tn_BW",
/*  0x0433, "??",       no ISO-639 abbreviation for Venda */
    0x0434, "xh",    /* (country?) */
    0x0435, "zu",    /* (country?) */
    0x0436, "af_ZA",
    0x0438, "fo_FO",
    0x0439, "hi_IN",
    0x043a, "mt_MT",
/*  0x043b, "??",       no ISO-639 abbreviation for Sami */
    0x043c, "gd_GB",
    0x043d, "yi",    /* (country?) */
    0x043e, "ms_MY",
    0x0441, "sw_KE",
    0x0801, "ar_IQ",
    0x0804, "zh_CN",
    0x0807, "de_CH",
    0x0809, "en_GB",
    0x080a, "es_MX",
    0x080c, "fr_BE",
    0x0810, "it_CH",
    0x0812, "ko_KR",
    0x0813, "nl_BE",
    0x0814, "no_NO_NY",
    0x0816, "pt_PT",
    0x0818, "ro_MD",
    0x0819, "ru_MD",
    0x081a, "sh_YU",
    0x081d, "sv_FI",
    0x083c, "ga_IE",
    0x083e, "ms_BN",
    0x0c01, "ar_EG",
    0x0c04, "zh_HK",
    0x0c07, "de_AT",
    0x0c09, "en_AU",
    0x0c0a, "es_ES", /* (modern sort) */
    0x0c0c, "fr_CA",
    0x0c1a, "sr_YU",
    0x1001, "ar_LY",
    0x1004, "zh_SG",
    0x1007, "de_LU",
    0x1009, "en_CA",
    0x100a, "es_GT",
    0x100c, "fr_CH",
    0x1401, "ar_DZ",
    0x1404, "zh_MO",
    0x1407, "de_LI",
    0x1409, "en_NZ",
    0x140a, "es_CR",
    0x140c, "fr_LU",
    0x1801, "ar_MA",
    0x1809, "en_IE",
    0x180a, "es_PA",
    0x180c, "fr_MC",
    0x1c01, "ar_TN",
    0x1c09, "en_ZA",
    0x1c0a, "es_DO",
    0x2001, "ar_OM",
    0x2009, "en_JM",
    0x200a, "es_VE",
    0x2401, "ar_YE",
    0x2409, "en",    /* ("Caribbean", which could be any of many countries) */
    0x240a, "es_CO",
    0x2801, "ar_SY",
    0x2809, "en_BZ",
    0x280a, "es_PE",
    0x2c01, "ar_JO",
    0x2c09, "en_TT",
    0x2c0a, "es_AR",
    0x3001, "ar_LB",
    0x3009, "en_ZW",
    0x300a, "es_EC",
    0x3401, "ar_KW",
    0x3409, "en_PH",
    0x340a, "es_CL",
    0x3801, "ar_AE",
    0x380a, "es_UY",
    0x3c01, "ar_BH",
    0x3c0a, "es_PY",
    0x4001, "ar_QA",
    0x400a, "es_BO",
    0x440a, "es_SV",
    0x480a, "es_HN",
    0x4c0a, "es_NI",
    0x500a, "es_PR"
};

/*
 * Windows locale string to Encoding converter
 * default file.encoding is "8859_1".
 */
static char *encoding_names[] = {
    "ar", "Cp1256",
    "be", "Cp1251",
    "bg", "Cp1251",
    "cs", "Cp1250",
    "el", "Cp1253",
    "et", "Cp1257",
    "iw", "Cp1255",
    "hu", "Cp1250",
    "ja", "MS932",
    "ko", "MS949",
    "lt", "Cp1257",
    "lv", "Cp1257",
    "mk", "Cp1251",
    "pl", "Cp1250",
    "ro", "Cp1250",
    "ru", "Cp1251",
    "sh", "Cp1250",
    "sk", "Cp1250",
    "sl", "Cp1250",
    "sq", "Cp1250",
    "sr", "Cp1251",
    "th", "MS874",
    "tr", "Cp1254",
    "uk", "Cp1251",
    "zh", "GBK",
    "zh_TW", "MS950",
    "",
};

#define timezone_names_length (sizeof(timezone_names) / sizeof(timezone_names[0]))
