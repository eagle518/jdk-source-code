/*
 * @(#)RfcDateParser.java	1.5 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.net.cookie;

import java.util.Date;
import java.util.Locale;
import java.text.SimpleDateFormat;
import java.util.TimeZone;

/**
 * A parser for date strings commonly found in http and email headers that
 * follow various RFC conventions.  Given a date-string, the parser will
 * attempt to parse it by trying matches with a set of patterns, returning
 * null on failure, a Date object on success.
 *
 */
class RfcDateParser 
{
    private boolean isGMT = false;
    
    private static final String[] standardFormats    = {
        "EEEE', 'dd-MMM-yy HH:mm:ss z",         // RFC 850 (obsoleted by 1036)
        "EEEE', 'dd-MMM-yy HH:mm:ss",           // ditto but no tz. Happens too often
        "EEE', 'dd-MMM-yyyy HH:mm:ss z",        // RFC 822/1123
        "EEE', 'dd MMM yyyy HH:mm:ss z",        // REMIND what rfc? Apache/1.1
        "EEEE', 'dd MMM yyyy HH:mm:ss z",       // REMIND what rfc? Apache/1.1
        "EEE', 'dd MMM yyyy hh:mm:ss z",        // REMIND what rfc? Apache/1.1
        "EEEE', 'dd MMM yyyy hh:mm:ss z",       // REMIND what rfc? Apache/1.1
        "EEE MMM dd HH:mm:ss z yyyy",           // Date's string output format
        "EEE MMM dd HH:mm:ss yyyy",             // ANSI C asctime format()
        "EEE', 'dd-MMM-yy HH:mm:ss",            // No time zone 2 digit year RFC 1123
        "EEE', 'dd-MMM-yyyy HH:mm:ss"           // No time zone RFC 822/1123
    };

    private static final String[] gmtStandardFormats = {
        "EEEE',' dd-MMM-yy HH:mm:ss 'GMT'",     // RFC 850 (obsoleted by 1036)
        "EEE',' dd-MMM-yyyy HH:mm:ss 'GMT'",    // RFC 822/1123
        "EEE',' dd MMM yyyy HH:mm:ss 'GMT'",    // REMIND what rfc? Apache/1.1
        "EEEE',' dd MMM yyyy HH:mm:ss 'GMT'",    // REMIND what rfc? Apache/1.1
        "EEE',' dd MMM yyyy hh:mm:ss 'GMT'",    // REMIND what rfc? Apache/1.1
        "EEEE',' dd MMM yyyy hh:mm:ss 'GMT'",    // REMIND what rfc? Apache/1.1
        "EEE MMM dd HH:mm:ss 'GMT' yyyy"        // Date's string output format
    };
    String                      dateString;

    RfcDateParser(String dateString) 
    {
        this.dateString = dateString.trim();

        if (this.dateString.indexOf("GMT") != -1) 
	{
            isGMT = true;
        }
    }

    Date getDate() 
    {
        // format is wdy, DD-Mon-yyyy HH:mm:ss GMT
        int arrayLen = isGMT
                       ? gmtStandardFormats.length
                       : standardFormats.length;

        for (int i = 0; i < arrayLen; i++) 
	{
            Date d;

            if (isGMT) 
	    {
                d = tryParsing(gmtStandardFormats[i]);
            } 
	    else 
	    {
                d = tryParsing(standardFormats[i]);
            }

            if (d != null) 
	    {
                return d;
            }
        }

        return null;
    }

    private Date tryParsing(String format) 
    {
        SimpleDateFormat df = new SimpleDateFormat(format, Locale.US);

        if (isGMT) 
	{
            df.setTimeZone(TimeZone.getTimeZone("GMT"));
        }

        try 
	{
            return df.parse(dateString);
        } 
	catch (Exception e) 
	{
            return null;
        }
    }
}
