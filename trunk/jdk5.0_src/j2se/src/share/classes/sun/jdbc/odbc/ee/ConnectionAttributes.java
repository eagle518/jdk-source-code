/*
 * @(#)ConnectionAttributes.java	1.0 02/04/05
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.jdbc.odbc.ee;

import java.util.Properties;

/**
 * Connection Attributes for a <code> JdbcOdbcConnection </code>. This supplies
 * the Connection attributes to a datasource/connectionpool implementation.
 * <p>
 * Instance of this class maintains userid, password, url, charset and logintimeout.
 * </p>
 *
 * @version	1.0, 05/04/02
 * @author	Binod P.G
 */ 
public class ConnectionAttributes {

    /** Jdbc url **/
    private String url = null;
    
    /** Userid **/    
    private String user = null;
    
    /** Password **/    
    private String password = null;
    
    /** Charset **/    
    private String charSet = null;
    
    /** logintimeout **/
    private int loginTimeout = 0;
     
    /**
     * Constructor. 
     *
     * @param	url		Jdbc url
     * @param	user		Userid
     * @param	password	Password
     * @param	charset		Characterset
     * @param	timeout		Login timeout
     */    
    public ConnectionAttributes(String databaseName, String user, String password, String charset, int timeout) {
        this.url = "jdbc:odbc:"+databaseName;
        this.user = user;
        this.password = password;
        this.charSet = charset;
        this.loginTimeout = timeout;
    }
    
    /**
     * Gets the user-id for the connection.
     *
     * @return	UserId
     */
    public String getUser(){
    	return user;
    }
    
    /**
     * Gets the password for the connection.
     *
     * @return	Password
     */    
    public String getPassword(){
    	return password;
    }

    /**
     * Gets the Url for the connection.
     *
     * @return	Jdbc Url
     */     
    public String getUrl(){
    	return url;
    }        
    
    /**
     * Gets the Character encoding scheme for the connection.
     *
     * @return	Character set
     */    
    public String getCharSet(){
    	return charSet;
    }
    
    /**
     * Gets the Logintimeout for the connection.
     *
     * @return	login timeout
     */    
    public int getLoginTimeout(){
    	return loginTimeout;
    }
        
    /**
     * Gets properties for connection. This properties object is compatible with
     * the properties required by JdbcOdbcDriver object for creating a connection.
     * 
     * @return	properties
     */    
    public Properties getProperties() {
    	Properties p = new Properties();    
        if (charSet != null) {
    	    p.put("charSet",charSet);
        } 
	
	if (user != null) {
    	    p.put("user", user);
        }
	
	if (password != null) {
    	    p.put("password", password);
        }
	
	if (url != null){
	    p.put("url",url);	
	}	
	p.put("loginTimeout",""+loginTimeout);
   	return p;    
    }   
    
}