/*
 * @(#)BrowserKeystore.java	1.18 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.security;

import java.lang.reflect.Constructor;
import java.lang.reflect.Field;
import java.lang.reflect.InvocationHandler;
import java.lang.reflect.Method;
import java.lang.reflect.Proxy;
import java.security.Provider;
import java.security.Security;
import java.util.HashMap;
import java.text.MessageFormat;
import javax.swing.JButton;
import javax.swing.JPasswordField;
import com.sun.deploy.config.Config;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.services.Service;
import com.sun.deploy.util.Trace;
import com.sun.deploy.ui.UIFactory;
import java.net.PasswordAuthentication;


public class BrowserKeystore 
{

    /**
     * Register security providers for browser keystore.
     */
    public static void registerSecurityProviders() {

	// Setup browser keystore support for 1.5 or later
	if (Config.isJavaVersionAtLeast15())
	{
	    Service service = com.sun.deploy.services.ServiceManager.getService();

	    if (service.isIExplorer())
	    {
		try
		{
		    // Setup MS crypto provider if available
		    Class providerClass = Class.forName("com.sun.deploy.security.MSCryptoProvider", true,
							 ClassLoader.getSystemClassLoader());
            	
		    if (providerClass != null)
		    {
			// Create provider instance
			Provider provider = (Provider) providerClass.newInstance();
                	
			// Insert provider to the end of the provider list
			Security.insertProviderAt(provider, Security.getProviders().length + 1);
		    }
		}
		catch (Throwable e)
		{
		}
	    }

	    // We only check JSS package when using Mozilla browser 
	    if (service.isNetscape())
	    { 
		// Check if JSS is configured
		if (isJSSCryptoConfigured())
		{
		    Trace.msgSecurityPrintln("browserkeystore.jss.config");
	    	    try
            	    {
			// Setup Mozilla JSS provider if available
			Class providerClass = Class.forName("com.sun.deploy.security.MozillaJSSProvider", true,
							    ClassLoader.getSystemClassLoader());

			if (providerClass != null)
			{
                    	    // Create provider instance
                    	    Provider provider = (Provider) providerClass.newInstance();

                    	    // Insert provider to the end of the provider list
                    	    Security.insertProviderAt(provider, Security.getProviders().length + 1);
			}
            	    }
		    catch (Throwable e)
		    {
			Trace.msgSecurityPrintln("browserkeystore.jss.notconfig");
		    }
		}
		else
		{
		    Trace.msgSecurityPrintln("browserkeystore.jss.notconfig");
		}
	    }
	}
    }

    private static Object cryptoManager = null;
    private static boolean initializeJSS = false;

    public static synchronized Object getJSSCryptoManager() 
    {
        if (cryptoManager == null && initializeJSS == false)
        {
	    initializeJSS = true;
	    
            // Determine Mozilla user profile location
	    String mozillaUserProfileDir = null;

	    if (Config.getInstance().isBrowserFireFox()) {
               mozillaUserProfileDir = Config.getInstance().getFireFoxUserProfileDirectory();
	    }
	    else {
               mozillaUserProfileDir = Config.getInstance().getMozillaUserProfileDirectory();
	    }
	    Trace.msgSecurityPrintln("browserkeystore.mozilla.dir", new Object[] {mozillaUserProfileDir});

	    if (mozillaUserProfileDir != null)
            {
		try {		
		    // org.mozilla.jss.CryptoManager.InitializationValues initializationValues = new InitializationValues(configDir);
		    // initializationValues.installJSSProvider = false;				    
		    Class jssInitializationValues = Class.forName("org.mozilla.jss.CryptoManager$InitializationValues", true,
								   ClassLoader.getSystemClassLoader());
		    Constructor constructorInitializationValues = jssInitializationValues.getConstructor(new Class[]{String.class});
		    Object initializationValues = constructorInitializationValues.newInstance(new Object[]{mozillaUserProfileDir});
		    Field fieldInstallJSSProvider = jssInitializationValues.getField("installJSSProvider");
		    fieldInstallJSSProvider.setBoolean(initializationValues, false);
		    
		    // Initialize JSS crypto manager using reflection
		    //org.mozilla.jss.CryptoManager.initialize(initializationValues);
		    //cryptoManager = org.mozilla.jss.CryptoManager.getInstance();
		    Class jssCryptoManager = Class.forName("org.mozilla.jss.CryptoManager", true,
							    ClassLoader.getSystemClassLoader());
							    
		    Method inimeth = jssCryptoManager.getMethod("initialize", new Class[] {jssInitializationValues});
		    
		    // Invoke initialize method
		    Object retobj = inimeth.invoke(null, new Object[] {initializationValues} );

		    // Invoke getInstance method
		    Method ginstmeth = jssCryptoManager.getMethod("getInstance", null);
		    Object cm = ginstmeth.invoke(null, null);

		    // org.mozilla.jss.CrytoManager.setPasswordCallback(new JSSPasswordCallbackProxy());
		    //
		    Class jssPasswordCallback = Class.forName("org.mozilla.jss.util.PasswordCallback", true,
							      ClassLoader.getSystemClassLoader());
		    Method setPasswordCallbackMethod = jssCryptoManager.getMethod("setPasswordCallback", new Class[]{jssPasswordCallback} );

		    // Create proxy for PasswordCallback
		    InvocationHandler handler = new JSSPasswordCallbackInvocationHandler();
		    Class proxyClass = Proxy.getProxyClass(ClassLoader.getSystemClassLoader(), new Class[]{jssPasswordCallback});
		    Object passwordCallback = proxyClass.getConstructor(new Class[] { InvocationHandler.class }).newInstance(new Object[] { handler });

		    // Invoke setPasswordCallback method
		    setPasswordCallbackMethod.invoke(cm, new Object[]{passwordCallback});

		    // Set CrytoManager only if it is fully initialized.
		    cryptoManager = cm;
		    
		    Trace.msgSecurityPrintln("browserkeystore.jss.yes");
		}
		catch (ClassNotFoundException cnfe) {		
		    Trace.msgSecurityPrintln("browserkeystore.jss.no");
		    return null;
		}
		catch (Throwable e)
		{ 
		    e.printStackTrace();
	  	    Trace.msgSecurityPrintln("browserkeystore.jss.no");
		    return null;
		}
	    }
        }

        return cryptoManager;
    }

    /**
     * Return true if JSS crypto is configured.
     */
    public static boolean isJSSCryptoConfigured()
    {
	// Check JSS package 
	return (getJSSCryptoManager() != null);
    }
    
    /**
     * JSSPasswordCallbackInvocationHandler is a handler for PasswordCallback proxy.
     */
    private static class JSSPasswordCallbackInvocationHandler implements java.lang.reflect.InvocationHandler
    {
	// Hash map to store number of password attempts for a particular token.
	private HashMap passwordAttempts = new HashMap();
    
	public Object invoke(Object proxy, Method method, Object[] args) throws Throwable
	{
	    if (args != null && args[0] != null)    {

		// String tokenName = PasswordCallbackInfo.getName();
		Object passwordCallbackInfo = args[0];
		Class clazzPasswordCallbackInfo = passwordCallbackInfo.getClass();
		Method getNameMethod = clazzPasswordCallbackInfo.getMethod("getName", null);
		String tokenName = (String) getNameMethod.invoke(passwordCallbackInfo, null);
		String methodName = method.getName();

		// Determine how many attempts have been for the token's password
		Integer attempts = (Integer) passwordAttempts.get(tokenName);

		// If user have attempted for more than two incorrect passwords, dismiss.
		if (attempts == null || attempts.intValue() < 2)    
		{		    
		    if (attempts == null)
			passwordAttempts.put(tokenName, new Integer(1));
		    else
			passwordAttempts.put(tokenName, new Integer(attempts.intValue() + 1));
    		
		    // Popup UI for password
		    char pwArray[] = getPasswordDialog(tokenName);

		    // org.mozilla.jss.util.Password pw = new org.mozilla.jss.util.Password(tokenpass);
		    Class clazzPassword = Class.forName("org.mozilla.jss.util.Password", true, ClassLoader.getSystemClassLoader());
		    Class[] argClass = new Class[] {char[].class};
		    Constructor passwordConstructor = clazzPassword.getConstructor(argClass);

		    // Return if password is entered		
		    if (pwArray != null)	
		    {
			Object arglist[] = new Object[] { pwArray }; 
			return passwordConstructor.newInstance(arglist);    		
		    }    
		}
	    }	
	    	
	    // Class jssPasswordCallbackGiveUpException = PasswordCallback.GiveUpException.class;
	    Class jssPasswordCallbackGiveUpException = Class.forName("org.mozilla.jss.util.PasswordCallback$GiveUpException", true,
								     ClassLoader.getSystemClassLoader());
								     
	    throw (Throwable) jssPasswordCallbackGiveUpException.newInstance();								     	    
	}

	private char[] getPasswordDialog(final String tokenName) 
	{  
            MessageFormat mf = new MessageFormat(ResourceManager.getMessage(
                                    "browserkeystore.password.dialog.text"));
	    Object [] args = { tokenName };
	    String dialogMsg = mf.format(args);

            // Pass translated strings to the dialog.  
            CredentialInfo passwordInfo = 
                    UIFactory.showPasswordDialog(null, 
                    ResourceManager.getMessage("password.dialog.title"), 
                    dialogMsg, false, false, null, false);    
                    
            // If user pressed "Cancel" in password dialog, return value
            // from dialog will be null.  Check here if user pressed "Cancel".
            if ( passwordInfo == null ) {
                return null;
            } else {
                // return password
                return passwordInfo.getPassword();
            }
	}
    }
}
