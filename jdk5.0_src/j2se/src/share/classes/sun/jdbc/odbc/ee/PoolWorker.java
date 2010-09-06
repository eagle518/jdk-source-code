/*
 * @(#)PoolWorker.java	1.0 02/04/05
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.jdbc.odbc.ee;

/**
 * A worker thread maintains the Object pool.
 *
 * @version	1.0, 05/04/02
 * @author	Binod P.G
 */ 
public class PoolWorker extends Thread{
    
    /** ObjectPool **/
    private ObjectPool pool;

    /** worker states **/
    private final int NOTSTARTED = 0;
    private final int STARTED = 1;
    private final int STOPPED= 2;
            
    /** thread state **/
    private int state = NOTSTARTED;
    

    
    /**
     * Constructs a Worker thread.
     *
     * @param	pool	A connectionpool object.
     */
    public PoolWorker(ObjectPool pool) {
        this.pool = pool;
    }
    
    /**
     * Starts the thread in a controlled fashion.
     */
    public void start() {
    	if (state == NOTSTARTED) {
    	    state = STARTED;    	
    	    super.start();
    	}
    }
    
    /**
     * Set the flag to stop the worker thread.
     */
    public void release() {
        state = STOPPED;
        pool.markError("Pool maintenance stopped. Pool is either shutdown or there is an error!");
    }
    
    /**
     * Keep maintaining the pool at specific intervals.
     */
    public void run(){
	while (state == STARTED) {
	    try {
	        pool.getTracer().trace ("Worker Thread : Maintenance of " + pool.getName() + "started");
	        pool.maintain();
	        // If cold shutdown is completed, current pool size will be zero.
	        if ( pool.getCurrentSize() == 0 ) {
	            pool.shutDown(true); 
	        }
	        pool.getTracer().trace ("Worker Thread : Maintenance of " + pool.getName() + "completed");
	        sleep(pool.getMaintenanceInterval() * 1000);
	    }catch (InterruptedException ie) {
	        pool.markError("Maintenance Thread Interrupted : " + ie.getMessage());
	    }catch (Exception e) {
	        pool.markError("Maintenance Thread Error : " + e.getMessage());	    
	    } 
	}
    }    
}