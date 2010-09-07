/*
 *  @(#)MozillaBrowserService.java	1.7 10/03/31 12:02:41
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * This class abstracts basic services Mozilla Java plugin wants from the 
 * Mozilla browser. This includes proxy/cookie etc.
 */

package sun.plugin2.main.server;

import java.net.URL;
import java.net.PasswordAuthentication;
import java.security.SecureRandom;
import com.sun.deploy.net.cookie.CookieHandler;
import com.sun.deploy.net.cookie.CookieUnavailableException;
import com.sun.deploy.net.proxy.BrowserProxyConfig;
import com.sun.deploy.net.proxy.BrowserProxyInfo;
import com.sun.deploy.net.proxy.ProxyHandler;
import com.sun.deploy.net.proxy.ProxyType;
import com.sun.deploy.services.Service;
import com.sun.deploy.security.AbstractBrowserAuthenticator;
import sun.plugin.services.BrowserService;
import sun.plugin2.util.SystemUtil;

public class MozillaBrowserService implements BrowserService {

    public Object getAppletContext() {
        throw new UnsupportedOperationException();
    }

    public Object getBeansContext() {
        throw new UnsupportedOperationException();
    }

    public boolean isIExplorer() { return false; }

    public boolean isNetscape() { return true; }

    public float getBrowserVersion() {
        throw new UnsupportedOperationException();
    }

    public boolean isConsoleIconifiedOnClose() {
        throw new UnsupportedOperationException();
    }

    public boolean installBrowserEventListener() {
        throw new UnsupportedOperationException();
    }

    public String mapBrowserElement(String rawName) {
        throw new UnsupportedOperationException();
    }

    public BrowserProxyConfig getProxyConfig() {
        return new BrowserProxyConfig() {
                 public BrowserProxyInfo getBrowserProxyInfo() {
                     BrowserProxyInfo info = new BrowserProxyInfo(); 
                     info.setType(ProxyType.BROWSER);
                     return info;
                 }

                /**
                 * add system proxy info to BrowserProxyInfo
                 */
                public void getSystemProxy(BrowserProxyInfo bpi) {
                    throw new UnsupportedOperationException();
                }

            };
    }

    /*
     * Gnome desktop users may be able to set the system-wide proxy setting
     * through Mozilla networking menu. This isn't supported by Windows.
     */
    public ProxyHandler getSystemProxyHandler() {
        int osType = SystemUtil.getOSType();
        if (osType == SystemUtil.WINDOWS) {
            throw new UnsupportedOperationException();
        } else {
            try {
                return (ProxyHandler)
                    (Class.forName("com.sun.deploy.net.proxy.MSystemProxyHandler").
                     newInstance());
            } catch (Exception cnfEx) {
                return null;
            }
        }
    }

    /*
     * Returns the browser proxy handler.
     */
    public ProxyHandler getBrowserProxyHandler() {
        return new MozillaProxyHandler();
    }

    /*
     * Returns the browser auto-proxy handler.
     */
    public ProxyHandler getAutoProxyHandler() {
        int osType = SystemUtil.getOSType();

	try { 
            if (osType == SystemUtil.WINDOWS) {
                return (ProxyHandler)
		    (Class.forName("com.sun.deploy.net.proxy.WMozillaAutoProxyHandler").newInstance());
	    }
	    else {
            	return (ProxyHandler)
		    (Class.forName("com.sun.deploy.net.proxy.DummyAutoProxyHandler").newInstance());
	    }
        } catch (Exception cnfEx) {
            return null;
        }
    }
    
    /* 
     * Returns the cookie handler.
     */
    public CookieHandler getCookieHandler() {
        return new MozillaCookieHandler();
    }

    
    /**
     * Returns the browser authenticator.
     */
    public com.sun.deploy.security.BrowserAuthenticator getBrowserAuthenticator() {
        return new MozillaBrowserAuthenticator();
    }

    
    /**
     * Return browser signing root certificate store. 
     */
    public com.sun.deploy.security.CertStore getBrowserSigningRootCertStore()
    {
        return null;
    }

    /**
     * Return browser SSL root certificate store. 
     */
    public com.sun.deploy.security.CertStore getBrowserSSLRootCertStore()
    {
        return null;
    }

    /**
     * Return browser trusted signing certificate store. 
     */
    public com.sun.deploy.security.CertStore getBrowserTrustedCertStore()
    {
	// No system default trusted signing cert store is in Mozilla
	return null;
    }

    /**
     * Return browser client authentication key store. 
     */
    public java.security.KeyStore getBrowserClientAuthKeyStore()
    {
        return null;
    }

     /**
     * Return offline handler.
     */
    public com.sun.deploy.net.offline.OfflineHandler getOfflineHandler()
    {
	return null;
    }

    private Service getPlatformService()
    {
        int osType = SystemUtil.getOSType();
       
        try {
            if (osType == SystemUtil.WINDOWS) {
                return (Service)
                    (Class.forName("com.sun.deploy.services.WPlatformService").newInstance());
            } else if (osType == SystemUtil.UNIX) {
                return (Service)
                    (Class.forName("com.sun.deploy.services.MPlatformService").newInstance());
            } else {
                throw new UnsupportedOperationException();
            }
        } catch (Exception cnfEx) {
            return null;
        }
    }
    
    /**
     * Return secure random. This should belong to Platform specific service.
     * Ideally, we should have ServiceManager to return platform and browser
     * specific service rather than mixing them together. The current Service
     * mechanism should be cleaned up.
     */
    public SecureRandom getSecureRandom() 
    {
        Service platformService = getPlatformService();

        return platformService != null ? platformService.getSecureRandom() : null;
    }

    /**
     * Return the Credential Manager. Same as getSecureRandom(), this should go 
     * to a different class. 
     */
    public com.sun.deploy.security.CredentialManager getCredentialManager() 
    {
        Service platformService = getPlatformService();
        
        return platformService != null ? platformService.getCredentialManager() : null;
    }

    class MozillaProxyHandler extends com.sun.deploy.net.proxy.AbstractBrowserProxyHandler
    {
        /**
         * <p> method to obtain the proxy string when automatic proxy config
         * is used. </p>
         *
         * @param url URL.
         * @returns Proxy string.
         */
        protected String findProxyForURL(String url) {
            return ((MozillaPlugin) ProxySupport.getCurrentPlugin()).getProxy(url);
        }
    }
        
    // This is unused in the current implementation;
    // see MozillaPlugin.getCookie() / setCookie()
    class MozillaCookieHandler implements com.sun.deploy.net.cookie.CookieHandler 
    {
        public void setCookieInfo(URL url, String value)
            throws CookieUnavailableException
        {
            throw new CookieUnavailableException("Should not call this");
        }
        
        /**
         * <p> Returns the corresponding cookie value with respect to the given URL.
         * </p>
         *
         * @param url URL
         * @returns String contains the corresponding cookie value.
         */
        public String getCookieInfo(URL url)
            throws CookieUnavailableException
        {
            throw new CookieUnavailableException("Should not call this");
        }
    }


    // This is unused in the current implementation;
    // see MozillaPlugin.getAuthentication()
    class MozillaBrowserAuthenticator extends com.sun.deploy.security.AbstractBrowserAuthenticator {
	public PasswordAuthentication getAuthentication(String protocol, String siteName, int port, 
	    String scheme, String realm, URL requestingURL, boolean proxy) {
            throw new RuntimeException("Should not call this");
	}
    }
}
