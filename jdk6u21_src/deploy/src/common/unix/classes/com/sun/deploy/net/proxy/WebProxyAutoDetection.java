/*
 * @(#)WebProxyAutoDetection.java	1.52 04/28/2005
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.net.proxy;

import java.util.StringTokenizer;
import com.sun.deploy.util.Trace;

/**
 * The class supports then generation of a Web Proxy Auto Detection URL
 * based on the machine it is being run on.
 *
 * @author Ashley Woodsom
 */
public class WebProxyAutoDetection {
    
    private static final int MINIMUM_DOMAIN_LEVEL = 2;
    
    private static native String getFQHostName();
    
    
    /**
     * Count the levels of domains that exist in a domain name
     *
     * @param domain
     *            domain name being evaluated
     * @return the number of '.' sperated domains
     */
    private static int getDomainLevel( String domain ) {
        
        int tokenCount = 0;
        
        if( domain != null ) {
            StringTokenizer st = new StringTokenizer( domain, ".");
            tokenCount = st.countTokens();
        }
        
        return tokenCount;
    }
    
    /**
     * Generate the Web Proxy Detection URL to use for the current domain
     *
     * @return the url string, or null
     */
    public static String getWPADURL() {
        
        String url = null;
        String host = getFQHostName();
        String domainName = "";
        
        if( host != null ) {
            
            // The host name must get stripped from the domain name
            StringTokenizer st = new StringTokenizer( host, "." );
            
            // First token is the host name... gone
            st.nextToken();
            
            // Build the domain name with what is left
            while( st.hasMoreTokens() ) {
                domainName += st.nextToken();
                
                // Put the seperator back on
                if( st.hasMoreTokens() ) {
                    domainName += ".";
                }
            }
            
            // Make sure the Domain Name is big enough to support WPAD
            if( getDomainLevel(domainName) >= MINIMUM_DOMAIN_LEVEL ){
                
                // Build the URL
                url = buildWPADURL( domainName);
            } else {
                Trace.msgNetPrintln("net.proxy.browser.pDetectionError", new Object[] {domainName});
            }
        }
        
        // Return the valid URL or Null if none was created
        return url;
    }
    
    /**
     * Builds a WPAD URL from a given domain name
     *
     * @param domain
     *           FQDN of the computer
     * @return the Autodection URL string
     */
    private static String buildWPADURL( String domain ) {
        
        return "http://wpad." + domain + "/wpad.dat";
        
    }
 
}
