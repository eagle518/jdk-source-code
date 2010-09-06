/*
 * @(#)JdbcOdbcPlatform.java	1.3 02/07/01
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/**
 * Class for returning platform information in JDBC-ODBC bridge. 
 * <p>
 * This class returns platform information and provides methods for
 * converting integer types into byte arrays depending on the
 * platform.
 * </p>
 *
 * @version	1.0, 27/05/02
 * @author	Evani Sai Surya Kiran
 */ 

package sun.jdbc.odbc;

import java.sql.*;

public class JdbcOdbcPlatform {

    /**
     * This function returns if the platform is 32-bit
     * @return	true,  if platform is 32 bit
     *        	false, otherwise.
     */	 
    public static boolean is32BitPlatform() {
        if(sizeofSQLLEN == 4) {
            return true;
        }
        return false;
    }
	 
    /**
     * This function returns if the platform is 64-bit
     * @return	true,  if platform is 64 bit
     *        	false, otherwise.
     */	 
    public static boolean is64BitPlatform() {
        if(sizeofSQLLEN == 8) {
            return true;
        }
        return false;
    }
	 
    /**
     * This function returns the platform dependent size of a length buffer.
     * @return	size of length buffer
     */	 
    public static int getLengthBufferSize() {
        return(sizeofSQLLEN);
    }
	 
    /**
     * This function returns a platform dependent byte representation of
     * an integer.
     * @param	integer to be converted to bytes
     * @return	platform dependent byte representation of the integer passed
     */
    public static byte[] convertIntToByteArray(int i) {
        byte[] b = new byte[sizeofSQLLEN];
        JdbcOdbc.intToBytes(i, b);
        return b;
    }
	
    /**
     * This function returns a platform dependent byte representation of
     * a long.
     * @param	long to be converted to bytes
     * @return	platform dependent byte representation of the long passed
     */
    public static byte[] convertLongToByteArray(long l) {
        byte[] b = new byte[sizeofSQLLEN];
        JdbcOdbc.longToBytes(l, b);
        return b;
    }
	
    static final int sizeofSQLLEN = JdbcOdbc.getSQLLENSize();

}
