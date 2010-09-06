/*
 * @(#)JdbcOdbcUtils.java	1.0 02/03/07
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.jdbc.odbc;

import java.util.Date;
import java.util.Calendar;

/**
 * Common utility class for jdbc-odbc bridge.
 *
 * @version	1.0, 07/02/02
 * @author	Binod P.G
 */ 
public class JdbcOdbcUtils {

    /**
     * Converts the date (in GMT) to a milliseconds equivalent of a date in the 
     * calendar timezone.
     * 
     * @param date Input date to be converted
     * @param cal  Calendar object which hold the timezone information
     * @return a long equivalent of converted datetime data.
     */			
    public long convertFromGMT (java.util.Date date, Calendar cal){
	long offsetFromUTC = cal.getTimeZone().getRawOffset();		
	return date.getTime() + offsetFromUTC;
    }
    
    /**
     * Converts date to GMT timezone from the timezone of cal and returns 
     * a millisecond equivalent.
     * 
     * @param date Input date to be converted
     * @param cal  Calendar object which hold the timezone information
     * @return a long equivalent of converted datetime data.
     */	
    public long convertToGMT (java.util.Date date, Calendar cal){		
	long offsetFromUTC = cal.getTimeZone().getRawOffset();
	return date.getTime() - offsetFromUTC;
    }
	    
    /**
     * Constructor.
     */
    public JdbcOdbcUtils() {
    	// Nothing to do.
    }    
    
}
