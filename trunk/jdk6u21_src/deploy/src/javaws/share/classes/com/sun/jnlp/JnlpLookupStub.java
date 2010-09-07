/*
 * @(#)JnlpLookupStub.java	1.32 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
            JNLPClassLoaderIf loader = JNLPClassLoaderUtil.getInstance();
            if (loader == null) {
                return null;
            }
            if (name.equals("javax.jnlp.BasicService")) {
                // Check if it is the javax.jnlp.BasisService
                return loader.getBasicService();
            } else if (name.equals("javax.jnlp.FileOpenService")) {
                // Check if it is the javax.jnlp.FileOpenService
                return loader.getFileOpenService();
            } else if (name.equals("javax.jnlp.FileSaveService")) {
                // Check if it is the javax.jnlp.FileSaveService
                return loader.getFileSaveService();
            } else if (name.equals("javax.jnlp.ExtensionInstallerService")) {
                // Check if it is the javax.jnlp.ExtensionInstallerService
                // (This service is only available for installers)
                return loader.getExtensionInstallerService();
            } else if (name.equals("javax.jnlp.DownloadService")) {
                // Check if it is the javax.jnlp.DownloadService
                return loader.getDownloadService();
            } else if (name.equals("javax.jnlp.ClipboardService")) {
                // Check if it is the javax.jnlp.ClipboardService
                return loader.getClipboardService();
            } else if (name.equals("javax.jnlp.PrintService")) {
                // Check if it is the javax.jnlp.PrintService
                return loader.getPrintService();
            } else if (name.equals("javax.jnlp.PersistenceService")) {
                // Check if it is the javax.jnlp.PersistenceService
                return loader.getPersistenceService();
            } else if (name.equals("javax.jnlp.ExtendedService")) {
                // Check if it is the javax.jnlp.ExtendedService
                return loader.getExtendedService();
            } else if (name.equals("javax.jnlp.SingleInstanceService")) {
                // Check if it is the javax.jnlp.SingleInstanceService
                return loader.getSingleInstanceService();
            } else if (name.equals("javax.jnlp.IntegrationService")) {
                return loader.getIntegrationService();
            } else if (name.equals("javax.jnlp.DownloadService2")) {
                return loader.getDownloadService2();
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
                    "javax.jnlp.SingleInstanceService",
                    "com.sun.jnlp.IntegrationService"
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



