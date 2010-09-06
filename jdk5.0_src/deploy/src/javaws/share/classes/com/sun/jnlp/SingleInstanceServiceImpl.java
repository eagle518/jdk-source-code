/*
 * @(#)SingleInstanceServiceImpl.java	1.11 04/04/07
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jnlp;

import java.net.*;
import javax.jnlp.*;
import com.sun.javaws.jnl.LaunchDesc;
import com.sun.javaws.jnl.XMLFormat;
import com.sun.javaws.*;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;
import com.sun.deploy.config.Config;
import com.sun.deploy.si.*;
import java.security.AccessController;
import java.security.PrivilegedAction;

public final class SingleInstanceServiceImpl extends SingleInstanceImpl implements SingleInstanceService {
    static private SingleInstanceServiceImpl _sharedInstance = null;

    public static synchronized SingleInstanceServiceImpl getInstance() {
        if (_sharedInstance == null) {
            _sharedInstance = new SingleInstanceServiceImpl();
        }
        return _sharedInstance;
    }


    public void addSingleInstanceListener(javax.jnlp.SingleInstanceListener sil) {
	if (sil == null) return;

	// generate the unique jnlp url
	final LaunchDesc ld = JNLPClassLoader.getInstance().getLaunchDesc();
	URL jnlpUrl = ld.getCanonicalHome();
	final String jnlpUrlString = jnlpUrl.toString();

	AccessController.doPrivileged(new PrivilegedAction() {
		public Object run() {
		    if (SingleInstanceManager.isServerRunning(jnlpUrlString)) {
			
		
			String[] appArgs = Globals.getApplicationArgs();
			
			if (appArgs != null) {	
			    ld.getApplicationDescriptor().setArguments(appArgs);
			}
			// send the JNLP file to the server port
			if (SingleInstanceManager.connectToServer(
								  ld.toString())) {
			    // if we get OK from server, we are done
			  
			    Main.systemExit(0);
			}
			// else continue
		
		    }
		    return null;
		}
	    });

	super.addSingleInstanceListener(new TransferListener(sil), jnlpUrlString);

    }

    public void removeSingleInstanceListener(javax.jnlp.SingleInstanceListener sil) {
	super.removeSingleInstanceListener(new TransferListener(sil));
    }


    // overrides isSame in com.sun.deploy.SingleInstanceImpl
    public boolean isSame(String inputString, String idString) {
        LaunchDesc ld = null;
        // extract the jnlp href
        try {
            ld = XMLFormat.parse(inputString.getBytes());
        } catch (Exception e) {
            Trace.ignoredException(e);
        }

        if (ld != null) {
            URL url = ld.getCanonicalHome();
            Trace.println("GOT: " + url.toString(), TraceLevel.BASIC);

            if (idString.equals(url.toString())) {
		return true;
	    }
	}
	return false;

    }

    // overrides getArguments in com.sun.deploy.SingleInstanceImpl
    public String[] getArguments(String inputString, String idString) {
        LaunchDesc ld = null; 
        // extract the jnlp href 
        try { 
            ld = XMLFormat.parse(inputString.getBytes()); 
        } catch (Exception e) { 
            Trace.ignoredException(e); 
        } 
 
        if (ld != null) { 
	    return ld.getApplicationDescriptor().getArguments();
	}
        return new String[0];
    }

    private class TransferListener implements DeploySIListener {
	SingleInstanceListener _sil;

	public TransferListener( SingleInstanceListener sil) {
	    _sil = sil;
        }

	public void newActivation(String [] params) {
	    _sil.newActivation(params);
	}

	public Object getSingleInstanceListener() {
	    return _sil;
	}
    }

}
