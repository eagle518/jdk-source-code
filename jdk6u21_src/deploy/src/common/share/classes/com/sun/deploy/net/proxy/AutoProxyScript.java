/*
 * @(#)AutoProxyScript.java	1.16 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.net.proxy;


/**
 * AutoProxyScript is used for determining ProxyInfo in the auto
 * proxy configuration. 
 * 
 * It implements all emulated JavaScript functions which are used 
 * by IE and Navigator for finding the proxy setting.
 */

public interface AutoProxyScript
{
    public static final String jsGlobal = 
	"var _mon = new Array('JAN', 'FEB', 'MAR', 'APR', 'MAY', 'JUN', 'JUL', 'AUG', 'SEP', 'OCT', 'NOV', 'DEC');" +
	"var _day = new Array('SUN', 'MON', 'TUE', 'WED', 'THU', 'FRI', 'SAT');" +
	"function _isGmt(i) {" +
	   " return typeof i == 'string' && i == 'GMT'; }";

    public static final String jsDnsDomainIs = 
        "function dnsDomainIs(host, domain) {" +
	    "if (domain != null && domain.charAt(0) != '.')" +
	    	"return shExpMatch(host, domain);" +
	    "return shExpMatch(host, '*' + domain); }";

	public static final String jsconvert_addr = 
		"function convert_addr(ipchars) {\n"+
		"    var bytes = ipchars.split('.');\n"+
		"    var result = ((bytes[0] & 0xff) << 24) |\n"+
		"                 ((bytes[1] & 0xff) << 16) |\n"+
		"                 ((bytes[2] & 0xff) <<  8) |\n"+
		"                  (bytes[3] & 0xff);\n"+
		"    return result;\n"+
		"}\n" ;

    public static final String jsIsInNetForNS = 
		"function isInNet(ipaddr, pattern, maskstr) {\n"+
		"    var ipPattern = /^(\\d{1,3})\\.(\\d{1,3})\\.(\\d{1,3})\\.(\\d{1,3})$/;\n"+
        "    var test = ipaddr.match(ipPattern);\n"+
		"    if (test == null) {\n"+
		"        ipaddr = dnsResolve(ipaddr);\n"+
		"        if (ipaddr == null)\n"+
		"            return false;\n"+
		"    } else if ((test[1] > 255) || (test[2] > 255) || \n"+
		"               (test[3] > 255) || (test[4] > 255) ) {\n"+
		"        return false;\n"+
		"    }\n"+
		"    var host = convert_addr(ipaddr);\n"+
		"    var pat  = convert_addr(pattern);\n"+
		"    var mask = convert_addr(maskstr);\n"+
		"    return ((host & mask) == (pat & mask));\n"+
		"    \n"+
		"}\n";

    public static final String jsIsInNetForIE = 
		"function isInNet(ipaddr, pattern, maskstr) {\n"+
		"    var ipPattern = /^(\\d{1,3})\\.(\\d{1,3})\\.(\\d{1,3})\\.(\\d{1,3})$/;\n"+
        "    var test = ipaddr.match(ipPattern);\n"+
		"    if (test == null) {\n"+
		"        ipaddr = dnsResolve(ipaddr);\n"+
		"        if (ipaddr == null)\n"+
		"            return false;\n"+
		"    } else if ((test[1] > 255) || (test[2] > 255) || \n"+
		"               (test[3] > 255) || (test[4] > 255) ) {\n"+
		"        return false;\n"+
		"    }\n"+
		"    var host = convert_addr(ipaddr);\n"+
		"    var pat  = convert_addr(pattern);\n"+
		"    var mask = convert_addr(maskstr);\n"+
		"    return ((host & mask) == (pat & mask));\n"+
		"    \n"+
		"}\n";

    public static final String jsIsPlainHostName = 
	"function isPlainHostName(host){" +
	    "return (dnsDomainLevels(host) == 0); }";

    public static final String jsIsResolvable = 
	"function isResolvable(host){" +
	    "return (dnsResolve(host) != ''); }";

    public static final String jsLocalHostOrDomainIs = 
	"function localHostOrDomainIs(host, hostdom){" +
	    "return shExpMatch(hostdom, host + '*'); }";

    public static final String jsDnsDomainLevels = 
	"function dnsDomainLevels(host){" +
	    "var s = host + '';" +
	    "for (var i=0, j=0; i < s.length; i++)" +
		"if (s.charAt(i) == '.')" +
		    "j++;" +
	    "return j; }";

    public static final String jsDnsResolveForNS = 
	"function dnsResolve(host){" +
	    "if (typeof host != 'string' || dnsDomainLevels(host) != 3) return ''; " +
	    "for (var i=0; i < host.length; i++) " +
		"if ((host.charAt(i) < '0' || host.charAt(i) > '9') && host.charAt(i) != '.') return ''; " +
	    "return host; }";

	// fix for 6175601: javawebstart crash if autoproxy script uses IsInNet function
	// the dnsResolve implementation is loaded from different ActiveX Object, depending
	// on whether we are currently running java plugin or java web start
	public static final String jsDnsResolveActiveXObj = System.getProperty("javaplugin.version") != null ? "'JavaPlugin'" : "'JavaWebStart.isInstalled'";

    public static final String jsDnsResolveForIE = 
		"function dnsResolve(host){" +
		"if (typeof host != 'string') return ''; " +
		"var isIpFormat = false;" +
		"if (dnsDomainLevels(host) == 3){" +
		"for (var i=0; i < host.length; i++){" +
		"if ((host.charAt(i) >= '0' && host.charAt(i) <= '9') || host.charAt(i) == '.') " +
		"isIpFormat = true;" +
		"else {" + 
		"isIpFormat = false;" + 
		"break;" +
		"}" +
		"}" +
		"}" +
		"if (isIpFormat == true) return host;" +
		"else {" +
		"var axObj = new ActiveXObject(" + 
		jsDnsResolveActiveXObj +
		");" +
		"return axObj.dnsResolve(host);" +
		"}" +
		"}";

    public static final String jsMyIpAddress_0 = 
	"function myIpAddress(){" +
	    "return '";

    public static final String jsMyIpAddress_1 = 
	    "'; }";

    public static final String jsShExpMatch = 
	"function shExpMatch(str, shexp){ " +
	    " if (typeof str != 'string' || typeof shexp != 'string') return false; " +
	    " if (shexp == '*') return true; " +
	    " if (str == '' && shexp == '') return true; " +
	    " str = str.toLowerCase();" +
	    " shexp = shexp.toLowerCase();" +
	    " var index = shexp.indexOf('*');" +
	    " if (index == -1) { return (str == shexp); } " +
	    " else if (index == 0) { " +
		" for (var i=0; i <= str.length; i++) { " +
		    " if (shExpMatch(str.substring(i), shexp.substring(1))) return true; " +
		" } return false; } " +
	     "else { " +
		"var sub = null, sub2 = null; " +
		"sub = shexp.substring(0, index);" +
		"if (index <= str.length) sub2 = str.substring(0, index); " +
		"if (sub != '' && sub2 != '' && sub == sub2) { " +
		    "return shExpMatch(str.substring(index), shexp.substring(index)); }" +
		"else {	return false; }" +
	     "} }";


    public static final String jsEnableDateRange = 
    	"function _dateRange(day1, month1, year1, day2, month2, year2, gmt){" +
	    "if (typeof day1 != 'number' || day1 <= 0 || typeof month1 != 'string' || typeof year1 != 'number' || year1 <= 0" +
	      " || typeof day2 != 'number' || day2 <= 0 || typeof month2 != 'string' || typeof year2 != 'number' || year2 <= 0" +
	      " || typeof gmt != 'boolean') return false; " +

	    "var m1 = -1, m2 = -1;" +

	    "for (var i=0; i < _mon.length; i++){" +
		"if (_mon[i] == month1)" +
		    "m1 = i;" +
		"if (_mon[i] == month2)" +
		    "m2 = i;" +
	    "}" +

	    "var cur = new Date();" +
	    "var d1 = new Date(year1, m1, day1, 0, 0, 0);" +
	    "var d2 = new Date(year2, m2, day2, 23, 59, 59);" +

	    "if (gmt == true)" +
 		"cur = new Date(cur.getTime() - cur.getTimezoneOffset() * 60 * 1000);" +

	    "return ((d1.getTime() <= cur.getTime()) && (cur.getTime() <= d2.getTime()));" +
	"}" +
	"function dateRange(p1, p2, p3, p4, p5, p6, p7){" +
	    "var cur = new Date();" +

	    "if (typeof p1 == 'undefined')" +
		"return false;" +
	    "else if (typeof p2 == 'undefined' || _isGmt(p2))" +
	    "{" +
		"if ((typeof p1) == 'string')" +
		     "return _dateRange(1, p1, cur.getFullYear(), 31, p1, cur.getFullYear(), _isGmt(p2));" +
		"else if (typeof p1 == 'number' && p1 > 31)" +
		     "return _dateRange(1, 'JAN', p1, 31, 'DEC', p1, _isGmt(p2));" +
		"else {" +
		     "for (var i=0; i < _mon.length; i++)" +
			  "if (_dateRange(p1, _mon[i], cur.getFullYear(), p1, _mon[i], cur.getFullYear(), _isGmt(p2)))" +
			      " return true;" +
		     "return false;" +
		"}" +
	     "}" +
	     "else if (typeof p3 == 'undefined' || _isGmt(p3))" +
	     "{" +
		"if ((typeof p1) == 'string')" +
		     "return _dateRange(1, p1, cur.getFullYear(), 31, p2, cur.getFullYear(), _isGmt(p3));" +
		"else if (typeof p1 == 'number' && typeof p2 == 'number' && (p1 > 31 || p2 > 31))" +
		     "return _dateRange(1, 'JAN', p1, 31, 'DEC', p2, _isGmt(p3));" +
		"else " +
		"{" +
		     "if ((typeof p2) == 'string')" +
		     "{" +
			  "return _dateRange(p1, p2, cur.getFullYear(), p1, p2, cur.getFullYear(), _isGmt(p3));" +
		     "}" +
		     "else " +
		     "{" +
			  "for (var i=0; i < _mon.length; i++)" +
			       "if (_dateRange(p1, _mon[i], cur.getFullYear(), p2, _mon[i], cur.getFullYear(), _isGmt(p3)))" +
				   "return true;" +
 	      		  "return false;" +
		     "}" +
		 "}" +
	     "}" +
	     "else if (typeof p4 == 'undefined' || _isGmt(p4))" +
		 "return _dateRange(p1, p2, p3, p1, p2, p3, _isGmt(p4));" +
	     "else if (typeof p5 == 'undefined' || _isGmt(p5))" +
	     "{" +
		 "if (typeof p2 == 'number')" +
		      "return _dateRange(1, p1, p2, 31, p3, p4, _isGmt(p5));" +
		 "else " +
		      "return _dateRange(p1, p2, cur.getFullYear(), p3, p4, cur.getFullYear(), _isGmt(p5))" +
	     "}" +
	     "else if (typeof p6 == 'undefined')" +
		 "return false;" +
	     "else " +
		 "return _dateRange(p1, p2, p3, p4, p5, p6, _isGmt(p7));" +
	"}";

    public static final String jsEnableTimeRange = 
	"function timeRange(p1, p2, p3, p4, p5, p6, p7) {" +
	      "if (typeof p1 == 'undefined')" +
		   "return false;" +
	      "else if (typeof p2 == 'undefined' || _isGmt(p2))" +
		   "return _timeRange(p1, 0, 0, p1, 59, 59, _isGmt(p2));" +
	      "else if (typeof p3 == 'undefined' || _isGmt(p3))" +
		   "return _timeRange(p1, 0, 0, p2, 0, 0, _isGmt(p3));" +
	      "else if (typeof p4 == 'undefined')" +
		   "return false;" +
	      "else if (typeof p5 == 'undefined' || _isGmt(p5))" +
		   "return _timeRange(p1, p2, 0, p3, p4, 0, _isGmt(p5));" +
	      "else if (typeof p6 == 'undefined')" +
		   "return false;" +
	      "else " +
		   "return _timeRange(p1, p2, p3, p4, p5, p6, _isGmt(p7));" +
	"}" +
	"function _timeRange(hour1, min1, sec1, hour2, min2, sec2, gmt) {" +
	      "if (typeof hour1 != 'number' || typeof min1 != 'number' || typeof sec1 != 'number' " +
		  "|| hour1 < 0 || min1 < 0 || sec1 < 0 " +
		  "|| typeof hour2 != 'number' || typeof min2 != 'number' || typeof sec2 != 'number' " +
		  "|| hour2 < 0 || min2 < 0 || sec2 < 0 " +
		  "|| typeof gmt != 'boolean')  return false; " +
	      "var cur = new Date();" +
	      "var d1 = new Date();" +
	      "var d2 = new Date();" +
	      "d1.setHours(hour1);" +
	      "d1.setMinutes(min1);" +
	      "d1.setSeconds(sec1);" +
	      "d2.setHours(hour2);" +
	      "d2.setMinutes(min2);" +
	      "d2.setSeconds(sec2);" +
	      "if (gmt == true)" +
		  "cur = new Date(cur.getTime() - cur.getTimezoneOffset() * 60 * 1000);" +
	      "return ((d1.getTime() <= cur.getTime()) && (cur.getTime() <= d2.getTime()));" +
	"}";

    public static final String jsEnableWeekdayRange = 
	"function weekdayRange(wd1, wd2, gmt){" +
	      "if (typeof wd1 == 'undefined') " +
		  "return false;" +
	      "else if (typeof wd2 == 'undefined' || _isGmt(wd2)) " +
		  "return _weekdayRange(wd1, wd1, _isGmt(wd2)); " +
	      "else " +
		  "return _weekdayRange(wd1, wd2, _isGmt(gmt)); }" +
	"function _weekdayRange(wd1, wd2, gmt) {" +
	      "if (typeof wd1 != 'string' || typeof wd2 != 'string' || typeof gmt != 'boolean') return false; " +
	      "var w1 = -1, w2 = -1;" +
	      "for (var i=0; i < _day.length; i++) {" +
		  "if (_day[i] == wd1)" +
		      "w1 = i;" +
		  "if (_day[i] == wd2)" +
		      "w2 = i; }" +
	      "var cur = new Date();" +
	      "if (gmt == true)" +
		  "cur = new Date(cur.getTime() - cur.getTimezoneOffset() * 60 * 1000);" +
	      "var w3 = cur.getDay();" +
	      "if (w1 > w2)" +
		  "w2 = w2 + 7;" +
	      "if (w1 > w3)" +
		  "w3 = w3 + 7;" +
	      "return (w1 <= w3 && w3 <= w2); }";
}


