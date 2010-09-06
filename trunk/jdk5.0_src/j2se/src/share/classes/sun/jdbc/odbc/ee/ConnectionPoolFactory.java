/*
 * @(#)ConnectionPoolFactory.java	1.0 02/04/05
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.jdbc.odbc.ee;

import java.util.Hashtable;

/**
 * Connection Pool Factory. It keeps ConnectionPools referenced by its
 * name.
 *
 * @version	1.0, 05/04/02
 * @author	Binod P.G
 */ 
public class ConnectionPoolFactory {
    
    /** ConnectionPools**/
    private static Hashtable pools; 
        
    /**
     * Gets the connectionPool from the cache and returns.
     * If pool is not existing creates a pool.
     *
     * @param	dataSourceName Name of Datasource.
     * @return	A ConnectionPool object.
     */
    public static ConnectionPool obtainConnectionPool (String dataSourceName) {
        if (pools == null) {
    	    pools = new Hashtable();
    	}    
        if ( pools.containsKey(dataSourceName)  && (pools.get(dataSourceName) != null)) {
             return (ConnectionPool) pools.get(dataSourceName);
        } else {
             ConnectionPool pool = new ConnectionPool( dataSourceName);
             pools.put(dataSourceName, pool);
             return pool;
        }
    }
    
    /**
     * Removes a pool from ConnectionPoolFactory.
     *
     * @param	poolName	Name of the pool.
     */
    public static void removePool(String poolName){
        pools.remove(poolName);
    }
}