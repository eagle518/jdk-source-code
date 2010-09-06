/*
 * @(#)ConnectionEventListener.java	1.0 02/04/05
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.jdbc.odbc.ee;

import javax.sql.ConnectionEvent;
import java.sql.SQLException;

/**
 * This implements <code> javax.sql.ConnectionEventListener </code> interface. 
 * <p>
 * Each <code> ConnectionPool </code> have one <code> ConnectionEventListener </code>. 
 * <code> ConnectionEventListener </code> is added to each <code> PooledConnection </code> instance, 
 * which handles close and error events in the <code> Connection </code> and 
 * <code> PooledConnection </code>. <code> ConnectionPool </code> uses this to get an 
 * "inuse" <code> Connection </code> back to the pool, while application program closes the connection.
 * </p>
 *
 * @version	1.0, 05/04/02
 * @author	Amit Handa
 */ 

public class ConnectionEventListener implements javax.sql.ConnectionEventListener{

    /** Object to keep the source of error  **/
    private PooledObject objPool;	

    /** Name of pool **/
    private String name;

    /**
     * Constructs a new listener with the pool's name.     
     *
     * @param	name Name of the pool.
     */
    public ConnectionEventListener(String name){
        this.name = name;
    }
     
    /**
     * This method is triggered while a connection is closed. <code> ConnectionEventListener </code>
     * analyses the event and put the object back to the pool for use.
     *
     * @see javax.sql.ConnectionEvent
     * @param	event	ConnectionEvent
     */    
    public void connectionClosed(ConnectionEvent event){
	Object obj = event.getSource();
	objPool = (PooledObject)obj;
	ConnectionPool pool = ConnectionPoolFactory.obtainConnectionPool(name);
	pool.checkIn(objPool);
    }

    /**
     * This method is triggered while an error is closed. <code> ConnectionEventListener </code>
     * analyses the event and put the object back to the pool for sweeping.
     *
     * @see javax.sql.ConnectionEvent
     * @param	event	An event created with exception.
     */    
    public void connectionErrorOccurred(ConnectionEvent event){
	Object obj = event.getSource();
	objPool = (PooledObject)obj;
	objPool.markForSweep();
	ConnectionPool pool = ConnectionPoolFactory.obtainConnectionPool(name);
	pool.checkIn(objPool);	
    }
    
    /**
     * If an application server keeps a cache of PooledConnections, it is possible that, it may 
     * attempt getting <code> ConnectionHandler </code> without checking out. In that case, initiates the
     * checkout process.
     *
     * @param	event	A connectionevent
     * @throws	SQLException  If checkout is not successful.
     */
    public void connectionCheckOut(ConnectionEvent event) throws SQLException{
	Object obj = event.getSource();
	objPool = (PooledObject)obj;
	ConnectionPool pool = ConnectionPoolFactory.obtainConnectionPool(name);
	pool.tryCheckOut(objPool);        	     
    }
}