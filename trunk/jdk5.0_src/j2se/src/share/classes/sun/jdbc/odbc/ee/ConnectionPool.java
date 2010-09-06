/*
 * @(#)ConnectionPool.java	1.0 02/04/05
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.jdbc.odbc.ee;

import java.util.Properties;
import java.util.Hashtable;
import java.sql.SQLException;

/**
 * Actual Connection Pool. There will be one connection pool for each
 * binding of a <code> ConnectionPoolDataSource </code>. This is maintained by a 
 * ConnectionPoolFactory.
 * <p>
 * Apart from the behaviour of <code> ObjectPool </code>, this class applies a 
 * <code> ConnectionEventListener </code> to the pool, which take care connection events.
 * </p>
 * <p>
 * This class overrides implement create method in the <code> ObjectPool </code> to
 * return a new object for the pool. Also it overrides <code> destroyFromPool </code> to
 * handle the <code> ConnectionEventListener </code>.
 * </p>
 *
 * @see ConnectionPoolFactory
 *
 * @version	1.0, 05/04/02
 * @author	Binod P.G
 */ 
public class ConnectionPool extends ObjectPool{        
        
    /** ConnectionEventListener for this pool **/
    private ConnectionEventListener cel;

    /** Connection properties **/
    Properties cp;
    
    /**
     * Construct a new Connection pool for a specific datasource.
     *
     * @param	Name of the connection pool.
     */
    public ConnectionPool(String name) {
        super(name);
        cel = new ConnectionEventListener(name);
    }    
    
    /**
     * Set Connection Properties. These properties will be used as the
     * default properties to create the connection.
     *
     * @param	p   Connection properties.
     */
    public void setConnectionDetails(Properties p) {
        this.cp = p;
    }
        
    /**
     * Creates a new Pooled Connection. If this method is called with a 
     * null argument, new connection will be created with default connection 
     * properies. If properies are passed correctly , that will be used for creating
     * the connection.
     * 
     * @param	p	Properties for the connection.
     * @return a new PooledObject
     * @throws SQLException	If creating the PooledObject fails.
     */
    protected PooledObject create(Properties p) throws SQLException{
    	Properties info = null;
    	if (p!=null){
    	   info = p;
    	} else {
    	   info = cp;
    	}
        PooledConnection jpc = new PooledConnection(info, super.getTracer());  
        jpc.addConnectionEventListener(cel);
        return jpc;  
    }            
    
    /**
     * Actually destroy the object . This will be called only while an error occurs or
     * while sutting down the pool.
     * 
     * @param	jpc	Object to be destroyed.
     * @param	pool	hashtable where the pooledobject resides.
     */
    protected void destroyFromPool(PooledObject jpc,Hashtable pool) {
    	super.destroyFromPool(jpc,pool);
        ((PooledConnection)jpc).removeConnectionEventListener(cel);	
    }    
    
}
