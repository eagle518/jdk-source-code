/*
 * @(#)PooledObject.java	1.0 02/04/05
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.jdbc.odbc.ee;

import java.util.Properties;

/**
 * A  Pooled object interface.
 *
 * @version	1.0, 05/04/02
 * @author	Binod P.G
 */ 
public interface PooledObject {
 
    /** Object is checked in to the pool **/    
    public final int CHECKEDIN = 1;
    
    /** Object is checked out from the pool **/        
    public final int CHECKEDOUT = 2;
    
    /** Object is marked for sweep **/
    public final int MARKEDFORSWEEP = 3;            
     
    /**
     * Match the the pooled object for the given properties.
     *
     * @param	p	Properties while creating.
     * @return  True if matches.
     */    
    public abstract boolean isMatching(Properties p);
    
    /**
     * Checks whether the pooled object is usable or not.
     *
     * @return	True if it is usable.
     */
    public abstract boolean isUsable();    
    
    /**
     * Mark pooled object as usable.
     */
    public abstract void markUsable();    
        
    /**
     * Mark the object as checked out
     */    
    public abstract void checkedOut();  
     
    /**
     * Mark the object as checked in
     */         
    public abstract void checkedIn();
         
    /**
     * This marks the object for sweeping.
     */
    public abstract void markForSweep();
        
    /**
     * Checks whether the object is marked for sweeping.
     * 
     * @return	If it is already marked for sweeping.
     */
    public abstract boolean isMarkedForSweep();    
 
    /**
     * Destroys the pooled object. This will abruptly destroy the object.
     * Generally used when there is an error or connection pool is shutdown.
     */
    public abstract void destroy() throws Exception;
 
    /**
     * Get the time of creation of this object.
     *
     * @return the Time of creation of this object.
     */
     public abstract long getCreatedTime();           
          
}