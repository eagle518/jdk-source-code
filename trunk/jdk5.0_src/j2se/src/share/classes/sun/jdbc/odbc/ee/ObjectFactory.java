/*
 * @(#)ObjectFactory.java	1.0 02/04/05
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.jdbc.odbc.ee;

import sun.jdbc.odbc.*;
import javax.naming.*;
import javax.naming.spi.*;
import java.util.Hashtable;

/**
 * Objectfactory implementation required by JNDI.
 *
 * @version	1.0, 05/04/02
 * @author	Binod P.G
 */ 
public class ObjectFactory implements javax.naming.spi.ObjectFactory {

    /**
     * This method is used by JNDI to reconstruct the DataSource/ConnectionPoolDataSource
     * object when retrieved.
     *
     * @param	refObj	Referenced object
     * @param	name	Name
     * @param	ctx	Naming context
     * @param	env	Environment
     * @return	Reconstructed Datasource object
     */
    public Object getObjectInstance(Object refObj, Name name, Context ctx,
				    Hashtable<?,?> env) 
	throws Exception
    {
    	Reference ref = (Reference) refObj;
    	String className = ref.getClassName();
    	String dbn = (String) ref.get("databaseName").getContent();
    	String dsn = (String) ref.get("dataSourceName").getContent();
    	String user = (String) ref.get("user").getContent();
    	String password = (String) ref.get("password").getContent();
    	String charSet = (String) ref.get("charSet").getContent();    	
    	int loginTimeout = Integer.parseInt((String) ref.get("loginTimeout").getContent());
    	
    	if ( className.equals("sun.jdbc.odbc.ee.DataSource") ){
    	    DataSource ds = new DataSource();
    	    if (dbn != null) ds.setDatabaseName(dbn);
    	    if (dsn != null) ds.setDataSourceName(dsn);
    	    if (user != null) ds.setUser(user);    	        	    
    	    if (password != null) ds.setPassword(password); 
	    if (charSet != null) ds.setCharSet(charSet);    	    
    	    ds.setLoginTimeout(loginTimeout);   
    	    return ds;
    	}
    	
    	if ( className.equals("sun.jdbc.odbc.ee.ConnectionPoolDataSource") ){
    	    if (dsn==null) {
    	        throw new NamingException("Datasource Name is null for a connection pool");
    	    }
    	    ConnectionPoolDataSource jcp = new ConnectionPoolDataSource(dsn);
	    String maxStatements = (String) ref.get("maxStatements").getContent();
	    String initialPoolSize = (String) ref.get("initialPoolSize").getContent();
	    String minPoolSize = (String) ref.get("minPoolSize").getContent();
	    String maxPoolSize = (String) ref.get("maxPoolSize").getContent();	    
	    String maxIdleTime = (String) ref.get("maxIdleTime").getContent();
	    String propertyCycle = (String) ref.get("propertyCycle").getContent();
	    String timeoutFromPool = (String) ref.get("timeoutFromPool").getContent();	    	    	    	    	    
	    String mInterval = (String) ref.get("mInterval").getContent();
	    if (dbn != null) jcp.setDatabaseName(dbn);
	    if (user != null) jcp.setUser(user);
	    if (password != null) jcp.setPassword(password);
	    if (charSet != null) jcp.setCharSet(charSet);
	    jcp.setLoginTimeout(loginTimeout);	    	    	    
	    jcp.setMaxStatements(maxStatements);
	    jcp.setInitialPoolSize(initialPoolSize);
	    jcp.setMinPoolSize(minPoolSize);
	    jcp.setMaxPoolSize(maxPoolSize);	    
	    jcp.setMaxIdleTime(maxIdleTime);
	    jcp.setPropertyCycle(propertyCycle);
	    jcp.setTimeoutFromPool(timeoutFromPool);
	    jcp.setMaintenanceInterval(mInterval);
	    return jcp;	    	    	    	    	    	    	    	    	    	    	    	    	    	    
    	}
    	 
    	if ( className.equals("sun.jdbc.odbc.ee.XADataSource") ){
		// To be implemented
    	}
    	 	
    	return null;
    }

}
