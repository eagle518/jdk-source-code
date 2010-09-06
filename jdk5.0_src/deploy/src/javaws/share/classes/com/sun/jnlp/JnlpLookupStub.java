/*
 * @(#)JnlpLookupStub.java	1.26 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.jnlp;
import javax.jnlp.ServiceManagerStub;
import javax.jnlp.UnavailableServiceException;
import java.security.AccessController;
import java.security.PrivilegedAction;
 
/**
 * The JnlpStub implements the lookup of services for Java Web Start
 * Only a single instance of this object exists at any
 * given time.
 */
public final class JnlpLookupStub implements ServiceManagerStub {
    
    public Object lookup(final String name) throws UnavailableServiceException {
	// This might initialize a service, so do this with all permissions
        Object service = (Object)AccessController.doPrivileged(new PrivilegedAction() {
	    public Object run() {
	
		return findService(name);
	    }
	});
	
        if (service == null) throw new UnavailableServiceException(name);

        return service;
	
    }
    
    private Object findService(String name) {
	if (name != null) {
            if (name.equals("javax.jnlp.BasicService")) {
                // Check if it is the javax.jnlp.BasisService
                return BasicServiceImpl.getInstance();
            } else if (name.equals("javax.jnlp.FileOpenService")) {
                // Check if it is the javax.jnlp.FileOpenService
                return FileOpenServiceImpl.getInstance();
            } else if (name.equals("javax.jnlp.FileSaveService")) {
                // Check if it is the javax.jnlp.FileSaveService
                return FileSaveServiceImpl.getInstance();
            } else if (name.equals("javax.jnlp.ExtensionInstallerService")) {
                // Check if it is the javax.jnlp.ExtensionInstallerService
                // (This service is only available for installers)
                return ExtensionInstallerServiceImpl.getInstance();
            } else if (name.equals("javax.jnlp.DownloadService")) {
                // Check if it is the javax.jnlp.DownloadService
                return DownloadServiceImpl.getInstance();
            } else if (name.equals("javax.jnlp.ClipboardService")) {
                // Check if it is the javax.jnlp.ClipboardService
                return ClipboardServiceImpl.getInstance();
            } else if (name.equals("javax.jnlp.PrintService")) {
                // Check if it is the javax.jnlp.PrintService
                return PrintServiceImpl.getInstance();
            } else if (name.equals("javax.jnlp.PersistenceService")) {
                // Check if it is the javax.jnlp.PersistenceService
                return PersistenceServiceImpl.getInstance();
	    } else if (name.equals("javax.jnlp.ExtendedService")) {
		// Check if it is the javax.jnlp.ExtendedService
		return ExtendedServiceImpl.getInstance();
	    } else if (name.equals("javax.jnlp.SingleInstanceService")) {
		// Check if it is the javax.jnlp.SingleInstanceService
		return SingleInstanceServiceImpl.getInstance();
	    }
	}

	return null;
    } 


    public String[] getServiceNames() {
        if (ExtensionInstallerServiceImpl.getInstance() != null) {
            return new String[]  {
                    "javax.jnlp.BasicService",
                    "javax.jnlp.FileOpenService",
                    "javax.jnlp.FileSaveService",
                    "javax.jnlp.ExtensionInstallerService",
                    "javax.jnlp.DownloadService",
                    "javax.jnlp.ClipboardService",
                    "javax.jnlp.PersistenceService",
                    "javax.jnlp.PrintService",
		    "javax.jnlp.ExtendedService", 
		    "javax.jnlp.SingleInstanceService" 
            };
        } else {
            return new String[]  {
                    "javax.jnlp.BasicService",
                    "javax.jnlp.FileOpenService",
                    "javax.jnlp.FileSaveService",
                    "javax.jnlp.DownloadService",
                    "javax.jnlp.ClipboardService",
                    "javax.jnlp.PersistenceService",
                    "javax.jnlp.PrintService",
		    "javax.jnlp.ExtendedService", 
		    "javax.jnlp.SingleInstanceService" 
            };
        }
    }
}



