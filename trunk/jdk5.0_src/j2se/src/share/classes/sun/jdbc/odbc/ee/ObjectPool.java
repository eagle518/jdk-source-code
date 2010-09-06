/*
 * @(#)ObjectPool.java	1.0 02/04/05
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.jdbc.odbc.ee;

import sun.jdbc.odbc.JdbcOdbcTracer;
import java.util.Properties;
import java.util.Enumeration;
import java.util.Hashtable;
import java.sql.SQLException;

/**
 * Abstract Object Pool. 
 * <p>
 * A <code> jdbcOdbcPooledObject </code> interface is defining a object to be pooled.
 * Protected methods can be overridden to reuse this class for pooling
 * any object, including <code> PooledConnection </code>.
 * </p>
 * <p>
 * Object pool is developed with good thread-safety. This makes concurrent access to the pool
 * little difficuilt. However, with a effective management of the pool sizes, maximum leverage
 * of pooling can be obtained.
 * </p>
 *
 * @version	1.0, 05/04/02
 * @author	Binod P.G
 */ 
public abstract class ObjectPool{
    
    /** Properties of ConnectionPoolDataSource **/
    private int initialSize, maxSize, minSize, maxIdleTime, timeoutFromPool, mInterval;
    
    /** Current number of pooled objects **/
    private int currentSize=0;    
    
    /** Logical Name of pool**/
    private String name;
    
    /** Pool of objects **/
    private Hashtable freePool;

    /** Pool of locked objects **/
    private Hashtable lockedObjects;

    /** Pool of objects to be cleaned **/
    private Hashtable garbagePool;

    /** Worker thread **/
    private PoolWorker worker;
    
    /** Tracer object **/
    private JdbcOdbcTracer tracer = new JdbcOdbcTracer();
    
    /** boolean checking whether pool is usable or not **/
    private boolean usable = true;

    /** boolean checking whether pool is initialized or not **/
    private boolean initialized = false;

    /** Error message **/
    private String errorMessage;
                
    /**
     * Construct a new Connection pool for a specific datasource.
     */
    public ObjectPool(String name) {
        this.name = name;
        worker = new PoolWorker(this);
        freePool = new Hashtable();
        lockedObjects = new Hashtable();
        garbagePool = new Hashtable();
    }
    
    /**
     * Set the properties of connection pool.
     *
     * @param	pr	PoolProperties
     * @throws SQLException	If Maintenance interval is zero.
     */
    public void setProperties(PoolProperties pr) throws SQLException{

        tracer.trace("Setting the properties in Pool");
        	      
        initialSize 	= pr.get(PoolProperties.INITIALPOOLSIZE);
        minSize 	= pr.get(PoolProperties.MINPOOLSIZE);
        maxSize 	= pr.get(PoolProperties.MAXPOOLSIZE);
        timeoutFromPool = pr.get(PoolProperties.TIMEOUTFROMPOOL);
        mInterval 	= pr.get(PoolProperties.MAINTENANCEINTERVAL);
        maxIdleTime 	= pr.get(PoolProperties.MAXIDLETIME);
                
        if (minSize > initialSize) {
            initialSize = minSize;
            tracer.trace("Connection Pool: Initial Size is set to Max Size ");       
        }        
        
        if (maxSize < initialSize && maxSize !=0 ) {
	    maxSize = initialSize;
            tracer.trace("Connection Pool: Maximum size is less than Initial size, using the Initial size ");
        } 
        
        if (mInterval == 0) {
            throw new SQLException("Maintenance interval cannot be zero");
        }
        
    }

    /**
     * Initializing the pool.
     *
     * @throws SQLException In case of error in initialize.
     */
    public void initializePool() throws SQLException {
        tracer.trace("Setting the properties in Pool");    
        if (initialized) {
            return;
        }
        initialized = true;
        fillThePool(initialSize);                
        worker.start();
    }    

    /**
     * Fill the pool.     
     *
     * @param	size	Size required for the pool.
     * @throws 	SQLException In case of error in creation of new objects.
     */    
    protected void fillThePool(int size) throws SQLException{
        tracer.trace("fillThePool: Filling the pool upto :"+size + "from :" + currentSize);
        // We dont need to fill the pool if it is not usable.
        if (!usable) {
            tracer.trace("The pool is marked non usable. Not filling the pool");
            return;
        }
        try {
            while (currentSize < size) {
                addNew(createObject());
            }
        } catch (Exception e) {
            tracer.trace("fillThePool: Exception thrown in filling."+e.getMessage());
            throw new SQLException(e.getMessage());
        }
    }
    
    /**
     * Creates a new Pooled object. 
     * 
     * @return a new PooledObject
     * @throws	SQLException	In case of error while creating the object.
     */
    protected PooledObject createObject() throws SQLException{
        return createObject(null);
    }
    
    /**
     * Creates a new Pooled object with a set of properties.
     * 
     * @return a new PooledObject
     * @throws SQLException In case of error while creating.
     */
    protected synchronized PooledObject createObject(Properties p) throws SQLException{
        PooledObject jpo = create(p);  
        currentSize++;
        return jpo;
    }            
    
    /**
     * Abstract method supposed to return a new pooled object.
     *
     * @return a new PooledObject.
     * @throws SQLException In case of error while creating the object.     
     */
    protected abstract PooledObject create(Properties p) throws SQLException;
    
    /**
     * Add the object to the pool
     *
     * @param	jpo	Pooled Object.
     */
    protected synchronized void addNew(PooledObject jpo) {
        freePool.put(jpo, new Long(System.currentTimeMillis()));
    }

    /**
     * Verifies the object and mark for sweeping.
     *
     * @param	jpo	PooledObject to be verified.
     * @return	A boolean indicating whether the marking is done or not.
     */
    protected boolean checkAndMark(PooledObject jpo) {
        if (freePool.containsKey(jpo)){
            long idleTime = ((Long) freePool.get(jpo)).longValue();
            boolean timedout = false;
            boolean idled = false;
            if ((jpo.getCreatedTime() + (timeoutFromPool * 1000)) < System.currentTimeMillis() && 
                (timeoutFromPool != 0) ) {                
                timedout = true;            
            }
            if ((idleTime + (maxIdleTime * 1000)) < System.currentTimeMillis() &&
                (maxIdleTime !=0)) {
                idled = true;
            }
            if ( timedout || idled || (! jpo.isUsable())) {
	       jpo.markForSweep();
	       garbagePool.put(jpo,"");
	       freePool.remove(jpo);
	       return true;
	    } else if (jpo.isMarkedForSweep()){
	       garbagePool.put(jpo,"");
	       freePool.remove(jpo);
	       return true;
	    }
	    return false;
	}
	return false;
    }
    
    /**
     * Actually destroy the object . This will be calles only while an error occurs or
     * while sutting down the pool.
     * 
     * @param	jpo	Object to be swept.
     * @param	pool	Internal hashtable
     */
    protected void destroyFromPool(PooledObject jpo,Hashtable pool) {
    	try {
	    jpo.destroy();
	} catch( Exception e) {
	    // Do nothing.
	    tracer.trace( "Connection Pool : Exception while destroying + e.getMessage()");
	}
	pool.remove(jpo);
	currentSize --;	
    }    
    
    /**
     * Get an object from the pool.
     *
     * @return a PooledObject.
     * @throws SQLException in case creation of new object fails or maximum limit is reached.
     */
    public synchronized PooledObject checkOut() throws SQLException{
        if (! usable) throw new SQLException(" Connection Pool: " + errorMessage);    
    	Enumeration e = freePool.keys();
    	while (e.hasMoreElements()) {
    	    PooledObject jpo = (PooledObject) e.nextElement();
            if ( (!checkAndMark(jpo)) && freePool.containsKey(jpo) ) {
                lockedObjects.put(jpo, "");
                freePool.remove(jpo);
                jpo.checkedOut();
                return jpo;
            }
    	}    	
    	if (currentSize < maxSize || maxSize == 0){
            PooledObject jpo = createObject();
            lockedObjects.put(jpo, "");
            jpo.checkedOut();                       
            return jpo;
    	}
    	throw new SQLException("Maximum limit has reached and no connection is free");
    } 
    
    /**
     * Get an object from the pool for the specified properties. A matching will be done
     * to get an appropriate pooledobject. If nothing is available, a new object will be created
     * depending on the maximum size of the pool.
     *
     * @return a PooledObject.
     * @throws SQLException in case creation of new object fails or maximum limit is reached.
     */
    public synchronized PooledObject checkOut(Properties p) throws SQLException{
        if (! usable) throw new SQLException(" Connection Pool: " + errorMessage);
    	Enumeration e = freePool.keys();
    	while (e.hasMoreElements()) {
    	    PooledObject jpo = (PooledObject) e.nextElement();
            if ( (!checkAndMark(jpo)) && freePool.containsKey(jpo) && jpo.isMatching(p) ) {
                lockedObjects.put(jpo, "");
                freePool.remove(jpo);
                jpo.checkedOut();
                return jpo;
            }
    	}    	
    	if (currentSize < maxSize || maxSize == 0){
            PooledObject jpo = createObject(p);
            lockedObjects.put(jpo, "");
            jpo.checkedOut();
            return jpo;
    	}
    	throw new SQLException("Maximum limit has reached and no connection is free");
    } 
    
    /**
     * This method is used to mimic a checkOut of a specific object which is already checkedIn.
     *
     * @return a PooledObject.
     * @throws SQLException in case creation of new object fails.
     */
    public synchronized void tryCheckOut(PooledObject jpo) throws SQLException{
        if ( (!checkAndMark(jpo)) && freePool.containsKey(jpo) ) {
            lockedObjects.put(jpo, "");
            freePool.remove(jpo);
            jpo.checkedOut();
        } else {
            throw new SQLException("Object is not available for use" + freePool.containsKey(jpo));
        }
    } 
           
    /**
     * Put an object back to the pool.
     *
     * @param	jpo	A PooledObject to put into the pool.
     */
    public synchronized void checkIn(PooledObject jpo) {
	boolean timedout = false;
        if ((jpo.getCreatedTime() + (timeoutFromPool * 1000)) < System.currentTimeMillis() && 
            (timeoutFromPool != 0) ) {                
             timedout = true;            
        }                    
        if ( timedout || (!jpo.isUsable())) {
            jpo.markForSweep();
	    garbagePool.put(jpo,"");
	    lockedObjects.remove(jpo);
	} else {	   
	    jpo.checkedIn();
	    freePool.put(jpo, new Long(System.currentTimeMillis()));
	    lockedObjects.remove(jpo);        	    
	}
    } 
    
    /**
     * Get the current size of the pool.
     *
     * @return the current size of pool.
     */
    public int getCurrentSize() {
        return currentSize;
    }
    
    /**
     * Get the maintenance interval for the pool.
     *
     * @return Maintenance Interval.
     */
    public int getMaintenanceInterval() {
        return mInterval;
    }
        
    /**
     * Set the tracer involved in this object pool.
     *
     * @param	tracer	A JdbcOdbcTracer object.
     */
    public void setTracer(JdbcOdbcTracer tracer) {
        if (tracer != null) {
            this.tracer = tracer;
        }
    }

    /**
     * Set the error flag and mark that the pool cannot be used.
     *
     * @param	errorMessage ErrorMessage
     */
    public void markError(String errorMessage) {
        usable = false;
        this.errorMessage = errorMessage;
    }
        
    /**
     * Get the tracer involved in this datasource.
     *
     * @return	A JdbcOdbcTracer object.
     */
    public JdbcOdbcTracer getTracer() {
        return tracer;
    }
        
    /**
     * Get the name of the Pool.
     *
     * @return Name of the pool.
     */
    public String getName() {
        return name;
    }    
    
    /**
     * Maintains the pool.
     * @throws an SQLException If an error occurs.
     */
    public void maintain() throws SQLException{
        tracer.trace("Before <maintenance> Locked :" + lockedObjects.size() + " free :" + freePool.size() + "garbage :" + garbagePool.size() + "current size :" + currentSize);
        Enumeration e = garbagePool.keys();
        while (e.hasMoreElements()) {
            PooledObject jpo = (PooledObject) e.nextElement();
            destroyFromPool(jpo, garbagePool);
        }
        synchronized(this){
            Enumeration e1 = freePool.keys();
            while (e1.hasMoreElements()) {
            	PooledObject jpo = (PooledObject) e1.nextElement();
            	checkAndMark(jpo);
            }
        }
        fillThePool(minSize);
        tracer.trace("Before <maintenance> Locked :" + lockedObjects.size() + " free :" + freePool.size() + "garbage :" + garbagePool.size() + "current size :" + currentSize);        
    }
    
    /**
     * Shutdown the pool. There are two ways of shutting down the pool.
     * <p>
     *	1. Hot shutdown  : This closes the pool immediately.
     * </p>
     * <p>
     *  2. Cold shutdown : No new connections will be issued from pool.
     *			   Existing connections will be allowed to timeout.
     * </p>
     * 
     * @param hotShutdown	A boolean indicating whether we need to shutdown 
     *				the pool immediately or not.
     */
    public void shutDown (boolean hotShutdown) {
        if (hotShutdown == true) {
            worker.release();        
            shutDownNow();
        } else {
	    markError("Being shut down now");            
        }
    }    
    
    /**
     * Clear all the connections and shutdown the pool.
     */
    private synchronized void shutDownNow() {
        try{
	    tracer.trace("Shutting down the pool");        
	    ConnectionPoolFactory.removePool(name);
	    Enumeration e = garbagePool.keys();
            while (e.hasMoreElements()) {
	        PooledObject jpo = (PooledObject) e.nextElement();
                destroyFromPool(jpo, garbagePool);
	    }    
	    e = freePool.keys();
            while (e.hasMoreElements()) {
	        PooledObject jpo = (PooledObject) e.nextElement();
                destroyFromPool(jpo, freePool);
	    }	    
	    e = lockedObjects.keys();
            while (e.hasMoreElements()) {
	        PooledObject jpo = (PooledObject) e.nextElement();
                destroyFromPool(jpo, lockedObjects);
	    }	    	    
        } catch(Exception e){
            e.printStackTrace();
            tracer.trace("An error occurred while shutting down " + e);
        }
    }
}
