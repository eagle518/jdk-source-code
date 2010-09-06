/*
 * @(#)PoolProperties.java	1.0 02/04/05
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.jdbc.odbc.ee;

import java.util.Hashtable;

/**
 * Properies of the ObjectPool. This defines a set of properties required
 * by the Object Pool.
 *
 * @version	1.0, 05/04/02
 * @author	Binod P.G
 */ 
public class PoolProperties {
     
    /**  Minimum pool size **/    
    public final static String MINPOOLSIZE = "minPoolSize";
    
    /**  Maximum pool size  **/    
    public final static String MAXPOOLSIZE = "maxPoolSize";    

    /**  Initial pool size  **/    
    public final static String INITIALPOOLSIZE = "initialPoolSize";    

    /**  Maximum Idle Time of an Object **/    
    public final static String MAXIDLETIME = "maxIdleTime";
    
    /**  Timeout of an object from the pool **/    
    public final static String TIMEOUTFROMPOOL = "timeOutFromPool";        
    
    /** Maintenance Interval of Pool **/
    public final static String MAINTENANCEINTERVAL = "mInterval";    
    
    /** Hashtable which keeps the properties **/
    private Hashtable properties = new Hashtable();         
    
    /**
     * Constructs the properties with default values.
     */
    public PoolProperties(){
        properties.put(MINPOOLSIZE, new Integer(0));
        properties.put(MAXPOOLSIZE, new Integer(0));
        properties.put(INITIALPOOLSIZE, new Integer(0));
        properties.put(MAXIDLETIME, new Integer(0));
        properties.put(TIMEOUTFROMPOOL, new Integer(0));
        properties.put(MAINTENANCEINTERVAL, new Integer(0));
    }
    
    /**
     * Get the value of property based on the key.
     *
     * @param	key	Key of the property.
     */
    public int get(String key){
    	return ((Integer) properties.get(key)).intValue();
    }
    
    /**
     * Set the property value.
     *
     * @param	key	Key of the property.
     * @param	value	Value of the property.
     */
    public void set(String key, int value){
    	properties.put(key, new Integer(value));
    }
}